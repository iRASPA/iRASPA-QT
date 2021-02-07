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
#include "openglatomsphereshader.h"
#include "openglatomperspectiveimpostershader.h"
#include "openglatomorthographicimpostershader.h"
#include "openglambientocclusionshadowmapshader.h"

class OpenGLAtomPickingShader;
class OpenGLTextRenderingShader;

class OpenGLAtomShader: public QOpenGLFunctions_3_3_Core
{
public:
  OpenGLAtomShader();

  bool initializeOpenGLFunctions() override;
  void paintGL(std::shared_ptr<RKCamera> camera, RKRenderQuality quality, GLuint structureUniformBuffer);
  void initializeVertexArrayObject();
  void loadShader(void);

  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeLightUniforms();

  void reloadData();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::shared_ptr<RKRenderStructure>> structure);
  void reloadAmbientOcclusionData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality);
private:
  OpenGLAtomSphereShader _atomShader;
  OpenGLAtomOrthographicImposterShader _atomOrthographicImposterShader;
  OpenGLAtomPerspectiveImposterShader _atomPerspectiveImposterShader;
  OpenGLAmbientOcclusionShadowMapShader _atomAmbientOcclusionShader;

  size_t _numberOfAtoms = 0;
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> _renderStructures;

  friend OpenGLAtomPickingShader;
  friend OpenGLTextRenderingShader;
};
