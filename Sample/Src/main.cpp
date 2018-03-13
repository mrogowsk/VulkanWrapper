#include "VulkanWrapper.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <tuple>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "../external/glm/glm.hpp"
#include "../external/glm/gtc/matrix_transform.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../external/tiny_obj_loader.h"

#include "Timer.h"
#include "Window.h"

class VulkanTutorial
{
public:

    struct UniformBufferObject
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription()
        {
            return {0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
        {
            return {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0,
                    1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(pos),
                    2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(pos) + sizeof(color)};
        }
    };

    struct Buffer
    {
        vkw::Buffer buffer;
        VkDeviceSize size;
        VkDeviceSize offset;
    };

    VulkanTutorial() = default;

    void Init(const VkExtent2D &extent, HINSTANCE hInstance, HWND hWnd);
    void CreateResources();
    void CreateGfxPipeline();
    void CreateCommandBuffers();
    void CreateSemaphores();
    void Update();
    void Draw();
    void WaitDevice();

private:

    vkw::Instance instance_;
    vkw::Surface surface_;
    VkSurfaceFormatKHR surfaceFormat_;

    vkw::PhysicalDevice physDevice_;
    vkw::MemoryProperties memProps_;
    vkw::Device device_;

    vkw::Queue gfxQueue_;

    vkw::Swapchain swapchain_;
    uint32_t swapchainImageCount_ = 0;
    std::vector<vkw::Image> swapchainImages_;
    VkExtent2D extent_ = {0,0};
    std::vector<vkw::ImageView> swapchainImageViews_;
    std::vector<vkw::Framebuffer> framebuffers_;

    vkw::RenderPass renderPass_;
    vkw::DescriptorSetLayout descSetLayout_;
    vkw::PipelineLayout pipelineLayout_;
    vkw::Pipeline gfxPipeline_;

    vkw::CommandPool cmdPool_;

    vkw::Buffer objBuffer_;
    vkw::DescriptorBufferInfo objBufferDesc_;
    VkDeviceSize indexBufferOffset_ = 0;
    uint32_t indexCount_ = 0;
    vkw::Image texImage_;
    vkw::ImageView texImageView_;
    vkw::Sampler sampler_;
    vkw::Image depthImage_;
    VkFormat depthFormat_;
    vkw::ImageView depthImageView_;
    vkw::DeviceMemory deviceMemory_;

    vkw::Buffer uniformBuffer_;
    vkw::DescriptorBufferInfo uniformBufferDesc_;
    vkw::DeviceMemory stgMemory_;

    vkw::DescriptorPool descPool_;
    vkw::DescriptorSet descSet_;

    std::vector<vkw::CommandBuffer> cmdBuffers_;

    std::vector<vkw::Semaphore> imageAvailableSemaphore_;
    std::vector<vkw::Semaphore> renderFinishedSemaphore_;
    uint32_t frameIdx = 0;
};

std::tuple<vkw::PhysicalDevice, uint32_t, VkSurfaceCapabilitiesKHR,
    std::vector<VkSurfaceFormatKHR>, std::vector<VkPresentModeKHR>, bool> GetSuitableDeviceAndQueue(const vkw::Span<vkw::PhysicalDevice> &physDevices,
                                                                                                    const vkw::Surface &surface)
{
    for (const auto &phDev : physDevices)
    {
        auto deviceProps = phDev.GetProperties();
        auto deviceFeatures = phDev.GetFeatures();
        auto deviceExtensions = phDev.EnumerateExtensions();
        auto deviceSurfaceCaps = phDev.GetSurfaceCapabilities(surface);
        auto deviceSurfaceFormats = phDev.GetSurfaceFormats(surface);
        auto devicePresentModes = phDev.GetSurfacePresentModes(surface);
        if ((deviceProps.deviceType & (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU | VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)) != 0
            && deviceFeatures.geometryShader != 0 && deviceFeatures.samplerAnisotropy != 0
            && !deviceSurfaceFormats.empty() && !devicePresentModes.empty()
            && std::find_if(deviceExtensions.begin(), deviceExtensions.end(), [](const vkw::Extension &e) -> bool {return e.name.compare(VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;}) != deviceExtensions.end())
        {
            auto queueProps = phDev.GetQueueFamilyProperties();
            uint32_t idx = 0;
            for (const auto &qp : queueProps)
            {
                if ((qp.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0
                    && phDev.GetSurfaceSupport(surface, idx))
                {
                    return {phDev, idx, deviceSurfaceCaps, deviceSurfaceFormats, devicePresentModes, true};
                }
                ++idx;
            }
        }
    }

    return {};
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const vkw::Span<VkSurfaceFormatKHR> &formats)
{
    for (const auto &f : formats)
    {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return f;
        }
    }

    return formats[0];
}

VkFormat chooseDepthFormat(const vkw::PhysicalDevice &physDevice)
{
    const VkFormat formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};

    for (const auto& f : formats)
    {
        auto ret = physDevice.GetFormatProperties(f);
        if (ret.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            return f;
        }
    }

    return VK_FORMAT_UNDEFINED;
}

bool hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkPresentModeKHR chooseSwapPresentMode(const vkw::Span<VkPresentModeKHR> &presentModes)
{
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : presentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
        else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        {
            bestMode = availablePresentMode;
        }
    }

    return bestMode;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, const VkExtent2D &wndExtent)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
    {
        return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = {wndExtent.width, wndExtent.height};

    actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

static std::vector<char> readFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

std::tuple<std::vector<VulkanTutorial::Vertex>, std::vector<uint32_t>> loadObj(const std::string &fileName)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str()))
    {
        throw std::runtime_error(err);
    }

    std::vector<VulkanTutorial::Vertex> vertices;
    std::vector<uint32_t> indices;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            VulkanTutorial::Vertex vertex = {};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = {1.0f, 1.0f, 1.0f};

            vertices.push_back(vertex);
            indices.push_back(static_cast<uint32_t>(indices.size()));
        }
    }

    return {vertices, indices};
}

std::tuple<std::vector<char>, VkExtent2D> loadImage(const std::string &fileName)
{
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(fileName.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    size_t imageSize = texWidth * texHeight * 4;
    VkExtent2D extent {static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight)};
    std::vector<char> image(imageSize);
    memcpy(image.data(), pixels, imageSize);
    stbi_image_free(pixels);

    return {image, extent};
}

uint32_t findMemoryType(const vkw::MemoryProperties &memProps, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < memProps.types.size(); i++)
    {
        if ((typeFilter & (1 << i)) && (memProps.types[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

int main()
{
    VulkanTutorial vkT;
    VkExtent2D wndExtent = {1920, 1080};
    Window wnd;
    wnd.create("Vulkan Test", wndExtent.width, wndExtent.height, 100, 35);

    vkT.Init(wndExtent, wnd.getInstance(), wnd.getWindow());

    vkT.CreateResources();

    vkT.CreateGfxPipeline();

    vkT.CreateCommandBuffers();

    vkT.CreateSemaphores();

    wnd.open();
    MSG msg = {};
    Timer timer, timer2;

    for (;;)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) && msg.message != WM_QUIT)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
        {
            break;
        }

        vkT.Update();

        vkT.Draw();

        timer.tick();
        if (timer.getTime() > 1.0f)
        {
            std::cout << timer.getFps() << "\n";
            timer.reset();
        }
    }

    vkT.WaitDevice();

    return 0;
}

void VulkanTutorial::WaitDevice()
{
    device_.WaitIdle();
}

void VulkanTutorial::Init(const VkExtent2D &extent, HINSTANCE hInstance, HWND hWnd)
{
    std::vector<std::string> layers, extensions;
    extensions.push_back("VK_KHR_win32_surface");
    extensions.push_back("VK_KHR_surface");
#if defined _DEBUG || !defined(NDEBUG)
    layers.push_back("VK_LAYER_LUNARG_standard_validation");
    extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

    instance_ = vkw::CreateInstance(layers, extensions, vkw::ApplicationInfo("Vulkan Test App", vkw::Version(1), "", vkw::Version(1), vkw::Version(1)));
#if defined _DEBUG || !defined(NDEBUG)
    instance_.CreateDebugReportCallback();
#endif

    surface_ = instance_.CreateWin32Surface(hInstance, hWnd);

    auto [physDevice, gfxQueueIdx, surfaceCaps, surfaceFormats, presentModes, ret] = GetSuitableDeviceAndQueue(instance_.EnumeratePhysicalDevices(), surface_);
    if (!ret) { throw(std::runtime_error("failed to find a suitable GPU!")); }

    physDevice_ = physDevice;
    memProps_ = physDevice_.GetMemoryProperties();
    surfaceFormat_ = chooseSwapSurfaceFormat(surfaceFormats);
    auto presentMode = chooseSwapPresentMode(presentModes);
    extent_ = chooseSwapExtent(surfaceCaps, extent);
    swapchainImageCount_ = surfaceCaps.minImageCount + 1 <= surfaceCaps.maxImageCount ? surfaceCaps.minImageCount + 1 : surfaceCaps.maxImageCount;

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    device_ = physDevice_.CreateDevice(vkw::QueueCreateInfo(gfxQueueIdx, 1, 1.0f), {VK_KHR_SWAPCHAIN_EXTENSION_NAME}, &deviceFeatures);

    swapchain_ = device_.CreateSwapchain(surface_, swapchainImageCount_, surfaceFormat_, extent_,
                                         VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, presentMode, surfaceCaps.currentTransform);
    swapchainImages_ = swapchain_.GetImages();
    for (const auto &i : swapchainImages_)
    {
        swapchainImageViews_.emplace_back(i.CreateImageView(VK_IMAGE_VIEW_TYPE_2D, surfaceFormat_.format, VK_IMAGE_ASPECT_COLOR_BIT));
    }

    gfxQueue_ = device_.GetQueue(gfxQueueIdx);
    cmdPool_ = device_.CreateCommandPool(gfxQueueIdx);
}

void VulkanTutorial::CreateResources()
{
    auto [vertexBuffer, indexBuffer] = loadObj("../Assets/chalet.obj");

    auto [image, imageExtent] = loadImage("../Assets/chalet.jpg");
    auto imageBufferSize = image.size();

    auto vertexBufferSize = sizeof(vertexBuffer[0]) * vertexBuffer.size();
    auto indexBufferSize = sizeof(indexBuffer[0]) * indexBuffer.size();
    indexBufferOffset_ = vertexBufferSize;
    indexCount_ = static_cast<uint32_t>(indexBuffer.size());
    objBufferDesc_.range = vertexBufferSize + indexBufferSize;

    objBuffer_ = device_.CreateBuffer(objBufferDesc_.range, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    objBufferDesc_.buffer = &objBuffer_;
    texImage_ = device_.CreateImage2D(imageExtent, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1);
    depthFormat_ = chooseDepthFormat(physDevice_);
    depthImage_ = device_.CreateImage2D(extent_, depthFormat_, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1);
    auto stgBuffer = device_.CreateBuffer(objBufferDesc_.range + imageBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto objBufferMemReq = objBuffer_.GetMemoryRequirements();
    auto texImageMemReq = texImage_.GetMemoryRequirements();
    auto depthImageMemReq = depthImage_.GetMemoryRequirements();
    auto stgBufferMemReq = stgBuffer.GetMemoryRequirements();
    VkDeviceSize depthImageOffset = texImageMemReq.size;
    if (depthImageOffset % depthImageMemReq.alignment != 0)
    {
        depthImageOffset = depthImageOffset + depthImageMemReq.alignment - depthImageOffset % depthImageMemReq.alignment;
    }

    deviceMemory_ = device_.AllocateMemory(depthImageOffset + depthImageMemReq.size + objBufferMemReq.size,
                                           findMemoryType(memProps_, objBufferMemReq.memoryTypeBits & texImageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
    auto stgMemory = device_.AllocateMemory(stgBufferMemReq.size, findMemoryType(memProps_, stgBufferMemReq.memoryTypeBits,
                                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

    texImage_.BindMemory(deviceMemory_, 0);
    texImageView_ = texImage_.CreateImageView(VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
    depthImage_.BindMemory(deviceMemory_, depthImageOffset);
    depthImageView_ = depthImage_.CreateImageView(VK_IMAGE_VIEW_TYPE_2D, depthFormat_, VK_IMAGE_ASPECT_DEPTH_BIT);
    objBufferDesc_.offset = depthImageOffset + depthImageMemReq.size;
    objBuffer_.BindMemory(deviceMemory_, objBufferDesc_.offset);
    stgBuffer.BindMemory(stgMemory, 0);

    void *data = stgMemory.Map();
    memcpy(data, image.data(), imageBufferSize);
    memcpy(static_cast<char*>(data) + imageBufferSize, vertexBuffer.data(), vertexBufferSize);
    memcpy(static_cast<char*>(data) + imageBufferSize + vertexBufferSize, indexBuffer.data(), indexBufferSize);
    stgMemory.Unmap();

    VkBufferImageCopy region = {};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {imageExtent.width, imageExtent.height, 1};

    auto copyResourcesCmdBuffer = cmdPool_.AllocateCommandBuffer();
    copyResourcesCmdBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    copyResourcesCmdBuffer.CopyBuffer(stgBuffer, objBuffer_, objBufferDesc_.range, imageBufferSize);

    copyResourcesCmdBuffer.PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                           texImage_.CreateMemoryBarrier(0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                                                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT));
    copyResourcesCmdBuffer.CopyBufferToImage(stgBuffer, texImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, region);
    copyResourcesCmdBuffer.PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
                                           {texImage_.CreateMemoryBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                                                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT),
                                            depthImage_.CreateMemoryBarrier(0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                                                                            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                                                                            VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencilComponent(depthFormat_) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0))});
    copyResourcesCmdBuffer.End();
    gfxQueue_.Submit(copyResourcesCmdBuffer);
    gfxQueue_.WaitIdle();
    cmdPool_.FreeCommandBuffers(copyResourcesCmdBuffer);
}

void VulkanTutorial::CreateGfxPipeline()
{
    uniformBufferDesc_.range = sizeof(UniformBufferObject);
    uniformBuffer_ = device_.CreateBuffer(uniformBufferDesc_.range, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    uniformBufferDesc_.buffer = &uniformBuffer_;
    auto uniformBufferMemReq = uniformBuffer_.GetMemoryRequirements();
    stgMemory_ = device_.AllocateMemory(uniformBufferMemReq.size, findMemoryType(memProps_, uniformBufferMemReq.memoryTypeBits,
                                                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
    uniformBufferDesc_.offset = 0;
    uniformBuffer_.BindMemory(stgMemory_, uniformBufferDesc_.offset);

    vkw::SamplerDescription samplerDesc;
    samplerDesc.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerDesc.anisotropyEnable = VK_TRUE;
    samplerDesc.maxAnisotropy = 16.0f;
    sampler_ = device_.CreateSampler(samplerDesc);

    auto vertexShader = device_.CreateShaderModule(readFile("../Src/Shaders/vert.spv"));
    auto fragmentShader = device_.CreateShaderModule(readFile("../Src/Shaders/frag.spv"));

    vkw::Pipeline::ShaderStage vertexShaderStage(vertexShader, "main", VK_SHADER_STAGE_VERTEX_BIT);
    vkw::Pipeline::ShaderStage fragmentShaderStage(fragmentShader, "main", VK_SHADER_STAGE_FRAGMENT_BIT);

    vkw::GraphicsPipelineStateDescription gfxPipelineDesc;
    gfxPipelineDesc.vertexInputState = vkw::Pipeline::VertexInputState(Vertex::getBindingDescription(), Vertex::getAttributeDescriptions());
    gfxPipelineDesc.inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vkw::Pipeline::ViewportState viewportState(extent_);
    gfxPipelineDesc.viewportState = &viewportState;
    gfxPipelineDesc.rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkw::Pipeline::MultisampleState multisamleState;
    gfxPipelineDesc.multisampleState = &multisamleState;
    vkw::Pipeline::DepthStencilState depthStencilState;
    gfxPipelineDesc.depthStencilState = &depthStencilState;
    vkw::Pipeline::ColorBlendState colorBlendState;
    colorBlendState.attachments.resize(1, {});
    colorBlendState.attachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    gfxPipelineDesc.colorBlendState = &colorBlendState;

    descPool_ = device_.CreateDescriptorPool(1, {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}});
    descSetLayout_ = device_.CreateDescriptorSetLayout({{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT},
                                                        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}});
    descSet_ = descPool_.AllocateDescriptorSet(descSetLayout_);

    device_.UpdateDescriptorSet(descSet_, 0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBufferDesc_);
    device_.UpdateDescriptorSet(descSet_, 1, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, vkw::DescriptorImageInfo(sampler_, texImageView_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

    pipelineLayout_ = device_.CreatePipelineLayout(descSetLayout_);
    renderPass_ = device_.CreateRenderPass({vkw::AttachmentDescription(surfaceFormat_.format, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED,
                                                                       VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR),
                                            vkw::AttachmentDescription(depthFormat_, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_IMAGE_LAYOUT_UNDEFINED,
                                                                       VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)},
                                           vkw::SubpassDescription(VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                                                   VkAttachmentReference{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}),
                                           vkw::SubpassDependency(VK_SUBPASS_EXTERNAL, 0, 0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                                                                     VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));

    gfxPipeline_ = device_.CreateGraphicsPipeline(renderPass_, 0, {vertexShaderStage, fragmentShaderStage}, pipelineLayout_, gfxPipelineDesc);

    for (const auto &imageView : swapchainImageViews_)
    {
        framebuffers_.push_back(device_.CreateFramebuffer(renderPass_, extent_, {&imageView, &depthImageView_}));
    }
}

void VulkanTutorial::CreateCommandBuffers()
{
    cmdBuffers_ = cmdPool_.AllocateCommandBuffers(static_cast<uint32_t>(framebuffers_.size()));
    size_t cmdBufferIndex = 0;
    for (const auto &cb : cmdBuffers_)
    {
        cb.Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
        cb.BeginRenderPass(renderPass_, framebuffers_[cmdBufferIndex], extent_, {VkClearValue{{0.0f, 0.0f, 0.0f, 0.0f}}, VkClearValue{{1.0f, 0}}});
        cb.BindGraphicsPipeline(gfxPipeline_);
        cb.BindVertexBuffers(objBuffer_, 0);
        cb.BindIndexBuffer(objBuffer_, indexBufferOffset_, VK_INDEX_TYPE_UINT32);
        cb.BindGraphicsDescriptorSets(pipelineLayout_, descSet_);
        cb.DrawIndexed(indexCount_);
        cb.EndRenderPass();
        cb.End();
        ++cmdBufferIndex;
    }
}

void VulkanTutorial::CreateSemaphores()
{
    for (const auto &f : framebuffers_)
    {
        imageAvailableSemaphore_.emplace_back(device_.createSemaphore(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));
        renderFinishedSemaphore_.emplace_back(device_.createSemaphore());
    }
}

void VulkanTutorial::Update()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    UniformBufferObject ubo = {};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * 0.5f * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(extent_.width) / extent_.height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    // use push constants instead!
    void *data = stgMemory_.Map();
    memcpy(data, &ubo, sizeof(ubo));
    stgMemory_.Unmap();
}

void VulkanTutorial::Draw()
{
    auto semaphoreId = frameIdx % swapchainImageCount_;
    auto[imageIndex, res] = swapchain_.AcquireNextImage(imageAvailableSemaphore_[semaphoreId]);

    gfxQueue_.Submit(cmdBuffers_[imageIndex], imageAvailableSemaphore_[semaphoreId], renderFinishedSemaphore_[semaphoreId]);

    res = gfxQueue_.Present(swapchain_, imageIndex, renderFinishedSemaphore_[semaphoreId]);

    ++frameIdx;
}