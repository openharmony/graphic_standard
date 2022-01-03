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

#include "window_property.h"

namespace OHOS {
namespace Rosen {
void WindowProperty::SetWindowRect(const struct Rect& rect)
{
    windowRect_ = rect;
}

void WindowProperty::SetWindowType(WindowType type)
{
    type_ = type;
}

void WindowProperty::SetWindowMode(WindowMode mode)
{
    mode_ = mode;
}

void WindowProperty::SetFullScreen(bool isFullScreen)
{
    isFullScreen_ = isFullScreen;
}

void WindowProperty::SetFocusable(bool isFocusable)
{
    focusable_ = isFocusable;
}

void WindowProperty::SetTouchable(bool isTouchable)
{
    touchable_ = isTouchable;
}

void WindowProperty::SetPrivacyMode(bool isPrivate)
{
    isPrivacyMode_ = isPrivate;
}

void WindowProperty::SetTransparent(bool isTransparent)
{
    isTransparent_ = isTransparent;
}

void WindowProperty::SetAlpha(float alpha)
{
    alpha_ = alpha;
}
void WindowProperty::SetDisplayId(int32_t displayId)
{
    displayId_ = displayId;
}
void WindowProperty::SetParentId(const std::string& parentId)
{
    parentId_ = parentId;
}
void WindowProperty::SetWindowId(const std::string& windowId)
{
    windowId_ = windowId;
}

Rect WindowProperty::GetWindowRect() const
{
    return windowRect_;
}

WindowType WindowProperty::GetWindowType() const
{
    return type_;
}

WindowMode WindowProperty::GetWindowMode() const
{
    return mode_;
}

bool WindowProperty::GetFullScreen() const
{
    return isFullScreen_;
}

bool WindowProperty::GetFocusable() const
{
    return focusable_;
}

bool WindowProperty::GetTouchable() const
{
    return touchable_;
}

bool WindowProperty::GetPrivacyMode() const
{
    return isPrivacyMode_;
}

bool WindowProperty::GetTransparent() const
{
    return isTransparent_;
}

float WindowProperty::GetAlpha() const
{
    return alpha_;
}

int WindowProperty::GetDisplayId() const
{
    return displayId_;
}

const std::string& WindowProperty::GetParentId() const
{
    return parentId_;
}

const std::string& WindowProperty::GetWindowId() const
{
    return windowId_;
}

bool WindowProperty::Marshalling(Parcel& parcel) const
{
    // write windowRect_
    if (!(parcel.WriteInt32(windowRect_.posX_) && parcel.WriteInt32(windowRect_.posY_) &&
        parcel.WriteUint32(windowRect_.width_) && parcel.WriteUint32(windowRect_.height_))) {
        return false;
    }

    // write type_
    if (!parcel.WriteUint32(static_cast<uint32_t>(type_))) {
        return false;
    }

    // write mode_
    if (!parcel.WriteUint32(static_cast<uint32_t>(mode_))) {
        return false;
    }

    // write isFullScreen_
    if (!parcel.WriteBool(isFullScreen_)) {
        return false;
    }

    // write focusable_
    if (!parcel.WriteBool(focusable_)) {
        return false;
    }

    // write touchable_
    if (!parcel.WriteBool(touchable_)) {
        return false;
    }

    // write isPrivacyMode_
    if (!parcel.WriteBool(isPrivacyMode_)) {
        return false;
    }

    // write isTransparent_
    if (!parcel.WriteBool(isTransparent_)) {
        return false;
    }

    // write alpha_
    if (!parcel.WriteFloat(alpha_)) {
        return false;
    }

    // write displayId_
    if (!parcel.WriteInt32(displayId_)) {
        return false;
    }

    // write parentId_
    if (!parcel.WriteString(parentId_)) {
        return false;
    }

    // write windowId_
    if (!parcel.WriteString(windowId_)) {
        return false;
    }
    return true;
}

sptr<WindowProperty> WindowProperty::Unmarshalling(Parcel& parcel)
{
    sptr<WindowProperty> property(new WindowProperty());
    Rect rect = { parcel.ReadInt32(), parcel.ReadInt32(), parcel.ReadUint32(), parcel.ReadUint32() };
    property->SetWindowRect(rect);

    property->SetWindowType(static_cast<WindowType>(parcel.ReadUint32()));
    property->SetWindowMode(static_cast<WindowMode>(parcel.ReadUint32()));
    property->SetFullScreen(parcel.ReadBool());
    property->SetFocusable(parcel.ReadBool());
    property->SetTouchable(parcel.ReadBool());
    property->SetPrivacyMode(parcel.ReadBool());
    property->SetTransparent(parcel.ReadBool());
    property->SetAlpha(parcel.ReadFloat());
    property->SetDisplayId(parcel.ReadInt32());
    property->SetParentId(parcel.ReadString());
    property->SetWindowId(parcel.ReadString());
    return property;
}
}
}
