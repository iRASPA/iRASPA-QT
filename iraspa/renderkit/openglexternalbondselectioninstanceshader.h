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
#include <QGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include "openglshader.h"
#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"
#include "openglbondshader.h"

class OpenGLInternalBondSelectionStripesShader;
class OpenGLExternalBondSelectionStripesShader;
class OpenGLInternalBondSelectionWorleyNoise3DShader;
class OpenGLExternalBondSelectionWorleyNoise3DShader;
class OpenGLInternalBondSelectionGlowShader;
class OpenGLExternalBondSelectionGlowShader;

class OpenGLExternalBondSelectionInstanceShader: public QOpenGLFunctions_3_3_Core
{
public:
  OpenGLExternalBondSelectionInstanceShader();
  void deleteBuffers();
  void generateBuffers();

  void initializeVertexArrayObject();

  void reloadData();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures);
private:
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> _renderStructures;

  std::vector<std::vector<size_t>> _numberOfAllBonds;
  std::vector<std::vector<GLuint>> _vertexAllBondsInstanceBuffer;

  std::vector<std::vector<size_t>> _numberOfSingleBonds;
  std::vector<std::vector<GLuint>> _vertexSingleBondsInstanceBuffer;

  std::vector<std::vector<size_t>> _numberOfDoubleBonds;
  std::vector<std::vector<GLuint>> _vertexDoubleBondsInstanceBuffer;

  std::vector<std::vector<size_t>> _numberOfPartialDoubleBonds;
  std::vector<std::vector<GLuint>> _vertexPartialDoubleBondsInstanceBuffer;

  std::vector<std::vector<size_t>> _numberOfTripleBonds;
  std::vector<std::vector<GLuint>> _vertexTripleBondsInstanceBuffer;

  friend OpenGLInternalBondSelectionStripesShader;
  friend OpenGLExternalBondSelectionStripesShader;
  friend OpenGLInternalBondSelectionWorleyNoise3DShader;
  friend OpenGLExternalBondSelectionWorleyNoise3DShader;
  friend OpenGLInternalBondSelectionGlowShader;
  friend OpenGLExternalBondSelectionGlowShader;
};
