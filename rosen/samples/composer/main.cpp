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

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <securec.h>
#include <iostream>
#include <unistd.h>

#include <vsync_helper.h>
#include <display_type.h>
#include <surface.h>
#include "hdi_backend.h"
#include "hdi_layer.h"
#include "hdi_layer_info.h"
#include "hdi_output.h"
#include "hdi_screen.h"

using namespace OHOS;
using namespace OHOS::Rosen;
using namespace std;

namespace {
#define LOG(fmt, ...) ::OHOS::HiviewDFX::HiLog::Info(             \
    ::OHOS::HiviewDFX::HiLogLabel {LOG_CORE, 0, "Hellocomposer"}, \
    "%{public}s: " fmt, __func__, ##__VA_ARGS__)
}

class HelloComposer : public IBufferConsumerListenerClazz {
public:
    void Init(int32_t width, int32_t height, HdiBackend* backend);
    void DoDrawData(void *image, int width, int height);
    void Draw();
    void Sync(int64_t, void *data);
    void CreatePyhsicalScreen(std::shared_ptr<HdiOutput> &output);
    void DoPrepareCompleted(OHOS::sptr<Surface> &surface, const struct PrepareCompleteParam &param);
    virtual void OnBufferAvailable() override;
    SurfaceError ProduceBuffer(OHOS::sptr<Surface> &produceSurface, int width, int height);
    bool FillLayer(std::shared_ptr<HdiLayerInfo> &showLayer, OHOS::sptr<Surface> &pSurface,
        OHOS::sptr<Surface> &cSurface, uint32_t zorder, IRect &dstRect);

    int32_t freq = 30;
    int width = 0;
    int height = 0;
    int display_w = 480;
    int display_h = 960;

private:
    uint32_t currentModeIndex = 0;
    std::vector<DisplayModeInfo> displayModeInfos;
    OHOS::sptr<Surface> pSurface = nullptr;
    OHOS::sptr<Surface> cSurface = nullptr;
    OHOS::sptr<Surface> pSurface1 = nullptr;
    OHOS::sptr<Surface> cSurface1 = nullptr;
    std::unique_ptr<HdiScreen> screen = nullptr;
    std::shared_ptr<HdiOutput> output = nullptr;
    std::vector<std::shared_ptr<HdiOutput>> outputs;
    HdiBackend* backend = nullptr;
    sptr<SurfaceBuffer> fbBuffer = nullptr;
    bool ready = false;
};

void HelloComposer::OnBufferAvailable()
{
}

void HelloComposer::DoDrawData(void *image, int width, int height)
{
    static uint32_t value = 0xff0000ff;

    uint32_t *pixel = static_cast<uint32_t *>(image);
    for (int x = 0; x < width; x++) {
        for (int y = 0;  y < height; y++) {
            *pixel++ = value;
        }
    }
}

SurfaceError HelloComposer::ProduceBuffer(OHOS::sptr<Surface> &produceSurface, int width, int height)
{
    OHOS::sptr<SurfaceBuffer> buffer;
    int32_t releaseFence;
    BufferRequestConfig config = {
        .width = width,
        .height = height,
        .strideAlignment = 0x8,
        .format = PIXEL_FMT_RGBA_8888,
        .usage = produceSurface->GetDefaultUsage(),
    };

    SurfaceError ret = produceSurface->RequestBufferWithFence(buffer, releaseFence, config);
    if (ret != 0) {
        LOG("RequestBuffer failed: %{public}s", SurfaceErrorStr(ret).c_str());
        return ret;
    }
    if (buffer == nullptr) {
        LOG("%s: buffer is nullptr", __func__);
        return SURFACE_ERROR_NULLPTR;
    }

    auto addr = static_cast<uint8_t *>(buffer->GetVirAddr());
    LOG("buffer w:%{public}d h:%{public}d stride:%{public}d", buffer->GetWidth(), buffer->GetHeight(), buffer->GetBufferHandle()->stride);
    DoDrawData(addr, buffer->GetWidth(), buffer->GetHeight());

    BufferFlushConfig flushConfig = {
        .damage = {
            .w = width,
            .h = height,
        },
    };
    ret = produceSurface->FlushBuffer(buffer, -1, flushConfig);

    LOG("Sync %{public}s", SurfaceErrorStr(ret).c_str());
    return SURFACE_ERROR_OK;
}

void HelloComposer::Draw()
{
    do {
        static int count = 0;

        if (count >= 2000) {
            count = 0;
        }

        std::vector<LayerInfoPtr> layerVec;
        std::shared_ptr<HdiLayerInfo> showLayer = HdiLayerInfo::CreateHdiLayerInfo();
        std::shared_ptr<HdiLayerInfo> showLayer1 = HdiLayerInfo::CreateHdiLayerInfo();

        if (count <= 50 || count >= 500) {
            int32_t zorder = 0;
            IRect dstRect;
            dstRect.x = 0;  // Absolute coordinates, with offset
            dstRect.y = 0;
            dstRect.w = width;
            dstRect.h = height;
            if (!FillLayer(showLayer, pSurface, cSurface, zorder, dstRect)) {
                continue;
            }
            layerVec.emplace_back(showLayer);
        }

        if (count <= 100 || count >= 1000) {
            int zorder1 = 0;
            IRect dstRect1;
            dstRect1.x = width;  // Absolute coordinates, with offset
            dstRect1.y = width;
            dstRect1.w = width;
            dstRect1.h = height;
            if (!FillLayer(showLayer1, pSurface1, cSurface1, zorder1, dstRect1)) {
                continue;
            }
            layerVec.emplace_back(showLayer1);
        }
        LOG("Draw count:%{public}d layer size %{public}d", count, (int32_t)layerVec.size());
        output->SetLayerInfo(layerVec);
        backend->Repaint(outputs);
        count++;
    } while (false);
}

bool HelloComposer::FillLayer(std::shared_ptr<HdiLayerInfo> &showLayer, OHOS::sptr<Surface> &pSurface,
        OHOS::sptr<Surface> &cSurface, uint32_t zorder, IRect &dstRect)
{
    if (ProduceBuffer(pSurface, width, height) != SURFACE_ERROR_OK) {
        LOG("Produce cBuffer failed");
        return false;
    }

    OHOS::sptr<SurfaceBuffer> cbuffer = nullptr;
    int32_t fence;
    int64_t timestamp;
    Rect damage;
    SurfaceError ret = cSurface->AcquireBuffer(cbuffer, fence, timestamp, damage);
    if (ret != SURFACE_ERROR_OK) {
        LOG("Acquire cBuffer failed");
        return false;
    }

    IRect srcRect;
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = width;
    srcRect.h = height;

    LayerAlpha alpha = { .enPixelAlpha = true };

    showLayer->SetSurface(cSurface);
    showLayer->SetBuffer(cbuffer, fence);
    showLayer->SetZorder(zorder);
    showLayer->SetAlpha(alpha);
    showLayer->SetTransform(TransformType::ROTATE_NONE);
    showLayer->SetCompositionType(CompositionType::COMPOSITION_DEVICE);
    showLayer->SetVisibleRegion(1, srcRect);
    showLayer->SetDirtyRegion(srcRect);
    showLayer->SetLayerSize(dstRect);
    showLayer->SetBlendType(BlendType::BLEND_SRCOVER);
    showLayer->SetCropRect(srcRect);
    showLayer->SetPreMulti(false);

    return true;
}

void HelloComposer::Sync(int64_t, void *data)
{
    cout << "hello Sync: ready" << ready << std::endl;
    struct OHOS::FrameCallback cb = {
        .frequency_ = freq,
        .timestamp_ = 0,
        .userdata_ = data,
        .callback_ = std::bind(&HelloComposer::Sync, this, SYNC_FUNC_ARG),
    };

    OHOS::VsyncError ret = OHOS::VsyncHelper::Current()->RequestFrameCallback(cb);
    if (ret) {
        LOG("RequestFrameCallback inner %{public}d\n", ret);
    }

    if (!ready) {
        return;
    }

    Draw();
}

void HelloComposer::Init(int32_t width, int32_t height, HdiBackend* backend)
{
    this->backend = backend;

    cSurface = Surface::CreateSurfaceAsConsumer();
    cSurface->SetDefaultWidthAndHeight(width, height);
    cSurface->SetDefaultUsage(HBM_USE_CPU_READ | HBM_USE_CPU_WRITE | HBM_USE_MEM_DMA);

    OHOS::sptr<IBufferProducer> producer = cSurface->GetProducer();
    pSurface = Surface::CreateSurfaceAsProducer(producer);
    cSurface->RegisterConsumerListener(this);
    LOG("%s: create surface", __func__);

    cSurface1 = Surface::CreateSurfaceAsConsumer();
    cSurface1->SetDefaultWidthAndHeight(width, height);
    cSurface1->SetDefaultUsage(HBM_USE_CPU_READ | HBM_USE_CPU_WRITE | HBM_USE_MEM_DMA);

    OHOS::sptr<IBufferProducer> producer1 = cSurface1->GetProducer();
    pSurface1 = Surface::CreateSurfaceAsProducer(producer1);
    cSurface1->RegisterConsumerListener(this);
    LOG("%s: create surface1", __func__);

    Sync(0, nullptr);
}

void HelloComposer::CreatePyhsicalScreen(std::shared_ptr<HdiOutput> &output)
{
    std::cout << "CreatePyhsicalScreen begin" << std::endl;
    screen = HdiScreen::CreateHdiScreen(output->GetScreenId());
    screen->Init();
    screen->GetScreenSuppportedModes(displayModeInfos);
    this->output = output;
    outputs.push_back(output);
    int supportModeNum = displayModeInfos.size();
    if (supportModeNum > 0) {
        screen->GetScreenMode(currentModeIndex);
        LOG("currentModeIndex:%{public}d", currentModeIndex);
        for (int i = 0; i < supportModeNum; i++) {
            LOG("modes(%{public}d) %{public}dx%{public}d freq:%{public}d", displayModeInfos[i].id, displayModeInfos[i].width,
                displayModeInfos[i].height, displayModeInfos[i].freshRate);
            if (displayModeInfos[i].id == static_cast<int32_t>(currentModeIndex)) {
                this->freq = 30;
                this->display_w =  displayModeInfos[i].width;
                this->display_h = displayModeInfos[i].height;
                break;
            }
        }
        screen->SetScreenPowerStatus(DispPowerStatus::POWER_STATUS_ON);
    }
    ready = true;
    std::cout << "CreatePyhsicalScreen end " << std::endl;
}

void HelloComposer::DoPrepareCompleted(OHOS::sptr<Surface> &surface, const struct PrepareCompleteParam &param)
{
    if (param.needFlushFramebuffer) {
        BufferRequestConfig requestConfig = {
            .width = display_w,
            .height = display_h,
            .strideAlignment = 0x08,
            .format = PIXEL_FMT_BGRA_8888,
            .usage = HBM_USE_CPU_READ | HBM_USE_CPU_WRITE | HBM_USE_MEM_DMA,
            .timeout = 0,
        };

        BufferFlushConfig flushConfig = {
            .damage = {
                .w = display_w,
                .h = display_h,
            }
        };
        int releaseFence = -1;
        surface->RequestBufferWithFence(fbBuffer, releaseFence, requestConfig);
        auto addr = static_cast<uint8_t *>(fbBuffer->GetVirAddr());

        memset_s(addr, fbBuffer->GetSize(), 0, fbBuffer->GetSize());
        surface->FlushBuffer(fbBuffer, -1, flushConfig);
    }
}


static void OnScreenPlug(std::shared_ptr<HdiOutput> &output, bool connected, void* data)
{
    LOG("enter OnScreenPlug, connected is %{public}d", connected);
    std::cout << "OnScreenPlug begin " << std::endl;
    auto* thisPtr = static_cast<HelloComposer *>(data);
    if (connected) {
        std::cout << "OnScreenPlug connetcted " << std::endl;
        thisPtr->CreatePyhsicalScreen(output);
    }
    //std::cout << "OnScreenPlug not connetected " << std::endl;
}


static void OnPrepareCompleted(OHOS::sptr<Surface> &surface, const struct PrepareCompleteParam &param, void* data)
{
    LOG("enter OnPrepareCompleted");
    auto *thisPtr = static_cast<HelloComposer *> (data);
    thisPtr->DoPrepareCompleted(surface, param);
}

int main(int argc, const char *argv[])
{
    HelloComposer m;

    LOG("start to HdiBackend::GetInstance");
    std::cout << "begin " << std::endl;
    HdiBackend* backend = OHOS::Rosen::HdiBackend::GetInstance();
    if (backend == nullptr) {
        LOG("HdiBackend::GetInstance fail");
        return -1;
    }
    std::cout << "RegScreenHotplug " << std::endl;
    backend->RegScreenHotplug(OnScreenPlug, &m);
    std::cout << "RegPrepareComplete " << std::endl;
    backend->RegPrepareComplete(OnPrepareCompleted, &m);

    m.width = 200;
    m.height = 200;
    // sleep(1);
    std::cout << "main EventRunner " << std::endl;
    auto runner = OHOS::AppExecFwk::EventRunner::Create(false);
    auto handler = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    handler->PostTask(std::bind(&HelloComposer::Init, &m, m.width, m.height, backend));
    runner->Run();
    return 0;
}
