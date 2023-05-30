set(HEADERS
    slot_map.h
    )

add_library(slot_map INTERFACE)
target_include_directories(slot_map INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
target_compile_features(slot_map INTERFACE cxx_std_17)

