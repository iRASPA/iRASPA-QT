QMAKEVERSION = $$[QMAKE_VERSION]
ISQT5 = $$find(QMAKEVERSION, ^[2-9])
isEmpty( ISQT5 ) {
error("Use the qmake include with Qt5 or greater, on Ubuntu that is qmake");
}

TEMPLATE = subdirs
SUBDIRS  = iraspa \
           tests
CONFIG += ordered warn_on qt debug_and_release
