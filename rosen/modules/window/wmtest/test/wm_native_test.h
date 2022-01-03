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

#ifndef OHOS_ROSEN_WM_NATIVE_TEST_H
#define OHOS_ROSEN_WM_NATIVE_TEST_H

#include "inative_test.h"

namespace OHOS::Rosen {
class WMNativeTest : public INativeTest {
public:
    virtual ~WMNativeTest() = default;
    virtual std::string GetDescription() const override;
    virtual std::string GetDomain() const override;
    virtual int32_t GetID() const override;
    virtual uint32_t GetLastTime() const override;

    virtual void Run(int32_t argc, const char **argv) override;

private:
    void PostTask(std::function<void()> func, uint32_t delayTime = 0);
    void ExitTest();
    int64_t GetNowTime();
};
}

#endif // OHOS_ROSEN_WM_NATIVE_TEST_H
