cmake_minimum_required(VERSION 3.14)

project(
    EXAGE_PROJECT_NAME
    VERSION 0.1.0
    DESCRIPTION "An exage project"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(EXAGE_PATH)

add_library(
    ${PROJECT_NAME}
    SHARED
    src/DefaultScript.cpp
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    exage
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)