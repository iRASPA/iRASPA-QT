include_guard(GLOBAL)

find_package(PkgConfig QUIET)

# Exposes a third-party library as the imported target iRASPA::<name>.
#
# MODULES   pkg-config module names, tried first.
# LIBRARIES library names for the find_library() fallback, which is what
#           Windows needs because vcpkg does not ship .pc files for everything.
# HEADER    header used to locate the include directory of the fallback.
function(iraspa_external_library name)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "" "HEADER" "MODULES;LIBRARIES")

  if(TARGET iRASPA::${name})
    return()
  endif()

  if(PKG_CONFIG_FOUND AND ARG_MODULES)
    pkg_check_modules(IRASPA_PC_${name} QUIET IMPORTED_TARGET GLOBAL ${ARG_MODULES})
    if(IRASPA_PC_${name}_FOUND)
      add_library(iRASPA::${name} ALIAS PkgConfig::IRASPA_PC_${name})
      return()
    endif()
  endif()

  add_library(iraspa_external_${name} INTERFACE)
  add_library(iRASPA::${name} ALIAS iraspa_external_${name})

  foreach(library IN LISTS ARG_LIBRARIES)
    find_library(IRASPA_${name}_${library}_LIBRARY NAMES ${library})
    if(NOT IRASPA_${name}_${library}_LIBRARY)
      message(FATAL_ERROR
        "Could not find the '${library}' library that iRASPA needs. Install it, "
        "or point CMAKE_PREFIX_PATH at the directory that contains it.")
    endif()
    target_link_libraries(iraspa_external_${name}
      INTERFACE "${IRASPA_${name}_${library}_LIBRARY}")
  endforeach()

  if(ARG_HEADER)
    find_path(IRASPA_${name}_INCLUDE_DIR NAMES ${ARG_HEADER})
    if(NOT IRASPA_${name}_INCLUDE_DIR)
      message(FATAL_ERROR
        "Could not find '${ARG_HEADER}' that iRASPA needs. Install the "
        "development package, or extend CMAKE_PREFIX_PATH.")
    endif()
    target_include_directories(iraspa_external_${name}
      SYSTEM INTERFACE "${IRASPA_${name}_INCLUDE_DIR}")
  endif()
endfunction()

# A macro rather than a function so that the Qt version variables and the
# imported targets end up in the scope of the top level CMakeLists.
macro(iraspa_find_dependencies)
  set(_iraspa_qt_components Core Gui Widgets Concurrent LinguistTools)
  if(IRASPA_USE_OPENGL)
    list(APPEND _iraspa_qt_components OpenGL)
  endif()

  find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
  if(IRASPA_USE_OPENGL AND QT_VERSION_MAJOR GREATER_EQUAL 6)
    list(APPEND _iraspa_qt_components OpenGLWidgets)
  endif()

  # iraspagui/helpbrowser.h derives from a web view on Qt 5 when this option is
  # on; Qt 6 dropped the module from qtbase and the class is a plain QWidget
  # there regardless. Off by default so the Linux packages do not pull in
  # QtWebEngine for a browser that is already empty on every Qt 6 build.
  set(IRASPA_QT_WEB_COMPONENT "")
  if(IRASPA_USE_WEBENGINE)
    if(QT_VERSION VERSION_LESS 5.6)
      set(IRASPA_QT_WEB_COMPONENT WebKitWidgets)
    elseif(QT_VERSION_MAJOR EQUAL 5)
      set(IRASPA_QT_WEB_COMPONENT WebEngineWidgets)
    endif()
  endif()
  if(IRASPA_QT_WEB_COMPONENT)
    list(APPEND _iraspa_qt_components ${IRASPA_QT_WEB_COMPONENT})
  endif()

  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${_iraspa_qt_components})

  # renderkit/opengl is written against desktop OpenGL 3.3 core. Qt guards
  # QOpenGLFunctions_3_3_Core with '#if !defined(QT_OPENGL_ES_2)', so a qtbase
  # built for OpenGL ES leaves every shader without a base class. Ubuntu ships
  # such a qtbase on arm64 up to 18.04. Caught here because the alternative is
  # a screenful of 'expected class-name' errors that point at the wrong thing.
  if(IRASPA_USE_OPENGL)
    if(QT_VERSION_MAJOR EQUAL 5 AND Qt5Gui_OPENGL_IMPLEMENTATION
       AND NOT Qt5Gui_OPENGL_IMPLEMENTATION STREQUAL "GL")
      set(_iraspa_qt_is_gles TRUE)
    elseif(QT_FEATURE_opengles2)
      set(_iraspa_qt_is_gles TRUE)
    else()
      set(_iraspa_qt_is_gles FALSE)
    endif()
    if(_iraspa_qt_is_gles)
      message(FATAL_ERROR
        "This Qt is built for OpenGL ES, which does not provide the desktop "
        "OpenGL 3.3 core profile that the OpenGL renderer needs. Install a "
        "desktop OpenGL build of Qt, or configure with -DIRASPA_USE_OPENGL=OFF "
        "and -DIRASPA_USE_VULKAN=ON.")
    endif()
  endif()

  find_package(OpenCL REQUIRED)
  find_package(ZLIB REQUIRED)
  find_package(Python3 REQUIRED COMPONENTS Development.Embed)

  iraspa_external_library(ffmpeg
    MODULES libavcodec libavformat libavutil libswscale
    LIBRARIES avcodec avformat avutil swscale
    HEADER libavcodec/avcodec.h
  )
  # moviemaker.cpp asks libavcodec for an H.264 encoder at runtime and never
  # calls x264 itself. The explicit link is only kept on Windows, where the
  # vcpkg FFmpeg port still expects it.
  if(WIN32)
    iraspa_external_library(x264 MODULES x264 LIBRARIES x264 HEADER x264.h)
  endif()
  iraspa_external_library(lzma MODULES liblzma LIBRARIES lzma HEADER lzma.h)

  if(IRASPA_USE_VULKAN)
    iraspa_find_vulkan()
  endif()
endmacro()

# The Vulkan backend talks to MoltenVK directly on macOS (as opposed to going
# through the loader), because the Metal surface is created by hand in
# renderkit/vulkan/makeviewmetal.mm.
macro(iraspa_find_vulkan)
  if(APPLE)
    find_library(IRASPA_MOLTENVK_LIBRARY NAMES MoltenVK)
    find_path(IRASPA_VULKAN_INCLUDE_DIR NAMES vulkan/vulkan.h)
    if(NOT IRASPA_MOLTENVK_LIBRARY OR NOT IRASPA_VULKAN_INCLUDE_DIR)
      message(FATAL_ERROR
        "The Vulkan backend needs MoltenVK. Install it with 'brew install molten-vk' "
        "or configure with -DIRASPA_USE_VULKAN=OFF.")
    endif()

    add_library(iraspa_vulkan INTERFACE)
    add_library(iRASPA::vulkan ALIAS iraspa_vulkan)
    target_link_libraries(iraspa_vulkan INTERFACE
      "${IRASPA_MOLTENVK_LIBRARY}"
      "-framework Cocoa"
      "-framework QuartzCore"
      "-framework Metal"
      "-framework IOKit"
      "-framework IOSurface"
      "-framework Foundation"
    )
    target_include_directories(iraspa_vulkan
      SYSTEM INTERFACE "${IRASPA_VULKAN_INCLUDE_DIR}")
    target_compile_definitions(iraspa_vulkan INTERFACE
      VK_USE_PLATFORM_MACOS_MVK VK_USE_PLATFORM_METAL_EXT)
  else()
    find_package(Vulkan REQUIRED)

    add_library(iraspa_vulkan INTERFACE)
    add_library(iRASPA::vulkan ALIAS iraspa_vulkan)
    target_link_libraries(iraspa_vulkan INTERFACE Vulkan::Vulkan)

    if(WIN32)
      target_compile_definitions(iraspa_vulkan INTERFACE VK_USE_PLATFORM_WIN32_KHR)
    else()
      iraspa_external_library(xcb MODULES xcb LIBRARIES xcb HEADER xcb/xcb.h)
      target_link_libraries(iraspa_vulkan INTERFACE iRASPA::xcb)
      target_compile_definitions(iraspa_vulkan INTERFACE VK_USE_PLATFORM_XCB_KHR)
    endif()
  endif()
endmacro()
