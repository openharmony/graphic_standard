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

#include "hdi_layer.h"

#include "hdi_log.h"
#include "hdi_device.h"

namespace OHOS {
namespace Rosen {

/* rs create layer and set layer info begin */
std::shared_ptr<HdiLayer> HdiLayer::CreateHdiLayer(uint32_t layerId)
{
    return std::make_shared<HdiLayer>(layerId);
}

HdiLayer::HdiLayer(uint32_t layerId) : layerId_(layerId)
{
}

HdiLayer::~HdiLayer()
{
    SurfaceError ret = ReleasePrevBuffer();
    if (ret != SURFACE_ERROR_OK) {
        HLOGE("Destory hdiLayer, ReleaseBuffer failed, ret is %{public}d", ret);
    }
}

void HdiLayer::Init()
{
    if (prevSbuffer_ == nullptr) {
        prevSbuffer_ = new LayerBufferInfo();
    }

    if (currSbuffer_ == nullptr) {
        currSbuffer_ = new LayerBufferInfo();
    }
}

void HdiLayer::SetHdiLayerInfo(uint32_t screenId)
{
    /*
        Some hardware platforms may not support all layer settings.
        If the current function is not supported, continue other layer settings.
     */

    HdiDevice *device = HdiDevice::GetInstance();
    if (device == nullptr || layerInfo_ == nullptr) {
        return;
    }

    int32_t ret = device->SetLayerAlpha(screenId, layerId_, layerInfo_->GetAlpha());
    CheckRet(ret, "SetLayerAlpha");

    ret = device->SetLayerSize(screenId, layerId_, layerInfo_->GetLayerSize());
    CheckRet(ret, "SetLayerSize");

    ret = device->SetTransformMode(screenId, layerId_, layerInfo_->GetTransformType());
    CheckRet(ret, "SetTransformMode");

    ret = device->SetLayerVisibleRegion(screenId, layerId_, layerInfo_->GetVisibleNum(),
                                         layerInfo_->GetVisibleRegion());
    CheckRet(ret, "SetLayerVisibleRegion");

    ret = device->SetLayerDirtyRegion(screenId, layerId_, layerInfo_->GetDirtyRegion());
    CheckRet(ret, "SetLayerDirtyRegion");

    ret = device->SetLayerBuffer(screenId, layerId_, layerInfo_->GetBuffer()->GetBufferHandle(),
                                  layerInfo_->GetAcquireFence());
    CheckRet(ret, "SetLayerBuffer");

    ret = device->SetLayerCompositionType(screenId, layerId_, layerInfo_->GetCompositionType());
    CheckRet(ret, "SetLayerCompositionType");

    ret = device->SetLayerBlendType(screenId, layerId_, layerInfo_->GetBlendType());
    CheckRet(ret, "SetLayerBlendType");

    ret = device->SetLayerCrop(screenId, layerId_, layerInfo_->GetCropRect());
    CheckRet(ret, "SetLayerCrop");

    ret = device->SetLayerZorder(screenId, layerId_, layerInfo_->GetZorder());
    CheckRet(ret, "SetLayerZorder");

    ret = device->SetLayerPreMulti(screenId, layerId_, layerInfo_->IsPreMulti());
    CheckRet(ret, "SetLayerPreMulti");
}

uint32_t HdiLayer::GetLayerId() const
{
    return layerId_;
}

const LayerInfoPtr& HdiLayer::GetLayerInfo()
{
    return layerInfo_;
}

void HdiLayer::SetLayerStatus(bool inUsing)
{
    isInUsing_ = inUsing;
}

bool HdiLayer::GetLayerStatus() const
{
    return isInUsing_;
}

void HdiLayer::UpdateLayerInfo(const LayerInfoPtr &layerInfo)
{
    if (layerInfo == nullptr) {
        return;
    }

    /* If the layer is updated, it indicates that the layer will be used
     * in the frame. Mark it.
     */

    isInUsing_ = true;
    layerInfo_ = layerInfo;

    currSbuffer_->sbuffer_ = layerInfo_->GetBuffer();
    currSbuffer_->acquireFence_ = layerInfo_->GetAcquireFence();
}

void HdiLayer::ReleaseBuffer()
{
    if (currSbuffer_->sbuffer_ != prevSbuffer_->sbuffer_) {
        SurfaceError ret = ReleasePrevBuffer();
        if (ret != SURFACE_ERROR_OK) {
            HLOGE("ReleaseBuffer failed, ret is %{public}d", ret);
        }

        /* copy currSbuffer to prevSbuffer */
        prevSbuffer_->sbuffer_ = currSbuffer_->sbuffer_;
        prevSbuffer_->acquireFence_ = currSbuffer_->acquireFence_;
        prevSbuffer_->releaseFence_ = currSbuffer_->releaseFence_;
    } else {
        prevSbuffer_->acquireFence_ = Merge(currSbuffer_->acquireFence_, prevSbuffer_->acquireFence_);
        prevSbuffer_->releaseFence_ = Merge(currSbuffer_->releaseFence_, prevSbuffer_->releaseFence_);
    }
}

SurfaceError HdiLayer::ReleasePrevBuffer()
{
    /*
     * The first time, prevSbuffer is empty and the releaseFence is invalid.
     * Therefore, we do not release the buffer.
     */

    if (prevSbuffer_->sbuffer_ == nullptr) {
        return SURFACE_ERROR_OK;
    }

    if (layerInfo_ == nullptr) {
        return SURFACE_ERROR_NULLPTR;
    }

    int32_t fenceFd = prevSbuffer_->releaseFence_->Dup();
    SurfaceError ret = layerInfo_->GetSurface()->ReleaseBuffer(prevSbuffer_->sbuffer_, fenceFd);
    if (ret != SURFACE_ERROR_OK) {
        return ret;
    }

    prevSbuffer_->sbuffer_ = nullptr;
    prevSbuffer_->releaseFence_ = nullptr;

    return ret;
}

void HdiLayer::MergeWithFramebufferFence(const sptr<SyncFence> &fbAcquireFence)
{
    if (fbAcquireFence != nullptr) {
        currSbuffer_->releaseFence_ = Merge(currSbuffer_->releaseFence_, fbAcquireFence);
    }
}

void HdiLayer::MergeWithLayerFence(const sptr<SyncFence> &layerReleaseFence)
{
    if (layerReleaseFence != nullptr) {
        prevSbuffer_->releaseFence_ = Merge(prevSbuffer_->releaseFence_, layerReleaseFence);
    }
}
/* backend get layer info end */

sptr<SyncFence> HdiLayer::Merge(const sptr<SyncFence> &fence1, const sptr<SyncFence> &fence2)
{
    return SyncFence::MergeFence("ReleaseFence", fence1, fence2);
}

void HdiLayer::CheckRet(int32_t ret, const char* func)
{
    if (ret != DISPLAY_SUCCESS) {
        HLOGD("call hdi %{public}s failed, ret is %{public}d", func, ret);
    }
}

} // namespace Rosen
} // namespace OHOS
