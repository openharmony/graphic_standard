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

#ifndef GLES_RENDER_BACKEND_H
#define GLES_RENDER_BACKEND_H
#include "interface_render_backend.h"
#include "egl_manager.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/gl/GrGLInterface.h"

namespace OHOS {
namespace Rosen {
const int STENCIL_BUFFER_SIZE = 8;
class GLESRenderBackend : public IRenderBackend {
public:
    GLESRenderBackend() noexcept;
    ~GLESRenderBackend() override;
    void InitDrawContext() override;
    void MakeCurrent() override;
    void SwapBuffers() override;
    void* CreateSurface(void* window) override;
    void SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height) override;
    GrContext* GetGrContext() const
    {
        return grContext_.get();
    }
    bool SetUpGrContext();
    void Destroy() override;
    void RenderFrame() override;
    SkCanvas* AcquireCanvas(std::unique_ptr<RSSurfaceFrame>& frame) override;
private:
    EGLManager* eglManager_ = nullptr;
    sk_sp<GrContext> grContext_ = nullptr;
    sk_sp<SkSurface> skSurface_ = nullptr;
};
}
}
#endif