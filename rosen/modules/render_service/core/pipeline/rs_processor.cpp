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

#include "pipeline/rs_processor.h"
#include "display_type.h"
namespace OHOS {
namespace Rosen {

std::unique_ptr<SkCanvas> RSProcessor::CreateCanvas(sptr<Surface> producerSurface, BufferRequestConfig requestConfig)
{
    auto ret = producerSurface->RequestBuffer(buffer_, releaseFence_, requestConfig);
    if (ret != SURFACE_ERROR_OK || buffer_ == nullptr) {
        return nullptr;
    }

    auto addr = static_cast<uint32_t*>(buffer_->GetVirAddr());
    if (addr == nullptr) {
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(buffer_->GetWidth(), buffer_->GetHeight(),
                                        kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    return SkCanvas::MakeRasterDirect(info, addr, buffer_->GetSize() / buffer_->GetHeight());
}

void RSProcessor::DrawBuffer(SkCanvas* canvas, const SkMatrix& matrix, sptr<OHOS::SurfaceBuffer> buffer)
{
    if (!canvas) {
        return;
    }
    auto addr = static_cast<uint32_t*>(buffer->GetVirAddr());
    if (addr == nullptr || buffer->GetWidth() <= 0 || buffer->GetHeight() <= 0) {
        return;
    }
    SkImageInfo layerInfo = SkImageInfo::Make(buffer->GetWidth(), buffer->GetHeight(),
        kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkPixmap pixmap(layerInfo, addr, buffer->GetSize() / buffer->GetHeight());
    SkBitmap bitmap;
    if (bitmap.installPixels(pixmap)) {
        canvas->save();
        canvas->concat(matrix);
        canvas->drawBitmapRect(bitmap, SkRect::MakeXYWH(0, 0, buffer->GetWidth(), buffer->GetHeight()), nullptr);
        canvas->restore();
    }
}

void RSProcessor::FlushBuffer(sptr<Surface> surface, BufferFlushConfig flushConfig)
{
    if (!surface) {
        return;
    }
    surface->FlushBuffer(buffer_, releaseFence_, flushConfig);
}

}
}