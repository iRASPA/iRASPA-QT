cmake_minimum_required(VERSION 3.21)

# Must be set before project() so that the compiler checks use the same target.
if(APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
endif()

# The application itself lives in iraspa/, which is a project in its own right
# so that it can be configured directly from the source tarballs the packaging
# scripts produce. This file adds the unit tests on top of it.
project(iRASPA-QT LANGUAGES CXX)

add_subdirectory(iraspa)

if(IRASPA_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
