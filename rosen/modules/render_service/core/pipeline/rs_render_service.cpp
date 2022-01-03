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

#include "pipeline/rs_render_service.h"

#include <iservice_registry.h>
#include <string>
#include <system_ability_definition.h>

#include "pipeline/rs_main_thread.h"
#include "pipeline/rs_render_node_map.h"
#include "pipeline/rs_render_service_listener.h"
#include "pipeline/rs_surface_render_node.h"
#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
RSRenderService& RSRenderService::GetInstance()
{
    static RSRenderService instance;
    return instance;
}

RSRenderService::RSRenderService() {}

RSRenderService::~RSRenderService() noexcept {}

void RSRenderService::Init()
{
    screenManager_ = CreateOrGetScreenManager();
    if (screenManager_ == nullptr || !screenManager_->Init()) {
        ROSEN_LOGI("RSRenderService CreateOrGetScreenManager fail");
    }

    mainThread_ = RSMainThread::Instance();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    samgr->AddSystemAbility(RENDER_SERVICE, &GetInstance());
}

void RSRenderService::Run()
{
    ROSEN_LOGI("RSRenderService::Run");
    mainThread_->Start();
}

void RSRenderService::CommitTransaction(std::unique_ptr<RSTransactionData>& transactionData)
{
    mainThread_->RecvRSTransactionData(transactionData);
}

sptr<Surface> RSRenderService::CreateNodeAndSurface(const RSSurfaceRenderNodeConfig& config)
{
    std::shared_ptr<RSSurfaceRenderNode> node = std::make_shared<RSSurfaceRenderNode>(config);
    if (node == nullptr) {
        ROSEN_LOGE("RSRenderService::CreateNodeAndSurface CreateNode fail");
        return nullptr;
    }
    sptr<Surface> surface = Surface::CreateSurfaceAsConsumer();
    if (surface == nullptr) {
        ROSEN_LOGE("RSRenderService::CreateNodeAndSurface get consumer surface fail");
        return nullptr;
    }
    node->SetConsumer(surface);
    auto baseNodePtr = std::static_pointer_cast<RSBaseRenderNode>(node);
    std::function<void()> registerNode = [baseNodePtr, this]() -> void {
        this->mainThread_->GetContext().GetNodeMap().RegisterRenderNode(baseNodePtr);
    };
    mainThread_->PostTask(registerNode);
    std::weak_ptr<RSSurfaceRenderNode> surfaceRenderNode(node);
    sptr<IBufferConsumerListener> listener = new RSRenderServiceListener(surfaceRenderNode);
    SurfaceError ret = surface->RegisterConsumerListener(listener);
    if (ret != SURFACE_ERROR_OK) {
        ROSEN_LOGE("RSRenderService::CreateNodeAndSurface Register Consumer Listener fail");
        return nullptr;
    }
    return surface;
}

ScreenId RSRenderService::GetDefaultScreenId()
{
    if (screenManager_ == nullptr) {
        return INVALID_SCREEN_ID;
    }
    return screenManager_->GetDefaultScreenId();
}

ScreenId RSRenderService::CreateVirtualScreen(
    const std::string &name,
    uint32_t width,
    uint32_t height,
    sptr<Surface> surface,
    ScreenId mirrorId,
    int32_t flags)
{
    if (screenManager_ == nullptr) {
        return INVALID_SCREEN_ID;
    }

    return screenManager_->CreateVirtualScreen(name, width, height, surface, mirrorId, flags);
}

void RSRenderService::RemoveVirtualScreen(ScreenId id)
{
    if (screenManager_ == nullptr) {
        return;
    }
    screenManager_->RemoveVirtualScreen(id);
}

void RSRenderService::SetScreenChangeCallback(sptr<RSIScreenChangeCallback> callback)
{
    if (screenManager_ == nullptr) {
        return;
    }
    screenManager_->AddScreenChangeCallback(callback);
}

void RSRenderService::SetScreenActiveMode(ScreenId id, uint32_t modeId)
{
    if (screenManager_ == nullptr) {
        return;
    }
    screenManager_->SetScreenActiveMode(id, modeId);
}

void RSRenderService::SetScreenPowerStatus(ScreenId id, ScreenPowerStatus status)
{
    if (screenManager_ == nullptr) {
        return;
    }
    screenManager_->SetScreenPowerStatus(id, status);
}

RSScreenModeInfo RSRenderService::GetScreenActiveMode(ScreenId id)
{
    RSScreenModeInfo screenModeInfo;
    if (screenManager_ == nullptr) {
        return screenModeInfo;
    }
    screenManager_->GetScreenActiveMode(id, screenModeInfo);
    return screenModeInfo;
}

std::vector<RSScreenModeInfo> RSRenderService::GetScreenSupportedModes(ScreenId id)
{
    if (screenManager_ == nullptr) {
        return {};
    }
    return screenManager_->GetScreenSupportedModes(id);
}

RSScreenCapability RSRenderService::GetScreenCapability(ScreenId id)
{
    RSScreenCapability screenCapability;
    if (screenManager_ == nullptr) {
        return screenCapability;
    }
    return screenManager_->GetScreenCapability(id);
}

ScreenPowerStatus RSRenderService::GetScreenPowerStatus(ScreenId id)
{
    if (screenManager_ == nullptr) {
        return INVAILD_POWER_STATUS;
    }
    return screenManager_->GetScreenPowerStatus(id);
}

RSScreenData RSRenderService::GetScreenData(ScreenId id)
{
    RSScreenData screenData;
    if (screenManager_ == nullptr) {
        return screenData;
    }
    return screenManager_->GetScreenData(id);
}
} // namespace Rosen
} // namespace OHOS
