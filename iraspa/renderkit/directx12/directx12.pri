INCLUDEPATH += $$PWD

LIBS += -ldxgi -ld3d12

QT += core-private gui-private

include(geometry/geometry.pri)

SOURCES += $$PWD/qd3d12window.cpp \
    $$PWD/rkfontatlas.cpp \
    $$PWD/directxwindow.cpp

HEADERS += $$PWD/qd3d12window.h \
    $$PWD/rkfontatlas.h \
    $$PWD/directxwindow.h
