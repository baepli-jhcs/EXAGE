cmake_minimum_required(VERSION 3.14)

project(EXitor LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----
if (PROJECT_IS_TOP_LEVEL)
  find_package(EXAGE REQUIRED)
endif()

add_subdirectory(external)

add_executable(EXitor src/main.cpp src/Editor.cpp src/Panels/Hierarchy.cpp src/Panels/ComponentList.cpp src/Panels/ComponentEditor.cpp)

target_include_directories(EXitor PRIVATE src)

target_link_libraries(
    EXitor PRIVATE
    EXAGE::EXAGE
    ImGuiFileDialog
)
target_compile_features(EXitor PRIVATE cxx_std_20)

set_property(TARGET EXitor PROPERTY WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../)