cmake_minimum_required(VERSION 3.21)

# Must be set before project() so that the compiler checks use the same target.
if(APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
endif()

# bump_version.py rewrites the literal below. It is deliberately not a cache
# entry, so that a bump takes effect in build directories that already exist;
# -DIRASPA_VERSION= still wins, which is how the release workflow stamps the
# packages with the tag it is releasing.
if(NOT DEFINED IRASPA_VERSION)
  set(IRASPA_VERSION "2.3.7")
endif()

project(iRASPA
  VERSION ${IRASPA_VERSION}
  DESCRIPTION "Visualization and editing package for material science"
  HOMEPAGE_URL "https://github.com/iRASPA/iRASPA-QT"
  LANGUAGES CXX
)

if(APPLE)
  enable_language(OBJCXX)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(GNUInstallDirs)
include(IRASPADependencies)
include(IRASPAShaders)

option(IRASPA_USE_OPENGL "Build the OpenGL renderer backend" ON)
option(IRASPA_USE_VULKAN "Build the Vulkan renderer backend" ON)
option(IRASPA_USE_WEBENGINE "Build the Qt 5 help browser against QtWebEngine/QtWebKit" OFF)
option(IRASPA_BUILD_TESTS "Build the unit tests" OFF)
option(IRASPA_SNAP_PACKAGE "Look the bundled data files up in the Snap layout" OFF)
option(IRASPA_BUILD_DEB_PACKAGE "Configure CPack to build a Debian package" OFF)

if(NOT IRASPA_USE_OPENGL AND NOT IRASPA_USE_VULKAN)
  message(FATAL_ERROR
    "At least one of IRASPA_USE_OPENGL and IRASPA_USE_VULKAN has to be enabled.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

iraspa_find_dependencies()

# Warning set and platform quirks shared by every iRASPA target.
add_library(iraspa_compile_options INTERFACE)
add_library(iRASPA::compile_options ALIAS iraspa_compile_options)

if(MSVC)
  target_compile_options(iraspa_compile_options INTERFACE /W3 /bigobj)
  target_compile_definitions(iraspa_compile_options INTERFACE
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
  )
  target_link_options(iraspa_compile_options INTERFACE /STACK:4194304)
else()
  target_compile_options(iraspa_compile_options INTERFACE
    -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wno-gnu-anonymous-struct
  )
endif()

if(APPLE AND CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
  # Qt 6 qyieldcpu.h calls __yield(); arm_acle.h has to be visible everywhere.
  target_compile_options(iraspa_compile_options INTERFACE
    $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-include;arm_acle.h>
  )
endif()

target_compile_definitions(iraspa_compile_options INTERFACE
  QT_DEPRECATED_WARNINGS
  IRASPA_VERSION_STRING="${PROJECT_VERSION}"
  $<$<CONFIG:RelWithDebInfo>:QT_MESSAGELOGCONTEXT>
  $<$<BOOL:${IRASPA_USE_OPENGL}>:USE_OPENGL>
  $<$<BOOL:${IRASPA_USE_VULKAN}>:USE_VULKAN>
  $<$<BOOL:${IRASPA_USE_WEBENGINE}>:USE_WEBENGINE>
  $<$<BOOL:${IRASPA_SNAP_PACKAGE}>:USE_SNAP>
)

add_subdirectory(mathkit)
add_subdirectory(foundationkit)
add_subdirectory(symmetrykit)

if(APPLE)
  add_executable(iraspa MACOSX_BUNDLE)
elseif(WIN32)
  add_executable(iraspa WIN32)
else()
  add_executable(iraspa)
endif()

target_link_libraries(iraspa PRIVATE
  iRASPA::compile_options
  iRASPA::mathkit
  iRASPA::foundationkit
  iRASPA::symmetrykit
  iRASPA::ffmpeg
  iRASPA::lzma
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Concurrent
  OpenCL::OpenCL
  Python3::Python
  ZLIB::ZLIB
)

if(WIN32)
  target_link_libraries(iraspa PRIVATE iRASPA::x264)
endif()

if(IRASPA_QT_WEB_COMPONENT)
  target_link_libraries(iraspa PRIVATE
    Qt${QT_VERSION_MAJOR}::${IRASPA_QT_WEB_COMPONENT})
endif()

if(IRASPA_USE_OPENGL)
  target_link_libraries(iraspa PRIVATE Qt${QT_VERSION_MAJOR}::OpenGL)
  if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    target_link_libraries(iraspa PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
  endif()
endif()

if(IRASPA_USE_VULKAN)
  target_link_libraries(iraspa PRIVATE iRASPA::vulkan)
endif()

# These four modules are compiled straight into the application: they reference
# each other in both directions, so splitting them into libraries would only
# introduce circular dependencies.
add_subdirectory(simulationkit)
add_subdirectory(renderkit)
add_subdirectory(iraspakit)
add_subdirectory(iraspagui)

set(IRASPA_TRANSLATIONS
  i18n/iraspa_en.ts
  i18n/iraspa_nl.ts
  i18n/iraspa_de.ts
  i18n/iraspa_ar.ts
  i18n/iraspa_fa.ts
  i18n/iraspa_ja.ts
  i18n/iraspa_hi.ts
  i18n/iraspa_zh_Hans.ts
  i18n/iraspa_es.ts
  i18n/iraspa_pt.ts
  i18n/iraspa_ko.ts
  i18n/iraspa_ru.ts
)

# The application looks the compiled catalogues up under ":/i18n", which is the
# resource prefix qt_add_translations() uses by default.
if(QT_VERSION_MAJOR GREATER_EQUAL 6)
  qt_add_translations(iraspa TS_FILES ${IRASPA_TRANSLATIONS})
else()
  qt5_add_translation(IRASPA_QM_FILES ${IRASPA_TRANSLATIONS})
  set(translations_resource "${CMAKE_CURRENT_BINARY_DIR}/iraspatranslations.qrc")
  set(entries "")
  foreach(qm_file IN LISTS IRASPA_QM_FILES)
    file(RELATIVE_PATH entry "${CMAKE_CURRENT_BINARY_DIR}" "${qm_file}")
    string(APPEND entries "        <file>${entry}</file>\n")
  endforeach()
  file(WRITE "${translations_resource}.in"
    "<RCC>\n    <qresource prefix=\"/i18n\">\n${entries}    </qresource>\n</RCC>\n")
  configure_file("${translations_resource}.in" "${translations_resource}" COPYONLY)
  qt5_add_resources(IRASPA_QM_RESOURCES "${translations_resource}")
  target_sources(iraspa PRIVATE ${IRASPA_QM_RESOURCES})
endif()

if(APPLE)
  set(IRASPA_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/datafiles/iraspa.icns")
  target_sources(iraspa PRIVATE "${IRASPA_BUNDLE_ICON}")
  set_source_files_properties("${IRASPA_BUNDLE_ICON}" PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources")

  set_target_properties(iraspa PROPERTIES
    OUTPUT_NAME "iRASPA"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
    MACOSX_BUNDLE_BUNDLE_NAME "iRASPA"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.yourcompany.iRASPA"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_ICON_FILE "iraspa.icns"
    MACOSX_BUNDLE_COPYRIGHT "Copyright (C) iRASPA authors"
  )

  if(IRASPA_USE_VULKAN)
    # Ship MoltenVK inside the bundle so that the application also runs on
    # machines without a Vulkan installation.
    target_sources(iraspa PRIVATE "${IRASPA_MOLTENVK_LIBRARY}")
    set_source_files_properties("${IRASPA_MOLTENVK_LIBRARY}" PROPERTIES
      MACOSX_PACKAGE_LOCATION "Frameworks")
    set_target_properties(iraspa PROPERTIES
      INSTALL_RPATH "@executable_path/../Frameworks"
      BUILD_WITH_INSTALL_RPATH TRUE
    )
  endif()
else()
  set_target_properties(iraspa PROPERTIES OUTPUT_NAME "iraspa")
endif()

install(TARGETS iraspa
  BUNDLE  DESTINATION .
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

if(UNIX AND NOT APPLE)
  install(FILES datafiles/iraspa.desktop
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications")
  install(FILES datafiles/iraspa.png
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pixmaps")

  foreach(icon_size IN ITEMS 16 32 64 128 256 512)
    install(FILES "datafiles/iraspa.iconset/icon_${icon_size}x${icon_size}.png"
      DESTINATION
        "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${icon_size}x${icon_size}/apps"
      RENAME iraspa.png)
  endforeach()

  install(FILES
      datafiles/databasecoremof.irspdoc
      datafiles/databasecoremofddec.irspdoc
      datafiles/databaseiza.irspdoc
      datafiles/libraryofstructures.irspdoc
      datafiles/acknowledgedlicenses.pdf
      datafiles/license-gpl.pdf
    DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/iraspa")
endif()

add_subdirectory(packaging)

message(STATUS "iRASPA ${PROJECT_VERSION} configuration")
message(STATUS "  Qt version .......... ${Qt${QT_VERSION_MAJOR}Core_VERSION}")
message(STATUS "  OpenGL backend ...... ${IRASPA_USE_OPENGL}")
message(STATUS "  Vulkan backend ...... ${IRASPA_USE_VULKAN}")
message(STATUS "  Qt WebEngine ........ ${IRASPA_USE_WEBENGINE}")
message(STATUS "  Unit tests .......... ${IRASPA_BUILD_TESTS}")
