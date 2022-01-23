INCLUDEPATH += $$PWD

contains(DEFINES,USE_OPENGL){
   include(opencl/opencl.pri)
  }

SOURCES += \
    $$PWD/forcefieldsets.cpp \
    $$PWD/forcefieldset.cpp \
    $$PWD/forcefieldtype.cpp \
    $$PWD/constants.cpp \
    $$PWD/marchingcubes.cpp

HEADERS += \
    $$PWD/lookuptable.h \
    $$PWD/simulationkit.h \
    $$PWD/forcefieldsets.h \
    $$PWD/forcefieldset.h \
    $$PWD/forcefieldtype.h \
    $$PWD/constants.h \
    $$PWD/marchingcubes.h
