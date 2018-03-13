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

MemoryProperties::MemoryProperties(const VkPhysicalDeviceMemoryProperties &memoryProperties)
    : types(memoryProperties.memoryTypes, memoryProperties.memoryTypes + memoryProperties.memoryTypeCount)
    , heaps(memoryProperties.memoryHeaps, memoryProperties.memoryHeaps + memoryProperties.memoryHeapCount)
{}

QueueCreateInfo::operator VkDeviceQueueCreateInfo() const
{
    return {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            pNext,
            flags,
            queueFamilyIndex,
            static_cast<uint32_t>(queuePriorities.size()),
            queuePriorities.data()};
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice device)
    : device_(device)
{
    assert(device);
}

std::vector<Extension> PhysicalDevice::EnumerateExtensions() const
{
    assert(device_);
    uint32_t propertyCount;
    VK_CALL(vkEnumerateDeviceExtensionProperties(device_, nullptr, &propertyCount, nullptr));

    if (propertyCount > 0)
    {
        std::vector<VkExtensionProperties> properties(propertyCount);
        VK_CALL(vkEnumerateDeviceExtensionProperties(device_, nullptr, &propertyCount, properties.data()));

        return {properties.begin(), properties.end()};
    }

    return {};
}

VkPhysicalDeviceProperties PhysicalDevice::GetProperties() const
{
    assert(device_);
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device_, &properties);
    return properties;
}

VkFormatProperties PhysicalDevice::GetFormatProperties(VkFormat format) const
{
    assert(device_);
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(device_, format, &formatProperties);
    return formatProperties;
}

std::optional<VkImageFormatProperties> PhysicalDevice::GetImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling,
                                                                                VkImageUsageFlags usage, VkImageCreateFlags flags) const
{
    assert(device_);
    VkImageFormatProperties formatProperties;
    auto result = vkGetPhysicalDeviceImageFormatProperties(device_, format, type, tiling, usage, flags, &formatProperties);
    if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
    {
        return {};
    }
    VK_CALL(result);

    return formatProperties;
}

VkPhysicalDeviceFeatures PhysicalDevice::GetFeatures() const
{
    assert(device_);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device_, &features);
    return features;
}

MemoryProperties PhysicalDevice::GetMemoryProperties() const
{
    assert(device_);
    VkPhysicalDeviceMemoryProperties properties;
    vkGetPhysicalDeviceMemoryProperties(device_, &properties);
    return MemoryProperties(properties);
}

std::vector<VkQueueFamilyProperties> PhysicalDevice::GetQueueFamilyProperties() const
{
    assert(device_);
    uint32_t queueFamilyPropertyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(device_, &queueFamilyPropertyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device_, &queueFamilyPropertyCount, queueFamilyProperties.data());

    return queueFamilyProperties;
}

VkSurfaceCapabilitiesKHR PhysicalDevice::GetSurfaceCapabilities(const Surface &surface) const
{
    assert(device_ && surface);
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VK_CALL(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_, VkSurfaceKHR(surface), &surfaceCapabilities));
    return surfaceCapabilities;
}

std::vector<VkSurfaceFormatKHR> PhysicalDevice::GetSurfaceFormats(const Surface &surface) const
{
    assert(device_ && surface);
    uint32_t surfaceFormatCount;
    VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(device_, VkSurfaceKHR(surface), &surfaceFormatCount, nullptr));
    if (surfaceFormatCount > 0)
    {
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        VK_CALL(vkGetPhysicalDeviceSurfaceFormatsKHR(device_, VkSurfaceKHR(surface), &surfaceFormatCount, surfaceFormats.data()));
        return surfaceFormats;
    }

    return {};
}

std::vector<VkPresentModeKHR> PhysicalDevice::GetSurfacePresentModes(const Surface &surface) const
{
    assert(device_ && surface);
    uint32_t surfacePresentModeCount;
    VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(device_, VkSurfaceKHR(surface), &surfacePresentModeCount, nullptr));
    if (surfacePresentModeCount > 0)
    {
        std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
        VK_CALL(vkGetPhysicalDeviceSurfacePresentModesKHR(device_, VkSurfaceKHR(surface), &surfacePresentModeCount, surfacePresentModes.data()));
        return surfacePresentModes;
    }

    return {};
}

bool PhysicalDevice::GetSurfaceSupport(const Surface &surface, uint32_t queueFamilyIndex) const
{
    assert(device_ && surface);
    VkBool32 supported;
    VK_CALL(vkGetPhysicalDeviceSurfaceSupportKHR(device_, queueFamilyIndex, VkSurfaceKHR(surface), &supported));
    return supported != 0;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool PhysicalDevice::GetWin32PresentationSupport(uint32_t queueFamilyIndex) const
{
    assert(device_);
    return vkGetPhysicalDeviceWin32PresentationSupportKHR(device_, queueFamilyIndex) != 0;
}
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
bool PhysicalDevice::GetXlibPresentationSupport(uint32_t queueFamilyIndex) const
{
    assert(device_);
    return vkGetPhysicalDeviceXlibPresentationSupportKHR(device_, queueFamilyIndex) != 0;
}
#endif

Device PhysicalDevice::CreateDevice(const Span<QueueCreateInfo> &queueCreateInfo,
                                    const Span<std::string> &enabledExtensions,
                                    const VkPhysicalDeviceFeatures *enabledFeatures) const
{
    return CreateDeviceExt(nullptr, queueCreateInfo, enabledExtensions, enabledFeatures);
}

Device PhysicalDevice::CreateDeviceExt(const void *pNext, const Span<QueueCreateInfo> &queueCreateInfo,
                                       const Span<std::string> &enabledExtensions,
                                       const VkPhysicalDeviceFeatures *enabledFeatures) const
{
    assert(device_ && queueCreateInfo);

    const auto queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.Count());
    auto pQueueCreateInfo = static_cast<VkDeviceQueueCreateInfo*>(alloca(sizeof(VkDeviceQueueCreateInfo) * queueCreateInfoCount));
    queueCreateInfo.Emplace(pQueueCreateInfo);

    const auto enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.Count());
    const char **ppEnabledExtensionNames = nullptr;
    if (enabledExtensionCount > 0)
    {
        ppEnabledExtensionNames = static_cast<const char **>(alloca(sizeof(char *) * enabledExtensionCount));
        for (uint32_t i = 0; i < enabledExtensionCount; ++i)
        {
            ppEnabledExtensionNames[i] = enabledExtensions[i].c_str();
        }
    }

    VkDeviceCreateInfo createInfo = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                                     pNext, 0,
                                     queueCreateInfoCount, pQueueCreateInfo,
                                     0, nullptr, enabledExtensionCount, ppEnabledExtensionNames, enabledFeatures};

    VkDevice device;
    VK_CALL(vkCreateDevice(device_, &createInfo, nullptr, &device));

    return Device(device);
}

} // namespace vkw