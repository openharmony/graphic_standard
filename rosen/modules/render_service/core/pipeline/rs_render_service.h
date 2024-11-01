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

#ifndef RENDER_SERVICE_PIPELINE_RS_RENDER_SERVICE_H
#define RENDER_SERVICE_PIPELINE_RS_RENDER_SERVICE_H
#include <map>
#include <unordered_set>

#include "screen_manager/rs_screen_manager.h"
#include "transaction/rs_render_service_stub.h"
#include "vsync_controller.h"
#include "vsync_distributor.h"

namespace OHOS {
namespace Rosen {
class RSMainThread;
class RSSyncTask;
class RSRenderServiceConnection;

class RSRenderService : public RSRenderServiceStub {
public:
    RSRenderService();
    ~RSRenderService() noexcept;

    RSRenderService(const RSRenderService&) = delete;
    RSRenderService& operator=(const RSRenderService&) = delete;

    bool Init();
    void Run();

private:
    int Dump(int fd, const std::vector<std::u16string>& args) override;
    void DoDump(std::unordered_set<std::u16string>& argSets, std::string& dumpString) const;
    sptr<RSIRenderServiceConnection> CreateConnection(const sptr<RSIConnectionToken>& token) override;
    void RemoveConnection(sptr<IRemoteObject> token);

    RSMainThread* mainThread_ = nullptr;
    sptr<RSScreenManager> screenManager_;

    friend class RSRenderServiceConnection;
    mutable std::mutex mutex_;
    std::map<sptr<IRemoteObject>, sptr<RSIRenderServiceConnection>> connections_;

    sptr<VSyncController> rsVSyncController_;
    sptr<VSyncController> appVSyncController_;

    sptr<VSyncDistributor> rsVSyncDistributor_;
    sptr<VSyncDistributor> appVSyncDistributor_;
};
} // Rosen
} // OHOS

#endif // RENDER_SERVICE_PIPELINE_RS_RENDER_SERVICE_H
