include(CMakeFindDependencyMacro)

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/razeTargets.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/razeTargets.cmake")
endif()