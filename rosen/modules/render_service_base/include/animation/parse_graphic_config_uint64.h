/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef RENDER_SERVICE_BASE_ANIMATION_PARSE_GRAPHIC_CONFIG_UINT64_H
#define RENDER_SERVICE_BASE_ANIMATION_PARSE_GRAPHIC_CONFIG_UINT64_H

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

namespace OHOS {
namespace Rosen {
/*
 * Parse a whole-token decimal uint64 from graphic animation config text
 * (/etc/rosen/property.config PROPERTY / ID fields).
 * Reject empty, overflow, leading/trailing junk, '+', hex, floats, and negatives.
 * Valid in-range values keep the same numeric result as strtoull on digit-only input.
 */
inline bool ParseGraphicConfigUint64(std::string_view text, uint64_t &out)
{
    if (text.empty()) {
        return false;
    }
    uint64_t value = 0;
    auto result = std::from_chars(text.data(), text.data() + text.size(), value);
    if (result.ec != std::errc() || result.ptr != text.data() + text.size()) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseGraphicConfigUint64(const std::string &text, uint64_t &out)
{
    return ParseGraphicConfigUint64(std::string_view(text), out);
}

inline bool ParseGraphicConfigUint64(const char *text, uint64_t &out)
{
    if (text == nullptr) {
        return false;
    }
    return ParseGraphicConfigUint64(std::string_view(text), out);
}
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_ANIMATION_PARSE_GRAPHIC_CONFIG_UINT64_H
