INCLUDEPATH += $$PWD

include(projects.pri)
include(structures.pri)

SOURCES += \
    $$PWD/iraspakitprotocols.cpp

HEADERS += \
    $$PWD/iraspakit.h \
    $$PWD/iraspakitprotocols.h
