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

#ifndef OHOS_ROSEN_WINDOW_H
#define OHOS_ROSEN_WINDOW_H

#include <refbase.h>
#include "iremote_object.h"
#include "wm_common.h"
#include "window_property.h"
#include "window_life_cycle_interface.h"

namespace OHOS {
namespace Rosen {
class Window : public RefBase {
public:
    static sptr<Window> Create(const std::string& id, sptr<WindowProperty>& property);
    static sptr<Window> Find(const std::string& id);

    //    virtual sptr<Surface> GetSurface() const = 0;
    virtual bool GetVisibility() const = 0;
    virtual Rect GetRect() const = 0;
    virtual WindowType GetType() const = 0;
    virtual WindowMode GetMode() const = 0;
    virtual const std::string& GetId() const = 0;
    virtual sptr<WindowProperty> GetProperty() const = 0;

    virtual WMError SetVisibility(bool visibility) = 0;
    virtual WMError SetWindowType(WindowType type) = 0;
    virtual WMError SetWindowMode(WindowMode mode) = 0;

    virtual WMError Destroy() = 0;
    virtual WMError Show() = 0;
    virtual WMError Hide() = 0;

    virtual WMError MoveTo(int32_t x, int32_t y) = 0;
    virtual WMError Resize(uint32_t width, uint32_t height) = 0;

    virtual void RegisterLifeCycleListener(sptr<IWindowLifeCycle>& listener) = 0;
};
}
}
#endif // OHOS_ROSEN_WINDOW_H
