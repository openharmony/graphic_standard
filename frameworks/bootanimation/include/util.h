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

#ifndef FRAMEWORKS_BOOTANIMATION_INCLUDE_UTIL_H
#define FRAMEWORKS_BOOTANIMATION_INCLUDE_UTIL_H

#include "contrib/minizip/unzip.h"
#include "contrib/minizip/zip.h"
#include "zlib.h"

#include <cstdint>
#include <dirent.h>
#include <functional>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <hilog/log.h>
#include <if_system_ability_manager.h>
#include <ipc_skeleton.h>
#include <iservice_registry.h>
#include <iremote_object.h>
#include <platform/ohos/rs_irender_service.h>
#include <system_ability_definition.h>

namespace OHOS {
#define LOG(fmt, ...) ::OHOS::HiviewDFX::HiLog::Info(             \
    ::OHOS::HiviewDFX::HiLogLabel {LOG_CORE, 0, "BootAnimation"}, \
    "%{public}s: " fmt, __func__, ##__VA_ARGS__)
int64_t GetNowTime();
void PostTask(std::function<void()> func, uint32_t delayTime = 0);
bool UnzipFile(const std::string& srcFilePath, const std::string& dstFilePath);
int RemoveDir(const char *dir);
int CountPicNum(const char *dir, int32_t& picNo);
void WaitRenderServiceInit();
} // namespace OHOS

#endif // FRAMEWORKS_BOOTANIMATION_INCLUDE_UTIL_H
