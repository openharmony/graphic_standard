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

#include "window_impl.h"
#include "window_adapter.h"
#include "window_agent.h"

#ifndef _NEW_RENDERSERVER_
#include "adapter.h"
#endif

namespace OHOS {
namespace Rosen {
std::map<std::string, sptr<Window>> WindowImpl::windowMap_;

// virtual sptr<Surface> WindowImpl::GetSurface() const
//{
//    return surface_;
//}

WindowImpl::WindowImpl(const sptr<WindowProperty>& property)
{
    property_ = property;
}

WindowImpl::~WindowImpl()
{
//    surface_ = nullptr;
    Destroy();
}

sptr<Window> WindowImpl::Find(const std::string& id)
{
    auto iter = windowMap_.find(ConvertId(id));
    if (iter == windowMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

WMError WindowImpl::SetWindowVisibility(const std::string& id, bool isVisible)
{
    return WindowAdapter::GetInstance().SetWindowVisibility(ConvertId(id), isVisible);
}

const std::string& WindowImpl::ConvertId(const std::string& id)
{
    return id;
}

bool WindowImpl::GetVisibility() const
{
    return visibility_;
}

Rect WindowImpl::GetRect() const
{
    return property_->GetWindowRect();
}

WindowType WindowImpl::GetType() const
{
    return property_->GetWindowType();
}

WindowMode WindowImpl::GetMode() const
{
    return property_->GetWindowMode();
}

const std::string& WindowImpl::GetId() const
{
    return property_->GetWindowId();
}

sptr<WindowProperty> WindowImpl::GetProperty() const
{
    return property_;
}

WMError WindowImpl::SetVisibility(bool visibility)
{
    if (!isAdded_) {
        visibility_ = visibility;
        return WMError::WM_OK;
    }
    if (visibility_ != visibility) {
        // TODO
    }
    return WMError::WM_OK;
}

WMError WindowImpl::SetWindowType(WindowType type)
{
    if (!isAdded_) {
        property_->SetWindowType(type);
        return WMError::WM_OK;
    }
    if (property_->GetWindowType() != type) {
        // TODO
    }
    return WMError::WM_OK;
}

WMError WindowImpl::SetWindowMode(WindowMode mode)
{
    if (!isAdded_) {
        property_->SetWindowMode(mode);
        return WMError::WM_OK;
    }
    if (property_->GetWindowMode() != mode) {
        // TODO
    }
    return WMError::WM_OK;
}

WMError WindowImpl::Destroy()
{
#ifdef _NEW_RENDERSERVER_
    // TODO surface
    if (isAdded_) {
        std::string convertedId = ConvertId(GetId());
        if (windowMap_.count(convertedId) == 0) {
            return WMError::WM_ERROR_DESTROYED_OBJECT;
        }
        WMError ret = WindowAdapter::GetInstance().RemoveWindow(convertedId);
        windowMap_.erase(convertedId);
        isAdded_ = false;
        return ret;
    }
#else
    Adapter::DestroyWestonWindow();
#endif
    return WMError::WM_OK;
}

WMError WindowImpl::Show()
{
#ifdef _NEW_RENDERSERVER_
    WMError ret;
    if (isAdded_) {
        ret = SetWindowVisibility(GetId(), true);
    } else {
        sptr<WindowImpl> window(this);
        sptr<IWindow> windowAgent(new WindowAgent(window));
        ret = WindowAdapter::GetInstance().AddWindow(windowAgent, property_, renderNodeId_);
    }
    if (ret == WMError::WM_OK || ret == WMError::WM_ERROR_DEATH_RECIPIENT) {
        if (!isAdded_) {
            isAdded_ = true;
            windowMap_.insert({ ConvertId(GetId()), this });
        }
        if (lifecycleListener_ != nullptr) {
            lifecycleListener_->AfterForeground();
        }
    }
    return ret;
#else
    /* weston adapter */
    WMError rtn = Adapter::Show(GetId());
    if (rtn == WMError::WM_OK) {
        if (lifecycleListener_ != nullptr) {
            lifecycleListener_->AfterForeground();
            lifecycleListener_->AfterFocused();
            WLOGFI("WindowImpl::Show AfterForeground was ivoked");
        }
    } else {
        WLOGFE("WindowImpl::Show error=%d", static_cast<int>(rtn));
    }
    return rtn;
#endif
}

WMError WindowImpl::Hide()
{
#ifdef _NEW_RENDERSERVER_
    WMError ret = SetWindowVisibility(GetId(), false);
    if (ret == WMError::WM_OK && lifecycleListener_ != nullptr) {
        lifecycleListener_->AfterBackground();
    }
    return ret;
#else
    /* weston adapter */
    WMError rtn = Adapter::Hide(GetId());
    if (rtn == WMError::WM_OK) {
        if (lifecycleListener_ != nullptr) {
            lifecycleListener_->AfterUnFocused();
            lifecycleListener_->AfterBackground();
            WLOGFI("WindowImpl::Show AfterBackground was ivoked");
        }
    } else {
        WLOGFE("WindowImpl::Show error=%d", static_cast<int>(rtn));
    }
    return rtn;
#endif
}

WMError WindowImpl::MoveTo(int32_t x, int32_t y)
{
    /* weston adapter */
    return Adapter::MoveTo(GetId(), x, y);
}

WMError WindowImpl::Resize(uint32_t width, uint32_t height)
{
    /* weston adapter */
    return Adapter::Resize(GetId(), width, height);
}

void WindowImpl::RegisterLifeCycleListener(sptr<IWindowLifeCycle>& listener)
{
    lifecycleListener_ = listener;
}

void WindowImpl::UpdateRect(const struct Rect& rect)
{
    property_->SetWindowRect(rect);
}

void WindowImpl::UpdateMode(WindowMode mode)
{
    property_->SetWindowMode(mode);
}
}
}