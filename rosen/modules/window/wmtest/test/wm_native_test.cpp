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

#include "wm_native_test.h"

#include <cstdio>
#include <unistd.h>
#include <signal.h>
#include "window.h"
#include "window_life_cycle_interface.h"
#include "window_property.h"
#include "window_scene.h"
#include "wm_common.h"

using namespace OHOS::Rosen;

namespace {
WMNativeTest g_autoload;
} // namespace

std::string WMNativeTest::GetDescription() const
{
    constexpr const char *desc = "normal window";
    return desc;
}

std::string WMNativeTest::GetDomain() const
{
    constexpr const char *desc = "wmclient";
    return desc;
}

int32_t WMNativeTest::GetID() const
{
    constexpr int32_t id = 1;
    return id;
}

uint32_t WMNativeTest::GetLastTime() const
{
    constexpr uint32_t lastTime = LAST_TIME_FOREVER;
    return lastTime;
}

void WMNativeTest::Run(int32_t argc, const char **argv)
{
    int displayId = 0;
    sptr<IWindowLifeCycle> listener = nullptr;
    sptr<WindowScene> scene = new WindowScene();
    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext = nullptr;
    WMError rtn = scene->Init(displayId, abilityContext, listener);
    if (rtn != OHOS::Rosen::WMError::WM_OK) {
        return;
    }

    while(true) {
        scene->GoForeground();
        sleep(3);
        scene->GoBackground();
        sleep(3);
    }
}
