INCLUDEPATH += $$PWD

include(geometry/geometry.pri)

SOURCES += \
    $$PWD/openglenergyvolumerenderedsurface.cpp \
    $$PWD/opengllocalaxesshader.cpp \
    $$PWD/opengluniformstringliterals.cpp \
    $$PWD/openglwindow.cpp \
    $$PWD/openglglobalaxesbackgroundshader.cpp \
    $$PWD/openglglobalaxesshader.cpp \
    $$PWD/openglglobalaxessystemshader.cpp \
    $$PWD/openglglobalaxestextshader.cpp \
    $$PWD/openglatompickingshader.cpp \
    $$PWD/openglatomselectionshader.cpp \
    $$PWD/openglatomshader.cpp \
    $$PWD/openglatomsphereshader.cpp \
    $$PWD/openglbondselectionshader.cpp \
    $$PWD/openglbondshader.cpp \
    $$PWD/openglboundingboxcylindershader.cpp \
    $$PWD/openglboundingboxshader.cpp \
    $$PWD/openglboundingboxsphereshader.cpp \
    $$PWD/openglcrystalcylinderobjectpickingshader.cpp \
    $$PWD/openglcrystalcylinderobjectselectionglowshader.cpp \
    $$PWD/openglcrystalcylinderobjectselectionstripesshader.cpp \
    $$PWD/openglcrystalcylinderobjectselectionworleynoise3dshader.cpp \
    $$PWD/openglcrystalcylinderobjectshader.cpp \
    $$PWD/openglcrystalellipseobjectpickingshader.cpp \
    $$PWD/openglcrystalellipseobjectshader.cpp \
    $$PWD/openglcrystalellipsoidobjectselectionglowshader.cpp \
    $$PWD/openglcrystalellipsoidobjectselectionstripesshader.cpp \
    $$PWD/openglcrystalellipsoidobjectselectionworleynoise3dshader.cpp \
    $$PWD/openglcrystalellipsoidselectioninstanceshader.cpp \
    $$PWD/openglcrystalpolygonalprismobjectpickingshader.cpp \
    $$PWD/openglcrystalpolygonalprismobjectselectionglowshader.cpp \
    $$PWD/openglcrystalpolygonalprismobjectselectionstripesshader.cpp \
    $$PWD/openglcrystalpolygonalprismobjectselectionworleynoise3dshader.cpp \
    $$PWD/openglcrystalpolygonalprismobjectshader.cpp \
    $$PWD/openglcrystalpolygonalprismselectioninstanceshader.cpp \
    $$PWD/openglcylinderobjectpickingshader.cpp \
    $$PWD/openglcylinderobjectselectionglowshader.cpp \
    $$PWD/openglcylinderobjectselectionstripesshader.cpp \
    $$PWD/openglcylinderobjectselectionworleynoise3dshader.cpp \
    $$PWD/openglcylinderobjectshader.cpp \
    $$PWD/openglcylinderselectioninstanceshader.cpp \
    $$PWD/openglellipseobjectpickingshader.cpp \
    $$PWD/openglellipseobjectshader.cpp \
    $$PWD/openglellipsoidobjectselectionglowshader.cpp \
    $$PWD/openglellipsoidobjectselectionstripesshader.cpp \
    $$PWD/openglellipsoidobjectselectionworleynoise3dshader.cpp \
    $$PWD/openglellipsoidselectioninstanceshader.cpp \
    $$PWD/openglexternalbondpickingshader.cpp \
    $$PWD/openglexternalbondselectionglowshader.cpp \
    $$PWD/openglexternalbondselectioninstanceshader.cpp \
    $$PWD/openglexternalbondselectionstripesshader.cpp \
    $$PWD/openglexternalbondselectionworleynoise3dshader.cpp \
    $$PWD/openglinternalbondpickingshader.cpp \
    $$PWD/openglinternalbondselectionglowshader.cpp \
    $$PWD/openglinternalbondselectioninstanceshader.cpp \
    $$PWD/openglinternalbondselectionstripesshader.cpp \
    $$PWD/openglinternalbondselectionworleynoise3dshader.cpp \
    $$PWD/openglcrystalcylinderselectioninstanceshader.cpp \
    $$PWD/openglobjectselectionshader.cpp \
    $$PWD/openglpolygonalprismobjectselectionglowshader.cpp \
    $$PWD/openglpolygonalprismobjectselectionstripesshader.cpp \
    $$PWD/openglpolygonalprismobjectselectionworleynoise3dshader.cpp \
    $$PWD/openglpolygonalprismselectioninstanceshader.cpp \
    $$PWD/openglselectionshader.cpp \
    $$PWD/openglobjectshader.cpp \
    $$PWD/openglpickingshader.cpp \
    $$PWD/openglpolygonalprismobjectpickingshader.cpp \
    $$PWD/openglpolygonalprismobjectshader.cpp \
    $$PWD/openglshader.cpp \
    $$PWD/openglinternalbondshader.cpp \
    $$PWD/opengltextrenderingshader.cpp \
    $$PWD/openglunitcellshader.cpp \
    $$PWD/openglunitcellsphereshader.cpp \
    $$PWD/openglunitcellcylindershader.cpp \
    $$PWD/openglatomorthographicimpostershader.cpp \
    $$PWD/openglatomperspectiveimpostershader.cpp \
    $$PWD/openglambientocclusionshadowmapshader.cpp \
    $$PWD/glgeterror.cpp \
    $$PWD/openglenergysurface.cpp \
    $$PWD/openglatomselectionworleynoise3dshader.cpp \
    $$PWD/openglatomselectionworleynoise3dperspectiveimpostershader.cpp \
    $$PWD/openglatomselectionworleynoise3dorthographicimpostershader.cpp \
    $$PWD/openglatomselectionstripesorthographicimpostershader.cpp \
    $$PWD/openglatomselectionstripesperspectiveimpostershader.cpp \
    $$PWD/openglatomselectionstripesshader.cpp \
    $$PWD/openglexternalbondshader.cpp \
    $$PWD/openglbackgroundshader.cpp \
    $$PWD/openglatomselectionglowshader.cpp \
    $$PWD/openglblurshader.cpp \
    $$PWD/rkfontatlas.cpp


HEADERS += \
    $$PWD/openglenergyvolumerenderedsurface.h \
    $$PWD/opengllocalaxesshader.h \
    $$PWD/opengluniformstringliterals.h \
    $$PWD/openglwindow.h \
    $$PWD/openglglobalaxesbackgroundshader.h \
    $$PWD/openglglobalaxesshader.h \
    $$PWD/openglglobalaxessystemshader.h \
    $$PWD/openglglobalaxestextshader.h \
    $$PWD/openglatompickingshader.h \
    $$PWD/openglatomselectionshader.h \
    $$PWD/openglatomshader.h \
    $$PWD/glgeterror.h \
    $$PWD/openglatomsphereshader.h \
    $$PWD/openglbondselectionshader.h \
    $$PWD/openglbondshader.h \
    $$PWD/openglboundingboxcylindershader.h \
    $$PWD/openglboundingboxshader.h \
    $$PWD/openglboundingboxsphereshader.h \
    $$PWD/openglcrystalcylinderobjectpickingshader.h \
    $$PWD/openglcrystalcylinderobjectselectionglowshader.h \
    $$PWD/openglcrystalcylinderobjectselectionstripesshader.h \
    $$PWD/openglcrystalcylinderobjectselectionworleynoise3dshader.h \
    $$PWD/openglcrystalcylinderobjectshader.h \
    $$PWD/openglcrystalellipseobjectpickingshader.h \
    $$PWD/openglcrystalellipseobjectshader.h \
    $$PWD/openglcrystalellipsoidobjectselectionglowshader.h \
    $$PWD/openglcrystalellipsoidobjectselectionstripesshader.h \
    $$PWD/openglcrystalellipsoidobjectselectionworleynoise3dshader.h \
    $$PWD/openglcrystalellipsoidselectioninstanceshader.h \
    $$PWD/openglcrystalpolygonalprismobjectpickingshader.h \
    $$PWD/openglcrystalpolygonalprismobjectselectionglowshader.h \
    $$PWD/openglcrystalpolygonalprismobjectselectionstripesshader.h \
    $$PWD/openglcrystalpolygonalprismobjectselectionworleynoise3dshader.h \
    $$PWD/openglcrystalpolygonalprismobjectshader.h \
    $$PWD/openglcrystalpolygonalprismselectioninstanceshader.h \
    $$PWD/openglcylinderobjectpickingshader.h \
    $$PWD/openglcylinderobjectselectionglowshader.h \
    $$PWD/openglcylinderobjectselectionstripesshader.h \
    $$PWD/openglcylinderobjectselectionworleynoise3dshader.h \
    $$PWD/openglcylinderobjectshader.h \
    $$PWD/openglcylinderselectioninstanceshader.h \
    $$PWD/openglellipseobjectpickingshader.h \
    $$PWD/openglellipseobjectshader.h \
    $$PWD/openglellipsoidobjectselectionglowshader.h \
    $$PWD/openglellipsoidobjectselectionstripesshader.h \
    $$PWD/openglellipsoidobjectselectionworleynoise3dshader.h \
    $$PWD/openglellipsoidselectioninstanceshader.h \
    $$PWD/openglexternalbondpickingshader.h \
    $$PWD/openglexternalbondselectionglowshader.h \
    $$PWD/openglexternalbondselectioninstanceshader.h \
    $$PWD/openglexternalbondselectionstripesshader.h \
    $$PWD/openglexternalbondselectionworleynoise3dshader.h \
    $$PWD/openglinternalbondpickingshader.h \
    $$PWD/openglinternalbondselectionglowshader.h \
    $$PWD/openglinternalbondselectioninstanceshader.h \
    $$PWD/openglinternalbondselectionstripesshader.h \
    $$PWD/openglinternalbondselectionworleynoise3dshader.h \
    $$PWD/openglcrystalcylinderselectioninstanceshader.h \
    $$PWD/openglobjectselectionshader.h \
    $$PWD/openglpolygonalprismobjectselectionglowshader.h \
    $$PWD/openglpolygonalprismobjectselectionstripesshader.h \
    $$PWD/openglpolygonalprismobjectselectionworleynoise3dshader.h \
    $$PWD/openglpolygonalprismselectioninstanceshader.h \
    $$PWD/openglselectionshader.h \
    $$PWD/openglobjectshader.h \
    $$PWD/openglpickingshader.h \
    $$PWD/openglpolygonalprismobjectpickingshader.h \
    $$PWD/openglpolygonalprismobjectshader.h \
    $$PWD/openglshader.h \
    $$PWD/openglinternalbondshader.h \
    $$PWD/opengltextrenderingshader.h \
    $$PWD/openglunitcellshader.h \
    $$PWD/openglunitcellsphereshader.h \
    $$PWD/openglunitcellcylindershader.h \
    $$PWD/openglatomorthographicimpostershader.h \
    $$PWD/openglatomperspectiveimpostershader.h \
    $$PWD/openglambientocclusionshadowmapshader.h \
    $$PWD/openglenergysurface.h \
    $$PWD/openglatomselectionworleynoise3dshader.h \
    $$PWD/openglatomselectionworleynoise3dperspectiveimpostershader.h \
    $$PWD/openglatomselectionworleynoise3dorthographicimpostershader.h \
    $$PWD/openglatomselectionstripesorthographicimpostershader.h \
    $$PWD/openglatomselectionstripesperspectiveimpostershader.h \
    $$PWD/openglatomselectionstripesshader.h \
    $$PWD/openglexternalbondshader.h \
    $$PWD/openglbackgroundshader.h \
    $$PWD/openglatomselectionglowshader.h \
    $$PWD/openglblurshader.h \
    $$PWD/rkfontatlas.h
