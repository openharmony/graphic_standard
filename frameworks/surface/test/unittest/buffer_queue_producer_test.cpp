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

#include "buffer_queue_producer_test.h"

#include "buffer_consumer_listener.h"

using namespace OHOS;

namespace OHOS {
void BufferQueueProducerTest::SetUpTestCase()
{
    bq = new BufferQueue("test");
    bq->Init();
    sptr<IBufferConsumerListener> listener = new BufferConsumerListener();
    bq->RegisterConsumerListener(listener);
    bqp = new BufferQueueProducer(bq);
}

void BufferQueueProducerTest::TearDownTestCase()
{
    bq = nullptr;
    bqp = nullptr;
}

namespace {
HWTEST_F(BufferQueueProducerTest, QueueSize, testing::ext::TestSize.Level0)
{
    ASSERT_EQ(bqp->GetQueueSize(), (uint32_t)SURFACE_DEFAULT_QUEUE_SIZE);

    SurfaceError ret = bqp->SetQueueSize(2);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->SetQueueSize(SURFACE_MAX_QUEUE_SIZE + 1);
    ASSERT_NE(ret, SURFACE_ERROR_OK);

    ASSERT_EQ(bqp->GetQueueSize(), 2u);
    ASSERT_EQ(bq->queueSize_, 2u);
}

HWTEST_F(BufferQueueProducerTest, ReqCan, testing::ext::TestSize.Level0)
{
    sptr<SurfaceBuffer> buffer;
    int32_t releaseFence;
    int32_t sequence;

    SurfaceError ret = bqp->RequestBuffer(sequence,
                                          buffer, releaseFence, requestConfig, deletingBuffers);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->CancelBuffer(sequence, bedata);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);
}

HWTEST_F(BufferQueueProducerTest, ReqCanCan, testing::ext::TestSize.Level0)
{
    sptr<SurfaceBuffer> buffer;
    int32_t releaseFence;
    int32_t sequence;

    SurfaceError ret = bqp->RequestBuffer(sequence,
                                          buffer, releaseFence, requestConfig, deletingBuffers);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->CancelBuffer(sequence, bedata);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->CancelBuffer(sequence, bedata);
    ASSERT_NE(ret, SURFACE_ERROR_OK);
}

HWTEST_F(BufferQueueProducerTest, ReqReqReqCanCan, testing::ext::TestSize.Level0)
{
    sptr<SurfaceBuffer> buffer1;
    sptr<SurfaceBuffer> buffer2;
    sptr<SurfaceBuffer> buffer3;
    int32_t releaseFence;
    int32_t sequence1;
    int32_t sequence2;
    int32_t sequence3;
    SurfaceError ret;

    ret = bqp->RequestBuffer(sequence1, buffer1, releaseFence, requestConfig, deletingBuffers);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);
    ASSERT_EQ(buffer1, nullptr);

    ret = bqp->RequestBuffer(sequence2, buffer2, releaseFence, requestConfig, deletingBuffers);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);
    ASSERT_NE(buffer2, nullptr);

    ret = bqp->RequestBuffer(sequence3, buffer3, releaseFence, requestConfig, deletingBuffers);
    ASSERT_NE(ret, SURFACE_ERROR_OK);
    ASSERT_EQ(buffer3, nullptr);

    ret = bqp->CancelBuffer(sequence1, bedata);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->CancelBuffer(sequence2, bedata);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->CancelBuffer(sequence3, bedata);
    ASSERT_NE(ret, SURFACE_ERROR_OK);
}

HWTEST_F(BufferQueueProducerTest, ReqFlu, testing::ext::TestSize.Level0)
{
    sptr<SurfaceBuffer> buffer;
    int32_t releaseFence;
    int32_t flushFence;
    int32_t sequence;

    SurfaceError ret = bqp->RequestBuffer(sequence, buffer, releaseFence, requestConfig, deletingBuffers);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->FlushBuffer(sequence, bedata, -1, flushConfig);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    sptr<SurfaceBufferImpl> bufferImpl = static_cast<SurfaceBufferImpl*>(buffer.GetRefPtr());
    ret = bq->AcquireBuffer(bufferImpl, flushFence, timestamp, damage);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bq->ReleaseBuffer(bufferImpl, -1);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);
}

HWTEST_F(BufferQueueProducerTest, ReqFluFlu, testing::ext::TestSize.Level0)
{
    sptr<SurfaceBuffer> buffer;
    int32_t releaseFence;
    int32_t flushFence;
    int32_t sequence;

    SurfaceError ret = bqp->RequestBuffer(sequence, buffer, releaseFence, requestConfig, deletingBuffers);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->FlushBuffer(sequence, bedata, -1, flushConfig);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bqp->FlushBuffer(sequence, bedata, -1, flushConfig);
    ASSERT_NE(ret, SURFACE_ERROR_OK);

    sptr<SurfaceBufferImpl> bufferImpl = static_cast<SurfaceBufferImpl*>(buffer.GetRefPtr());
    ret = bq->AcquireBuffer(bufferImpl, flushFence, timestamp, damage);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);

    ret = bq->ReleaseBuffer(bufferImpl, -1);
    ASSERT_EQ(ret, SURFACE_ERROR_OK);
}
}
} // namespace OHOS
