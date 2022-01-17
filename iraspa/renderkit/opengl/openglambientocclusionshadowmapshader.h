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
#include "openglshader.h"
#include "openglatomsphereshader.h"
#include "rkrenderkitprotocols.h"
#include "openglatomorthographicimpostershader.h"

class OpenGLAmbientOcclusionShadowMapShader: public OpenGLShader
{
public:
  OpenGLAmbientOcclusionShadowMapShader(OpenGLAtomSphereShader &atomShader,OpenGLAtomOrthographicImposterShader &atomOrthographicImposterShader);
  ~OpenGLAmbientOcclusionShadowMapShader() override final;
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void reloadData(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality);
  void deleteBuffers();
  void generateBuffers();
  void initializeVertexArrayObject();
  void loadShader(void) override final;

  void adjustAmbientOcclusionTextureSize();
  void  updateAmbientOcclusionTextures(std::shared_ptr<RKRenderDataSource> dataSource, RKRenderQuality quality);
  GLuint ambientOcclusionProgram() {return _ambientOcclusionProgram;}
  GLuint shadowMapProgram() {return _shadowMapProgram;}

  std::vector<std::vector<GLuint>> &generatedAmbientOcclusionTextures() {return _generatedAmbientOcclusionTexture;}
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::shared_ptr<RKRenderObject>> structure);

private:
  GLuint _ambientOcclusionProgram;
  GLuint _shadowMapProgram;

  OpenGLAtomSphereShader& _atomShader;
  OpenGLAtomOrthographicImposterShader& _atomOrthographicImposterShader;

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  GLint _atomAmbientOcclusionMapUniformLocation;
  GLint _atomAmbientOcclusionMapWeightUniformLocation;

  GLint _atomAmbientOcclusionMapPositionAttributeLocation;
  GLint _atomAmbientOcclusionMapInstancePositionAttributeLocation;
  GLint _atomAmbientOcclusionMapScaleAttributeLocation;

  GLint _atomShadowMapVertexPositionAttributeLocation;
  GLint _atomShadowMapInstancePositionAttributeLocation;
  GLint _atomShadowMapScaleAttributeLocation;

  std::vector<std::vector<GLuint>> _atomShadowMapVertexArrayObject;
  std::vector<std::vector<GLuint>> _atomAmbientOcclusionMapVertexArrayObject;
  std::vector<std::vector<GLuint>> _ambientOcclusionFrameBufferObject;
  std::vector<std::vector<GLuint>> _generatedAmbientOcclusionTexture;

  QCache<RKRenderObject*, std::vector<uint16_t>> _cache;

  static const std::string _vertexAmbientOcclusionShaderSource;
  static const std::string _fragmentAmbientOcclusionShaderSource;

  static const std::string _vertexShadowMapShaderSource;
  static const std::string _fragmentShadowMapShaderSource;
};

