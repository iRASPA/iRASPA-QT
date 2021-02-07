/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#pragma once

#include <vector>
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QGLFunctions>
#else
  #include <QOpenGLFunctions>
#endif
#include <QOpenGLFunctions_3_3_Core>
#include "openglshader.h"
#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"
#include "openglobjectshader.h"
#include "openglcrystalcylinderselectioninstanceshader.h"
#include "openglcrystalellipsoidselectioninstanceshader.h"
#include "openglcrystalpolygonalprismselectioninstanceshader.h"
#include "openglcylinderselectioninstanceshader.h"
#include "openglellipsoidselectioninstanceshader.h"
#include "openglpolygonalprismselectioninstanceshader.h"

#include "openglcrystalcylinderobjectselectionstripesshader.h"
#include "openglcrystalcylinderobjectselectionworleynoise3dshader.h"
#include "openglcrystalcylinderobjectselectionglowshader.h"
#include "openglcrystalellipsoidobjectselectionstripesshader.h"
#include "openglcrystalellipsoidobjectselectionworleynoise3dshader.h"
#include "openglcrystalellipsoidobjectselectionglowshader.h"
#include "openglcrystalpolygonalprismobjectselectionstripesshader.h"
#include "openglcrystalpolygonalprismobjectselectionworleynoise3dshader.h"
#include "openglcrystalpolygonalprismobjectselectionglowshader.h"

#include "openglcylinderobjectselectionstripesshader.h"
#include "openglcylinderobjectselectionworleynoise3dshader.h"
#include "openglcylinderobjectselectionglowshader.h"
#include "openglellipsoidobjectselectionstripesshader.h"
#include "openglellipsoidobjectselectionworleynoise3dshader.h"
#include "openglellipsoidobjectselectionglowshader.h"
#include "openglpolygonalprismobjectselectionstripesshader.h"
#include "openglpolygonalprismobjectselectionworleynoise3dshader.h"
#include "openglpolygonalprismobjectselectionglowshader.h"


class OpenGLObjectSelectionShader : public QOpenGLFunctions_3_3_Core
{
public:
  OpenGLObjectSelectionShader(OpenGLObjectShader &_objectShader);

  bool initializeOpenGLFunctions() override;
  void paintGL(GLuint structureUniformBuffer);
  void paintGLGlow(GLuint structureUniformBuffer);
  void initializeVertexArrayObject();
  void loadShader(void);

  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeLightUniforms();

  void reloadData();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
private:
  OpenGLCrystalCylinderSelectionInstanceShader _instanceCrystalCylinderShader;
  OpenGLCrystalEllipsoidSelectionInstanceShader _instanceCrystalEllipsoidShader;
  OpenGLCrystalPolygonalPrismSelectionInstanceShader _instanceCrystalPolygonalPrismShader;
  OpenGLCylinderSelectionInstanceShader _instanceCylinderShader;
  OpenGLEllipsoidSelectionInstanceShader _instanceEllipsoidShader;
  OpenGLPolygonalPrismSelectionInstanceShader _instancePolygonalPrismShader;

  OpenGLCrystalCylinderObjectSelectionStripesShader _crystalCylinderStripesShader;
  OpenGLCrystalCylinderObjectSelectionWorleyNoise3DShader _crystalCylinderWorleyNoise3DShader;
  OpenGLCrystalCylinderObjectSelectionGlowShader _crystalCylinderGlowShader;

  OpenGLCrystalEllipsoidObjectSelectionStripesShader _crystalEllipsoidStripesShader;
  OpenGLCrystalEllipsoidObjectSelectionWorleyNoise3DShader _crystalEllipsoidWorleyNoise3DShader;
  OpenGLCrystalEllipsoidObjectSelectionGlowShader _crystalEllipsoidGlowShader;

  OpenGLCrystalPolygonalPrismObjectSelectionStripesShader _crystalPolygonalPrismStripesShader;
  OpenGLCrystalPolygonalPrismObjectSelectionWorleyNoise3DShader _crystalPolygonalPrismWorleyNoise3DShader;
  OpenGLCrystalPolygonalPrismObjectSelectionGlowShader _crystalPolygonalPrismGlowShader;

  OpenGLCylinderObjectSelectionStripesShader _cylinderStripesShader;
  OpenGLCylinderObjectSelectionWorleyNoise3DShader _cylinderWorleyNoise3DShader;
  OpenGLCylinderObjectSelectionGlowShader _cylinderGlowShader;

  OpenGLEllipsoidObjectSelectionStripesShader _ellipsoidStripesShader;
  OpenGLEllipsoidObjectSelectionWorleyNoise3DShader _ellipsoidWorleyNoise3DShader;
  OpenGLEllipsoidObjectSelectionGlowShader _ellipsoidGlowShader;

  OpenGLPolygonalPrismObjectSelectionStripesShader _polygonalPrismStripesShader;
  OpenGLPolygonalPrismObjectSelectionWorleyNoise3DShader _polygonalPrismWorleyNoise3DShader;
  OpenGLPolygonalPrismObjectSelectionGlowShader _polygonalPrismGlowShader;
};
