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

#include "command/rs_root_node_command.h"

#include "pipeline/rs_root_render_node.h"
#include "pipeline/rs_surface_render_node.h"
#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {

void RootNodeCommandHelper::Create(RSContext& context, NodeId id)
{
    auto node = std::make_shared<RSRootRenderNode>(id, context.weak_from_this());
    context.GetMutableNodeMap().RegisterRenderNode(node);
}

void RootNodeCommandHelper::AttachRSSurfaceNode(RSContext& context, NodeId id, NodeId surfaceNodeId)
{
    if (auto node = context.GetNodeMap().GetRenderNode<RSRootRenderNode>(id)) {
        node->AttachRSSurfaceNode(surfaceNodeId);
        context.GetGlobalRootRenderNode()->AddChild(node);
    }
}

void RootNodeCommandHelper::AttachToUniSurfaceNode(RSContext& context, NodeId id, NodeId surfaceNodeId)
{
    auto& nodeMap = context.GetNodeMap();
    auto parent = nodeMap.GetRenderNode<RSSurfaceRenderNode>(surfaceNodeId);
    auto node = nodeMap.GetRenderNode<RSRootRenderNode>(id);
    if (!parent) {
        RS_LOGE("unirender: RootNodeCommandHelper::AddToSurfaceNode no parent surfaceNode");
    }
    if (!node) {
        RS_LOGE("unirender: RootNodeCommandHelper::AddToSurfaceNode no RootRenderNode");
    }
    if (node && parent) {
        parent->AddChild(node);
    }
}

} // namespace Rosen
} // namespace OHOS
