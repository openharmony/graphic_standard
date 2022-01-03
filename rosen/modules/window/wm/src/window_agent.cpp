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

#include "window_agent.h"
#include "window_manager_hilog.h"

namespace OHOS {
namespace Rosen {
namespace {
    constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0, "WindowAgent"};
}

WindowAgent::WindowAgent(sptr<WindowImpl>& windowImpl)
{
    window_ = windowImpl;
}

void WindowAgent::UpdateWindowProperty(const WindowProperty& windowProperty)
{
}

void WindowAgent::UpdateWindowRect(const struct Rect& rect)
{
    if (window_ == nullptr) {
        WLOGFE("window_ is nullptr");
        return;
    }
    window_->UpdateRect(rect);
}

void WindowAgent::UpdateWindowMode(WindowMode mode)
{
    if (window_ == nullptr) {
        WLOGFE("window_ is nullptr");
        return;
    }
    window_->UpdateMode(mode);
}
} // namespace Rosen
} // namespace OHOS
