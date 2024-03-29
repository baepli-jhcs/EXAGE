cmake_minimum_required(VERSION 3.14)

if (POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif ()

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

# include(cmake/conan.cmake)
include(cmake/CPM.cmake)

# # ---- Conan ----
# conan_cmake_run(CONANFILE conanfile.txt
#                 BASIC_SETUP CMAKE_TARGETS
#                 BUILD missing)

# ---- Declare library ----
add_library(
        EXAGE_EXAGE
        src/Core/Core.cpp
        src/Core/Debug.cpp
        src/Core/Timer.cpp
        src/Filesystem/Directories.cpp
        src/Graphics/GraphicsContext.cpp
        src/Graphics/Shader.cpp
        src/Graphics/Texture.cpp
        src/GUI/ImGui.cpp
        src/GUI/ImGuiPlatform/imgui_impl_glfw.cpp
        src/GUI/ImGuiPlatform/imgui_impl_vulkan.cpp
        src/GUI/RmlUi.cpp
        src/Graphics/Utils/BufferTypes.cpp
        src/Graphics/Utils/QueueCommand.cpp
        src/platform/GLFW/GLFWClipboard.cpp
        src/platform/GLFW/GLFWindow.cpp
        src/platform/Vulkan/VkBootstrap.cpp
        src/platform/Vulkan/VulkanBuffer.cpp
        src/platform/Vulkan/VulkanCommandBuffer.cpp
        src/platform/Vulkan/VulkanContext.cpp
        src/platform/Vulkan/VulkanFence.cpp
        src/platform/Vulkan/VulkanFrameBuffer.cpp
        src/platform/Vulkan/VulkanPipeline.cpp
        src/platform/Vulkan/VulkanQueue.cpp
        src/platform/Vulkan/VulkanResourceManager.cpp
        src/platform/Vulkan/VulkanSampler.cpp
        src/platform/Vulkan/VulkanShader.cpp
        src/platform/Vulkan/VulkanSwapchain.cpp
        src/platform/Vulkan/VulkanTexture.cpp
        src/Projects/Level.cpp
        src/Projects/Project.cpp
        src/Projects/Serialization.cpp
        src/Renderer/Scene/Loader/Converter.cpp
        src/Renderer/Scene/Loader/Loader.cpp
        src/GUI/Fonts.cpp
        src/Scene/Entity.cpp
        src/Scene/Hierarchy.cpp
        src/Scene/Rotation3D.cpp
        src/Scene/Scene.cpp
        src/System/Clipboard.cpp
        src/System/Window.cpp
        src/Graphics/Utils/SlotBuffer.cpp
        src/Graphics/Utils/VirtualAllocator.cpp
)
add_library(EXAGE::EXAGE ALIAS EXAGE_EXAGE)

# Platform defines
if (WIN32)
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_WINDOWS)
elseif (UNIX)
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_LINUX)
elseif (APPLE)
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_MACOS)
endif ()


if (NOT BUILD_SHARED_LIBS)
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_STATIC_DEFINE)
endif ()

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

#target_include_directories(
#        EXAGE_EXAGE SYSTEM
#        PUBLIC
#        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/export>"
#)

target_compile_features(EXAGE_EXAGE PUBLIC cxx_std_20)

find_package(alpaca REQUIRED)
find_package(assimp REQUIRED)
find_package(cereal REQUIRED)
find_package(EnTT REQUIRED)
find_package(fmt REQUIRED)
find_package(freeimage REQUIRED)
find_package(freetype REQUIRED)
find_path(FP16_INCLUDE_DIRS "fp16.h")
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
# find_package(Ktx REQUIRED)
find_package(nlohmann_json REQUIRED)
#find_package(RmlUi REQUIRED)
#find_package(rttr REQUIRED)
find_package(Stb REQUIRED)
find_package(unofficial-shaderc REQUIRED)
find_package(tinyfiledialogs REQUIRED)
find_package(tl-expected REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(unofficial-vulkan-memory-allocator-hpp REQUIRED)
find_package(zstd REQUIRED)

add_subdirectory(external)

CPMAddPackage(
        NAME RmlUi
        GITHUB_REPOSITORY baepli-jhcs/RmlUi
        VERSION 5.1
        GIT_TAG 8889025cf4fb6f279ee39f4aa45c5bb631f31025
)

target_link_libraries(EXAGE_EXAGE PRIVATE
        assimp::assimp
        bc7enc_rdo
        fmt::fmt
        freeimage::FreeImage
        unofficial::shaderc::shaderc
)


target_link_libraries(EXAGE_EXAGE PUBLIC
        alpaca::alpaca
        cereal::cereal
        EnTT::EnTT
        glfw
        glm::glm
        ImGui
        # KTX::ktx
        nlohmann_json::nlohmann_json
        RmlCore
        #        RTTR::Core_Lib
        tl::expected
        tinyfiledialogs::tinyfiledialogs
        Vulkan::Headers
        #        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        #        VulkanMemoryAllocator-Hpp
        unofficial::VulkanMemoryAllocator-Hpp::VulkanMemoryAllocator-Hpp
        $<IF:$<TARGET_EXISTS:zstd::libzstd_shared>,zstd::libzstd_shared,zstd::libzstd_static>
)

target_include_directories(EXAGE_EXAGE PRIVATE ${FP16_INCLUDE_DIRS} ${Stb_INCLUDE_DIR})

target_compile_definitions(EXAGE_EXAGE PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE VULKAN_HPP_NO_EXCEPTIONS)

# If Debug, add EXAGE_DEBUG define, if Release, add EXAGE_RELEASE define
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_DEBUG)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(EXAGE_EXAGE PUBLIC EXAGE_RELEASE)
endif ()

# ---- Editor ----
option(EXAGE_BUILD_EDITOR "Build EXitor" ON)
if (EXAGE_BUILD_EDITOR)
    add_subdirectory(exitor)
endif ()

# ---- Install rules ----

if (NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif ()

# ---- Developer mode ----

if (NOT EXAGE_DEVELOPER_MODE)
    return()
elseif (NOT PROJECT_IS_TOP_LEVEL)
    message(
            AUTHOR_WARNING
            "Developer mode is intended for developers of EXAGE"
    )
endif ()

include(cmake/dev-mode.cmake)
