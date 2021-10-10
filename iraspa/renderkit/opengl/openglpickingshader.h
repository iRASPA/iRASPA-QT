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
#include <array>
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QGLFunctions>
#else
  #include <QOpenGLFunctions>
#endif
#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"

#include "openglshader.h"
#include "openglatomshader.h"
#include "openglbondshader.h"
#include "openglobjectshader.h"
#include "openglinternalbondshader.h"
#include "openglexternalbondshader.h"
#include "openglcrystalcylinderobjectshader.h"
#include "openglcrystalellipseobjectshader.h"
#include "openglcrystalpolygonalprismobjectshader.h"
#include "openglcylinderobjectshader.h"
#include "openglellipseobjectshader.h"
#include "openglpolygonalprismobjectshader.h"

#include "openglatompickingshader.h"
#include "openglinternalbondpickingshader.h"
#include "openglexternalbondpickingshader.h"
#include "openglcrystalcylinderobjectpickingshader.h"
#include "openglcrystalellipseobjectpickingshader.h"
#include "openglcrystalpolygonalprismobjectpickingshader.h"
#include "openglcylinderobjectpickingshader.h"
#include "openglellipseobjectpickingshader.h"
#include "openglpolygonalprismobjectpickingshader.h"

class OpenGLPickingShader: public OpenGLShader
{
public:
  OpenGLPickingShader(OpenGLAtomShader &atomShader, OpenGLBondShader &bondShader, OpenGLObjectShader &objectShader);

  void initializeEmbeddedOpenGLFunctions();
  void loadShader();
  void initializeVertexArrayObject();
  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void generateFrameBuffers();

  void paintGL(int width,int height,GLuint structureUniformBuffer);
  void resizeGL(int w, int h);

  std::array<int,4> pickTexture(int x, int y, int width, int height);

  GLuint &depthTexture() {return _depthTexture;}
  GLuint &colorTexture() {return _texture;}

  void reloadData();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
private:
  OpenGLAtomPickingShader _atomPickingShader;
  OpenGLInternalBondPickingShader _internalBondPickingShader;
  OpenGLExternalBondPickingShader _externalBondPickingShader;
  OpenGLCrystalCylinderObjectPickingShader _crystalCylinderPickingShader;
  OpenGLCrystalEllipseObjectPickingShader _crystalEllipsePickingShader;
  OpenGLCrystalPolygonalPrismObjectPickingShader _crystalPolyogonalPrismPickingShader;
  OpenGLCylinderObjectPickingShader _cylinderPickingShader;
  OpenGLEllipseObjectPickingShader _ellipsePickingShader;
  OpenGLPolygonalPrismObjectPickingShader _polygonalPrismPickingShader;

  GLuint _frameBufferObject;
  GLuint _texture;
  GLuint _depthTexture;

  friend OpenGLAtomPickingShader;
  friend OpenGLInternalBondPickingShader;
  friend OpenGLExternalBondPickingShader;
};
