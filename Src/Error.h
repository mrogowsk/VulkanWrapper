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

#pragma once

#include <cassert>
#include <stdexcept>

#include <vulkan/vulkan.h>

namespace vkw
{

inline std::string to_string(VkResult result)
{
    switch(result)
    {
        case VK_SUCCESS: return "Success";
        case VK_NOT_READY: return "Not ready";
        case VK_TIMEOUT: return "Timeout";
        case VK_EVENT_SET: return "Event set";
        case VK_EVENT_RESET: return "Event reset";
        case VK_INCOMPLETE: return "Incomplete";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "Out of host memory";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "Out of device memory";
        case VK_ERROR_INITIALIZATION_FAILED: return "Initialization failed";
        case VK_ERROR_DEVICE_LOST: return "Device Lost";
        case VK_ERROR_MEMORY_MAP_FAILED: return "Memory map failed";
        case VK_ERROR_LAYER_NOT_PRESENT: return "Layer not present";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "Extension not present";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "Feature not present";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "Incompatible driver";
        case VK_ERROR_TOO_MANY_OBJECTS: return "Too many objects";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "Format not supported";
        case VK_ERROR_FRAGMENTED_POOL: return "Fragmented pool";
        case VK_ERROR_SURFACE_LOST_KHR: return "Surface lost";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "Native window in use";
        case VK_SUBOPTIMAL_KHR: return "Suboptimal";
        case VK_ERROR_OUT_OF_DATE_KHR: return "Out of date";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "Incompatible display";
        case VK_ERROR_VALIDATION_FAILED_EXT: return "Validation failed";
        case VK_ERROR_INVALID_SHADER_NV: return "Invalid shader";
        default: return "Invalid";
    }
}

class Exception : public std::runtime_error
{
public:
    explicit Exception(VkResult result);

    VkResult GetResult() const { return result_; }

private:
    VkResult result_;
};

} // namespace vkw

#define VK_CALL(expr) do { const auto r = (expr); if (r < VK_SUCCESS) throw vkw::Exception(r); } while (0)