INCLUDEPATH += $$PWD

include(geometry/geometry.pri)

 contains(DEFINES,USE_OPENGL){
   include(opengl/opengl.pri)
 }
 contains(DEFINES,USE_VULKAN){
   include(vulkan/vulkan.pri)
 }


SOURCES += \
    $$PWD/rkimposters.cpp \
    $$PWD/rkcamera.cpp \
    $$PWD/rkglobalaxes.cpp \
    $$PWD/rklight.cpp \
    $$PWD/rklocalaxes.cpp \
    $$PWD/rkrendererbackend.cpp \
    $$PWD/rkrenderkitprotocols.cpp \
    $$PWD/rkribbonmesh.cpp \
    $$PWD/ribbonaolayout.cpp \
    $$PWD/ribbonaotexturepostprocess.cpp \
    $$PWD/trackball.cpp \
    $$PWD/rkrenderuniforms.cpp \
    $$PWD/volumetransferfunctions.cpp


HEADERS += \
    $$PWD/rkimposters.h \
    $$PWD/renderkit.h \
    $$PWD/rkcamera.h \
    $$PWD/rkglobalaxes.h \
    $$PWD/rklight.h \
    $$PWD/rklocalaxes.h \
    $$PWD/rkfontatlas.h \
    $$PWD/rkrendererbackend.h \
    $$PWD/rkrenderkitprotocols.h \
    $$PWD/rkribbonmesh.h \
    $$PWD/ribbonaolayout.h \
    $$PWD/ribbonaotexturepostprocess.h \
    $$PWD/trackball.h \
    $$PWD/rkrenderuniforms.h \
    $$PWD/volumetransferfunctions.h
