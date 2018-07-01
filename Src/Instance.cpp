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

#include <iostream>
#include <sstream>

#ifdef VK_USE_PLATFORM_WIN32_KHR
#include <Windows.h>
#endif

#include "Error.h"

namespace
{

VKAPI_ATTR VkBool32 VKAPI_CALL StandardDebugReportCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT /* objectType */,
    uint64_t /* object */,
    size_t /* location */,
    int32_t /* messageCode */,
    const char * /* pLayerPrefix */,
    const char *pMessage,
    void * /* pUserData */)
{
    std::stringstream out;
    out << "Vulkan ";
    if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0)
    {
        out << "Error";
    }
    else if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0)
    {
        out << "Debug";
    }
    else if ((flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0)
    {
        out << "Performance Warning";
    }
    else if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0)
    {
        out << "Warning";
    }
    else if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0)
    {
        out << "Information";
    }

    out << ": " << pMessage << "\n";
#if _WIN32
    OutputDebugString(out.str().c_str());
#else
    std::cerr << out.str();
#endif
    return VK_FALSE;
}

} // namespace

namespace vkw
{

ApplicationInfo::ApplicationInfo(std::string name, Version version,
                                 std::string engineName, Version engineVersion,
                                 Version apiVersion)
    : applicationName(std::move(name))
    , applicationVersion(version)
    , engineName(std::move(engineName))
    , engineVersion(engineVersion)
    , apiVersion(apiVersion)
{}

ApplicationInfo::operator VkApplicationInfo() const
{
    return {VK_STRUCTURE_TYPE_APPLICATION_INFO,
            pNext,
            applicationName.c_str(),
            applicationVersion.impl.version,
            engineName.c_str(),
            engineVersion.impl.version,
            apiVersion.impl.version };
}

Instance::Instance(VkInstance instance)
    : instance_(instance)
{
    assert(instance);
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
Surface Instance::CreateWin32Surface(void *hInstance, void *hWnd) const
{
    return CreateWin32SurfaceExt(nullptr, hInstance, hWnd);
}

Surface Instance::CreateWin32SurfaceExt(const void *pNext, void *hInstance, void *hWnd) const
{
    assert(instance_ && hInstance && hWnd);

    VkWin32SurfaceCreateInfoKHR surfaceInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
                                               pNext,
                                               0,
                                               reinterpret_cast<HINSTANCE>(hInstance),
                                               reinterpret_cast<HWND>(hWnd)};

    VkSurfaceKHR surface;
    VK_CALL(vkCreateWin32SurfaceKHR(instance_, &surfaceInfo, nullptr, &surface));

    return Surface(instance_, surface);
}
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
Surface Instance::CreateXlibSurface(Display *dpy, Window window) const
{
    return CreateXlibSurfaceExt(nullptr, dpy, window);
}

Surface Instance::CreateXlibSurfaceExt(const void *pNext, Display *dpy, Window window) const
{
    assert(instance_ && dpy);

    VkXlibSurfaceCreateInfoKHR surfaceInfo = {VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                                              pNext,
                                              0,
                                              dpy,
                                              window};

    VkSurfaceKHR surface;
    VK_CALL(vkCreateXlibSurfaceKHR(instance_, &surfaceInfo, nullptr, &surface));

    return Surface(instance_, surface);
}
#endif

void Instance::CreateDebugReportCallback(VkDebugReportFlagsEXT flags)
{
    assert(instance_);
    VkDebugReportCallbackCreateInfoEXT createInfo = {VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT};
    createInfo.flags = flags;
    createInfo.pfnCallback = &StandardDebugReportCallback;
    CreateDebugReportCallback(createInfo);
}

void Instance::CreateDebugReportCallback(const VkDebugReportCallbackCreateInfoEXT &createInfo)
{
    assert(instance_);
    auto vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT"));
    if (vkCreateDebugReportCallbackEXT == nullptr)
    {
        VK_CALL(VK_ERROR_INITIALIZATION_FAILED);
        return;
    }

    VkDebugReportCallbackEXT debugReportCallback;
    VK_CALL(vkCreateDebugReportCallbackEXT(instance_, &createInfo, nullptr, &debugReportCallback));

    debugReportCallback_ = std::unique_ptr<void, std::function<void(void*)>>(debugReportCallback, [=](void *debugReportCallback) {
        auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT"));
        if (vkDestroyDebugReportCallbackEXT != nullptr)
        {
            vkDestroyDebugReportCallbackEXT(instance_, static_cast<VkDebugReportCallbackEXT>(debugReportCallback), nullptr);
        }});
}

std::vector<PhysicalDevice> Instance::EnumeratePhysicalDevices() const
{
    assert(instance_);
    uint32_t physicalDeviceCount;
    VK_CALL(vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr));

    if (physicalDeviceCount > 0)
    {
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        VK_CALL(vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices.data()));
        return {physicalDevices.begin(), physicalDevices.end()};
    }

    return {};
}

Instance CreateInstance(const Span<std::string> &enabledLayerNames,
                        const Span<std::string> &enabledExtensionNames,
                        const ApplicationInfo &applicationInfo,
                        VkInstanceCreateFlags flags)
{
    return CreateInstanceExt(nullptr, enabledLayerNames, enabledExtensionNames, applicationInfo, flags);
}

Instance CreateInstanceExt(const void *pNext,
                           const Span<std::string> &enabledLayerNames,
                           const Span<std::string> &enabledExtensionNames,
                           const ApplicationInfo &applicationInfo,
                           VkInstanceCreateFlags flags)
{
    VkApplicationInfo vkApplicationInfo(applicationInfo);

    const auto enabledLayerCount = static_cast<uint32_t>(enabledLayerNames.Count());
    const char **ppEnabledLayerNames = nullptr;
    if (enabledLayerCount > 0)
    {
        ppEnabledLayerNames = static_cast<const char **>(alloca(sizeof(char *) * enabledLayerCount));
        for (uint32_t i = 0; i < enabledLayerCount; ++i)
        {
            ppEnabledLayerNames[i] = enabledLayerNames[i].c_str();
        }
    }

    const auto enabledExtensionCount = static_cast<uint32_t>(enabledExtensionNames.Count());
    const char **ppEnabledExtensionNames = nullptr;
    if (enabledExtensionCount > 0)
    {
        ppEnabledExtensionNames = static_cast<const char **>(alloca(sizeof(char *) * enabledExtensionCount));
        for (uint32_t i = 0; i < enabledExtensionCount; ++i)
        {
            ppEnabledExtensionNames[i] = enabledExtensionNames[i].c_str();
        }
    }

    VkInstanceCreateInfo instanceCreateInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                                               pNext,
                                               flags,
                                               &vkApplicationInfo,
                                               enabledLayerCount,
                                               ppEnabledLayerNames,
                                               enabledExtensionCount,
                                               ppEnabledExtensionNames};

    VkInstance vkInstance;
    VK_CALL(vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance));
    return Instance(vkInstance);
}

} // namespace vkw