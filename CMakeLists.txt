CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

project(VulkanWrapper)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

find_package(Vulkan REQUIRED)
if(LINUX)
    find_package(X11 REQUIRED)
endif()

ADD_LIBRARY(VulkanWrapper Include/VulkanWrapper.h
                          Src/Buffer.cpp
                          Src/BufferView.cpp
                          Src/CommandBuffer.cpp
                          Src/CommandPool.cpp
                          Src/DescriptorPool.cpp
                          Src/Device.cpp
                          Src/DeviceMemory.cpp
                          Src/Error.h
                          Src/Error.cpp
                          Src/Event.cpp
                          Src/Fence.cpp
                          Src/Global.cpp
                          Src/Image.cpp
                          Src/ImageView.cpp
                          Src/Instance.cpp
                          Src/PhysicalDevice.cpp
                          Src/PipelineCache.cpp
                          Src/QueryPool.cpp
                          Src/Queue.cpp
                          Src/RenderPass.cpp
                          Src/Semaphore.cpp
                          Src/Swapchain.cpp)

set_property(TARGET VulkanWrapper PROPERTY POSITION_INDEPENDENT_CODE ON)
target_compile_features(VulkanWrapper PUBLIC cxx_std_17)
if(WIN32)
    target_link_libraries(VulkanWrapper ${Vulkan_LIBRARIES})
endif()
if(LINUX)
    target_link_libraries(VulkanWrapper X11 ${Vulkan_LIBRARIES})
endif()
target_include_directories(VulkanWrapper PUBLIC ${CMAKE_SOURCE_DIR}/Include ${Vulkan_INCLUDE_DIRS})
