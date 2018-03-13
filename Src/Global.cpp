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

Version::Version(const uint32_t major, const uint32_t minor, const uint32_t patch)
{
    assert(major < 0x400 && minor < 0x400 && patch < 0x1000);
    impl.version = VK_MAKE_VERSION(major, minor, patch);
}

uint32_t Version::GetMajor() const
{
    return VK_VERSION_MAJOR(impl.version);
}

uint32_t Version::GetMinor() const
{
    return VK_VERSION_MINOR(impl.version);
}

uint32_t Version::GetPatch() const
{
    return VK_VERSION_PATCH(impl.version);
}

bool operator==(const Version &lhs, const Version &rhs)
{
    return lhs.impl.version == rhs.impl.version;
}

bool operator!=(const Version &lhs, const Version &rhs)
{
    return lhs.impl.version != rhs.impl.version;
}

bool operator<(const Version &lhs, const Version &rhs)
{
    return lhs.impl.version < rhs.impl.version;
}

bool operator>(const Version &lhs, const Version &rhs)
{
    return lhs.impl.version > rhs.impl.version;
}

bool operator<=(const Version &lhs, const Version &rhs)
{
    return lhs.impl.version <= rhs.impl.version;
}

bool operator>=(const Version &lhs, const Version &rhs)
{
    return lhs.impl.version >= rhs.impl.version;
}

Extension::Extension(const VkExtensionProperties &properties)
    : name(static_cast<const char*>(properties.extensionName))
{
    version.impl.version = properties.specVersion;
}

Layer::Layer(const VkLayerProperties &properties)
    : name(static_cast<const char*>(properties.layerName))
    , description(static_cast<const char*>(properties.description))
{
    specVersion.impl.version = properties.specVersion;
    implementationVersion.impl.version = properties.implementationVersion;
    extensions = EnumerateExtensions(name);
}

std::vector<Layer> EnumerateLayers()
{
    uint32_t propertyCount;
    VK_CALL(vkEnumerateInstanceLayerProperties(&propertyCount, nullptr));

    if (propertyCount > 0)
    {
        std::vector<VkLayerProperties> properties(propertyCount);
        VK_CALL(vkEnumerateInstanceLayerProperties(&propertyCount, properties.data()));

        return {properties.begin(), properties.end()};
    }

    return {};
}

std::vector<Extension> EnumerateExtensions(const std::string &layerName)
{
    const char *pLayerStr = layerName.length() > 0 ? layerName.c_str() : nullptr;
    uint32_t propertyCount;
    VK_CALL(vkEnumerateInstanceExtensionProperties(pLayerStr, &propertyCount, nullptr));

    if (propertyCount > 0)
    {
        std::vector<VkExtensionProperties> properties(propertyCount);
        VK_CALL(vkEnumerateInstanceExtensionProperties(pLayerStr, &propertyCount, properties.data()));

        return {properties.begin(), properties.end()};
    }

    return {};
}

} // namespace vkw