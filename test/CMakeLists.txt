cmake_minimum_required(VERSION 3.14)

project(EXAGETests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(EXAGE REQUIRED)
  enable_testing()
endif()

find_package(Catch2 REQUIRED)
include(Catch)

# ---- Tests ----

add_executable(EXAGE_test source/EXAGE_test.cpp)
target_link_libraries(
    EXAGE_test PRIVATE
    EXAGE::EXAGE
    Catch2::Catch2WithMain
)
target_compile_features(EXAGE_test PRIVATE cxx_std_20)

catch_discover_tests(EXAGE_test)

# ---- End-of-file commands ----

add_folders(Test)
