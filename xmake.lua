add_rules("mode.debug", "mode.release")

add_requires("fmt", "glfw", "glm", "magic_enum", "unordered_dense v3.0.0", "vk-bootstrap", "volk")

set_project("EXAGE")
set_version("0.1.0")
set_languages("c++20")

target("EXAGE")
    set_kind("shared")


    add_files("src/**.cpp")
    add_headerfiles("include/**.h")
    add_includedirs("include")

    add_files("platform/**.cpp")
    add_headerfiles("platform/**.h")
    add_includedirs("platform")

    -- add_rules("utils.glsl2spv", {outputdir = "shaders/build"})
    -- add_files("shaders/src/**.vert", "shaders/src/**.frag")

    add_packages("fmt", "glfw", "glm", "magic_enum", "unordered_dense", "vk-bootstrap", "volk", {public = true})

    if is_mode("debug") then
        add_defines("EXAGE_DEBUG")
    else
	  add_defines("EXAGE_RELEASE")
    end

    add_defines("EXAGE_EXPORT=__declspec(dllexport)")

includes("tests/xmake.lua")