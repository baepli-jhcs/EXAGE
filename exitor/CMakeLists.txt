cmake_minimum_required(VERSION 3.14)

project(EXitor LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----
if (PROJECT_IS_TOP_LEVEL)
    find_package(EXAGE REQUIRED)
endif ()

add_subdirectory(external)

add_executable(EXitor
        src/Editor.cpp
        src/main.cpp
        src/Dialogs/FileDialog.cpp
        src/Dialogs/FolderDialog.cpp
        src/Filesystem/VFS.cpp
        src/LevelEditor/LevelEditor.cpp
        src/LevelEditor/Panels/ComponentEditor.cpp
        src/LevelEditor/Panels/ComponentList.cpp
        src/LevelEditor/Panels/ContentBrowser.cpp
        src/LevelEditor/Panels/Hierarchy.cpp
        src/LevelEditor/Panels/LevelPanel.cpp
        src/ProjectSelector/ProjectSelector.cpp
)

target_include_directories(EXitor PRIVATE src)

target_link_libraries(
        EXitor PRIVATE
        EXAGE::EXAGE
        ImGui
)
target_compile_features(EXitor PRIVATE cxx_std_20)

set_property(TARGET EXitor PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../)