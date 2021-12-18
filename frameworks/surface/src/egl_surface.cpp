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

#include <surface.h>

#include <hilog/log.h>

#include "buffer_log.h"
#include "producer_egl_surface.h"

namespace OHOS {
namespace {
constexpr HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0, "EglSurface" };
}

sptr<EglSurface> EglSurface::CreateEglSurfaceAsProducer(sptr<IBufferProducer>& producer)
{
    if (producer == nullptr) {
        BLOGE("Failure, Reason: producer is nullptr.");
        return nullptr;
    }

    sptr<ProducerEglSurface> surface = new ProducerEglSurface(producer);
    if (surface == nullptr) {
        BLOGE("Failure, Reason: no memory.");
    }
    return surface;
}
} // namespace OHOS