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

#include "rs_surface_frame_ohos_gl.h"

namespace OHOS {
namespace Rosen {
RSSurfaceFrameOhosGl::RSSurfaceFrameOhosGl(int32_t width, int32_t height)
    : RSSurfaceFrameOhos(width, height), eglSurface_(EGL_NO_SURFACE), colorSpace_(ColorGamut::COLOR_GAMUT_SRGB)
{
}

RSSurfaceFrameOhosGl::~RSSurfaceFrameOhosGl()
{
}

void RSSurfaceFrameOhosGl::SetColorSpace(ColorGamut colorSpace)
{
    colorSpace_ = colorSpace;
}

ColorGamut RSSurfaceFrameOhosGl::GetColorSpace() const
{
    return colorSpace_;
}

void RSSurfaceFrameOhosGl::SetSurface(EGLSurface surface)
{
    eglSurface_ = surface;
}
} // namespace Rosen
} // namespace OHOS
