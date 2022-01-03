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

#ifndef FRAMEWORKS_GRAPHIC_RS_SCREEN_MANAGER_TEST_UNITTEST_SCREEN_MANAGER_TEST_H
#define FRAMEWORKS_GRAPHIC_RS_SCREEN_MANAGER_TEST_UNITTEST_SCREEN_MANAGER_TEST_H

#include <gtest/gtest.h>
#include <hilog/log.h>

#include <rs_screen_manager.h>
#include <surface.h>

namespace OHOS {
namespace Rosen {
class RSScreenManagerTest : public testing::Test {
public:
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = { LOG_CORE, 0, "RSScreenManager" };

    static void SetUpTestCase();
    static void TearDownTestCase();

    static inline sptr<OHOS::Rosen::RSScreenManager> screenManager;
    static inline ScreenId screenId;
    static inline sptr<OHOS::Surface> cSurface = nullptr;
    static inline sptr<OHOS::IBufferProducer> producer = nullptr;
    static inline sptr<OHOS::Surface> pSurface = nullptr;
};
} // namespace Rosen
} // namespace OHOS

#endif // FRAMEWORKS_GRAPHIC_RS_SCREEN_MANAGER_TEST_UNITTEST_SCREEN_MANAGER_TEST_H
