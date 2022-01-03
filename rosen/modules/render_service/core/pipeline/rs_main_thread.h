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

#ifndef RS_MAIN_THREAD
#define RS_MAIN_THREAD

#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include "common/rs_thread_handler.h"
#include "common/rs_thread_looper.h"
#include "pipeline/rs_context.h"
#include "platform/drawing/rs_vsync_client.h"

namespace OHOS {
namespace Rosen {
class RSTransactionData;

class RSMainThread {
public:
    static RSMainThread* Instance();

    void Start();
    void RecvRSTransactionData(std::unique_ptr<RSTransactionData>& rsTransactionData);
    void RequestNextVSync();
    void PostTask(RSTaskMessage::RSTask task);

    RSContext& GetContext()
    {
        return context_;
    }
    const RSContext& GetContext() const
    {
        return context_;
    }

    std::thread::id Id() const
    {
        return mainThreadId_;
    }
private:
    RSMainThread();
    ~RSMainThread() noexcept;
    RSMainThread(const RSMainThread&) = delete;
    RSMainThread(const RSMainThread&&) = delete;
    RSMainThread& operator=(const RSMainThread&) = delete;
    RSMainThread& operator=(const RSMainThread&&) = delete;

    void OnVsync(uint64_t timestamp);
    void ProcessCommand();
    void Draw();
    std::mutex transitionDataMutex_;
    std::unique_ptr<RSThreadLooper> threadLooper_ = nullptr;
    std::unique_ptr<RSThreadHandler> threadHandler_ = nullptr;
    RSTaskHandle taskHandle_ = nullptr;
    RSTaskMessage::RSTask mainLoop_;
    std::unique_ptr<RSVsyncClient> vsyncClient_ = nullptr;
    std::queue<std::unique_ptr<RSTransactionData>> cacheCommandQueue_;
    std::queue<std::unique_ptr<RSTransactionData>> effectCommandQueue_;

    RSContext context_;

    std::thread::id mainThreadId_;
};
} // namespace Rosen
} // namespace OHOS

#endif