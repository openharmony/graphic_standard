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

#include "pipeline/rs_draw_cmd_list.h"

#include <unordered_map>

#include "pipeline/rs_draw_cmd.h"
#include "pipeline/rs_paint_filter_canvas.h"
#include "platform/common/rs_log.h"
#include "transaction/rs_marshalling_helper.h"

namespace OHOS {
namespace Rosen {
#ifdef ROSEN_OHOS
using OpUnmarshallingFunc = OpItem* (*)(Parcel& parcel);

static std::unordered_map<RSOpType, OpUnmarshallingFunc> opUnmarshallingFuncLUT = {
    { RECT_OPITEM,                 RectOpItem::Unmarshalling },
    { ROUND_RECT_OPITEM,           RoundRectOpItem::Unmarshalling },
    { IMAGE_WITH_PARM_OPITEM,      ImageWithParmOpItem::Unmarshalling },
    { DRRECT_OPITEM,               DRRectOpItem::Unmarshalling },
    { OVAL_OPITEM,                 OvalOpItem::Unmarshalling },
    { REGION_OPITEM,               RegionOpItem::Unmarshalling },
    { ARC_OPITEM,                  ArcOpItem::Unmarshalling },
    { SAVE_OPITEM,                 SaveOpItem::Unmarshalling },
    { RESTORE_OPITEM,              RestoreOpItem::Unmarshalling },
    { FLUSH_OPITEM,                FlushOpItem::Unmarshalling },
    { MATRIX_OPITEM,               MatrixOpItem::Unmarshalling },
    { CLIP_RECT_OPITEM,            ClipRectOpItem::Unmarshalling },
    { CLIP_RRECT_OPITEM,           ClipRRectOpItem::Unmarshalling },
    { CLIP_REGION_OPITEM,          ClipRegionOpItem::Unmarshalling },
    { TRANSLATE_OPITEM,            TranslateOpItem::Unmarshalling },
    { TEXTBLOB_OPITEM,             TextBlobOpItem::Unmarshalling },
    { BITMAP_OPITEM,               BitmapOpItem::Unmarshalling },
    { BITMAP_RECT_OPITEM,          BitmapRectOpItem::Unmarshalling },
    { BITMAP_NINE_OPITEM,          BitmapNineOpItem::Unmarshalling },
    { ADAPTIVE_RRECT_OPITEM,       AdaptiveRRectOpItem::Unmarshalling },
    { CLIP_ADAPTIVE_RRECT_OPITEM,  ClipAdaptiveRRectOpItem::Unmarshalling },
    { PATH_OPITEM,                 PathOpItem::Unmarshalling },
    { CLIP_PATH_OPITEM,            ClipPathOpItem::Unmarshalling },
    { PAINT_OPITEM,                PaintOpItem::Unmarshalling },
    { CONCAT_OPITEM,               ConcatOpItem::Unmarshalling },
    { SAVE_LAYER_OPITEM,           SaveLayerOpItem::Unmarshalling },
    { DRAWABLE_OPITEM,             DrawableOpItem::Unmarshalling },
    { PICTURE_OPITEM,              PictureOpItem::Unmarshalling },
    { POINTS_OPITEM,               PointsOpItem::Unmarshalling },
    { VERTICES_OPITEM,             VerticesOpItem::Unmarshalling },
    { MULTIPLY_ALPHA_OPITEM,       MultiplyAlphaOpItem::Unmarshalling },
    { SAVE_ALPHA_OPITEM,           SaveAlphaOpItem::Unmarshalling },
    { RESTORE_ALPHA_OPITEM,        RestoreAlphaOpItem::Unmarshalling },
};

static OpUnmarshallingFunc GetOpUnmarshallingFunc(RSOpType type)
{
    auto it = opUnmarshallingFuncLUT.find(type);
    if (it == opUnmarshallingFuncLUT.end()) {
        return nullptr;
    }
    return it->second;
}
#endif

DrawCmdList::DrawCmdList(int w, int h) : width_(w), height_(h) {}

DrawCmdList::~DrawCmdList()
{
    ClearOp();
}

void DrawCmdList::AddOp(std::unique_ptr<OpItem>&& op)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ops_.push_back(std::move(op));
}

void DrawCmdList::ClearOp()
{
    ops_.clear();
}

DrawCmdList& DrawCmdList::operator=(DrawCmdList&& that)
{
    ops_.swap(that.ops_);
    return *this;
}

void DrawCmdList::Playback(SkCanvas& canvas, const SkRect* rect) const
{
    RSPaintFilterCanvas filterCanvas(&canvas);
    Playback(filterCanvas, rect);
}

void DrawCmdList::Playback(RSPaintFilterCanvas& canvas, const SkRect* rect) const
{
#ifdef ROSEN_OHOS
    if (width_ <= 0 || height_ <= 0) {
        return;
    }
    for (auto& it : ops_) {
        if (it == nullptr) {
            continue;
        }
        it->Draw(canvas, rect);
    }
#endif
}

int DrawCmdList::GetSize() const
{
    return ops_.size();
}

int DrawCmdList::GetWidth() const
{
    return width_;
}

int DrawCmdList::GetHeight() const
{
    return height_;
}

#ifdef ROSEN_OHOS
bool DrawCmdList::Marshalling(Parcel& parcel) const
{
    bool success = true;
    success &= RSMarshallingHelper::Marshalling(parcel, width_);
    success &= RSMarshallingHelper::Marshalling(parcel, height_);
    success &= RSMarshallingHelper::Marshalling(parcel, GetSize());
    ROSEN_LOGD("unirender: DrawCmdList::Marshalling start, size = %d", GetSize());
    for (const auto& item : ops_) {
        auto type = item->GetType();
        success &= RSMarshallingHelper::Marshalling(parcel, type);
        success &= item->Marshalling(parcel);
        if (!success) {
            ROSEN_LOGE("unirender: failed opItem Marshalling, optype = %d, UnmarshallingFunc define = %d",
                       type, GetOpUnmarshallingFunc(type) != nullptr);
            return success;
        }
    }
    return success;
}

DrawCmdList* DrawCmdList::Unmarshalling(Parcel& parcel)
{
    int width;
    int height;
    int size;
    if (!RSMarshallingHelper::Unmarshalling(parcel, width)) {
        return nullptr;
    }
    if (!RSMarshallingHelper::Unmarshalling(parcel, height)) {
        return nullptr;
    }
    if (!RSMarshallingHelper::Unmarshalling(parcel, size)) {
        return nullptr;
    }

    ROSEN_LOGD("unirender: DrawCmdList::Unmarshalling start, size = %d", size);
    std::unique_ptr<DrawCmdList> drawCmdList = std::make_unique<DrawCmdList>(width, height);
    for (int i = 0; i < size; ++i) {
        RSOpType type;
        if (!RSMarshallingHelper::Unmarshalling(parcel, type)) {
            return nullptr;
        }
        auto func = GetOpUnmarshallingFunc(type);
        if (!func) {
            ROSEN_LOGW("unirender: opItem Unmarshalling func not define, optype = %d", type);
            continue;
        }

        OpItem* item = (*func)(parcel);
        if (!item) {
            ROSEN_LOGE("unirender: failed opItem Unmarshalling, optype = %d", type);
            return nullptr;
        }

        drawCmdList->AddOp(std::unique_ptr<OpItem>(item));
    }
    ROSEN_LOGD("unirender: DrawCmdList::Unmarshalling success, size = %d", drawCmdList->GetSize());

    return drawCmdList.release();
}
#endif
} // namespace Rosen
} // namespace OHOS
