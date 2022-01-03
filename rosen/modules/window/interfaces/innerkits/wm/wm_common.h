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

#ifndef OHOS_ROSEN_WM_COMMON_H
#define OHOS_ROSEN_WM_COMMON_H

#include <cstdint>

namespace OHOS {
namespace Rosen {
enum class WindowType : uint32_t {
    WINDOW_TYPE_LAUNCHER = 0,

    WINDOW_TYPE_MEDIA = 1,
    WINDOW_TYPE_APP_MAIN_WINDOW = 2,
    WINDOW_TYPE_APP_SUBWINDOW = 3,

    WINDOW_TYPE_WALLPAPER = 5,
    WINDOW_TYPE_APP_LAUNCH = 10,
    WINDOW_TYPE_DOCK_SLICE = 20,
    WINDOW_TYPE_INCOMING_CALL = 30,
    WINDOW_TYPE_SEARCHING_BAR = 40,
    WINDOW_TYPE_SYSTEM_ALARM_WINDOW = 50,
    WINDOW_TYPE_INPUT_METHOD_FLOAT = 60,
    WINDOW_TYPE_FLOAT = 70,
    WINDOW_TYPE_TOAST = 80,
    WINDOW_TYPE_STATUS_BAR = 90,
    WINDOW_TYPE_PANEL = 100,
    WINDOW_TYPE_KEYGUARD = 110,
    WINDOW_TYPE_VOLUME_OVERLAY = 120,
    WINDOW_TYPE_NAVIGATION_BAR = 130,
    WINDOW_TYPE_DRAGGING_EFFECT = 140,
    WINDOW_TYPE_POINTER = 150,

    WINDOW_TYPE_MAX,
};

enum class WindowMode : uint32_t {
    WINDOW_MODE_FULLSCREEN,
    WINDOW_MODE_SPLIT,
    WINDOW_MODE_FLOATING,
    WINDOW_MODE_PIP
};

enum class WMError : int32_t {
    WM_OK = 0,
    WM_ERROR_SAMGR = 100,
    WM_ERROR_IPC_FAILED = 101,
    WM_ERROR_NO_MEM = 110,
    WM_ERROR_NULLPTR = 120,
    WM_ERROR_INVALID_PARAM = 130,
    WM_ERROR_DESTROYED_OBJECT = 140,
    WM_ERROR_DEATH_RECIPIENT = 150,
    WM_ERROR_UNKNOWN,

    /* weston adater */
    WM_ERROR_WINDOWMANAGER_GETINSTANCE = 100000,
    WM_ERROR_WINDOWMANAGER_INIT = 100001,
    WM_ERROR_WINDOWMANAGER_SHOW = 100002,
    WM_ERROR_WINDOWMANAGER_HIDE = 100003,
    WM_ERROR_WINDOWMANAGER_MOVE = 100004,
    WM_ERROR_WINDOWMANAGER_RESIZE = 100005,
};

struct Rect {
    int32_t posX_;
    int32_t posY_;
    uint32_t width_;
    uint32_t height_;
};
}
}
#endif // OHOS_ROSEN_WM_COMMON_H
