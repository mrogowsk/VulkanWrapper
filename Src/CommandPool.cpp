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

CommandPool::CommandPool(VkDevice device, VkCommandPool commandPool)
    : cmdPool_(device, commandPool)
{
    assert(device && commandPool);
}

void CommandPool::Reset(VkCommandPoolResetFlags flags) const
{
    assert(cmdPool_);
    VK_CALL(vkResetCommandPool(cmdPool_.GetCreator(), cmdPool_, flags));
}

CommandBuffer CommandPool::AllocateCommandBuffer(VkCommandBufferLevel level) const
{
    return AllocateCommandBufferExt(nullptr, level);
}

CommandBuffer CommandPool::AllocateCommandBufferExt(const void *pNext, VkCommandBufferLevel level) const
{
    assert(cmdPool_);
    VkCommandBufferAllocateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, pNext, cmdPool_, level, 1};
    VkCommandBuffer cmdBuffer;
    VK_CALL(vkAllocateCommandBuffers(cmdPool_.GetCreator(), &createInfo, &cmdBuffer));
    return CommandBuffer(cmdBuffer);
}

std::vector<CommandBuffer> CommandPool::AllocateCommandBuffers(uint32_t commandBufferCount, VkCommandBufferLevel level) const
{
    return AllocateCommandBuffersExt(nullptr, commandBufferCount, level);
}

std::vector<CommandBuffer> CommandPool::AllocateCommandBuffersExt(const void *pNext, uint32_t commandBufferCount, VkCommandBufferLevel level) const
{
    assert(cmdPool_);
    VkCommandBufferAllocateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, pNext, cmdPool_, level, commandBufferCount};
    std::vector<CommandBuffer> cmdBuffers(commandBufferCount);
    VK_CALL(vkAllocateCommandBuffers(cmdPool_.GetCreator(), &createInfo, reinterpret_cast<VkCommandBuffer*>(cmdBuffers.data())));
    return cmdBuffers;
}

void CommandPool::FreeCommandBuffers(const Span<CommandBuffer> &commandBuffers) const
{
    assert(cmdPool_);
    vkFreeCommandBuffers(cmdPool_.GetCreator(), cmdPool_, commandBuffers.Count(), reinterpret_cast<const VkCommandBuffer*>(commandBuffers.Data()));
}

} // namespace vkw