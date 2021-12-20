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

#include "wmclient_native_test_23.h"

#include <cstdio>
#include <fstream>
#include <sstream>

#include <display_type.h>
#include <gslogger.h>
#include <option_parser.h>
#include <securec.h>
#include <window_manager.h>
#include <zlib.h>

#include "inative_test.h"
#include "native_test_class.h"
#include "util.h"

using namespace OHOS;

namespace {
class WMClientNativeTest23 : public INativeTest, public IScreenShotCallback {
public:
    std::string GetDescription() const override
    {
        constexpr const char *desc = "screen capture (--stride=200 (ms) --total-time=3000 (ms))";
        return desc;
    }

    std::string GetDomain() const override
    {
        constexpr const char *domain = "wmclient";
        return domain;
    }

    int32_t GetID() const override
    {
        constexpr int32_t id = 23;
        return id;
    }

    AutoLoadService GetAutoLoadService() const override
    {
        return AutoLoadService::WindowManager;
    }

    void Run(int32_t argc, const char **argv) override
    {
        OptionParser parser;
        parser.AddOption("s", "stride", stride);
        parser.AddOption("t", "total-time", totalTime);
        if (parser.Parse(argc, argv)) {
            GSLOG2SE(ERROR) << parser.GetErrorString();
            ExitTest();
            return;
        }

        Run2();
    }

    void Run2()
    {
        windowManager->ListenNextScreenShot(0, this);
        totalTime -= stride - 16;
        if (totalTime < 0) {
            GSLOG2SO(INFO) << "ScreenCapture Complete";
            ExitTest();
        } else {
            // PostTask(std::bind(&WMClientNativeTest23::Run2, this), stride);
            PostTask(std::bind(&WMClientNativeTest23::Run2, this));
        }
    }

    int32_t Compress(std::unique_ptr<uint8_t[]> &src, unsigned long &ul, const uint8_t *data, uint32_t size) const
    {
        auto ret = compress(src.get(), &ul, data, size);
        if (ret) {
            GSLOG2SE(ERROR) << "compress failed with " << ret;
        }

        return ret;
    }

    void WriteInt(std::ofstream &ofs, int32_t integer)
    {
        ofs.write(reinterpret_cast<const char *>(&integer), sizeof(integer));
    }

    void OnScreenShot(const struct WMImageInfo &info) override
    {
        auto ptr = static_cast<const uint8_t *>(info.data);
        auto compressed = std::make_unique<uint8_t[]>(compressBound(info.size));
        unsigned long ulength = info.size;

        if (Compress(compressed, ulength, ptr, info.size) != Z_OK) {
            printf("compress failed\n");
            ExitTest();
            return;
        }

        static bool first = true;
        if (first) {
            first = false;
            std::ofstream rawDataFile(captureFilepath, std::ofstream::binary | std::ofstream::out);
            rawDataFile.write("RAW.diff", 0x8);
            WriteInt(rawDataFile, info.width);
            WriteInt(rawDataFile, info.height);
            GSLOG2SO(INFO) << "generate capture at /data/screen_capture.raw";
        }

        std::ofstream rawDataFile(captureFilepath, std::ofstream::binary | std::ofstream::app);
        WriteInt(rawDataFile, 0x2); // header_type
        WriteInt(rawDataFile, 0); // offset
        WriteInt(rawDataFile, info.size); // total length
        WriteInt(rawDataFile, ulength); // ulength
        rawDataFile.write(reinterpret_cast<const char *>(compressed.get()), ulength);

        // for SIGBUS ADRALN
        if (ulength % 0x4 != 0) {
            for (uint32_t i = 0; i < 0x4 - (ulength % 0x4); i++) {
                const char *nosence = "\0";
                rawDataFile.write(nosence, 1);
            }
        }
    }

private:
    int32_t totalTime = 3000;
    int32_t stride = 200;
    static constexpr const char *captureFilepath = "/data/screen_capture.raw";
} g_autoload;
} // namespace
