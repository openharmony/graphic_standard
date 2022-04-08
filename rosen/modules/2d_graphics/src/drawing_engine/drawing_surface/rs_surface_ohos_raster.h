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

#ifndef RS_SURFACE_OHOS_RASTER_H
#define RS_SURFACE_OHOS_RASTER_H

#include <surface.h>
#include "rs_surface.h"
#include "rs_surface_ohos.h"
#include "rs_surface_frame_ohos_raster.h"

namespace OHOS {
namespace Rosen {
class RSSurfaceOhosRaster : public RSSurfaceOhos {
public:
    explicit RSSurfaceOhosRaster(const sptr<Surface>& producer);
    ~RSSurfaceOhosRaster() override;
    std::unique_ptr<RSSurfaceFrame> RequestFrame(int32_t width, int32_t height) override;
    bool FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame) override;
    SkCanvas* GetCanvas(std::unique_ptr<RSSurfaceFrame>& frame) override;
private:
    std::unique_ptr<RSSurfaceFrameOhosRaster> frame_;
};
} // namespace Rosen
} // namespace OHOS

#endif
