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

#include "animation/rs_implicit_animator.h"

#include "animation/rs_animation_callback.h"
#include "animation/rs_path_animation.h"
#include "pipeline/rs_node_map.h"

namespace OHOS {
namespace Rosen {
namespace {
static constexpr int TIMING_PROTOCOL_INDEX = 0;
static constexpr int TIMING_CURVE_INDEX = 1;
static constexpr int FINISH_INDEX = 2;
} // namespace

void RSImplicitAnimator::OpenImplicitAnimation(const RSAnimationTimingProtocol& timingProtocol,
    const RSAnimationTimingCurve& timingCurve, const std::function<void()>& finishCallback)
{
    globalImplicitParams_.push({ timingProtocol, timingCurve, finishCallback });
    implicitAnimations_.push({});
    keyframeAnimations_.push({});
    BeginImplicitCurveAnimation();
}

std::vector<std::shared_ptr<RSAnimation>> RSImplicitAnimator::CloseImplicitAnimation()
{
    std::vector<std::pair<std::shared_ptr<RSAnimation>, NodeId>> currentAnimations;
    if (globalImplicitParams_.empty() || implicitAnimations_.empty() || keyframeAnimations_.empty()) {
        ROSEN_LOGE("Failed to close implicit animation, need to open implicit animation firstly!");
        return {};
    }

    auto finishCallback = std::get<FINISH_INDEX>(globalImplicitParams_.top());
    if (implicitAnimations_.top().empty()) {
        ROSEN_LOGD("No implicit animations created!");
        if (finishCallback == nullptr) {
            globalImplicitParams_.pop();
            implicitAnimations_.pop();
            keyframeAnimations_.pop();
            EndImplicitCurveAnimation();
            return {};
        } else {
            CreateEmptyAnimation();
        }
    }
    currentAnimations = implicitAnimations_.top();

    for (const auto& [animationInfo, keyframeAnimation] : keyframeAnimations_.top()) {
        auto target = RSNodeMap::Instance().GetNode<RSNode>(animationInfo.first);
        if (target == nullptr) {
            ROSEN_LOGE(
                "Failed to start implicit keyframe animation[%llu], target is null!", keyframeAnimation->GetId());
            continue;
        }

        target->AddAnimation(keyframeAnimation);
    }

    std::shared_ptr<AnimationFinishCallback> animationFinishCallback;
    if (finishCallback != nullptr) {
        animationFinishCallback = std::make_shared<AnimationFinishCallback>(finishCallback);
    }

    std::vector<std::shared_ptr<RSAnimation>> resultAnimations;
    for (const auto& [animation, nodeId] : currentAnimations) {
        if (animation == nullptr) {
            continue;
        }

        if (animationFinishCallback != nullptr) {
            animation->SetFinishCallback(animationFinishCallback);
        }

        resultAnimations.emplace_back(animation);
    }

    globalImplicitParams_.pop();
    implicitAnimations_.pop();
    keyframeAnimations_.pop();
    EndImplicitCurveAnimation();
    return resultAnimations;
}

void RSImplicitAnimator::BeginImplicitKeyFrameAnimation(float fraction, const RSAnimationTimingCurve& timingCurve)
{
    if (globalImplicitParams_.empty()) {
        ROSEN_LOGE("Failed to begin keyframe implicit animation, need to open implicit animation firstly!");
        return;
    }

    auto paramsTuple = globalImplicitParams_.top();
    auto keyframeAnimationParam = std::make_shared<RSImplicitKeyframeAnimationParam>(
        std::get<TIMING_PROTOCOL_INDEX>(paramsTuple), timingCurve, fraction);
    PushImplicitParam(keyframeAnimationParam);
}

void RSImplicitAnimator::BeginImplicitKeyFrameAnimation(float fraction)
{
    if (globalImplicitParams_.empty()) {
        ROSEN_LOGE("Failed to begin keyframe implicit animation, need to open implicit animation firstly!");
        return;
    }

    BeginImplicitKeyFrameAnimation(fraction, std::get<TIMING_CURVE_INDEX>(globalImplicitParams_.top()));
}

void RSImplicitAnimator::EndImplicitKeyFrameAnimation()
{
    if (implicitAnimationParams_.empty() ||
        implicitAnimationParams_.top()->GetType() != ImplicitAnimationParamType::KEYFRAME) {
        ROSEN_LOGE("Failed to end keyframe implicit animation, need to begin keyframe implicit animation firstly!");
        return;
    }

    PopImplicitParam();
}

bool RSImplicitAnimator::NeedImplicitAnimation()
{
    return !implicitAnimationParams_.empty();
}

void RSImplicitAnimator::BeginImplicitCurveAnimation()
{
    if (globalImplicitParams_.empty()) {
        ROSEN_LOGE("Failed to begin curve implicit animation, need to open implicit animation firstly!");
        return;
    }

    [[maybe_unused]] auto& [protocal, curve, unused] = globalImplicitParams_.top();
    auto curveAnimationParam = std::make_shared<RSImplicitCurveAnimationParam>(protocal, curve);
    PushImplicitParam(curveAnimationParam);
}

void RSImplicitAnimator::EndImplicitCurveAnimation()
{
    if (implicitAnimationParams_.empty() ||
        implicitAnimationParams_.top()->GetType() != ImplicitAnimationParamType::CURVE) {
        ROSEN_LOGE("Failed to end curve implicit animation, need to begin curve implicit animation firstly!");
        return;
    }

    PopImplicitParam();
}

void RSImplicitAnimator::BeginImplicitPathAnimation(const std::shared_ptr<RSMotionPathOption>& motionPathOption)
{
    if (globalImplicitParams_.empty()) {
        ROSEN_LOGE("Failed to begin path implicit animation, need to open implicit animation firstly!");
        return;
    }

    [[maybe_unused]] auto& [protocal, curve, unused] = globalImplicitParams_.top();
    auto pathAnimationParam = std::make_shared<RSImplicitPathAnimationParam>(protocal, curve, motionPathOption);
    PushImplicitParam(pathAnimationParam);
}

void RSImplicitAnimator::EndImplicitPathAnimation()
{
    if (implicitAnimationParams_.empty() ||
        implicitAnimationParams_.top()->GetType() != ImplicitAnimationParamType::PATH) {
        ROSEN_LOGE("Failed to end path implicit animation, need to begin path implicit animation firstly!");
        return;
    }

    PopImplicitParam();
}

void RSImplicitAnimator::BeginImplicitTransition(const std::shared_ptr<const RSTransitionEffect>& effect)
{
    if (globalImplicitParams_.empty()) {
        ROSEN_LOGE("Failed to begin implicit transition, need to open implicit transition firstly!");
        return;
    }

    [[maybe_unused]] auto& [protocal, curve, unused] = globalImplicitParams_.top();
    auto transitionParam = std::make_shared<RSImplicitTransitionParam>(protocal, curve, effect);
    PushImplicitParam(transitionParam);
}

void RSImplicitAnimator::EndImplicitTransition()
{
    if (implicitAnimationParams_.empty() ||
        implicitAnimationParams_.top()->GetType() != ImplicitAnimationParamType::TRANSITION) {
        ROSEN_LOGE("Failed to end implicit transition, need to begin implicit transition firstly!");
        return;
    }

    PopImplicitParam();
}

void RSImplicitAnimator::PushImplicitParam(const std::shared_ptr<RSImplicitAnimationParam>& implicitParam)
{
    implicitAnimationParams_.emplace(implicitParam);
}

void RSImplicitAnimator::PopImplicitParam()
{
    if (implicitAnimationParams_.empty()) {
        ROSEN_LOGE("Failed to pop implicit params, params stack is empty!");
        return;
    }

    implicitAnimationParams_.pop();
}

std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitTransition(RSNode& target, bool isTransitionIn)
{
    if (globalImplicitParams_.empty() || implicitAnimations_.empty() || keyframeAnimations_.empty()) {
        ROSEN_LOGE("Failed to create implicit transition, need to open implicit transition firstly!");
        return {};
    }
    std::shared_ptr<RSAnimation> transition = nullptr;
    auto params = implicitAnimationParams_.top();
    switch (params->GetType()) {
        case ImplicitAnimationParamType::TRANSITION: {
            auto transitionImplicitParam = std::static_pointer_cast<RSImplicitTransitionParam>(params);
            transition = transitionImplicitParam->CreateAnimation(isTransitionIn);
            break;
        }
        default:
            break;
    }
    if (transition != nullptr) {
        target.AddAnimation(transition);
        implicitAnimations_.top().push_back({ transition, target.GetId() });
    }
    return transition;
}

void RSImplicitAnimator::CreateEmptyAnimation()
{
    auto target = RSNodeMap::Instance().GetAnimationFallbackNode();
    if (target == nullptr) {
        ROSEN_LOGE("RSImplicitAnimator::CreateEmptyAnimation, target is nullptr");
        return;
    }
    CreateImplicitAnimation(*target, RSAnimatableProperty::INVALID, 0.f, 0.f);
    return;
}
template<typename T>
std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const T& startValue, const T& endValue)
{
    if (globalImplicitParams_.empty() || implicitAnimations_.empty() || keyframeAnimations_.empty()) {
        ROSEN_LOGE("Failed to create implicit animation, need to open implicit animation firstly!");
        return {};
    }

    std::shared_ptr<RSAnimation> animation;
    auto params = implicitAnimationParams_.top();
    switch (params->GetType()) {
        case ImplicitAnimationParamType::CURVE: {
            auto curveImplicitParam = static_cast<RSImplicitCurveAnimationParam*>(params.get());
            animation = curveImplicitParam->CreateAnimation(property, startValue, endValue);
            break;
        }
        case ImplicitAnimationParamType::KEYFRAME: {
            auto keyframeImplicitParam = static_cast<RSImplicitKeyframeAnimationParam*>(params.get());
            auto& keyframeAnimations = keyframeAnimations_.top();
            auto keyframeIter = keyframeAnimations.find({ target.GetId(), property });
            SetPropertyValue(target, property, endValue);
            if (keyframeIter == keyframeAnimations.end()) {
                animation = keyframeImplicitParam->CreateAnimation(property, startValue, endValue);
                keyframeAnimations[{ target.GetId(), property }] = animation;
            } else {
                keyframeImplicitParam->AddKeyframe(keyframeIter->second, startValue, endValue);
                return keyframeIter->second;
            }
            break;
        }
        case ImplicitAnimationParamType::PATH: {
            auto pathImplicitParam = static_cast<RSImplicitPathAnimationParam*>(params.get());
            animation = pathImplicitParam->CreateAnimation(property, startValue, endValue);
            break;
        }
        default:
            ROSEN_LOGE("Failed to create animation, unknow type!");
            break;
    }

    if (animation == nullptr) {
        ROSEN_LOGE("Failed to create animation!");
        return nullptr;
    }

    if (params->GetType() != ImplicitAnimationParamType::KEYFRAME) {
        target.AddAnimation(animation);
    }

    implicitAnimations_.top().push_back({ animation, target.GetId() });
    return animation;
}

template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const int& startValue, const int& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const float& startValue, const float& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const Color& startValue, const Color& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const Matrix3f& startValue, const Matrix3f& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const Vector2f& startValue, const Vector2f& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const Vector4f& startValue, const Vector4f& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(
    RSNode& target, const RSAnimatableProperty& property, const Quaternion& startValue, const Quaternion& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(RSNode& target,
    const RSAnimatableProperty& property, const std::shared_ptr<RSFilter>& startValue,
    const std::shared_ptr<RSFilter>& endValue);
template std::shared_ptr<RSAnimation> RSImplicitAnimator::CreateImplicitAnimation(RSNode& target,
    const RSAnimatableProperty& property, const Vector4<Color>& startValue, const Vector4<Color>& endValue);
} // namespace Rosen
} // namespace OHOS
