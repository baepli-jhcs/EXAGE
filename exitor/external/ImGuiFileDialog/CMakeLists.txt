cmake_minimum_required(VERSION 3.14)

project(ImGuiFileDialog VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(ImGuiFileDialog STATIC
    ImGuiFileDialog.cpp
    ImGuiFileDialog.h
    ImGuiFileDialogConfig.h
)

target_link_libraries(ImGuiFileDialog PUBLIC
    ImGui
)

target_include_directories(ImGuiFileDialog PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR})