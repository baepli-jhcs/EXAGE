cmake_minimum_required(VERSION 3.10)

project(
    ImGui
    VERSION 1.89.3
    DESCRIPTION "Bloat-free Immediate Mode Graphical User interface for C++ with minimal dependencies"
    LANGUAGES C CXX
)

set(IMGUI_SOURCES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    misc/cpp/imgui_stdlib.cpp
)

add_library(${PROJECT_NAME} SHARED ${IMGUI_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_API=__declspec\(dllexport\))
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES imgui.h imgui_internal.h imstb_rectpack.h imstb_textedit.h imstb_truetype.h misc/cpp/imgui_stdlib.h DESTINATION include)