add_rules("mode.debug", "mode.release", "mode.releasedbg")

add_requires("debugbreak", "entt v3.11.1", "fmt", "glfw", "glm", "imgui v1.89.3-docking", "magic_enum", "unordered_dense v3.0.0", "vk-bootstrap", "vulkan-headers")
add_requires("vcpkg::tl-expected", {alias = "tl_expected"})
add_requires("vcpkg::vulkan-memory-allocator-hpp", {alias = "vma-hpp"})

set_project("EXAGE")
set_version("0.1.0")
set_languages("c++20")

add_rules("plugin.vsxmake.autoupdate")

target("EXAGE")
    set_kind("static")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h", {public = false})
    add_headerfiles("include/**.h")
    add_includedirs("include", {public = true})

    add_files("platform/**.cpp")
    add_headerfiles("platform/**.h")
    add_includedirs("platform", {public = true})

    add_packages("debugbreak", "entt", "fmt", "glfw", "glm", "imgui", "magic_enum", "tl_expected", "unordered_dense", "vk-bootstrap", "vulkan-headers", "vma-hpp", {public = true})

    if is_mode("debug") or is_mode("releasedbg") then
        add_defines("EXAGE_DEBUG")
    else
	  add_defines("EXAGE_RELEASE")
    end

    if is_plat("windows") then
        add_defines("EXAGE_WINDOWS")
    end

    if is_plat("macosx") then
        add_defines("EXAGE_MACOS")
    end

    if is_plat("linux") then
        add_defines("EXAGE_LINUX")
    end

    if is_plat("android") then
        add_defines("EXAGE_ANDROID")
    end

includes("exitor/xmake.lua")
includes("tests/xmake.lua")