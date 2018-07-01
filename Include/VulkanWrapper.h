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
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

namespace vkw
{

namespace Impl
{

template <typename T>
class Object
{
public:

    operator T()
    {
        return static_cast<T>(obj_.get());
    }

    operator T() const
    {
        return static_cast<T>(obj_.get());
    }

protected:

    std::unique_ptr<void, std::function<void(void*)>> obj_;
};

template <typename T, void(*D)(T, const VkAllocationCallbacks*)>
class DispatchableObject : public Object<T>
{
public:

    DispatchableObject()
    {
        Object<T>::obj_ = std::unique_ptr<void, std::function<void(void*)>>(VK_NULL_HANDLE, [](void*) {});
    }

    explicit DispatchableObject(T& t)
    {
        Object<T>::obj_ = std::unique_ptr<void, std::function<void(void*)>>(t, [](void* t) { D(static_cast<T>(t), nullptr); });
    }
};

template <typename T, typename M, void(*D)(M, T, const VkAllocationCallbacks*)>
class NonDispatchableObject : public Object<T>
{
public:

    NonDispatchableObject()
        : m_(VK_NULL_HANDLE)
    {
        Object<T>::obj_ = std::unique_ptr<void, std::function<void(void*)>>(VK_NULL_HANDLE, [](void*) {});
    }

    NonDispatchableObject(M m, T& t, bool destroyable = true)
        : m_(m)
    {
        if (destroyable)
        {
            Object<T>::obj_ = std::unique_ptr<void, std::function<void(void*)>>(t, [=](void* t) { D(m, static_cast<T>(t), nullptr); });
        }
        else
        {
            Object<T>::obj_ = std::unique_ptr<void, std::function<void(void*)>>(t, [](void*) {});
        }
    }

    M GetCreator() const
    {
        return m_;
    }

protected:

    M m_;
};

} // namepsace Impl

template <typename T>
class Span
{
public:

    Span() = default;
    ~Span() = default;

    Span(const Span &other)
        : elementCount_(other.elementCount_)
    {
        elements_ = other.elements_;
    }

    Span(Span &&other) noexcept
        : elementCount_(other.elementCount_)
    {
        elements_ = other.elements_;
        other.elements_ = nullptr;
        other.elementCount_ = 0;
    }

    Span& operator=(const Span &other)
    {
        elements_ = other.elements_;
        elementCount_ = other.elementCount_;
    }

    Span& operator=(Span &&other) noexcept
    {
        elements_ = other.elements_;
        other.elements_ = nullptr;
        elementCount_ = other.elementCount_;
        other.elementCount_ = 0;
    }

    const T* begin() const
    {
        return elements_;
    }

    const T* cbegin() const
    {
        return elements_;
    }

    const T* end() const
    {
        return elements_ + elementCount_;
    }

    const T* cend() const
    {
        return elements_ + elementCount_;
    }

    Span(std::nullptr_t) {}

    template <typename indexType>
    Span(const T *ptr, indexType count)
        : elementCount_(static_cast<uint32_t>(count))
    {
        elements_ = ptr;
    }

    Span(const T &element)
        : elementCount_(1)
    {
        elements_ = &element;
    }

    template <std::size_t size>
    Span(const std::array<T, size> &elements)
        : elementCount_(size)
    {
        elements_ = &*std::cbegin(elements);
    }

    Span(const std::vector<T> &elements)
        : elementCount_(static_cast<uint32_t>(std::cend(elements) - std::cbegin(elements)))
    {
        elements_ = &*std::cbegin(elements);
    }

    Span(const std::initializer_list<T> &list)
        : elementCount_(static_cast<uint32_t>(list.size()))
    {
        elements_ = &*std::begin(list);
    }

    explicit operator bool() const
    {
        return elements_ != nullptr && elementCount_ > 0;
    }

    const T& operator[](const size_t index) const
    {
        assert(index < elementCount_ && elements_ != nullptr);
        return elements_[index];
    }

    uint32_t Count() const
    {
        return elementCount_;
    }

    const T* Data() const
    {
        return elements_;
    }

    uint32_t Size() const
    {
        return elementCount_ * sizeof(T);
    }

    template <typename type>
    void Emplace(type *pTypes) const
    {
        for (uint32_t i = 0; i < elementCount_; ++i)
        {
            pTypes[i] = static_cast<type>(elements_[i]);
        }
    }

private:

    const T *elements_ = nullptr;
    const uint32_t elementCount_ = 0;
}; // class Span

template <typename T>
class Span2
{
public:

    Span2() = default;
    ~Span2() = default;

    Span2(const Span2 &other)
        : elementCount_(other.elementCount_)
        , ptrType_(other.ptrType)
    {
        ptr_ = other.ptr_;
    }

    Span2(Span2 &&other) noexcept
        : elementCount_(other.elementCount_)
        , ptrType_(other.ptrType_)
    {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        other.elementCount_ = 0;
    }

    Span2& operator=(const Span2 &other)
    {
        ptrType_ = other.ptrType_;
        ptr_ = other.ptr_;
        elementCount_ = other.elementCount_;
    }

    Span2& operator=(Span2 &&other) noexcept
    {
        ptrType_ = other.ptrType_;
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        elementCount_ = other.elementCount_;
        other.elementCount_ = 0;
    }

    Span2(std::nullptr_t) { ptr_ = nullptr; }

    template <typename indexType>
    Span2(const T *ptr, indexType count)
        : elementCount_(static_cast<uint32_t>(count))
        , ptrType_(false)
    {
        elements_ = ptr;
    }

    template <typename indexType>
    Span2(const T **ptrs, indexType count)
        : elementCount_(static_cast<uint32_t>(count))
        , ptrType_(true)
    {
        ptrs_ = ptrs;
    }

    Span2(const T &element)
        : elementCount_(1)
        , ptrType_(false)
    {
        elements_ = &element;
    }

    Span2(const T *ptr)
        : elementCount_(1)
        , ptrType_(false)
    {
        elements_ = ptr;
    }

    template <std::size_t size>
    Span2(const std::array<T, size> &elements)
        : elementCount_(size)
        , ptrType_(false)
    {
        elements_ = &*std::cbegin(elements);
    }

    template <std::size_t size>
    Span2(const std::array<const T*, size> &ptrs)
        : elementCount_(size)
        , ptrType_(true)
    {
        ptrs_ = &*std::cbegin(ptrs);
    }

    Span2(const std::vector<T> &elements)
        : elementCount_(static_cast<uint32_t>(std::cend(elements) - std::cbegin(elements)))
        , ptrType_(false)
    {
        elements_ = &*std::cbegin(elements);
    }

    Span2(const std::vector<const T*> &ptrs)
        : elementCount_(static_cast<uint32_t>(std::cend(ptrs) - std::cbegin(ptrs)))
        , ptrType_(true)
    {
        ptrs_ = &*std::cbegin(ptrs);
    }

    Span2(const std::initializer_list<T> &list)
        : elementCount_(static_cast<uint32_t>(list.size()))
        , ptrType_(false)
    {
        elements_ = &*std::begin(list);
    }

    Span2(const std::initializer_list<const T*> &list)
        : elementCount_(static_cast<uint32_t>(list.size()))
        , ptrType_(true)
    {
        ptrs_ = &*std::begin(list);
    }

    explicit operator bool() const
    {
        return ptr_ != nullptr && elementCount_ > 0;
    }

    const T& operator[](const size_t index) const
    {
        assert(index < elementCount_ && ptr_ != nullptr);
        if (ptrType_)
        {
            return *ptrs_[index];
        }
        else
        {
            return elements_[index];
        }
    }

    bool ContainsElements() const { return !ptrType_; }

    bool ContainsPtrs() const { return ptrType_; }

    uint32_t Count() const
    {
        return elementCount_;
    }

    uint32_t Size() const
    {
        return elementCount_ * (ptrType_ ? sizeof(T*) : sizeof(T));
    }

    template <typename type>
    void Emplace(type *pTypes) const
    {
        if (ptrType_)
        {
            for (uint32_t i = 0; i < elementCount_; ++i)
            {
                pTypes[i] = static_cast<type>(*ptrs_[i]);
            }
        }
        else
        {
            for (uint32_t i = 0; i < elementCount_; ++i)
            {
                pTypes[i] = static_cast<type>(elements_[i]);
            }
        }
    }

private:

    union
    {
        const T *elements_;
        const T *const *ptrs_;
        const void *ptr_;
    };
    bool ptrType_ = false; // false = elements, true = ptrs
    const uint32_t elementCount_ = 0;
}; // class Span2

struct Version
{
    Version() = default;
    explicit Version(uint32_t major, uint32_t minor = 0, uint32_t patch = 0);

    uint32_t GetMajor() const;
    uint32_t GetMinor() const;
    uint32_t GetPatch() const;

    struct Impl
    {
        uint32_t version = 0;
    } impl;
};

bool operator==(const Version& lhs, const Version& rhs);
bool operator!=(const Version& lhs, const Version& rhs);
bool operator< (const Version& lhs, const Version& rhs);
bool operator> (const Version& lhs, const Version& rhs);
bool operator<=(const Version& lhs, const Version& rhs);
bool operator>=(const Version& lhs, const Version& rhs);

struct Extension
{
    explicit Extension(const VkExtensionProperties &properties);

    std::string name;
    Version version;
};

struct Layer
{
    explicit Layer(const VkLayerProperties &properties);

    std::string            name;
    Version                specVersion;
    Version                implementationVersion;
    std::string            description;
    std::vector<Extension> extensions;
};

std::vector<Layer> EnumerateLayers();
std::vector<Extension> EnumerateExtensions(const std::string &layerName = {});

class BufferView;
class DescriptorSet;
class DescriptorSetLayout;
class DeviceMemory;
class Event;
class Fence;
class Framebuffer;
class Image;
class ImageView;
class PipelineCache;
class PipelineLayout;
class QueryPool;
class Queue;
class RenderPass;
class Sampler;
class Semaphore;
class ShaderModule;
class Surface;
class Swapchain;

struct SpecializationInfo
{
    SpecializationInfo() = default;

    template<typename T>
    SpecializationInfo(const T *pData, size_t entryCount)
        : dataSize(sizeof(T) * entryCount), pData(pData)
    {
        for (size_t i = 0; i < entryCount; ++i) { AppendEntry<T>(false); }
        UpdateEntryData();
    }

    template<typename T>
    explicit SpecializationInfo(const T &pData)
        : entries({{0, 0, sizeof(T)}}), dataSize(sizeof(T)), pData(&pData)
    {
        UpdateEntryData();
    }

    template<typename T>
    explicit SpecializationInfo(const Span<T> &entries)
        : dataSize(sizeof(T) * entries.size()), pData(entries.data())
    {
        for (const auto &e : entries) { AppendEntry<T>(false); }
        UpdateEntryData();
    }

    SpecializationInfo(const void *pData, size_t dataSize, const Span<VkSpecializationMapEntry> &entries)
        : entries(entries.begin(), entries.end()), dataSize(dataSize), pData(pData)
    {
        UpdateEntryData();
    }

    template <typename... types>
    typename std::enable_if<sizeof...(types) == 0>::type AppendEntries(bool adjustDataSize)
    {}

    template<typename T, typename... types>
    void AppendEntries(bool adjustDataSize = true)
    {
        AppendEntry<T>(adjustDataSize);
        AppendEntries<types...>(adjustDataSize);
    }

    template<typename T>
    void AppendEntry(bool adjustDataSize = true)
    {
        if (!entries.empty())
        {
            const VkSpecializationMapEntry &lastEntry = entries.back();
            uint32_t nextOffset = lastEntry.offset + static_cast<uint32_t>(lastEntry.size);
            if (nextOffset % alignof(T) != 0)
            {
                nextOffset = nextOffset + alignof(T) - nextOffset % alignof(T);
            }
            entries.push_back({lastEntry.constantID + 1, nextOffset, sizeof(T)});
            if (adjustDataSize)
            {
                dataSize = entries.back().offset + sizeof(T);
            }
        }
        else
        {
            entries.push_back({0, 0, sizeof(T)});
            if (adjustDataSize)
            {
                dataSize = sizeof(T);
            }
        }
        UpdateEntryData();
    }

private:
    void UpdateEntryData()
    {
        mapEntryCount = static_cast<uint32_t>(entries.size());
        pMapEntries = entries.data();
    }

    uint32_t mapEntryCount = 0;
    const VkSpecializationMapEntry *pMapEntries = nullptr;

public:
    size_t dataSize = 0;
    const void *pData = nullptr;

private:
    std::vector<VkSpecializationMapEntry> entries;
}; // struct SpecializationInfo

template <typename... types>
SpecializationInfo CreateSpecializationInfo(const void *pData, size_t dataSize)
{
    SpecializationInfo specializationInfo(pData, dataSize);
    specializationInfo.AppendEntries<types...>(false);
    return specializationInfo;
}

template <typename... types>
SpecializationInfo CreateSpecializationInfo(const void *pData)
{
    SpecializationInfo specializationInfo;
    specializationInfo.AppendEntries<types...>();
    specializationInfo.pData = pData;
    return specializationInfo;
}

class ShaderModule
{
public:

    ShaderModule() = default;
    explicit ShaderModule(VkDevice device, VkShaderModule shaderModule)
        : shaderModule_(device, shaderModule) {}

    explicit operator bool() const
    {
        return shaderModule_ != VK_NULL_HANDLE;
    }

    explicit operator VkShaderModule() const
    {
        return shaderModule_;
    }

    bool operator== (const ShaderModule &other) const
    {
        return shaderModule_ == other.shaderModule_;
    }

    bool operator!= (const ShaderModule &other) const
    {
        return shaderModule_ != other.shaderModule_;
    }

private:

    Impl::NonDispatchableObject<VkShaderModule, VkDevice, vkDestroyShaderModule> shaderModule_;
}; // class ShaderModule

class Pipeline
{
public:

    Pipeline() = default;
    explicit Pipeline(VkDevice device, VkPipeline pipeline)
        : pipeline_(device, pipeline) {}

    explicit operator bool() const
    {
        return pipeline_ != VK_NULL_HANDLE;
    }

    explicit operator VkPipeline() const
    {
        return pipeline_;
    }

    bool operator== (const Pipeline &other) const
    {
        return pipeline_ == other.pipeline_;
    }

    bool operator!= (const Pipeline &other) const
    {
        return pipeline_ != other.pipeline_;
    }

    struct ShaderStage
    {
        ShaderStage() = default;
        explicit ShaderStage(const ShaderModule &module)
            : module(&module) {}

        ShaderStage(const ShaderModule &module, std::string entryPointName,
                    VkShaderStageFlagBits stage, VkPipelineShaderStageCreateFlags flags = 0,
                    SpecializationInfo *pSpecializationInfo = nullptr)
            : module(&module), entryPointName(std::move(entryPointName)), stage(stage), flags(flags)
            , pSpecializationInfo(pSpecializationInfo) {}

        ShaderStage(const void *pNext, const ShaderModule &module, std::string entryPointName,
                    VkShaderStageFlagBits stage, VkPipelineShaderStageCreateFlags flags = 0,
                    SpecializationInfo *pSpecializationInfo = nullptr)
            : pNext(pNext), module(&module), entryPointName(std::move(entryPointName)), stage(stage)
            , flags(flags), pSpecializationInfo(pSpecializationInfo) {}

        explicit operator VkPipelineShaderStageCreateInfo() const
        {
            return {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, pNext, flags, stage,
                    VkShaderModule(*module), entryPointName.c_str(), reinterpret_cast<VkSpecializationInfo*>(pSpecializationInfo)};
        }

        const void *pNext = nullptr;
        VkPipelineShaderStageCreateFlags flags = 0;
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_ALL;
        const ShaderModule *module = nullptr;
        std::string entryPointName;
        SpecializationInfo *pSpecializationInfo = nullptr;
    }; // struct PipelineShaderStage

    struct VertexInputState
    {
        VertexInputState() = default;
        VertexInputState(const Span<VkVertexInputBindingDescription> &vertexBindingDescriptions,
                         const Span<VkVertexInputAttributeDescription> &vertexAttributeDescriptions, VkPipelineVertexInputStateCreateFlags flags = 0)
            : vertexBindingDescriptions(vertexBindingDescriptions.cbegin(), vertexBindingDescriptions.cend())
            , vertexAttributeDescriptions(vertexAttributeDescriptions.cbegin(), vertexAttributeDescriptions.cend()), flags(flags) {}
        VertexInputState(const void *pNext, const Span<VkVertexInputBindingDescription> &vertexBindingDescriptions,
                         const Span<VkVertexInputAttributeDescription> &vertexAttributeDescriptions, VkPipelineVertexInputStateCreateFlags flags = 0)
            : pNext(pNext), vertexBindingDescriptions(vertexBindingDescriptions.cbegin(), vertexBindingDescriptions.cend())
            , vertexAttributeDescriptions(vertexAttributeDescriptions.cbegin(), vertexAttributeDescriptions.cend()), flags(flags) {}

        const void *pNext = nullptr;
        VkPipelineVertexInputStateCreateFlags flags = 0;
        std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions;
        std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptions;

        explicit operator VkPipelineVertexInputStateCreateInfo() const
        {
            return {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, pNext, flags,
                    static_cast<uint32_t>(vertexBindingDescriptions.size()),
                    vertexBindingDescriptions.empty() ? nullptr : vertexBindingDescriptions.data(),
                    static_cast<uint32_t>(vertexAttributeDescriptions.size()),
                    vertexAttributeDescriptions.empty() ? nullptr : vertexAttributeDescriptions.data()};
        }
    };

    struct InputAssemblyState
    {
        InputAssemblyState() = default;
        explicit InputAssemblyState(VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable = VK_FALSE,
                           VkPipelineInputAssemblyStateCreateFlags flags = 0)
            : topology(topology), primitiveRestartEnable(primitiveRestartEnable), flags(flags) {}
        InputAssemblyState(const void *pNext, VkPrimitiveTopology topology, VkBool32 primitiveRestartEnable = VK_FALSE,
                           VkPipelineInputAssemblyStateCreateFlags flags = 0)
            : pNext(pNext), topology(topology), primitiveRestartEnable(primitiveRestartEnable), flags(flags) {}

    private:
        const VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    public:
        const void *pNext = nullptr;
        VkPipelineInputAssemblyStateCreateFlags flags = 0;
        VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 primitiveRestartEnable = VK_FALSE;
    };
    static_assert(sizeof(InputAssemblyState) == sizeof(VkPipelineDynamicStateCreateInfo), "sizeof(InputAssemblyState) != sizeof(VkPipelineDynamicStateCreateInfo)!");

    struct TessellationState
    {
        TessellationState() = default;
        explicit TessellationState(uint32_t patchControlPoints, VkPipelineTessellationStateCreateFlags flags = 0)
            : patchControlPoints(patchControlPoints), flags(flags) {}
        TessellationState(const void *pNext, uint32_t patchControlPoints, VkPipelineTessellationStateCreateFlags flags = 0)
            : pNext(pNext), patchControlPoints(patchControlPoints), flags(flags) {}

    private:
        const VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    public:
        const void *pNext = nullptr;
        VkPipelineTessellationStateCreateFlags flags = 0;
        uint32_t patchControlPoints = 1;
    };
    static_assert(sizeof(TessellationState) == sizeof(VkPipelineTessellationStateCreateInfo), "sizeof(TessellationState) != sizeof(VkPipelineTessellationStateCreateInfo)!");

    struct ViewportState
    {
        ViewportState() = default;
        ViewportState(VkViewport viewport, VkRect2D scissor, VkPipelineViewportStateCreateFlags flags = 0)
            : viewports(1, viewport), scissors(1, scissor), flags(flags) {}
        explicit ViewportState(VkExtent2D viewportExtent, VkPipelineViewportStateCreateFlags flags = 0)
            : viewports(1, {0.0f, 0.0f, static_cast<float>(viewportExtent.width), static_cast<float>(viewportExtent.height), 0.0f, 1.0f})
            , scissors(1, {{0, 0}, viewportExtent}), flags(flags) {}
        ViewportState(VkExtent2D viewportExtent, VkExtent2D scissorExtent, VkPipelineViewportStateCreateFlags flags = 0)
            : viewports(1, {0.0f, 0.0f, static_cast<float>(viewportExtent.width), static_cast<float>(viewportExtent.height), 0.0f, 1.0f})
            , scissors(1, {{0, 0}, scissorExtent}), flags(flags) {}
        ViewportState(const void *pNext, VkViewport viewport, VkRect2D scissor, VkPipelineViewportStateCreateFlags flags = 0)
            : pNext(pNext), viewports(1, viewport), scissors(1, scissor), flags(flags) {}
        ViewportState(const void *pNext, VkExtent2D viewportExtent, VkPipelineViewportStateCreateFlags flags = 0)
            : pNext(pNext), viewports(1, {0.0f, 0.0f, static_cast<float>(viewportExtent.width), static_cast<float>(viewportExtent.height), 0.0f, 1.0f})
            , scissors(1, {{0, 0}, viewportExtent}), flags(flags) {}
        ViewportState(const void *pNext, VkExtent2D viewportExtent, VkExtent2D scissorExtent, VkPipelineViewportStateCreateFlags flags = 0)
            : pNext(pNext), viewports(1, {0.0f, 0.0f, static_cast<float>(viewportExtent.width), static_cast<float>(viewportExtent.height), 0.0f, 1.0f})
            , scissors(1, {{0, 0}, scissorExtent}), flags(flags) {}

        const void *pNext = nullptr;
        VkPipelineViewportStateCreateFlags flags = 0;
        std::vector<VkViewport> viewports;
        std::vector<VkRect2D> scissors;

        explicit operator VkPipelineViewportStateCreateInfo() const
        {
            assert(viewports.size() == scissors.size());
            return {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, pNext, flags,
                    static_cast<uint32_t>(viewports.size()),
                    viewports.empty() ? nullptr : viewports.data(),
                    static_cast<uint32_t>(scissors.size()),
                    scissors.empty() ? nullptr : scissors.data()};
        }
    };

    struct RasterizationState
    {
        RasterizationState() = default;
        explicit RasterizationState(VkBool32 rasterizerDiscardEnable)
            : rasterizerDiscardEnable(rasterizerDiscardEnable) {}
        RasterizationState(VkCullModeFlags cullMode, VkPolygonMode polygonMode, VkFrontFace frontFace, VkBool32 depthClampEnable = VK_FALSE,
                           VkBool32 depthBiasEnable = VK_FALSE, float depthBiasConstantFactor = 0.0f, float depthBiasClamp = 0.0f,
                           float depthBiasSlopeFactor = 0.0f, float lineWidth = 1.0f, VkPipelineRasterizationStateCreateFlags flags = 0)
            : cullMode(cullMode), polygonMode(polygonMode), frontFace(frontFace), depthClampEnable(depthClampEnable)
            , depthBiasEnable(depthBiasEnable), depthBiasConstantFactor(depthBiasConstantFactor), depthBiasClamp(depthBiasClamp)
            , depthBiasSlopeFactor(depthBiasSlopeFactor), lineWidth(lineWidth), flags(flags) {}
        RasterizationState(const void *pNext, VkCullModeFlags cullMode, VkPolygonMode polygonMode, VkFrontFace frontFace, VkBool32 depthClampEnable = VK_FALSE,
                           VkBool32 depthBiasEnable = VK_FALSE, float depthBiasConstantFactor = 0.0f, float depthBiasClamp = 0.0f,
                           float depthBiasSlopeFactor = 0.0f, float lineWidth = 1.0f, VkPipelineRasterizationStateCreateFlags flags = 0)
            : pNext(pNext), cullMode(cullMode), polygonMode(polygonMode), frontFace(frontFace), depthClampEnable(depthClampEnable)
            , depthBiasEnable(depthBiasEnable), depthBiasConstantFactor(depthBiasConstantFactor), depthBiasClamp(depthBiasClamp)
            , depthBiasSlopeFactor(depthBiasSlopeFactor), lineWidth(lineWidth), flags(flags) {}

    private:
        VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    public:
        const void *pNext = nullptr;
        VkPipelineRasterizationStateCreateFlags flags = 0;
        VkBool32 depthClampEnable = VK_FALSE;
        VkBool32 rasterizerDiscardEnable = VK_FALSE;
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
        VkBool32 depthBiasEnable = VK_FALSE;
        float depthBiasConstantFactor = 0.0f;
        float depthBiasClamp = 0.0f;
        float depthBiasSlopeFactor = 0.0f;
        float lineWidth = 1.0f;
    };
    static_assert(sizeof(RasterizationState) == sizeof(VkPipelineRasterizationStateCreateInfo), "sizeof(RasterizationState) != sizeof(VkPipelineRasterizationStateCreateInfo)!");

    struct MultisampleState
    {
        MultisampleState() = default;
        explicit MultisampleState(VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable = VK_FALSE, float minSampleShading = 1.0f,
                         const VkSampleMask* pSampleMask = nullptr, VkBool32 alphaToCoverageEnable = VK_FALSE, VkBool32 alphaToOneEnable = VK_FALSE,
                         VkPipelineMultisampleStateCreateFlags flags = 0)
            : rasterizationSamples(rasterizationSamples), sampleShadingEnable(sampleShadingEnable), minSampleShading(minSampleShading)
            , pSampleMask(pSampleMask), alphaToCoverageEnable(alphaToCoverageEnable), alphaToOneEnable(alphaToOneEnable)
            , flags(flags) {}
        MultisampleState(const void *pNext, VkSampleCountFlagBits rasterizationSamples, VkBool32 sampleShadingEnable = VK_FALSE, float minSampleShading = 1.0f,
                         const VkSampleMask* pSampleMask = nullptr, VkBool32 alphaToCoverageEnable = VK_FALSE, VkBool32 alphaToOneEnable = VK_FALSE,
                         VkPipelineMultisampleStateCreateFlags flags = 0)
            : pNext(pNext), rasterizationSamples(rasterizationSamples), sampleShadingEnable(sampleShadingEnable), minSampleShading(minSampleShading)
            , pSampleMask(pSampleMask), alphaToCoverageEnable(alphaToCoverageEnable), alphaToOneEnable(alphaToOneEnable)
            , flags(flags) {}

    private:
        VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    public:
        const void *pNext = nullptr;
        VkPipelineMultisampleStateCreateFlags flags = 0;
        VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkBool32 sampleShadingEnable = VK_FALSE;
        float minSampleShading = 1.0f;
        const VkSampleMask* pSampleMask = nullptr;
        VkBool32 alphaToCoverageEnable = VK_FALSE;
        VkBool32 alphaToOneEnable = VK_FALSE;
    };
    static_assert(sizeof(MultisampleState) == sizeof(VkPipelineMultisampleStateCreateInfo), "sizeof(MultisampleState) != sizeof(VkPipelineMultisampleStateCreateInfo)!");

    struct DepthStencilState
    {
        DepthStencilState() = default;
        explicit DepthStencilState(VkBool32 depthTestEnable)
            : depthTestEnable(depthTestEnable) {}
        DepthStencilState(VkBool32 depthWriteEnable, VkCompareOp depthCompareOp)
            : depthWriteEnable(depthWriteEnable), depthCompareOp(depthCompareOp) {}

    private:
        VkStructureType sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    public:
        const void *pNext = nullptr;
        VkPipelineDepthStencilStateCreateFlags flags = 0;
        VkBool32 depthTestEnable = VK_TRUE;
        VkBool32 depthWriteEnable = VK_TRUE;
        VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
        VkBool32 depthBoundsTestEnable = VK_FALSE;
        VkBool32 stencilTestEnable = VK_FALSE;
        VkStencilOpState front = {};
        VkStencilOpState back = {};
        float minDepthBounds = 0.0f;
        float maxDepthBounds = 1.0f;
    };
    static_assert(sizeof(DepthStencilState) == sizeof(VkPipelineDepthStencilStateCreateInfo), "sizeof(DepthStencilState) != sizeof(VkPipelineDepthStencilStateCreateInfo)!");

    struct ColorBlendState
    {
        ColorBlendState() = default;

        const void *pNext = nullptr;
        VkPipelineColorBlendStateCreateFlags flags = 0;
        VkBool32 logicOpEnable = VK_FALSE;
        VkLogicOp logicOp = VK_LOGIC_OP_COPY;
        std::vector<VkPipelineColorBlendAttachmentState> attachments;
        float blendConstants[4] = {};

        explicit operator VkPipelineColorBlendStateCreateInfo() const
        {
            return {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, pNext, flags, logicOpEnable, logicOp,
                    static_cast<uint32_t>(attachments.size()), attachments.empty() ? nullptr : attachments.data(),
                    blendConstants[0], blendConstants[1], blendConstants[2], blendConstants[3]};
        }
    };

    struct DynamicState
    {
        DynamicState() = default;
        explicit DynamicState(const Span<VkDynamicState> &dynamicStates, VkPipelineDynamicStateCreateFlags flags = 0)
            : dynamicStates(dynamicStates.cbegin(), dynamicStates.cend()), flags(flags) {}
        DynamicState(const void *pNext, const Span<VkDynamicState> &dynamicStates, VkPipelineDynamicStateCreateFlags flags = 0)
            : pNext(pNext), dynamicStates(dynamicStates.cbegin(), dynamicStates.cend()), flags(flags) {}

        const void *pNext = nullptr;
        VkPipelineDynamicStateCreateFlags flags = 0;
        std::vector<VkDynamicState> dynamicStates;

        explicit operator VkPipelineDynamicStateCreateInfo() const
        {
            return {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, pNext, flags,
                    static_cast<uint32_t>(dynamicStates.size()), dynamicStates.empty() ? nullptr : dynamicStates.data()};
        }
    };

private:

    Impl::NonDispatchableObject<VkPipeline, VkDevice, vkDestroyPipeline> pipeline_;
}; // class Pipeline

struct GraphicsPipelineStateDescription
{
    GraphicsPipelineStateDescription() = default;

    Pipeline::VertexInputState vertexInputState;
    Pipeline::InputAssemblyState inputAssemblyState;
    Pipeline::TessellationState *tessellationState = nullptr;
    Pipeline::ViewportState *viewportState = nullptr;
    Pipeline::RasterizationState rasterizationState;
    Pipeline::MultisampleState *multisampleState = nullptr;
    Pipeline::DepthStencilState *depthStencilState = nullptr;
    Pipeline::ColorBlendState *colorBlendState = nullptr;
    Pipeline::DynamicState *dynamicState = nullptr;
};

class PipelineCache
{
public:

    PipelineCache() = default;
    explicit PipelineCache(VkDevice device, VkPipelineCache pipelineCache)
        : pipelineCache_(device, pipelineCache)
    {}

    explicit operator bool() const
    {
        return pipelineCache_ != VK_NULL_HANDLE;
    }

    explicit operator VkPipelineCache() const
    {
        return pipelineCache_;
    }

    bool operator== (const PipelineCache &other) const
    {
        return pipelineCache_ == other.pipelineCache_;
    }

    bool operator!= (const PipelineCache &other) const
    {
        return pipelineCache_ != other.pipelineCache_;
    }

    std::vector<char> GetData() const;
    void Merge(const Span2<PipelineCache> &caches) const;

private:

    Impl::NonDispatchableObject<VkPipelineCache, VkDevice, vkDestroyPipelineCache> pipelineCache_;
}; // class PipelineCache


class Buffer
{
public:
    Buffer() = default;
    explicit Buffer(VkDevice device, VkBuffer buffer);

    explicit operator bool() const
    {
        return buffer_ != VK_NULL_HANDLE;
    }

    explicit operator VkBuffer() const
    {
        return buffer_;
    }

    bool operator==(const Buffer &other) const
    {
        return buffer_ == other.buffer_;
    }

    bool operator!=(const Buffer &other) const
    {
        return buffer_ != other.buffer_;
    }

    BufferView CreateBufferView(VkFormat format, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE, VkBufferViewCreateFlags flags = 0) const;
    BufferView CreateBufferViewExt(const void *pNext, VkFormat format, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE, VkBufferViewCreateFlags flags = 0) const;

    VkMemoryRequirements GetMemoryRequirements() const;

    void BindMemory(const DeviceMemory &memory, VkDeviceSize offset) const;

    VkBufferMemoryBarrier CreateMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDeviceSize offset = 0,
                                              VkDeviceSize size = VK_WHOLE_SIZE) const;
    VkBufferMemoryBarrier CreateMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkDeviceSize offset = 0,
                                                 VkDeviceSize size = VK_WHOLE_SIZE) const;
    VkBufferMemoryBarrier CreateConcurrentMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, uint32_t srcQueueFamilyIndex,
                                                        uint32_t dstQueueFamilyIndex, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    VkBufferMemoryBarrier CreateConcurrentMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, uint32_t srcQueueFamilyIndex,
                                                           uint32_t dstQueueFamilyIndex, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

private:
    Impl::NonDispatchableObject<VkBuffer, VkDevice, vkDestroyBuffer> buffer_;
}; // class Buffer

class BufferView
{
public:
    BufferView() = default;
    explicit BufferView(VkDevice device, VkBufferView view);

    explicit operator bool() const
    {
        return view_ != VK_NULL_HANDLE;
    }

    explicit operator VkBufferView() const
    {
        return view_;
    }

    bool operator==(const BufferView &other) const
    {
        return view_ == other.view_;
    }

    bool operator!=(const BufferView &other) const
    {
        return view_ != other.view_;
    }

private:
    Impl::NonDispatchableObject<VkBufferView, VkDevice, vkDestroyBufferView> view_;
}; // class BufferView

struct MemoryBarrier
{
    MemoryBarrier() = default;
    MemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
        : srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask) {}
    MemoryBarrier(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
        : pNext(pNext), srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask) {}

private:
    VkStructureType sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;

public:
    const void *pNext = nullptr;
    VkAccessFlags srcAccessMask = 0;
    VkAccessFlags dstAccessMask = 0;
};
static_assert(sizeof(MemoryBarrier) == sizeof(VkMemoryBarrier), "sizeof(MemoryBarrier) != sizeof(VkMemoryBarrier)!");

class CommandBuffer
{
public:
    CommandBuffer() = default;
    explicit CommandBuffer(VkCommandBuffer cmdBuffer)
        : cmdBuffer_(cmdBuffer) {}

    explicit operator bool() const
    {
        return cmdBuffer_ != VK_NULL_HANDLE;
    }

    explicit operator VkCommandBuffer() const
    {
        return cmdBuffer_;
    }

    bool operator== (const CommandBuffer &other) const
    {
        return cmdBuffer_ == other.cmdBuffer_;
    }

    bool operator!= (const CommandBuffer &other) const
    {
        return cmdBuffer_ != other.cmdBuffer_;
    }

    void Begin(VkCommandBufferUsageFlags flags = 0) const;
    void BeginExt(const void *pNext, VkCommandBufferUsageFlags flags = 0) const;

    void BeginSecondary(VkCommandBufferUsageFlags flags = 0, VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = 0, VkQueryPipelineStatisticFlags pipelineStatistics = 0) const;
    void BeginSecondary(const RenderPass &renderPass, uint32_t subpass, VkCommandBufferUsageFlags flags = 0,
                        VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = 0, VkQueryPipelineStatisticFlags pipelineStatistics = 0) const;
    void BeginSecondary(const RenderPass &renderPass, uint32_t subpass, const Framebuffer &framebuffer, VkCommandBufferUsageFlags flags = 0,
                        VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = 0, VkQueryPipelineStatisticFlags pipelineStatistics = 0) const;
    void BeginSecondaryExt(const void *pBeginInfoNext, const void *pInheritanceInfoNext, VkCommandBufferUsageFlags flags = 0, VkBool32 occlusionQueryEnable = VK_FALSE,
                           VkQueryControlFlags queryFlags = 0, VkQueryPipelineStatisticFlags pipelineStatistics = 0) const;
    void BeginSecondaryExt(const void *pBeginInfoNext, const void *pInheritanceInfoNext, const RenderPass &renderPass, uint32_t subpass, VkCommandBufferUsageFlags flags = 0,
                           VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = 0, VkQueryPipelineStatisticFlags pipelineStatistics = 0) const;
    void BeginSecondaryExt(const void *pBeginInfoNext, const void *pInheritanceInfoNext, const RenderPass &renderPass, uint32_t subpass, const Framebuffer &framebuffer,
                           VkCommandBufferUsageFlags flags = 0, VkBool32 occlusionQueryEnable = VK_FALSE, VkQueryControlFlags queryFlags = 0, VkQueryPipelineStatisticFlags pipelineStatistics = 0) const;

    void Reset(VkCommandBufferResetFlags flags = 0) const;
    void End() const;

    void FillBuffer(const Buffer &dstBuffer, uint32_t data, VkDeviceSize dstOffset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

    template <typename T>
    void FillBuffer(const Buffer &dstBuffer, T data, VkDeviceSize dstOffset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const
    {
        FillBuffer(dstBuffer, *reinterpret_cast<uint32_t*>(&data), dstOffset, size);
    }

    void UpdateBuffer(const Buffer &dstBuffer, const void* pData, VkDeviceSize size) const;
    void UpdateBuffer(const Buffer &dstBuffer, const void* pData, VkDeviceSize dstOffset, VkDeviceSize size) const;

    template <typename T>
    void UpdateBuffer(const Buffer &dstBuffer, const T &data) const
    {
        UpdateBuffer(dstBuffer, &data, sizeof(T));
    }

    template <typename T>
    void UpdateBuffer(const Buffer &dstBuffer, const T &data, uint32_t offset) const
    {
        UpdateBuffer(dstBuffer, &data, offset, sizeof(T));
    }

    void CopyBuffer(const Buffer &srcBuffer, const Buffer &dstBuffer, VkDeviceSize size, VkDeviceSize srcOffset = 0, VkDeviceSize dstOffset = 0) const;
    void CopyBufferRegions(const Buffer &srcBuffer, const Buffer &dstBuffer, const Span<VkBufferCopy> &regions) const;
    void CopyBufferToImage(const Buffer &srcBuffer, const Image &dstImage, VkImageLayout imageLayout, const Span<VkBufferImageCopy> &regions) const;

    void ClearColorImage(const Image &image, VkImageLayout imageLayout, const VkClearColorValue &color, uint32_t baseMipLevel = 0,
                         uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const;
    void ClearColorImage(const Image &image, VkImageLayout imageLayout, const VkClearColorValue &color, const Span<VkImageSubresourceRange> &ranges) const;
    void ClearDepthStencilImage(const Image &image, VkImageLayout imageLayout, const VkClearDepthStencilValue &depthStencil, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const;
    void ClearDepthStencilImage(const Image &image, VkImageLayout imageLayout, const VkClearDepthStencilValue &depthStencil, const Span<VkImageSubresourceRange> &ranges) const;
    void ClearAttachments(const Span<VkClearAttachment> &attachments, const Span<VkClearRect> &rects) const;

    void CopyImageToBuffer(const Image &srcImage, VkImageLayout imageLayout, const Buffer &dstBuffer, const Span<VkBufferImageCopy> &regions) const;
    void CopyImage(const Image &srcImage, VkImageLayout srcImageLayout, const Image &dstImage, VkImageLayout dstImageLayout, const Span<VkImageCopy> &regions) const;
    void BlitImage(const Image &srcImage, VkImageLayout srcImageLayout, const Image &dstImage, VkImageLayout dstImageLayout, const Span<VkImageBlit> &regions, VkFilter filter) const;

    void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<MemoryBarrier> &memoryBarriers, VkDependencyFlags dependencyFlags = 0) const;
    void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<VkBufferMemoryBarrier> &bufferMemoryBarriers, VkDependencyFlags dependencyFlags = 0) const;
    void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<VkImageMemoryBarrier> &imageMemoryBarriers, VkDependencyFlags dependencyFlags = 0) const;
    void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const Span<MemoryBarrier> &memoryBarriers, const Span<VkBufferMemoryBarrier> &bufferMemoryBarriers,
                         const Span<VkImageMemoryBarrier> &imageMemoryBarriers, VkDependencyFlags dependencyFlags = 0) const;

    void BindComputePipeline(const Pipeline &pipeline) const;
    void BindGraphicsPipeline(const Pipeline &pipeline) const;

    void BindComputeDescriptorSets(const PipelineLayout &layout, const Span<DescriptorSet> &descriptorSets, uint32_t firstSet = 0, const Span<uint32_t> &dynamicOffsets = {}) const;
    void BindGraphicsDescriptorSets(const PipelineLayout &layout, const Span<DescriptorSet> &descriptorSets, uint32_t firstSet = 0, const Span<uint32_t> &dynamicOffsets = {}) const;

    void Dispatch(uint32_t x, uint32_t y = 1, uint32_t z = 1) const;
    void DispatchIndirect(const Buffer &buffer, VkDeviceSize offset = 0) const;

    void PushConstants(const PipelineLayout &layout, VkShaderStageFlags stageFlags, const void *pValues, uint32_t offset, uint32_t size) const;

    template <typename T>
    void PushConstants(const PipelineLayout &layout, VkShaderStageFlags stageFlags, const T &values) const
    {
        PushConstants(layout, stageFlags, values, sizeof(T));
    }

    template <typename T>
    void PushConstants(const PipelineLayout &layout, VkShaderStageFlags stageFlags, const T &values, uint32_t offset) const
    {
        PushConstants(layout, stageFlags, &values, offset, sizeof(T));
    }

    void SetViewport(const VkViewport &viewport, uint32_t firstViewport = 0) const;
    void SetViewports(const Span<VkViewport> &viewports, uint32_t firstViewport = 0) const;
    void SetScissor(const VkRect2D &scissor, uint32_t firstScissor = 0) const;
    void SetScissors(const Span<VkRect2D> &scissors, uint32_t firstScissor = 0) const;
    void SetLineWidth(float lineWidth) const;
    void SetDepthBias(float depthBiasConstantFactor = 0.0f, float depthBiasClamp = 0.0f, float depthBiasSlopeFactor = 0.0f);
    void SetBlendConstants(const float blendConstants[4]) const;
    void SetDepthBounds(float minDepthBounds = 0.0f, float maxDepthBounds = 1.0f) const;
    void SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference) const;
    void SetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask) const;
    void SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask) const;
    void ResolveImage(const Image &srcImage, VkImageLayout srcImageLayout, const Image &dstImage, VkImageLayout dstImageLayout, const Span<VkImageResolve> &regions) const;

    void SetEvent(const Event &event, VkPipelineStageFlags stageMask) const;
    void ResetEvent(const Event &event, VkPipelineStageFlags stageMask) const;
    void WaitEvents(const Span2<Event> &events, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
                    const Span<MemoryBarrier> &memoryBarriers = {}, const Span<VkBufferMemoryBarrier> &bufferMemoryBarriers = {}, const Span<VkImageMemoryBarrier> &imageMemoryBarriers = {}) const;

    void ResetQueryPool(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount) const;
    void BeginQuery(const QueryPool &queryPool, uint32_t query, VkQueryControlFlags flags = 0) const;
    void EndQuery(const QueryPool &queryPool, uint32_t query) const;
    void CopyQueryPoolResults(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount, const Buffer &dstBuffer, VkDeviceSize dstOffset = 0, VkQueryResultFlags flags = 0) const;
    void CopyQueryPoolResults(const QueryPool &queryPool, uint32_t firstQuery, uint32_t queryCount, uint32_t elementCountPerResult,
                              const Buffer &dstBuffer, VkDeviceSize dstOffset = 0, VkQueryResultFlags flags = 0) const;
    void WriteTimeStamp(const QueryPool &queryPool, uint32_t query, VkPipelineStageFlagBits pipelineStage) const;

    void BindVertexBuffers(const Span2<Buffer> &vertexBuffers, uint32_t firstBinding = 0) const;
    void BindVertexBuffers(const Span2<Buffer> &vertexBuffers, const Span<VkDeviceSize> &offsets, uint32_t firstBinding = 0) const;

    void BindIndexBuffer(const Buffer &indexBuffer, VkDeviceSize offset = 0, VkIndexType indexType = VK_INDEX_TYPE_UINT32) const;

    void Draw(uint32_t vertexCount, uint32_t firstVertex = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0) const;
    void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t instanceCount = 1, uint32_t firstInstance = 0) const;
    void DrawIndirect(const Buffer &buffer, VkDeviceSize offset, uint32_t drawCount = 1, uint32_t stride = 0) const;
    void DrawIndexedIndirect(const Buffer &buffer, VkDeviceSize offset, uint32_t drawCount = 1, uint32_t stride = 0) const;

    void BeginRenderPass(const RenderPass &renderPass, const Framebuffer &framebuffer, const VkExtent2D &renderArea, const Span<VkClearValue> &clearValues = {},
                         VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
    void BeginRenderPass(const RenderPass &renderPass, const Framebuffer &framebuffer, const VkRect2D &renderArea, const Span<VkClearValue> &clearValues = {},
                         VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
    void BeginRenderPassExt(const void *pNext, const RenderPass &renderPass, const Framebuffer &framebuffer, const VkExtent2D &renderArea, const Span<VkClearValue> &clearValues = {},
                            VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
    void BeginRenderPassExt(const void *pNext, const RenderPass &renderPass, const Framebuffer &framebuffer, const VkRect2D &renderArea, const Span<VkClearValue> &clearValues = {},
                            VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
    void NextSubpass(VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE) const;
    void EndRenderPass() const;

    void ExecuteCommands(const Span<CommandBuffer> &commandBuffers) const;

private:
    VkCommandBuffer cmdBuffer_ = VK_NULL_HANDLE;
}; // class CommandBuffer
static_assert(sizeof(CommandBuffer) == sizeof(VkCommandBuffer), "sizeof(CommandBuffer) != sizeof(VkCommandBuffer)!");

class CommandPool
{
public:
    CommandPool() = default;
    explicit CommandPool(VkDevice device, VkCommandPool commandPool);

    explicit operator bool() const
    {
        return cmdPool_ != VK_NULL_HANDLE;
    }

    explicit operator VkCommandPool() const
    {
        return cmdPool_;
    }

    bool operator==(const CommandPool &other) const
    {
        return cmdPool_ == other.cmdPool_;
    }

    bool operator!=(const CommandPool &other) const
    {
        return cmdPool_ != other.cmdPool_;
    }

    void Reset(VkCommandPoolResetFlags flags = 0) const;

    CommandBuffer AllocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
    CommandBuffer AllocateCommandBufferExt(const void *pNext, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
    std::vector<CommandBuffer> AllocateCommandBuffers(uint32_t commandBufferCount, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
    std::vector<CommandBuffer> AllocateCommandBuffersExt(const void *pNext, uint32_t commandBufferCount, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) const;
    void FreeCommandBuffers(const Span<CommandBuffer> &commandBuffers) const;

private:
    Impl::NonDispatchableObject<VkCommandPool, VkDevice, vkDestroyCommandPool> cmdPool_;

}; // class CommandPool

class DescriptorPool
{
public:
    DescriptorPool() = default;
    explicit DescriptorPool(VkDevice device, VkDescriptorPool descriptorPool)
        : descriptorPool_(device, descriptorPool)
    {}

    explicit operator bool() const
    {
        return descriptorPool_ != VK_NULL_HANDLE;
    }

    explicit operator VkDescriptorPool() const
    {
        return descriptorPool_;
    }

    bool operator==(const DescriptorPool &other) const
    {
        return descriptorPool_ == other.descriptorPool_;
    }

    bool operator!=(const DescriptorPool &other) const
    {
        return descriptorPool_ != other.descriptorPool_;
    }

    void Reset(VkDescriptorPoolResetFlags flags = 0) const;

    DescriptorSet AllocateDescriptorSet(const DescriptorSetLayout &setLayout) const;
    DescriptorSet AllocateDescriptorSetExt(const void *pNext, const DescriptorSetLayout &setLayout) const;
    std::vector<DescriptorSet> AllocateDescriptorSets(const Span2<DescriptorSetLayout> &setLayouts) const;
    std::vector<DescriptorSet> AllocateDescriptorSetsExt(const void *pNext, const Span2<DescriptorSetLayout> &setLayouts) const;

    void FreeDescriptorSets(const Span<DescriptorSet> &descriptorSets) const;

private:
    Impl::NonDispatchableObject<VkDescriptorPool, VkDevice, vkDestroyDescriptorPool> descriptorPool_;

}; // class DescriptorPool

class DescriptorSet
{
public:
    DescriptorSet() = default;
    explicit DescriptorSet(VkDescriptorSet descriptorSet)
        : descriptorSet_(descriptorSet)
    {}

    explicit operator bool() const
    {
        return descriptorSet_ != VK_NULL_HANDLE;
    }

    explicit operator VkDescriptorSet() const
    {
        return descriptorSet_;
    }

    bool operator== (const DescriptorSet &other) const
    {
        return descriptorSet_ == other.descriptorSet_;
    }

    bool operator!= (const DescriptorSet &other) const
    {
        return descriptorSet_ != other.descriptorSet_;
    }

private:
    VkDescriptorSet descriptorSet_ = VK_NULL_HANDLE;
}; // class DescriptorSet
static_assert(sizeof(DescriptorSet) == sizeof(VkDescriptorSet), "sizeof(DescriptorSet) != sizeof(VkDescriptorSet)!");

class DescriptorSetLayout
{
public:
    DescriptorSetLayout() = default;
    explicit DescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout)
        : descriptorSetLayout_(device, descriptorSetLayout) {}

    explicit operator bool() const
    {
        return descriptorSetLayout_ != VK_NULL_HANDLE;
    }

    explicit operator VkDescriptorSetLayout() const
    {
        return descriptorSetLayout_;
    }

    bool operator==(const DescriptorSetLayout &other) const
    {
        return descriptorSetLayout_ == other.descriptorSetLayout_;
    }

    bool operator!=(const DescriptorSetLayout &other) const
    {
        return descriptorSetLayout_ != other.descriptorSetLayout_;
    }

private:
    Impl::NonDispatchableObject<VkDescriptorSetLayout, VkDevice, vkDestroyDescriptorSetLayout> descriptorSetLayout_;

}; // class DescriptorSetLayout

struct DescriptorImageInfo
{
    DescriptorImageInfo() = default;
    DescriptorImageInfo(const Sampler &sampler, const ImageView &imageView, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED)
        : sampler(&sampler), imageView(&imageView), imageLayout(imageLayout)
    {}

    const Sampler *sampler = nullptr;
    const ImageView *imageView = nullptr;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    explicit operator VkDescriptorImageInfo() const;
};

struct DescriptorBufferInfo
{
    DescriptorBufferInfo() = default;
    explicit DescriptorBufferInfo(const Buffer &buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE)
        : buffer(&buffer), offset(offset), range(range)
    {}

    const Buffer *buffer = nullptr;
    VkDeviceSize offset = 0;
    VkDeviceSize range = VK_WHOLE_SIZE;

    explicit operator VkDescriptorBufferInfo() const;
};

struct ImageDescription
{
private:
    VkStructureType sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;

public:
    const void* pNext = nullptr;
    VkImageCreateFlags flags = 0;
    VkImageType imageType = VK_IMAGE_TYPE_2D;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkExtent3D extent = {1,1,1};
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = 0;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uint32_t queueFamilyIndexCount = 0;
    const uint32_t* pQueueFamilyIndices = nullptr;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};
static_assert(sizeof(ImageDescription) == sizeof(VkImageCreateInfo), "sizeof(ImageDescription) != sizeof(VkImageCreateInfo)!");

struct SamplerDescription
{
private:
    VkStructureType sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

public:
    const void* pNext = nullptr;
    VkSamplerCreateFlags flags = 0;
    VkFilter magFilter = VK_FILTER_LINEAR;
    VkFilter minFilter = VK_FILTER_LINEAR;
    VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    float mipLodBias = 0.f;
    VkBool32 anisotropyEnable = VK_FALSE;
    float maxAnisotropy = 1.0f;
    VkBool32 compareEnable = VK_FALSE;
    VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
    float minLod = 0.f;
    float maxLod = VK_LOD_CLAMP_NONE;
    VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    VkBool32 unnormalizedCoordinates = VK_FALSE;
};
static_assert(sizeof(SamplerDescription) == sizeof(VkSamplerCreateInfo), "sizeof(SamplerDescription) != sizeof(VkSamplerCreateInfo)!");

struct AttachmentDescription
{
    AttachmentDescription() = default;

    AttachmentDescription(VkFormat format, VkAttachmentLoadOp loadOp, VkImageLayout initialLayout, VkAttachmentStoreOp storeOp,
                          VkImageLayout finalLayout, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkAttachmentDescriptionFlags flags = 0)
        : format(format), loadOp(loadOp), initialLayout(initialLayout), storeOp(storeOp), finalLayout(finalLayout)
        , samples(samples), flags(flags) {}

    AttachmentDescription(VkFormat format, VkAttachmentLoadOp loadOp, VkAttachmentLoadOp stencilLoadOp, VkImageLayout initialLayout,
                          VkAttachmentStoreOp storeOp, VkAttachmentStoreOp stencilStoreOp, VkImageLayout finalLayout,
                          VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkAttachmentDescriptionFlags flags = 0)
        : format(format), loadOp(loadOp), stencilLoadOp(stencilLoadOp), initialLayout(initialLayout)
        , storeOp(storeOp), stencilStoreOp(stencilStoreOp), finalLayout(finalLayout) , samples(samples), flags(flags) {}

    VkAttachmentDescriptionFlags flags = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED;
};
static_assert(sizeof(AttachmentDescription) == sizeof(VkAttachmentDescription), "sizeof(AttachmentDescription) != sizeof(VkAttachmentDescription)!");

struct SubpassDescription
{
    SubpassDescription() = default;

    explicit SubpassDescription(const Span<VkAttachmentReference> &colorAttachments, const VkAttachmentReference &depthStencilAttachment = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED},
                                const Span<VkAttachmentReference> &inputAttachments = {}, const Span<uint32_t> &preserveAttachments = {}, VkSubpassDescriptionFlags flags = 0)
        : colorAttachments(colorAttachments.cbegin(), colorAttachments.cend())
        , depthStencilAttachment(depthStencilAttachment), flags(flags)
        , inputAttachments(inputAttachments.cbegin(), inputAttachments.cend())
        , preserveAttachments(preserveAttachments.cbegin(), preserveAttachments.cend()) {}

    SubpassDescription(const Span<VkAttachmentReference> &colorAttachments, const VkAttachmentReference &depthStencilAttachment,
                       const Span<VkAttachmentReference> &resolveAttachments, const Span<VkAttachmentReference> &inputAttachments = {},
                       const Span<uint32_t> &preserveAttachments = {}, VkSubpassDescriptionFlags flags = 0)
        : colorAttachments(colorAttachments.cbegin(), colorAttachments.cend())
        , depthStencilAttachment(depthStencilAttachment)
        , resolveAttachments(resolveAttachments.cbegin(), resolveAttachments.cend())
        , inputAttachments(inputAttachments.cbegin(), inputAttachments.cend())
        , preserveAttachments(preserveAttachments.cbegin(), preserveAttachments.cend()), flags(flags) {}

    VkSubpassDescriptionFlags flags = 0;
    VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    std::vector<VkAttachmentReference> inputAttachments;
    std::vector<VkAttachmentReference> colorAttachments;
    std::vector<VkAttachmentReference> resolveAttachments;
    VkAttachmentReference depthStencilAttachment = {VK_ATTACHMENT_UNUSED, VK_IMAGE_LAYOUT_UNDEFINED};
    std::vector<uint32_t> preserveAttachments;

    explicit operator VkSubpassDescription() const;
};

struct SubpassDependency
{
    SubpassDependency() = default;
    SubpassDependency(uint32_t srcSubpass, uint32_t dstSubpass, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                      VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                      VkDependencyFlags dependencyFlags = 0)
        : srcSubpass(srcSubpass), dstSubpass(dstSubpass), srcAccessMask(srcAccessMask), dstAccessMask(dstAccessMask)
        , srcStageMask(srcStageMask), dstStageMask(dstStageMask), dependencyFlags(dependencyFlags) {}

    uint32_t srcSubpass = VK_SUBPASS_EXTERNAL;
    uint32_t dstSubpass = 0;
    VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkAccessFlags srcAccessMask = 0;
    VkAccessFlags dstAccessMask = 0;
    VkDependencyFlags dependencyFlags = 0;
};
static_assert(sizeof(SubpassDependency) == sizeof(VkSubpassDependency), "sizeof(SubpassDependency) != sizeof(VkSubpassDependency)!");

class Device
{
  public:
    Device() = default;
    explicit Device(VkDevice device);

    explicit operator bool() const
    {
        return device_ != VK_NULL_HANDLE;
    }

    explicit operator VkDevice() const
    {
        return device_;
    }

    bool operator==(const Device &other) const
    {
        return device_ == other.device_;
    }

    bool operator!=(const Device &other) const
    {
        return device_ != other.device_;
    }

    void WaitIdle() const;

    Buffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags = 0) const;
    Buffer CreateBufferExt(const void *pNext, VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags = 0) const;
    Buffer CreateConcurrentBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const Span<uint32_t> &queueFamilyIndices, VkBufferCreateFlags flags = 0) const;
    Buffer CreateConcurrentBufferExt(const void *pNext, VkDeviceSize size, VkBufferUsageFlags usage, const Span<uint32_t> &queueFamilyIndices, VkBufferCreateFlags flags = 0) const;

    Image CreateImage(const ImageDescription &imageDescription) const;
    Image CreateImage(VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
                      VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout initialLayout) const;
    Image CreateImageExt(const void *pNext, VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
                         VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, VkImageLayout initialLayout) const;
    Image CreateConcurrentImage(VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
                                VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, const Span<uint32_t> &queueFamilyIndices, VkImageLayout initialLayout) const;
    Image CreateConcurrentImageExt(const void *pNext, VkImageCreateFlags flags, VkImageType type, VkFormat format, const VkExtent3D &extent, uint32_t mipLevels, uint32_t arrayLayers,
                                   VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, const Span<uint32_t> &queueFamilyIndices, VkImageLayout initialLayout) const;

    Image CreateLinearlyTiledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage,
                                   VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED, VkImageCreateFlags flags = 0) const;
    Image CreateLinearlyTiledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage,
                                      VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentLinearlyTiledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, const Span<uint32_t> &queueFamilyIndices,
                                             VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentLinearlyTiledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, const Span<uint32_t> &queueFamilyIndices,
                                                VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED, VkImageCreateFlags flags = 0) const;

    Image CreateImage1D(uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateImage1DExt(const void *pNext, uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentImage1D(uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                  const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentImage1DExt(const void *pNext, uint32_t extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                     const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;

    Image CreateImage2D(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateImage2DExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentImage2D(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                  const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentImage2DExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                     const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;

    Image CreateMultiSampledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                  uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateMultiSampledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                     uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentMultiSampledImage(const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                            const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentMultiSampledImageExt(const void *pNext, const VkExtent2D &extent, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples,
                                               const Span<uint32_t> &queueFamilyIndices, uint32_t arrayLayers = 1, VkImageCreateFlags flags = 0) const;

    Image CreateImage3D(const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, VkImageCreateFlags flags = 0) const;
    Image CreateImage3DExt(const void *pNext, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentImage3D(const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                  const Span<uint32_t> &queueFamilyIndices, VkImageCreateFlags flags = 0) const;
    Image CreateConcurrentImage3DExt(const void *pNext, const VkExtent3D &extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels,
                                     const Span<uint32_t> &queueFamilyIndices, VkImageCreateFlags flags = 0) const;

    Sampler CreateSampler(const SamplerDescription &samplerDescription) const;

    DeviceMemory AllocateMemory(VkDeviceSize allocationSize, uint32_t memoryTypeIndex) const;
    DeviceMemory AllocateMemoryExt(const void *pNext, VkDeviceSize allocationSize, uint32_t memoryTypeIndex) const;

    Queue GetQueue(uint32_t queueFamilyIndex = 0, uint32_t queueIndex = 0) const;

    CommandPool CreateCommandPool(uint32_t queueFamilyIndex = 0, VkCommandPoolCreateFlags flags = 0) const;
    CommandPool CreateCommandPoolExt(const void *pNext, uint32_t queueFamilyIndex = 0, VkCommandPoolCreateFlags flags = 0) const;

    Swapchain CreateSwapchain(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                              VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                              VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                              uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;
    Swapchain CreateSwapchainExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                 VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                 VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                                 uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;
    Swapchain CreateConcurrentSwapchain(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                        VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, const Span<uint32_t> &queueFamilyIndices,
                                        VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                                        uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;
    Swapchain CreateConcurrentSwapchainExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                           VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, const Span<uint32_t> &queueFamilyIndices,
                                           VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                           VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                                           uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;

    ShaderModule CreateShaderModule(const Span<uint32_t> &code) const;
    ShaderModule CreateShaderModuleExt(const void *pNext, const Span<uint32_t> &code) const;
    ShaderModule CreateShaderModule(const Span<char> &code) const;
    ShaderModule CreateShaderModuleExt(const void *pNext, const Span<char> &code) const;

    PipelineCache CreatePipelineCache(size_t initialDataSize = 0, const void* pInitialData = nullptr) const;
    PipelineCache CreatePipelineCacheExt(const void *pNext, size_t initialDataSize = 0, const void* pInitialData = nullptr) const;

    Pipeline CreateComputePipeline(const Pipeline::ShaderStage &stage, const PipelineLayout &layout, const PipelineCache &pipelineCache = {},
                                   VkPipelineCreateFlags flags = 0, const Pipeline &basePipeline = {}) const;
    Pipeline CreateComputePipelineExt(const void *pNext, const Pipeline::ShaderStage &stage, const PipelineLayout &layout,
                                      const PipelineCache &pipelineCache = {}, VkPipelineCreateFlags flags = 0, const Pipeline &basePipeline = {}) const;

    VkResult WaitForFences(const Span2<Fence> &fences, uint64_t timeoutInNanoSeconds = UINT64_MAX, bool waitAll = true) const;
    void ResetFences(const Span2<Fence> &fences) const;

    DescriptorSetLayout CreateDescriptorSetLayout(const Span<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags = 0) const;
    DescriptorSetLayout CreateDescriptorSetLayoutExt(const void *pNext, const Span<VkDescriptorSetLayoutBinding> &bindings, VkDescriptorSetLayoutCreateFlags flags = 0) const;

    PipelineLayout CreatePipelineLayout(const Span2<DescriptorSetLayout> &setLayouts, const Span<VkPushConstantRange> &pushConstantRanges = {},
                                        VkPipelineLayoutCreateFlags flags = 0) const;
    PipelineLayout CreatePipelineLayoutExt(const void *pNext, const Span2<DescriptorSetLayout> &setLayouts, const Span<VkPushConstantRange> &pushConstantRanges = {},
                                           VkPipelineLayoutCreateFlags flags = 0) const;

    DescriptorPool CreateDescriptorPool(uint32_t maxSets, const Span<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags = 0) const;
    DescriptorPool CreateDescriptorPoolExt(const void *pNext, uint32_t maxSets, const Span<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags = 0) const;

    void UpdateDescriptorSet(const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement, VkDescriptorType descriptorType,
                             const Span<DescriptorBufferInfo> &bufferInfo) const;

    void UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                VkDescriptorType descriptorType, const Span<DescriptorBufferInfo> &bufferInfo) const;

    void UpdateDescriptorSet(const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                             VkDescriptorType descriptorType, const Span<DescriptorImageInfo> &imageInfo) const;

    void UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                VkDescriptorType descriptorType, const Span<DescriptorImageInfo> &imageInfo) const;

    void UpdateDescriptorSet(const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                             VkDescriptorType descriptorType, const Span2<BufferView> &bufferViews) const;

    void UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement,
                                VkDescriptorType descriptorType, const Span2<BufferView> &bufferViews) const;

    void UpdateDescriptorSet(const DescriptorSet &srcSet, uint32_t srcBinding, uint32_t srcStartingArrayElement,
                             const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement, uint32_t descriptorCount = 1) const;

    void UpdateDescriptorSetExt(const void *pNext, const DescriptorSet &srcSet, uint32_t srcBinding, uint32_t srcStartingArrayElement,
                                const DescriptorSet &dstSet, uint32_t dstBinding, uint32_t dstStartingArrayElement, uint32_t descriptorCount = 1) const;

    RenderPass CreateRenderPass(const Span<AttachmentDescription> &attachments, const Span<SubpassDescription> &subpasses,
                                const Span<SubpassDependency> &dependencies, VkRenderPassCreateFlags flags = 0) const;
    RenderPass CreateRenderPassExt(const void *pNext, const Span<AttachmentDescription> &attachments, const Span<SubpassDescription> &subpasses,
                                   const Span<SubpassDependency> &dependencies = {}, VkRenderPassCreateFlags flags = 0) const;

    Framebuffer CreateFramebuffer(const RenderPass &renderPass, uint32_t width, uint32_t height, const Span2<ImageView> &attachments = {},
                                  uint32_t layers = 1, VkFramebufferCreateFlags flags = 0) const;
    Framebuffer CreateFramebufferExt(const void *pNext, const RenderPass &renderPass, uint32_t width, uint32_t height, const Span2<ImageView> &attachments = {},
                                     uint32_t layers = 1, VkFramebufferCreateFlags flags = 0) const;
    Framebuffer CreateFramebuffer(const RenderPass &renderPass, VkExtent2D extent, const Span2<ImageView> &attachments = {},
                                  uint32_t layers = 1, VkFramebufferCreateFlags flags = 0) const;
    Framebuffer CreateFramebufferExt(const void *pNext, const RenderPass &renderPass, VkExtent2D extent, const Span2<ImageView> &attachments = {},
                                     uint32_t layers = 1, VkFramebufferCreateFlags flags = 0) const;

    Pipeline CreateGraphicsPipeline(const RenderPass &renderPass, uint32_t subpass, const Span<Pipeline::ShaderStage> &stages, const PipelineLayout &layout,
                                    const GraphicsPipelineStateDescription &gfxPipeDesc, const PipelineCache &pipelineCache = PipelineCache(),
                                    VkPipelineCreateFlags flags = 0, const Pipeline &basePipeline = {}) const;
    Pipeline CreateGraphicsPipelineExt(const void *pNext, const RenderPass &renderPass, uint32_t subpass, const Span<Pipeline::ShaderStage> &stages, const PipelineLayout &layout,
                                       const GraphicsPipelineStateDescription &gfxPipeDesc, const PipelineCache &pipelineCache = PipelineCache(),
                                       VkPipelineCreateFlags flags = 0, const Pipeline &basePipeline = {}) const;

    // the pipelineStageFlag is used for the pWaitDstStageMask parameters in the VkSubmitInfo struct
    Semaphore createSemaphore(VkPipelineStageFlags pipelineStageFlag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkSemaphoreCreateFlags flags = 0) const;
    // the pipelineStageFlag is used for the pWaitDstStageMask parameters in the VkSubmitInfo struct
    Semaphore createSemaphoreExt(const void *pNext, VkPipelineStageFlags pipelineStageFlag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkSemaphoreCreateFlags flags = 0) const;

    Fence CreateFence(VkFenceCreateFlags flags = 0) const;
    Fence CreateFenceExt(void *pNext, VkFenceCreateFlags flags = 0) const;

    Event CreateEvent(VkEventCreateFlags flags = 0) const;
    Event CreateEventExt(const void *pNext, VkEventCreateFlags flags = 0) const;

    QueryPool CreateQueryPool(VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics = 0, VkQueryPoolCreateFlags flags = 0) const;
    QueryPool CreateQueryPoolExt(const void *pNext, VkQueryType queryType, uint32_t queryCount, VkQueryPipelineStatisticFlags pipelineStatistics = 0, VkQueryPoolCreateFlags flags = 0) const;

  private:
    Impl::DispatchableObject<VkDevice, vkDestroyDevice> device_;
}; // class Device

struct MappedMemoryRange
{
    friend class DeviceMemory;

    explicit MappedMemoryRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
        : offset(offset), size(size)
    {}

private:
    VkStructureType sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    const void *pNext = nullptr;
    mutable VkDeviceMemory memory;

public:
    VkDeviceSize offset = 0;
    VkDeviceSize size = VK_WHOLE_SIZE;
};
static_assert(sizeof(MappedMemoryRange) == sizeof(VkMappedMemoryRange), "sizeof(MappedMemoryRange) != sizeof(VkMappedMemoryRange)!");

struct MappedMemoryRangeExt
{
    friend class DeviceMemory;

    explicit MappedMemoryRangeExt(const void *pNext = nullptr, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE)
        : pNext(pNext), offset(offset), size(size)
    {}

private:
    VkStructureType sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;

public:
    const void *pNext = nullptr;

private:
    mutable VkDeviceMemory memory;

public:
    VkDeviceSize offset = 0;
    VkDeviceSize size = VK_WHOLE_SIZE;
};
static_assert(sizeof(MappedMemoryRangeExt) == sizeof(VkMappedMemoryRange), "sizeof(MappedMemoryRangeExt) != sizeof(VkMappedMemoryRange)!");

class DeviceMemory
{
public:
    DeviceMemory() = default;
    explicit DeviceMemory(VkDevice device, VkDeviceMemory memory);

    explicit operator bool() const
    {
        return memory_ != VK_NULL_HANDLE;
    }

    explicit operator VkDeviceMemory() const
    {
        return memory_;
    }

    bool operator==(const DeviceMemory &other) const
    {
        return memory_ == other.memory_;
    }

    bool operator!=(const DeviceMemory &other) const
    {
        return memory_ != other.memory_;
    }

    VkDeviceSize GetDeviceMemoryCommitment() const;

    void* Map(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE, VkMemoryMapFlags flags = 0) const;
    void Unmap() const;

    void FlushMappedMemoryRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void FlushMappedMemoryRangeExt(const void *pNext, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void InvalidateMappedMemoryRange(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;
    void InvalidateMappedMemoryRangeExt(const void *pNext, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE) const;

    void FlushMappedMemoryRanges(const Span<MappedMemoryRange> &ranges) const;
    void FlushMappedMemoryRangesExt(const Span<MappedMemoryRangeExt> &ranges) const;
    void InvalidateMappedMemoryRanges(const Span<MappedMemoryRange> &ranges) const;
    void InvalidateMappedMemoryRangesExt(const Span<MappedMemoryRangeExt> &ranges) const;

private:
    Impl::NonDispatchableObject<VkDeviceMemory, VkDevice, vkFreeMemory> memory_;
}; // class DeviceMemory

class Event
{
public:

    Event() = default;
    explicit Event(VkDevice device, VkEvent event);

    explicit operator bool() const
    {
        return event_ != VK_NULL_HANDLE;
    }

    explicit operator VkEvent() const
    {
        return event_;
    }

    bool operator== (const Event &other) const
    {
        return event_ == other.event_;
    }

    bool operator!= (const Event &other) const
    {
        return event_ != other.event_;
    }

    void Set() const;
    void Reset() const;
    VkResult GetStatus() const;

private:

    Impl::NonDispatchableObject<VkEvent, VkDevice, vkDestroyEvent> event_;
}; // class Event

class Fence
{
public:

    Fence() = default;
    explicit Fence(VkDevice device, VkFence fence);

    explicit operator bool() const
    {
        return fence_ != VK_NULL_HANDLE;
    }

    explicit operator VkFence() const
    {
        return fence_;
    }

    bool operator== (const Fence &other) const
    {
        return fence_ == other.fence_;
    }

    bool operator!= (const Fence &other) const
    {
        return fence_ != other.fence_;
    }

    VkResult Wait(uint64_t timeoutInNanoSeconds = UINT64_MAX) const;
    void Reset() const;
    VkResult GetStatus() const;

private:

    Impl::NonDispatchableObject<VkFence, VkDevice, vkDestroyFence> fence_;
}; // class Fence

class Framebuffer
{
public:

    Framebuffer() = default;
    explicit Framebuffer(VkDevice device, VkFramebuffer framebuffer)
        : framebuffer_(device, framebuffer)
    {}

    explicit operator bool() const
    {
        return framebuffer_ != VK_NULL_HANDLE;
    }

    explicit operator VkFramebuffer() const
    {
        return framebuffer_;
    }

    bool operator== (const Framebuffer &other) const
    {
        return framebuffer_ == other.framebuffer_;
    }

    bool operator!= (const Framebuffer &other) const
    {
        return framebuffer_ != other.framebuffer_;
    }

private:

    Impl::NonDispatchableObject<VkFramebuffer, VkDevice, vkDestroyFramebuffer> framebuffer_;
}; // class Framebuffer

class Image
{
  public:
    Image() = default;
    explicit Image(VkDevice device, VkImage image, bool destroyable = true);

    explicit operator bool() const
    {
        return image_ != VK_NULL_HANDLE;
    }

    explicit operator VkImage() const
    {
        return image_;
    }

    bool operator==(const Image &other) const
    {
        return image_ == other.image_;
    }

    bool operator!=(const Image &other) const
    {
        return image_ != other.image_;
    }

    VkSubresourceLayout GetSubresourceLayout(VkImageAspectFlags aspectMask, uint32_t mipLevel = 0, uint32_t arrayLayer = 0) const;

    ImageView CreateImageView(VkImageViewType type, VkFormat format, VkImageAspectFlags aspectMask, uint32_t baseMipLevel = 0,
                              uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0,
                              uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS, VkComponentMapping components = {},
                              VkImageViewCreateFlags flags = 0) const;
    ImageView CreateImageViewExt(const void *pNext, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectMask,
                                 uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0,
                                 uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS, VkComponentMapping components = {},
                                 VkImageViewCreateFlags flags = 0) const;

    VkMemoryRequirements GetMemoryRequirements() const;

    void BindMemory(const DeviceMemory &memory, VkDeviceSize offset) const;

    VkImageMemoryBarrier CreateMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                                             VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t baseMipLevel = 0,
                                             uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0,
                                             uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const;
    VkImageMemoryBarrier CreateMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask,
                                                uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
                                                uint32_t baseArrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const;
    VkImageMemoryBarrier CreateConcurrentMemoryBarrier(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldLayout,
                                                       VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t srcQueueFamilyIndex,
                                                       uint32_t dstQueueFamilyIndex, uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS,
                                                       uint32_t baseArrayLayer = 0, uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const;
    VkImageMemoryBarrier CreateConcurrentMemoryBarrierExt(const void *pNext, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask,
                                                          VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask,
                                                          uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex, uint32_t baseMipLevel = 0,
                                                          uint32_t levelCount = VK_REMAINING_MIP_LEVELS, uint32_t baseArrayLayer = 0,
                                                          uint32_t layerCount = VK_REMAINING_ARRAY_LAYERS) const;

  private:
    Impl::NonDispatchableObject<VkImage, VkDevice, vkDestroyImage> image_;
}; // class Image

class ImageView
{
public:
    ImageView() = default;
    explicit ImageView(VkDevice device, VkImageView view);

    explicit operator bool() const
    {
        return view_ != VK_NULL_HANDLE;
    }

    explicit operator VkImageView() const
    {
        return view_;
    }

    bool operator==(const ImageView &other) const
    {
        return view_ == other.view_;
    }

    bool operator!=(const ImageView &other) const
    {
        return view_ != other.view_;
    }

private:
    Impl::NonDispatchableObject<VkImageView, VkDevice, vkDestroyImageView> view_;
}; // class ImageView

struct MemoryProperties
{
    MemoryProperties() = default;
    ~MemoryProperties() = default;
    explicit MemoryProperties(const VkPhysicalDeviceMemoryProperties &memoryProperties);
    MemoryProperties(const MemoryProperties &other) = default;
    MemoryProperties(MemoryProperties &&other) noexcept = default;
    MemoryProperties& operator=(const MemoryProperties &other) = default;
    MemoryProperties& operator=(MemoryProperties &&other) noexcept = default;

    std::vector<VkMemoryType> types;
    std::vector<VkMemoryHeap> heaps;
};

struct QueueCreateInfo
{
    friend class PhysicalDevice;

    explicit QueueCreateInfo(uint32_t queueFamilyIndex = 0, uint32_t queueCount = 1, float queuePriorities = .5f)
        : queueFamilyIndex(queueFamilyIndex), queuePriorities(queueCount, queuePriorities) {}

    explicit QueueCreateInfo(const void *pNext, uint32_t queueFamilyIndex = 0, uint32_t queueCount = 1, float queuePriorities = .5f)
        : pNext(pNext), queueFamilyIndex(queueFamilyIndex), queuePriorities(queueCount, queuePriorities) {}

    explicit QueueCreateInfo(uint32_t queueFamilyIndex, const Span<float> &queuePriorities)
        : queueFamilyIndex(queueFamilyIndex), queuePriorities(queuePriorities.begin(), queuePriorities.end()) {}

    explicit QueueCreateInfo(const void *pNext, uint32_t queueFamilyIndex, const Span<float> &queuePriorities)
        : pNext(pNext), queueFamilyIndex(queueFamilyIndex), queuePriorities(queuePriorities.begin(), queuePriorities.end()) {}

    const void *pNext = nullptr;
    VkDeviceQueueCreateFlags flags = 0;
    uint32_t queueFamilyIndex = 0;
    std::vector<float> queuePriorities;

    explicit operator VkDeviceQueueCreateInfo() const;
};


class PhysicalDevice
{
  public:
    PhysicalDevice() = default;
    explicit PhysicalDevice(VkPhysicalDevice device);

    explicit operator bool() const
    {
        return device_ != VK_NULL_HANDLE;
    }

    explicit operator VkPhysicalDevice() const
    {
        return device_;
    }

    bool operator==(const PhysicalDevice &other) const
    {
        return device_ == other.device_;
    }

    bool operator!=(const PhysicalDevice &other) const
    {
        return device_ != other.device_;
    }

    std::vector<Extension> EnumerateExtensions() const;
    VkPhysicalDeviceProperties GetProperties() const;
    VkFormatProperties GetFormatProperties(VkFormat format) const;
    std::optional<VkImageFormatProperties> GetImageFormatProperties(VkFormat format, VkImageType type,
                                                                    VkImageTiling tiling, VkImageUsageFlags usage,
                                                                    VkImageCreateFlags flags = 0) const;
    VkPhysicalDeviceFeatures GetFeatures() const;
    MemoryProperties GetMemoryProperties() const;
    std::vector<VkQueueFamilyProperties> GetQueueFamilyProperties() const;
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(const Surface &surface) const;
    std::vector<VkSurfaceFormatKHR> GetSurfaceFormats(const Surface &surface) const;
    std::vector<VkPresentModeKHR> GetSurfacePresentModes(const Surface &surface) const;
    bool GetSurfaceSupport(const Surface &surface, uint32_t queueFamilyIndex) const;
#ifdef VK_USE_PLATFORM_WIN32_KHR
    bool GetWin32PresentationSupport(uint32_t queueFamilyIndex) const;
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
    bool GetXlibPresentationSupport(uint32_t queueFamilyIndex, Display* dpy, VisualID visualID) const;
#endif
    Device CreateDevice(const Span<QueueCreateInfo> &queueCreateInfo = QueueCreateInfo(),
                        const Span<std::string> &enabledExtensions = {},
                        const VkPhysicalDeviceFeatures *enabledFeatures = nullptr) const;
    Device CreateDeviceExt(const void *pNext, const Span<QueueCreateInfo> &queueCreateInfo = QueueCreateInfo(),
                           const Span<std::string> &enabledExtensions = {},
                           const VkPhysicalDeviceFeatures *enabledFeatures = nullptr) const;

  private:
    VkPhysicalDevice device_;
}; // class PhysicalDevice


class Instance
{
  public:
    Instance() = default;
    explicit Instance(VkInstance instance);

    explicit operator bool() const
    {
        return instance_ != VK_NULL_HANDLE;
    }

    explicit operator VkInstance() const
    {
        return instance_;
    }

    bool operator==(const Instance &other) const
    {
        return instance_ == other.instance_;
    }

    bool operator!=(const Instance &other) const
    {
        return instance_ != other.instance_;
    }

    std::vector<PhysicalDevice> EnumeratePhysicalDevices() const;

#ifdef VK_USE_PLATFORM_WIN32_KHR
    Surface CreateWin32Surface(void *hInstance, void *hWnd) const;
    Surface CreateWin32SurfaceExt(const void *pNext, void *hInstance, void *hWnd) const;
#endif

#ifdef VK_USE_PLATFORM_XLIB_KHR
    Surface CreateXlibSurface(Display *dpy, Window window) const;
    Surface CreateXlibSurfaceExt(const void *pNext, Display *dpy, Window window) const;
#endif

    // Creates a debug report callback with an internal call back function
    // Requires the VK_EXT_debug_report extension
    void CreateDebugReportCallback(VkDebugReportFlagsEXT flags = (VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                                                  VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                                  VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                                                  VK_DEBUG_REPORT_INFORMATION_BIT_EXT));
    // Requires the VK_EXT_debug_report extension
    void CreateDebugReportCallback(const VkDebugReportCallbackCreateInfoEXT &createInfo);

  private:

    Impl::DispatchableObject<VkInstance, vkDestroyInstance> instance_;
    std::unique_ptr<void, std::function<void(void*)>> debugReportCallback_ = std::unique_ptr<void, std::function<void(void*)>>(nullptr, [](void*) {});
}; // class Instance

struct ApplicationInfo
{
    ApplicationInfo() = default;
    ApplicationInfo(std::string name, Version version,
                    std::string engineName = {}, Version engineVersion = {},
                    Version apiVersion = {});

    const void *pNext = nullptr;
    std::string applicationName;
    Version applicationVersion;
    std::string engineName;
    Version engineVersion;
    Version apiVersion;

    explicit operator VkApplicationInfo() const;
};

Instance CreateInstance(const Span<std::string> &enabledLayerNames = {},
                        const Span<std::string> &enabledExtensionNames = {},
                        const ApplicationInfo &applicationInfo = {},
                        VkInstanceCreateFlags flags = 0);
Instance CreateInstanceExt(const void *pNext,
                           const Span<std::string> &enabledLayerNames = {},
                           const Span<std::string> &enabledExtensionNames = {},
                           const ApplicationInfo &applicationInfo = {},
                           VkInstanceCreateFlags flags = 0);

class PipelineLayout
{
public:

    PipelineLayout() = default;
    explicit PipelineLayout(VkDevice device, VkPipelineLayout layout)
        : layout_(device, layout)
    {}

    explicit operator bool() const
    {
        return layout_ != VK_NULL_HANDLE;
    }

    explicit operator VkPipelineLayout() const
    {
        return layout_;
    }

    bool operator== (const PipelineLayout &other) const
    {
        return layout_ == other.layout_;
    }

    bool operator!= (const PipelineLayout &other) const
    {
        return layout_ != other.layout_;
    }

private:

    Impl::NonDispatchableObject<VkPipelineLayout, VkDevice, vkDestroyPipelineLayout> layout_;
}; // class PipelineLayout

class QueryPool
{
public:

    QueryPool() = default;
    explicit QueryPool(VkDevice device, VkQueryPool queryPool);

    explicit operator bool() const
    {
        return queryPool_ != VK_NULL_HANDLE;
    }

    explicit operator VkQueryPool() const
    {
        return queryPool_;
    }

    bool operator== (const QueryPool &other) const
    {
        return queryPool_ == other.queryPool_;
    }

    bool operator!= (const QueryPool &other) const
    {
        return queryPool_ != other.queryPool_;
    }

    VkResult GetResults(uint32_t firstQuery, uint32_t queryCount, void* pData, size_t dataSize, VkDeviceSize stride, VkQueryResultFlags flags = 0) const;
    VkResult GetResults(uint32_t firstQuery, uint32_t queryCount, uint32_t *pData, uint32_t elementCountPerResult = 1, VkQueryResultFlags flags = 0) const;
    VkResult GetResults(uint32_t firstQuery, uint32_t queryCount, uint64_t *pData, uint32_t elementCountPerResult = 1, VkQueryResultFlags flags = 0) const;
    template <typename T>
    VkResult GetResults(uint32_t firstQuery, uint32_t queryCount, T *pData, VkQueryResultFlags flags = 0) const
    {
        return GetResults(firstQuery, queryCount, pData, queryCount * sizeof(T), sizeof(T), flags);
    }

private:

    Impl::NonDispatchableObject<VkQueryPool, VkDevice, vkDestroyQueryPool> queryPool_;
}; // class QueryPool

class Queue
{
public:
    Queue() = default;
    explicit Queue(VkQueue queue)
        : queue_(queue) {}

    explicit operator bool() const
    {
        return queue_ != VK_NULL_HANDLE;
    }

    explicit operator VkQueue() const
    {
        return queue_;
    }

    bool operator== (const Queue &other) const
    {
        return queue_ == other.queue_;
    }

    bool operator!= (const Queue &other) const
    {
        return queue_ != other.queue_;
    }

    void WaitIdle() const;

    void Submit(const Span<CommandBuffer> &commandBuffers, const Span2<Semaphore> &waitSemaphores = {},
                const Span2<Semaphore> &signalSemaphores = {}, const Fence &signalFence = {}) const;
    void SubmitExt(const void *pNext, const Span<CommandBuffer> &commandBuffers, const Span2<Semaphore> &waitSemaphores = {},
                   const Span2<Semaphore> &signalSemaphores = {}, const Fence &signalFence = {}) const;

    VkResult Present(const Swapchain &swapchain, uint32_t imageIndex, const Span2<Semaphore> &waitSemaphores = {}) const;
    VkResult PresentExt(const void *pNext, const Swapchain &swapchain, uint32_t imageIndex, const Span2<Semaphore> &waitSemaphores = {}) const;
    std::vector<VkResult> Present(const Span2<Swapchain> &swapchains, const Span<uint32_t> &imageIndices, const Span2<Semaphore> &waitSemaphores = {}) const;
    std::vector<VkResult> PresentExt(const void *pNext, const Span2<Swapchain> &swapchains, const Span<uint32_t> &imageIndices, const Span2<Semaphore> &waitSemaphores = {}) const;

private:
    VkQueue queue_ = VK_NULL_HANDLE;
}; // class Queue
static_assert(sizeof(Queue) == sizeof(VkQueue), "sizeof(Queue) != sizeof(VkQueue)!");

class RenderPass
{
public:

    RenderPass() = default;
    explicit RenderPass(VkDevice device, VkRenderPass renderPass);

    explicit operator bool() const
    {
        return renderPass_ != VK_NULL_HANDLE;
    }

    explicit operator VkRenderPass() const
    {
        return renderPass_;
    }

    bool operator== (const RenderPass &other) const
    {
        return renderPass_ == other.renderPass_;
    }

    bool operator!= (const RenderPass &other) const
    {
        return renderPass_ != other.renderPass_;
    }

    VkExtent2D GetRenderAreaGranularity() const;

private:

    Impl::NonDispatchableObject<VkRenderPass, VkDevice, vkDestroyRenderPass> renderPass_;
}; // class RenderPass

class Sampler
{
public:
    Sampler() = default;
    explicit Sampler(VkDevice device, VkSampler sampler)
        : sampler_(device, sampler)
    {}

    explicit operator bool() const
    {
        return sampler_ != VK_NULL_HANDLE;
    }

    explicit operator VkSampler() const
    {
        return sampler_;
    }

    bool operator==(const Sampler &other) const
    {
        return sampler_ == other.sampler_;
    }

    bool operator!=(const Sampler &other) const
    {
        return sampler_ != other.sampler_;
    }

private:
    Impl::NonDispatchableObject<VkSampler, VkDevice, vkDestroySampler> sampler_;

}; // class Sampler

class Semaphore
{
public:

    Semaphore() = default;
    explicit Semaphore(VkDevice device, VkSemaphore semaphore, VkPipelineStageFlags pipelineStageFlag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    explicit operator bool() const
    {
        return semaphore_ != VK_NULL_HANDLE;
    }

    explicit operator VkSemaphore() const
    {
        return semaphore_;
    }

    bool operator== (const Semaphore &other) const
    {
        return semaphore_ == other.semaphore_;
    }

    bool operator!= (const Semaphore &other) const
    {
        return semaphore_ != other.semaphore_;
    }

    void SetPipeLineStageFlag(VkPipelineStageFlags pipelineStageFlag);
    VkPipelineStageFlags GetPipeLineStageFlag() const;

private:

    Impl::NonDispatchableObject<VkSemaphore, VkDevice, vkDestroySemaphore> semaphore_;
    VkPipelineStageFlags pipelineStageFlag_ = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
}; // class Semaphore

class Surface
{
public:
    Surface() = default;
    explicit Surface(VkInstance instance, VkSurfaceKHR surface)
        : surface_(instance, surface) {}

    explicit operator bool() const
    {
        return VkSurfaceKHR(surface_) != VK_NULL_HANDLE;
    }

    explicit operator VkSurfaceKHR() const
    {
        return VkSurfaceKHR(surface_);
    }

    bool operator==(const Surface &other) const
    {
        return VkSurfaceKHR(surface_) == VkSurfaceKHR(other.surface_);
    }

    bool operator!=(const Surface &other) const
    {
        return VkSurfaceKHR(surface_) != VkSurfaceKHR(other.surface_);
    }

private:
    Impl::NonDispatchableObject<VkSurfaceKHR, VkInstance, vkDestroySurfaceKHR> surface_;
}; // class Surface

class Swapchain
{
public:

    Swapchain() = default;
    explicit Swapchain(VkDevice device, VkSwapchainKHR swapchain)
        : swapchain_(device, swapchain) {}

    explicit operator bool() const
    {
        return swapchain_ != VK_NULL_HANDLE;
    }

    explicit operator VkSwapchainKHR() const
    {
        return swapchain_;
    }

    bool operator== (const Swapchain &other) const
    {
        return swapchain_ == other.swapchain_;
    }

    bool operator!= (const Swapchain &other) const
    {
        return swapchain_ != other.swapchain_;
    }

    std::vector<Image> GetImages() const;
    std::tuple<uint32_t, VkResult> AcquireNextImage(const Fence &fence) const;
    std::tuple<uint32_t, VkResult> AcquireNextImage(const Semaphore &semaphore, const Fence &fence = {}) const;
    std::tuple<uint32_t, VkResult> AcquireNextImage(uint64_t timeoutInNanoSeconds, const Fence &fence) const;
    std::tuple<uint32_t, VkResult> AcquireNextImage(uint64_t timeoutInNanoSeconds, const Semaphore &semaphore, const Fence &fence = {}) const;

    Swapchain Recreate(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                       VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                       VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                       uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;
    Swapchain RecreateExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                          VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                          VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                          uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;
    Swapchain RecreateConcurrent(const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                 VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, const Span<uint32_t> &queueFamilyIndices,
                                 VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                 VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                                 uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;
    Swapchain RecreateConcurrentExt(const void *pNext, const Surface &surface, uint32_t minImageCount, const VkSurfaceFormatKHR &format, const VkExtent2D &extent,
                                    VkImageUsageFlags imageUsage, VkPresentModeKHR presentMode, const Span<uint32_t> &queueFamilyIndices,
                                    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                                    VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VkBool32 clipped = VK_TRUE,
                                    uint32_t imageArrayLayers = 1, VkSwapchainCreateFlagsKHR flags = 0) const;

private:

    Impl::NonDispatchableObject<VkSwapchainKHR, VkDevice, vkDestroySwapchainKHR> swapchain_;
}; // class Swapchain

} // namespace vkw