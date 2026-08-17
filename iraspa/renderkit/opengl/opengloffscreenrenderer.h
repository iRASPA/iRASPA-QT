#pragma once

#include <memory>
#include <vector>

#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSurfaceFormat>

#include "openglbackgroundshader.h"
#include "openglblurshader.h"
#include "openglboundingboxshader.h"
#include "openglenergysurface.h"
#include "openglenergyvolumerenderedsurface.h"
#include "openglatomshader.h"
#include "openglbondshader.h"
#include "openglobjectshader.h"
#include "openglunitcellshader.h"
#include "opengllocalaxesshader.h"
#include "openglselectionshader.h"
#include "openglribbonshader.h"
#include "openglribbonselectionshader.h"
#include "openglribbonambientocclusionshader.h"
#include "openglpickingshader.h"
#include "opengltextrenderingshader.h"
#include "openglglobalaxesshader.h"
#include "openglshader.h"
#include "rkrenderkitprotocols.h"

class OpenGLOffscreenRenderer : public OpenGLShader
{
public:
  OpenGLOffscreenRenderer(int width, int height);
  ~OpenGLOffscreenRenderer() override;

  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures);
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source);
  void reloadData(RKRenderQuality quality);
  QImage renderSceneToImage(int width, int height, RKRenderQuality quality);

  void loadShader() override;

private:
  void initializeGL(int width, int height);
  void makeContextCurrent();
  void ensureRibbonFallbackAmbientOcclusionTexture();
  void deleteRibbonTextures();
  void ensureRibbonTextureStorage();
  void reloadRibbonAmbientOcclusionTextures(RKRenderQuality quality);
  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeIsosurfaceUniforms();
  void initializeLightUniforms();
  void initializeGlobalAxesUniforms();
  void updateTransformUniforms();
  void updateStructureUniforms();
  void updateIsosurfaceUniforms();
  void updateLightUniforms();
  void updateGlobalAxesUniforms();
  void drawSceneOpaqueToFramebuffer(GLuint framebuffer);
  void drawSceneVolumeRenderedSurfacesToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture);
  void drawSceneTransparentToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture, int width, int height);

  QOffscreenSurface _surface;
  QOpenGLContext _context;
  bool _initialized = false;
  GLuint _program = 0;
  RKRenderQuality _quality = RKRenderQuality::picture;
  std::shared_ptr<RKRenderDataSource> _dataSource;
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures{};
  std::weak_ptr<RKCamera> _camera;
  qreal _devicePixelRatio = 1.0;
  GLint _maxTextureSize = 0;
  GLint _maxNumberOfClipPlanes = 0;
  GLint _maxSampleCount = 0;
  GLint _maxSampleColorCount = 0;
  GLint _maxSampleDepthCount = 0;
  GLint _maxMultiSampling = 1;
  GLint _multiSampling = 1;
  GLint _width = 1;
  GLint _height = 1;
  GLuint _frameUniformBuffer = 0;
  GLuint _structureUniformBuffer = 0;
  GLuint _isosurfaceUniformBuffer = 0;
  GLuint _lightsUniformBuffer = 0;
  GLuint _globalAxesUniformBuffer = 0;
  GLuint _downSamplerVertexArray = 0;
  GLuint _quad_vertexbuffer = 0;
  GLuint _quad_indexbuffer = 0;
  GLint _downSampleInputTextureUniformLocation = 0;
  GLint _blurredInputTextureUniformLocation = 0;
  GLint _downSamplePositionAttributeLocation = 0;
  GLint _numberOfMultiSamplePointsUniformLocation = 0;

  OpenGLBackgroundShader _backgroundShader;
  OpenGLBlurShader _blurShader;
  OpenGLEnergySurface _energySurfaceShader;
  OpenGLEnergyVolumeRenderedSurface _energyVolumeRenderedSurface;
  OpenGLBoundingBoxShader _boundingBoxShader;
  OpenGLGlobalAxesShader _globalAxesShader;
  OpenGLAtomShader _atomShader;
  OpenGLBondShader _bondShader;
  OpenGLObjectShader _objectShader;
  OpenGLUnitCellShader _unitCellShader;
  OpenGLLocalAxesShader _localAxesShader;
  OpenGLSelectionShader _selectionShader;
  OpenGLRibbonShader _ribbonShader;
  OpenGLRibbonSelectionShader _ribbonSelectionShader;
  OpenGLRibbonAmbientOcclusionShader _ribbonAmbientOcclusionShader;
  OpenGLPickingShader _pickingShader;
  OpenGLTextRenderingShader _textShader;
  std::vector<std::vector<GLuint>> _ribbonTextures;
  std::vector<std::vector<GLuint>> _ribbonRawTextures;
  GLuint _ribbonFallbackAmbientOcclusionTexture = 0;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;
};
