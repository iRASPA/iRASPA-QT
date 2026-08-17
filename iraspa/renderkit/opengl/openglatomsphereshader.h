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
#include <QOpenGLFunctions_3_3_Core>
#include "rkrenderkitprotocols.h"

class OpenGLAtomShader;
class OpenGLAtomOrthographicImposterShader;
class OpenGLAtomPerspectiveImposterShader;
class OpenGLAmbientOcclusionShadowMapShader;
class OpenGLAtomPickingShader;
class OpenGLTextRenderingShader;
class OpenGLRibbonAmbientOcclusionShader;

class OpenGLAtomSphereShader: public QOpenGLFunctions_3_3_Core
{
public:
  OpenGLAtomSphereShader();
  void deleteBuffers();
  void generateBuffers();

  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
private:
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  std::vector<std::vector<size_t>> _numberOfDrawnAtoms;
  std::vector<std::vector<GLuint>> _instancePositionBuffer;

  friend OpenGLAtomShader;
  friend OpenGLAtomOrthographicImposterShader;
  friend OpenGLAtomPerspectiveImposterShader;
  friend OpenGLAmbientOcclusionShadowMapShader;
  friend OpenGLAtomPickingShader;
  friend OpenGLTextRenderingShader;
  friend OpenGLRibbonAmbientOcclusionShader;
};
