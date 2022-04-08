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

#ifndef RS_SURFACE_OHOS_H
#define RS_SURFACE_OHOS_H

#include <surface.h>
#include "rs_surface.h"
#include "window.h"
#include "rs_surface_frame_ohos.h"
#include "drawing_engine/drawing_utils.h"
#include "drawing_engine/drawing_proxy.h"

namespace OHOS {
namespace Rosen {
class RSSurfaceOhos : public RSSurface {
public:
    static std::shared_ptr<RSSurface> CreateSurface(sptr<Surface> surface);

    explicit RSSurfaceOhos(const sptr<Surface>& producer);
    ~RSSurfaceOhos() override {};

    bool IsValid() const override
    {
        return producer_ != nullptr;
    }

    sptr<Surface> GetSurface() const
    {
        return producer_;
    }
protected:
    sptr<Surface> producer_;
};
} // namespace Rosen
} // namespace OHOS
#endif
