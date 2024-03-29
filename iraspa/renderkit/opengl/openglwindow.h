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

#include <QObject>
#include <QtGlobal>
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>
#include <QOpenGLFunctions_3_3_Core>
#if (QT_VERSION < QT_VERSION_CHECK(5,4,0))
  #include <QGLWidget>
#elif (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  #include <QOpenGLWidget>
#else
  #if defined(Q_OS_WIN)
    #include <QtOpenGLWidgets/QOpenGLWidget>
  #else 
    #include <QOpenGLWidget>
  #endif
#endif
#include <QTimer>
#include <QPoint>
#include <QCache>
#include <QStringList>
#include <QToolButton>
#include <QFrame>
#include <QStackedWidget>
#include <QOpenGLTexture>
#include <foundationkit.h>
#include <optional>
#include "rkrenderkitprotocols.h"
#include "rkrenderuniforms.h"
#include "openglatomshader.h"
#include "openglbondshader.h"
#include "openglunitcellshader.h"
#include "openglatompickingshader.h"
#include "openglbackgroundshader.h"
#include "openglatomselectionshader.h"
#include "openglblurshader.h"
#include "openglenergysurface.h"
#include "openglenergyvolumerenderedsurface.h"
#include "rkcamera.h"
#include "trackball.h"
#include "openglbondselectionshader.h"
#include "openglboundingboxshader.h"
#include "openglpickingshader.h"
#include "openglobjectshader.h"
#include "openglselectionshader.h"
#include "opengltextrenderingshader.h"
#include "openglglobalaxesshader.h"
#include "opengllocalaxesshader.h"

#define CL_TARGET_OPENCL_VERSION 120
#ifdef Q_OS_MACOS
  #include <OpenCL/opencl.h>
  #include <OpenGL/OpenGL.h>
#else
  #include <CL/opencl.h>
#endif

enum class Tracking
{
  none = 0,
  panning = 1,                   // alt + right-mouse drag
  trucking = 2,                  // ctrl + right-mouse drag
  addToSelection = 3,            // command-key
  newSelection = 4,              // shift-key
  draggedAddToSelection = 5,     // drag + command
  draggedNewSelection = 6,       // drag + shift
  backgroundClick = 7,
  measurement = 8,               // alt-key
  translateSelection = 9,        // alt and command-key
  other = 10
};

class OpenGLWindow : public QOpenGLWindow, public RKRenderViewController, public OpenGLShader
{
  Q_OBJECT

public:
  OpenGLWindow(QWidget* parent = nullptr);
  ~OpenGLWindow();

  void redraw() override final;
  void redrawWithQuality(RKRenderQuality quality) override final;

  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderObject>>> structures) override final;
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source) override final;
  void reloadData() override final;
  void reloadData(RKRenderQuality ambientOcclusionQuality) override final;
  void reloadAmbientOcclusionData() override final;
  void reloadRenderData() override final;
  void reloadSelectionData() override final;
  void reloadRenderMeasurePointsData() override final;
  void reloadBoundingBoxData() override final;
  void reloadGlobalAxesData() override final;
  void reloadBackgroundImage() override final;
  void reloadStructureUniforms() override final;

  void invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderObject>> structures) override;
  void invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderObject>> structures) override;

  void updateTransformUniforms() override final;
  void updateStructureUniforms() override final;
  void updateIsosurfaceUniforms() override final;
  void updateLightUniforms() override final;
  void updateGlobalAxesUniforms() override final;

  void updateVertexArrays() override final;

  void loadShader(void) override final;

  QImage renderSceneToImage(int width, int height, RKRenderQuality quality) override final;
  std::array<int,4> pickTexture(int x, int y, int width, int height) override final;

  void setControlPanel(bool iskeyAltOn);
  void updateControlPanel();
  const QStringList& logData() const override final {return _logData;};
private:
  bool _isOpenGLInitialized;
  GLuint _program;
  RKRenderQuality _quality = RKRenderQuality::high;
  std::shared_ptr<RKRenderDataSource> _dataSource;

  QWidget* _parent;
  QStringList _logData{};
  QOpenGLDebugLogger *_logger{};
  void handleLoggedMessage(const QOpenGLDebugMessage &debugMessage);

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> _renderStructures = std::vector<std::vector<std::shared_ptr<RKRenderObject>>>{};
  GLuint _sceneFrameBuffer;
  GLuint _sceneDepthTexture;
  GLuint _sceneResolveDepthFrameBuffer;
  GLuint _sceneResolvedDepthTexture;
  GLuint _sceneTexture;
  GLuint _glowSelectionTexture;
  GLuint _downSamplerVertexArray;
  GLuint _quad_vertexbuffer;
  GLuint _quad_indexbuffer;
  qreal _devicePixelRatio;
  int _maxTextureSize;
  GLint _maxNumberOfClipPlanes;
  GLint _maxSampleCount;
  GLint _maxSampleColorCount;
  GLint _maxSampleDepthCount;
  GLint _maxMultiSampling;
  GLint _multiSampling;
  GLint _width;
  GLint _height;

  GLuint _frameUniformBuffer;
  GLuint _structureUniformBuffer;
  GLuint _isosurfaceUniformBuffer;
  GLuint _lightsUniformBuffer;
  GLuint _globalAxesUniformBuffer;
  GLsync _fence;

  std::weak_ptr<RKCamera> _camera;

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

  OpenGLPickingShader _pickingShader;

  OpenGLTextRenderingShader _textShader;

  void drawSceneOpaqueToFramebuffer(GLuint framebuffer);
  void drawSceneVolumeRenderedSurfacesToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture);
  void drawSceneTransparentToFramebuffer(GLuint framebuffer, GLuint sceneResolvedDepthTexture, int width, int height, qreal devicePixelRatio);

  void adjustAmbientOcclusionTextureSize();

  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeIsosurfaceUniforms();
  void initializeLightUniforms();
  void initializeGlobalAxesUniforms();

  TrackBall _trackBall = TrackBall();

  std::optional<QPoint> _startPoint = std::nullopt;
  std::optional<QPoint> _panStartPoint = std::nullopt;

  GLint _downSampleInputTextureUniformLocation;
  GLint _blurredInputTextureUniformLocation;
  GLint _downSamplePositionAttributeLocation;
  GLint _numberOfMultiSamplePointsUniformLocation;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;

  Tracking _tracking = Tracking::none;
  QPoint _origin;
  QPoint _draggedPos;

  QTimer *_timer;
  void timeoutEventHandler();
protected:
  virtual void initializeGL() final override;
  virtual void resizeGL( int w, int h ) final override;
  virtual void paintGL() final override;
  virtual void paintOverGL() final override;
  virtual void paintUnderGL() final override;
  void keyPressEvent( QKeyEvent* e ) final override;
  void mousePressEvent(QMouseEvent *event) final override;
  void mouseMoveEvent(QMouseEvent *event) final override;
  void mouseReleaseEvent(QMouseEvent *event) final override;
  void wheelEvent(QWheelEvent *event ) final override;
};
