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
#include "cappedcylindersinglebondgeometry.h"
#include "cappedcylinderdoublebondgeometry.h"
#include "cappedcylinderpartialdoublebondgeometry.h"
#include "cappedcylindertriplebondgeometry.h"

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
  GLuint program() {return _program;}
  GLuint stencilProgram() {return _programStencil;}
  GLuint boxProgram() {return _programBox;}
private:
  GLuint _program;
  GLuint _programStencil;
  GLuint _programBox;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  std::vector<std::vector<size_t>> _numberOfAllBonds;
  std::vector<std::vector<size_t>> _allBondNumberOfIndices;
  std::vector<std::vector<GLuint>> _vertexAllBondsArrayObject;
  std::vector<std::vector<GLuint>> _vertexAllBondsStencilArrayObject;
  std::vector<std::vector<GLuint>> _vertexAllBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexAllBondsBuffer;
  std::vector<std::vector<GLuint>> _indexAllBondsBuffer;

  std::vector<std::vector<size_t>> _numberOfSingleBonds;
  std::vector<std::vector<size_t>> _singleBondNumberOfIndices;
  std::vector<std::vector<GLuint>> _vertexSingleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexSingleBondsArrayObject;
  std::vector<std::vector<GLuint>> _vertexSingleBondsStencilArrayObject;
  std::vector<std::vector<GLuint>> _vertexSingleBondsBuffer;
  std::vector<std::vector<GLuint>> _indexSingleBondsBuffer;

  std::vector<std::vector<size_t>> _numberOfDoubleBonds;
  std::vector<std::vector<size_t>> _doubleBondNumberOfIndices;
  std::vector<std::vector<GLuint>> _vertexDoubleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexDoubleBondsArrayObject;
  std::vector<std::vector<GLuint>> _vertexDoubleBondsStencilArrayObject;
  std::vector<std::vector<GLuint>> _vertexDoubleBondsBuffer;
  std::vector<std::vector<GLuint>> _indexDoubleBondsBuffer;

  std::vector<std::vector<size_t>> _numberOfPartialDoubleBonds;
  std::vector<std::vector<size_t>> _partialDoubleBondNumberOfIndices;
  std::vector<std::vector<GLuint>> _vertexPartialDoubleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexPartialDoubleBondsArrayObject;
  std::vector<std::vector<GLuint>> _vertexPartialDoubleBondsStencilArrayObject;
  std::vector<std::vector<GLuint>> _vertexPartialDoubleBondsBuffer;
  std::vector<std::vector<GLuint>> _indexPartialDoubleBondsBuffer;

  std::vector<std::vector<size_t>> _numberOfTripleBonds;
  std::vector<std::vector<size_t>> _tripleBondNumberOfIndices;
  std::vector<std::vector<GLuint>> _vertexTripleBondsInstanceBuffer;
  std::vector<std::vector<GLuint>> _vertexTripleBondsArrayObject;
  std::vector<std::vector<GLuint>> _vertexTripleBondsStencilArrayObject;
  std::vector<std::vector<GLuint>> _vertexTripleBondsBuffer;
  std::vector<std::vector<GLuint>> _indexTripleBondsBuffer;

  GLint _vertexNormalAttributeLocation;
  GLint _vertexPositionAttributeLocation;
  GLint _instancePositionFirstAtomAttributeLocation;
  GLint _instancePositionSecondAtomAttributeLocation;
  GLint _instanceColorFirstAtomAttributeLocation;
  GLint _instanceColorSecondAtomAttributeLocation;
  GLint _instanceScaleAttributeLocation;

  GLint _vertexStencilNormalAttributeLocation;
  GLint _vertexStencilPositionAttributeLocation;
  GLint _instanceStencilPositionFirstAtomAttributeLocation;
  GLint _instanceStencilPositionSecondAtomAttributeLocation;
  GLint _instanceStencilColorFirstAtomAttributeLocation;
  GLint _instanceStencilColorSecondAtomAttributeLocation;
  GLint _instanceStencilScaleAttributeLocation;

  GLint _boxVertexPositionAttributeLocation;
  GLint _boxVertexNormalAttributeLocation;

  GLuint _boxVertexArray;
  GLuint _boxVertexBuffer;
  GLuint _boxIndexBuffer;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;

  static const std::string _vertexShaderSourceStencil;
  static const std::string _fragmentShaderSourceStencil;

  static const std::string _vertexShaderSourceBox;
  static const std::string _fragmentShaderSourceBox;

  friend OpenGLExternalBondPickingShader;
  friend OpenGLExternalBondSelectionStripesShader;
  friend OpenGLExternalBondSelectionWorleyNoise3DShader;
  friend OpenGLExternalBondSelectionGlowShader;
};
