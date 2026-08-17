#-------------------------------------------------
#
# Project created by QtCreator 2018-03-18T19:44:19
#
#-------------------------------------------------

QT += core gui widgets concurrent

equals(QT_MAJOR_VERSION, 5):lessThan (QT_MINOR_VERSION, 6): QT += webkitwidgets
equals(QT_MAJOR_VERSION, 5):greaterThan (QT_MINOR_VERSION, 5): QT += webenginewidgets

DEFINES += USE_VULKAN
DEFINES += USE_OPENGL

contains(DEFINES, USE_OPENGL){
  QT +=  opengl
  equals(QT_MAJOR_VERSION, 6): QT += openglwidgets
}

CONFIG += c++17

include(mathkit/mathkit.pri)
include(foundationkit/foundationkit.pri)
include(symmetrykit/symmetrykit.pri)
include(simulationkit/simulationkit.pri)
include(renderkit/renderkit.pri)
include(iraspakit/iraspakit.pri)
include(iraspagui/iraspagui.pri)

ICON = datafiles/iraspa.icns

TRANSLATIONS = i18n/iraspa_en.ts \
               i18n/iraspa_nl.ts \
               i18n/iraspa_de.ts \
               i18n/iraspa_ar.ts \
               i18n/iraspa_fa.ts \
               i18n/iraspa_ja.ts \
               i18n/iraspa_hi.ts \
               i18n/iraspa_zh_Hans.ts \
               i18n/iraspa_es.ts \
               i18n/iraspa_pt.ts \
               i18n/iraspa_ko.ts \
               i18n/iraspa_ru.ts

CONFIG += lrelease embed_translations

macx|unix:!macx {
  !exists($$shell_path($$[QT_HOST_BINS]/lrelease)): \
    error("lrelease not found in $$[QT_HOST_BINS]. Install Qt linguist tools (e.g. brew install qttools).")
}

OTHER_FILES = $$TRANSLATIONS

macx{
  TARGET = iRASPA
}
win32{
  TARGET = iraspa
}
unix:!macx{
  TARGET = iraspa
}

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS


macx{
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15

  # Homebrew on Apple Silicon uses /opt/homebrew; Intel Macs use /usr/local.
  HOMEBREW_PREFIX = /usr/local
  exists(/opt/homebrew/include) {
    HOMEBREW_PREFIX = /opt/homebrew
  }

  # Qt 6 qyieldcpu.h uses __yield(); arm_acle.h must be visible on Apple Silicon.
  equals(QMAKE_HOST.arch, arm64) {
    QMAKE_CXXFLAGS += -include arm_acle.h
  }
  # Qt mkspecs treat implicit declarations as errors on recent Xcode toolchains.
  QMAKE_CXXFLAGS += -Wno-error=implicit-function-declaration
  INCLUDEPATH += $$HOMEBREW_PREFIX/include

  contains(DEFINES,USE_VULKAN){
    DEFINES += VK_USE_PLATFORM_MACOS_MVK VK_USE_PLATFORM_METAL_EXT
    LIBS += -L$$HOMEBREW_PREFIX/lib -lMoltenVK
    LIBS += -framework Cocoa -framework QuartzCore -framework Metal -framework IOKit -framework Foundation -framework IOSurface
    MOLTENVK_DYLIB = $$HOMEBREW_PREFIX/lib/libMoltenVK.dylib
    VULKAN_DATA.path = Contents/Frameworks
    VULKAN_DATA.files = $$MOLTENVK_DYLIB
    QMAKE_BUNDLE_DATA += VULKAN_DATA
    QMAKE_RPATHDIR += @executable_path/../Frameworks
  }

  QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wno-gnu-anonymous-struct
  exists($$HOMEBREW_PREFIX/bin/python3-config) {
    INCLUDEPATH += $$system($$HOMEBREW_PREFIX/bin/python3-config --include | sed -e 's:-I::g')
  }
  QMAKE_LFLAGS += -framework OpenCL -framework Accelerate
  LIBS += -L$$HOMEBREW_PREFIX/lib -lx264 -lswscale -lavutil -lavformat -lavcodec -llzma -lz
  exists($$HOMEBREW_PREFIX/bin/python3-config) {
    LIBS += $$system($$HOMEBREW_PREFIX/bin/python3-config --embed --ldflags --libs 2>/dev/null)
  }
}

win32{
  DEFINES += _CRT_SECURE_NO_WARNINGS
  contains(DEFINES,USE_VULKAN){
    DEFINES += VK_USE_PLATFORM_WIN32_KHR
    isEmpty(VULKAN_SDK) {
      VULKAN_SDK = $$(VULKAN_SDK)
    }
    isEmpty(VULKAN_SDK) {
      VULKAN_SDK = "C:/VulkanSDK/1.2.141.2"
    }
    VULKAN_SDK_PATH = $$VULKAN_SDK
    VULKAN_DYLIB = $${VULKAN_SDK_PATH}/Lib/vulkan-1.lib
    LIBS += $$VULKAN_DYLIB
    INCLUDEPATH += $${VULKAN_SDK_PATH}/Include
  }
  contains(DEFINES,USE_OPENGL){
    CONFIG(debug, debug|release){
      LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/OpenCL.lib"
    } else {
      LIBS += "C:/vcpkg/installed/x64-windows-static/lib/OpenCL.lib"
    }
  }

  QMAKE_CXXFLAGS_DEBUG += /F 4194304 /Zi /Od /MDd
  QMAKE_LFLAGS_DEBUG   += /STACK:4194304  /INCREMENTAL /DEBUG /MDd

  QMAKE_CXXFLAGS_RELEASE += /F 4194304 /Zi /MD /Zi /O2 /Ob1 /D NDEBUG
  QMAKE_LFLAGS_RELEASE   += /STACK:4194304  /DEBUG /INCREMENTAL

  INCLUDEPATH += "C:/vcpkg/installed/x64-windows-static/include"
  INCLUDEPATH += "C:/vcpkg/installed/x64-windows/include/python3.11/"


  CONFIG(debug, debug|release){
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avutil.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avfilter.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avformat.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avdevice.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avcodec.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/swresample.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/swscale.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/libx264.lib"
    LIBS += "C:/vcpkg/installed/x64-windows/debug/lib/libx265.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/vpx.lib"
    LIBS += "C:/vcpkg/installed/x64-windows/lib/python311.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/lzma.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/zlibd.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/bz2d.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/OpenCL.lib"
    LIBS += strmiids.lib mfplat.lib mfuuid.lib secur32.lib ws2_32.lib bcrypt.lib kernel32.lib cfgmgr32.lib user32.lib ole32.lib advapi32.lib msvcrtd.lib
} else {
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avutil.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avfilter.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avformat.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avdevice.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avcodec.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/swresample.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/swscale.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/libx264.lib"
    LIBS += "C:/vcpkg/installed/x64-windows/lib/libx265.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/vpx.lib"
    LIBS += "C:/vcpkg/installed/x64-windows/lib/python311.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/lzma.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/zlib.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/bz2.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/OpenCL.lib"

    LIBS += strmiids.lib mfplat.lib mfuuid.lib secur32.lib ws2_32.lib bcrypt.lib kernel32.lib cfgmgr32.lib user32.lib ole32.lib advapi32.lib
  }
}

unix:!macx{
  contains(DEFINES,USE_VULKAN){
    DEFINES += VK_USE_PLATFORM_XCB_KHR
    QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -I/usr/include $$system(python3-config --includes) -I/usr/include/ffmpeg
    LIBS += -lvulkan -lxcb -lOpenCL -lavcodec -lavutil -lavformat -lswscale $$system(python3-config --embed > /dev/null 2>&1  && python3-config --embed --libs  || python3-config --libs)  -llzma -lz
  }
  contains(DEFINES,USE_OPENGL){
    QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wl,--stack,4194304 -I/usr/include $$system(python3-config --includes) -I/usr/include/ffmpeg
    LIBS += -lOpenCL -lavcodec -lavutil -lavformat -lswscale $$system(python3-config --embed > /dev/null 2>&1  && python3-config --embed --libs  || python3-config --libs)  -llzma -lz
  }
}


RESOURCES += \
    iraspagui/iraspa-resources.qrc \
    renderkit/opengl/ribbonshaders.qrc
