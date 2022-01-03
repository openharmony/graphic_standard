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

#ifndef RS_SCREEN_CHANGE_CALLBACK_DEATH_RECIPIENT_H
#define RS_SCREEN_CHANGE_CALLBACK_DEATH_RECIPIENT_H

#include <iremote_object.h>

#include "rs_screen_manager.h"

namespace OHOS {
namespace Rosen {
class RSScreenChangeCallbackDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    RSScreenChangeCallbackDeathRecipient(sptr<RSScreenManager> screenManager);
    virtual ~RSScreenChangeCallbackDeathRecipient() = default;

    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

private:
    sptr<RSScreenManager> screenManager_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS

#endif // RS_SCREEN_CHANGE_CALLBACK_DEATH_RECIPIENT_H
