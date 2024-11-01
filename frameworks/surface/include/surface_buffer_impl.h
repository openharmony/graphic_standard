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

#ifndef FRAMEWORKS_SURFACE_INCLUDE_SURFACE_BUFFER_IMPL_H
#define FRAMEWORKS_SURFACE_INCLUDE_SURFACE_BUFFER_IMPL_H

#include <buffer_extra_data.h>
#include <buffer_handle_parcel.h>
#include <buffer_handle_utils.h>
#include <surface_buffer.h>
#include <idisplay_gralloc.h>
#include "egl_data.h"

namespace OHOS {
class SurfaceBufferImpl : public SurfaceBuffer {
public:
    using IDisplayGrallocSptr = std::shared_ptr<::OHOS::HDI::Display::V1_0::IDisplayGralloc>;
    static IDisplayGrallocSptr GetDisplayGralloc();

    SurfaceBufferImpl();
    SurfaceBufferImpl(int seqNum);
    virtual ~SurfaceBufferImpl();

    static SurfaceBufferImpl *FromBase(const sptr<SurfaceBuffer>& buffer);

    GSError Alloc(const BufferRequestConfig &config) override;
    GSError Map() override;
    GSError Unmap() override;
    GSError FlushCache() override;
    GSError InvalidateCache() override;

    BufferHandle *GetBufferHandle() const override;
    int32_t GetWidth() const override;
    int32_t GetHeight() const override;
    int32_t GetStride() const override;
    int32_t GetFormat() const override;
    int64_t GetUsage() const override;
    uint64_t GetPhyAddr() const override;
    int32_t GetKey() const override;
    void *GetVirAddr() override;
    int32_t GetFileDescriptor() const override;
    uint32_t GetSize() const override;

    const ColorGamut& GetSurfaceBufferColorGamut() const override;
    const TransformType& GetSurfaceBufferTransform() const override;
    void SetSurfaceBufferColorGamut(const ColorGamut& colorGamut) override;
    void SetSurfaceBufferTransform(const TransformType& transform) override;

    const ScalingMode& GetSurfaceBufferScalingMode() const override;

    int32_t GetSurfaceBufferWidth() const override;
    int32_t GetSurfaceBufferHeight() const override;
    void SetSurfaceBufferWidth(int32_t width) override;
    void SetSurfaceBufferHeight(int32_t width) override;

    int32_t GetSeqNum() const override;
    
    sptr<EglData> GetEglData() const override;
    void SetEglData(const sptr<EglData>& data) override;

    void SetExtraData(const sptr<BufferExtraData> &bedata) override;
    const sptr<BufferExtraData>& GetExtraData() const override;

    void SetBufferHandle(BufferHandle *handle) override;
    GSError WriteToMessageParcel(MessageParcel &parcel) override;
    GSError ReadFromMessageParcel(MessageParcel &parcel) override;

private:
    void FreeBufferHandleLocked();

    BufferHandle *handle_ = nullptr;
    int32_t sequenceNumber_ = -1;
    sptr<BufferExtraData> bedata_ = nullptr;
    sptr<EglData> eglData_ = nullptr;
    ColorGamut surfaceBufferColorGamut_ = ColorGamut::COLOR_GAMUT_SRGB;
    TransformType transform_ = TransformType::ROTATE_NONE;
    ScalingMode surfaceBufferScalingMode_ = ScalingMode::SCALING_MODE_SCALE_TO_WINDOW;
    int32_t surfaceBufferWidth_ = 0;
    int32_t surfaceBufferHeight_ = 0;
    mutable std::mutex mutex_;

    static IDisplayGrallocSptr displayGralloc_;
};
} // namespace OHOS

#endif // FRAMEWORKS_SURFACE_INCLUDE_SURFACE_BUFFER_IMPL_H
