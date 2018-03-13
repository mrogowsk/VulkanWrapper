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

Buffer::Buffer(VkDevice device, VkBuffer buffer)
    : buffer_(device, buffer)
{
    assert(device && buffer);
}

BufferView Buffer::CreateBufferView(VkFormat format, VkDeviceSize offset, VkDeviceSize range, VkBufferViewCreateFlags flags) const
{
    return CreateBufferViewExt(nullptr, format, offset, range, flags);
}

BufferView Buffer::CreateBufferViewExt(const void *pNext, VkFormat format, VkDeviceSize offset, VkDeviceSize range, VkBufferViewCreateFlags flags) const
{
    assert(buffer_);

    VkBufferViewCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO, pNext, flags, buffer_, format, offset, range};
    VkBufferView view;
    VK_CALL(vkCreateBufferView(buffer_.GetCreator(), &createInfo, nullptr, &view));
    return BufferView(buffer_.GetCreator(), view);
}

VkMemoryRequirements Buffer::GetMemoryRequirements() const
{
    assert(buffer_);
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(buffer_.GetCreator(), buffer_, &memoryRequirements);
    return memoryRequirements;
}

void Buffer::BindMemory(const DeviceMemory &memory, VkDeviceSize offset) const
{
    assert(buffer_ && memory);
    VK_CALL(vkBindBufferMemory(buffer_.GetCreator(), buffer_, VkDeviceMemory(memory), offset));
}

VkBufferMemoryBarrier Buffer::CreateMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDeviceSize offset, VkDeviceSize size) const
{
    return CreateConcurrentMemoryBarrierExt(nullptr, srcAccessMask, dstAccessMask, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, offset, size);
}

VkBufferMemoryBarrier Buffer::CreateMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDeviceSize offset,
                                                     VkDeviceSize size) const
{
    return CreateConcurrentMemoryBarrierExt(pNext, srcAccessMask, dstAccessMask, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, offset, size);
}

VkBufferMemoryBarrier Buffer::CreateConcurrentMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, uint32_t srcQueueFamilyIndex,
                                                            uint32_t dstQueueFamilyIndex, VkDeviceSize offset, VkDeviceSize size) const
{
    return CreateConcurrentMemoryBarrierExt(nullptr, srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, offset, size);
}

VkBufferMemoryBarrier Buffer::CreateConcurrentMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                               uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, VkDeviceSize offset, VkDeviceSize size) const
{
    return {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER, pNext, srcAccessMask, dstAccessMask, srcQueueFamilyIndex, dstQueueFamilyIndex, buffer_, offset, size};
}

} // namespace vkw