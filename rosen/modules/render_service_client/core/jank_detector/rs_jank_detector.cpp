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

#include "jank_detector/rs_jank_detector.h"

#include <unistd.h>
#include "base/hiviewdfx/hisysevent/interfaces/native/innerkits/hisysevent/include/hisysevent.h"
#include "platform/common/rs_log.h"

namespace {
void DrawEventReport(uint64_t totalTime, uint64_t uiDrawTime, uint64_t renderDrawTime, int dropUiFrameNum,
    const std::string& abilityName)
{
    int32_t pid = getpid();
    uint32_t uid = getuid();
    std::string domain = "GRAPHIC";
    std::string stringId = "NO_DRAW";
    std::string msg = "It took " + std::to_string(totalTime) + "ns to draw, "
        + "UI took " + std::to_string(uiDrawTime) + "ns to draw, "
        + "RSRenderThread took " + std::to_string(renderDrawTime) + "ns to draw, "
        + "RSRenderThread dropped " + std::to_string(dropUiFrameNum) + " UI Frames";

    OHOS::HiviewDFX::HiSysEvent::Write(domain, stringId,
        OHOS::HiviewDFX::HiSysEvent::EventType::FAULT,
        "PID", pid,
        "UID", uid,
        "ABILITY_NAME", abilityName,
        "MSG", msg);
}
}

namespace OHOS {
namespace Rosen {
uint64_t RSJankDetector::GetSysTimeNs()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
}

void RSJankDetector::SetRefreshPeriod(uint64_t refreshPeriod)
{
    refreshPeriod_ = refreshPeriod;
}

void RSJankDetector::UpdateUiDrawFrameMsg(uint64_t startTimeStamp, uint64_t endTimeStamp,
    const std::string& abilityName)
{
    UiDrawFrameMsg uiFrame;
    uiFrame.startTimeStamp = startTimeStamp;
    uiFrame.endTimeStamp = endTimeStamp;
    uiFrame.abilityName = abilityName;
    {
        std::unique_lock<std::mutex> frameLock(frameMutex_);
        uiDrawFrames_.emplace_back(uiFrame);
    }
}

void RSJankDetector::ProcessUiDrawFrameMsg()
{
    preUiDrawFrames_.clear();
    {
        std::unique_lock<std::mutex> frameLock(frameMutex_);
        std::swap(preUiDrawFrames_, uiDrawFrames_);
    }
}

void RSJankDetector::CalculateSkippedFrame(uint64_t renderStartTimeStamp, uint64_t renderEndTimeStamp)
{
    int dropUiFrameNum = 0;
    uint64_t uiStartTimeStamp = 0;
    uint64_t uiEndTimeStamp = 0;
    std::string abilityName;
    if (!preUiDrawFrames_.empty()) {
        UiDrawFrameMsg uiDrawFrame = preUiDrawFrames_.front();
        dropUiFrameNum = preUiDrawFrames_.size() - 1;
        uiStartTimeStamp = uiDrawFrame.startTimeStamp;
        uiEndTimeStamp = uiDrawFrame.endTimeStamp;
        abilityName = uiDrawFrame.abilityName;
    }

    uint64_t totalTime = renderEndTimeStamp - uiStartTimeStamp;
    uint64_t uiDrawTime = uiEndTimeStamp - uiStartTimeStamp;
    uint64_t renderDrawTime = renderEndTimeStamp - renderStartTimeStamp;

    // Currently a frame takes two vsync times
    int skippedFrame = totalTime / (refreshPeriod_ * 2);
    if ((skippedFrame >= JANK_SKIPPED_THRESHOLD) || (dropUiFrameNum >= JANK_SKIPPED_THRESHOLD)) {
        DrawEventReport(totalTime, uiDrawTime, renderDrawTime, dropUiFrameNum, abilityName);
        ROSEN_LOGD("%s took %llu, UI took %llu, RSRenderThread took %llu, RSRenderThread dropped %d UI Frames",
            abilityName.c_str(), totalTime, uiDrawTime, renderDrawTime);
    }
}
} // namespace Rosen
} // namespace OHOS
