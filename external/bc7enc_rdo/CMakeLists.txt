set(HEADERS
    bc7enc.h
    rgbcx.h
    )

set(SOURCES
    bc7enc.cpp
    rgbcx.cpp
    )

add_library(bc7enc_rdo STATIC ${HEADERS} ${SOURCES})
target_include_directories(bc7enc_rdo INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
target_compile_features(bc7enc_rdo INTERFACE cxx_std_17)