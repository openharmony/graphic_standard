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

#include "pipeline/rs_processor_factory.h"

#include "pipeline/rs_hardware_processor.h"
#include "pipeline/rs_software_processor.h"

namespace OHOS {
namespace Rosen {

std::shared_ptr<RSProcessor> RSProcessorFactory::CreateProcessor(ScreenState state)
{
    switch(state) {
        case ScreenState::PRODUCER_SURFACE_ENABLE:
            return std::make_shared<RSSoftwareProcessor>();
        case ScreenState::HDI_OUTPUT_ENABLE:
            return std::make_shared<RSHardwareProcessor>();
        default:
            return nullptr;
    }
}

}
}