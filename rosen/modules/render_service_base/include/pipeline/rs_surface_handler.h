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
#ifndef RENDER_SERVICE_CLIENT_CORE_PIPELINE_RS_SURFACE_HANDLER_H
#define RENDER_SERVICE_CLIENT_CORE_PIPELINE_RS_SURFACE_HANDLER_H

#include <surface.h>

#include "common/rs_common_def.h"
#include "sync_fence.h"

namespace OHOS {
namespace Rosen {
class RSSurfaceHandler {
public:
    RSSurfaceHandler() = default;
    virtual ~RSSurfaceHandler() = default;
    virtual NodeId GetId() const = 0;
    void SetConsumer(const sptr<Surface>& consumer);
    void SetBuffer(const sptr<SurfaceBuffer>& buffer);
    void SetFence(sptr<SyncFence> fence);
    void SetDamageRegion(const Rect& damage);
    void IncreaseAvailableBuffer();
    int32_t ReduceAvailableBuffer();

    sptr<SurfaceBuffer>& GetBuffer()
    {
        return buffer_;
    }

    sptr<SyncFence> GetFence() const
    {
        return fence_;
    }

    sptr<SurfaceBuffer>& GetPreBuffer()
    {
        return preBuffer_;
    }

    sptr<SyncFence> GetPreFence() const
    {
        return preFence_;
    }

    const Rect& GetDamageRegion() const
    {
        return damageRect_;
    }

    const sptr<Surface>& GetConsumer() const
    {
        return consumer_;
    }

    int32_t GetAvailableBufferCount() const
    {
        return bufferAvailableCount_;
    }

    void SetGlobalZOrder(float globalZOrder);
    float GetGlobalZOrder() const;

protected:
    sptr<Surface> consumer_;

private:
    Rect damageRect_;
    float globalZOrder_ = 0.0f;
    std::atomic<int> bufferAvailableCount_ = 0;
    sptr<SurfaceBuffer> buffer_;
    sptr<SurfaceBuffer> preBuffer_;
    sptr<SyncFence> fence_;
    sptr<SyncFence> preFence_;
};
}
}
#endif // RENDER_SERVICE_CLIENT_CORE_PIPELINE_RS_SURFACE_HANDLER_H
