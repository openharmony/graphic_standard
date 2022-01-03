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

#include "window_scene.h"
#include "window_impl.h"
#ifndef _NEW_RENDERSERVER_
#include "adapter.h"
#endif

namespace OHOS {
namespace Rosen {
const std::string WindowScene::MAIN_WINDOW_ID = "main window";

WindowScene::~WindowScene()
{
    WLOGFI("~WindowScene");
    if (listener_ != nullptr) {
        listener_->BeforeDestroy();
    }
    if (mainWindow_ != nullptr) {
        mainWindow_->Destroy();
        mainWindow_ = nullptr;
    }
}

WMError WindowScene::Init(int32_t displayId, std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext,
    sptr<IWindowLifeCycle>& listener)
{
    displayId_ = displayId;
    abilityContext_ = abilityContext;
    sptr<WindowProperty> property = new WindowProperty();
    property->SetDisplayId(displayId);
    property->SetWindowId(MAIN_WINDOW_ID);
    property->SetDisplayId(displayId);

#ifndef _NEW_RENDERSERVER_
    /* weston adapter */
    Adapter::Init();
#endif

    mainWindow_ = CreateWindow(MAIN_WINDOW_ID, property);
    if (mainWindow_ == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    mainWindow_->RegisterLifeCycleListener(listener);
    listener_ = listener;
    if (listener != nullptr) {
        listener_->AfterCreated();
    }
    return WMError::WM_OK;
}

sptr<Window> WindowScene::CreateWindow(const std::string& windowId, sptr<WindowProperty>& property) const
{
#ifdef _NEW_RENDERSERVER_
    return Window::Create(windowId, property);
#else
    /* weston adapter */
    if (!Adapter::CreateWestonWindow(property)) {
        WLOGFE("WindowScene::CreateWindow fail to CreateWestonWindow");
        return nullptr;
    }
    Rect rect;
    if (!Adapter::GetMainWindowRect(rect)) {
        WLOGFE("WindowScene::CreateWindow fail to GetMainWindowRect");
        return nullptr;
    }
    property->SetParentId("");
    property->SetWindowId(windowId);
    property->SetWindowRect(rect);
    sptr<Window> window = new WindowImpl(property);
    return window;
#endif
}

WMError WindowScene::Show(sptr<Window>& window)
{
    if (window == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    return window->Show();
}

const sptr<Window>& WindowScene::GetMainWindow() const
{
    return mainWindow_;
}

WMError WindowScene::GoForeground()
{
    if (mainWindow_ == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    if (isMainWindowAdded_) {
        return WindowImpl::SetWindowVisibility(mainWindow_->GetProperty()->GetWindowId(), true);
    }
    WMError res = mainWindow_->Show();
    if (res == WMError::WM_OK) {
        isMainWindowAdded_ = true;
    }
    return res;
}

WMError WindowScene::GoBackground() const
{
    if (mainWindow_ == nullptr) {
        return WMError::WM_ERROR_NULLPTR;
    }
    return mainWindow_->Hide();
}
} // namespace Rosen
} // namespace OHOS
