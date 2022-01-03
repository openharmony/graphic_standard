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

#include "window_stub.h"
#include "ipc_skeleton.h"
#include "window_manager_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "WindowStub"};
}

int WindowStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    int ret = 0;
    WLOGFI("WindowStub::OnRemoteRequest code is %{public}d", code);
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WLOGFE("InterfaceToken check failed");
        return -1;
    }
    switch (code) {
        case TRANS_ID_UPDATE_WINDOW_PROPERTY: {
            break;
        }
        case TRANS_ID_UPDATE_WINDOW_RECT: {
            struct Rect rect{ data.ReadInt32(), data.ReadInt32(), data.ReadUint32(), data.ReadUint32() };
            UpdateWindowRect(rect);
            break;
        }
        case TRANS_ID_UPDATE_WINDOW_MODE: {
            WindowMode mode = static_cast<WindowMode>(data.ReadUint32());
            UpdateWindowMode(mode);
            break;
        }
        default:
            break;
    }
    return ret;
}
} // namespace Rosen
} // namespace OHOS
