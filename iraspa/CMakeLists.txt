cmake_minimum_required(VERSION 3.17)

if (WIN32)
    project(iRASPA LANGUAGES CXX)
elseif(UNIX)
    project(iRASPA)
endif()

# Enable & Force C++17 Standard
SET ( CMAKE_CXX_STANDARD                17 )
SET ( CMAKE_CXX_STANDARD_REQUIRED       ON )

# Disable GNU Standard
# i.e. use -std=c++17 instead of the default -std=gnu++17
SET ( CMAKE_CXX_EXTENSIONS              OFF )

SET ( CMAKE_CXX_FLAGS "-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic" )
SET ( CMAKE_CXX_FLAGS_DEBUG "-O0 -g" )


set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)



#======================= INCLUSION OF Qt =======================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH /Users/dubbelda/Qt/6.0.1/clang_64/)
find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6Gui REQUIRED)
find_package(Qt6OpenGL REQUIRED)
find_package(Qt6Concurrent REQUIRED)
find_package(Qt6OpenGLWidgets REQUIRED)
find_package(PythonLibs REQUIRED)
find_package(OpenCL REQUIRED)

include_directories(${PYTHON_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)

include_directories(iraspagui)
include_directories(iraspakit)
include_directories(spglib)
include_directories(spglib)
include_directories(mathkit)
include_directories(foundationkit)
include_directories(symmetrykit)
include_directories(simulationkit)
include_directories(renderkit)

file(GLOB_RECURSE SOURCES
    "${CMAKE_SOURCE_DIR}/iraspagui/*.ui"
    "${CMAKE_SOURCE_DIR}/iraspagui/*.qrc"
    "${CMAKE_SOURCE_DIR}/iraspagui/*.h"
    "${CMAKE_SOURCE_DIR}/iraspagui/*.cpp"
    "${CMAKE_SOURCE_DIR}/iraspakit/*.h"
    "${CMAKE_SOURCE_DIR}/iraspakit/*.cpp"
    "${CMAKE_SOURCE_DIR}/spglib/*.h"
    "${CMAKE_SOURCE_DIR}/spglib/*.c"
    "${CMAKE_SOURCE_DIR}/mathkit/*.h"
    "${CMAKE_SOURCE_DIR}/mathkit/*.cpp"
    "${CMAKE_SOURCE_DIR}/foundationkit/*.h"
    "${CMAKE_SOURCE_DIR}/foundationkit/*.cpp"
    "${CMAKE_SOURCE_DIR}/symmetrykit/*.h"
    "${CMAKE_SOURCE_DIR}/symmetrykit/*.cpp"
    "${CMAKE_SOURCE_DIR}/simulationkit/*.h"
    "${CMAKE_SOURCE_DIR}/simulationkit/*.cpp"
    "${CMAKE_SOURCE_DIR}/renderkit/*.h"
    "${CMAKE_SOURCE_DIR}/renderkit/*.cpp"
)

#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${CMAKE_SOURCE_DIR}/iraspagui)

# Add the executable
if (WIN32) 
    add_executable(iRASPA WIN32 ${SOURCES})
elseif(UNIX)
    add_executable(iRASPA ${SOURCES})
endif()

# Add the target includes for iRASPA 
target_include_directories(iRASPA PRIVATE ${CMAKE_SOURCE_DIR}/iraspagui)

#===================== LINKING LIBRARIES =======================#
target_link_libraries(iRASPA Qt6::Widgets)
target_link_libraries(iRASPA Qt6::Gui)
target_link_libraries(iRASPA Qt6::OpenGL)
target_link_libraries(iRASPA Qt6::Concurrent)
target_link_libraries(iRASPA Qt6::OpenGLWidgets)
target_link_libraries(iRASPA ${OpenCL_LIBRARIES})
target_link_libraries(iRASPA ${PYTHON_LIBRARIES})
target_link_libraries(iRASPA -lx264)
target_link_libraries(iRASPA -lswscale)
target_link_libraries(iRASPA -lavutil)
target_link_libraries(iRASPA -lavformat)
target_link_libraries(iRASPA -lavcodec)
target_link_libraries(iRASPA -llzma)
target_link_libraries(iRASPA -lz)
target_link_libraries(iRASPA "-framework Accelerate")
