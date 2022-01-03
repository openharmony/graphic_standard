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

#ifndef OHOS_ROSEN_WINDOW_IMPL_H
#define OHOS_ROSEN_WINDOW_IMPL_H
#include <refbase.h>
#include <map>
#include "window.h"

namespace OHOS {
namespace Rosen {
class WindowImpl : public Window {
public:
    WindowImpl(const sptr<WindowProperty>& property);
    ~WindowImpl();

    static sptr<Window> Find(const std::string& id);
    static WMError SetWindowVisibility(const std::string& id, bool isVisible);
    static const std::string& ConvertId(const std::string& id);

//    virtual sptr<Surface> GetSurface() const override;
    virtual bool GetVisibility() const override;
    virtual Rect GetRect() const override;
    virtual WindowType GetType() const override;
    virtual WindowMode GetMode() const override;
    virtual const std::string& GetId() const override;
    virtual sptr<WindowProperty> GetProperty() const override;

    virtual WMError SetVisibility(bool visibility) override;
    virtual WMError SetWindowType(WindowType type) override;
    virtual WMError SetWindowMode(WindowMode mode) override;

    virtual WMError Destroy() override;
    virtual WMError Show() override;
    virtual WMError Hide() override;
    virtual WMError MoveTo(int32_t x, int32_t y) override;
    virtual WMError Resize(uint32_t width, uint32_t height) override;

    virtual void RegisterLifeCycleListener(sptr<IWindowLifeCycle>& listener) override;

    void UpdateRect(const struct Rect& rect);
    void UpdateMode(WindowMode mode);
private:
    static std::map<std::string, sptr<Window>> windowMap_;
    sptr<WindowProperty> property_;
    bool visibility_ { false };
    bool isAdded_ { false };
    sptr<IWindowLifeCycle> lifecycleListener_;
//    int renderNodeId_ { 0 };
//    sptr<Surface> surface_;
};
}
}
#endif // OHOS_ROSEN_WINDOW_IMPL_H
