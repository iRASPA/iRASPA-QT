INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

macx{
  INCLUDEPATH += /usr/local/include
}

SOURCES += \
    $$PWD/binpacker.cpp \
    $$PWD/indexpath.cpp \
    $$PWD/scanner.cpp \
    $$PWD/characterset.cpp \
    $$PWD/logreporting.cpp \
    $$PWD/zip.cpp

HEADERS += \
    $$PWD/binpacker.h \
    $$PWD/foundationkit.h \
    $$PWD/indexpath.h \
    $$PWD/scanner.h \
    $$PWD/characterset.h \
    $$PWD/flatten.h \
    $$PWD/aspectratiosingleitemlayout.h \
    $$PWD/zipreader.h \
    $$PWD/zipwriter.h \
    $$PWD/signalblocker.h \
    $$PWD/logreporting.h \
    $$PWD/serialization.h
