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

#ifndef INTERFACES_INNERKITS_WINDOW_SCENE_H
#define INTERFACES_INNERKITS_WINDOW_SCENE_H

#include <refbase.h>
#include <iremote_object.h>
#include <../../ability_runtime/include/ability_context.h>

#include "window.h"
#include "window_property.h"
#include "window_life_cycle_interface.h"

namespace OHOS {
namespace Rosen {
class WindowScene : public RefBase {
public:
    static const int32_t DEFAULT_DISPLAY_ID = 0;
    static const std::string MAIN_WINDOW_ID;

    WindowScene() = default;
    ~WindowScene();

    WMError Init(int32_t displayId, std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext,
        sptr<IWindowLifeCycle>& listener);

    sptr<Window> CreateWindow(const std::string& id, sptr<WindowProperty>& property) const;
    WMError Show(sptr<Window>& window);

    const sptr<Window>& GetMainWindow() const;

    WMError GoForeground();
    WMError GoBackground() const;
private:
    sptr<Window> mainWindow_ = nullptr;
    int32_t displayId_ = DEFAULT_DISPLAY_ID;
    volatile bool isMainWindowAdded_ = false;
    sptr<IWindowLifeCycle> listener_ = nullptr;

    std::shared_ptr<AbilityRuntime::AbilityContext> abilityContext_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS
#endif // INTERFACES_INNERKITS_WINDOW_SCENE_H
