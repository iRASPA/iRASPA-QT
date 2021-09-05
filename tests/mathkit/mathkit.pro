QT += core testlib widgets
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

INCLUDEPATH += ../../iraspa/mathkit

include(../../iraspa/mathkit/mathkit.pri)

QMAKE_LFLAGS += -framework Accelerate
LIBS += -L/usr/local/lib -llzma -lz

TEMPLATE = app

HEADERS +=   \
  tst_hnf.h \
  tst_smf.h 

SOURCES +=  main.cpp \
  tst_hnf.cpp \
  tst_smf.cpp 
