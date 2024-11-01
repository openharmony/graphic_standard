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

#include "rs_screen_manager.h"
#include "pipeline/rs_main_thread.h"

namespace OHOS {
namespace Rosen {
using namespace HiviewDFX;
namespace impl {
std::once_flag RSScreenManager::createFlag_;
sptr<OHOS::Rosen::RSScreenManager> RSScreenManager::instance_ = nullptr;

sptr<OHOS::Rosen::RSScreenManager> RSScreenManager::GetInstance() noexcept
{
    std::call_once(createFlag_, []() {
        instance_ = new RSScreenManager();
    });

    return instance_;
}

RSScreenManager::RSScreenManager()
{
}

RSScreenManager::~RSScreenManager() noexcept
{
}

bool RSScreenManager::Init() noexcept
{
    composer_ = HdiBackend::GetInstance();
    if (composer_ == nullptr) {
        HiLog::Error(LOG_LABEL, "%{public}s: Failed to get composer.", __func__);
        return false;
    }

    if (composer_->RegScreenHotplug(&RSScreenManager::OnHotPlug, this) == -1) {
        HiLog::Error(LOG_LABEL, "%{public}s: Failed to register OnHotPlug Func to composer.", __func__);
        return false;
    }

    // call ProcessScreenHotPlugEvents() for primary screen immediately in main thread.
    ProcessScreenHotPlugEvents();

    return true;
}

void RSScreenManager::OnHotPlug(std::shared_ptr<HdiOutput> &output, bool connected, void *data)
{
    if (output == nullptr) {
        HiLog::Error(LOG_LABEL, "%{public}s: output is nullptr.", __func__);
        return;
    }

    RSScreenManager *screenManager = nullptr;
    if (data != nullptr) {
        screenManager = static_cast<RSScreenManager *>(data);
    } else {
        screenManager = static_cast<RSScreenManager *>(RSScreenManager::GetInstance().GetRefPtr());
    }

    if (screenManager == nullptr) {
        HiLog::Error(LOG_LABEL, "%{public}s: Failed to find RSScreenManager instance.", __func__);
        return;
    }

    screenManager->OnHotPlugEvent(output, connected);
}

void RSScreenManager::OnHotPlugEvent(std::shared_ptr<HdiOutput> &output, bool connected)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingHotPlugEvents_.emplace_back(ScreenHotPlugEvent{output, connected});
    }

    // This func would be called in main thread first time immediately after calling composer_->RegScreenHotplug,
    // but at this time the RSMainThread object would not be ready to handle this, so we need to call
    // ProcessScreenHotPlugEvents() after this func in RSScreenManager::Init().

    // Normally, this func would be called in hdi's hw-ipc threads(but sometimes in main thread, maybe),
    // so we should notify the RSMainThread to postTask to call ProcessScreenHotPlugEvents().
    auto mainThread = RSMainThread::Instance();
    if (mainThread == nullptr) {
        return;
    }
    mainThread->RequestNextVSync();
}

void RSScreenManager::ProcessScreenHotPlugEvents()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &event : pendingHotPlugEvents_) {
        if (event.connected) {
            ProcessScreenConnectedLocked(event.output);
        } else {
            ProcessScreenDisConnectedLocked(event.output);
        }
    }
    for (auto id : connectedIds_) {
        for (auto &cb : screenChangeCallbacks_) {
            cb->OnScreenChanged(id, ScreenEvent::CONNECTED);
        }
    }
    mipiCheckInFirstHotPlugEvent_ = true;
    pendingHotPlugEvents_.clear();
    connectedIds_.clear();
}

void RSScreenManager::ProcessScreenConnectedLocked(std::shared_ptr<HdiOutput> &output)
{
    if (output == nullptr) {
        HiLog::Error(LOG_LABEL, "%{public}s: output is nullptr.", __func__);
        return;
    }

    bool isVirtual = false;
    ScreenId id = ToScreenId(output->GetScreenId());

    if (screens_.count(id) == 1) {
        HiLog::Warn(LOG_LABEL, "%{public}s: The screen for id %{public}" PRIu64 " already existed.", __func__, id);

        // [PLANNING]: should we erase it and create a new one?
        for (auto &cb : screenChangeCallbacks_) {
            cb->OnScreenChanged(id, ScreenEvent::DISCONNECTED);
        }
        screens_.erase(id);
    }

    screens_[id] = std::make_unique<RSScreen>(id, isVirtual, output, nullptr);

    if (screens_[id]->GetCapability().type == InterfaceType::DISP_INTF_MIPI) {
        if (!mipiCheckInFirstHotPlugEvent_) {
            defaultScreenId_ = id;
        }
        mipiCheckInFirstHotPlugEvent_ = true;
    } else if (defaultScreenId_ == INVALID_SCREEN_ID) {
        defaultScreenId_ = id;
    }

    HiLog::Info(LOG_LABEL, "%{public}s: A new screen(id %{public}" PRIu64 ") connected.", __func__, id);
    connectedIds_.emplace_back(id);
}

void RSScreenManager::ProcessScreenDisConnectedLocked(std::shared_ptr<HdiOutput> &output)
{
    ScreenId id = ToScreenId(output->GetScreenId());

    if (screens_.count(id) == 0) {
        HiLog::Warn(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".", __func__, id);
    } else {
        for (auto &cb : screenChangeCallbacks_) {
            cb->OnScreenChanged(id, ScreenEvent::DISCONNECTED);
        }
        screens_.erase(id);
        HiLog::Info(LOG_LABEL, "%{public}s: Screen(id %{public}" PRIu64 ") disconnected.", __func__, id);
    }

    if (id == defaultScreenId_) {
        HandleDefaultScreenDisConnectedLocked();
    }
}

// If the previous primary screen disconnected, we traversal the left screens
// and find the first physical screen to be the default screen.
// If there was no physical screen left, we set the first screen as default, no matter what type it is.
// At last, if no screen left, we set Default Screen Id to INVALID_SCREEN_ID.
void RSScreenManager::HandleDefaultScreenDisConnectedLocked()
{
    defaultScreenId_ = INVALID_SCREEN_ID;
    for (const auto &[id, screen] : screens_) {
        if (!screen->IsVirtual()) {
            defaultScreenId_ = id;
            break;
        }
    }

    if (defaultScreenId_ == INVALID_SCREEN_ID) {
        if (!screens_.empty()) {
            defaultScreenId_ = screens_.cbegin()->first;
        }
    }
}

void RSScreenManager::SetDefaultScreenId(ScreenId id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    defaultScreenId_ = id;
}

void RSScreenManager::SetScreenMirror(ScreenId id, ScreenId toMirror)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (screens_.count(id) == 0) {
        HiLog::Warn(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".", __func__, id);
        return;
    }

    screens_[id]->SetMirror(toMirror);
}

ScreenId RSScreenManager::GenerateVirtualScreenIdLocked()
{
    if (!freeVirtualScreenIds_.empty()) {
        ScreenId id = freeVirtualScreenIds_.front();
        freeVirtualScreenIds_.pop();
        return id;
    }

    // The left 32 bits is for virtual screen id.
    return (static_cast<ScreenId>(maxVirtualScreenNum_++) << 32) | 0xffffffffu;
}

void RSScreenManager::ReuseVirtualScreenIdLocked(ScreenId id)
{
    freeVirtualScreenIds_.push(id);
}

void RSScreenManager::GetVirtualScreenResolutionLocked(ScreenId id,
    RSVirtualScreenResolution& virtualScreenResolution) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".", __func__, id);
        return;
    }

    virtualScreenResolution.SetVirtualScreenWidth(static_cast<uint32_t>(screens_.at(id)->Width()));
    virtualScreenResolution.SetVirtualScreenHeight(static_cast<uint32_t>(screens_.at(id)->Height()));
}

void RSScreenManager::GetScreenActiveModeLocked(ScreenId id, RSScreenModeInfo& screenModeInfo) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".", __func__, id);
        return;
    }

    auto modeInfo = screens_.at(id)->GetActiveMode();
    if (!modeInfo) {
        HiLog::Error(LOG_LABEL, "%{public}s: Failed to get active mode for screen %{public}" PRIu64 ".", __func__, id);
        return;
    }

    screenModeInfo.SetScreenWidth(modeInfo->width);
    screenModeInfo.SetScreenHeight(modeInfo->height);
    screenModeInfo.SetScreenRefreshRate(modeInfo->freshRate);
    screenModeInfo.SetScreenModeId(screens_.at(id)->GetActiveModePosByModeId(modeInfo->id));
}

std::vector<RSScreenModeInfo> RSScreenManager::GetScreenSupportedModesLocked(ScreenId id) const
{
    std::vector<RSScreenModeInfo> screenSupportedModes;
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".", __func__, id);
        return screenSupportedModes;
    }

    const auto& displaySupportedModes = screens_.at(id)->GetSupportedModes();
    screenSupportedModes.resize(displaySupportedModes.size());
    for (decltype(displaySupportedModes.size()) idx = 0; idx < displaySupportedModes.size(); ++idx) {
        screenSupportedModes[idx].SetScreenWidth(displaySupportedModes[idx].width);
        screenSupportedModes[idx].SetScreenHeight(displaySupportedModes[idx].height);
        screenSupportedModes[idx].SetScreenRefreshRate(displaySupportedModes[idx].freshRate);
        screenSupportedModes[idx].SetScreenModeId(displaySupportedModes[idx].id);
    }
    return screenSupportedModes;
}

RSScreenCapability RSScreenManager::GetScreenCapabilityLocked(ScreenId id) const
{
    RSScreenCapability screenCapability;
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return screenCapability;
    }
    if (screens_.at(id)->IsVirtual()) {
        HiLog::Warn(LOG_LABEL, "%{public}s: only name attribute is valid for virtual screen.\n", __func__);
        screenCapability.SetName(screens_.at(id)->Name());
        return screenCapability;
    }

    const auto& capability = screens_.at(id)->GetCapability();
    std::vector<RSScreenProps> props;
    uint32_t propCount = capability.propertyCount;
    props.resize(propCount);
    for (uint32_t propIndex = 0; propIndex < propCount; propIndex++) {
        props[propIndex] = RSScreenProps(capability.props[propIndex].name, capability.props[propIndex].propId,
            capability.props[propIndex].value);
    }
    screenCapability.SetName(capability.name);
    screenCapability.SetType(static_cast<ScreenInterfaceType>(capability.type));
    screenCapability.SetPhyWidth(capability.phyWidth);
    screenCapability.SetPhyHeight(capability.phyHeight);
    screenCapability.SetSupportLayers(capability.supportLayers);
    screenCapability.SetVirtualDispCount(capability.virtualDispCount);
    screenCapability.SetSupportWriteback(capability.supportWriteBack);
    screenCapability.SetProps(props);
    return screenCapability;
}

ScreenPowerStatus RSScreenManager::GetScreenPowerStatuslocked(ScreenId id) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return INVALID_POWER_STATUS;
    }

    ScreenPowerStatus status = static_cast<ScreenPowerStatus>(screens_.at(id)->GetPowerStatus());
    return status;
}

std::vector<ScreenId> RSScreenManager::GetAllScreenIds()
{
    std::vector<ScreenId> ids;
    for (std::unordered_map<ScreenId, std::unique_ptr<OHOS::Rosen::RSScreen>>::iterator iter = screens_.begin();
        iter != screens_.end(); ++iter) {
        ids.emplace_back(iter->first);
    }
    return ids;
}

ScreenId RSScreenManager::CreateVirtualScreen(
    const std::string &name,
    uint32_t width,
    uint32_t height,
    sptr<Surface> surface,
    ScreenId mirrorId,
    int32_t flags
    )
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (surface != nullptr) {
        uint64_t surfaceId = surface->GetUniqueId();
        for (auto &[_, screen] : screens_) {
            if (!screen->IsVirtual()) {
                continue;
            }
            auto screenSurface = screen->GetProducerSurface();
            if (screenSurface == nullptr) {
                continue;
            }
            if (screenSurface->GetUniqueId() == surfaceId) {
                HiLog::Error(LOG_LABEL, "surface %{public}" PRIu64 " is used, create virtualscreen failed!", surfaceId);
                return INVALID_SCREEN_ID;
            }
        }
    } else {
        HiLog::Debug(LOG_LABEL, "%{public}s: surface is nullptr.\n", __func__);
    }

    VirtualScreenConfigs configs;
    ScreenId newId = GenerateVirtualScreenIdLocked();
    configs.id = newId;
    configs.mirrorId = mirrorId;
    configs.name = name;
    configs.width = width;
    configs.height = height;
    configs.surface = surface;
    configs.flags = flags;

    screens_[newId] = std::make_unique<RSScreen>(configs);
    HiLog::Debug(LOG_LABEL, "%{public}s: create virtual screen(id %{public}" PRIu64 ").\n", __func__, newId);
    return newId;
}

int32_t RSScreenManager::SetVirtualScreenSurface(ScreenId id, sptr<Surface> surface)
{
    uint64_t surfaceId = surface->GetUniqueId();
    for (auto &[screenId, screen] : screens_) {
        if (!screen->IsVirtual() || screenId == id) {
            continue;
        }
        auto screenSurface = screen->GetProducerSurface();
        if (screenSurface == nullptr) {
            continue;
        }
        if (screenSurface->GetUniqueId() == surface->GetUniqueId()) {
            HiLog::Error(LOG_LABEL, "surface %{public}" PRIu64 " is used, set surface failed!", surfaceId);
            return SURFACE_NOT_UNIQUE;
        }
    }
    screens_.at(id)->SetProducerSurface(surface);
    HiLog::Debug(LOG_LABEL, "%{public}s:  set virtual screen surface success! \n", __func__);
    return SUCCESS;
}

void RSScreenManager::RemoveVirtualScreen(ScreenId id)
{
    std::lock_guard<std::mutex> lock(mutex_);

    RemoveVirtualScreenLocked(id);
}

void RSScreenManager::RemoveVirtualScreenLocked(ScreenId id)
{
    if (screens_.count(id) == 0) {
        HiLog::Warn(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".", __func__, id);
        return;
    }

    screens_.erase(id);

    // Update other screens' mirrorId.
    for (auto &[id, screen] : screens_) {
        if (screen->MirrorId() == id) {
            screen->SetMirror(INVALID_SCREEN_ID);
        }
    }
    HiLog::Debug(LOG_LABEL, "%{public}s: remove virtual screen(id %{public}" PRIu64 ").\n", __func__, id);

    ReuseVirtualScreenIdLocked(id);
}

void RSScreenManager::SetScreenActiveMode(ScreenId id, uint32_t modeId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return;
    }
    screens_.at(id)->SetActiveMode(modeId);
}

int32_t RSScreenManager::SetVirtualScreenResolution(ScreenId id, uint32_t width, uint32_t height)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no virtual screen for id %{public}" PRIu64 ".\n", __func__, id);
        return SCREEN_NOT_FOUND;
    }
    screens_.at(id)->SetResolution(width, height);
    HiLog::Debug(LOG_LABEL, "%{public}s:  set virtual screen resolution success! \n", __func__);
    return SUCCESS;
}

void RSScreenManager::SetScreenPowerStatus(ScreenId id, ScreenPowerStatus status)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return;
    }
    screens_.at(id)->SetPowerStatus(static_cast<uint32_t>(status));

    /*
     * If app adds the first frame when power on the screen, delete the code
     */
    if (status == ScreenPowerStatus::POWER_STATUS_ON) {
        auto mainThread = RSMainThread::Instance();
        if (mainThread == nullptr) {
            return;
        }
        mainThread->RequestNextVSync();
        HiLog::Info(LOG_LABEL, "Set system power on, request a frame");
    }
}

void RSScreenManager::GetVirtualScreenResolution(ScreenId id, RSVirtualScreenResolution& virtualScreenResolution) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    GetVirtualScreenResolutionLocked(id, virtualScreenResolution);
}

void RSScreenManager::GetScreenActiveMode(ScreenId id, RSScreenModeInfo& screenModeInfo) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    GetScreenActiveModeLocked(id, screenModeInfo);
}

std::vector<RSScreenModeInfo> RSScreenManager::GetScreenSupportedModes(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    return GetScreenSupportedModesLocked(id);
}

RSScreenCapability RSScreenManager::GetScreenCapability(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    return GetScreenCapabilityLocked(id);
}

ScreenPowerStatus RSScreenManager::GetScreenPowerStatus(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    return GetScreenPowerStatuslocked(id);
}

RSScreenData RSScreenManager::GetScreenData(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    RSScreenData screenData;
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return screenData;
    }
    RSScreenCapability capability = GetScreenCapabilityLocked(id);
    RSScreenModeInfo activeMode;
    GetScreenActiveModeLocked(id, activeMode);
    std::vector<RSScreenModeInfo> supportModes = GetScreenSupportedModesLocked(id);
    ScreenPowerStatus powerStatus = GetScreenPowerStatuslocked(id);
    screenData.SetCapability(capability);
    screenData.SetActivityModeInfo(activeMode);
    screenData.SetSupportModeInfo(supportModes);
    screenData.SetPowerStatus(powerStatus);
    return screenData;
}

int32_t RSScreenManager::GetScreenBacklight(ScreenId id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetScreenBacklightlocked(id);
}

int32_t RSScreenManager::GetScreenBacklightlocked(ScreenId id) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return INVALID_BACKLIGHT_VALUE;
    }

    int32_t level = screens_.at(id)->GetScreenBacklight();
    return level;
}

void RSScreenManager::SetScreenBacklight(ScreenId id, uint32_t level)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return;
    }
    screens_.at(id)->SetScreenBacklight(level);
}

ScreenInfo RSScreenManager::QueryScreenInfo(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    ScreenInfo info;
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".", __func__, id);
        return info;
    }

    const auto &screen = screens_.at(id);

    info.width = screen->Width();
    info.height = screen->Height();
    (void)screen->GetScreenColorGamut(info.colorGamut);

    if (!screen->IsEnable()) {
        info.state = ScreenState::DISABLED;
    } else if (!screen->IsVirtual()) {
        info.state = ScreenState::HDI_OUTPUT_ENABLE;
    } else {
        info.state = ScreenState::PRODUCER_SURFACE_ENABLE;
    }
    info.rotationMatrix = screen->GetRotationMatrix();

    return info;
}

sptr<Surface> RSScreenManager::GetProducerSurface(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    // assert screens_.count(id) == 1
    return screens_.at(id)->GetProducerSurface();
}

std::shared_ptr<HdiOutput> RSScreenManager::GetOutput(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);

    // assert screens_.count(id) == 1
    return screens_.at(id)->GetOutput();
}

int32_t RSScreenManager::AddScreenChangeCallback(const sptr<RSIScreenChangeCallback> &callback)
{
    if (callback == nullptr) {
        HiLog::Error(LOG_LABEL, "%{public}s: callback is NULL.", __func__);
        return INVALID_ARGUMENTS;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    // when the callback first registered, maybe there were some physical screens already connected,
    // so notify to remote immediately.
    for (const auto &[id, screen] : screens_) {
        if (!screen->IsVirtual()) {
            callback->OnScreenChanged(id, ScreenEvent::CONNECTED);
        }
    }
    screenChangeCallbacks_.push_back(callback);
    HiLog::Debug(LOG_LABEL, "%{public}s: add a remote callback succeed.", __func__);
    return SUCCESS;
}

void RSScreenManager::RemoveScreenChangeCallback(const sptr<RSIScreenChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = screenChangeCallbacks_.begin(); it != screenChangeCallbacks_.end(); it++) {
        if (*it == callback) {
            screenChangeCallbacks_.erase(it);
            HiLog::Debug(LOG_LABEL, "%{public}s: remove a remote callback succeed.", __func__);
            break;
        }
    }
}

void RSScreenManager::DisplayDump(std::string& dumpString)
{
    int32_t index = 0;
    for (const auto &[id, screen] : screens_) {
        screen->DisplayDump(index, dumpString);
        index++;
    }
}

void RSScreenManager::SurfaceDump(std::string& dumpString)
{
    int32_t index = 0;
    for (const auto &[id, screen] : screens_) {
        screen->SurfaceDump(index, dumpString);
        index++;
    }
}

void RSScreenManager::FpsDump(std::string& dumpString, std::string& arg)
{
    int32_t index = 0;
    dumpString += "\n-- The recently fps records info of screens:\n";
    for (const auto &[id, screen] : screens_) {
        screen->FpsDump(index, dumpString, arg);
        index++;
    }
}

int32_t RSScreenManager::GetScreenSupportedColorGamutsLocked(ScreenId id, std::vector<ScreenColorGamut>& mode) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return StatusCode::SCREEN_NOT_FOUND;
    }
    return screens_.at(id)->GetScreenSupportedColorGamuts(mode);
}

int32_t RSScreenManager::GetScreenColorGamutLocked(ScreenId id, ScreenColorGamut& mode) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return StatusCode::SCREEN_NOT_FOUND;
    }
    return screens_.at(id)->GetScreenColorGamut(mode);
}

int32_t RSScreenManager::SetScreenColorGamutLocked(ScreenId id, int32_t modeIdx)
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return StatusCode::SCREEN_NOT_FOUND;
    }
    return screens_.at(id)->SetScreenColorGamut(modeIdx);
}

int32_t RSScreenManager::SetScreenGamutMapLocked(ScreenId id, ScreenGamutMap mode)
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return StatusCode::SCREEN_NOT_FOUND;
    }
    return screens_.at(id)->SetScreenGamutMap(mode);
}

int32_t RSScreenManager::GetScreenGamutMapLocked(ScreenId id, ScreenGamutMap &mode) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return StatusCode::SCREEN_NOT_FOUND;
    }
    return screens_.at(id)->GetScreenGamutMap(mode);
}

bool RSScreenManager::RequestRotationLocked(ScreenId id, ScreenRotation rotation)
{
    auto iter = screens_.find(id);
    if (iter == screens_.end()) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return false;
    }
    return iter->second->SetRotation(rotation);
}

ScreenRotation RSScreenManager::GetRotationLocked(ScreenId id) const
{
    auto iter = screens_.find(id);
    if (iter == screens_.end()) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return ScreenRotation::INVALID_SCREEN_ROTATION;
    }
    return iter->second->GetRotation();
}

int32_t RSScreenManager::GetScreenHDRCapabilityLocked(ScreenId id, RSScreenHDRCapability& screenHdrCapability) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return StatusCode::SCREEN_NOT_FOUND;
    }
    HDRCapability hdrCapability = screens_.at(id)->GetHDRCapability();
    std::vector<ScreenHDRFormat> hdrFormats;
    uint32_t formatCount = hdrCapability.formatCount;
    hdrFormats.resize(formatCount);
    for (uint32_t index = 0; index < formatCount; index++) {
        hdrFormats[index] = static_cast<ScreenHDRFormat>(hdrCapability.formats[index]);
    }
    screenHdrCapability.SetMaxLum(hdrCapability.maxLum);
    screenHdrCapability.SetMaxAverageLum(hdrCapability.maxAverageLum);
    screenHdrCapability.SetMinLum(hdrCapability.minLum);
    screenHdrCapability.SetHdrFormats(hdrFormats);
    return StatusCode::SUCCESS;
}

int32_t RSScreenManager::GetScreenTypeLocked(ScreenId id, RSScreenType& type) const
{
    if (screens_.count(id) == 0) {
        HiLog::Error(LOG_LABEL, "%{public}s: There is no screen for id %{public}" PRIu64 ".\n", __func__, id);
        return StatusCode::SCREEN_NOT_FOUND;
    }

    type = screens_.at(id)->GetScreenType();
    return StatusCode::SUCCESS;
}

int32_t RSScreenManager::GetScreenSupportedColorGamuts(ScreenId id, std::vector<ScreenColorGamut>& mode) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetScreenSupportedColorGamutsLocked(id, mode);
}

int32_t RSScreenManager::GetScreenColorGamut(ScreenId id, ScreenColorGamut &mode) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetScreenColorGamutLocked(id, mode);
}

int32_t RSScreenManager::SetScreenColorGamut(ScreenId id, int32_t modeIdx)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return SetScreenColorGamutLocked(id, modeIdx);
}

int32_t RSScreenManager::SetScreenGamutMap(ScreenId id, ScreenGamutMap mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return SetScreenGamutMapLocked(id, mode);
}

int32_t RSScreenManager::GetScreenGamutMap(ScreenId id, ScreenGamutMap &mode) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetScreenGamutMapLocked(id, mode);
}

bool RSScreenManager::RequestRotation(ScreenId id, ScreenRotation rotation)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return RequestRotationLocked(id, rotation);
}

ScreenRotation RSScreenManager::GetRotation(ScreenId id) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetRotationLocked(id);
}

int32_t RSScreenManager::GetScreenHDRCapability(ScreenId id, RSScreenHDRCapability& screenHdrCapability) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetScreenHDRCapabilityLocked(id, screenHdrCapability);
}

int32_t RSScreenManager::GetScreenType(ScreenId id, RSScreenType& type) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetScreenTypeLocked(id, type);
}
} // namespace impl

sptr<RSScreenManager> CreateOrGetScreenManager()
{
    return impl::RSScreenManager::GetInstance();
}
} // namespace Rosen
} // namespace OHOS
