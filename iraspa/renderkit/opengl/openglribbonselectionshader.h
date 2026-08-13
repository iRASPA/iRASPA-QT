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
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  #include <QGLFunctions>
#else
  #include <QOpenGLFunctions>
#endif
#include "openglshader.h"
#include "rkrenderkitprotocols.h"

class OpenGLRibbonShader;

class OpenGLRibbonSelectionShader: public OpenGLShader
{
public:
  explicit OpenGLRibbonSelectionShader(OpenGLRibbonShader &ribbonShader);

  void loadShader(void) override;
  void paintOverlayGL(GLuint structureUniformBuffer);
  void paintGlowGL(GLuint structureUniformBuffer);
  void reloadData();
  void initializeVertexArrayObject();
  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);

private:
  void drawSelectedRanges(RKRenderRibbonSource *ribbonSource, int sceneIndex, int movieIndex, GLuint program);

  OpenGLRibbonShader &_ribbonShader;
  GLuint _glowProgram = 0;
  GLuint _worleyProgram = 0;
  GLuint _stripedProgram = 0;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures;

  static std::string loadShaderStageSource(const char *fileName);
  static std::string buildSelectionVertexShaderSource(const char *fileName, bool includeWorleyHelpers);
  static std::string buildSelectionFragmentShaderSource(const char *fileName, bool includeWorleyHelpers);
};
