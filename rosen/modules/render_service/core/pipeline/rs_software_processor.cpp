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

#include "pipeline/rs_software_processor.h"

#include "include/core/SkMatrix.h"

namespace OHOS {
namespace Rosen {

RSSoftwareProcessor::RSSoftwareProcessor() {}

RSSoftwareProcessor::~RSSoftwareProcessor() {}

void RSSoftwareProcessor::Init(ScreenId id)
{
    sptr<RSScreenManager> screenManager = CreateOrGetScreenManager();
    if (!screenManager) {
        return;
    }
    producerSurface_ = screenManager->GetProducerSurface(id);
    //TODO config size update
    BufferRequestConfig requestConfig = {
        .width = 0x100,
        .height = 0x100,
        .strideAlignment = 0x8,
        .format = PIXEL_FMT_RGBA_8888,
        .usage = HBM_USE_CPU_READ | HBM_USE_CPU_WRITE | HBM_USE_MEM_DMA,
        .timeout = 0,
    };
    auto uniqueCanvasPtr = CreateCanvas(producerSurface_, requestConfig);
    canvas_ = std::move(uniqueCanvasPtr);
}

void RSSoftwareProcessor::PostProcess()
{
    //TODO config size update
    BufferFlushConfig flushConfig = {
        .damage = {
            .x = 0,
            .y = 0,
            .w = 0x100,
            .h = 0x100,
        },
    };
    FlushBuffer(producerSurface_, flushConfig);
}

void RSSoftwareProcessor::ProcessSurface(RSSurfaceRenderNode& node)
{
    if (!canvas_) {
        return;
    }
    auto consumerSurface = node.GetConsumer();
    if (!consumerSurface) {
        return;
    }
    int32_t flushFence = 0;
    int64_t timestamp = 0;
    OHOS::Rect damage;
    sptr<OHOS::SurfaceBuffer> buffer = nullptr;
    SurfaceError ret = consumerSurface->AcquireBuffer(buffer, flushFence, timestamp, damage);
    if (buffer == nullptr || ret != SURFACE_ERROR_OK) {
        return;
    }
    DrawBuffer(canvas_.get(), node.GetMatrix(), buffer);
    //TODO release buffer
}

}
}