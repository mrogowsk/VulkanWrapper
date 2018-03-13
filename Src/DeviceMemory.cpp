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

DeviceMemory::DeviceMemory(VkDevice device, VkDeviceMemory memory)
    : memory_(device, memory)
{
    assert(device && memory);
}

VkDeviceSize DeviceMemory::GetDeviceMemoryCommitment() const
{
    assert(memory_);
    VkDeviceSize committedMemory;
    vkGetDeviceMemoryCommitment(memory_.GetCreator(), memory_, &committedMemory);
    return committedMemory;
}

void* DeviceMemory::Map(VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags) const
{
    assert(memory_);
    void *ptr;
    VK_CALL(vkMapMemory(memory_.GetCreator(), memory_, offset, size, flags, &ptr));
    return ptr;
}

void DeviceMemory::Unmap() const
{
    assert(memory_);
    vkUnmapMemory(memory_.GetCreator(), memory_);
}

void DeviceMemory::FlushMappedMemoryRange(VkDeviceSize offset, VkDeviceSize size) const
{
    FlushMappedMemoryRanges({MappedMemoryRange(offset, size)});
}

void DeviceMemory::FlushMappedMemoryRangeExt(const void *pNext, VkDeviceSize offset, VkDeviceSize size) const
{
    FlushMappedMemoryRangesExt({MappedMemoryRangeExt(pNext, offset, size)});
}

void DeviceMemory::InvalidateMappedMemoryRange(VkDeviceSize offset, VkDeviceSize size) const
{
    InvalidateMappedMemoryRanges({MappedMemoryRange(offset, size)});
}

void DeviceMemory::InvalidateMappedMemoryRangeExt(const void *pNext, VkDeviceSize offset, VkDeviceSize size) const
{
    InvalidateMappedMemoryRangesExt({MappedMemoryRangeExt(pNext, offset, size)});
}

void DeviceMemory::FlushMappedMemoryRanges(const Span<MappedMemoryRange> &ranges) const
{
    assert(memory_);
    for (const auto &r : ranges) { r.memory = memory_; }
    VK_CALL(vkFlushMappedMemoryRanges(memory_.GetCreator(), ranges.Count(), reinterpret_cast<const VkMappedMemoryRange*>(ranges.Data())));
}

void DeviceMemory::FlushMappedMemoryRangesExt(const Span<MappedMemoryRangeExt> &ranges) const
{
    assert(memory_);
    for (const auto &r : ranges) { r.memory = memory_; }
    VK_CALL(vkFlushMappedMemoryRanges(memory_.GetCreator(), ranges.Count(), reinterpret_cast<const VkMappedMemoryRange*>(ranges.Data())));
}

void DeviceMemory::InvalidateMappedMemoryRanges(const Span<MappedMemoryRange> &ranges) const
{
    assert(memory_);
    for (const auto &r : ranges) { r.memory = memory_; }
    VK_CALL(vkInvalidateMappedMemoryRanges(memory_.GetCreator(), ranges.Count(), reinterpret_cast<const VkMappedMemoryRange*>(ranges.Data())));
}

void DeviceMemory::InvalidateMappedMemoryRangesExt(const Span<MappedMemoryRangeExt> &ranges) const
{
    assert(memory_);
    for (const auto &r : ranges) { r.memory = memory_; }
    VK_CALL(vkInvalidateMappedMemoryRanges(memory_.GetCreator(), ranges.Count(), reinterpret_cast<const VkMappedMemoryRange*>(ranges.Data())));
}

} // namespace vkw