QT += core testlib widgets
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

INCLUDEPATH += ../../iraspa/mathkit
INCLUDEPATH += ../../iraspa/foundationkit
INCLUDEPATH += ../../iraspa/symmetrykit

include(../../iraspa/mathkit/mathkit.pri)
include(../../iraspa/foundationkit/foundationkit.pri)
include(../../iraspa/symmetrykit/symmetrykit.pri)

QMAKE_LFLAGS += -framework Accelerate
LIBS += -L/usr/local/lib -llzma -lz

TEMPLATE = app

HEADERS +=   \
  tst_symmetrykitsmallestprimitivecellnopartialoccupancies.h \
  tst_symmetrykitsmallestprimitivecell.h 

SOURCES +=  main.cpp \
  tst_symmetrykitsmallestprimitivecellnopartialoccupancies.cpp \
  tst_symmetrykitsmallestprimitivecell.cpp 

install_test_files.path = $$OUT_PWD
install_test_files.files = $$PWD/spglibtestdata
COPIES += install_test_files