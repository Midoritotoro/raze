include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(package raze)

install(
    TARGETS raze 
    EXPORT razeTargets
    DESTINATION "${CMAKE_INSTALL_LIBDIR}")

set(
    "RAZE_INSTALL_CMAKEDIR" "${CMAKE_INSTALL_LIBDIR}/cmake/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)

set_property(CACHE "RAZE_INSTALL_CMAKEDIR" PROPERTY TYPE PATH)
mark_as_advanced("RAZE_INSTALL_CMAKEDIR")

install(
    EXPORT razeTargets
    FILE razeTargets.cmake
    DESTINATION lib/cmake/${package}
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${package}Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake"
  INSTALL_DESTINATION "lib/cmake/${package}"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)

  write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake"
        VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION}"
        COMPATIBILITY AnyNewerVersion)

install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/${package}ConfigVersion.cmake
      DESTINATION lib/cmake/${package}
  )

export(
    EXPORT razeTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/${package}Config.cmake"
)

if(MSVC)
    set(pdb_file "")
    set(pdb_file_destination "")

    if(BUILD_SHARED_LIBS)
        set(pdb_file "$<TARGET_PDB_FILE:raze>")
        set(pdb_file_destination "${CMAKE_INSTALL_BINDIR}")
    else()
        set(pdb_file "$<TARGET_FILE_DIR:raze>/$<TARGET_FILE_PREFIX:raze>$<TARGET_FILE_BASE_NAME:raze>.pdb")
        set(pdb_file_destination "${CMAKE_INSTALL_LIBDIR}")
    endif()

    install(FILES "${pdb_file}"
        COMPONENT ${CMAKE_INSTALL_LIBDIR}
        CONFIGURATIONS Debug RelWithDebInfo
        DESTINATION "${pdb_file_destination}"
        OPTIONAL)
endif()

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()