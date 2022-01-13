/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <surface.h>
#include <unistd.h>

#include "command/rs_base_node_command.h"
#include "command/rs_display_node_command.h"
#include "command/rs_surface_node_command.h"
#include "common/rs_common_def.h"
#include "display_type.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImageInfo.h"
#include "pipeline/rs_render_result.h"
#include "pipeline/rs_render_thread.h"
#include "png.h"
#include "render_context/render_context.h"
#include "string_ex.h"
#include "surface_buffer.h"
#include "ui/rs_node.h"
#include "ui/rs_surface_extractor.h"
#include "ui/rs_ui_director.h"
#include "unique_fd.h"
#include "transaction/rs_interfaces.h"
#include "ui/rs_display_node.h"
#include "ui/rs_surface_node.h"
// temporary debug
#include "foundation/graphic/standard/rosen/modules/render_service_base/src/platform/ohos/rs_surface_frame_ohos.h"
#include "foundation/graphic/standard/rosen/modules/render_service_base/src/platform/ohos/rs_surface_ohos.h"


using namespace OHOS;
using namespace OHOS::Rosen;
using namespace std;

namespace detail {
constexpr int MILLI_SECS_PER_SECOND = 1000;
constexpr int MICRO_SECS_PER_MILLISECOND = 1000;
constexpr int MICRO_SECS_PER_SECOND = MICRO_SECS_PER_MILLISECOND * MILLI_SECS_PER_SECOND;
constexpr uint8_t BIT_DEPTH_VALUE = 8;
constexpr int SLEEP_TIME = 20;
constexpr int MAX_BACKLIGHT = 100;

RenderContext* renderContext = nullptr;

template <typename Duration>
using SysTime = std::chrono::time_point<std::chrono::system_clock, Duration>;
using SysMicroSeconds = SysTime<std::chrono::microseconds>;

using WriteToPngParam = struct {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t bitDepth;
    const uint8_t *data;
};

bool WriteToPng(const std::string &fileName, const WriteToPngParam &param)
{
    png_structp pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (pngStruct == nullptr) {
        std::cout << "png_create_write_struct error, nullptr!" << std::endl;
        return false;
    }
    png_infop pngInfo = png_create_info_struct(pngStruct);
    if (pngInfo == nullptr) {
        std::cout << "png_create_info_struct error, nullptr!" << std::endl;
        png_destroy_write_struct(&pngStruct, nullptr);
        return false;
    }
    FILE *fp = fopen(fileName.c_str(), "wb");
    if (fp == nullptr) {
        std::cout << "open file error, nullptr!" << std::endl;
        png_destroy_write_struct(&pngStruct, &pngInfo);
        return false;
    }
    png_init_io(pngStruct, fp);

    // set png header
    png_set_IHDR(pngStruct, pngInfo,
        param.width, param.height,
        param.bitDepth,
        PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);
    png_set_packing(pngStruct); // set packing info
    png_write_info(pngStruct, pngInfo); // write to header

    for (uint32_t i = 0; i < param.height; i++) {
        png_write_row(pngStruct, param.data + (i * param.stride));
    }

    png_write_end(pngStruct, pngInfo);

    // free
    png_destroy_write_struct(&pngStruct, &pngInfo);
    int ret = fclose(fp);
    if (ret != 0) {
        std::cout << "close fp failed" << std::endl;
    }
    return true;
}

uint64_t MicroSecondsSinceEpoch()
{
    SysMicroSeconds tmp = std::chrono::system_clock::now();
    return tmp.time_since_epoch().count();
}

std::string FormattedTimeStamp()
{
    std::stringstream ss;
    auto microsecondsSinceEpoch = MicroSecondsSinceEpoch();
    auto seconds = static_cast<time_t>(microsecondsSinceEpoch / MICRO_SECS_PER_SECOND);

    ::tm* localTime = ::localtime(&seconds);

    ss << std::setw(4) << (localTime->tm_year + 1900) << "-"
       << std::setfill('0') << std::setw(2) << (localTime->tm_mon + 1) << "-"
       << std::setfill('0') << std::setw(2) << localTime->tm_mday << "_"
       << std::setfill('0') << std::setw(2) << localTime->tm_hour << "-"
       << std::setfill('0') << std::setw(2) << localTime->tm_min << "-"
       << std::setfill('0') << std::setw(2) << localTime->tm_sec << ".";
    auto micros = microsecondsSinceEpoch % MICRO_SECS_PER_SECOND;
    ss << std::setfill('0') << std::setw(6) << micros;
    return ss.str();
}

#ifdef ACE_ENABLE_GL
RenderContext* GetRenderContext() {
    if (renderContext != nullptr) {
        return renderContext;
    }

    renderContext = RenderContextFactory::GetInstance().CreateEngine();
    renderContext->InitializeEglContext();
    return renderContext;
}
#endif // ACE_ENABLE_GL
} // namespace detail

std::unique_ptr<RSSurfaceFrame> framePtr;

void DrawSurface(
    SkRect surfaceGeometry, uint32_t color, SkRect shapeGeometry, std::shared_ptr<RSSurfaceNode> surfaceNode)
{
    auto x = surfaceGeometry.x();
    auto y = surfaceGeometry.y();
    auto width = surfaceGeometry.width();
    auto height = surfaceGeometry.height();
    surfaceNode->SetBounds(x, y, width, height);
    std::shared_ptr<RSSurface> rsSurface = RSSurfaceExtractor::ExtractRSSurface(surfaceNode);
    if (rsSurface == nullptr) {
        return;
    }
#ifdef ACE_ENABLE_GL
    rsSurface->SetRenderContext(detail::GetRenderContext());
#endif // ACE_ENABLE_GL
    auto framePtr = rsSurface->RequestFrame(width, height);
    if (framePtr == nullptr) {
        std::cout << __func__ << ": framePtr is nullptr!" << std::endl;
        return;
    }
    auto canvas = framePtr->GetCanvas();
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kFill_Style);
    paint.setStrokeWidth(20);
    paint.setStrokeJoin(SkPaint::kRound_Join);
    paint.setColor(color);

    canvas->drawRect(shapeGeometry, paint);
    framePtr->SetDamageRegion(0, 0, width, height);
    auto framPtr1 = std::move(framePtr);
    rsSurface->FlushFrame(framPtr1);
}

std::shared_ptr<RSSurfaceNode> CreateSurface()
{
    RSSurfaceNodeConfig config;
    return RSSurfaceNode::Create(config);
}

// Toy DMS.
using DisplayId = ScreenId;
class MyDMS {
private:
    struct Display {
        DisplayId id;
        RSScreenModeInfo activeMode;
    };
    std::unordered_map<DisplayId, Display> displays_;
    mutable std::recursive_mutex mutex_;
    RSInterfaces& rsInterface_;
    DisplayId defaultDisplayId_;

    void Init()
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        rsInterface_.SetScreenChangeCallback([this](ScreenId id, ScreenEvent event) {
            switch (event) {
                case ScreenEvent::CONNECTED: {
                    this->OnDisplayConnected(id);
                    break;
                }
                case ScreenEvent::DISCONNECTED: {
                    this->OnDisplayDisConnected(id);
                    break;
                }
                default:
                    break;
            }
        });

        defaultDisplayId_ = rsInterface_.GetDefaultScreenId();
        displays_[defaultDisplayId_] =
            Display { defaultDisplayId_, rsInterface_.GetScreenActiveMode(defaultDisplayId_) };

        auto supportModes = rsInterface_.GetScreenSupportedModes(defaultDisplayId_);
        std::cout << "supportModes size: " << supportModes.size() << std::endl;
        for (const auto &mode : supportModes) {
            std::cout << "ModeId: " << mode.GetScreenModeId() << ":\n";
            std::cout << "Width: " << mode.GetScreenWidth() << ", Height: " << mode.GetScreenHeight();
            std::cout << ", FreshRate: " << mode.GetScreenFreshRate() << "Hz.\n";
            std::cout << "---------------------------------------" << std::endl;
        }

        auto capabilities = rsInterface_.GetScreenCapability(defaultDisplayId_);
        std::cout << "capabilities:\n " << "name: " << capabilities.GetName() << "\n";
        std::cout << "type: " << capabilities.GetType() << "\n";
        std::cout << "phyWidth: " << capabilities.GetPhyWidth() << "\n";
        std::cout << "phyHeight: " << capabilities.GetPhyHeight() << "\n";
        std::cout << "supportLayers: " << capabilities.GetSupportLayers() << "\n";
        std::cout << "virtualDispCount: " << capabilities.GetVirtualDispCount() << "\n";
        std::cout << "---------------------------------------" << std::endl;
    }

public:
    MyDMS() : rsInterface_(RSInterfaces::GetInstance())
    {
        Init();
    }
    ~MyDMS() noexcept = default;

    DisplayId GetDefaultDisplayId()
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        return defaultDisplayId_;
    }

    DisplayId CreateVirtualDisplay(
        const std::string &name,
        uint32_t width,
        uint32_t height,
        sptr<Surface> surface,
        ScreenId mirrorId,
        int flags)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto displayId = rsInterface_.CreateVirtualScreen(name, width, height, surface, mirrorId, flags);
        displays_[displayId] = Display {displayId, {}};
        return displayId;
    }

    int32_t GetDisplayBacklight(DisplayId id)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (displays_.count(id) == 0) {
            cout << "MyDMS: No display " << id << "!" << endl;
            return -1;
        }
        return rsInterface_.GetScreenBacklight(id);
    }

    void SetDisplayBacklight(DisplayId id, uint32_t level)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (displays_.count(id) == 0) {
            cout << "MyDMS: No display " << id << "!" << endl;
            return;
        }
        return rsInterface_.SetScreenBacklight(id, level);
    }

    std::optional<RSScreenModeInfo> GetDisplayActiveMode(DisplayId id) const
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (displays_.count(id) == 0) {
            cout << "MyDMS: No display " << id << "!" << endl;
            return {};
        }
        return displays_.at(id).activeMode;
    }

    void OnDisplayConnected(ScreenId id)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        displays_[id] = Display { id, rsInterface_.GetScreenActiveMode(id) };
        std::cout << "MyDMS: Display " << id << " connected." << endl;
    }

    void OnDisplayDisConnected(ScreenId id)
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        if (displays_.count(id) == 0) {
            std::cout << "MyDMS: No display " << id << "!" << std::endl;
        } else {
            std::cout << "MyDMS: Display " << id << " disconnected." << endl;
            displays_.erase(id);
            if (id == defaultDisplayId_) {
                defaultDisplayId_ = rsInterface_.GetDefaultScreenId();
                std::cout << "MyDMS: DefaultDisplayId changed, new DefaultDisplayId is" << defaultDisplayId_ << "."
                          << endl;
            }
        }
    }
};
MyDMS g_dms;

class ImageReader {
public:
    ImageReader() {}
    ~ImageReader() noexcept
    {
    }

    bool Init()
    {
        csurface_ = Surface::CreateSurfaceAsConsumer();
        if (csurface_ == nullptr) {
            // log
            return false;
        }

        auto producer = csurface_->GetProducer();
        psurface_ = Surface::CreateSurfaceAsProducer(producer);
        if (psurface_ == nullptr) {
            // log
            return false;
        }

        listener_ = new BufferListener(*this);
        SurfaceError ret = csurface_->RegisterConsumerListener(listener_);
        if (ret != SURFACE_ERROR_OK) {
            // log
            return false;
        }
        return true;
    }

    sptr<Surface> GetSurface() const
    {
        return psurface_;
    }
private:
    class BufferListener : public IBufferConsumerListener {
    public:
        BufferListener(ImageReader &imgReader) : imgReader_(imgReader)
        {
        }
        ~BufferListener() noexcept override = default;
        void OnBufferAvailable() override
        {
            imgReader_.OnVsync();
        }

    private:
        ImageReader &imgReader_;
    };
    friend class BufferListener;

    void DumpBuffer(const sptr<SurfaceBuffer> &buf)
    {
        BufferHandle *bufferHandle =  buf->GetBufferHandle();
        if (bufferHandle == nullptr) {
            printf("bufferHandle nullptr!\n");
            return;
        }

        uint32_t width = bufferHandle->width;
        uint32_t height = bufferHandle->height;
        uint32_t stride = bufferHandle->stride;
        uint8_t *addr = (uint8_t *)buf->GetVirAddr();

        detail::WriteToPngParam param;
        param.width = width;
        param.height = height;
        param.data = addr;
        param.stride = stride;
        param.bitDepth = detail::BIT_DEPTH_VALUE;

        if (access("/data", F_OK) < 0) {
            std::cout << "ImageReader::DumpBuffer(): Can't access data directory!" << std::endl;
            return;
        }

        std::string timeStamp = detail::FormattedTimeStamp();
        std::string dumpFileName = "/data/dumpImg-" + std::to_string(getpid()) + "-" + timeStamp + ".png";

        bool ret = WriteToPng(dumpFileName, param);
        if (ret) {
            std::cout << "ImageReader::Dumpbuffer(): dump" << dumpFileName << std::endl;
        }
    }

    void OnVsync()
    {
        std::cout << "ImageReader::OnVsync" << std::endl;

        sptr<SurfaceBuffer> cbuffer = nullptr;
        int32_t fence = -1;
        int64_t timestamp = 0;
        Rect damage;
        auto sret = csurface_->AcquireBuffer(cbuffer, fence, timestamp, damage);
        UniqueFd fenceFd(fence);
        if (cbuffer == nullptr || sret != OHOS::SURFACE_ERROR_OK) {
            std::cout << "ImageReader::OnVsync: surface buffer is null!" << std::endl;
            return;
        }

        DumpBuffer(cbuffer);

        if (cbuffer != prevBuffer_) {
            if (prevBuffer_ != nullptr) {
                SurfaceError ret = csurface_->ReleaseBuffer(prevBuffer_, -1);
                if (ret != SURFACE_ERROR_OK) {
                    std::cout << "ImageReader::OnVsync: release buffer error! err: " << ret << std::endl;
                    return;
                }
            }

            prevBuffer_ = cbuffer;
        }
    }

    sptr<IBufferConsumerListener> listener_;
    sptr<Surface> csurface_; // cosumer surface
    sptr<Surface> psurface_; // producer surface
    sptr<SurfaceBuffer> prevBuffer_;
};

int main()
{
    auto transactionProxy = RSTransactionProxy::GetInstance();
    if (transactionProxy == nullptr) {
        return -1;
    }

    DisplayId id = g_dms.GetDefaultDisplayId();
    std::cout << "level =" << g_dms.GetDisplayBacklight(id) << std::endl;
    int lightValue = 0;
    g_dms.SetDisplayBacklight(id, lightValue);
    std::cout << "after set to 0, level =" << g_dms.GetDisplayBacklight(id) << std::endl;
    lightValue = detail::MAX_BACKLIGHT;
    g_dms.SetDisplayBacklight(id, lightValue);
    std::cout << "after set to 100, level =" << g_dms.GetDisplayBacklight(id) << std::endl;

    ImageReader imgReader;
    if (!imgReader.Init()) {
        cout << "ImgReader init failed!" << endl;
    }
    DisplayId virtualDisplayId = g_dms.CreateVirtualDisplay(
        "virtualDisplay", 480, 300, imgReader.GetSurface(), INVALID_SCREEN_ID, -1);
    cout << "VirtualScreenId: " << virtualDisplayId << endl;
    cout << "-------------------------------------------------------" << endl;

    RSDisplayNodeConfig config {virtualDisplayId};
    RSDisplayNode::SharedPtr displayNode = RSDisplayNode::Create(config);
    auto surfaceNode = CreateSurface();
    DrawSurface(SkRect::MakeXYWH(0, 0, 200, 200), 0xffa10f1b, SkRect::MakeXYWH(0, 0, 200, 200), surfaceNode);
    displayNode->AddChild(surfaceNode, -1);
    transactionProxy->FlushImplicitTransaction();
    while (1) {
        sleep(detail::SLEEP_TIME);
        DrawSurface(SkRect::MakeXYWH(0, 0, 200, 200), 0xffa10f1b, SkRect::MakeXYWH(0, 0, 200, 200), surfaceNode);
        transactionProxy->FlushImplicitTransaction();
    }

    return 0;
}