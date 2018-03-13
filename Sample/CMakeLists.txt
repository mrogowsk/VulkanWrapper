CMAKE_MINIMUM_REQUIRED(VERSION 3.7.2)

project(VulkanWrapperSample)

set(ADDITIONAL_LIBRARIES_DEBUG ${CMAKE_SOURCE_DIR}/../build/Debug/VulkanWrapper.lib)
set(ADDITIONAL_LIBRARIES_RELEASE ${CMAKE_SOURCE_DIR}/../build/Release/VulkanWrapper.lib)

find_package(Vulkan REQUIRED)

ADD_EXECUTABLE(VulkanWrapperSample Src/main.cpp
                                   Src/Timer.cpp
                                   Src/Timer.h
                                   Src/Window.cpp
                                   Src/Window.h)

target_compile_features(VulkanWrapperSample PUBLIC cxx_std_17)
target_link_libraries(VulkanWrapperSample debug ${ADDITIONAL_LIBRARIES_DEBUG} optimized ${ADDITIONAL_LIBRARIES_RELEASE} general ${Vulkan_LIBRARIES})
target_include_directories(VulkanWrapperSample PUBLIC ${CMAKE_SOURCE_DIR}/../Include ${Vulkan_INCLUDE_DIRS})