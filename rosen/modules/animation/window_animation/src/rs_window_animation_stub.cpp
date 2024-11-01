/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "rs_window_animation_stub.h"

#include "rs_iwindow_animation_finished_callback.h"
#include "rs_window_animation_log.h"
#include "rs_window_animation_target.h"

namespace OHOS {
namespace Rosen {
const std::map<uint32_t, WindowAnimationStubFunc> RSWindowAnimationStub::stubFuncMap_{
    std::make_pair(RSIWindowAnimationController::ON_START_APP, &RSWindowAnimationStub::StartApp),
    std::make_pair(RSIWindowAnimationController::ON_APP_TRANSITION, &RSWindowAnimationStub::AppTransition),
    std::make_pair(RSIWindowAnimationController::ON_MINIMIZE_WINDOW, &RSWindowAnimationStub::MinimizeWindow),
    std::make_pair(RSIWindowAnimationController::ON_CLOSE_WINDOW, &RSWindowAnimationStub::CloseWindow),
    std::make_pair(RSIWindowAnimationController::ON_SCREEN_UNLOCK, &RSWindowAnimationStub::ScreenUnlock),
};

int RSWindowAnimationStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption &option)
{
    WALOGD("Window animation on remote request!");
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        WALOGE("Failed to check interface token!");
        return ERR_INVALID_STATE;
    }

    const auto func = stubFuncMap_.find(code);
    if (func == stubFuncMap_.end()) {
        WALOGE("Failed to find function handler!");
        return ERR_UNKNOWN_TRANSACTION;
    }

    return (this->*(func->second))(data, reply);
}

int RSWindowAnimationStub::StartApp(MessageParcel& data, MessageParcel& reply)
{
    WALOGD("Window animation start app!");
    StartingAppType type = static_cast<StartingAppType>(data.ReadInt32());
    sptr<RSWindowAnimationTarget> startingWindowTarget(data.ReadParcelable<RSWindowAnimationTarget>());
    if (startingWindowTarget == nullptr) {
        WALOGE("Failed to read starting window target!");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> finishcallbackObject = data.ReadRemoteObject();
    sptr<RSIWindowAnimationFinishedCallback> finishedCallback =
        iface_cast<RSIWindowAnimationFinishedCallback>(finishcallbackObject);
    if (finishedCallback == nullptr) {
        WALOGE("Failed to read animation finished callback!");
        return ERR_INVALID_DATA;
    }

    OnStartApp(type, startingWindowTarget, finishedCallback);
    return ERR_NONE;
}

int RSWindowAnimationStub::AppTransition(MessageParcel& data, MessageParcel& reply)
{
    WALOGD("Window animation transition!");
    sptr<RSWindowAnimationTarget> fromWindowTarget(data.ReadParcelable<RSWindowAnimationTarget>());
    if (fromWindowTarget == nullptr) {
        WALOGE("Failed to read animation target from!");
        return ERR_INVALID_DATA;
    }

    sptr<RSWindowAnimationTarget> toWindowTarget(data.ReadParcelable<RSWindowAnimationTarget>());
    if (toWindowTarget == nullptr) {
        WALOGE("Failed to read animation target to!");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> finishcallbackObject = data.ReadRemoteObject();
    sptr<RSIWindowAnimationFinishedCallback> finishedCallback =
        iface_cast<RSIWindowAnimationFinishedCallback>(finishcallbackObject);
    if (finishedCallback == nullptr) {
        WALOGE("Failed to read animation finished callback!");
        return ERR_INVALID_DATA;
    }

    OnAppTransition(fromWindowTarget, toWindowTarget, finishedCallback);
    return ERR_NONE;
}

int RSWindowAnimationStub::MinimizeWindow(MessageParcel& data, MessageParcel& reply)
{
    WALOGD("Window animation minimize window!");
    sptr<RSWindowAnimationTarget> minimizingWindow(data.ReadParcelable<RSWindowAnimationTarget>());
    if (minimizingWindow == nullptr) {
        WALOGE("Failed to read minimizing window!");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> finishcallbackObject = data.ReadRemoteObject();
    sptr<RSIWindowAnimationFinishedCallback> finishedCallback =
        iface_cast<RSIWindowAnimationFinishedCallback>(finishcallbackObject);
    if (finishedCallback == nullptr) {
        WALOGE("Failed to read animation finished callback!");
        return ERR_INVALID_DATA;
    }

    OnMinimizeWindow(minimizingWindow, finishedCallback);
    return ERR_NONE;
}

int RSWindowAnimationStub::CloseWindow(MessageParcel& data, MessageParcel& reply)
{
    WALOGD("Window animation close window!");
    sptr<RSWindowAnimationTarget> closingWindow(data.ReadParcelable<RSWindowAnimationTarget>());
    if (closingWindow == nullptr) {
        WALOGE("Failed to read closing window!");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> finishcallbackObject = data.ReadRemoteObject();
    sptr<RSIWindowAnimationFinishedCallback> finishedCallback =
        iface_cast<RSIWindowAnimationFinishedCallback>(finishcallbackObject);
    if (finishedCallback == nullptr) {
        WALOGE("Failed to read animation finished callback!");
        return ERR_INVALID_DATA;
    }

    OnCloseWindow(closingWindow, finishedCallback);
    return ERR_NONE;
}

int RSWindowAnimationStub::ScreenUnlock(MessageParcel& data, MessageParcel& reply)
{
    WALOGD("Window animation screen unlock!");
    sptr<IRemoteObject> finishcallbackObject = data.ReadRemoteObject();
    sptr<RSIWindowAnimationFinishedCallback> finishedCallback =
        iface_cast<RSIWindowAnimationFinishedCallback>(finishcallbackObject);
    if (finishedCallback == nullptr) {
        WALOGE("Failed to read animation finished callback!");
        return ERR_INVALID_DATA;
    }

    OnScreenUnlock(finishedCallback);
    return ERR_NONE;
}
} // namespace Rosen
} // namespace OHOS
