add_rules("mode.debug", "mode.release", "mode.releasedbg")

add_requires("debugbreak", "entt v3.11.1", "fmt", "glfw", "glm", "magic_enum", "unordered_dense v3.0.0", "vk-bootstrap", "vulkan-headers")
add_requires("vcpkg::alpaca", {alias = "alpaca"})
add_requires("imgui v1.89.3-docking", {alias = "imgui", configs = {shared = true}})
add_requires("vcpkg::tl-expected", {alias = "tl_expected"})
add_requires("vcpkg::vulkan-memory-allocator-hpp", {alias = "vma-hpp"})

set_project("EXAGE")
set_version("0.1.0")
set_languages("c++20")

target("EXAGE")
    set_kind("static")

    add_files("src/**.cpp")
    add_headerfiles("src/**.h", {public = false})
    add_headerfiles("include/exage/**.h")
    add_includedirs("include", {public = true})

    add_packages("alpaca", "debugbreak", "entt", "fmt", "glfw", "glm", "imgui", "magic_enum", "tl_expected", "unordered_dense", "vk-bootstrap", "vulkan-headers", "vma-hpp", {public = true})

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

    set_policy("build.optimization.lto", true)

includes("exitor/xmake.lua")
includes("tests/xmake.lua")
