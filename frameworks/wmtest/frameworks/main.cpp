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

#include <iomanip>
#include <iostream>
#include <securec.h>
#include <sstream>
#include <unistd.h>
#include <vector>

#include <vsync_helper.h>
#include <window_manager.h>
#include <option_parser.h>

#include "inative_test.h"
#include "native_test_class.h"

using namespace OHOS;

class MainOption : public OptionParser {
public:
    MainOption();
    int32_t Parse(int32_t argc, const char **argv);

    // attr
    std::string domain = "";
    int32_t testcase = -1;
    int32_t displayID = 0;
};

MainOption::MainOption()
{
    AddArguments(domain);
    AddArguments(testcase);
    AddOption("d", "display", displayID);
}

int32_t MainOption::Parse(int32_t argc, const char **argv)
{
    // ignore wmtest(argv0)
    return OptionParser::Parse(argc - 1, argv + 1);
}

namespace {
void Usage(const char *argv0)
{
    std::cerr << "Usage: " << argv0 << " [option] type id" << std::endl;
    std::cerr << "-d, --display[=0]  Created Window's Display ID" << std::endl;
    auto visitFunc = [](const INativeTest *test) {
        std::stringstream ss;
        ss << test->GetDomain() << ", id=";
        ss << test->GetID() << ": ";
        ss << test->GetDescription();
        if (test->GetLastTime() != INativeTest::LAST_TIME_FOREVER) {
            constexpr double msecToSec = 1000.0;
            ss << " (last " << std::setprecision(1) << test->GetLastTime() / msecToSec << "s)";
        }
        std::cout << ss.str() << std::endl;
    };
    INativeTest::VisitTests(visitFunc);
}
} // namespace

int32_t main(int32_t argc, const char **argv)
{
    // parse option
    MainOption option;
    if (option.Parse(argc, argv)) {
        std::cerr << option.GetErrorString() << std::endl;
        Usage(argv[0]);
        return 1;
    }

    // find test
    INativeTest *found = nullptr;
    auto visitFunc = [&option, &found](INativeTest *test) {
        if (test->GetDomain() == option.domain && test->GetID() == option.testcase) {
            found = test;
        }
    };
    INativeTest::VisitTests(visitFunc);
    if (found == nullptr) {
        printf("not found test %d\n", option.testcase);
        return 1;
    }

    // default value assign
    NativeTestFactory::defaultDisplayID = option.displayID;

    // run test
    auto runner = AppExecFwk::EventRunner::Create(false);
    auto handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    handler->PostTask(std::bind(&INativeTest::Run, found, option.GetSkippedArgc(), option.GetSkippedArgv()));
    if (found->GetLastTime() != INativeTest::LAST_TIME_FOREVER) {
        handler->PostTask(std::bind(&AppExecFwk::EventRunner::Stop, runner), found->GetLastTime());
    }

    printf("%d %s run! pid=%d\n", found->GetID(), found->GetDescription().c_str(), getpid());
    runner->Run();
    return 0;
}
