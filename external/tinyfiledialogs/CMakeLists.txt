set(HEADERS
    tinyfiledialogs.h
    )

set(SOURCES
    tinyfiledialogs.cpp
    )

add_library(tinyfiledialogs STATIC ${HEADERS} ${SOURCES})
target_include_directories(tinyfiledialogs INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
target_compile_features(tinyfiledialogs INTERFACE cxx_std_17)