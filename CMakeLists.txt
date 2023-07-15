cmake_minimum_required(VERSION 3.14)

cmake_policy(SET CMP0091 NEW)

include(cmake/prelude.cmake)

project(
    EXAGE
    VERSION 0.1.0
    DESCRIPTION "An extensible game engine"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

include(cmake/CPM.cmake)

# ---- Declare library ----
add_library(
    EXAGE_EXAGE
    src/Core/Core.cpp
    src/Core/Debug.cpp
    src/Core/Window.cpp
    src/Filesystem/Directories.cpp
    src/Graphics/GraphicsContext.cpp
    src/Graphics/Shader.cpp
    src/Graphics/Texture.cpp
    src/Graphics/HLPD/ImGuiTools.cpp
    src/Graphics/HLPD/ImGuiPlatform/imgui_impl_glfw.cpp
    src/Graphics/HLPD/ImGuiPlatform/imgui_impl_vulkan.cpp
    src/Graphics/Utils/BufferTypes.cpp
    src/Graphics/Utils/QueueCommand.cpp
    src/platform/GLFW/GLFWindow.cpp
    src/platform/Vulkan/VkBootstrap.cpp
    src/platform/Vulkan/VulkanBuffer.cpp
    src/platform/Vulkan/VulkanCommandBuffer.cpp
    src/platform/Vulkan/VulkanContext.cpp
    src/platform/Vulkan/VulkanFrameBuffer.cpp
    src/platform/Vulkan/VulkanPipeline.cpp
    src/platform/Vulkan/VulkanQueue.cpp
    src/platform/Vulkan/VulkanResourceManager.cpp
    src/platform/Vulkan/VulkanSampler.cpp
    src/platform/Vulkan/VulkanShader.cpp
    src/platform/Vulkan/VulkanSwapchain.cpp
    src/platform/Vulkan/VulkanTexture.cpp
    src/Projects/Level.cpp
    src/Projects/Serialization.cpp
    src/Renderer/Renderer.cpp
    src/Renderer/GeometryPass/GeometryRenderer.cpp
    src/Renderer/GeometryPass/MeshSystem.cpp
    src/Renderer/LightingPass/DirectLightingSystem.cpp
    src/Renderer/LightingPass/LightingRenderer.cpp
    src/Renderer/Scene/Camera.cpp
    src/Renderer/Scene/SceneBuffer.cpp
    src/Renderer/Scene/Loader/Converter.cpp
    src/Renderer/Scene/Loader/Loader.cpp
    src/Renderer/ShadowPass/DirectionalShadowSystem.cpp
    src/Renderer/ShadowPass/ShadowRenderer.cpp
    src/Renderer/ShadowPass/PointShadowSystem.cpp
    src/Renderer/Utils/Fonts.cpp
    src/Scene/Entity.cpp
    src/Scene/Hierarchy.cpp
    src/Scene/Rotation3D.cpp
    src/Scene/Scene.cpp
)
add_library(EXAGE::EXAGE ALIAS EXAGE_EXAGE)

# Platform defines
if (WIN32)
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_WINDOWS)
elseif (UNIX)
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_LINUX)
elseif (APPLE)
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_MACOS)
endif()


if(NOT BUILD_SHARED_LIBS)
  target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_STATIC_DEFINE)
endif()

set_target_properties(
    EXAGE_EXAGE PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    EXPORT_NAME EXAGE
    OUTPUT_NAME EXAGE
)

target_include_directories(
    EXAGE_EXAGE ${warning_guard}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_include_directories(
    EXAGE_EXAGE SYSTEM
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
)

target_compile_features(EXAGE_EXAGE PUBLIC cxx_std_20)

CPMAddPackage(NAME assimp GITHUB_REPOSITORY assimp/assimp VERSION 5.2.5)

add_subdirectory(external)

find_package(alpaca REQUIRED)
find_package(cereal REQUIRED)
find_package(EnTT REQUIRED)
find_package(fmt REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Ktx REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Stb REQUIRED)
find_package(shaderc REQUIRED)
find_package(tl-expected REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(vulkan-memory-allocator REQUIRED)
find_package(zstd REQUIRED)

target_link_libraries(EXAGE_EXAGE PUBLIC
    alpaca::alpaca
    assimp
    cereal::cereal
    EnTT::EnTT
    fmt::fmt
    glfw
    glm::glm
    ImGui
    KTX::ktx
    nlohmann_json::nlohmann_json
    shaderc::shaderc
    stb::stb
    slot_map
    tl::expected
    vulkan-headers::vulkan-headers
    vulkan-memory-allocator::vulkan-memory-allocator
    VulkanMemoryAllocator-Hpp
    $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>
)

target_compile_definitions(EXAGE_EXAGE PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE VULKAN_HPP_NO_EXCEPTIONS)

# If Debug, add EXAGE_DEBUG define, if Release, add EXAGE_RELEASE define
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_DEBUG)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_RELEASE)
endif()

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Editor ----
option(EXAGE_BUILD_EDITOR "Build EXitor" ON)
if (EXAGE_BUILD_EDITOR)
    add_subdirectory(exitor)
endif()

# ---- Developer mode ----

if(NOT EXAGE_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of EXAGE"
  )
endif()

include(cmake/dev-mode.cmake)
