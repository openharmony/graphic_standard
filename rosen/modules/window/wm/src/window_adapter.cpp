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

#include "window_adapter.h"
#include "iremote_broker.h"
#include "system_ability_definition.h"
#include "iservice_registry.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "WindowAdapter"};
}

IMPLEMENT_SINGLE_INSTANCE(WindowAdapter);

WMError WindowAdapter::AddWindow(sptr<IWindow>& window, const sptr<WindowProperty>& windowProperty, int renderNodeId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!InitWMSProxyLocked()) {
        return WMError::WM_ERROR_SAMGR;
    }
    return windowManagerServiceProxy_->AddWindow(window, windowProperty, renderNodeId);
}

WMError WindowAdapter::RemoveWindow(const std::string& windowId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!InitWMSProxyLocked()) {
        return WMError::WM_ERROR_SAMGR;
    }
    return windowManagerServiceProxy_->RemoveWindow(windowId);
}

WMError WindowAdapter::SetWindowVisibility(const std::string& windowId, bool visibility)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!InitWMSProxyLocked()) {
        return WMError::WM_ERROR_SAMGR;
    }
    return windowManagerServiceProxy_->SetWindowVisibility(windowId, visibility);
}

bool WindowAdapter::InitWMSProxyLocked()
{
    if (!windowManagerServiceProxy_) {
        sptr<ISystemAbilityManager> systemAbilityManager =
                SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (!systemAbilityManager) {
            WLOGFE("Failed to get system ability mgr.");
            return false;
        }

        sptr<IRemoteObject> remoteObject = systemAbilityManager->GetSystemAbility(WINDOW_MANAGER_SERVICE_ID);
        if (!remoteObject) {
            WLOGFE("Failed to get window manager service.");
            return false;
        }

        windowManagerServiceProxy_ = iface_cast<IWindowManager>(remoteObject);
        if ((!windowManagerServiceProxy_) || (!windowManagerServiceProxy_->AsObject())) {
            WLOGFE("Failed to get system window manager services");
            return false;
        }

        wmsDeath_ = new WMSDeathRecipient();
        if (!wmsDeath_) {
            WLOGFE("Failed to create death Recipient ptr WMSDeathRecipient");
            return false;
        }
        if (!remoteObject->AddDeathRecipient(wmsDeath_)) {
            WLOGFE("Failed to add death recipient");
            return false;
        }
    }
    return true;
}

void WindowAdapter::ClearWindowAdapter()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((windowManagerServiceProxy_ != nullptr) && (windowManagerServiceProxy_->AsObject() != nullptr)) {
        windowManagerServiceProxy_->AsObject()->RemoveDeathRecipient(wmsDeath_);
    }
    windowManagerServiceProxy_ = nullptr;
}

void WMSDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
{
    if (wptrDeath == nullptr) {
        WLOGFE("wptrDeath is null");
        return;
    }

    sptr<IRemoteObject> object = wptrDeath.promote();
    if (!object) {
        WLOGFE("object is null");
        return;
    }
    WindowAdapter::GetInstance().ClearWindowAdapter();
    return;
}
} // namespace Rosen
} // namespace OHOS