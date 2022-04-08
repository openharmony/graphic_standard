/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "rs_surface_ohos.h"

#include "rs_surface_frame_ohos.h"
#include "rs_surface_ohos_gl.h"
#include "rs_surface_ohos_raster.h"

namespace OHOS {
namespace Rosen {
RSSurfaceOhos::RSSurfaceOhos(const sptr<Surface>& producer)
    : producer_(producer)
{
    producer_->SetQueueSize(5);
}

std::shared_ptr<RSSurface> RSSurfaceOhos::CreateSurface(sptr<Surface> surface)
{
    auto type = Setting::GetRenderBackendType();
    std::shared_ptr<RSSurface> producer = nullptr;
    switch (type) {
        case RenderBackendType::GLES:
#ifdef ACE_ENABLE_GL
            LOGI("RSSurfaceOhos::CreateSurface with gles backend");
            producer = std::make_shared<RSSurfaceOhosGl>(surface);
#endif
            break;
        case RenderBackendType::SOFTWARE:
            LOGI("RSSurfaceOhos::CreateSurface with software backend");
            producer = std::make_shared<RSSurfaceOhosRaster>(surface);
            break;
        default:
            break;
    }
    return producer;
}
} // namespace Rosen
} // namespace OHOS