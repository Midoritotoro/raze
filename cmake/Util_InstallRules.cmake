include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(package raze)

set(
    RAZE_INSTALL_CMAKEDIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE PATH
    "CMake package config location relative to the install prefix"
)

mark_as_advanced(RAZE_INSTALL_CMAKEDIR)

install(
    TARGETS raze
    EXPORT razeTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    PATTERN "*.inl"
)

install(
    EXPORT razeTargets
    FILE razeTargets.cmake
    NAMESPACE raze::
    DESTINATION "${RAZE_INSTALL_CMAKEDIR}"
)

configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/${package}Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake"
    INSTALL_DESTINATION "${RAZE_INSTALL_CMAKEDIR}"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${RAZE_INSTALL_CMAKEDIR}"
)

export(
    EXPORT razeTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/razeTargets.cmake"
    NAMESPACE raze::
)

if(PROJECT_IS_TOP_LEVEL)
    include(CPack)
endif()
