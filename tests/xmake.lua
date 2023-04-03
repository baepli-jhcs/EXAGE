add_requires("catch2")

target("EXAGE_test")
    set_kind("binary")
    add_deps("EXAGE")
    add_files("src/**.cpp")
    add_links("EXAGE")
    add_packages("catch2")
