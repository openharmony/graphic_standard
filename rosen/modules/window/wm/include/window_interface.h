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

#ifndef OHOS_WINDOW_INTERFACE_H
#define OHOS_WINDOW_INTERFACE_H

#include "iremote_broker.h"
#include "window_property.h"

namespace OHOS {
namespace Rosen {
class IWindow : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.IWindow");

    enum {
        TRANS_ID_UPDATE_WINDOW_PROPERTY = 1,
        TRANS_ID_UPDATE_WINDOW_RECT,
        TRANS_ID_UPDATE_WINDOW_MODE,
    };

    virtual void UpdateWindowProperty(const WindowProperty& windowProperty) = 0;
    virtual void UpdateWindowRect(const struct Rect& rect) = 0;
    virtual void UpdateWindowMode(WindowMode mode) = 0;
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_WINDOW_INTERFACE_H
