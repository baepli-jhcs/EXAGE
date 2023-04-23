set_project("EXitor")
set_version("0.1.0")
set_languages("c++20")

target("EXitor")
    set_kind("binary")
    add_deps("EXAGE")
    add_headerfiles("src/**.h")
    add_files("src/**.cpp")
    add_links("EXAGE")
    set_policy("build.optimization.lto", true)

    set_rundir("$(projectdir)")
