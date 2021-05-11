#-------------------------------------------------
#
# Project created by QtCreator 2018-03-18T19:44:19
#
#-------------------------------------------------

QT += core gui widgets opengl concurrent

equals(QT_MAJOR_VERSION, 5):lessThan (QT_MINOR_VERSION, 6): QT += webkitwidgets
equals(QT_MAJOR_VERSION, 5):greaterThan (QT_MINOR_VERSION, 5): QT += webenginewidgets
equals(QT_MAJOR_VERSION, 6): QT += openglwidgets

CONFIG += c++1z

include(spglib/spglib.pri)
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
               i18n/iraspa_es.ts

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
  #QMAKE_CXXFLAGS += -Wl,--stack,4194304
  QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wno-gnu-anonymous-struct
  INCLUDEPATH += $$system(python-config --include | sed -e 's:-I::g')
  QMAKE_LFLAGS += -framework OpenCL -framework Python -framework Accelerate
  LIBS += -L/usr/local/lib -lx264 -lswscale -lavutil -lavformat -lavcodec -llzma -lz
}

win32{
  QMAKE_CXXFLAGS += /F 4194304 /Zi /MD
  QMAKE_LFLAGS   += /STACK:4194304  /INCREMENTAL:NO /DEBUG /MD

  INCLUDEPATH += "C:/vcpkg/installed/x64-windows-static/include"
  INCLUDEPATH += "C:/vcpkg/installed/x64-windows-static/include/python2.7"


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
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/vpxmtd.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/python27_d.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/lzmad.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/zlibd.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/debug/lib/OpenCL.lib"
    LIBS += secur32.lib ws2_32.lib bcrypt.lib kernel32.lib cfgmgr32.lib user32.lib ole32.lib advapi32.lib msvcrtd.lib
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
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/vpxmt.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/python27.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/lzma.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/zlib.lib"
    LIBS += "C:/vcpkg/installed/x64-windows-static/lib/OpenCL.lib"

    LIBS += secur32.lib ws2_32.lib bcrypt.lib kernel32.lib cfgmgr32.lib user32.lib ole32.lib advapi32.lib
  }
}

unix:!macx{
  QMAKE_CXXFLAGS += -g -std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wl,--stack,4194304 -I/usr/include $$system(python3-config --includes) -I/usr/include/ffmpeg
  LIBS += -lOpenCL -lavcodec -lavutil -lavformat -lswscale $$system(python3-config --embed > /dev/null 2>&1  && python3-config --embed --libs  || python3-config --libs)  -llzma -lz
}

RESOURCES += \
    iraspagui/iraspa-resources.qrc
