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

Image::Image(VkDevice device, VkImage image, bool destroyable)
    : image_(device, image, destroyable)
{
    assert(device && image);
}

VkSubresourceLayout Image::GetSubresourceLayout(VkImageAspectFlags aspectMask, uint32_t mipLevel, uint32_t arrayLayer) const
{
    assert(image_ && aspectMask);

    VkImageSubresource subresource = {aspectMask, mipLevel, arrayLayer};
    VkSubresourceLayout subresourceLayout;
    vkGetImageSubresourceLayout(image_.GetCreator(), image_, &subresource, &subresourceLayout);
    return subresourceLayout;
}

ImageView Image::CreateImageView(VkImageViewType type, VkFormat format, VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount,
                                 uint32_t baseArrayLayer, uint32_t layerCount, VkComponentMapping components, VkImageViewCreateFlags flags) const
{
    return CreateImageViewExt(nullptr, type, format, aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount, components, flags);
}

ImageView Image::CreateImageViewExt(const void *pNext, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectMask, uint32_t baseMipLevel,
                                    uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, VkComponentMapping components, VkImageViewCreateFlags flags) const
{
    assert(image_);

    VkImageViewCreateInfo createInfo = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, pNext, flags, image_, type, format, components,
                                        {aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount}};
    VkImageView view;
    VK_CALL(vkCreateImageView(image_.GetCreator(), &createInfo, nullptr, &view));
    return ImageView(image_.GetCreator(), view);
}

VkMemoryRequirements Image::GetMemoryRequirements() const
{
    assert(image_);
    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(image_.GetCreator(), image_, &memoryRequirements);
    return memoryRequirements;
}

void Image::BindMemory(const DeviceMemory &memory, VkDeviceSize offset) const
{
    assert(image_ && memory);
    VK_CALL(vkBindImageMemory(image_.GetCreator(), image_, VkDeviceMemory(memory), offset));
}

VkImageMemoryBarrier Image::CreateMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                                                VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t baseMipLevel,
                                                uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
{
    return CreateConcurrentMemoryBarrierExt(nullptr, srcAccessMask, dstAccessMask, oldLayout, newLayout, aspectMask, VK_QUEUE_FAMILY_IGNORED,
                                            VK_QUEUE_FAMILY_IGNORED, baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

VkImageMemoryBarrier Image::CreateMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                   VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask,
                                                   uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
{
    return CreateConcurrentMemoryBarrierExt(pNext, srcAccessMask, dstAccessMask, oldLayout, newLayout, aspectMask, VK_QUEUE_FAMILY_IGNORED,
                                            VK_QUEUE_FAMILY_IGNORED, baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

VkImageMemoryBarrier Image::CreateConcurrentMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                                                          VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t srcQueueFamilyIndex,
                                                          uint32_t dstQueueFamilyIndex, uint32_t baseMipLevel, uint32_t levelCount,
                                                          uint32_t baseArrayLayer, uint32_t layerCount) const
{
    return CreateConcurrentMemoryBarrierExt(nullptr, srcAccessMask, dstAccessMask, oldLayout, newLayout, aspectMask, srcQueueFamilyIndex,
                                            dstQueueFamilyIndex, baseMipLevel, levelCount, baseArrayLayer, layerCount);
}

VkImageMemoryBarrier Image::CreateConcurrentMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                             VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask,
                                                             uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, uint32_t baseMipLevel,
                                                             uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount) const
{
    return {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, pNext, srcAccessMask, dstAccessMask, oldLayout, newLayout, srcQueueFamilyIndex,
            dstQueueFamilyIndex, image_, {aspectMask, baseMipLevel, levelCount, baseArrayLayer, layerCount}};
}

} // namespace vkw