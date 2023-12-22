if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/EXAGE-${PROJECT_VERSION}"
      CACHE PATH ""
  )
endif()

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package EXAGE)

install(
    DIRECTORY
    include/
    "${PROJECT_BINARY_DIR}/export/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT EXAGE_Development
)

install(
    TARGETS EXAGE_EXAGE ImGui bc7enc_rdo
    EXPORT EXAGETargets
    RUNTIME #
    COMPONENT EXAGE_Runtime
    LIBRARY #
    COMPONENT EXAGE_Runtime
    NAMELINK_COMPONENT EXAGE_Development
    ARCHIVE #
    COMPONENT EXAGE_Development
    INCLUDES #
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

# Allow package maintainers to freely override the path for the configs
set(
    EXAGE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE PATH "CMake package config location relative to the install prefix"
)
mark_as_advanced(EXAGE_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${EXAGE_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT EXAGE_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${EXAGE_INSTALL_CMAKEDIR}"
    COMPONENT EXAGE_Development
)

install(
    EXPORT EXAGETargets
    NAMESPACE EXAGE::
    DESTINATION "${EXAGE_INSTALL_CMAKEDIR}"
    COMPONENT EXAGE_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
