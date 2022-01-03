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

#include "adapter.h"
#include "window_impl.h"

namespace OHOS {
namespace Rosen {

static inline sptr<OHOS::Window> westonMainWindow_ = nullptr;

Adapter::Adapter()
{
}

Adapter::~Adapter()
{
}

OHOS::Rosen::WMError Adapter::Init()
{
    // "AbilityWindow::Init()"
    auto wmi = WindowManager::GetInstance();
    if (wmi == nullptr) {
        WLOGFE("Adapter::Init WindowManager::GetInstance() is nullptr.");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_GETINSTANCE;
    }

    auto wret = wmi->Init();
    if (wret != OHOS::WM_OK) {
        WLOGFE("Adapter::Init WindowManager::Init() return %d", wret);
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_INIT;
    }
    WLOGFI("%{public}s end.", __func__);
    return OHOS::Rosen::WMError::WM_OK;
}

bool Adapter::CreateWestonWindow(sptr<OHOS::Rosen::WindowProperty>& property)
{
    sptr<OHOS::WindowOption> config = OHOS::WindowOption::Get();
    if (!WindowOptionConvertToWeston(property, config)) {
        WLOGFE("Adapter::CreateWindow convert windowOption fail");
        return false;
    }

    auto wmi = OHOS::WindowManager::GetInstance();
    if (wmi == nullptr) {
        WLOGFE("Adapter::ConvertToWindowOption WindowManager::GetInstance() is nullptr.");
        return false;
    }

    WLOGFI("%{public}s begin wms->CreateWindow.", __func__);
    auto retvalCreate = wmi->CreateWindow(westonMainWindow_, config);
    WLOGFI("%{public}s end wms->CreateWindow.", __func__);

    WriteDataToSurface();

    if (retvalCreate != OHOS::WMError::WM_OK) {
        WLOGFE("Adapter::ConvertToWindowOption WindowManager::CreateWindow() return %d", retvalCreate);
        return false;
    }
    return true;
}

void Adapter::DestroyWestonWindow()
{
    if (westonMainWindow_ != nullptr) {
        OHOS::WMError err = westonMainWindow_->Destroy();
        westonMainWindow_ = nullptr;
        if (err == OHOS::WMError::WM_OK) {
            WLOGFI("DestroyWestonWindow OK");
            return;
        }
        WLOGFE("DestroyWestonWindow errorcode=%d", static_cast<int>(err));
    }
    WLOGFE("DestroyWestonWindow westonMainWindow_ is null");
}

bool Adapter::WindowOptionConvertToWeston(sptr<OHOS::Rosen::WindowProperty>& property, sptr<OHOS::WindowOption>& config)
{
    // window type
    config->SetWindowType(OHOS::WindowType::WINDOW_TYPE_NORMAL);
    config->SetWindowMode(OHOS::WindowMode::WINDOW_MODE_FULL);
    return true;
}

bool Adapter::GetMainWindowRect(OHOS::Rosen::Rect &rect)
{
    if (westonMainWindow_ == nullptr) {
        return false;
    }
    rect.posX_ = westonMainWindow_->GetX();
    rect.posY_ = westonMainWindow_->GetY();
    rect.width_ = westonMainWindow_->GetWidth();
    rect.height_ = westonMainWindow_->GetHeight();
    return true;
}

static inline bool hide = false;

OHOS::Rosen::WMError Adapter::Show(const std::string& wid)
{
    if (westonMainWindow_ == nullptr) {
        WLOGFE("Adapter::Show westonMainWindow_ is null");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_SHOW;
    }

    if (!hide) {
        auto promise = westonMainWindow_->Show();
        if (promise == nullptr) {
            WLOGFE("Adapter::Show Show error");
            return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_SHOW;
        }
        auto error = promise->Await();
        if (error != OHOS::WMError::WM_OK) {
            WLOGFE("Adapter::Show Show error=%d", static_cast<int>(error));
            return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_SHOW;
        }
    }

    auto promise2 = westonMainWindow_->SwitchTop();
    if (promise2 == nullptr) {
        WLOGFE("Adapter::Show SwitchTop error");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_SHOW;
    }
    auto error2 = promise2->Await();
    if (error2 != OHOS::WMError::WM_OK) {
        WLOGFE("Adapter::Show SwitchTop error=%d", static_cast<int>(error2));
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_SHOW;
    }

    return OHOS::Rosen::WMError::WM_OK;
}

void Adapter::WriteDataToSurface()
{
    /* write data to surface */
    sptr<Surface> surface = westonMainWindow_->GetSurface();
    sptr<SurfaceBuffer> buffer;
    BufferRequestConfig rconfig;
    if (GetSurfaceBuffer(surface, buffer, rconfig)) {
        BlackDraw(buffer->GetVirAddr(), rconfig.width, rconfig.height, 1);
    } else {
        WLOGFE("Adapter::WriteDataToSurface GetSurfaceBuffer fail");
    }
    BufferFlushConfig fconfig = {
        .damage = {
            .w = rconfig.width,
            .h = rconfig.height,
        },
    };
    surface->FlushBuffer(buffer, -1, fconfig);
}

OHOS::Rosen::WMError Adapter::Hide(const std::string& wid)
{
    if (westonMainWindow_ == nullptr) {
        WLOGFE("Adapter::Hide westonMainWindow_ is null");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_HIDE;
    }
    auto promise = westonMainWindow_->Hide();
    if (promise == nullptr) {
        WLOGFE("Adapter::Hide Hide error");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_HIDE;
    }
    auto error = promise->Await();
    if (error != OHOS::WMError::WM_OK) {
        WLOGFE("Adapter::Hide Hide error=%d", static_cast<int>(error));
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_HIDE;
    }
    hide = true;
    return OHOS::Rosen::WMError::WM_OK;
}

OHOS::Rosen::WMError Adapter::MoveTo(const std::string& wid, int32_t x, int32_t y)
{
    if (westonMainWindow_ == nullptr) {
        WLOGFE("Adapter::MoveTo westonMainWindow_ is null");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_MOVE;
    }
    auto promise = westonMainWindow_->Move(x, y);
    if (promise == nullptr) {
        WLOGFE("Adapter::MoveTo promise is null");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_MOVE;
    }
    return OHOS::Rosen::WMError::WM_OK;
}

OHOS::Rosen::WMError Adapter::Resize(const std::string& wid, uint32_t width, uint32_t height)
{
    if (westonMainWindow_ == nullptr) {
        WLOGFE("Adapter::Resize westonMainWindow_ is null");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_RESIZE;
    }
    auto promise = westonMainWindow_->Resize(width, height);
    if (promise == nullptr) {
        WLOGFE("Adapter::Resize promise is null");
        return OHOS::Rosen::WMError::WM_ERROR_WINDOWMANAGER_RESIZE;
    }
    return OHOS::Rosen::WMError::WM_OK;
}

bool Adapter::GetSurfaceBuffer(sptr<Surface> &surface, sptr<SurfaceBuffer> &buffer, BufferRequestConfig &rconfig)
{
    int32_t releaseFence;
    rconfig = {
            .width = surface->GetDefaultWidth(),
            .height = surface->GetDefaultHeight(),
            .strideAlignment = 0x8,
            .format = 12, // PIXEL_FMT_RGBA_8888,
            .usage = surface->GetDefaultUsage(),
            .timeout = 0,
    };
    OHOS::SurfaceError ret = surface->RequestBuffer(buffer, releaseFence, rconfig);
    if (ret == 40601000) { // OHOS::SurfaceError::SURFACE_ERROR_NO_BUFFER
        WLOGFE("Adapter::GetSurfaceBuffer RequestBuffer error=%d", static_cast<int>(ret));
        return false;
    } else if (ret != 0) { // OHOS::SurfaceError::SURFACE_ERROR_OK
        WLOGFE("Adapter::GetSurfaceBuffer RequestBuffer is NG error=%d", static_cast<int>(ret));
        return false;
    } else if (buffer == nullptr) {
        WLOGFE("Adapter::GetSurfaceBuffer buffer is nullptr");
        return false;
    }
    return true;
}

void Adapter::ColorDraw(void *vaddr, uint32_t width, uint32_t height, uint32_t count)
{
    auto addr = static_cast<uint32_t *>(vaddr);
    if (addr == nullptr) {
        return;
    }

    constexpr uint32_t wdiv = 2;
    constexpr uint32_t colorTable[][wdiv] = {
        {0xffff0000, 0xffff00ff},
        {0xffff0000, 0xffffff00},
        {0xff00ff00, 0xffffff00},
        {0xff00ff00, 0xff00ffff},
        {0xff0000ff, 0xff00ffff},
        {0xff0000ff, 0xffff00ff},
        {0xff777777, 0xff777777},
        {0xff777777, 0xff777777},
    };
    const uint32_t hdiv = sizeof(colorTable) / sizeof(*colorTable);

    for (uint32_t i = 0; i < height; i++) {
        auto table = colorTable[i / (height / hdiv)];
        for (uint32_t j = 0; j < wdiv; j++) {
            auto color = table[j];
            for (uint32_t k = j * width / wdiv; k < (j + 1) * width / wdiv; k++) {
                addr[i * width + k] = color;
            }
        }
    }
}

void Adapter::BlackDraw(void *vaddr, uint32_t width, uint32_t height, uint32_t count)
{
    auto addr = static_cast<uint32_t *>(vaddr);
    if (addr == nullptr) {
        return;
    }

    for (uint32_t i = 0; i < width * height; i++) {
        addr[i] = 0xff00ff00;
    }
}

void Adapter::FlushDraw(void *vaddr, uint32_t width, uint32_t height, uint32_t count)
{
    auto addr = static_cast<uint8_t *>(vaddr);
    if (addr == nullptr) {
        return;
    }

    constexpr uint32_t bpp = 4;
    constexpr uint32_t color1 = 0xff / 3 * 0;
    constexpr uint32_t color2 = 0xff / 3 * 1;
    constexpr uint32_t color3 = 0xff / 3 * 2;
    constexpr uint32_t color4 = 0xff / 3 * 3;
    constexpr uint32_t bigDiv = 7;
    constexpr uint32_t smallDiv = 10;
    uint32_t c = count % (bigDiv * smallDiv);
    uint32_t stride = width * bpp;
    uint32_t beforeCount = height * c / bigDiv / smallDiv;
    uint32_t afterCount = height - beforeCount - 1;

    size_t len = ((stride * height) < (beforeCount * stride) ? (stride * height) : (beforeCount * stride));
    set(addr, color3, len);

    len = ((stride * height) < (afterCount * stride) ? (stride * height) : (afterCount * stride));
    set(addr + (beforeCount + 1) * stride, color1, len);

    for (uint32_t i = 0; i < bigDiv; i++) {
        len = ((stride * height) < (stride) ? (stride * height) : (stride));
        set(addr + (i * height / bigDiv) * stride, color4, len);
    }

    len = ((stride * height) < (stride) ? (stride * height) : (stride));
    set(addr + beforeCount * stride, color2, len);
}

void Adapter::set(void *vaddr, char ch, size_t len)
{
    char *ptr = static_cast<char *>(vaddr);
    for (size_t i = 0; i < len; i++) {
        ptr[i] = ch;
    }
}
}
}
