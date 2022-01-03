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

#ifndef OHOS_WINDOW_MANAGER_INTERFACE_H
#define OHOS_WINDOW_MANAGER_INTERFACE_H

#include "iremote_broker.h"
#include "window_property.h"
#include "window_interface.h"

namespace OHOS {
namespace Rosen {
class IWindowManager : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IWindowManager");

    enum {
        TRANS_ID_ADD_WINDOW,
        TRANS_ID_REMOVE_WINDOW,
        TRANS_ID_SET_WINDOW_VISIBILITY,
    };

    virtual WMError AddWindow(sptr<IWindow>& window, const sptr<WindowProperty>& windowProperty, int renderNodeId) = 0;
    virtual WMError RemoveWindow(const std::string& windowId) = 0;
    virtual WMError SetWindowVisibility(const std::string& windowId, bool visibility) = 0;
};
}
}
#endif //OHOS_WINDOW_MANAGER_INTERFACE_H
