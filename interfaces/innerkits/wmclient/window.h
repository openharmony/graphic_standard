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

#ifndef INTERFACES_INNERKITS_WMCLIENT_WINDOW_H
#define INTERFACES_INNERKITS_WMCLIENT_WINDOW_H

#include <refbase.h>
#include <surface.h>
#include <promise.h>

#include "window_manager_input_type.h"
#include "window_manager_type.h"

namespace OHOS {
class Window : public RefBase {
public:
    virtual sptr<Surface> GetSurface() const = 0;
    virtual sptr<IBufferProducer> GetProducer() const = 0;
    virtual int32_t       GetID() const = 0;
    virtual int32_t       GetX() const = 0;
    virtual int32_t       GetY() const = 0;
    virtual uint32_t      GetWidth() const = 0;
    virtual uint32_t      GetHeight() const = 0;
    virtual uint32_t      GetDestWidth() const = 0;
    virtual uint32_t      GetDestHeight() const = 0;
    virtual bool          GetVisibility() const = 0;
    virtual WindowType    GetType() const = 0;
    virtual WindowMode    GetMode() const = 0;

    virtual sptr<Promise<GSError>> Show() = 0;
    virtual sptr<Promise<GSError>> Hide() = 0;
    virtual sptr<Promise<GSError>> Move(int32_t x, int32_t y) = 0;
    virtual sptr<Promise<GSError>> SwitchTop() = 0;
    virtual sptr<Promise<GSError>> SetWindowType(WindowType type) = 0;
    virtual sptr<Promise<GSError>> SetWindowMode(WindowMode mode) = 0;
    virtual sptr<Promise<GSError>> Resize(uint32_t width, uint32_t height) = 0;
    virtual sptr<Promise<GSError>> ScaleTo(uint32_t width, uint32_t height) = 0;
    virtual GSError Rotate(WindowRotateType type) = 0;
    virtual GSError Destroy() = 0;

    // prop listener
    virtual void OnPositionChange(WindowPositionChangeFunc func) = 0;
    virtual void OnSizeChange(WindowSizeChangeFunc func) = 0;
    virtual void OnVisibilityChange(WindowVisibilityChangeFunc func) = 0;
    virtual void OnTypeChange(WindowTypeChangeFunc func) = 0;
    virtual void OnModeChange(WindowModeChangeFunc func) = 0;
    virtual void OnSplitStatusChange(SplitStatusChangeFunc func) = 0;

    // listener
    virtual GSError OnTouch(OnTouchFunc cb) = 0;
    virtual GSError OnKey(OnKeyFunc cb) = 0;

    // pointer listener
    virtual GSError OnPointerEnter(PointerEnterFunc func) = 0;
    virtual GSError OnPointerLeave(PointerLeaveFunc func) = 0;
    virtual GSError OnPointerMotion(PointerMotionFunc func) = 0;
    virtual GSError OnPointerButton(PointerButtonFunc func) = 0;
    virtual GSError OnPointerFrame(PointerFrameFunc func) = 0;
    virtual GSError OnPointerAxis(PointerAxisFunc func) = 0;
    virtual GSError OnPointerAxisSource(PointerAxisSourceFunc func) = 0;
    virtual GSError OnPointerAxisStop(PointerAxisStopFunc func) = 0;
    virtual GSError OnPointerAxisDiscrete(PointerAxisDiscreteFunc func) = 0;

    // keyboard listener
    virtual GSError OnKeyboardKeymap(KeyboardKeymapFunc func) = 0;
    virtual GSError OnKeyboardEnter(KeyboardEnterFunc func) = 0;
    virtual GSError OnKeyboardLeave(KeyboardLeaveFunc func) = 0;
    virtual GSError OnKeyboardKey(KeyboardKeyFunc func) = 0;
    virtual GSError OnKeyboardModifiers(KeyboardModifiersFunc func) = 0;
    virtual GSError OnKeyboardRepeatInfo(KeyboardRepeatInfoFunc func) = 0;

    // touch listener
    virtual GSError OnTouchDown(TouchDownFunc func) = 0;
    virtual GSError OnTouchUp(TouchUpFunc func) = 0;
    virtual GSError OnTouchMotion(TouchMotionFunc func) = 0;
    virtual GSError OnTouchFrame(TouchFrameFunc func) = 0;
    virtual GSError OnTouchCancel(TouchCancelFunc func) = 0;
    virtual GSError OnTouchShape(TouchShapeFunc func) = 0;
    virtual GSError OnTouchOrientation(TouchOrientationFunc func) = 0;
};
} // namespace OHOS

#endif // INTERFACES_INNERKITS_WMCLIENT_WINDOW_H
