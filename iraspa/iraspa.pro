#-------------------------------------------------
#
# Project created by QtCreator 2018-03-18T19:44:19
#
#-------------------------------------------------

QT += core gui widgets concurrent

equals(QT_MAJOR_VERSION, 5):lessThan (QT_MINOR_VERSION, 6): QT += webkitwidgets
equals(QT_MAJOR_VERSION, 5):greaterThan (QT_MINOR_VERSION, 5): QT += webenginewidgets

#DEFINES += USE_VULKAN
#DEFINES += USE_DIRECTX
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
  contains(DEFINES,USE_VULKAN){
    QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.15
    DEFINES += VK_USE_PLATFORM_MACOS_MVK
    # CHANGE HERE TO YOUR SDK PATH:
    VULKAN_SDK_PATH = /usr/local/VulkanSDK/1.2.182.0
    VULKAN_DYLIB = $${VULKAN_SDK_PATH}/macOS/lib/libvulkan.1.dylib
    LIBS += $$VULKAN_DYLIB
    LIBS += -framework Cocoa -framework QuartzCore
    # Copy dylib to app bundle
    VULKAN_DATA.path = Contents/Frameworks
    VULKAN_DATA.files = $$VULKAN_DYLIB
    QMAKE_BUNDLE_DATA += VULKAN_DATA

    INCLUDEPATH += $${VULKAN_SDK_PATH}/macOS/include
    # Fix @rpath
    QMAKE_RPATHDIR += @executable_path/../Frameworks

    #QMAKE_CXXFLAGS += -Wl,--stack,4194304
    QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wno-gnu-anonymous-struct
    INCLUDEPATH += $$system(python-config --include | sed -e 's:-I::g')
    QMAKE_LFLAGS += -framework Python -framework Accelerate
    LIBS += -L/usr/local/lib -lx264 -lswscale -lavutil -lavformat -lavcodec -llzma -lz
  }

  contains(DEFINES,USE_OPENGL) {
    #QMAKE_CXXFLAGS += -Wl,--stack,4194304
    INCLUDEPATH += $$system(python-config --include | sed -e 's:-I::g')
    QMAKE_LFLAGS +=  -framework OpenCL -framework Python -framework Accelerate

    CONFIG(debug, debug|release){
      QMAKE_CXXFLAGS += -g -O0 -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wno-gnu-anonymous-struct  -fsanitize=address
      LIBS += -L/usr/local/lib -lx264 -lswscale -lavutil -lavformat -lavcodec -llzma -lz -fsanitize=address
      }
      else
      {
        QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wno-gnu-anonymous-struct
        LIBS += -L/usr/local/lib -lx264 -lswscale -lavutil -lavformat -lavcodec -llzma -lz
    }
  }
}

win32{
  DEFINES += _CRT_SECURE_NO_WARNINGS
  contains(DEFINES,USE_VULKAN){
    DEFINES += VK_USE_PLATFORM_WIN32_KHR
    # CHANGE HERE TO YOUR SDK PATH:
    VULKAN_SDK_PATH = "C:/VulkanSDK/1.2.141.2"
    VULKAN_DYLIB = $${VULKAN_SDK_PATH}/Lib/vulkan-1.lib
    LIBS += $$VULKAN_DYLIB
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
  INCLUDEPATH += "C:/vcpkg/installed/x64-windows-static/include/python2.7/"


  CONFIG(debug, debug|release){
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avutil.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avfilter.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avformat.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avresample.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avdevice.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/avcodec.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/swresample.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/swscale.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/libx264.lib"
    LIBS += "C:/vcpkg/installed/x64-windows/debug/lib/x265-static.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/vpxd.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/python27_d.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/lzmad.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/zlibd.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/OpenCL.lib"
    LIBS += strmiids.lib mfplat.lib mfuuid.lib secur32.lib ws2_32.lib bcrypt.lib kernel32.lib cfgmgr32.lib user32.lib ole32.lib advapi32.lib msvcrtd.lib
} else {
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avutil.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avfilter.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avformat.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avresample.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avdevice.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/avcodec.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/swresample.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/swscale.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/libx264.lib"
    LIBS += "C:/vcpkg/installed/x64-windows/lib/x265-static.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/vpx.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/python27.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/lzma.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/zlib.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/OpenCL.lib"

    LIBS += strmiids.lib mfplat.lib mfuuid.lib secur32.lib ws2_32.lib bcrypt.lib kernel32.lib cfgmgr32.lib user32.lib ole32.lib advapi32.lib
  }
}

unix:!macx{
  contains(DEFINES,USE_OPENGL){
    QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wl,--stack,4194304 -I/usr/include $$system(python3-config --includes) -I/usr/include/ffmpeg
    LIBS += -lOpenCL -lavcodec -lavutil -lavformat -lswscale $$system(python3-config --embed > /dev/null 2>&1  && python3-config --embed --libs  || python3-config --libs)  -llzma -lz
  }
}


RESOURCES += \
    iraspagui/iraspa-resources.qrc
