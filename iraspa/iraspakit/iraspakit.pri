INCLUDEPATH += $$PWD

include(projects.pri)
include(objects.pri)
include(interfaces.pri)

SOURCES += \
    $$PWD/iraspakitprotocols.cpp

HEADERS += \
    $$PWD/iraspakit.h \
    $$PWD/iraspakitprotocols.h
