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

#ifndef RENDER_SERVICE_BASE_TRANSACTION_RS_MARSHALLING_HELPER_H
#define RENDER_SERVICE_BASE_TRANSACTION_RS_MARSHALLING_HELPER_H

#include <memory>
#ifdef ROSEN_OHOS

#include <parcel.h>

#include "common/rs_common_def.h"

template<typename T>
class sk_sp;
class SkData;
class SkDrawable;
class SkFlattenable;
class SkImage;
class SkImageFilter;
class SkPaint;
class SkPath;
class SkPicture;
class SkRegion;
class SkTextBlob;
class SkVertices;

namespace OHOS {
namespace Rosen {
class DrawCmdList;
class RSFilter;
class RSImage;
class RSMask;
class RSPath;
class RSShader;
template<typename T>
class RSRenderCurveAnimation;
template<typename T>
class RSRenderKeyframeAnimation;
class RSRenderPathAnimation;
class RSRenderTransition;
class RSRenderTransitionEffect;

class RSMarshallingHelper {
public:
    static bool WriteToParcel(Parcel &parcel, const void* data, size_t size);
    static const void* ReadFromParcel(Parcel& parcel, size_t size);

    // default marshalling and unmarshalling method for POD types
    // [PLANNING]: implement marshalling & unmarshalling methods for other types (e.g. RSImage, drawCMDList)
    template<typename T>
    static bool Marshalling(Parcel& parcel, const T& val)
    {
        return parcel.WriteUnpadBuffer(&val, sizeof(T));
    }
    template<typename T>
    static bool Unmarshalling(Parcel& parcel, T& val)
    {
        if (const uint8_t* buff = parcel.ReadUnpadBuffer(sizeof(T))) {
            val = *(reinterpret_cast<const T*>(buff));
            return true;
        }
        return false;
    }

    template<typename T>
    static bool Marshalling(Parcel& parcel, const T* val, int count)
    {
        return parcel.WriteUnpadBuffer(val, count * sizeof(T));
    }
    template<typename T>
    static bool Unmarshalling(Parcel& parcel, T*& val, int count)
    {
        if (const uint8_t* buff = parcel.ReadUnpadBuffer(count * sizeof(T))) {
            val = reinterpret_cast<const T*>(buff);
            return true;
        }
        return false;
    }

    // reloaded marshalling & unmarshalling function for types
#define DECLARE_FUNCTION_OVERLOAD(TYPE)                       \
    static bool Marshalling(Parcel& parcel, const TYPE& val); \
    static bool Unmarshalling(Parcel& parcel, TYPE& val);

    // basic types
    DECLARE_FUNCTION_OVERLOAD(bool)
    DECLARE_FUNCTION_OVERLOAD(int8_t)
    DECLARE_FUNCTION_OVERLOAD(uint8_t)
    DECLARE_FUNCTION_OVERLOAD(int16_t)
    DECLARE_FUNCTION_OVERLOAD(uint16_t)
    DECLARE_FUNCTION_OVERLOAD(int32_t)
    DECLARE_FUNCTION_OVERLOAD(uint32_t)
    DECLARE_FUNCTION_OVERLOAD(int64_t)
    DECLARE_FUNCTION_OVERLOAD(uint64_t)
    DECLARE_FUNCTION_OVERLOAD(float)
    DECLARE_FUNCTION_OVERLOAD(double)
    // skia types
    DECLARE_FUNCTION_OVERLOAD(SkPath)
    DECLARE_FUNCTION_OVERLOAD(SkPaint)
    DECLARE_FUNCTION_OVERLOAD(SkRegion)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkData>)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkFlattenable>)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkTextBlob>)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkPicture>)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkDrawable>)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkImageFilter>)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkImage>)
    DECLARE_FUNCTION_OVERLOAD(sk_sp<SkVertices>)
    // RS types
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSShader>)
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSPath>)
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSFilter>)
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSMask>)
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSImage>)
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<DrawCmdList>)
    // animation
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSRenderPathAnimation>)
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSRenderTransition>)
    DECLARE_FUNCTION_OVERLOAD(std::shared_ptr<RSRenderTransitionEffect>)
#undef DECLARE_FUNCTION_OVERLOAD

    // reloaded marshalling & unmarshalling function for animation template
#define DECLARE_TEMPLATE_OVERLOAD(TEMPLATE)                                           \
    template<typename T>                                                              \
    static bool Marshalling(Parcel& parcel, const std::shared_ptr<TEMPLATE<T>>& val); \
    template<typename T>                                                              \
    static bool Unmarshalling(Parcel& parcel, std::shared_ptr<TEMPLATE<T>>& val);

    DECLARE_TEMPLATE_OVERLOAD(RSRenderCurveAnimation)
    DECLARE_TEMPLATE_OVERLOAD(RSRenderKeyframeAnimation)
#undef DECLARE_TEMPLATE_OVERLOAD

    // reloaded marshalling & unmarshalling function for std::vector
    template<typename T>
    static bool Marshalling(Parcel& parcel, const std::vector<T>& val);
    template<typename T>
    static bool Unmarshalling(Parcel& parcel, std::vector<T>& val);
private:
    static void ReleaseMemory(void* data, int* fd, size_t size);
    inline static std::atomic<uint32_t> shmemCount = 0;
    static constexpr size_t MAX_DATA_SIZE = 128 * 1024 * 1024; // 128M
    static constexpr size_t MIN_DATA_SIZE = 8 * 1024;         // 8k
};

} // namespace Rosen
} // namespace OHOS

#endif // ROSEN_OHOS
#endif // RENDER_SERVICE_BASE_TRANSACTION_RS_MARSHALLING_HELPER_H
