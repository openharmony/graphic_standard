/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef RENDER_SERVICE_CORE_PIPELINE_RS_RENDER_SERVICE_UTIL_H
#define RENDER_SERVICE_CORE_PIPELINE_RS_RENDER_SERVICE_UTIL_H

#include <surface.h>
#include "display_type.h"
#include "hdi_backend.h"
#include "hdi_layer_info.h"
#include "common/rs_obj_abs_geometry.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "pipeline/rs_display_render_node.h"
#include "include/core/SkImage.h"
#include "pipeline/rs_paint_filter_canvas.h"
#include "pipeline/rs_surface_render_node.h"
#include "property/rs_transition_properties.h"
#include "screen_manager/screen_types.h"
#include "sync_fence.h"
#ifdef RS_ENABLE_GL
#include "rs_egl_image_manager.h"
#endif // RS_ENABLE_GL

namespace OHOS {

namespace Rosen {
struct BufferDrawParam {
    sptr<OHOS::SurfaceBuffer> buffer;
    sptr<SyncFence> acquireFence = SyncFence::INVALID_FENCE;
    SkMatrix matrix;
    SkRect srcRect;
    SkRect dstRect;
    SkRect clipRect;
    Vector4f cornerRadius;
    bool isNeedClip = true;
    SkPaint paint;
    ColorGamut targetColorGamut = ColorGamut::COLOR_GAMUT_SRGB;
};

struct ComposeInfo {
    IRect srcRect;
    IRect dstRect;
    IRect visibleRect;
    int32_t zOrder{0};
    LayerAlpha alpha;
    sptr<SurfaceBuffer> buffer;
    sptr<SyncFence> fence = SyncFence::INVALID_FENCE;
    sptr<SurfaceBuffer> preBuffer;
    sptr<SyncFence> preFence = SyncFence::INVALID_FENCE;
    BlendType blendType;
};

class RsRenderServiceUtil {
public:
    using CanvasPostProcess = std::function<void(RSPaintFilterCanvas&, BufferDrawParam&)>;
    static void ComposeSurface(std::shared_ptr<HdiLayerInfo> layer, sptr<Surface> consumerSurface,
        std::vector<LayerInfoPtr>& layers, ComposeInfo info, RSBaseRenderNode* node = nullptr);
    static void DrawBuffer(RSPaintFilterCanvas& canvas, BufferDrawParam& bufferDrawParam,
        CanvasPostProcess process = nullptr);

#ifdef RS_ENABLE_GL
    static void DrawImage(std::shared_ptr<RSEglImageManager> eglImageManager, GrContext* grContext,
        RSPaintFilterCanvas& canvas, BufferDrawParam& bufferDrawParam, CanvasPostProcess process);
#endif // RS_ENABLE_GL

    static BufferDrawParam CreateBufferDrawParam(RSSurfaceRenderNode& node, SkMatrix canvasMatrix = SkMatrix(),
        ScreenRotation rotation = ScreenRotation::ROTATION_0);
    static void DealAnimation(RSPaintFilterCanvas& canvas, RSSurfaceRenderNode& node, BufferDrawParam& params,
        const Vector2f& center);
    static void InitEnableClient();
    static bool CreateYuvToRGBABitMap(sptr<OHOS::SurfaceBuffer> buffer, std::vector<uint8_t>& newBuffer,
        SkBitmap& bitmap);

    static void DropFrameProcess(RSSurfaceHandler& node);
    static bool ConsumeAndUpdateBuffer(RSSurfaceHandler& node, bool toReleaseBuffer = false);

private:
    static SkMatrix GetCanvasTransform(const RSSurfaceRenderNode& node, const SkMatrix& canvasMatrix,
        ScreenRotation rotation, SkRect clipRect);
    static bool IsNeedClient(RSSurfaceRenderNode* node);
    static bool CreateBitmap(sptr<OHOS::SurfaceBuffer> buffer, SkBitmap& bitmap);

    static bool CreateNewColorGamutBitmap(sptr<OHOS::SurfaceBuffer> buffer, std::vector<uint8_t>& newGamutBuffer,
        SkBitmap& bitmap, ColorGamut srcGamut, ColorGamut dstGamut);
    static bool enableClient;
};
} // Rosen
} // OHOS
#endif
