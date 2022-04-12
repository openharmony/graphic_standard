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

#include "surface_buffer_impl.h"

#include <mutex>

#include <message_parcel.h>
#include <securec.h>

#include "buffer_log.h"
#include "buffer_manager.h"
#include "buffer_extra_data_impl.h"

namespace OHOS {
SurfaceBufferImpl::SurfaceBufferImpl()
{
    {
        static std::mutex mutex;
        mutex.lock();

        static int sequence_number_ = 0;
        sequenceNumber_ = sequence_number_++;

        mutex.unlock();
    }
    bedata_ = new BufferExtraDataImpl;
    handle_ = nullptr;
    eglData_ = nullptr;
    BLOGD("ctor +[%{public}d]", sequenceNumber_);
}

SurfaceBufferImpl::SurfaceBufferImpl(int seqNum)
{
    sequenceNumber_ = seqNum;
    handle_ = nullptr;
    bedata_ = new BufferExtraDataImpl;
    BLOGD("ctor =[%{public}d]", sequenceNumber_);
}

SurfaceBufferImpl::~SurfaceBufferImpl()
{
    BLOGD("dtor ~[%{public}d] handle_ %{public}p", sequenceNumber_, handle_);
    if (handle_) {
        if (handle_->virAddr != nullptr) {
            BLOGD("dtor ~[%{public}d] virAddr %{public}p", sequenceNumber_, handle_->virAddr);
            BufferManager::GetInstance()->Unmap(handle_);
        }
        FreeBufferHandle(handle_);
    }
    eglData_ = nullptr;
}

SurfaceBufferImpl *SurfaceBufferImpl::FromBase(const sptr<SurfaceBuffer>& buffer)
{
    return static_cast<SurfaceBufferImpl*>(buffer.GetRefPtr());
}

BufferHandle *SurfaceBufferImpl::GetBufferHandle() const
{
    return handle_;
}

GSError SurfaceBufferImpl::SetSurfaceBufferWidth(int32_t width)
{
    surfaceBufferWidth_ = width;
    return GSERROR_OK;
}

GSError SurfaceBufferImpl::SetSurfaceBufferHeight(int32_t height)
{
    surfaceBufferHeight_ = height;
    return GSERROR_OK;
}

int32_t SurfaceBufferImpl::GetSurfaceBufferHeight() const
{
    return surfaceBufferHeight_;
}

int32_t SurfaceBufferImpl::GetSurfaceBufferWidth() const
{
    return surfaceBufferWidth_;
}

GSError SurfaceBufferImpl::SetSurfaceBufferColorGamut(ColorGamut colorGamut)
{
    surfaceBufferColorGamut_ = colorGamut;
    return GSERROR_OK;
}

ColorGamut SurfaceBufferImpl::GetSurfaceBufferColorGamut() const
{
    return surfaceBufferColorGamut_;
}

GSError SurfaceBufferImpl::SetSurfaceBufferTransform(TransformType transform)
{
    transform_ = transform;
    return GSERROR_OK;
}

TransformType SurfaceBufferImpl::GetSurfaceBufferTransform() const
{
    return transform_;
}

int32_t SurfaceBufferImpl::GetWidth() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return -1;
    }
    return handle_->width;
}

int32_t SurfaceBufferImpl::GetHeight() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return -1;
    }
    return handle_->height;
}

int32_t SurfaceBufferImpl::GetStride() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return -1;
    }
    return handle_->stride;
}

int32_t SurfaceBufferImpl::GetFormat() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return -1;
    }
    return handle_->format;
}

int64_t SurfaceBufferImpl::GetUsage() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return -1;
    }
    return handle_->usage;
}

uint64_t SurfaceBufferImpl::GetPhyAddr() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return 0;
    }
    return handle_->phyAddr;
}

int32_t SurfaceBufferImpl::GetKey() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return -1;
    }
    return handle_->key;
}

void *SurfaceBufferImpl::GetVirAddr() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return nullptr;
    }
    return handle_->virAddr;
}

int32_t SurfaceBufferImpl::GetFileDescriptor() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return -1;
    }
    return handle_->fd;
}

uint32_t SurfaceBufferImpl::GetSize() const
{
    if (handle_ == nullptr) {
        BLOGW("handle is nullptr");
        return 0;
    }
    return handle_->size;
}

void SurfaceBufferImpl::SetExtraData(const sptr<BufferExtraData> &bedata)
{
    bedata_ = bedata;
}

const sptr<BufferExtraData>& SurfaceBufferImpl::GetExtraData() const
{
    return bedata_;
}

void SurfaceBufferImpl::SetBufferHandle(BufferHandle *handle)
{
    handle_ = handle;
}

void SurfaceBufferImpl::WriteToMessageParcel(MessageParcel &parcel)
{
    if (handle_ == nullptr) {
        BLOGE("Failure, Reason: handle_ is nullptr");
        return;
    }

    bool ret = WriteBufferHandle(parcel, *handle_);
    if (ret == false) {
        BLOGE("Failure, Reason: WriteBufferHandle return false");
    }
}

int32_t SurfaceBufferImpl::GetSeqNum() const
{
    return sequenceNumber_;
}

sptr<EglData> SurfaceBufferImpl::GetEglData() const
{
    return eglData_;
}

void SurfaceBufferImpl::SetEglData(const sptr<EglData>& data)
{
    eglData_ = data;
}
} // namespace OHOS
