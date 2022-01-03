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

#ifndef OHOS_ROSEN_WINDOW_LIFE_CYCLE_H
#define OHOS_ROSEN_WINDOW_LIFE_CYCLE_H

#include <refbase.h>

namespace OHOS {
namespace Rosen {
class IWindowLifeCycle : public RefBase {
public:
    virtual void AfterCreated() = 0;
    virtual void BeforeDestroy() = 0;
    virtual void AfterForeground() = 0;
    virtual void AfterBackground() = 0;
    virtual void AfterFocused() = 0;
    virtual void AfterUnFocused() = 0;
};
}
}
#endif // OHOS_ROSEN_WINDOW_LIFE_CYCLE_H
