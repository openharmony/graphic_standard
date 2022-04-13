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

#include "pipeline/rs_uni_render_visitor.h"

#include <surface.h>
#include <window.h>
#include <window_manager.h>
#include <window_option.h>

#include "common/rs_obj_abs_geometry.h"
#include "display_type.h"
#include "pipeline/rs_display_render_node.h"
#include "pipeline/rs_processor_factory.h"
#include "pipeline/rs_root_render_node.h"
#include "pipeline/rs_surface_render_node.h"
#include "pipeline/rs_uni_render_listener.h"
#include "platform/common/rs_log.h"
#include "platform/common/rs_system_properties.h"
#include "property/rs_properties_painter.h"
#include "render/rs_skia_filter.h"
#include "screen_manager/screen_types.h"

namespace OHOS {
namespace Rosen {

RSUniRenderVisitor::RSUniRenderVisitor() {}

RSUniRenderVisitor::~RSUniRenderVisitor() {}

void RSUniRenderVisitor::PrepareBaseRenderNode(RSBaseRenderNode& node)
{
    for (auto& child : node.GetChildren()) {
        if (auto c = child.lock()) {
            c->Prepare(shared_from_this());
        }
    }
    for (auto& child : node.GetDisappearingChildren()) {
        child->Prepare(shared_from_this());
    }
}

void RSUniRenderVisitor::PrepareDisplayRenderNode(RSDisplayRenderNode& node)
{
    isUniRenderForAll_ =
        RSSystemProperties::GetUniRenderEnabledType() == UniRenderEnabledType::UNI_RENDER_ENABLED_FOR_ALL;
    if (!isUniRenderForAll_) {
        RS_LOGI("RSUniRenderVisitor::PrepareDisplayRenderNode isUniRenderForAll_ false");
        uniRenderList_ = RSSystemProperties::GetUniRenderEnabledList();
    }
    PrepareBaseRenderNode(node);
}

void RSUniRenderVisitor::PrepareSurfaceRenderNode(RSSurfaceRenderNode& node)
{
    if (isUniRenderForAll_ || uniRenderList_.find(node.GetName()) != uniRenderList_.end()) {
        isUniRender_ = true;
        hasUniRender_ = true;
    }
    if (IsChildOfDisplayNode(node)) {
        auto currentGeoPtr = std::static_pointer_cast<RSObjAbsGeometry>(node.GetRenderProperties().GetBoundsGeometry());
        if (currentGeoPtr != nullptr) {
            currentGeoPtr->UpdateByMatrixFromParent(nullptr);
            currentGeoPtr->UpdateByMatrixFromRenderThread(node.GetMatrix());
            currentGeoPtr->UpdateByMatrixFromSelf();
        }
        PrepareBaseRenderNode(node);
    } else {
        bool dirtyFlag = dirtyFlag_;
        dirtyFlag_ = node.Update(dirtyManager_, parent_ ? &(parent_->GetRenderProperties()) : nullptr, dirtyFlag_);
        PrepareBaseRenderNode(node);
        dirtyFlag_ = dirtyFlag;
    }
    isUniRender_ = false;
}

void RSUniRenderVisitor::PrepareRootRenderNode(RSRootRenderNode& node)
{
    if (!isUniRender_ || !IsChildOfSurfaceNode(node)) {
        return;
    }
    parent_ = nullptr;
    dirtyFlag_ = false;
    PrepareCanvasRenderNode(node);
}

void RSUniRenderVisitor::PrepareCanvasRenderNode(RSCanvasRenderNode &node)
{
    bool dirtyFlag = dirtyFlag_;
    dirtyFlag_ = node.Update(dirtyManager_, parent_ ? &(parent_->GetRenderProperties()) : nullptr, dirtyFlag_);
    PrepareBaseRenderNode(node);
    dirtyFlag_ = dirtyFlag;
}

void RSUniRenderVisitor::ProcessBaseRenderNode(RSBaseRenderNode& node)
{
    for (auto& child : node.GetChildren()) {
        if (auto c = child.lock()) {
            c->Process(shared_from_this());
        }
    }
    for (auto& child : node.GetDisappearingChildren()) {
        child->Process(shared_from_this());
    }
}

void RSUniRenderVisitor::ProcessDisplayRenderNode(RSDisplayRenderNode& node)
{
    RS_LOGD("RSUniRenderVisitor::ProcessDisplayRenderNode node: %llu, child size:%d", node.GetChildrenCount(),
        node.GetId());
    globalZOrder_ = 0.0f;
    sptr<RSScreenManager> screenManager = CreateOrGetScreenManager();
    if (!screenManager) {
        RS_LOGE("RSUniRenderVisitor::ProcessDisplayRenderNode ScreenManager is nullptr");
        return;
    }
    screenInfo_ = screenManager->QueryScreenInfo(node.GetScreenId());
    switch (screenInfo_.state) {
        case ScreenState::PRODUCER_SURFACE_ENABLE:
            node.SetCompositeType(RSDisplayRenderNode::CompositeType::SOFTWARE_COMPOSITE);
            break;
        case ScreenState::HDI_OUTPUT_ENABLE:
            node.SetCompositeType(node.IsForceSoftComposite() ? RSDisplayRenderNode::CompositeType::COMPATIBLE_COMPOSITE
                : RSDisplayRenderNode::CompositeType::HARDWARE_COMPOSITE);
            break;
        default:
            RS_LOGE("RSUniRenderVisitor::ProcessDisplayRenderNode ScreenState unsupported");
            return;
    }
    processor_ = RSProcessorFactory::CreateProcessor(node.GetCompositeType());
    if (processor_ == nullptr) {
        RS_LOGE("RSUniRenderVisitor::ProcessDisplayRenderNode: RSProcessor is null!");
        return;
    }
    processor_->Init(node.GetScreenId());

    if (hasUniRender_) {
        std::shared_ptr<RSBaseRenderNode> nodePtr = node.shared_from_this();
        auto displayNodePtr = nodePtr->ReinterpretCastTo<RSDisplayRenderNode>();
        if (!displayNodePtr) {
            RS_LOGE("RSUniRenderVisitor::ProcessDisplayRenderNode ReinterpretCastTo fail");
            return;
        }
        sptr<IBufferConsumerListener> listener = new RSUniRenderListener(displayNodePtr);
        if (!node.CreateSurface(listener)) {
            RS_LOGE("RSUniRenderVisitor::ProcessDisplayRenderNode CreateSurface failed");
            return;
        }
        auto rsSurface = node.GetRSSurface();
        if (rsSurface == nullptr) {
            RS_LOGE("RSUniRenderVisitor::ProcessDisplayRenderNode No RSSurface found");
            return;
        }
        auto surfaceFrame = rsSurface->RequestFrame(screenInfo_.width, screenInfo_.height);
        if (surfaceFrame == nullptr) {
            RS_LOGE("RSUniRenderVisitor Request Frame Failed");
            return;
        }
        canvas_ = new RSPaintFilterCanvas(rsSurface->GetCanvas(surfaceFrame));
        canvas_->clear(SK_ColorTRANSPARENT);

        ProcessBaseRenderNode(node);

        rsSurface->FlushFrame(surfaceFrame);
        delete canvas_;
        canvas_ = nullptr;

        node.SetGlobalZOrder(uniZOrder_);
        processor_->ProcessSurface(node);
    } else {
        OHOS::sptr<SurfaceBuffer> buffer;
        RSProcessor::SpecialTask task = [] () {};
        bool ret = processor_->ConsumeAndUpdateBuffer(node, task, buffer);
        auto& surfaceConsumer = node.GetConsumer();
        if (ret && surfaceConsumer != nullptr) {
            (void)surfaceConsumer->ReleaseBuffer(node.GetBuffer(), -1);
        }
        ProcessBaseRenderNode(node);
    }
    processor_->PostProcess();
    RS_LOGD("RSUniRenderVisitor::ProcessDisplayRenderNode end");
}

void RSUniRenderVisitor::ProcessSurfaceRenderNode(RSSurfaceRenderNode& node)
{
    RS_LOGD("RSUniRenderVisitor::ProcessSurfaceRenderNode node: %llu, child size:%d", node.GetId(),
        node.GetChildrenCount());
    if (isUniRenderForAll_ || uniRenderList_.find(node.GetName()) != uniRenderList_.end()) {
        isUniRender_ = true;
    }
    if (isUniRender_) {
        // for window surface node - whose parent is display node
        if (IsChildOfDisplayNode(node)) {
            if (!node.GetRenderProperties().GetVisible()) {
                RS_LOGD("RSUniRenderVisitor::ProcessSurfaceRenderNode node: %llu invisible", node.GetId());
                return;
            }

            if (!canvas_) {
                RS_LOGE("RSUniRenderVisitor::ProcessSurfaceRenderNode, canvas is nullptr");
                return;
            }
            auto geoPtr = std::static_pointer_cast<RSObjAbsGeometry>(node.GetRenderProperties().GetBoundsGeometry());
            if (!geoPtr) {
                RS_LOGI("RSUniRenderVisitor::ProcessSurfaceRenderNode node:%llu, get geoPtr failed",
                    node.GetId());
                return;
            }
            OHOS::sptr<SurfaceBuffer> buffer;
            RSProcessor::SpecialTask task = [] () {};
            bool ret = processor_->ConsumeAndUpdateBuffer(node, task, buffer);
            auto& surfaceConsumer = node.GetConsumer();
            if (ret && surfaceConsumer != nullptr) {
                (void)surfaceConsumer->ReleaseBuffer(node.GetBuffer(), -1);
            }

            uniZOrder_ = globalZOrder_++;
            canvas_->save();
            canvas_->setMatrix(geoPtr->GetAbsMatrix());
            ProcessBaseRenderNode(node);
            canvas_->restore();
        } else {
            canvas_->save();
            canvas_->clipRect(SkRect::MakeXYWH(
                node.GetRenderProperties().GetBoundsPositionX(), node.GetRenderProperties().GetBoundsPositionY(),
                node.GetRenderProperties().GetBoundsWidth(), node.GetRenderProperties().GetBoundsHeight()));
            if (node.GetConsumer() != nullptr && node.GetBuffer() == nullptr) {
                RS_LOGI("RSUniRenderVisitor::ProcessSurfaceRenderNode buffer is not available, set black");
                canvas_->clear(SK_ColorBLACK);
            } else {
                OHOS::sptr<SurfaceBuffer> buffer;
                RSProcessor::SpecialTask task = [] () {};
                if (!processor_->ConsumeAndUpdateBuffer(node, task, buffer)) {
                    RS_LOGI("RSUniRenderVisitor::ProcessSurfaceRenderNode consume buffer fail");
                    return;
                }
                RS_LOGI("RSUniRenderVisitor::ProcessSurfaceRenderNode draw buffer on canvas");
                DrawBufferOnCanvas(node);
            }
            canvas_->restore();
        }
    } else {
        ProcessBaseRenderNode(node);
        node.SetGlobalZOrder(globalZOrder_++);
        processor_->ProcessSurface(node);
    }
    isUniRender_ = false;
}

void RSUniRenderVisitor::ProcessRootRenderNode(RSRootRenderNode& node)
{
    if (!isUniRender_ || !node.GetRenderProperties().GetVisible() || !IsChildOfSurfaceNode(node)) {
        return;
    }

    if (!canvas_) {
        RS_LOGE("RSUniRenderVisitor::ProcessRootRenderNode, canvas is nullptr");
        return;
    }

    canvas_->save();
    ProcessCanvasRenderNode(node);
    canvas_->restore();
}

void RSUniRenderVisitor::ProcessCanvasRenderNode(RSCanvasRenderNode& node)
{
    if (!node.GetRenderProperties().GetVisible()) {
        return;
    }
    if (!canvas_) {
        RS_LOGE("RSUniRenderVisitor::ProcessCanvasRenderNode, canvas is nullptr");
        return;
    }
    node.ProcessRenderBeforeChildren(*canvas_);
    ProcessBaseRenderNode(node);
    node.ProcessRenderAfterChildren(*canvas_);
}

void RSUnifiedRenderVisitor::SortZOrder(RSBaseRenderNode& node)
{
    auto& children = node.GetChildren();
    static auto compare = [](std::weak_ptr<RSBaseRenderNode> first, std::weak_ptr<RSBaseRenderNode> second) -> bool {
        auto node1 = RSBaseRenderNode::ReinterpretCast<RSRenderNode>(first.lock());
        auto node2 = RSBaseRenderNode::ReinterpretCast<RSRenderNode>(second.lock());
        if (node1 == nullptr || node2 == nullptr) {
            return false;
        }
        return node1->GetRenderProperties().GetPositionZ() < node2->GetRenderProperties().GetPositionZ();
    };
    std::stable_sort(children.begin(), children.end(), compare);
}

bool RSUniRenderVisitor::IsChildOfDisplayNode(RSBaseRenderNode& node)
{
    auto parent = node.GetParent().lock();
    return parent && parent->IsInstanceOf<RSDisplayRenderNode>();
}

bool RSUniRenderVisitor::IsChildOfSurfaceNode(RSBaseRenderNode& node)
{
    auto parent = node.GetParent().lock();
    return parent && parent->IsInstanceOf<RSSurfaceRenderNode>();
}

void RSUniRenderVisitor::DrawBufferOnCanvas(RSSurfaceRenderNode& node)
{
    auto buffer = node.GetBuffer();
    SkColorType colorType = (buffer->GetFormat() == PIXEL_FMT_BGRA_8888) ?
        kBGRA_8888_SkColorType : kRGBA_8888_SkColorType;
    SkImageInfo imageInfo = SkImageInfo::Make(buffer->GetWidth(), buffer->GetHeight(),
        colorType, kPremul_SkAlphaType);
    auto pixmap = SkPixmap(imageInfo, buffer->GetVirAddr(), buffer->GetStride());
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setAlphaf(node.GetAlpha() * node.GetRenderProperties().GetAlpha()); // ??
    SkBitmap bitmap;
    if (!bitmap.installPixels(pixmap)) {
        RS_LOGE("RSUniRenderVisitor::DrawBufferOnCanvas installPixels failed");
    }
    canvas_->save();
    const RSProperties& property = node.GetRenderProperties();
    auto filter = std::static_pointer_cast<RSSkiaFilter>(property.GetBackgroundFilter());
    if (filter != nullptr) {
        auto skRectPtr = std::make_unique<SkRect>();
        skRectPtr->setXYWH(node.GetRenderProperties().GetBoundsPositionX(), node.GetRenderProperties().GetBoundsPositionY(),
            node.GetRenderProperties().GetBoundsWidth(), node.GetRenderProperties().GetBoundsHeight());
        RSPropertiesPainter::SaveLayerForFilter(property, *canvas_, filter, skRectPtr);
        RSPropertiesPainter::RestoreForFilter(*canvas_);
    }
    canvas_->drawBitmapRect(bitmap,
        SkRect::MakeXYWH(0, 0,buffer->GetSurfaceBufferWidth(), buffer->GetSurfaceBufferHeight()),
        SkRect::MakeXYWH(node.GetRenderProperties().GetBoundsPositionX(), node.GetRenderProperties().GetBoundsPositionY(),
            node.GetRenderProperties().GetBoundsWidth(), node.GetRenderProperties().GetBoundsHeight()), &paint);
    canvas_->restore();
}
} // namespace Rosen
} // namespace OHOS


