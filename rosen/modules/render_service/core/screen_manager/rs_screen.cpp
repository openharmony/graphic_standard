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

#include "rs_screen.h"

#include <cinttypes>

namespace OHOS {
namespace Rosen {
using namespace HiviewDFX;

namespace impl {
RSScreen::RSScreen(ScreenId id,
    bool isVirtual,
    std::shared_ptr<HdiOutput> output,
    sptr<Surface> surface)
    : id_(id),
      isVirtual_(isVirtual),
      hdiOutput_(std::move(output)),
      producerSurface_(std::move(surface))
{
    if (!IsVirtual()) {
        name_ = "Screen_" + std::to_string(id_);
        PhysicalScreenInit();
    }
}

RSScreen::RSScreen(const VirtualSreenConfigs &configs)
    : id_(configs.id),
      mirrorId_(configs.mirrorId),
      name_(configs.name),
      width_(configs.width),
      height_(configs.height),
      isVirtual_(true),
      producerSurface_(configs.surface)
{
}

RSScreen::~RSScreen() noexcept
{
}

void RSScreen::PhysicalScreenInit() noexcept
{
    hdiScreen_ = HdiScreen::CreateHdiScreen(ScreenPhysicalId(id_));
    if (hdiScreen_ == nullptr) {
        HiLog::Error(LOG_LABEL, "%{public}s: RSScreen(id %{public}" PRIu64 ") failed to CreateHdiScreens.",
            __func__, id_);
        return;
    }

    hdiScreen_->Init();
    if (hdiScreen_->GetScreenSuppportedModes(supportedModes_) < 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: RSScreen(id %{public}" PRIu64 ") failed to GetScreenSuppportedModes.",
            __func__, id_);
    }
    if (hdiScreen_->GetScreenCapability(capability_) < 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: RSScreen(id %{public}" PRIu64 ") failed to GetScreenCapability.",
            __func__, id_);
    }
    auto status = DispPowerStatus::POWER_STATUS_ON;
    if (hdiScreen_->SetScreenPowerStatus(status) < 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: RSScreen(id %{public}" PRIu64 ") failed to SetScreenPowerStatus.",
            __func__, id_);
    }
    auto activeMode = GetActiveMode();
    if (activeMode) {
        width_ = activeMode->width;
        height_ = activeMode->height;
    }
}

ScreenId RSScreen::Id() const
{
    return id_;
}

ScreenId RSScreen::MirrorId() const
{
    return mirrorId_;
}

void RSScreen::SetMirror(ScreenId mirrorId)
{
    mirrorId_ = mirrorId;
}

const std::string& RSScreen::Name() const
{
    return name_;
}

uint32_t RSScreen::Width() const
{
    return width_;
}

uint32_t RSScreen::Height() const
{
    return height_;
}

bool RSScreen::IsEnable() const
{
    if (id_ == INVALID_SCREEN_ID) {
        return false;
    }

    if (!hdiOutput_ && !producerSurface_) {
        return false;
    }

    // TODO: maybe need more information to judge whether this screen is enable.
    return true;
}

bool RSScreen::IsVirtual() const
{
    return isVirtual_;
}

void RSScreen::SetActiveMode(uint32_t modeId)
{
    if (hdiScreen_->SetScreenMode(modeId) < 0) {
        return;
    }
}

void RSScreen::SetPowerStatus(uint32_t powerStatus)
{
    if (hdiScreen_->SetScreenPowerStatus(static_cast<DispPowerStatus>(powerStatus)) < 0) {
        return;
    }
}

std::optional<DisplayModeInfo> RSScreen::GetActiveMode() const
{
    uint32_t modeId = 0;

    if (hdiScreen_ == nullptr) {
        return {};
    }

    if (hdiScreen_->GetScreenMode(modeId) < 0) {
        // TODO: Error log
        return {};
    }

    if (supportedModes_.empty() || modeId >= supportedModes_.size()) {
        // TODO: Error log
        return {};
    }

    return supportedModes_[modeId];
}

const std::vector<DisplayModeInfo>& RSScreen::GetSupportedModes() const
{
    return supportedModes_;
}

const DisplayCapability& RSScreen::GetCapability() const
{
    return capability_;
}

uint32_t RSScreen::GetPowerStatus() const
{
    DispPowerStatus status;
    if (hdiScreen_->GetScreenPowerStatus(status) < 0) {
        return INVAILD_POWER_STATUS;
    }
    return static_cast<uint32_t>(status);
}

std::shared_ptr<HdiOutput> RSScreen::GetOutput() const
{
    return hdiOutput_;
}

sptr<Surface> RSScreen::GetProducerSurface() const
{
    return producerSurface_;
}
} // namespace impl
} // namespace Rosen
} // namespace OHOS
