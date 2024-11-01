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

#include "rs_surface_frame_ohos_raster.h"
#include <cstdint>

#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {

RSSurfaceFrameOhosRaster::RSSurfaceFrameOhosRaster(int32_t width, int32_t height)
{
    requestConfig_.width = width;
    requestConfig_.height = height;
    flushConfig_.damage.w = width;
    flushConfig_.damage.h = height;
}

void RSSurfaceFrameOhosRaster::SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height)
{
    flushConfig_.damage.x = left;
    flushConfig_.damage.y = top;
    flushConfig_.damage.w = width;
    flushConfig_.damage.h = height;
}

SkCanvas* RSSurfaceFrameOhosRaster::GetCanvas()
{
    if (buffer_ == nullptr || buffer_->GetWidth() <= 0 || buffer_->GetHeight() <= 0) {
        ROSEN_LOGW("buffer is invalid");
        return nullptr;
    }
    if (skSurface_ == nullptr) {
        CreateSurface();
    }
    return skSurface_->getCanvas();
}

sk_sp<SkSurface> RSSurfaceFrameOhosRaster::GetSurface()
{
    if (buffer_ == nullptr || buffer_->GetWidth() <= 0 || buffer_->GetHeight() <= 0) {
        ROSEN_LOGW("buffer is invalid");
        return nullptr;
    }
    if (skSurface_ == nullptr) {
        CreateSurface();
    }
    return skSurface_;
}

void RSSurfaceFrameOhosRaster::CreateSurface()
{
    auto addr = static_cast<uint32_t*>(buffer_->GetVirAddr());
    if (addr == nullptr) {
        ROSEN_LOGW("buffer addr is invalid");
        return;
    }
    SkImageInfo info =
        SkImageInfo::Make(buffer_->GetWidth(), buffer_->GetHeight(), kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    skSurface_ = SkSurface::MakeRasterDirect(info, addr, buffer_->GetStride());
}

int32_t RSSurfaceFrameOhosRaster::GetReleaseFence() const
{
    return releaseFence_;
}

void RSSurfaceFrameOhosRaster::SetReleaseFence(const int32_t& fence)
{
    releaseFence_ = fence;
}

} // namespace Rosen
} // namespace OHOS