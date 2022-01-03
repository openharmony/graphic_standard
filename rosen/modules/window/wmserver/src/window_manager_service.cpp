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

#include "window_manager_service.h"
#include "window_manager_hilog.h"

#include <cinttypes>
#include <unistd.h>

#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
//#include "sys_mgr_client.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "WindowManagerService"};

}
IMPLEMENT_SINGLE_INSTANCE(WindowManagerService);

const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(&WindowManagerService::GetInstance());

WindowManagerService::WindowManagerService() : SystemAbility(WINDOW_MANAGER_SERVICE_ID, true)
{
}

void WindowManagerService::OnStart()
{
    WLOGFI("WindowManagerService::OnStart start");
    if (!Init()) {
        return;
    }
}

bool WindowManagerService::Init()
{
    WLOGFI("WindowManagerService::Init start");
    bool ret = Publish(this);
    if (!ret) {
        WLOGFW("WindowManagerService::Init failed");
        return false;
    }
    WLOGFI("WindowManagerService::Init success");
    return true;
}

void WindowManagerService::OnStop()
{
    WLOGFI("ready to stop service.");
}

WMError WindowManagerService::AddWindow(sptr<IWindow>& window, const sptr<WindowProperty>& property, int renderNodeId)
{
    if (window == nullptr || property == nullptr) {
        WLOGFE("window is invalid");
        return WMError::WM_ERROR_NULLPTR;
    }
    std::string windowId = property->GetWindowId();
    std::string parentId = property->GetParentId();

    // build window tree
    sptr<WindowNode> node = new WindowNode();
    node->windowId_ = windowId;
    sptr<WindowNode> parent;
    GetNode(parentId, root_, parent);
    if (parent == nullptr) {
        WLOGFE("failed to add parent");
        return WMError::WM_ERROR_NULLPTR;
    }
    if (parent->parent_ != nullptr && parent->parent_->parent_ == root_) { // forbid subwindow has child
        return WMError::WM_ERROR_INVALID_PARAM;
    }
    node->parent_ = parent;
    parent->children_.push_back(node);

    sptr<WindowState> windowState = new WindowState();
    windowState->SetWindowToken(window);
    windowStateMap_.insert({windowId, windowState});
    windowIdMap_.insert({window->AsObject(), windowId});

    WMError res = SetWindowVisibility(windowId, true);
    if (res != WMError::WM_OK) {
        WLOGFE("failed to show window");
        RemoveWindow(windowId);
        return res;
    }

    // TODO: check property and determine property

    windowDeath_ = new WindowDeathRecipient();
    if (windowDeath_ == nullptr) {
        WLOGFE("failed to create death Recipient ptr WindowDeathRecipient");
        return WMError::WM_ERROR_DEATH_RECIPIENT;
    }
    if (!window->AsObject()->AddDeathRecipient(windowDeath_)) {
        WLOGFE("failed to add death recipient");
        return WMError::WM_ERROR_DEATH_RECIPIENT;
    }
    return WMError::WM_OK;
}

WMError WindowManagerService::RemoveWindow(const std::string& windowId)
{
    sptr<WindowNode> node;
    GetNode(windowId, root_, node);
    if (node == nullptr) {
        WLOGFE("window does not be added at window tree");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (node->parent_ == nullptr) {
        WLOGFE("window does not be added at root node");
        return WMError::WM_ERROR_NULLPTR;
    }

    WMError res = WMError::WM_OK;
    if (node->parent_ == root_) { // main window
        for (auto& child : node->children_) { // remove sub window
            res = RemoveWindowInner(child->windowId_);
        }
    }
    for (auto iter = node->parent_->children_.begin(); iter != node->parent_->children_.end(); ++iter) {
        if ((*iter)->windowId_ == windowId) { // remove itself
            node->parent_->children_.erase(iter);
            res = RemoveWindowInner(windowId);
            break;
        }
    }
    return res;
}

WMError WindowManagerService::RemoveWindowInner(const std::string& windowId)
{
    auto iter = windowStateMap_.find(windowId);
    if (iter == windowStateMap_.end()) {
        WLOGFE("window state could not be found");
        return WMError::WM_ERROR_DESTROYED_OBJECT;
    }
    sptr<IWindow> window = iter->second->GetWindowToken();
    if (windowIdMap_.count(window->AsObject()) == 0) {
        WLOGFE("window id could not be found");
        return WMError::WM_ERROR_DESTROYED_OBJECT;
    }

    if (window->AsObject() != nullptr) {
        window->AsObject()->RemoveDeathRecipient(windowDeath_);
    }
    windowIdMap_.erase(window->AsObject());
    windowStateMap_.erase(windowId);
    return WMError::WM_OK;
}

WMError WindowManagerService::SetWindowVisibility(const std::string& windowId, bool visibility)
{
    sptr<WindowNode> node;
    GetNode(windowId, root_, node);
    if (node == nullptr) {
        WLOGFE("window does not be added at window tree");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (node->parent_ == nullptr) {
        WLOGFE("window does not be added at root node");
        return WMError::WM_ERROR_NULLPTR;
    }

    if (node->parent_ == root_) { // main window
        sptr<WindowState> windowState = GetWindowState(windowId);
        if (windowState == nullptr) {
            WLOGFE("main window does not be found");
            return WMError::WM_ERROR_DESTROYED_OBJECT;
        }
        windowState->SetWindowVisibility(visibility);
        windowState->SetLayerVisibility(visibility);

        for (auto& child : node->children_) {
            windowState = GetWindowState(child->windowId_);
            if (windowState == nullptr) {
                WLOGFE("main window's child does not be found");
                continue;
            }
            windowState->SetLayerVisibility(visibility && windowState->GetWindowVisibility());
        }
    } else { // sub window
        sptr<WindowState> windowState = GetWindowState(node->parent_->windowId_);
        if (windowState == nullptr) {
            WLOGFE("sub window's parent window does not be found");
            return WMError::WM_ERROR_DESTROYED_OBJECT;
        }
        bool parentVisibility = windowState->GetWindowVisibility();

        windowState = GetWindowState(windowId);
        if (windowState == nullptr) {
            WLOGFE("sub window does not be found");
            return WMError::WM_ERROR_DESTROYED_OBJECT;
        }
        windowState->SetWindowVisibility(visibility);
        windowState->SetLayerVisibility(visibility && parentVisibility);
    }
    return WMError::WM_OK;
}

void WindowManagerService::GetNode(const std::string& windowId, const sptr<WindowNode>& root,
    sptr<WindowNode>& node) const
{
    if (root == nullptr) {
        return;
    }

    if (root->windowId_ == windowId) {
        node = root;
        return;
    }

    for (auto& child : root->children_) {
        GetNode(windowId, child, node);
    }
}

sptr<WindowState> WindowManagerService::GetWindowState(const std::string& windowId) const
{
    auto iter = windowStateMap_.find(windowId);
    if (iter == windowStateMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

void WindowManagerService::ClearWindow(sptr<IRemoteObject>& remoteObject)
{
    auto iter = windowIdMap_.find(remoteObject);
    if (iter == windowIdMap_.end()) {
        WLOGFE("window id could not be found");
        return;
    }
    std::string windowId = iter->second;
    RemoveWindow(windowId);
}

void WindowDeathRecipient::OnRemoteDied(const wptr<IRemoteObject>& wptrDeath)
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
    WindowManagerService::GetInstance().ClearWindow(object);
    return;
}
}
}