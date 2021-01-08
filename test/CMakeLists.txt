# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14)

#-------------------------------------------------------------------------------
# PROJECT configuration
#-------------------------------------------------------------------------------
project(${CMAKE_PROJECT_NAME}-test
  LANGUAGES CXX)

#-------------------------------------------------------------------------------
# GoogleTest configuration
#-------------------------------------------------------------------------------
include(FetchContent)
include(GoogleTest)

option(BUILD_GMOCK OFF)
option(INSTALL_GTEST OFF)

# Prevent GoogleTest from overriding our compiler/linker options when building
# with Visual Studio
#
# https://github.com/google/googletest/blob/master/googletest/README.md
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git)

FetchContent_MakeAvailable(googletest)

#-------------------------------------------------------------------------------
# EXECUTABLE configuration
#-------------------------------------------------------------------------------
add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE apptest.cxx canary.cxx memtest.cxx streamtest.cxx)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE ${LIBRARY_NAME}_NO_RUNTIME)

target_link_libraries(${PROJECT_NAME}
  PRIVATE gtest_main ${CMAKE_PROJECT_NAME})

if(BUILD_SHARED_LIBS)
  # XXX: On Windows platform with Visual Studio, the executable and GoogleTest
  # dll are put in different directories and at runtime, the loader cannot find
  # the dll. One solution is to force CMake to build dll's and exe's in the same
  # directory so at runtime the dll's can be found.
  #
  # https://stackoverflow.com/a/47260387
  set_target_properties(${PROJECT_NAME}
    PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  # This should be defined whenever Google Test is built as a shared library.
  #
  # https://github.com/google/googletest/blob/master/googletest/README.md
  target_compile_definitions(${PROJECT_NAME}
    PRIVATE GTEST_LINKED_AS_SHARED_LIBRARY=1)
endif()

gtest_discover_tests(${PROJECT_NAME}
  PROPERTIES TIMEOUT 60)
