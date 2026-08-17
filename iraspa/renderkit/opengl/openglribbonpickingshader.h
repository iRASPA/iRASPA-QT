/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#pragma once

#include <vector>
#include "openglshader.h"
#include "rkrenderkitprotocols.h"

class OpenGLRibbonShader;

class OpenGLRibbonPickingShader: public OpenGLShader
{
public:
  explicit OpenGLRibbonPickingShader(OpenGLRibbonShader &ribbonShader);

  void loadShader(void) override;
  void paintGL(GLuint structureUniformBuffer);
  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);

  GLuint program() const { return _program; }

private:
  void deleteBuffers();

  OpenGLRibbonShader &_ribbonShader;
  GLuint _program = 0;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;
  std::vector<std::vector<GLuint>> _vertexArrayObject;

  GLint _vertexPositionAttributeLocation = -1;
  GLint _vertexNormalAttributeLocation = -1;
  GLint _vertexSTAttributeLocation = -1;
  GLint _vertexPadAttributeLocation = -1;
  GLint _vertexStripeSTAttributeLocation = -1;

  static std::string loadShaderStageSource(const char *fileName);
  static std::string buildPickingVertexShaderSource();
  static std::string buildPickingFragmentShaderSource();
};
