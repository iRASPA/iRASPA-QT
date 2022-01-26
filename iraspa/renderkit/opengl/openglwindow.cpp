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

#include "openglwindow.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QPaintDevice>
#include <foundationkit.h>
#include "quadgeometry.h"
#include <QtOpenGL/QtOpenGL>
#include <iostream>
#include <array>
#include <algorithm>
#include "glgeterror.h"
#include <QDebug>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>
#include <optional>
#include <QPainter>
#include <QStylePainter>
#include "opengluniformstringliterals.h"

#if defined(Q_OS_WIN)
  #include "wingdi.h"
#endif

OpenGLWindow::OpenGLWindow(QWidget* parent): QOpenGLWindow(),
    _isOpenGLInitialized(false),
    _backgroundShader(),
    _blurShader(),
    _energySurfaceShader(),
    _energyVolumeRenderedSurface(),
    _boundingBoxShader(),
    _globalAxesShader(),
    _atomShader(),
    _bondShader(),
    _objectShader(),
    _unitCellShader(),
    _localAxesShader(),
    _selectionShader(_atomShader, _bondShader, _objectShader),
    _pickingShader(_atomShader, _bondShader, _objectShader),
    _textShader(),
    _timer(new QTimer(parent))
{
  connect(_timer, &QTimer::timeout, this, &OpenGLWindow::timeoutEventHandler);

  QSurfaceFormat format;
  format.setSamples(1);
  format.setDepthBufferSize(0);
  format.setStencilBufferSize(0);
  format.setVersion(3,3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(format);

  setSurfaceType(QWindow::OpenGLSurface);
}


OpenGLWindow::~OpenGLWindow()
{
  makeCurrent();
  _bondShader.deletePermanentBuffers();
}

void OpenGLWindow::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  makeCurrent();

  _dataSource = source;
  _boundingBoxShader.setRenderDataSource(source);
  _globalAxesShader.setRenderDataSource(source);
  if(std::shared_ptr<RKRenderDataSource> dataSource = source)
  {
    _camera = dataSource->camera();
    if (std::shared_ptr<RKCamera> camera = _camera.lock())
    {
      camera->updateCameraForWindowResize(this->size().width(),this->size().height());
      camera->resetForNewBoundingBox(dataSource->renderBoundingBox());
    }

    if (_isOpenGLInitialized)
    {
      _backgroundShader.reload(dataSource);

      _blurShader.resizeGL(_width, _height);
    }
  }

  if (_isOpenGLInitialized)
  {
    reloadData();
    update();

    if (source)
    {
      renderSceneToImage(width(), height(), RKRenderQuality::low);
    }
  }
}

void OpenGLWindow::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures)
{
  makeCurrent();
  _renderStructures = structures;

  _energySurfaceShader.setRenderStructures(structures);
  _energyVolumeRenderedSurface.setRenderStructures(structures);

  _atomShader.setRenderStructures(structures);
  _bondShader.setRenderStructures(structures);
   _objectShader.setRenderStructures(structures);
  _unitCellShader.setRenderStructures(structures);
  _localAxesShader.setRenderStructures(structures);

  _selectionShader.setRenderStructures(structures);
  _pickingShader.setRenderStructures(structures);

  _textShader.setRenderStructures(structures);
}


void OpenGLWindow::redraw()
{
  update();
}

void OpenGLWindow::redrawWithQuality(RKRenderQuality quality)
{
  _quality = quality;
  update();
}




void OpenGLWindow::invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  makeCurrent();
  _atomShader.invalidateCachedAmbientOcclusionTexture(structures);
}

void OpenGLWindow::invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures)
{
  makeCurrent();
  _energySurfaceShader.invalidateIsosurface(structures);
  _energyVolumeRenderedSurface.invalidateIsosurface(structures);
}

std::array<int,4> OpenGLWindow::pickTexture(int x, int y, int width, int height)
{
  makeCurrent();
  return _pickingShader.pickTexture(x,y,width,height);
}

void OpenGLWindow::initializeGL()
{
  if(!context())
  {
    QMessageBox messageBox;
    messageBox.setFixedSize(650, 200);
    messageBox.critical(nullptr, "Critical error", "No OpenGL context present, install OpenGL drivers (>=3.3)");
    QApplication::quit();
  }

  if (!initializeOpenGLFunctions()) 
	  qFatal("Failed to initialize OpenGL functions");

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
  _pickingShader.initializeEmbeddedOpenGLFunctions();

  _textShader.initializeOpenGLFunctions();

  _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenGL initialized"));

  GLint majorVersion = 0;
  GLint minorVersion = 0;
  GLint profile = 0;
  GLint flags = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
  glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                  + "OpenGL major version: " + QString::number(majorVersion)));
  _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                  + "OpenGL major version: " + QString::number(minorVersion)));

  if(majorVersion <= 3 && minorVersion < 3)
  {
    QMessageBox messageBox;
    messageBox.setFixedSize(650, 200);
    messageBox.critical(nullptr, "Critical error", "OpenGL version error, install OpenGL drivers (>=3.3)");
    QApplication::quit();
  }

  if(profile & GL_CONTEXT_CORE_PROFILE_BIT)
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                      + "OpenGL core profile"));
  }

  // get OpenGL capabilities
  glGetIntegerv(GL_MAX_CLIP_DISTANCES, &_maxNumberOfClipPlanes);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);

  _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                      + "max texture size: " + QString::number(_maxTextureSize)));

  glGetIntegerv(GLenum(GL_MAX_SAMPLES), &_maxSampleCount);
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &_maxSampleColorCount);
  glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &_maxSampleDepthCount);
  _maxMultiSampling = std::min(_maxSampleCount, std::min(_maxSampleColorCount, _maxSampleDepthCount));
  _multiSampling = std::max(1,std::min(8, std::min(_maxSampleCount, std::min(_maxSampleColorCount, _maxSampleDepthCount))));


  _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                        + "multisampling: " + QString::number(_multiSampling)));

  _devicePixelRatio = devicePixelRatio();
  _width = std::max(512, this->width());
  _height = std::max(512, this->height());

  _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                          + "pixel device ratio: " + QString::number(_devicePixelRatio)));

  glGenFramebuffers(1, &_sceneFrameBuffer);
  check_gl_error();

  glGenTextures(1, &_sceneDepthTexture);
  check_gl_error();

  glGenTextures(1, &_sceneTexture);
  check_gl_error();

  glGenTextures(1, &_glowSelectionTexture);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, _sceneFrameBuffer);
  check_gl_error();

  glActiveTexture(GL_TEXTURE0);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture);
  check_gl_error();

  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _glowSelectionTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
  check_gl_error();


  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneDepthTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_S), GLint(GL_CLAMP_TO_EDGE));
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_T), GLint(GL_CLAMP_TO_EDGE));
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MIN_FILTER), GLint(GL_NEAREST));
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MAG_FILTER), GLint(GL_NEAREST));
  check_gl_error();
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_DEPTH32F_STENCIL8, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
  check_gl_error();


  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture, 0);
  check_gl_error();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _glowSelectionTexture, 0);
  check_gl_error();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, _sceneDepthTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  check_gl_error();
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("initializeSceneFrameBuffer fatal error: framebuffer incomplete");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);


  glGenFramebuffers(1, &_sceneResolveDepthFrameBuffer);
  check_gl_error();

  glGenTextures(1, &_sceneResolvedDepthTexture);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, _sceneResolveDepthFrameBuffer);
  glBindTexture(GL_TEXTURE_2D, _sceneResolvedDepthTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, _width * _devicePixelRatio, _height * _devicePixelRatio, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _sceneResolvedDepthTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("Resolve Depth FrameBuffer fatal error: framebuffer incomplete");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  _blurShader.initializeFramebuffers();

  _pickingShader.generateFrameBuffers();

  // Set the clear color to white
  glClearColor( 1.0f, 1.0f, 0.0f, 1.0f );

  this->loadShader();
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
  _pickingShader.initializeVertexArrayObject();
  _textShader.initializeVertexArrayObject();

  initializeTransformUniforms();
  check_gl_error();
  initializeStructureUniforms();
  check_gl_error();
  initializeIsosurfaceUniforms();
  check_gl_error();
  initializeLightUniforms();
  check_gl_error();
  initializeGlobalAxesUniforms();
  check_gl_error();

  glGenVertexArrays(1,&_downSamplerVertexArray);
  glGenBuffers(1, &_quad_vertexbuffer);
  glGenBuffers(1, &_quad_indexbuffer);

  QuadGeometry quad = QuadGeometry();

  glBindVertexArray(_downSamplerVertexArray);
  check_gl_error();

  glBindBuffer(GL_ARRAY_BUFFER, _quad_vertexbuffer);
  glBufferData(GL_ARRAY_BUFFER, quad.vertices().size() * sizeof(RKVertex), quad.vertices().data(), GL_STATIC_DRAW);
  check_gl_error();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _quad_indexbuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, quad.indices().size()*sizeof(GLushort), quad.indices().data(), GL_STATIC_DRAW);
  check_gl_error();

  glVertexAttribPointer(_downSamplePositionAttributeLocation, 4, GL_FLOAT, GL_FALSE, sizeof(RKVertex), reinterpret_cast<GLvoid*>(offsetof(RKVertex,position)));
  glEnableVertexAttribArray(_downSamplePositionAttributeLocation);
  check_gl_error();

 // glActiveTexture(GL_TEXTURE0);
 // glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture);
 // check_gl_error();

  glBindVertexArray(0);
  check_gl_error();

  _isOpenGLInitialized = true;
}


void OpenGLWindow::resizeGL( int w, int h )
{
  if(_isOpenGLInitialized)
  {
    makeCurrent();
    check_gl_error();

    if (std::shared_ptr<RKCamera> camera = _camera.lock())
    {
      camera->updateCameraForWindowResize(w, h);
    }

    _pickingShader.resizeGL(w,h);

    _width = std::max(16, w);
    _height = std::max(16, h);

    glBindFramebuffer(GL_FRAMEBUFFER, _sceneFrameBuffer);

    // on nvidia, the multisampled (depth-)texture needs to be recreated to avoid memory corruption
    if(_sceneTexture)
      glDeleteTextures(1, &_sceneTexture);
    glGenTextures(1, &_sceneTexture);
    check_gl_error();

    if(_sceneDepthTexture)
      glDeleteTextures(1, &_sceneDepthTexture);
    glGenTextures(1, &_sceneDepthTexture);
    check_gl_error();

    if(_glowSelectionTexture)
      glDeleteTextures(1, &_glowSelectionTexture);
    glGenTextures(1, &_glowSelectionTexture);
    check_gl_error();

    glBindFramebuffer(GL_FRAMEBUFFER, _sceneFrameBuffer);
    check_gl_error();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
    check_gl_error();
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture, 0);
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _glowSelectionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, _width  * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
    check_gl_error();
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _glowSelectionTexture, 0);
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneDepthTexture);
    glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_S), GLint(GL_CLAMP_TO_EDGE));
    glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_T), GLint(GL_CLAMP_TO_EDGE));
    glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MIN_FILTER), GLint(GL_NEAREST));
    glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MAG_FILTER), GLint(GL_NEAREST));
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_DEPTH32F_STENCIL8, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
    check_gl_error();
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, _sceneDepthTexture, 0);
    check_gl_error();

    // check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
      qWarning("initializeSceneFrameBuffer fatal error: framebuffer incomplete: %0x",status);
      check_gl_error();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _sceneResolveDepthFrameBuffer);
    check_gl_error();
    if(_sceneResolvedDepthTexture)
      glDeleteTextures(1, &_sceneResolvedDepthTexture);
    check_gl_error();
    glGenTextures(1, &_sceneResolvedDepthTexture);
    check_gl_error();

    glBindTexture(GL_TEXTURE_2D, _sceneResolvedDepthTexture);
    check_gl_error();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    check_gl_error();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, _width * _devicePixelRatio, _height * _devicePixelRatio, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    check_gl_error();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _sceneResolvedDepthTexture, 0);
    check_gl_error();

    // check framebuffer completeness
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
      qWarning("Resolve Depth FrameBuffer fatal error: framebuffer incomplete");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
    {
      _blurShader.resizeGL(_width, _height);
    }
  }
}


void OpenGLWindow::paintGL()
{
  if(isValid())
  {
    check_gl_error();
    updateStructureUniforms();
    check_gl_error();
    updateTransformUniforms();
    check_gl_error();
    updateIsosurfaceUniforms();
    check_gl_error();
    updateLightUniforms();
    check_gl_error();
    updateGlobalAxesUniforms();
    check_gl_error();

    glEnable(GL_MULTISAMPLE);

    glDepthFunc(GL_LEQUAL);

    glViewport(0,0,_width,_height);

    // pass mouse coordinates (real coordinates twice larger for retina-displays)
    _pickingShader.paintGL(_structureUniformBuffer);

    glViewport(0,0,_width * _devicePixelRatio,_height * _devicePixelRatio);
    drawSceneOpaqueToFramebuffer(_sceneFrameBuffer);
    check_gl_error();

    // resolve depth-buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _sceneFrameBuffer);
    check_gl_error();
    glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), _sceneResolveDepthFrameBuffer);
    check_gl_error();
    glBlitFramebuffer(0, 0, _width * _devicePixelRatio, _height * _devicePixelRatio, 0, 0, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    check_gl_error();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    drawSceneVolumeRenderedSurfacesToFramebuffer(_sceneFrameBuffer, _sceneResolvedDepthTexture);

    // resolve depth-buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _sceneFrameBuffer);
    check_gl_error();
    glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), _sceneResolveDepthFrameBuffer);
    check_gl_error();
    glBlitFramebuffer(0, 0, _width * _devicePixelRatio, _height * _devicePixelRatio, 0, 0, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    check_gl_error();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    drawSceneTransparentToFramebuffer(_sceneFrameBuffer, _sceneResolvedDepthTexture, _width, _height, _devicePixelRatio);
    check_gl_error();

    glViewport(0,0,_width,_height);
    _blurShader.paintGL(_glowSelectionTexture, _width, _height);

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0,0,_width * _devicePixelRatio,_height * _devicePixelRatio);
    check_gl_error();

    GLfloat black[4] = {0.0,0.0,0.0,0.0};
    glDisable(GL_BLEND);
    glDisable (GL_DEPTH_TEST);
    glClearBufferfv(GL_COLOR, 0, black);
    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(_program);
    glBindVertexArray(_downSamplerVertexArray);
    check_gl_error();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture);
    glUniform1i(_downSampleInputTextureUniformLocation, 0);
    check_gl_error();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, _blurShader.blurredTexture());
    glUniform1i(_blurredInputTextureUniformLocation, 1);
    glUniform1i(_numberOfMultiSamplePointsUniformLocation, _multiSampling);
    glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT, nullptr);
    check_gl_error();

    glBindVertexArray(0);
    glUseProgram(0);
    check_gl_error();
  }
}

void OpenGLWindow::paintOverGL()
{
  if(_dataSource)
  {
    QColor color(Qt::lightGray);
    color.setAlpha(100);

    QPen pen;
    pen.setColor(Qt::darkGray);
    QVector<qreal> dashes{6.0,3.0};
    pen.setDashPattern(dashes);
    pen.setWidthF(2.0);

    QPainter painter(this);

    switch(_tracking)
    {
      case Tracking::draggedAddToSelection:
        painter.setPen(pen);
        painter.drawRect(QRect(_origin,_draggedPos));
        painter.fillRect(QRect(_origin,_draggedPos), QBrush(color));
        break;
      case Tracking::draggedNewSelection:
        painter.setPen(pen);
        painter.setPen(Qt::SolidLine);
        painter.drawRect(QRect(_origin,_draggedPos));
        painter.fillRect(QRect(_origin,_draggedPos), QBrush(color));
        break;
      default:
        break;
    }
  }
}

void OpenGLWindow::paintUnderGL()
{

}

QImage OpenGLWindow::renderSceneToImage(int width, int height, RKRenderQuality quality)
{
  GLuint sceneFrameBuffer;
  GLuint sceneDepthTexture;
  GLuint sceneTexture;
  GLuint glowSelectionTexture;

  GLuint sceneResolveDepthFrameBuffer;
  GLuint sceneResolvedDepthTexture;

  makeCurrent();

  glEnable(GL_MULTISAMPLE);
  glDepthFunc(GL_LEQUAL);

  _atomShader.reloadAmbientOcclusionData(_dataSource, quality);

  updateStructureUniforms();
  updateTransformUniforms();
  updateIsosurfaceUniforms();
  updateLightUniforms();



  glGenFramebuffers(1, &sceneFrameBuffer);
  glGenTextures(1, &sceneDepthTexture);
  glGenTextures(1, &sceneTexture);
  glGenTextures(1, &glowSelectionTexture);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, sceneFrameBuffer);
  check_gl_error();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, width, height, GL_TRUE);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  check_gl_error();

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, glowSelectionTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, width, height, GL_TRUE);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneDepthTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_S), GLint(GL_CLAMP_TO_EDGE));
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_T), GLint(GL_CLAMP_TO_EDGE));
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MIN_FILTER), GLint(GL_NEAREST));
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MAG_FILTER), GLint(GL_NEAREST));
  check_gl_error();
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_DEPTH32F_STENCIL8, width, height, GL_TRUE);
  check_gl_error();


  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, sceneTexture, 0);
  check_gl_error();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, glowSelectionTexture, 0);
  check_gl_error();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, sceneDepthTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("initializeSceneFrameBuffer fatal error: framebuffer incomplete");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glGenFramebuffers(1, &sceneResolveDepthFrameBuffer);
  check_gl_error();

  glGenTextures(1, &sceneResolvedDepthTexture);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, sceneResolveDepthFrameBuffer);
  glBindTexture(GL_TEXTURE_2D, sceneResolvedDepthTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  check_gl_error();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, sceneResolvedDepthTexture, 0);
  check_gl_error();

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("Resolve Depth FrameBuffer fatal error: framebuffer incomplete");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glViewport(0,0,width,height);


  drawSceneOpaqueToFramebuffer(sceneFrameBuffer);

  // resolve depth-buffer
  glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFrameBuffer);
  check_gl_error();
  glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), sceneResolveDepthFrameBuffer);
  check_gl_error();
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  check_gl_error();
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  drawSceneVolumeRenderedSurfacesToFramebuffer(sceneFrameBuffer, sceneResolvedDepthTexture);

  // resolve depth-buffer
  glBindFramebuffer(GL_READ_FRAMEBUFFER, sceneFrameBuffer);
  check_gl_error();
  glBindFramebuffer(GLenum(GL_DRAW_FRAMEBUFFER), sceneResolveDepthFrameBuffer);
  check_gl_error();
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  check_gl_error();
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  drawSceneTransparentToFramebuffer(sceneFrameBuffer, sceneResolvedDepthTexture, width, height, 1);

  GLuint downSamplerFrameBufferObject;
  GLuint downSamplerTexture;

  glGenFramebuffers(1, &downSamplerFrameBufferObject);
  glGenTextures(1, &downSamplerTexture);

  glBindFramebuffer(GL_FRAMEBUFFER, downSamplerFrameBufferObject);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D, downSamplerTexture);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  check_gl_error();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, downSamplerTexture, 0);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D, 0);

  // check framebuffer completeness
  status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("initializeHorizontalBlurFrameBuffer fatal error: framebuffer incomplete");
  }

  glViewport(0,0,width, height);
  check_gl_error();
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  check_gl_error();

  glUseProgram(_program);
  check_gl_error();
  glBindVertexArray(_downSamplerVertexArray);
  check_gl_error();

  // input scene texture
  glActiveTexture(GL_TEXTURE0);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, sceneTexture);
  check_gl_error();
  glUniform1i(_downSampleInputTextureUniformLocation, 0);
  check_gl_error();

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _blurShader.blurredTexture());
  glUniform1i(_blurredInputTextureUniformLocation, 1);
  glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT, nullptr);
  check_gl_error();

  glBindVertexArray(0);
  check_gl_error();
  glUseProgram(0);
  check_gl_error();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  check_gl_error();

  QImage img(width, height, QImage::Format_ARGB32);
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

void OpenGLWindow::drawSceneOpaqueToFramebuffer(GLuint framebuffer)
{
  glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
  check_gl_error();
  glDrawBuffer(GL_COLOR_ATTACHMENT0);
  check_gl_error();

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  check_gl_error();

  // clear the color, depth, and stencil buffer
  // the background default color is white
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  check_gl_error();

  _backgroundShader.paintGL();
  check_gl_error();

  glEnable(GL_DEPTH_TEST);

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _atomShader.paintGL(camera, _quality, _structureUniformBuffer);
    check_gl_error();

    _bondShader.paintGL(_structureUniformBuffer);
     check_gl_error();

    _objectShader.paintGL(_structureUniformBuffer);
    _unitCellShader.paintGL(_structureUniformBuffer);
    _localAxesShader.paintGL(_structureUniformBuffer);

    _boundingBoxShader.paintGL();
    _energySurfaceShader.paintGLOpaque(_structureUniformBuffer,_isosurfaceUniformBuffer);

  }
   glBindFramebuffer(GL_FRAMEBUFFER,0);
}




void OpenGLWindow::drawSceneVolumeRenderedSurfacesToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture)
{
  glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _energyVolumeRenderedSurface.paintGLOpaque(_structureUniformBuffer,_isosurfaceUniformBuffer, sceneResolvedDepthTexture);
  }

  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void OpenGLWindow::drawSceneTransparentToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture, int width, int height, qreal devicePixelRatio)
{
  glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _energyVolumeRenderedSurface.paintGLTransparent(_structureUniformBuffer,_isosurfaceUniformBuffer, sceneResolvedDepthTexture);

    _objectShader.paintGLTransparent(_structureUniformBuffer);

    _energySurfaceShader.paintGLTransparent(_structureUniformBuffer,_isosurfaceUniformBuffer);

    _selectionShader.paintGL(camera, _quality, _structureUniformBuffer);

    _textShader.paintGL(_structureUniformBuffer);

    _globalAxesShader.paintGL(width * devicePixelRatio, height * devicePixelRatio);
  }

  glViewport(0,0, width * devicePixelRatio, height * devicePixelRatio);
  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  check_gl_error();

  _selectionShader.paintGLGlow(_structureUniformBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER,0);
}

void OpenGLWindow::keyPressEvent( QKeyEvent* e )
{
  Q_UNUSED(e);

  makeCurrent();
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
  _timer->stop();

  _tracking = Tracking::none;

  makeCurrent();
  _startPoint = event->pos();
  _origin = event->pos();
  _draggedPos = event->pos();

  _quality = RKRenderQuality::medium;

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->setTrackBallRotation(simd_quatd(1.0, double3(0.0, 0.0, 0.0)));
  }


  if(event->modifiers() & Qt::ShiftModifier)   // contains shift
  {
    // Using the shift key means a new selection is chosen. If later a drag occurs it is modified to 'draggedNewSelection'
    _tracking = Tracking::newSelection;
  }
  else if(event->modifiers()  == Qt::ControlModifier)  // command, not option
  {
    // Using the command key means the selection is extended. If later a drag occurs it is modified to 'draggedAddToSelection'
    _tracking = Tracking::addToSelection;

  }
  else if(event->modifiers()  & Qt::AltModifier & Qt::ControlModifier)  // option and command
  {
    _tracking = Tracking::translateSelection;
  }
  else if(event->modifiers()  == Qt::AltModifier) // option, not command
  {
    _tracking = Tracking::measurement;
    #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
      _trackBall.start(event->pos().x(),event->pos().y(), 0, 0, this->width(), this->height());
    #else
      _trackBall.start(event->position().x(),event->position().y(), 0, 0, this->width(), this->height());
    #endif
  }
  else
  {
    _tracking = Tracking::backgroundClick;
    #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
      _trackBall.start(event->pos().x(),event->pos().y(), 0, 0, this->width(), this->height());
    #else
      _trackBall.start(event->position().x(),event->position().y(), 0, 0, this->width(), this->height());
    #endif
  }

  update();
}


void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
{
  makeCurrent();
  switch(_tracking)
  {
    case Tracking::none:
      break;
    case Tracking::newSelection:
      // convert to dragged version
      _tracking = Tracking::draggedNewSelection;
      _draggedPos = event->pos();
      break;
    case Tracking::addToSelection:
       // convert to dragged version
       _tracking = Tracking::draggedAddToSelection;
       _draggedPos = event->pos();
       break;
    case Tracking::draggedNewSelection:
      _draggedPos = event->pos();
       break;
    case Tracking::draggedAddToSelection:
      _draggedPos = event->pos();
      break;
    case Tracking::translateSelection:
       break;
    case Tracking::measurement:
       break;
    default:
       _tracking =Tracking::other;
       if(_startPoint)
       {
         #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
           simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->pos().x(), event->pos().y());
         #else
           simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->position().x(), event->position().y());
         #endif

         if (std::shared_ptr<RKCamera> camera = _camera.lock())
         {
           camera->setTrackBallRotation(trackBallRotation);
         }
       }
       break;
  }

  update();
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event)
{
  makeCurrent();

  switch(_tracking)
  {
    case Tracking::none:
      break;
    case Tracking::newSelection:
      break;
    case Tracking::addToSelection:
      break;
    case Tracking::draggedNewSelection:
      break;
    case Tracking::draggedAddToSelection:
      break;
    case Tracking::translateSelection:
      break;
    case Tracking::measurement:
      break;
    case Tracking::backgroundClick:
      break;
    default:
      #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->pos().x(), event->pos().y() );
      #else
        simd_quatd trackBallRotation = _trackBall.rollToTrackball(event->position().x(), event->position().y() );
      #endif

      if (std::shared_ptr<RKCamera> camera = _camera.lock())
      {
        simd_quatd worldRotation = camera->worldRotation();
        camera->setWorldRotation(trackBallRotation * worldRotation);
        camera->setTrackBallRotation(simd_quatd(1.0, double3(0.0, 0.0, 0.0)));
      }
      break;
  }

  _quality = RKRenderQuality::high;
  _tracking = Tracking::none;

  update();
}


void OpenGLWindow::wheelEvent(QWheelEvent *event)
{
  _quality = RKRenderQuality::medium;
  _timer->start(500);
  makeCurrent();
  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    camera->increaseDistance(event->angleDelta().y()/40.0);
  }
  update();
}

void OpenGLWindow::timeoutEventHandler()
{
  _timer->stop();
  _quality = RKRenderQuality::high;
  update();
}

void OpenGLWindow::initializeTransformUniforms()
{
  glGenBuffers(1, &_frameUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _frameUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, _frameUniformBuffer);
  check_gl_error();


  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
  check_gl_error();

  _energySurfaceShader.initializeTransformUniforms();
  _energyVolumeRenderedSurface.initializeTransformUniforms();
  check_gl_error();
  _boundingBoxShader.initializeTransformUniforms();
  check_gl_error();
  _globalAxesShader.initializeTransformUniforms();
  check_gl_error();
  _atomShader.initializeTransformUniforms();
  check_gl_error();
  _bondShader.initializeTransformUniforms();
  check_gl_error();

  _objectShader.initializeTransformUniforms();
  check_gl_error();
  _unitCellShader.initializeTransformUniforms();
  check_gl_error();
  _localAxesShader.initializeTransformUniforms();
  check_gl_error();
  _selectionShader.initializeTransformUniforms();
  check_gl_error();
  _pickingShader.initializeTransformUniforms();
  check_gl_error();
  _textShader.initializeTransformUniforms();
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  check_gl_error();
}

void OpenGLWindow::updateTransformUniforms()
{
  makeCurrent();
  double4x4 projectionMatrix = double4x4();
  double4x4 modelViewMatrix = double4x4();
  double4x4 modelMatrix = double4x4();
  double4x4 viewMatrix = double4x4();
  double4x4 axesProjectionMatrix = double4x4();
  double4x4 axesModelViewMatrix = double4x4();
  double bloomLevel = 1.0;;
  double bloomPulse = 1.0;
  bool isOrthographic = true;

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    projectionMatrix = camera->projectionMatrix();
    modelViewMatrix = camera->modelViewMatrix();
    modelMatrix = camera->modelMatrix();
    viewMatrix = camera->viewMatrix();
    isOrthographic = camera->isOrthographic();

    if(_dataSource)
    {
      double totalAxesSize = _dataSource->axes()->totalAxesSize();
      axesProjectionMatrix = camera->axesProjectionMatrix(totalAxesSize);
      axesModelViewMatrix = camera->axesModelViewMatrix();
    }

    bloomLevel = camera->bloomLevel();
    bloomPulse = camera->bloomPulse();
  }

  glBindBuffer(GL_UNIFORM_BUFFER, _frameUniformBuffer);
  check_gl_error();
  RKTransformationUniforms transformationUniforms = RKTransformationUniforms(projectionMatrix, modelViewMatrix, modelMatrix, viewMatrix, axesProjectionMatrix, axesModelViewMatrix, isOrthographic, bloomLevel, bloomPulse, _multiSampling);
  glBufferData (GL_UNIFORM_BUFFER, sizeof(RKTransformationUniforms), &transformationUniforms, GL_DYNAMIC_DRAW);
  check_gl_error();
  glBindBuffer(GL_UNIFORM_BUFFER,0);
}



void OpenGLWindow::initializeStructureUniforms()
{
  glGenBuffers(1, &_structureUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _structureUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, _structureUniformBuffer);
  check_gl_error();

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

  check_gl_error();
  std::vector<RKStructureUniforms> structureUniforms{RKStructureUniforms()};
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKStructureUniforms) * structureUniforms.size(), structureUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  check_gl_error();
}

void OpenGLWindow::updateStructureUniforms()
{
  makeCurrent();
  glBindBuffer(GL_UNIFORM_BUFFER, _structureUniformBuffer);

  std::vector<RKStructureUniforms> structureUniforms = std::vector<RKStructureUniforms>();

  for(uint i=0;i<_renderStructures.size();i++)
  {
    for(uint j=0;j<_renderStructures[i].size();j++)
    {
      RKStructureUniforms structureUniform = RKStructureUniforms(i, j, _renderStructures[i][j]);
      structureUniforms.push_back(structureUniform);
    }
  }

  if (structureUniforms.size() == 0)
  {
    structureUniforms.push_back(RKStructureUniforms());
  }


  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKStructureUniforms) * structureUniforms.size(), structureUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLWindow::initializeIsosurfaceUniforms()
{
  glGenBuffers(1, &_isosurfaceUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _isosurfaceUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 2, _isosurfaceUniformBuffer);

  _energySurfaceShader.initializeIsosurfaceUniforms();
  _energyVolumeRenderedSurface.initializeIsosurfaceUniforms();

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  check_gl_error();
}

void OpenGLWindow::updateIsosurfaceUniforms()
{
  makeCurrent();
  std::vector<RKIsosurfaceUniforms> isosurfaceUniforms;

  if(_dataSource)
  {
    for(uint i=0;i<_renderStructures.size();i++)
    {
      for(uint j=0;j<_renderStructures[i].size();j++)
      {
        RKIsosurfaceUniforms isosurfaceUniform = RKIsosurfaceUniforms(_renderStructures[i][j]);
        isosurfaceUniforms.push_back(isosurfaceUniform);
      }
    }
  }
  else
  {
    isosurfaceUniforms = {RKIsosurfaceUniforms()};
  }


  glBindBuffer(GL_UNIFORM_BUFFER, _isosurfaceUniformBuffer);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKIsosurfaceUniforms) * isosurfaceUniforms.size(), isosurfaceUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}



void OpenGLWindow::initializeLightUniforms()
{
  glGenBuffers(1, &_lightsUniformBuffer);
  check_gl_error();

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
  check_gl_error();
}



void OpenGLWindow::updateLightUniforms()
{
  makeCurrent();
  RKLightsUniforms lightUniforms = RKLightsUniforms(_dataSource);
  glBindBuffer(GL_UNIFORM_BUFFER, _lightsUniformBuffer);
  glBufferData(GL_UNIFORM_BUFFER, lightUniforms.lights.size() * sizeof(RKLightUniform), lightUniforms.lights.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void OpenGLWindow::initializeGlobalAxesUniforms()
{
  glGenBuffers(1, &_globalAxesUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _globalAxesUniformBuffer);
  check_gl_error();

  glBindBufferBase(GL_UNIFORM_BUFFER, 5, _globalAxesUniformBuffer);
  check_gl_error();

  _globalAxesShader.initializeGlobalAxesUniforms();
  check_gl_error();

  RKGlobalAxesUniforms uniformData = RKGlobalAxesUniforms(_dataSource);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKGlobalAxesUniforms), &uniformData, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  check_gl_error();
}

void OpenGLWindow::updateGlobalAxesUniforms()
{
  if(_dataSource)
  {
    makeCurrent();
    RKGlobalAxesUniforms uniformData = RKGlobalAxesUniforms(_dataSource);
    glBindBuffer(GL_UNIFORM_BUFFER, _globalAxesUniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RKGlobalAxesUniforms), &uniformData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }
}



void OpenGLWindow::reloadData()
{
  makeCurrent();

  qDebug() << "OpenGLWindow::reloadData";

  _energySurfaceShader.reloadData();
  _energyVolumeRenderedSurface.reloadData();
  _boundingBoxShader.reloadData();
  _globalAxesShader.reloadData();

  _atomShader.reloadData();
  _atomShader.reloadAmbientOcclusionData(_dataSource, RKRenderQuality::low);
  _bondShader.reloadData();
  _objectShader.reloadData();
  _unitCellShader.reloadData();
  _localAxesShader.reloadData();
  _selectionShader.reloadData();
  _pickingShader.reloadData();
  _textShader.reloadData();

  updateStructureUniforms();
  updateIsosurfaceUniforms();

  update();

  check_gl_error();
}

void OpenGLWindow::reloadData(RKRenderQuality quality)
{
  makeCurrent();

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
  _pickingShader.reloadData();
  _textShader.reloadData();

  update();

  check_gl_error();
}

void OpenGLWindow::reloadAmbientOcclusionData()
{
  makeCurrent();
  _atomShader.reloadAmbientOcclusionData(_dataSource, RKRenderQuality::low);
}

void OpenGLWindow::reloadRenderData()
{
  makeCurrent();

   _energySurfaceShader.reloadData();
   _energyVolumeRenderedSurface.reloadData();

  _atomShader.reloadData();
  _atomShader.reloadAmbientOcclusionData(_dataSource, RKRenderQuality:: low);
  _bondShader.reloadData();
  _objectShader.reloadData();
  _unitCellShader.reloadData();
  _localAxesShader.reloadData();
  _selectionShader.reloadData();
  _pickingShader.reloadData();
  _textShader.reloadData();

  update();
}

void OpenGLWindow::reloadRenderMeasurePointsData()
{
  makeCurrent();
}

void OpenGLWindow::reloadBoundingBoxData()
{
  _boundingBoxShader.initializeVertexArrayObject();
  makeCurrent();
}

void OpenGLWindow::reloadGlobalAxesData()
{
  makeCurrent();
}


void OpenGLWindow::reloadSelectionData()
{
  makeCurrent();

  _selectionShader.reloadData();
}

void OpenGLWindow::reloadBackgroundImage()
{
  makeCurrent();
  if(_dataSource)
  {
    _backgroundShader.reload(_dataSource);
  }
}

void OpenGLWindow::reloadStructureUniforms()
{
    makeCurrent();
    if(_dataSource)
    {
      updateStructureUniforms();
      updateIsosurfaceUniforms();
    }
}

void OpenGLWindow::updateVertexArrays()
{

}

void OpenGLWindow::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,OpenGLWindow::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,OpenGLWindow::_fragmentShaderSource.c_str());

  if (0 != vertexShader && 0 != fragmentShader)
  {
    _program = glCreateProgram();
    check_gl_error();

    glAttachShader(_program, vertexShader);
    check_gl_error();
    glAttachShader(_program, fragmentShader);
    check_gl_error();

    glBindFragDataLocation(_program, 0, "vFragColor");

    linkProgram(_program);

    _downSampleInputTextureUniformLocation = glGetUniformLocation(_program, "originalTexture");
    _blurredInputTextureUniformLocation = glGetUniformLocation(_program, "blurredTexture");
    _downSamplePositionAttributeLocation = glGetAttribLocation(_program, "position");
    _numberOfMultiSamplePointsUniformLocation = glGetUniformLocation(_program, "numberOfMultiSamplePoints");

    if (_downSampleInputTextureUniformLocation < 0) qDebug() << "Shader did not contain the 'originalTexture' uniform.";
    if (_downSamplePositionAttributeLocation < 0) qDebug() << "Shader did not contain the 'position' attribute.";

    glDeleteShader(vertexShader);
    check_gl_error();
    glDeleteShader(fragmentShader);
    check_gl_error();
  }
}

const std::string OpenGLWindow::_vertexShaderSource =
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

const std::string OpenGLWindow:: _fragmentShaderSource =
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
  vec4 sampleValue;

  // Calculate un-normalized texture coordinates
  vec2 tmp = floor(textureSize(originalTexture) * texcoord);

  // Find both the weighted and unweighted colors
  vec4 vColor = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < numberOfMultiSamplePoints ; i++)
  {
    sampleValue = texelFetch(originalTexture, ivec2(tmp), i);
    vColor += sampleValue;
  }

  vFragColor = vColor/float(numberOfMultiSamplePoints) + frameUniforms.bloomPulse * frameUniforms.bloomLevel * texture(blurredTexture,texcoord);
}
)foo");
