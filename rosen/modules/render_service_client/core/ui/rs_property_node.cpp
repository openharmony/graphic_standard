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

#include "ui/rs_property_node.h"

#include <algorithm>
#include <string>

#include "animation/rs_animation.h"
#include "animation/rs_curve_animation.h"
#include "animation/rs_implicit_animator.h"
#include "command/rs_property_node_command.h"
#include "common/rs_color.h"
#include "common/rs_obj_geometry.h"
#include "pipeline/rs_node_map.h"
#include "platform/common/rs_log.h"
#include "render/rs_path.h"
#include "transaction/rs_transaction_proxy.h"

namespace OHOS {
namespace Rosen {

RSPropertyNode::RSPropertyNode(bool isRenderServiceNode) : RSBaseNode(isRenderServiceNode), stagingProperties_(false) {}

RSPropertyNode::~RSPropertyNode()
{
    FallbackAnimationsToRoot();
}

void RSPropertyNode::OpenImplicitAnimation(const RSAnimationTimingProtocol& timingProtocol,
    const RSAnimationTimingCurve& timingCurve, const std::function<void()>& finishCallback)
{
    RSImplicitAnimator::Instance().OpenImplicitAnimation(timingProtocol, timingCurve, finishCallback);
}

std::vector<std::shared_ptr<RSAnimation>> RSPropertyNode::CloseImplicitAnimation()
{
    return RSImplicitAnimator::Instance().CloseImplicitAnimation();
}

void RSPropertyNode::AddKeyFrame(
    float fraction, const RSAnimationTimingCurve& timingCurve, const PropertyCallback& propertyCallback)
{
    if (propertyCallback == nullptr) {
        ROSEN_LOGE("Failed to add keyframe animation, property callback is null!");
        return;
    }

    RSImplicitAnimator::Instance().BeginImplicitKeyFrameAnimation(fraction, timingCurve);
    propertyCallback();
    RSImplicitAnimator::Instance().EndImplicitKeyFrameAnimation();
}

void RSPropertyNode::AddKeyFrame(float fraction, const PropertyCallback& propertyCallback)
{
    if (propertyCallback == nullptr) {
        ROSEN_LOGE("Failed to add keyframe animation, property callback is null!");
        return;
    }

    RSImplicitAnimator::Instance().BeginImplicitKeyFrameAnimation(fraction);
    propertyCallback();
    RSImplicitAnimator::Instance().EndImplicitKeyFrameAnimation();
}

std::vector<std::shared_ptr<RSAnimation>> RSPropertyNode::Animate(const RSAnimationTimingProtocol& timingProtocol,
    const RSAnimationTimingCurve& timingCurve, const PropertyCallback& propertyCallback,
    const std::function<void()>& finshCallback)
{
    if (propertyCallback == nullptr) {
        ROSEN_LOGE("Failed to add curve animation, property callback is null!");
        return {};
    }
    OpenImplicitAnimation(timingProtocol, timingCurve, finshCallback);
    propertyCallback();
    return CloseImplicitAnimation();
}

void RSPropertyNode::FallbackAnimationsToRoot()
{
    auto target = RSNodeMap::Instance().GetAnimationFallbackNode();

    if (target == nullptr) {
        ROSEN_LOGE("Failed to move animation to root, root node is null!");
        return;
    }
    for (const auto& [animationId, animation] : animations_) {
        target->AddAnimationInner(animation);
    }
}

void RSPropertyNode::AddAnimationInner(const std::shared_ptr<RSAnimation>& animation)
{
    animations_[animation->GetId()] = animation;
    animatingPropertyNum_[animation->GetProperty()]++;
}

void RSPropertyNode::RemoveAnimationInner(const std::shared_ptr<RSAnimation>& animation)
{
    auto animationItr = animations_.find(animation->GetId());
    animations_.erase(animationItr);
    animatingPropertyNum_[animation->GetProperty()]--;
}

void RSPropertyNode::FinishAnimationByProperty(const RSAnimatableProperty& property)
{
    for (const auto& [animationId, animation] : animations_) {
        if (animation->GetProperty() == property) {
            animation->Finish();
        }
    }
}

const RSProperties& RSPropertyNode::GetStagingProperties() const
{
    return stagingProperties_;
}

void RSPropertyNode::AddAnimation(const std::shared_ptr<RSAnimation>& animation)
{
    if (animation == nullptr) {
        ROSEN_LOGE("Failed to add animation, animation is null!");
        return;
    }

    auto animationId = animation->GetId();
    if (animations_.find(animationId) != animations_.end()) {
        ROSEN_LOGE("Failed to add animation, animation already exists!");
        return;
    }

    if (animation->GetDuration() <= 0) {
        FinishAnimationByProperty(animation->GetProperty());
    }

    AddAnimationInner(animation);
    animation->StartInner(shared_from_this());
}

void RSPropertyNode::RemoveAllAnimations()
{
    for (const auto& [id, animation] : animations_) {
        RemoveAnimation(animation);
    }
}

void RSPropertyNode::RemoveAnimation(const std::shared_ptr<RSAnimation>& animation)
{
    if (animation == nullptr) {
        ROSEN_LOGE("Failed to remove animation, animation is null!");
        return;
    }

    if (animations_.find(animation->GetId()) == animations_.end()) {
        ROSEN_LOGE("Failed to remove animation, animation not exists!");
        return;
    }

    animation->Finish();
}

void RSPropertyNode::SetMotionPathOption(const std::shared_ptr<RSMotionPathOption>& motionPathOption)
{
    motionPathOption_ = motionPathOption;
}

const std::shared_ptr<RSMotionPathOption> RSPropertyNode::GetMotionPathOption() const
{
    return motionPathOption_;
}

bool RSPropertyNode::HasPropertyAnimation(const RSAnimatableProperty& property)
{
    auto numIt = animatingPropertyNum_.find(property);
    if (numIt != animatingPropertyNum_.end() && numIt->second > 0) {
        return true;
    }

    return false;
}

void RSPropertyNode::SetBounds(float positionX, float positionY, float width, float height)
{
    SetBoundsPosition({ positionX, positionY });
    SetBoundsSize(width, height);
}

void RSPropertyNode::SetBoundsSize(float width, float height)
{
    SetBoundsWidth(width);
    SetBoundsHeight(height);
}

namespace {
template<typename T>
bool IsValid(const T& value)
{
    return true;
}
template<>
bool IsValid(const float& value)
{
    return !isinf(value);
}
template<>
bool IsValid(const Vector2f& value)
{
    return !value.IsInfinite();
}
} // namespace

#define SET_ANIMATABLE_PROPERTY(propertyName, value, property)                                                  \
    auto currentValue = stagingProperties_.Get##propertyName();                                                 \
    if (ROSEN_EQ(value, currentValue)) {                                                                        \
        return;                                                                                                 \
    }                                                                                                           \
    if (RSImplicitAnimator::Instance().NeedImplicitAnimaton() && IsValid(currentValue)) {                       \
        RSImplicitAnimator::Instance().CreateImplicitAnimation(*this, property, currentValue, value);           \
    } else if (HasPropertyAnimation(property)) {                                                                \
        std::unique_ptr<RSCommand> command =                                                                    \
            std::make_unique<RSPropertyNodeSet##propertyName##Delta>(GetId(), value - currentValue);            \
        RSTransactionProxy::GetInstance().AddCommand(command, IsRenderServiceNode());                           \
        stagingProperties_.Set##propertyName(value);                                                            \
    } else {                                                                                                    \
        std::unique_ptr<RSCommand> command = std::make_unique<RSPropertyNodeSet##propertyName>(GetId(), value); \
        RSTransactionProxy::GetInstance().AddCommand(command, IsRenderServiceNode());                           \
        stagingProperties_.Set##propertyName(value);                                                            \
    }

#define SET_NONANIMATABLE_PROPERTY(propertyName, value)                                                     \
    auto currentValue = stagingProperties_.Get##propertyName();                                             \
    if (ROSEN_EQ(value, currentValue)) {                                                                    \
        return;                                                                                             \
    }                                                                                                       \
    std::unique_ptr<RSCommand> command = std::make_unique<RSPropertyNodeSet##propertyName>(GetId(), value); \
    RSTransactionProxy::GetInstance().AddCommand(command, IsRenderServiceNode());                           \
    stagingProperties_.Set##propertyName(value);

void RSPropertyNode::SetBoundsWidth(float width)
{
    SET_ANIMATABLE_PROPERTY(BoundsWidth, width, RSAnimatableProperty::BOUNDS_WIDTH)
}

void RSPropertyNode::SetBoundsHeight(float height)
{
    SET_ANIMATABLE_PROPERTY(BoundsHeight, height, RSAnimatableProperty::BOUNDS_HEIGHT)
}

void RSPropertyNode::SetBoundsPosition(const Vector2f& boundsPosition)
{
    SET_ANIMATABLE_PROPERTY(BoundsPosition, boundsPosition, RSAnimatableProperty::BOUNDS_POSITION)
}

void RSPropertyNode::SetBoundsPositionX(float positionX)
{
    SET_ANIMATABLE_PROPERTY(BoundsPositionX, positionX, RSAnimatableProperty::BOUNDS_POSITION_X)
}

void RSPropertyNode::SetBoundsPositionY(float positionY)
{
    SET_ANIMATABLE_PROPERTY(BoundsPositionY, positionY, RSAnimatableProperty::BOUNDS_POSITION_Y)
}

void RSPropertyNode::SetFrame(float positionX, float positionY, float width, float height)
{
    SetFramePosition({ positionX, positionY });
    SetFrameSize(width, height);
}

void RSPropertyNode::SetFrameSize(float width, float height)
{
    SetFrameWidth(width);
    SetFrameHeight(height);
}

void RSPropertyNode::SetFrameWidth(float width)
{
    SET_ANIMATABLE_PROPERTY(FrameWidth, width, RSAnimatableProperty::FRAME_WIDTH)
}

void RSPropertyNode::SetFrameHeight(float height)
{
    SET_ANIMATABLE_PROPERTY(FrameHeight, height, RSAnimatableProperty::FRAME_HEIGHT)
}

void RSPropertyNode::SetFramePosition(const Vector2f& framePosition)
{
    SET_ANIMATABLE_PROPERTY(FramePosition, framePosition, RSAnimatableProperty::FRAME_POSITION)
}

void RSPropertyNode::SetFramePositionX(float positionX)
{
    SET_ANIMATABLE_PROPERTY(FramePositionX, positionX, RSAnimatableProperty::FRAME_POSITION_X)
}

void RSPropertyNode::SetFramePositionY(float positionY)
{
    SET_ANIMATABLE_PROPERTY(FramePositionY, positionY, RSAnimatableProperty::FRAME_POSITION_Y)
}

void RSPropertyNode::SetPositionZ(float positionZ)
{
    SET_ANIMATABLE_PROPERTY(PositionZ, positionZ, RSAnimatableProperty::POSITION_Z);
}

void RSPropertyNode::SetPivot(float pivotX, float pivotY)
{
    SetPivotX(pivotX);
    SetPivotY(pivotY);
}

void RSPropertyNode::SetPivotX(float pivotX)
{
    SET_ANIMATABLE_PROPERTY(PivotX, pivotX, RSAnimatableProperty::PIVOT_X);
}

void RSPropertyNode::SetPivotY(float pivotY)
{
    SET_ANIMATABLE_PROPERTY(PivotY, pivotY, RSAnimatableProperty::PIVOT_Y);
}

void RSPropertyNode::SetCornerRadius(float cornerRadius)
{
    SET_ANIMATABLE_PROPERTY(CornerRadius, cornerRadius, RSAnimatableProperty::CORNER_RADIUS);
}

void RSPropertyNode::SetRotation(const Vector4f& quaternion)
{
    SET_ANIMATABLE_PROPERTY(Quaternion, quaternion, RSAnimatableProperty::ROTATION_3D);
}

void RSPropertyNode::SetRotation(float degree)
{
    SET_ANIMATABLE_PROPERTY(Rotation, degree, RSAnimatableProperty::ROTATION);
}

void RSPropertyNode::SetRotation(float degreeX, float degreeY, float degreeZ)
{
    SetRotationX(degreeX);
    SetRotationY(degreeY);
    SetRotation(degreeZ);
}

void RSPropertyNode::SetRotationX(float degree)
{
    SET_ANIMATABLE_PROPERTY(RotationX, degree, RSAnimatableProperty::ROTATION_X);
}

void RSPropertyNode::SetRotationY(float degree)
{
    SET_ANIMATABLE_PROPERTY(RotationY, degree, RSAnimatableProperty::ROTATION_Y);
}

void RSPropertyNode::SetTranslate(const Vector2f& translate)
{
    SET_ANIMATABLE_PROPERTY(Translate, translate, RSAnimatableProperty::TRANSLATE);
}

void RSPropertyNode::SetTranslate(float translateX, float translateY, float translateZ)
{
    SetTranslate({ translateX, translateY });
    SetTranslateZ(translateZ);
}

void RSPropertyNode::SetTranslateX(float translate)
{
    SET_ANIMATABLE_PROPERTY(TranslateX, translate, RSAnimatableProperty::TRANSLATE_X);
}

void RSPropertyNode::SetTranslateY(float translate)
{
    SET_ANIMATABLE_PROPERTY(TranslateY, translate, RSAnimatableProperty::TRANSLATE_Y);
}

void RSPropertyNode::SetTranslateZ(float translate)
{
    SET_ANIMATABLE_PROPERTY(TranslateZ, translate, RSAnimatableProperty::TRANSLATE_Z);
}

void RSPropertyNode::SetScale(float scale)
{
    SetScaleX(scale);
    SetScaleY(scale);
}

void RSPropertyNode::SetScale(float scaleX, float scaleY)
{
    SetScaleX(scaleX);
    SetScaleY(scaleY);
}

void RSPropertyNode::SetScaleX(float scale)
{
    SET_ANIMATABLE_PROPERTY(ScaleX, scale, RSAnimatableProperty::SCALE_X);
}

void RSPropertyNode::SetScaleY(float scale)
{
    SET_ANIMATABLE_PROPERTY(ScaleY, scale, RSAnimatableProperty::SCALE_Y);
}

void RSPropertyNode::SetAlpha(float alpha)
{
    SET_ANIMATABLE_PROPERTY(Alpha, alpha, RSAnimatableProperty::ALPHA);
}

void RSPropertyNode::SetForegroundColor(uint32_t colorValue)
{
    SET_ANIMATABLE_PROPERTY(ForegroundColor, Color::FromArgbInt(colorValue), RSAnimatableProperty::FOREGROUND_COLOR);
}

void RSPropertyNode::SetBackgroundColor(uint32_t colorValue)
{
    SET_ANIMATABLE_PROPERTY(BackgroundColor, Color::FromArgbInt(colorValue), RSAnimatableProperty::BACKGROUND_COLOR);
}

void RSPropertyNode::SetBackgroundShader(std::shared_ptr<RSShader> shader)
{
    SET_NONANIMATABLE_PROPERTY(BackgroundShader, shader)
}

void RSPropertyNode::SetBgImage(std::shared_ptr<RSImage> image)
{
    SET_NONANIMATABLE_PROPERTY(BgImage, image)
}

void RSPropertyNode::SetBgImageSize(float width, float height)
{
    SetBgImageWidth(width);
    SetBgImageHeight(height);
}

void RSPropertyNode::SetBgImageWidth(float width)
{
    SET_ANIMATABLE_PROPERTY(BgImageWidth, width, RSAnimatableProperty::BGIMAGE_WIDTH);
}

void RSPropertyNode::SetBgImageHeight(float height)
{
    SET_ANIMATABLE_PROPERTY(BgImageHeight, height, RSAnimatableProperty::BGIMAGE_HEIGHT);
}

void RSPropertyNode::SetBgImagePosition(float positionX, float positionY)
{
    SetBgImagePositionX(positionX);
    SetBgImagePositionY(positionY);
}

void RSPropertyNode::SetBgImagePositionX(float positionX)
{
    SET_ANIMATABLE_PROPERTY(BgImagePositionX, positionX, RSAnimatableProperty::BGIMAGE_POSITION_X);
}

void RSPropertyNode::SetBgImagePositionY(float positionY)
{
    SET_ANIMATABLE_PROPERTY(BgImagePositionY, positionY, RSAnimatableProperty::BGIMAGE_POSITION_Y);
}

void RSPropertyNode::SetBorderColor(uint32_t colorValue)
{
    SET_ANIMATABLE_PROPERTY(BorderColor, Color::FromArgbInt(colorValue), RSAnimatableProperty::BORDER_COLOR);
}

void RSPropertyNode::SetBorderWidth(float width)
{
    SET_ANIMATABLE_PROPERTY(BorderWidth, width, RSAnimatableProperty::BORDER_WIDTH);
}

void RSPropertyNode::SetBorderStyle(uint32_t styleValue)
{
    BorderStyle style = static_cast<BorderStyle>(styleValue);
    SET_NONANIMATABLE_PROPERTY(BorderStyle, style)
}

void RSPropertyNode::SetSublayerTransform(Matrix3f sublayerTransform)
{
    SET_ANIMATABLE_PROPERTY(SublayerTransform, sublayerTransform, RSAnimatableProperty::SUB_LAYER_TRANSFORM);
}

void RSPropertyNode::SetBackgroundFilter(std::shared_ptr<RSFilter> backgroundFilter)
{
    SET_ANIMATABLE_PROPERTY(BackgroundFilter, backgroundFilter, RSAnimatableProperty::BACKGROUND_FILTER);
}

void RSPropertyNode::SetFilter(std::shared_ptr<RSFilter> filter)
{
    SET_ANIMATABLE_PROPERTY(Filter, filter, RSAnimatableProperty::FILTER);
}

void RSPropertyNode::SetCompositingFilter(std::shared_ptr<RSFilter> compositingFilter) {}

void RSPropertyNode::SetShadowColor(uint32_t colorValue)
{
    SET_ANIMATABLE_PROPERTY(ShadowColor, Color::FromArgbInt(colorValue), RSAnimatableProperty::SHADOW_COLOR);
}

void RSPropertyNode::SetShadowOffset(float offsetX, float offsetY)
{
    SetShadowOffsetX(offsetX);
    SetShadowOffsetY(offsetY);
}

void RSPropertyNode::SetShadowOffsetX(float offsetX)
{
    SET_ANIMATABLE_PROPERTY(ShadowOffsetX, offsetX, RSAnimatableProperty::SHADOW_OFFSET_X);
}

void RSPropertyNode::SetShadowOffsetY(float offsetY)
{
    SET_ANIMATABLE_PROPERTY(ShadowOffsetY, offsetY, RSAnimatableProperty::SHADOW_OFFSET_Y);
}

void RSPropertyNode::SetShadowAlpha(float alpha)
{
    SET_ANIMATABLE_PROPERTY(ShadowAlpha, alpha, RSAnimatableProperty::SHADOW_ALPHA);
}

void RSPropertyNode::SetShadowElevation(float elevation)
{
    SET_ANIMATABLE_PROPERTY(ShadowElevation, elevation, RSAnimatableProperty::SHADOW_ELEVATION);
}

void RSPropertyNode::SetShadowRadius(float radius)
{
    SET_ANIMATABLE_PROPERTY(ShadowRadius, radius, RSAnimatableProperty::SHADOW_RADIUS);
}

void RSPropertyNode::SetShadowPath(std::shared_ptr<RSPath> shadowpath)
{
    SET_NONANIMATABLE_PROPERTY(ShadowPath, shadowpath)
}

void RSPropertyNode::SetFrameGravity(Gravity gravity)
{
    SET_NONANIMATABLE_PROPERTY(FrameGravity, gravity)
}

void RSPropertyNode::SetClipBounds(std::shared_ptr<RSPath> path)
{
    SET_NONANIMATABLE_PROPERTY(ClipBounds, path)
}

void RSPropertyNode::SetClipToBounds(bool clipToBounds)
{
    SET_NONANIMATABLE_PROPERTY(ClipToBounds, clipToBounds)
}

void RSPropertyNode::SetClipToFrame(bool clipToFrame)
{
    SET_NONANIMATABLE_PROPERTY(ClipToFrame, clipToFrame)
}

void RSPropertyNode::SetVisible(bool visible)
{
    SET_NONANIMATABLE_PROPERTY(Visible, visible)
    auto type = visible ? RSTransitionEffectType::FADE_IN : RSTransitionEffectType::FADE_OUT;
    NotifyTransition({ RSTransitionEffect(type) }, GetId());
}

void RSPropertyNode::NotifyTransition(const std::vector<RSTransitionEffect> effects, NodeId nodeId)
{
    if (!RSImplicitAnimator::Instance().NeedImplicitAnimaton() || effects.empty()) {
        return;
    }
    auto node = std::static_pointer_cast<RSPropertyNode>(RSNodeMap::Instance().GetNode(nodeId).lock());
    if (node == nullptr) {
        return;
    }
    for (auto effect : effects) {
        if (effect.GetType() == RSTransitionEffectType::UNDEFINED) {
            continue;
        }
        RSImplicitAnimator::Instance().BeginImplicitTransition(effect);
        RSImplicitAnimator::Instance().CreateImplicitTransition(*(node.get()));
        RSImplicitAnimator::Instance().EndImplicitTransition();
    }
}

void RSPropertyNode::OnAddChildren()
{
    NotifyTransition({ RSTransitionEffect(RSTransitionEffectType::FADE_IN) }, GetId());
}

void RSPropertyNode::OnRemoveChildren()
{
    NotifyTransition({ RSTransitionEffect(RSTransitionEffectType::FADE_OUT) }, GetId());
}

void RSPropertyNode::AnimationFinish(long long animationId)
{
    auto animationItr = animations_.find(animationId);
    if (animationItr == animations_.end()) {
        ROSEN_LOGE("Failed to find animation[%llu]!", animationId);
        return;
    }

    auto animation = animationItr->second;
    if (animation == nullptr) {
        ROSEN_LOGE("Failed to finish animation[%llu], animation is null!", animationId);
        return;
    }

    animation->CallFinishCallback();
    RemoveAnimationInner(animation);
}
} // namespace Rosen
} // namespace OHOS
