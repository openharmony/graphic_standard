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

#ifndef OHOS_WINDOW_MANAGER_SERVICE_H
#define OHOS_WINDOW_MANAGER_SERVICE_H

#include <vector>
#include <map>

#include "window_manager_stub.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "nocopyable.h"
#include "single_instance.h"
#include "system_ability.h"
#include "window_state.h"

namespace OHOS {
namespace Rosen {
class WindowNode : public RefBase {
public:
    WindowNode() = default;
    ~WindowNode() = default;

    std::string windowId_ = "";
    sptr<WindowNode> parent_ = nullptr;
    std::vector<sptr<WindowNode>> children_;
};

class WindowDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    WindowDeathRecipient() = default;
    ~WindowDeathRecipient() = default;

    virtual void OnRemoteDied(const wptr<IRemoteObject>& wptrDeath) override;
};

class WindowManagerService : public SystemAbility, public WindowManagerStub {
DECLARE_SYSTEM_ABILITY(WindowManagerService);

DECLARE_SINGLE_INSTANCE_BASE(WindowManagerService);

public:
    ~WindowManagerService() = default;
    void OnStart() override;
    void OnStop() override;

    WMError AddWindow(sptr<IWindow>& window, const sptr<WindowProperty>& windowProperty, int renderNodeId) override;
    WMError RemoveWindow(const std::string& windowId) override;
    WMError SetWindowVisibility(const std::string& windowId, bool visibility) override;

    void ClearWindow(sptr<IRemoteObject>& remoteObject);

private:
    WindowManagerService();
    bool Init();
    void GetNode(const std::string& windowId, const sptr<WindowNode>& root, sptr<WindowNode>& node) const;
    sptr<WindowState> GetWindowState(const std::string& windowId) const;
    WMError RemoveWindowInner(const std::string& windowId);

    sptr<WindowNode> root_ = new WindowNode();
    std::map<std::string, sptr<WindowState>> windowStateMap_;
    std::map<sptr<IRemoteObject>, std::string> windowIdMap_;
    sptr<WindowDeathRecipient> windowDeath_ = nullptr;
};
}
}
#endif //OHOS_WINDOW_MANAGER_SERVICE_H
