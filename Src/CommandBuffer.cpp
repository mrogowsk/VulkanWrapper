/*
Copyright(c) 2018 Marcus Rogowsky

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "VulkanWrapper.h"

#include "Error.h"

namespace vkw
{

void CommandBuffer::Begin(VkCommandBufferUsageFlags flags) const
{
    BeginExt(nullptr, flags);
}

void CommandBuffer::BeginExt(const void *pNext, VkCommandBufferUsageFlags flags) const
{
    assert(cmdBuffer_);
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, pNext, flags, nullptr};
    VK_CALL(vkBeginCommandBuffer(cmdBuffer_, &beginInfo));
}

void CommandBuffer::BeginSecondary(VkCommandBufferUsageFlags flags, VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
                                   VkQueryPipelineStatisticFlags pipelineStatistics) const
{
    BeginSecondaryExt(nullptr, nullptr, {}, 0, {}, flags, occlusionQueryEnable, queryFlags, pipelineStatistics);
}

void CommandBuffer::BeginSecondary(const RenderPass &renderPass, uint32_t subpass, VkCommandBufferUsageFlags flags, VkBool32 occlusionQueryEnable,
                                   VkQueryControlFlags queryFlags, VkQueryPipelineStatisticFlags pipelineStatistics) const
{
    BeginSecondaryExt(nullptr, nullptr, renderPass, subpass, {}, flags, occlusionQueryEnable, queryFlags, pipelineStatistics);
}

void CommandBuffer::BeginSecondary(const RenderPass &renderPass, uint32_t subpass, const Framebuffer &framebuffer, VkCommandBufferUsageFlags flags,
                                   VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags, VkQueryPipelineStatisticFlags pipelineStatistics) const
{
    BeginSecondaryExt(nullptr, nullptr, renderPass, subpass, framebuffer, flags, occlusionQueryEnable, queryFlags, pipelineStatistics);
}

void CommandBuffer::BeginSecondaryExt(const void *pBeginInfoNext, const void *pInheritanceInfoNext, VkCommandBufferUsageFlags flags, VkBool32 occlusionQueryEnable,
                                      VkQueryControlFlags queryFlags, VkQueryPipelineStatisticFlags pipelineStatistics) const
{
    BeginSecondaryExt(pBeginInfoNext, pInheritanceInfoNext, {}, 0, {}, flags, occlusionQueryEnable, queryFlags, pipelineStatistics);
}

void CommandBuffer::BeginSecondaryExt(const void *pBeginInfoNext, const void *pInheritanceInfoNext, const RenderPass &renderPass, uint32_t subpass,
                                      VkCommandBufferUsageFlags flags, VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags, VkQueryPipelineStatisticFlags pipelineStatistics) const
{
    BeginSecondaryExt(pBeginInfoNext, pInheritanceInfoNext, renderPass, subpass, {}, flags, occlusionQueryEnable, queryFlags, pipelineStatistics);
}

void CommandBuffer::BeginSecondaryExt(const void *pBeginInfoNext, const void *pInheritanceInfoNext, const RenderPass &renderPass, uint32_t subpass,
                                      const Framebuffer &framebuffer, VkCommandBufferUsageFlags flags, VkBool32 occlusionQueryEnable, VkQueryControlFlags queryFlags,
                                      VkQueryPipelineStatisticFlags pipelineStatistics) const
{
    assert(cmdBuffer_);
    VkCommandBufferInheritanceInfo inheritanceInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO, pInheritanceInfoNext, VkRenderPass(renderPass), subpass,
                                                      VkFramebuffer(framebuffer), occlusionQueryEnable, queryFlags, pipelineStatistics};
    VkCommandBufferBeginInfo beginInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, pBeginInfoNext, flags, &inheritanceInfo};
    VK_CALL(vkBeginCommandBuffer(cmdBuffer_, &beginInfo));
}

void CommandBuffer::Reset(VkCommandBufferResetFlags flags) const
{
    assert(cmdBuffer_);
    VK_CALL(vkResetCommandBuffer(cmdBuffer_, flags));
}

void CommandBuffer::End() const
{
    assert(cmdBuffer_);
    VK_CALL(vkEndCommandBuffer(cmdBuffer_));
}

void CommandBuffer::FillBuffer(const Buffer &dstBuffer, uint32_t data, VkDeviceSize dstOffset, VkDeviceSize size) const
{
    assert(cmdBuffer_ && dstBuffer);
    vkCmdFillBuffer(cmdBuffer_, VkBuffer(dstBuffer), dstOffset, size, data);
}

void CommandBuffer::UpdateBuffer(const Buffer &dstBuffer, const void *pData, VkDeviceSize size) const
{
    UpdateBuffer(dstBuffer, pData, 0, size);
}

void CommandBuffer::UpdateBuffer(const Buffer &dstBuffer, const void *pData, VkDeviceSize dstOffset, VkDeviceSize size) const
{
    assert(cmdBuffer_ && dstBuffer);
    vkCmdUpdateBuffer(cmdBuffer_, VkBuffer(dstBuffer), dstOffset, size, pData);
}

void CommandBuffer::CopyBuffer(const Buffer &srcBuffer, const Buffer &dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset, VkDeviceSize dstOffset) const
{
    CopyBufferRegions(srcBuffer, dstBuffer, {VkBufferCopy{srcOffset, dstOffset, size}});
}

void CommandBuffer::CopyBufferRegions(const Buffer &srcBuffer, const Buffer &dstBuffer, const Span<VkBufferCopy> &regions) const
{
    assert(cmdBuffer_ && srcBuffer && dstBuffer && regions);
    vkCmdCopyBuffer(cmdBuffer_, VkBuffer(srcBuffer), VkBuffer(dstBuffer), regions.Count(), regions.Data());
}

void CommandBuffer::CopyBufferToImage(const Buffer &srcBuffer, const Image &dstImage, VkImageLayout imageLayout, const Span<VkBufferImageCopy> &regions) const
{
    assert(cmdBuffer_ && srcBuffer && dstImage && regions);
    vkCmdCopyBufferToImage(cmdBuffer_, VkBuffer(srcBuffer), VkImage(dstImage), imageLayout, regions.Count(), regions.Data());
}

void CommandBuffer::CopyImageToBuffer(const Image &srcImage, VkImageLayout imageLayout, const Buffer &dstBuffer, const Span<VkBufferImageCopy> &regions) const
{
    assert(cmdBuffer_ && srcImage && dstBuffer && regions);
    vkCmdCopyImageToBuffer(cmdBuffer_, VkImage(srcImage), imageLayout, VkBuffer(dstBuffer), regions.Count(), regions.Data());
}

void CommandBuffer::CopyImage(const Image &srcImage, VkImageLayout srcImageLayout, const Image &dstImage, VkImageLayout dstImageLayout, const Span<VkImageCopy> &regions) const
{
    assert(cmdBuffer_ && srcImage && dstImage && regions);
    vkCmdCopyImage(cmdBuffer_, VkImage(srcImage), srcImageLayout, VkImage(dstImage), dstImageLayout, regions.Count(), regions.Data());
}

void CommandBuffer::BlitImage(const Image &srcImage, VkImageLayout srcImageLayout, const Image &dstImage, VkImageLayout dstImageLayout, const Span<VkImageBlit> &regions, VkFilter filter) const
{
    assert(cmdBuffer_ && srcImage && dstImage && regions);
    vkCmdBlitImage(cmdBuffer_, VkImage(srcImage), srcImageLayout, VkImage(dstImage), dstImageLayout, regions.Count(), regions.Data(), filter);
}

void CommandBuffer::ClearColorImage(const Image &image, VkImageLayout imageLayout, const VkClearColorValue &color, uint32_t baseMipLevel,
                                    uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
{
    ClearColorImage(image, imageLayout, color, {VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, baseMipLevel, levelCount, baseArrayLayer, layerCount}});
}

void CommandBuffer::ClearColorImage(const Image &image, VkImageLayout imageLayout, const VkClearColorValue &color, const Span<VkImageSubresourceRange> &ranges) const
{
    assert(cmdBuffer_ && image && ranges);
    vkCmdClearColorImage(cmdBuffer_, VkImage(image), imageLayout, &color, ranges.Count(), ranges.Data());
}

void CommandBuffer::ClearDepthStencilImage(const Image &image, VkImageLayout imageLayout, const VkClearDepthStencilValue &depthStencil, VkImageAspectFlags aspectMask,
                                           uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
{
    ClearDepthStencilImage(image, imageLayout, depthStencil, {VkImageSubresourceRange{aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount}});
}

void CommandBuffer::ClearDepthStencilImage(const Image &image, VkImageLayout imageLayout, const VkClearDepthStencilValue &depthStencil, const Span<VkImageSubresourceRange> &ranges) const
{
    assert(cmdBuffer_ && image && ranges);
    vkCmdClearDepthStencilImage(cmdBuffer_, VkImage(image), imageLayout, &depthStencil, ranges.Count(), ranges.Data());
}

void CommandBuffer::ClearAttachments(const Span<VkClearAttachment> &attachments, const Span<VkClearRect> &rects) const
{
    assert(cmdBuffer_ && attachments && rects);
    vkCmdClearAttachments(cmdBuffer_, attachments.Count(), attachments.Data(), rects.Count(), rects.Data());
}

void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<MemoryBarrier> &memoryBarriers, VkDependencyFlags dependencyFlags) const
{
    assert(cmdBuffer_ && memoryBarriers);
    vkCmdPipelineBarrier(cmdBuffer_, srcStageMask, dstStageMask, dependencyFlags, memoryBarriers.Count(), reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.Data()), 0, nullptr, 0, nullptr);
}

void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<VkBufferMemoryBarrier> &bufferMemoryBarriers, VkDependencyFlags dependencyFlags) const
{
    assert(cmdBuffer_ && bufferMemoryBarriers);
    vkCmdPipelineBarrier(cmdBuffer_, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, bufferMemoryBarriers.Count(), reinterpret_cast<const VkBufferMemoryBarrier*>(bufferMemoryBarriers.Data()), 0, nullptr);
}

void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<VkImageMemoryBarrier> &imageMemoryBarriers, VkDependencyFlags dependencyFlags) const
{
    assert(cmdBuffer_ && imageMemoryBarriers);
    vkCmdPipelineBarrier(cmdBuffer_, srcStageMask, dstStageMask, dependencyFlags, 0, nullptr, 0, nullptr, imageMemoryBarriers.Count(), reinterpret_cast<const VkImageMemoryBarrier*>(imageMemoryBarriers.Data()));
}

void CommandBuffer::PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<MemoryBarrier> &memoryBarriers,
                                    const Span<VkBufferMemoryBarrier> &bufferMemoryBarriers, const Span<VkImageMemoryBarrier> &imageMemoryBarriers, VkDependencyFlags dependencyFlags) const
{
    assert(cmdBuffer_ && (memoryBarriers || bufferMemoryBarriers || imageMemoryBarriers));
    vkCmdPipelineBarrier(cmdBuffer_, srcStageMask, dstStageMask, dependencyFlags,
                         memoryBarriers.Count(), reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.Data()),
                         bufferMemoryBarriers.Count(), bufferMemoryBarriers.Data(),
                         imageMemoryBarriers.Count(), imageMemoryBarriers.Data());
}

void CommandBuffer::BindComputePipeline(const Pipeline &pipeline) const
{
    assert(cmdBuffer_ && pipeline);
    vkCmdBindPipeline(cmdBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, VkPipeline(pipeline));
}

void CommandBuffer::BindGraphicsPipeline(const Pipeline &pipeline) const
{
    assert(cmdBuffer_ && pipeline);
    vkCmdBindPipeline(cmdBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipeline(pipeline));
}

void CommandBuffer::BindComputeDescriptorSets(const PipelineLayout &layout, const Span<DescriptorSet> &descriptorSets, uint32_t firstSet, const Span<uint32_t> &dynamicOffsets) const
{
    assert(cmdBuffer_ && layout && descriptorSets);
    vkCmdBindDescriptorSets(cmdBuffer_, VK_PIPELINE_BIND_POINT_COMPUTE, VkPipelineLayout(layout), firstSet, descriptorSets.Count(),
                            reinterpret_cast<const VkDescriptorSet*>(descriptorSets.Data()), dynamicOffsets.Count(), dynamicOffsets.Data());
}

void CommandBuffer::BindGraphicsDescriptorSets(const PipelineLayout &layout, const Span<DescriptorSet> &descriptorSets, uint32_t firstSet, const Span<uint32_t> &dynamicOffsets) const
{
    assert(cmdBuffer_ && layout && descriptorSets);
    vkCmdBindDescriptorSets(cmdBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, VkPipelineLayout(layout), firstSet, descriptorSets.Count(),
                            reinterpret_cast<const VkDescriptorSet*>(descriptorSets.Data()), dynamicOffsets.Count(), dynamicOffsets.Data());
}

void CommandBuffer::Dispatch(uint32_t x, uint32_t y, uint32_t z) const
{
    assert(cmdBuffer_);
    vkCmdDispatch(cmdBuffer_, x, y, z);
}

void CommandBuffer::DispatchIndirect(const Buffer &buffer, VkDeviceSize offset) const
{
    assert(cmdBuffer_ && buffer);
    vkCmdDispatchIndirect(cmdBuffer_, VkBuffer(buffer), offset);
}

void CommandBuffer::PushConstants(const PipelineLayout &layout, VkShaderStageFlags stageFlags, const void *pValues, uint32_t offset, uint32_t size) const
{
    assert(cmdBuffer_ && layout && pValues);
    vkCmdPushConstants(cmdBuffer_, VkPipelineLayout(layout), stageFlags, offset, size, pValues);
}

void CommandBuffer::SetViewport(const VkViewport &viewport, uint32_t firstViewport) const
{
    assert(cmdBuffer_);
    vkCmdSetViewport(cmdBuffer_, firstViewport, 1, &viewport);
}

void CommandBuffer::SetViewports(const Span<VkViewport> &viewports, uint32_t firstViewport) const
{
    assert(cmdBuffer_);
    vkCmdSetViewport(cmdBuffer_, firstViewport, viewports.Count(), viewports.Data());
}

void CommandBuffer::SetScissor(const VkRect2D &scissor, uint32_t firstScissor) const
{
    assert(cmdBuffer_);
    vkCmdSetScissor(cmdBuffer_, firstScissor, 1, &scissor);
}

void CommandBuffer::SetScissors(const Span<VkRect2D> &scissors, uint32_t firstScissor) const
{
    assert(cmdBuffer_);
    vkCmdSetScissor(cmdBuffer_, firstScissor, scissors.Count(), scissors.Data());
}

void CommandBuffer::SetDepthBounds(float minDepthBounds, float maxDepthBounds) const
{
    assert(cmdBuffer_);
    vkCmdSetDepthBounds(cmdBuffer_, minDepthBounds, maxDepthBounds);
}

void CommandBuffer::SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
    assert(cmdBuffer_);
    vkCmdSetDepthBias(cmdBuffer_, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void CommandBuffer::SetBlendConstants(const float blendConstants[4]) const
{
    assert(cmdBuffer_);
    vkCmdSetBlendConstants(cmdBuffer_, blendConstants);
}

void CommandBuffer::SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference) const
{
    assert(cmdBuffer_);
    vkCmdSetStencilReference(cmdBuffer_, faceMask, reference);
}

void CommandBuffer::SetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask) const
{
    assert(cmdBuffer_);
    vkCmdSetStencilCompareMask(cmdBuffer_, faceMask, compareMask);
}

void CommandBuffer::SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask) const
{
    assert(cmdBuffer_);
    vkCmdSetStencilWriteMask(cmdBuffer_, faceMask, writeMask);
}

void CommandBuffer::ResolveImage(const Image &srcImage, VkImageLayout srcImageLayout, const Image &dstImage, VkImageLayout dstImageLayout, const Span<VkImageResolve> &regions) const
{
    assert(cmdBuffer_ && srcImage && dstImage && regions);
    vkCmdResolveImage(cmdBuffer_, VkImage(srcImage), srcImageLayout, VkImage(dstImage), dstImageLayout, regions.Count(), regions.Data());
}

void CommandBuffer::SetEvent(const Event &event, VkPipelineStageFlags stageMask) const
{
    assert(cmdBuffer_ && event);
    vkCmdSetEvent(cmdBuffer_, VkEvent(event), stageMask);
}

void CommandBuffer::ResetEvent(const Event &event, VkPipelineStageFlags stageMask) const
{
    assert(cmdBuffer_ && event);
    vkCmdResetEvent(cmdBuffer_, VkEvent(event), stageMask);
}

void CommandBuffer::WaitEvents(const Span2<Event> &events, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                               const Span<MemoryBarrier> &memoryBarriers, const Span<VkBufferMemoryBarrier> &bufferMemoryBarriers,
                               const Span<VkImageMemoryBarrier> &imageMemoryBarriers) const
{
    assert(cmdBuffer_ && events);
    const auto eventCount = events.Count();
    auto pEvents = static_cast<VkEvent*>(alloca(sizeof(VkEvent) * eventCount));
    events.Emplace(pEvents);
    vkCmdWaitEvents(cmdBuffer_, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarriers.Count(), reinterpret_cast<const VkMemoryBarrier*>(memoryBarriers.Data()),
                    bufferMemoryBarriers.Count(), bufferMemoryBarriers.Data(),
                    imageMemoryBarriers.Count(), imageMemoryBarriers.Data());
}

void CommandBuffer::ResetQueryPool(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount) const
{
    assert(cmdBuffer_ && queryPool);
    vkCmdResetQueryPool(cmdBuffer_, VkQueryPool(queryPool), firstQuery, queryCount);
}

void CommandBuffer::BeginQuery(const QueryPool &queryPool, uint32_t query, VkQueryControlFlags flags) const
{
    assert(cmdBuffer_ && queryPool);
    vkCmdBeginQuery(cmdBuffer_, VkQueryPool(queryPool), query, flags);
}

void CommandBuffer::EndQuery(const QueryPool &queryPool, uint32_t query) const
{
    assert(cmdBuffer_ && queryPool);
    vkCmdEndQuery(cmdBuffer_, VkQueryPool(queryPool), query);
}

void CommandBuffer::CopyQueryPoolResults(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount, const Buffer &dstBuffer, VkDeviceSize dstOffset, VkQueryResultFlags flags) const
{
    return CopyQueryPoolResults(queryPool, firstQuery, queryCount, 1, dstBuffer, dstOffset, flags);
}

void CommandBuffer::CopyQueryPoolResults(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount, uint32_t elementCountPerResult,
                                         const Buffer &dstBuffer, VkDeviceSize dstOffset, VkQueryResultFlags flags) const
{
    assert(cmdBuffer_ && queryPool && dstBuffer);
    vkCmdCopyQueryPoolResults(cmdBuffer_, VkQueryPool(queryPool), firstQuery, queryCount, VkBuffer(dstBuffer), dstOffset,
        (flags & VK_QUERY_RESULT_64_BIT) ? elementCountPerResult * sizeof(uint64_t) : elementCountPerResult * sizeof(uint32_t), flags);
}

void CommandBuffer::WriteTimeStamp(const QueryPool &queryPool, uint32_t query, VkPipelineStageFlagBits pipelineStage) const
{
    assert(cmdBuffer_ && queryPool);
    vkCmdWriteTimestamp(cmdBuffer_, pipelineStage, VkQueryPool(queryPool), query);
}

void CommandBuffer::SetLineWidth(float lineWidth) const
{
    assert(cmdBuffer_);
    vkCmdSetLineWidth(cmdBuffer_, lineWidth);
}

void CommandBuffer::BindVertexBuffers(const Span2<Buffer> &vertexBuffers, uint32_t firstBinding) const
{
    assert(vertexBuffers);
    auto pOffset = static_cast<VkDeviceSize*>(alloca(sizeof(VkDeviceSize) * vertexBuffers.Count()));
    memset(pOffset, 0, sizeof(VkDeviceSize) * vertexBuffers.Count());
    BindVertexBuffers(vertexBuffers, {pOffset, vertexBuffers.Count()}, firstBinding);
}

void CommandBuffer::BindVertexBuffers(const Span2<Buffer> &vertexBuffers, const Span<VkDeviceSize> &offsets, uint32_t firstBinding) const
{
    assert(cmdBuffer_ && vertexBuffers && offsets && (vertexBuffers.Count() == offsets.Count()));
    const auto bufferCount = vertexBuffers.Count();
    auto pBuffers = static_cast<VkBuffer*>(alloca(sizeof(VkBuffer) * bufferCount));
    vertexBuffers.Emplace(pBuffers);
    vkCmdBindVertexBuffers(cmdBuffer_, firstBinding, vertexBuffers.Count(), pBuffers, offsets.Data());
}

void CommandBuffer::BindIndexBuffer(const Buffer &indexBuffer, VkDeviceSize offset, VkIndexType indexType) const
{
    assert(cmdBuffer_ && indexBuffer);
    vkCmdBindIndexBuffer(cmdBuffer_, VkBuffer(indexBuffer), offset, indexType);
}

void CommandBuffer::Draw(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount, uint32_t firstInstance) const
{
    assert(cmdBuffer_);
    vkCmdDraw(cmdBuffer_, vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t instanceCount, uint32_t firstInstance) const
{
    assert(cmdBuffer_);
    vkCmdDrawIndexed(cmdBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::DrawIndirect(const Buffer &buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const
{
    assert(cmdBuffer_ && buffer);
    vkCmdDrawIndirect(cmdBuffer_, VkBuffer(buffer), offset, drawCount, stride);
}

void CommandBuffer::DrawIndexedIndirect(const Buffer &buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride) const
{
    assert(cmdBuffer_ && buffer);
    vkCmdDrawIndexedIndirect(cmdBuffer_, VkBuffer(buffer), offset, drawCount, stride);
}

void CommandBuffer::BeginRenderPass(const RenderPass &renderPass, const Framebuffer &framebuffer, const VkExtent2D &renderArea,
                                    const Span<VkClearValue> &clearValues, VkSubpassContents contents) const
{
    BeginRenderPassExt(nullptr, renderPass, framebuffer, {0, 0, renderArea}, clearValues, contents);
}

void CommandBuffer::BeginRenderPass(const RenderPass &renderPass, const Framebuffer &framebuffer, const VkRect2D &renderArea,
                                    const Span<VkClearValue> &clearValues, VkSubpassContents contents) const
{
    BeginRenderPassExt(nullptr, renderPass, framebuffer, renderArea, clearValues, contents);
}

void CommandBuffer::BeginRenderPassExt(const void *pNext, const RenderPass &renderPass, const Framebuffer &framebuffer,
                                       const VkExtent2D &renderArea, const Span<VkClearValue> &clearValues, VkSubpassContents contents) const
{
    BeginRenderPassExt(pNext, renderPass, framebuffer, {0, 0, renderArea}, clearValues, contents);
}

void CommandBuffer::BeginRenderPassExt(const void *pNext, const RenderPass &renderPass, const Framebuffer &framebuffer,
                                       const VkRect2D &renderArea, const Span<VkClearValue> &clearValues, VkSubpassContents contents) const
{
    assert(cmdBuffer_ && renderPass && framebuffer);

    VkRenderPassBeginInfo renderPassBegin = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, pNext, VkRenderPass(renderPass),
                                             VkFramebuffer(framebuffer), renderArea, clearValues.Count(), clearValues.Data()};
    vkCmdBeginRenderPass(cmdBuffer_, &renderPassBegin, contents);
}

void CommandBuffer::NextSubpass(VkSubpassContents contents) const
{
    assert(cmdBuffer_);
    vkCmdNextSubpass(cmdBuffer_, contents);
}

void CommandBuffer::EndRenderPass() const
{
    assert(cmdBuffer_);
    vkCmdEndRenderPass(cmdBuffer_);
}

void CommandBuffer::ExecuteCommands(const Span<CommandBuffer> &commandBuffers) const
{
    assert(cmdBuffer_ && commandBuffers);
    vkCmdExecuteCommands(cmdBuffer_, commandBuffers.Count(), reinterpret_cast<const VkCommandBuffer*>(commandBuffers.Data()));
}

} // namespace vkw