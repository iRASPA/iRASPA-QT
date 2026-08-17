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
#include "rkrenderkitprotocols.h"

class OpenGLExternalBondPickingShader;
class OpenGLExternalBondSelectionStripesShader;
class OpenGLExternalBondSelectionWorleyNoise3DShader;
class OpenGLExternalBondSelectionGlowShader;

class OpenGLExternalBondShader: public OpenGLShader
{
public:
  OpenGLExternalBondShader();
  void loadShader(void) override final;
  void deleteBuffers();
  void generateBuffers();
  void deletePermanentBuffers();
  void generatePermanentBuffers();

  void paintGL(GLuint structureUniformBuffer);

  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  GLuint imposterProgram() {return _programImposter;}
private:
  GLuint _programImposter;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  void paintGLImposters(GLuint structureUniformBuffer);
  void initializeImposterVertexArrayObject(GLuint vertexArrayObject, GLuint instanceBuffer);

  std::vector<std::vector<size_t>> _numberOfAllBonds;
  std::vector<std::vector<GLuint>> _vertexAllBondsImposterArrayObject;
  std::vector<std::vector<GLuint>> _vertexAllBondsInstanceBuffer;

  std::vector<std::vector<size_t>> _numberOfSingleBonds;
  std::vector<std::vector<GLuint>> _vertexSingleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexSingleBondsImposterArrayObject;

  std::vector<std::vector<size_t>> _numberOfDoubleBonds;
  std::vector<std::vector<GLuint>> _vertexDoubleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexDoubleBondsImposterArrayObject;

  std::vector<std::vector<size_t>> _numberOfPartialDoubleBonds;
  std::vector<std::vector<GLuint>> _vertexPartialDoubleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexPartialDoubleBondsImposterArrayObject;

  std::vector<std::vector<size_t>> _numberOfTripleBonds;
  std::vector<std::vector<GLuint>> _vertexTripleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexTripleBondsImposterArrayObject;

  GLint _imposterInstancePositionFirstAtomAttributeLocation;
  GLint _imposterInstancePositionSecondAtomAttributeLocation;
  GLint _imposterInstanceColorFirstAtomAttributeLocation;
  GLint _imposterInstanceColorSecondAtomAttributeLocation;
  GLint _imposterInstanceScaleAttributeLocation;
  GLint _imposterBondTypeUniformLocation;

  static const std::string _vertexShaderSourceImposter;
  static const std::string _fragmentShaderSourceImposter;

  friend OpenGLExternalBondPickingShader;
  friend OpenGLExternalBondSelectionStripesShader;
  friend OpenGLExternalBondSelectionWorleyNoise3DShader;
  friend OpenGLExternalBondSelectionGlowShader;
};
