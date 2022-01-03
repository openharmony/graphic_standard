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

#include "window_manager_proxy.h"
#include "ipc_types.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "WindowManagerProxy"};
}

WMError WindowManagerProxy::AddWindow(sptr<IWindow>& window, const sptr<WindowProperty>& property, int renderNodeId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteRemoteObject(window->AsObject())) {
        WLOGFE("Write IWindow failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteParcelable(property.GetRefPtr())) {
        WLOGFE("Write windowProperty failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteInt32(renderNodeId)) {
        WLOGFE("Write renderNodeId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!Remote()->SendRequest(TRANS_ID_ADD_WINDOW, data, reply, option)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::RemoveWindow(const std::string& windowId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteString(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!Remote()->SendRequest(TRANS_ID_REMOVE_WINDOW, data, reply, option)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}

WMError WindowManagerProxy::SetWindowVisibility(const std::string& windowId, bool visibility)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        WLOGFE("WriteInterfaceToken failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteString(windowId)) {
        WLOGFE("Write windowId failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!data.WriteBool(visibility)) {
        WLOGFE("Write visibility failed");
        return WMError::WM_ERROR_IPC_FAILED;
    }

    if (!Remote()->SendRequest(TRANS_ID_SET_WINDOW_VISIBILITY, data, reply, option)) {
        return WMError::WM_ERROR_IPC_FAILED;
    }

    int32_t ret = reply.ReadInt32();
    return static_cast<WMError>(ret);
}
}
}

