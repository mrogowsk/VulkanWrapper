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

Device::Device(VkDevice device)
    : device_(device)
{
    assert(device);
}

void Device::WaitIdle() const
{
    assert(device_);
    VK_CALL(vkDeviceWaitIdle(device_));
}

Buffer Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags) const
{
    return CreateConcurrentBufferExt(nullptr, size, usage, {}, flags);
}

Buffer Device::CreateBufferExt(const void *pNext, VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags) const
{
    return CreateConcurrentBufferExt(pNext, size, usage, {}, flags);
}

Buffer Device::CreateConcurrentBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const Span<uint32_t> &queueFamilyIndices, VkBufferCreateFlags flags) const
{
    return CreateConcurrentBufferExt(nullptr, size, usage, queueFamilyIndices, flags);
}

Buffer Device::CreateConcurrentBufferExt(const void *pNext, VkDeviceSize size, VkBufferUsageFlags usage, const Span<uint32_t> &queueFamilyIndices, VkBufferCreateFlags flags) const
{
    assert(device_);
    const auto queueFamilyIndexCount = queueFamilyIndices.Count();
    VkBufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, pNext, flags, size, usage,
                                     queueFamilyIndexCount > 0 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
                                     queueFamilyIndexCount, queueFamilyIndices.Data()};

    VkBuffer buffer;
    VK_CALL(vkCreateBuffer(device_, &createInfo, nullptr, &buffer));

    return Buffer(device_, buffer);
}

Image Device::CreateImage(const ImageDescription &imageDescription) const
{
    assert(device_);
    VkImage image;
    VK_CALL(vkCreateImage(device_, reinterpret_cast<const VkImageCreateInfo*>(&imageDescription), nullptr, &image));
    return Image(device_, image);
}

Image Device::CreateImage(VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
                          VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout initialLayout) const
{
    return CreateConcurrentImageExt(nullptr, flags, type, format, extent, mipLevels, arrayLayers, samples, tiling, usage, {}, initialLayout);
}

Image Device::CreateImageExt(const void *pNext, VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels,
                             uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout initialLayout) const
{
    return CreateConcurrentImageExt(pNext, flags, type, format, extent, mipLevels, arrayLayers, samples, tiling, usage, {}, initialLayout);
}

Image Device::CreateConcurrentImage(VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
                                    VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, const Span<uint32_t> &queueFamilyIndices, VkImageLayout initialLayout) const
{
    return CreateConcurrentImageExt(nullptr, flags, type, format, extent, mipLevels, arrayLayers, samples, tiling, usage, queueFamilyIndices, initialLayout);
}

Image Device::CreateConcurrentImageExt(const void *pNext, VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels,
                                       uint32_t arrayLayers, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage,
                                       const Span<uint32_t> &queueFamilyIndices, VkImageLayout initialLayout) const
{
    assert(device_);
    const auto queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.Count());
    VkImageCreateInfo imageCreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, pNext, flags, type, format, extent, mipLevels, arrayLayers, samples, tiling, usage,
                                         queueFamilyIndexCount > 0 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
                                         queueFamilyIndexCount, queueFamilyIndices.Data()};

    VkImage image;
    VK_CALL(vkCreateImage(device_, &imageCreateInfo, nullptr, &image));
    return Image(device_, image);
}

Image Device::CreateLinearlyTiledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkImageLayout initialLayout,
                                       VkImageCreateFlags flags) const
{
    return CreateConcurrentLinearlyTiledImageExt(nullptr, extent, format, usage, {}, initialLayout, flags);
}

Image Device::CreateLinearlyTiledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage,
                                          VkImageLayout initialLayout, VkImageCreateFlags flags) const
{
    return CreateConcurrentLinearlyTiledImageExt(pNext, extent, format, usage, {}, initialLayout, flags);
}

Image Device::CreateConcurrentLinearlyTiledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, const Span<uint32_t> &queueFamilyIndices,
                                                 VkImageLayout initialLayout, VkImageCreateFlags flags) const
{
    return CreateConcurrentLinearlyTiledImageExt(nullptr, extent, format, usage, queueFamilyIndices, initialLayout, flags);
}

Image Device::CreateConcurrentLinearlyTiledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage,
                                                    const Span<uint32_t> &queueFamilyIndices, VkImageLayout initialLayout, VkImageCreateFlags flags) const
{
    return CreateConcurrentImageExt(pNext, flags, VK_IMAGE_TYPE_2D, format, {extent.width, extent.height, 1},
                                    1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_LINEAR, usage, queueFamilyIndices, initialLayout);
}

Image Device::CreateImage1D(uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers,
                            VkImageCreateFlags flags) const
{
    return CreateConcurrentImage1DExt(nullptr, extent, format, usage, mipLevels, {}, arrayLayers, flags);
}

Image Device::CreateImage1DExt(const void *pNext, uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                               uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentImage1DExt(pNext, extent, format, usage, mipLevels, {}, arrayLayers, flags);
}

Image Device::CreateConcurrentImage1D(uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, const Span<uint32_t> &queueFamilyIndices,
                                      uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentImage1DExt(nullptr, extent, format, usage, mipLevels, queueFamilyIndices, arrayLayers, flags);
}

Image Device::CreateConcurrentImage1DExt(const void *pNext, uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                         const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentImageExt(pNext, flags, VK_IMAGE_TYPE_1D, format, {extent, 1, 1},
                                    mipLevels, arrayLayers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usage, queueFamilyIndices, VK_IMAGE_LAYOUT_UNDEFINED);
}

Image Device::CreateImage2D(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers,
                            VkImageCreateFlags flags) const
{
    return CreateConcurrentImage2DExt(nullptr, extent, format, usage, mipLevels, {}, arrayLayers, flags);
}

Image Device::CreateImage2DExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                               uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentImage2DExt(pNext, extent, format, usage, mipLevels, {}, arrayLayers, flags);
}

Image Device::CreateConcurrentImage2D(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                      const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentImage2DExt(nullptr, extent, format, usage, mipLevels, queueFamilyIndices, arrayLayers, flags);
}

Image Device::CreateConcurrentImage2DExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                         const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentImageExt(pNext, flags, VK_IMAGE_TYPE_2D, format, {extent.width, extent.height, 1},
                                    mipLevels, arrayLayers, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usage, queueFamilyIndices, VK_IMAGE_LAYOUT_UNDEFINED);
}

Image Device::CreateMultiSampledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                      uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentMultiSampledImageExt(nullptr, extent, format, usage, samples, {}, arrayLayers, flags);
}

Image Device::CreateMultiSampledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                         uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentMultiSampledImageExt(pNext, extent, format, usage, samples, {}, arrayLayers, flags);
}

Image Device::CreateConcurrentMultiSampledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                                const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentMultiSampledImageExt(nullptr, extent, format, usage, samples, queueFamilyIndices, arrayLayers, flags);
}

Image Device::CreateConcurrentMultiSampledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                                   const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers, VkImageCreateFlags flags) const
{
    return CreateConcurrentImageExt(pNext, flags, VK_IMAGE_TYPE_2D, format, {extent.width, extent.height, 1},
                                    1, arrayLayers, samples, VK_IMAGE_TILING_OPTIMAL, usage, queueFamilyIndices, VK_IMAGE_LAYOUT_UNDEFINED);
}

Image Device::CreateImage3D(const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, VkImageCreateFlags flags) const
{
    return CreateConcurrentImage3DExt(nullptr, extent, format, usage, mipLevels, {}, flags);
}

Image Device::CreateImage3DExt(const void *pNext, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, VkImageCreateFlags flags) const
{
    return CreateConcurrentImage3DExt(pNext, extent, format, usage, mipLevels, {}, flags);
}

Image Device::CreateConcurrentImage3D(const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                      const Span<uint32_t> &queueFamilyIndices, VkImageCreateFlags flags) const
{
    return CreateConcurrentImage3DExt(nullptr, extent, format, usage, mipLevels, queueFamilyIndices, flags);
}

Image Device::CreateConcurrentImage3DExt(const void *pNext, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                         const Span<uint32_t> &queueFamilyIndices, VkImageCreateFlags flags) const
{
    return CreateConcurrentImageExt(pNext, flags, VK_IMAGE_TYPE_3D, format, extent,
                                    mipLevels, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usage, queueFamilyIndices, VK_IMAGE_LAYOUT_UNDEFINED);
}

Sampler Device::CreateSampler(const SamplerDescription &samplerDescription) const
{
    assert(device_);
    VkSampler sampler;
    VK_CALL(vkCreateSampler(device_, reinterpret_cast<const VkSamplerCreateInfo*>(&samplerDescription), nullptr, &sampler));
    return Sampler(device_, sampler);
}

DeviceMemory Device::AllocateMemory(VkDeviceSize allocationSize, uint32_t memoryTypeIndex) const
{
    return AllocateMemoryExt(nullptr, allocationSize, memoryTypeIndex);
}

DeviceMemory Device::AllocateMemoryExt(const void *pNext, VkDeviceSize allocationSize, uint32_t memoryTypeIndex) const
{
    assert(device_);
    VkMemoryAllocateInfo allocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, pNext, allocationSize, memoryTypeIndex};
    VkDeviceMemory memory;
    VK_CALL(vkAllocateMemory(device_, &allocInfo, nullptr, &memory));
    return DeviceMemory(device_, memory);
}

Queue Device::GetQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) const
{
    assert(device_);
    VkQueue queue;
    vkGetDeviceQueue(device_, queueFamilyIndex, queueIndex, &queue);
    return Queue(queue);
}

CommandPool Device::CreateCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) const
{
    return CreateCommandPoolExt(nullptr, queueFamilyIndex, flags);
}

CommandPool Device::CreateCommandPoolExt(const void *pNext, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) const
{
    assert(device_);
    VkCommandPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, pNext, flags, queueFamilyIndex};
    VkCommandPool commandPool;
    VK_CALL(vkCreateCommandPool(device_, &createInfo, nullptr, &commandPool));
    return CommandPool(device_, commandPool);
}

Swapchain Device::CreateSwapchain(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                  VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, VkSurfaceTransformFlagBitsKHR preTransform,
                                  VkCompositeAlphaFlagBitsKHR compositeAlpha, VkBool32 clipped, uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    return CreateConcurrentSwapchainExt(nullptr, surface, minImageCount, format, extent, imageUsage, presentMode,
                                        {}, preTransform, compositeAlpha, clipped, imageArrayLayers, flags);
}

Swapchain Device::CreateSwapchainExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format,
                                     const VkExtent2D &extent, VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode,
                                     VkSurfaceTransformFlagBitsKHR preTransform, VkCompositeAlphaFlagBitsKHR compositeAlpha,
                                     VkBool32 clipped, uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    return CreateConcurrentSwapchainExt(pNext, surface, minImageCount, format, extent, imageUsage, presentMode,
                                        {}, preTransform, compositeAlpha, clipped, imageArrayLayers, flags);
}

Swapchain Device::CreateConcurrentSwapchain(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                            VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, const Span<uint32_t> &queueFamilyIndices,
                                            VkSurfaceTransformFlagBitsKHR preTransform, VkCompositeAlphaFlagBitsKHR compositeAlpha, VkBool32 clipped,
                                            uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    return CreateConcurrentSwapchainExt(nullptr, surface, minImageCount, format, extent, imageUsage, presentMode,
                                        queueFamilyIndices, preTransform, compositeAlpha, clipped, imageArrayLayers, flags);
}

Swapchain Device::CreateConcurrentSwapchainExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format,
                                               const VkExtent2D &extent, VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode,
                                               const Span<uint32_t> &queueFamilyIndices, VkSurfaceTransformFlagBitsKHR preTransform,
                                               VkCompositeAlphaFlagBitsKHR compositeAlpha, VkBool32 clipped, uint32_t imageArrayLayers, VkSwapchainCreateFlagsKHR flags) const
{
    assert(device_ && surface);
    const auto queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.Count());
    VkSwapchainCreateInfoKHR createInfo = {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, pNext, flags, VkSurfaceKHR(surface),
                                           minImageCount, format.format, format.colorSpace, extent, imageArrayLayers, imageUsage,
                                           queueFamilyIndexCount > 0 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
                                           queueFamilyIndexCount, queueFamilyIndices.Data(),
                                           preTransform, compositeAlpha, presentMode, clipped, VK_NULL_HANDLE};
    VkSwapchainKHR swapchain;
    VK_CALL(vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain));
    return Swapchain(device_, swapchain);
}

ShaderModule Device::CreateShaderModule(const Span<uint32_t> &code) const
{
    return CreateShaderModuleExt(nullptr, code);
}

ShaderModule Device::CreateShaderModuleExt(const void *pNext, const Span<uint32_t> &code) const
{
    assert(device_ && code);
    VkShaderModuleCreateInfo createInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, pNext, 0, code.Size(), code.Data()};
    VkShaderModule shaderModule;
    VK_CALL(vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule));
    return ShaderModule(device_, shaderModule);
}

ShaderModule Device::CreateShaderModule(const Span<char> &code) const
{
    return CreateShaderModuleExt(nullptr, Span<uint32_t>(reinterpret_cast<const uint32_t*>(code.Data()), code.Size() / sizeof(uint32_t)));
}

ShaderModule Device::CreateShaderModuleExt(const void *pNext, const Span<char> &code) const
{
    return CreateShaderModuleExt(pNext, Span<uint32_t>(reinterpret_cast<const uint32_t*>(code.Data()), code.Size() / sizeof(uint32_t)));
}

PipelineCache Device::CreatePipelineCache(size_t initialDataSize, const void *pInitialData) const
{
    return CreatePipelineCacheExt(nullptr, initialDataSize, pInitialData);
}

PipelineCache Device::CreatePipelineCacheExt(const void *pNext, size_t initialDataSize, const void *pInitialData) const
{
    assert(device_);

    VkPipelineCacheCreateInfo createInfo = {VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO, pNext, 0, initialDataSize, pInitialData};

    VkPipelineCache pipelineCache;
    VK_CALL(vkCreatePipelineCache(device_, &createInfo, nullptr, &pipelineCache));

    return PipelineCache(device_, pipelineCache);
}

Pipeline Device::CreateComputePipeline(const Pipeline::ShaderStage &stage, const PipelineLayout &layout, const PipelineCache &pipelineCache,
                                       VkPipelineCreateFlags flags, const Pipeline &basePipeline) const
{
    return CreateComputePipelineExt(nullptr, stage, layout, pipelineCache, flags, basePipeline);
}

Pipeline Device::CreateComputePipelineExt(const void *pNext, const Pipeline::ShaderStage &stage, const PipelineLayout &layout,
                                          const PipelineCache &pipelineCache, VkPipelineCreateFlags flags, const Pipeline &basePipeline) const
{
    assert(device_ && stage.module && layout);

    VkComputePipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, pNext,
                                              flags, VkPipelineShaderStageCreateInfo(stage), VkPipelineLayout(layout), VkPipeline(basePipeline), -1};
    VkPipeline pipeline;
    VK_CALL(vkCreateComputePipelines(device_, VkPipelineCache(pipelineCache), 1, &createInfo, nullptr, &pipeline));
    return Pipeline(device_, pipeline);
}

VkResult Device::WaitForFences(const Span2<Fence> &fences, uint64_t timeoutInNanoSeconds, bool waitAll) const
{
    assert(device_);

    const uint32_t fenceCount = fences.Count();
    if (fenceCount == 0)
    {
        return VK_SUCCESS;
    }

    auto pFences = static_cast<VkFence*>(alloca(sizeof(VkFence) * fenceCount));
    fences.Emplace(pFences);
    auto result = vkWaitForFences(device_, fenceCount, pFences, static_cast<VkBool32>(waitAll), timeoutInNanoSeconds);
    VK_CALL(result);
    return result;
}

void Device::ResetFences(const Span2<Fence> &fences) const
{
    assert(device_);

    const uint32_t fenceCount = fences.Count();
    if (fenceCount == 0)
    {
        return;
    }

    auto pFences = static_cast<VkFence*>(alloca(sizeof(VkFence) * fenceCount));
    fences.Emplace(pFences);
    VK_CALL(vkResetFences(device_, fenceCount, pFences));
}

DescriptorSetLayout Device::CreateDescriptorSetLayout(const Span<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags) const
{
    return CreateDescriptorSetLayoutExt(nullptr, bindings, flags);
}

DescriptorSetLayout Device::CreateDescriptorSetLayoutExt(const void *pNext, const Span<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags) const
{
    assert(device_);

    VkDescriptorSetLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, pNext, flags, bindings.Count(), bindings.Data()};
    VkDescriptorSetLayout setLayout;
    VK_CALL(vkCreateDescriptorSetLayout(device_, &createInfo, nullptr, &setLayout));
    return DescriptorSetLayout(device_, setLayout);
}

PipelineLayout Device::CreatePipelineLayout(const Span2<DescriptorSetLayout> &setLayouts, const Span<VkPushConstantRange> &pushConstantRanges, VkPipelineLayoutCreateFlags flags) const
{
    return CreatePipelineLayoutExt(nullptr, setLayouts, pushConstantRanges, flags);
}

PipelineLayout Device::CreatePipelineLayoutExt(const void *pNext, const Span2<DescriptorSetLayout> &setLayouts, const Span<VkPushConstantRange> &pushConstantRanges, VkPipelineLayoutCreateFlags flags) const
{
    assert(device_);

    const auto setLayoutCount = setLayouts.Count();
    VkDescriptorSetLayout *pSetLayouts = nullptr;
    if (setLayoutCount > 0)
    {
        pSetLayouts = static_cast<VkDescriptorSetLayout*>(alloca(sizeof(VkDescriptorSetLayout) * setLayoutCount));
        setLayouts.Emplace(pSetLayouts);
    }

    VkPipelineLayoutCreateInfo createInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, pNext, flags, setLayoutCount, pSetLayouts,
                                             pushConstantRanges.Count(), pushConstantRanges.Data()};
    VkPipelineLayout pipelineLayout;
    VK_CALL(vkCreatePipelineLayout(device_, &createInfo, nullptr, &pipelineLayout));
    return PipelineLayout(device_, pipelineLayout);
}

DescriptorPool Device::CreateDescriptorPool(uint32_t maxSets, const Span<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags) const
{
    return CreateDescriptorPoolExt(nullptr, maxSets, poolSizes, flags);
}

DescriptorPool Device::CreateDescriptorPoolExt(const void *pNext, uint32_t maxSets, const Span<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags) const
{
    assert(device_ && maxSets > 0);

    VkDescriptorPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, pNext, flags, maxSets, poolSizes.Count(), poolSizes.Data()};
    VkDescriptorPool descriptorPool;
    VK_CALL(vkCreateDescriptorPool(device_, &createInfo, nullptr, &descriptorPool));
    return DescriptorPool(device_, descriptorPool);
}

DescriptorBufferInfo::operator VkDescriptorBufferInfo() const
{
    return {VkBuffer(*buffer), offset, range};
}

DescriptorImageInfo::operator VkDescriptorImageInfo() const
{
    return {VkSampler(*sampler), VkImageView(*imageView), imageLayout};
}

void Device::UpdateDescriptorSet(const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement, VkDescriptorType descriptorType,
                                 const Span<DescriptorBufferInfo> &bufferInfo) const
{
    UpdateDescriptorSetExt(nullptr, dstSet, dstBinding, dstStartingArrayElement, descriptorType, bufferInfo);
}

void Device::UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                    VkDescriptorType descriptorType, const Span<DescriptorBufferInfo> &bufferInfo) const
{
    assert(device_ && dstSet && bufferInfo);

    const auto descriptorCount = bufferInfo.Count();
    auto *pBufferInfo = static_cast<VkDescriptorBufferInfo*>(alloca(sizeof(VkDescriptorBufferInfo) * descriptorCount));
    bufferInfo.Emplace(pBufferInfo);
    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, VkDescriptorSet(dstSet), dstBinding, dstStartingArrayElement, descriptorCount,
                                  descriptorType, nullptr, pBufferInfo, nullptr};
    vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
}

void Device::UpdateDescriptorSet(const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                 VkDescriptorType descriptorType, const Span<DescriptorImageInfo> &imageInfo) const
{
    UpdateDescriptorSetExt(nullptr, dstSet, dstBinding, dstStartingArrayElement, descriptorType, imageInfo);
}

void Device::UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                    VkDescriptorType descriptorType, const Span<DescriptorImageInfo> &imageInfo) const
{
    assert(device_ && dstSet && imageInfo);

    const auto descriptorCount = imageInfo.Count();
    auto *pImageInfo = static_cast<VkDescriptorImageInfo*>(alloca(sizeof(VkDescriptorImageInfo) * descriptorCount));
    imageInfo.Emplace(pImageInfo);
    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, VkDescriptorSet(dstSet), dstBinding, dstStartingArrayElement, descriptorCount,
                                  descriptorType, pImageInfo, nullptr, nullptr};
    vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
}

void Device::UpdateDescriptorSet(const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                 VkDescriptorType descriptorType, const Span2<BufferView> &bufferViews) const
{
    UpdateDescriptorSetExt(nullptr, dstSet, dstBinding, dstStartingArrayElement, descriptorType, bufferViews);
}

void Device::UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                    VkDescriptorType descriptorType, const Span2<BufferView> &bufferViews) const
{
    assert(device_ && dstSet && bufferViews);

    const auto descriptorCount = bufferViews.Count();
    auto *pBufferViews = static_cast<VkBufferView*>(alloca(sizeof(VkBufferView) * descriptorCount));
    bufferViews.Emplace(pBufferViews);
    VkWriteDescriptorSet write = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, pNext, VkDescriptorSet(dstSet), dstBinding, dstStartingArrayElement, descriptorCount,
                                  descriptorType, nullptr, nullptr, pBufferViews};
    vkUpdateDescriptorSets(device_, 1, &write, 0, nullptr);
}

void Device::UpdateDescriptorSet(const DescriptorSet &srcSet, uint32_t srcBinding, uint32_t srcStartingArrayElement,
                                 const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement, uint32_t descriptorCount) const
{
    UpdateDescriptorSetExt(nullptr, dstSet, dstBinding, dstStartingArrayElement, srcSet, srcBinding, srcStartingArrayElement, descriptorCount);
}

void Device::UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &srcSet, uint32_t srcBinding, uint32_t srcStartingArrayElement,
                                    const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement, uint32_t descriptorCount) const
{
    assert(device_ && dstSet && srcSet);

    VkCopyDescriptorSet copy = {VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET, pNext, VkDescriptorSet(srcSet), srcBinding, srcStartingArrayElement,
                                VkDescriptorSet(dstSet), dstBinding, dstStartingArrayElement, descriptorCount};
    vkUpdateDescriptorSets(device_, 0, nullptr, 1, &copy);
}

RenderPass Device::CreateRenderPass(const Span<AttachmentDescription> &attachments, const Span<SubpassDescription> &subpasses,
                                    const Span<SubpassDependency> &dependencies, VkRenderPassCreateFlags flags) const
{
    return CreateRenderPassExt(nullptr, attachments, subpasses, dependencies, flags);
}

RenderPass Device::CreateRenderPassExt(const void *pNext, const Span<AttachmentDescription> &attachments, const Span<SubpassDescription> &subpasses,
                                       const Span<SubpassDependency> &dependencies, VkRenderPassCreateFlags flags) const
{
    assert(device_ && subpasses);

    const auto subpassCount = subpasses.Count();
    auto pSubpasses = static_cast<VkSubpassDescription*>(alloca(sizeof(VkSubpassDescription) * subpassCount));
    subpasses.Emplace(pSubpasses);

    VkRenderPassCreateInfo createInfo = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, pNext, flags, attachments.Count(),
                                         reinterpret_cast<const VkAttachmentDescription*>(attachments.Data()),
                                         subpassCount, pSubpasses, dependencies.Count(),
                                         reinterpret_cast<const VkSubpassDependency*>(dependencies.Data())};

    VkRenderPass renderPass;
    VK_CALL(vkCreateRenderPass(device_, &createInfo, nullptr, &renderPass));
    return RenderPass(device_, renderPass);
}

Framebuffer Device::CreateFramebuffer(const RenderPass &renderPass, uint32_t width, uint32_t height, const Span2<ImageView> &attachments,
                                      uint32_t layers, VkFramebufferCreateFlags flags) const
{
    return CreateFramebufferExt(nullptr, renderPass, width, height, attachments, layers, flags);
}

Framebuffer Device::CreateFramebufferExt(const void *pNext, const RenderPass &renderPass, uint32_t width, uint32_t height,
                                         const Span2<ImageView> &attachments, uint32_t layers, VkFramebufferCreateFlags flags) const
{
    assert(device_ && renderPass && width > 0 && height > 0 && layers > 0);

    const auto attachmentCount = attachments.Count();
    VkImageView* pAttachments = nullptr;
    if (attachmentCount > 0)
    {
        pAttachments = static_cast<VkImageView*>(alloca(sizeof(VkImageView) * attachmentCount));
        attachments.Emplace(pAttachments);
    }

    VkFramebufferCreateInfo createInfo = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, pNext, flags, VkRenderPass(renderPass),
                                          attachmentCount, pAttachments, width, height, layers};

    VkFramebuffer framebuffer;
    VK_CALL(vkCreateFramebuffer(device_, &createInfo, nullptr, &framebuffer));
    return Framebuffer(device_, framebuffer);
}

Framebuffer Device::CreateFramebuffer(const RenderPass &renderPass, const VkExtent2D extent, const Span2<ImageView> &attachments,
                                      uint32_t layers, VkFramebufferCreateFlags flags) const
{
    return CreateFramebufferExt(nullptr, renderPass, extent.width, extent.height, attachments, layers, flags);
}

Framebuffer Device::CreateFramebufferExt(const void *pNext, const RenderPass &renderPass, const VkExtent2D extent,
                                         const Span2<ImageView> &attachments, uint32_t layers, VkFramebufferCreateFlags flags) const
{
    return CreateFramebufferExt(pNext, renderPass, extent.width, extent.height, attachments, layers, flags);
}

Pipeline Device::CreateGraphicsPipeline(const RenderPass &renderPass, uint32_t subpass, const Span<Pipeline::ShaderStage> &stages, const PipelineLayout &layout,
                                        const GraphicsPipelineStateDescription &gfxPipeDesc, const PipelineCache &pipelineCache,
                                        VkPipelineCreateFlags flags, const Pipeline &basePipeline) const
{
    return CreateGraphicsPipelineExt(nullptr, renderPass, subpass, stages, layout, gfxPipeDesc, pipelineCache, flags, basePipeline);
}

Pipeline Device::CreateGraphicsPipelineExt(const void *pNext, const RenderPass &renderPass, uint32_t subpass, const Span<Pipeline::ShaderStage> &stages,
                                           const PipelineLayout &layout, const GraphicsPipelineStateDescription &gfxPipeDesc,
                                           const PipelineCache &pipelineCache, VkPipelineCreateFlags flags, const Pipeline &basePipeline) const
{
    assert(device_ && renderPass && stages && layout);

    const auto stagesCount = stages.Count();
    auto pStages = static_cast<VkPipelineShaderStageCreateInfo*>(alloca(sizeof(VkPipelineShaderStageCreateInfo) * stagesCount));
    stages.Emplace(pStages);

    VkPipelineVertexInputStateCreateInfo vertexInputState = VkPipelineVertexInputStateCreateInfo(gfxPipeDesc.vertexInputState);
    VkPipelineViewportStateCreateInfo *pViewportState = nullptr;
    if (gfxPipeDesc.viewportState != nullptr)
    {
        pViewportState = static_cast<VkPipelineViewportStateCreateInfo*>(alloca(sizeof(VkPipelineViewportStateCreateInfo)));
        *pViewportState = VkPipelineViewportStateCreateInfo(*gfxPipeDesc.viewportState);
    }
    VkPipelineColorBlendStateCreateInfo *pColorBlendState = nullptr;
    if (gfxPipeDesc.colorBlendState != nullptr)
    {
        pColorBlendState = static_cast<VkPipelineColorBlendStateCreateInfo*>(alloca(sizeof(VkPipelineColorBlendStateCreateInfo)));
        *pColorBlendState = VkPipelineColorBlendStateCreateInfo(*gfxPipeDesc.colorBlendState);
    }
    VkPipelineDynamicStateCreateInfo *pDynamicState = nullptr;
    if (gfxPipeDesc.dynamicState != nullptr)
    {
        pDynamicState = static_cast<VkPipelineDynamicStateCreateInfo*>(alloca(sizeof(VkPipelineDynamicStateCreateInfo)));
        *pDynamicState = VkPipelineDynamicStateCreateInfo(*gfxPipeDesc.dynamicState);
    }

    VkGraphicsPipelineCreateInfo createInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, pNext, flags, stagesCount, pStages,
                                               &vertexInputState, reinterpret_cast<const VkPipelineInputAssemblyStateCreateInfo*>(&gfxPipeDesc.inputAssemblyState),
                                               reinterpret_cast<const VkPipelineTessellationStateCreateInfo*>(gfxPipeDesc.tessellationState),
                                               pViewportState, reinterpret_cast<const VkPipelineRasterizationStateCreateInfo*>(&gfxPipeDesc.rasterizationState),
                                               reinterpret_cast<const VkPipelineMultisampleStateCreateInfo*>(gfxPipeDesc.multisampleState),
                                               reinterpret_cast<const VkPipelineDepthStencilStateCreateInfo*>(gfxPipeDesc.depthStencilState),
                                               pColorBlendState, pDynamicState, VkPipelineLayout(layout), VkRenderPass(renderPass), subpass, VkPipeline(basePipeline), -1};

    VkPipeline pipeline;
    VK_CALL(vkCreateGraphicsPipelines(device_, VkPipelineCache(pipelineCache), 1, &createInfo, nullptr, &pipeline));
    return Pipeline(device_, pipeline);
}

Semaphore Device::createSemaphore(VkPipelineStageFlags pipelineStageFlag, VkSemaphoreCreateFlags flags) const
{
    return createSemaphoreExt(nullptr, pipelineStageFlag, flags);
}

Semaphore Device::createSemaphoreExt(const void *pNext, VkPipelineStageFlags pipelineStageFlag, VkSemaphoreCreateFlags flags) const
{
    assert(device_);
    VkSemaphoreCreateInfo createInfo = {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, pNext, flags};
    VkSemaphore semaphore;
    VK_CALL(vkCreateSemaphore(device_, &createInfo, nullptr, &semaphore));
    return Semaphore(device_, semaphore);
}

Fence Device::CreateFence(VkFenceCreateFlags flags) const
{
    return CreateFenceExt(nullptr, flags);
}

Fence Device::CreateFenceExt(void * pNext, VkFenceCreateFlags flags) const
{
    assert(device_);
    VkFenceCreateInfo createInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, pNext, flags};
    VkFence fence;
    VK_CALL(vkCreateFence(device_, &createInfo, nullptr, &fence));
    return Fence(device_, fence);
}

Event Device::CreateEvent(VkEventCreateFlags flags) const
{
    return CreateEventExt(nullptr, flags);
}

Event Device::CreateEventExt(const void *pNext, VkEventCreateFlags flags) const
{
    assert(device_);
    VkEventCreateInfo createInfo = {VK_STRUCTURE_TYPE_EVENT_CREATE_INFO, pNext, flags};
    VkEvent event;
    VK_CALL(vkCreateEvent(device_, &createInfo, nullptr, &event));
    return Event(device_, event);
}

QueryPool Device::CreateQueryPool(VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics, VkQueryPoolCreateFlags flags) const
{
    return CreateQueryPoolExt(nullptr, queryType, queryCount, pipelineStatistics, flags);
}

QueryPool Device::CreateQueryPoolExt(const void *pNext, VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics, VkQueryPoolCreateFlags flags) const
{
    assert(device_);
    VkQueryPoolCreateInfo createInfo = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, pNext, flags, queryType, queryCount, pipelineStatistics};
    VkQueryPool queryPool;
    VK_CALL(vkCreateQueryPool(device_, &createInfo, nullptr, &queryPool));
    return QueryPool(device_, queryPool);
}

SubpassDescription::operator VkSubpassDescription() const
{
    return {flags, pipelineBindPoint, static_cast<uint32_t>(inputAttachments.size()), inputAttachments.empty() ? nullptr : inputAttachments.data(),
            static_cast<uint32_t>(colorAttachments.size()), colorAttachments.empty() ? nullptr : colorAttachments.data(),
            resolveAttachments.empty() ? nullptr : resolveAttachments.data(), &depthStencilAttachment, static_cast<uint32_t>(preserveAttachments.size()),
            preserveAttachments.empty() ? nullptr : preserveAttachments.data()};
}

} // namespace vkw