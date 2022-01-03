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

#include "window_manager_stub.h"
#include "window_manager_hilog.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "WindowManagerStub"};
}

int32_t WindowManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    int ret = 0;
    WLOGFI("WindowManagerStub::OnRemoteRequest code is %{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("InterfaceToken check failed");
        return -1;
    }
    switch (code) {
        case TRANS_ID_ADD_WINDOW: {
            sptr<IRemoteObject> windowObject = data.ReadRemoteObject();
            sptr<IWindow> windowProxy = iface_cast<IWindow>(windowObject);
            sptr<WindowProperty> windowProperty = data.ReadStrongParcelable<WindowProperty>();
            int32_t renderNodeId = data.ReadInt32();
            WMError errCode = AddWindow(windowProxy, windowProperty, renderNodeId);
            ret = static_cast<int32_t>(errCode);
            reply.WriteInt32(ret);
            break;
        }
        case TRANS_ID_REMOVE_WINDOW: {
            std::string windowId = data.ReadString();
            WMError errCode = RemoveWindow(windowId);
            ret = static_cast<int32_t>(errCode);
            reply.WriteInt32(ret);
            break;
        }
        case TRANS_ID_SET_WINDOW_VISIBILITY: {
            std::string windowId = data.ReadString();
            bool visibility = data.ReadBool();
            WMError errCode = SetWindowVisibility(windowId, visibility);
            ret = static_cast<int32_t>(errCode);
            reply.WriteInt32(ret);
            break;
        }
        default:
            WLOGFW("unknown transaction code");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
            break;
    }
    return ret;
}
}
}
