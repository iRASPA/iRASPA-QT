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
#if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
  #include <QOpenGLWidget>
#else
  #include <QGLWidget>
#endif
#include <QPoint>
#include <QCache>
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
#include "rkcamera.h"
#include "trackball.h"
#include "skopenclenergygridunitcell.h"
#include "openglbondselectionshader.h"
#include "openglboundingboxshader.h"
#include "openglpickingshader.h"
#include "openglobjectshader.h"
#include "openglselectionshader.h"
#include "opengltextrenderingshader.h"

#ifdef Q_OS_MACOS
  #include <OpenCL/opencl.h>
  #include <OpenGL/OpenGL.h>
#else
  #include <CL/opencl.h>
#endif

enum class Tracking
{
  none,
  panning,                   // alt + right-mouse drag
  trucking,                  // ctrl + right-mouse drag
  addToSelection,            // command-key
  newSelection,              // shift-key
  draggedAddToSelection,     // drag + command
  draggedNewSelection,       // drag + shift
  backgroundClick,
  measurement,               // alt-key
  translateSelection,        // alt and command-key
  other
};

#if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
  class GLWidget : public QOpenGLWidget, public RKRenderViewController, public OpenGLShader, public LogReportingConsumer
#else
  class GLWidget : public QGLWidget, public RKRenderViewController, public OpenGLShader, public LogReportingConsumer
#endif
{
  Q_OBJECT

public:
  GLWidget(QWidget* parent = nullptr );
  ~GLWidget();
  void redraw() override final;
  void redrawWithQuality(RKRenderQuality quality) override final;

  void setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures) override final;
  void setRenderDataSource(std::shared_ptr<RKRenderDataSource> source) override final;
  void reloadData() override final;
  void reloadData(RKRenderQuality ambientOcclusionQuality) override final;
  void reloadAmbientOcclusionData() override final;
  void reloadRenderData() override final;
  void reloadSelectionData() override final;
  void reloadRenderMeasurePointsData() override final;
  void reloadBoundingBoxData() override final;
  void reloadBackgroundImage() override final;

  void setLogReportingWidget(LogReporting *logReporting)  override final;

  void invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderStructure>> structures) override;
  void invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderStructure>> structures) override;

  void updateTransformUniforms() override final;
  void updateStructureUniforms() override final;
  void updateIsosurfaceUniforms() override final;
  void updateLightUniforms() override final;

  void updateVertexArrays() override final;

  void loadShader(void) override final;

  QImage renderSceneToImage(int width, int height, RKRenderQuality quality) override final;
  std::array<int,4> pickTexture(int x, int y, int width, int height) override final;

  void computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderStructure>> structures) override final;
  void computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderStructure>> structures) override final;

  void setControlPanel(bool iskeyAltOn);
  void updateControlPanel();
private:
  bool _isOpenGLInitialized;
  bool _isOpenCLInitialized;
  GLuint _program;
  RKRenderQuality _quality = RKRenderQuality::high;
  std::shared_ptr<RKRenderDataSource> _dataSource;
  LogReporting* _logReporter = nullptr;

  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> _renderStructures = std::vector<std::vector<std::shared_ptr<RKRenderStructure>>>{};
  GLuint _sceneFrameBuffer;
  GLuint _sceneDepthTexture;
  GLuint _sceneTexture;
  GLuint _glowSelectionTexture;
  GLuint _downSamplerVertexArray;
  GLuint _quad_vertexbuffer;
  GLuint _quad_indexbuffer;
  int _devicePixelRatio;
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
  GLsync _fence;

  std::weak_ptr<RKCamera> _camera;

  OpenGLBackgroundShader _backgroundShader;
  OpenGLBlurShader _blurShader;
  OpenGLEnergySurface _energySurfaceShader;

  OpenGLBoundingBoxShader _boundingBoxShader;

  OpenGLAtomShader _atomShader;
  OpenGLBondShader _bondShader;
  OpenGLObjectShader _objectShader;
  OpenGLUnitCellShader _unitCellShader;

  OpenGLSelectionShader _selectionShader;

  OpenGLPickingShader _pickingShader;

  OpenGLTextRenderingShader _textShader;

#if defined (Q_OS_OSX)
  void initializeCL_Mac(cl_context &_clContext, cl_device_id &_clDeviceId, cl_command_queue &_clCommandQueue);
#elif defined(Q_OS_WIN)
  void initializeCL_Windows(cl_context &_clContext, cl_device_id &_clDeviceId, cl_command_queue &_clCommandQueue);
#elif defined(Q_OS_LINUX)
  void initializeCL_Linux(cl_context &_clContext, cl_device_id &_clDeviceId, cl_command_queue &_clCommandQueue);
#endif
  std::optional<cl_device_id> bestOpenCLDevice(cl_device_type device_type);
  void printDeviceInformation(cl_device_id &clDeviceId);
  bool supportsImageFormatCapabilities(cl_context &trial_clContext, cl_device_id &trial_clDeviceId);

  void drawSceneToFramebuffer(GLuint framebuffer);

  void adjustAmbientOcclusionTextureSize();

  void initializeTransformUniforms();
  void initializeStructureUniforms();
  void initializeIsosurfaceUniforms();
  void initializeLightUniforms();

  TrackBall _trackBall = TrackBall();

  std::optional<QPoint> _startPoint = std::nullopt;
  std::optional<QPoint> _panStartPoint = std::nullopt;

  GLint _downSampleInputTextureUniformLocation;
  GLint _blurredInputTextureUniformLocation;
  GLint _downSamplePositionAttributeLocation;
  GLint _numberOfMultiSamplePointsUniformLocation;

  static const std::string _vertexShaderSource;
  static const std::string _fragmentShaderSource;

  Tracking _tracking;
  QPoint _origin;

  QStackedWidget *_controlPanel;
  QFrame *_controlPanelCartesian;
  QFrame *_controlPanelBodyFrame;
  QToolButton *_buttonCartesianMinusTx;
  QToolButton *_buttonCartesianPlusTx;
  QToolButton *_buttonCartesianMinusTy;
  QToolButton *_buttonCartesianPlusTy;
  QToolButton *_buttonCartesianMinusTz;
  QToolButton *_buttonCartesianPlusTz;
  QToolButton *_buttonCartesianMinusRx;
  QToolButton *_buttonCartesianPlusRx;
  QToolButton *_buttonCartesianMinusRy;
  QToolButton *_buttonCartesianPlusRy;
  QToolButton *_buttonCartesianMinusRz;
  QToolButton *_buttonCartesianPlusRz;

  QToolButton *_buttonBodyFrameMinusTx;
  QToolButton *_buttonBodyFramePlusTx;
  QToolButton *_buttonBodyFrameMinusTy;
  QToolButton *_buttonBodyFramePlusTy;
  QToolButton *_buttonBodyFrameMinusTz;
  QToolButton *_buttonBodyFramePlusTz;
  QToolButton *_buttonBodyFrameMinusRx;
  QToolButton *_buttonBodyFramePlusRx;
  QToolButton *_buttonBodyFrameMinusRy;
  QToolButton *_buttonBodyFramePlusRy;
  QToolButton *_buttonBodyFrameMinusRz;
  QToolButton *_buttonBodyFramePlusRz;
protected:
   void initializeGL() final override;
   void resizeGL( int w, int h ) final override;
   void paintGL() final override;
   void keyPressEvent( QKeyEvent* e ) final override;
   void mousePressEvent(QMouseEvent *event) final override;
   void mouseMoveEvent(QMouseEvent *event) final override;
   void mouseReleaseEvent(QMouseEvent *event) final override;
   void wheelEvent(QWheelEvent *event ) final override;
signals:
   void pressedTranslateCartesianMinusX();
   void pressedTranslateCartesianPlusX();
   void pressedTranslateCartesianMinusY();
   void pressedTranslateCartesianPlusY();
   void pressedTranslateCartesianMinusZ();
   void pressedTranslateCartesianPlusZ();
   void pressedRotateCartesianMinusX();
   void pressedRotateCartesianPlusX();
   void pressedRotateCartesianMinusY();
   void pressedRotateCartesianPlusY();
   void pressedRotateCartesianMinusZ();
   void pressedRotateCartesianPlusZ();

   void pressedTranslateBodyFrameMinusX();
   void pressedTranslateBodyFramePlusX();
   void pressedTranslateBodyFrameMinusY();
   void pressedTranslateBodyFramePlusY();
   void pressedTranslateBodyFrameMinusZ();
   void pressedTranslateBodyFramePlusZ();
   void pressedRotateBodyFrameMinusX();
   void pressedRotateBodyFramePlusX();
   void pressedRotateBodyFrameMinusY();
   void pressedRotateBodyFramePlusY();
   void pressedRotateBodyFrameMinusZ();
   void pressedRotateBodyFramePlusZ();
};
