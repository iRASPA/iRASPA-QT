#include "opengloffscreenrenderer.h"

#include "glgeterror.h"
#include "opengluniformstringliterals.h"
#include "quadgeometry.h"
#include "rkcamera.h"
#include "rkimposters.h"
#include "rkrenderuniforms.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

#include <QDebug>
#include <QImage>
#include <QSurfaceFormat>

OpenGLOffscreenRenderer::OpenGLOffscreenRenderer(int width, int height)
    : _selectionShader(_atomShader, _bondShader, _objectShader),
      _ribbonSelectionShader(_ribbonShader),
      _ribbonAmbientOcclusionShader(_ribbonShader, _atomShader),
      _pickingShader(_atomShader, _bondShader, _objectShader, _ribbonShader)
{
  QSurfaceFormat format;
  format.setSamples(1);
  format.setDepthBufferSize(0);
  format.setStencilBufferSize(0);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);

  _surface.setFormat(format);
  _surface.create();
  if (!_surface.isValid())
  {
    throw std::runtime_error("failed to create an offscreen OpenGL surface");
  }

  _context.setFormat(format);
  if (!_context.create())
  {
    throw std::runtime_error("failed to create an offscreen OpenGL context");
  }
  if (!_context.makeCurrent(&_surface))
  {
    throw std::runtime_error("failed to make the offscreen OpenGL context current");
  }

  initializeGL(std::max(1, width), std::max(1, height));
}

OpenGLOffscreenRenderer::~OpenGLOffscreenRenderer()
{
  if (_context.isValid())
  {
    _context.makeCurrent(&_surface);
    deleteRibbonTextures();
    if (_ribbonFallbackAmbientOcclusionTexture != 0)
    {
      glDeleteTextures(1, &_ribbonFallbackAmbientOcclusionTexture);
      _ribbonFallbackAmbientOcclusionTexture = 0;
    }
    _bondShader.deletePermanentBuffers();
    _context.doneCurrent();
  }
}

void OpenGLOffscreenRenderer::makeContextCurrent()
{
  _context.makeCurrent(&_surface);
}

void OpenGLOffscreenRenderer::initializeGL(int width, int height)
{
  if (!initializeOpenGLFunctions())
  {
    throw std::runtime_error("failed to initialize OpenGL 3.3 functions");
  }

  glEnable(GL_MULTISAMPLE);

  _backgroundShader.initializeOpenGLFunctions();
  _blurShader.initializeOpenGLFunctions();
  _energySurfaceShader.initializeOpenGLFunctions();
  _energyVolumeRenderedSurface.initializeOpenGLFunctions();
  _boundingBoxShader.initializeOpenGLFunctions();
  _globalAxesShader.initializeOpenGLFunctions();
  _atomShader.initializeOpenGLFunctions();
  _bondShader.initializeOpenGLFunctions();
  _objectShader.initializeOpenGLFunctions();
  _unitCellShader.initializeOpenGLFunctions();
  _localAxesShader.initializeOpenGLFunctions();
  _selectionShader.initializeOpenGLFunctions();
  _ribbonShader.initializeOpenGLFunctions();
  _ribbonSelectionShader.initializeOpenGLFunctions();
  _ribbonAmbientOcclusionShader.initializeOpenGLFunctions();
  _pickingShader.initializeEmbeddedOpenGLFunctions();
  _textShader.initializeOpenGLFunctions();

  GLint majorVersion = 0;
  GLint minorVersion = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
  if (majorVersion <= 3 && minorVersion < 3)
  {
    throw std::runtime_error("OpenGL 3.3 or newer is required for offscreen export");
  }

  glGetIntegerv(GL_MAX_CLIP_DISTANCES, &_maxNumberOfClipPlanes);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
  glGetIntegerv(GLenum(GL_MAX_SAMPLES), &_maxSampleCount);
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &_maxSampleColorCount);
  glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &_maxSampleDepthCount);
  _maxMultiSampling = std::min(_maxSampleCount, std::min(_maxSampleColorCount, _maxSampleDepthCount));
  _multiSampling = std::max(1, std::min(8, _maxMultiSampling));

  _devicePixelRatio = 1.0;
  _width = width;
  _height = height;
  _quality = RKRenderQuality::picture;

  _blurShader.initializeFramebuffers();
  _pickingShader.generateFrameBuffers();

  loadShader();
  _backgroundShader.loadShader();
  _blurShader.loadShader();
  _energySurfaceShader.loadShader();
  _energyVolumeRenderedSurface.loadShader();
  _boundingBoxShader.loadShader();
  _globalAxesShader.loadShader();
  _atomShader.loadShader();
  _bondShader.loadShader();
  _objectShader.loadShader();
  _unitCellShader.loadShader();
  _localAxesShader.loadShader();
  _selectionShader.loadShader();
  _ribbonShader.loadShader();
  _ribbonSelectionShader.loadShader();
  _ribbonAmbientOcclusionShader.loadShader();
  _pickingShader.loadShader();
  _textShader.loadShader();

  _energySurfaceShader.generateBuffers();
  _energyVolumeRenderedSurface.generateBuffers();
  _boundingBoxShader.generateBuffers();
  _globalAxesShader.generateBuffers();
  _unitCellShader.generateBuffers();
  _localAxesShader.generateBuffers();
  _bondShader.generatePermanentBuffers();
  _textShader.generateTextures();

  _backgroundShader.initializeVertexArrayObject();
  _energySurfaceShader.initializeVertexArrayObject();
  _energyVolumeRenderedSurface.initializeVertexArrayObject();
  _blurShader.initializeVertexArrayObject();
  _boundingBoxShader.initializeVertexArrayObject();
  _globalAxesShader.initializeVertexArrayObject();
  _atomShader.initializeVertexArrayObject();
  _bondShader.initializeVertexArrayObject();
  _objectShader.initializeVertexArrayObject();
  _unitCellShader.initializeVertexArrayObject();
  _localAxesShader.initializeVertexArrayObject();
  _selectionShader.initializeVertexArrayObject();
  _ribbonShader.initializeVertexArrayObject();
  _ribbonSelectionShader.initializeVertexArrayObject();
  _pickingShader.initializeVertexArrayObject();
  _textShader.initializeVertexArrayObject();

  initializeTransformUniforms();
  initializeStructureUniforms();
  initializeIsosurfaceUniforms();
  initializeLightUniforms();
  initializeGlobalAxesUniforms();

  glGenVertexArrays(1, &_downSamplerVertexArray);
  glGenBuffers(1, &_quad_vertexbuffer);
  glGenBuffers(1, &_quad_indexbuffer);

  QuadGeometry quad;
  glBindVertexArray(_downSamplerVertexArray);
  glBindBuffer(GL_ARRAY_BUFFER, _quad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, quad.vertices().size() * sizeof(RKVertex), quad.vertices().data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad_indexbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad.indices().size() * sizeof(GLushort), quad.indices().data(), GL_STATIC_DRAW);
  glVertexAttribPointer(_downSamplePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex),
                        reinterpret_cast<GLvoid *>(offsetof(RKVertex, position)));
  glEnableVertexAttribArray(_downSamplePositionAttributeLocation);
  glBindVertexArray(0);

  _blurShader.resizeGL(_width, _height);
  _initialized = true;
}

void OpenGLOffscreenRenderer::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  _renderStructures = std::move(structures);
  if (!_initialized)
  {
    return;
  }
  makeContextCurrent();
  _energySurfaceShader.setRenderStructures(_renderStructures);
  _energyVolumeRenderedSurface.setRenderStructures(_renderStructures);
  _atomShader.setRenderStructures(_renderStructures);
  _bondShader.setRenderStructures(_renderStructures);
  _objectShader.setRenderStructures(_renderStructures);
  _unitCellShader.setRenderStructures(_renderStructures);
  _localAxesShader.setRenderStructures(_renderStructures);
  _selectionShader.setRenderStructures(_renderStructures);
  _ribbonShader.setRenderStructures(_renderStructures);
  _ribbonSelectionShader.setRenderStructures(_renderStructures);
  _ribbonAmbientOcclusionShader.setRenderStructures(_renderStructures);
  _pickingShader.setRenderStructures(_renderStructures);
  _textShader.setRenderStructures(_renderStructures);
  _ribbonShader.reloadData();
  _ribbonSelectionShader.reloadData();
  _pickingShader.reloadData();
}

void OpenGLOffscreenRenderer::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  makeContextCurrent();
  _dataSource = source;
  _boundingBoxShader.setRenderDataSource(source);
  _globalAxesShader.setRenderDataSource(source);
  if (source)
  {
    _camera = source->camera();
    _backgroundShader.reload(source);
    _blurShader.resizeGL(_width, _height);
  }
}

void OpenGLOffscreenRenderer::reloadData(RKRenderQuality quality)
{
  if (!_initialized)
  {
    return;
  }
  makeContextCurrent();
  _energySurfaceShader.reloadData();
  _energyVolumeRenderedSurface.reloadData();
  _boundingBoxShader.reloadData();
  _globalAxesShader.reloadData();
  _atomShader.reloadData();
  _atomShader.reloadAmbientOcclusionData(_dataSource, quality);
  _bondShader.reloadData();
  _objectShader.reloadData();
  _unitCellShader.reloadData();
  _localAxesShader.reloadData();
  _selectionShader.reloadData();
  _ribbonShader.reloadData();
  _ribbonSelectionShader.reloadData();
  _pickingShader.reloadData();
  reloadRibbonAmbientOcclusionTextures(quality);
  _textShader.reloadData();
  updateStructureUniforms();
  updateIsosurfaceUniforms();
}

void OpenGLOffscreenRenderer::ensureRibbonFallbackAmbientOcclusionTexture()
{
  if (_ribbonFallbackAmbientOcclusionTexture != 0)
  {
    return;
  }
  const float white = 1.0f;
  glGenTextures(1, &_ribbonFallbackAmbientOcclusionTexture);
  glBindTexture(GL_TEXTURE_2D, _ribbonFallbackAmbientOcclusionTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1, 1, 0, GL_RED, GL_FLOAT, &white);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLOffscreenRenderer::deleteRibbonTextures()
{
  for (size_t i = 0; i < _ribbonTextures.size(); ++i)
  {
    for (size_t j = 0; j < _ribbonTextures[i].size(); ++j)
    {
      if (_ribbonTextures[i][j] != 0)
      {
        glDeleteTextures(1, &_ribbonTextures[i][j]);
        _ribbonTextures[i][j] = 0;
      }
    }
  }
  _ribbonTextures.clear();
  for (size_t i = 0; i < _ribbonRawTextures.size(); ++i)
  {
    for (size_t j = 0; j < _ribbonRawTextures[i].size(); ++j)
    {
      if (_ribbonRawTextures[i][j] != 0)
      {
        glDeleteTextures(1, &_ribbonRawTextures[i][j]);
        _ribbonRawTextures[i][j] = 0;
      }
    }
  }
  _ribbonRawTextures.clear();
}

void OpenGLOffscreenRenderer::ensureRibbonTextureStorage()
{
  _ribbonTextures.resize(_renderStructures.size());
  _ribbonRawTextures.resize(_renderStructures.size());
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    _ribbonTextures[i].resize(_renderStructures[i].size(), 0);
    _ribbonRawTextures[i].resize(_renderStructures[i].size(), 0);
  }
}

void OpenGLOffscreenRenderer::reloadRibbonAmbientOcclusionTextures(RKRenderQuality quality)
{
  if (!_initialized)
  {
    return;
  }
  _ribbonAmbientOcclusionShader.syncRenderStructures(_renderStructures);
  ensureRibbonFallbackAmbientOcclusionTexture();
  ensureRibbonTextureStorage();
  _ribbonAmbientOcclusionShader.reloadData(_dataSource, quality, _ribbonTextures, _ribbonRawTextures);
}

void OpenGLOffscreenRenderer::initializeTransformUniforms()
{
  glGenBuffers(1, &_frameUniformBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, _frameUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, _frameUniformBuffer);
  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
  _energySurfaceShader.initializeTransformUniforms();
  _energyVolumeRenderedSurface.initializeTransformUniforms();
  _boundingBoxShader.initializeTransformUniforms();
  _globalAxesShader.initializeTransformUniforms();
  _atomShader.initializeTransformUniforms();
  _bondShader.initializeTransformUniforms();
  _objectShader.initializeTransformUniforms();
  _unitCellShader.initializeTransformUniforms();
  _localAxesShader.initializeTransformUniforms();
  _selectionShader.initializeTransformUniforms();
  _pickingShader.initializeTransformUniforms();
  _textShader.initializeTransformUniforms();
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::updateTransformUniforms()
{
  double4x4 projectionMatrix{};
  double4x4 modelViewMatrix{};
  double4x4 modelMatrix{};
  double4x4 viewMatrix{};
  double4x4 axesProjectionMatrix{};
  double4x4 axesModelViewMatrix{};
  double bloomLevel = 1.0;
  double bloomPulse = 1.0;
  bool isOrthographic = true;

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    projectionMatrix = camera->projectionMatrix();
    modelViewMatrix = camera->modelViewMatrix();
    modelMatrix = camera->modelMatrix();
    viewMatrix = camera->viewMatrix();
    isOrthographic = camera->isOrthographic();
    if (_dataSource)
    {
      axesProjectionMatrix = camera->axesProjectionMatrix(_dataSource->axes()->totalAxesSize());
      axesModelViewMatrix = camera->axesModelViewMatrix();
    }
    bloomLevel = camera->bloomLevel();
    bloomPulse = camera->bloomPulse();
  }

  glBindBuffer(GL_UNIFORM_BUFFER, _frameUniformBuffer);
  RKTransformationUniforms transformationUniforms(projectionMatrix, modelViewMatrix, modelMatrix, viewMatrix, axesProjectionMatrix,
                                                  axesModelViewMatrix, isOrthographic, bloomLevel, bloomPulse, _multiSampling);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKTransformationUniforms), &transformationUniforms, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::initializeStructureUniforms()
{
  glGenBuffers(1, &_structureUniformBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, _structureUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, _structureUniformBuffer);
  _energySurfaceShader.initializeStructureUniforms();
  _energyVolumeRenderedSurface.initializeStructureUniforms();
  _atomShader.initializeStructureUniforms();
  _bondShader.initializeStructureUniforms();
  _objectShader.initializeStructureUniforms();
  _unitCellShader.initializeStructureUniforms();
  _localAxesShader.initializeStructureUniforms();
  _selectionShader.initializeStructureUniforms();
  _pickingShader.initializeStructureUniforms();
  _textShader.initializeStructureUniforms();
  std::vector<RKStructureUniforms> structureUniforms{RKStructureUniforms()};
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKStructureUniforms) * structureUniforms.size(), structureUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::updateStructureUniforms()
{
  glBindBuffer(GL_UNIFORM_BUFFER, _structureUniformBuffer);
  std::vector<RKStructureUniforms> structureUniforms;
  size_t flatIndex = 0;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      RKStructureUniforms structureUniform(static_cast<uint>(i), static_cast<uint>(j), _renderStructures[i][j]);
      structureUniform.structureIdentifier = int32_t(flatIndex);
      structureUniforms.push_back(structureUniform);
      ++flatIndex;
    }
  }
  if (structureUniforms.empty())
  {
    structureUniforms.push_back(RKStructureUniforms());
  }
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKStructureUniforms) * structureUniforms.size(), structureUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::initializeIsosurfaceUniforms()
{
  glGenBuffers(1, &_isosurfaceUniformBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, _isosurfaceUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 2, _isosurfaceUniformBuffer);
  _energySurfaceShader.initializeIsosurfaceUniforms();
  _energyVolumeRenderedSurface.initializeIsosurfaceUniforms();
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::updateIsosurfaceUniforms()
{
  std::vector<RKIsosurfaceUniforms> isosurfaceUniforms;
  for (size_t i = 0; i < _renderStructures.size(); ++i)
  {
    for (size_t j = 0; j < _renderStructures[i].size(); ++j)
    {
      isosurfaceUniforms.push_back(RKIsosurfaceUniforms(_renderStructures[i][j]));
    }
  }
  if (isosurfaceUniforms.empty())
  {
    isosurfaceUniforms.push_back(RKIsosurfaceUniforms());
  }
  glBindBuffer(GL_UNIFORM_BUFFER, _isosurfaceUniformBuffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKIsosurfaceUniforms) * isosurfaceUniforms.size(), isosurfaceUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::initializeLightUniforms()
{
  glGenBuffers(1, &_lightsUniformBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, _lightsUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 3, _lightsUniformBuffer);
  _energySurfaceShader.initializeLightUniforms();
  _energyVolumeRenderedSurface.initializeLightUniforms();
  _boundingBoxShader.initializeLightUniforms();
  _globalAxesShader.initializeLightUniforms();
  _atomShader.initializeLightUniforms();
  _bondShader.initializeLightUniforms();
  _objectShader.initializeLightUniforms();
  _unitCellShader.initializeLightUniforms();
  _localAxesShader.initializeLightUniforms();
  _selectionShader.initializeLightUniforms();
  std::vector<RKLightsUniforms> lightUniforms{RKLightsUniforms()};
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKLightsUniforms) * lightUniforms.size(), lightUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::updateLightUniforms()
{
  RKLightsUniforms lightUniforms(_dataSource);
  glBindBuffer(GL_UNIFORM_BUFFER, _lightsUniformBuffer);
  glBufferData(GL_UNIFORM_BUFFER, lightUniforms.lights.size() * sizeof(RKLightUniform), lightUniforms.lights.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::initializeGlobalAxesUniforms()
{
  glGenBuffers(1, &_globalAxesUniformBuffer);
  glBindBuffer(GL_UNIFORM_BUFFER, _globalAxesUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 5, _globalAxesUniformBuffer);
  _globalAxesShader.initializeGlobalAxesUniforms();
  RKGlobalAxesUniforms uniformData(_dataSource);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKGlobalAxesUniforms), &uniformData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::updateGlobalAxesUniforms()
{
  if (!_dataSource)
  {
    return;
  }
  RKGlobalAxesUniforms uniformData(_dataSource);
  glBindBuffer(GL_UNIFORM_BUFFER, _globalAxesUniformBuffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKGlobalAxesUniforms), &uniformData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLOffscreenRenderer::drawSceneOpaqueToFramebuffer(GLuint framebuffer)
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  _backgroundShader.paintGL();
  glEnable(GL_DEPTH_TEST);
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _atomShader.paintGL(camera, _quality, _structureUniformBuffer);
    _bondShader.paintGL(_structureUniformBuffer);
    _objectShader.paintGL(_structureUniformBuffer);
    _unitCellShader.paintGL(_structureUniformBuffer);
    _localAxesShader.paintGL(_structureUniformBuffer);
    _ribbonShader.paintGL(_ribbonTextures, _ribbonRawTextures, _structureUniformBuffer, _ribbonFallbackAmbientOcclusionTexture,
                          int(_width * _devicePixelRatio), int(_height * _devicePixelRatio));
    _boundingBoxShader.paintGL();
    _energySurfaceShader.paintGLOpaque(_structureUniformBuffer, _isosurfaceUniformBuffer);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLOffscreenRenderer::drawSceneVolumeRenderedSurfacesToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture)
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    for (const RKBackToFrontItem &item : backToFrontRenderOrder(_renderStructures, camera.get()))
    {
      _energyVolumeRenderedSurface.paintGLOpaque(_structureUniformBuffer, _isosurfaceUniformBuffer, sceneResolvedDepthTexture,
                                                 static_cast<int>(item.sceneIndex), static_cast<int>(item.movieIndex));
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLOffscreenRenderer::drawSceneTransparentToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture, int width, int height)
{
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    for (const RKBackToFrontItem &item : backToFrontRenderOrder(_renderStructures, camera.get()))
    {
      _energyVolumeRenderedSurface.paintGLTransparent(_structureUniformBuffer, _isosurfaceUniformBuffer, sceneResolvedDepthTexture,
                                                     static_cast<int>(item.sceneIndex), static_cast<int>(item.movieIndex));
      _objectShader.paintGLTransparent(_structureUniformBuffer, static_cast<int>(item.sceneIndex), static_cast<int>(item.movieIndex));
      _energySurfaceShader.paintGLTransparent(_structureUniformBuffer, _isosurfaceUniformBuffer,
                                              static_cast<int>(item.sceneIndex), static_cast<int>(item.movieIndex));
    }
    _ribbonSelectionShader.paintOverlayGL(_structureUniformBuffer);
    _selectionShader.paintGL(camera, _quality, _structureUniformBuffer);
    _textShader.paintGL(_structureUniformBuffer);
    _globalAxesShader.paintGL(width, height);
  }
  glViewport(0, 0, width, height);
  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  _selectionShader.paintGLGlow(_structureUniformBuffer);
  _ribbonSelectionShader.paintGlowGL(_structureUniformBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

QImage OpenGLOffscreenRenderer::renderSceneToImage(int width, int height, RKRenderQuality quality)
{
  makeContextCurrent();
  const int w = std::max(1, width);
  const int h = std::max(1, height);
  _width = w;
  _height = h;
  _quality = quality;
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->updateCameraForWindowResize(w, h);
  }
  _blurShader.resizeGL(w, h);

  glEnable(GL_MULTISAMPLE);
  glDepthFunc(GL_LEQUAL);
  updateStructureUniforms();
  updateTransformUniforms();
  updateIsosurfaceUniforms();
  updateLightUniforms();
  updateGlobalAxesUniforms();

  GLuint sceneFrameBuffer = 0;
  GLuint sceneDepthTexture = 0;
  GLuint sceneTexture = 0;
  GLuint glowSelectionTexture = 0;
  GLuint sceneResolveDepthFrameBuffer = 0;
  GLuint sceneResolvedDepthTexture = 0;

  glGenFramebuffers(1, &sceneFrameBuffer);
  glGenTextures(1, &sceneDepthTexture);
  glGenTextures(1, &sceneTexture);
  glGenTextures(1, &glowSelectionTexture);
  glBindFramebuffer(GL_FRAMEBUFFER, sceneFrameBuffer);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, w, h, GL_TRUE);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, sceneTexture, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, glowSelectionTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, w, h, GL_TRUE);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, glowSelectionTexture, 0);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneDepthTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_DEPTH32F_STENCIL8, w, h, GL_TRUE);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, sceneDepthTexture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(1, &sceneResolveDepthFrameBuffer);
  glGenTextures(1, &sceneResolvedDepthTexture);
  glBindFramebuffer(GL_FRAMEBUFFER, sceneResolveDepthFrameBuffer);
  glBindTexture(GL_TEXTURE_2D, sceneResolvedDepthTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, sceneResolvedDepthTexture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0, 0, w, h);
  drawSceneOpaqueToFramebuffer(sceneFrameBuffer);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFrameBuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sceneResolveDepthFrameBuffer);
  glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  drawSceneVolumeRenderedSurfacesToFramebuffer(sceneFrameBuffer, sceneResolvedDepthTexture);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFrameBuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, sceneResolveDepthFrameBuffer);
  glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  drawSceneTransparentToFramebuffer(sceneFrameBuffer, sceneResolvedDepthTexture, w, h);
  _blurShader.paintGL(glowSelectionTexture, w, h);

  GLuint downSamplerFrameBufferObject = 0;
  GLuint downSamplerTexture = 0;
  glGenFramebuffers(1, &downSamplerFrameBufferObject);
  glGenTextures(1, &downSamplerTexture);
  glBindFramebuffer(GL_FRAMEBUFFER, downSamplerFrameBufferObject);
  glBindTexture(GL_TEXTURE_2D, downSamplerTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, downSamplerTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glViewport(0, 0, w, h);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  glUseProgram(_program);
  glBindVertexArray(_downSamplerVertexArray);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneTexture);
  glUniform1i(_downSampleInputTextureUniformLocation, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _blurShader.blurredTexture());
  glUniform1i(_blurredInputTextureUniformLocation, 1);
  glUniform1i(_numberOfMultiSamplePointsUniformLocation, _multiSampling);
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, nullptr);
  glBindVertexArray(0);
  glUseProgram(0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  QImage img(w, h, QImage::Format_ARGB32);
  glBindTexture(GL_TEXTURE_2D, downSamplerTexture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, img.bits());
  glBindTexture(GL_TEXTURE_2D, 0);

  glDeleteTextures(1, &downSamplerTexture);
  glDeleteFramebuffers(1, &downSamplerFrameBufferObject);
  glDeleteTextures(1, &sceneResolvedDepthTexture);
  glDeleteFramebuffers(1, &sceneResolveDepthFrameBuffer);
  glDeleteTextures(1, &glowSelectionTexture);
  glDeleteTextures(1, &sceneTexture);
  glDeleteTextures(1, &sceneDepthTexture);
  glDeleteFramebuffers(1, &sceneFrameBuffer);

  return img.mirrored();
}

void OpenGLOffscreenRenderer::loadShader()
{
  GLuint vertexShader = compileShaderOfType(GL_VERTEX_SHADER, _vertexShaderSource.c_str());
  GLuint fragmentShader = compileShaderOfType(GL_FRAGMENT_SHADER, _fragmentShaderSource.c_str());
  if (vertexShader == 0 || fragmentShader == 0)
  {
    throw std::runtime_error("failed to compile offscreen composite shaders");
  }
  _program = glCreateProgram();
  glAttachShader(_program, vertexShader);
  glAttachShader(_program, fragmentShader);
  glBindFragDataLocation(_program, 0, "vFragColor");
  linkProgram(_program);
  _downSampleInputTextureUniformLocation = glGetUniformLocation(_program, "originalTexture");
  _blurredInputTextureUniformLocation = glGetUniformLocation(_program, "blurredTexture");
  _downSamplePositionAttributeLocation = glGetAttribLocation(_program, "position");
  _numberOfMultiSamplePointsUniformLocation = glGetUniformLocation(_program, "numberOfMultiSamplePoints");
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
}

const std::string OpenGLOffscreenRenderer::_vertexShaderSource =
    OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
    std::string(R"foo(
in vec4 position;
out vec2 texcoord;
void main()
{
  gl_Position = position;
  texcoord = position.xy * vec2(0.5) + vec2(0.5);
}
)foo");

const std::string OpenGLOffscreenRenderer::_fragmentShaderSource =
    OpenGLUniformStringLiterals::OpenGLVersionStringLiteral +
    OpenGLUniformStringLiterals::OpenGLFrameUniformBlockStringLiteral +
    std::string(R"foo(
in vec2 texcoord;
out vec4 vFragColor;
uniform sampler2DMS originalTexture;
uniform sampler2D blurredTexture;
uniform int numberOfMultiSamplePoints;
void main()
{
  vec2 tmp = floor(textureSize(originalTexture) * texcoord);
  vec4 vColor = vec4(0.0);
  for (int i = 0; i < numberOfMultiSamplePoints; i++)
  {
    vColor += texelFetch(originalTexture, ivec2(tmp), i);
  }
  vFragColor = vColor / float(numberOfMultiSamplePoints) + frameUniforms.bloomPulse * frameUniforms.bloomLevel * texture(blurredTexture, texcoord);
}
)foo");
