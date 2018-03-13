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

std::vector<Image> Swapchain::GetImages() const
{
    assert(swapchain_);

    uint32_t swapchainImageCount;
    VK_CALL(vkGetSwapchainImagesKHR(swapchain_.GetCreator(), swapchain_, &swapchainImageCount, nullptr));

    if (swapchainImageCount > 0)
    {
        auto swapchainImages = static_cast<VkImage*>(alloca(swapchainImageCount * sizeof(VkImage)));
        VK_CALL(vkGetSwapchainImagesKHR(swapchain_.GetCreator(), swapchain_, &swapchainImageCount, swapchainImages));

        std::vector<Image> images;
        images.reserve(swapchainImageCount);
        for (uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            images.emplace_back(swapchain_.GetCreator(), swapchainImages[i], false);
        }

        return images;
    }

    return {};
}

std::tuple<uint32_t, VkResult> Swapchain::AcquireNextImage(const Fence &fence) const
{
    return AcquireNextImage(UINT64_MAX, Semaphore(), fence);
}

std::tuple<uint32_t, VkResult> Swapchain::AcquireNextImage(const Semaphore &semaphore, const Fence &fence) const
{
    return AcquireNextImage(UINT64_MAX, semaphore, fence);
}

std::tuple<uint32_t, VkResult> Swapchain::AcquireNextImage(uint64_t timeoutInNanoSeconds, const Fence &fence) const
{
    return AcquireNextImage(timeoutInNanoSeconds, Semaphore(), fence);
}

std::tuple<uint32_t, VkResult> Swapchain::AcquireNextImage(uint64_t timeoutInNanoSeconds, const Semaphore &semaphore, const Fence &fence) const
{
    assert(swapchain_ && (semaphore || fence));

    uint32_t imageIndex;
    auto result = vkAcquireNextImageKHR(swapchain_.GetCreator(), swapchain_, timeoutInNanoSeconds,
                                        VkSemaphore(semaphore), VkFence(fence), &imageIndex);
    VK_CALL(result);

    return std::make_tuple(imageIndex, result);
}

Swapchain Swapchain::Recreate(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                              VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, VkSurfaceTransformFlagBitsKHR preTransform,
                              VkCompositeAlphaFlagBitsKHR compositeAlpha, VkBool32 clipped, uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    return RecreateConcurrentExt(nullptr, surface, minImageCount, format, extent, imageUsage, presentMode,
                                 {}, preTransform, compositeAlpha, clipped, imageArrayLayers);
}

Swapchain Swapchain::RecreateExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format,
                                 const VkExtent2D &extent, VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode,
                                 VkSurfaceTransformFlagBitsKHR preTransform, VkCompositeAlphaFlagBitsKHR compositeAlpha,
                                 VkBool32 clipped, uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    return RecreateConcurrentExt(pNext, surface, minImageCount, format, extent, imageUsage, presentMode,
                                 {}, preTransform, compositeAlpha, clipped, imageArrayLayers);
}

Swapchain Swapchain::RecreateConcurrent(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                        VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, const Span<uint32_t> &queueFamilyIndices,
                                        VkSurfaceTransformFlagBitsKHR preTransform, VkCompositeAlphaFlagBitsKHR compositeAlpha, VkBool32 clipped,
                                        uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    return RecreateConcurrentExt(nullptr, surface, minImageCount, format, extent, imageUsage, presentMode,
                                 queueFamilyIndices, preTransform, compositeAlpha, clipped, imageArrayLayers);
}

Swapchain Swapchain::RecreateConcurrentExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format,
                                           const VkExtent2D &extent, VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode,
                                           const Span<uint32_t> &queueFamilyIndices, VkSurfaceTransformFlagBitsKHR preTransform,
                                           VkCompositeAlphaFlagBitsKHR compositeAlpha, VkBool32 clipped, uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    assert(swapchain_ && surface);
    const auto queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.Count());
    VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, pNext, flags, VkSurfaceKHR(surface),
                                           minImageCount, format.format, format.colorSpace, extent, imageArrayLayers, imageUsage,
                                           queueFamilyIndexCount > 0 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
                                           queueFamilyIndexCount, queueFamilyIndices.Data(),
                                           preTransform, compositeAlpha, presentMode, clipped, swapchain_};
    VkSwapchainKHR swapchain;
    VK_CALL(vkCreateSwapchainKHR(swapchain_.GetCreator(), &createInfo, nullptr, &swapchain));
    return Swapchain(swapchain_.GetCreator(), swapchain);
}


} // namespace vkw