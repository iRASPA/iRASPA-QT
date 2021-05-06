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

#include "glwidget.h"
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

#if defined(Q_OS_WIN)
  #include "wingdi.h"
#endif

#if defined(Q_OS_LINUX)
  #if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
    #include <QtPlatformHeaders/QGLXNativeContext>
  #endif
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5,4,0))
  GLWidget::GLWidget(QWidget* parent ): QOpenGLWidget(parent),
#else
  GLWidget::GLWidget(QWidget* parent ): QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
#endif
    _isOpenGLInitialized(false),
    _isOpenCLInitialized(false),
    _backgroundShader(),
    _blurShader(),
    _energySurfaceShader(),
    _boundingBoxShader(),
    _atomShader(),
    _bondShader(),
    _objectShader(),
    _unitCellShader(),
    _selectionShader(_atomShader, _bondShader, _objectShader),
    _pickingShader(_atomShader, _bondShader, _objectShader),
    _textShader(),
    _controlPanel(new QStackedWidget(this)),
    _controlPanelCartesian(new QFrame(this)),
    _controlPanelBodyFrame(new QFrame(this)),
    _timer(new QTimer(parent))
{
  setAttribute( Qt::WA_OpaquePaintEvent );
  setFocusPolicy(Qt::StrongFocus);

  connect(_timer, &QTimer::timeout, this, &GLWidget::timeoutEventHandler);

#if (QT_VERSION < QT_VERSION_CHECK(5,4,0))
  QGLFormat format;
  format.setSamples(1);
  format.setRgba(true);
  format.setDepth(false);
  format.setStencil(false);
  format.setVersion(3, 3);
  format.setProfile(QGLFormat::CoreProfile);
  setFormat(format);
#endif

  QGridLayout *layoutCartesian = new QGridLayout(this);
  layoutCartesian->setSpacing(4);
  _controlPanelCartesian->setLayout(layoutCartesian);

  _buttonCartesianMinusTx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusTx->setAutoRepeat(true);
  _buttonCartesianMinusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusTx->setText("-Tx");
  layoutCartesian->addWidget(_buttonCartesianMinusTx, 1, 0);
  QObject::connect(_buttonCartesianMinusTx, &QToolButton::clicked,this,&GLWidget::pressedTranslateCartesianMinusX);

  _buttonCartesianPlusTx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusTx->setAutoRepeat(true);
  _buttonCartesianPlusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusTx->setText("+Tx");
  layoutCartesian->addWidget(_buttonCartesianPlusTx, 1, 1);
  QObject::connect(_buttonCartesianPlusTx, &QToolButton::clicked,this,&GLWidget::pressedTranslateCartesianPlusX);

  _buttonCartesianMinusTy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusTy->setAutoRepeat(true);
  _buttonCartesianMinusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusTy->setText("-Ty");
  layoutCartesian->addWidget(_buttonCartesianMinusTy, 2, 0);
  QObject::connect(_buttonCartesianMinusTy, &QToolButton::clicked,this,&GLWidget::pressedTranslateCartesianMinusY);

  _buttonCartesianPlusTy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusTy->setAutoRepeat(true);
  _buttonCartesianPlusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusTy->setText("+Ty");
  layoutCartesian->addWidget(_buttonCartesianPlusTy, 2, 1);
  QObject::connect(_buttonCartesianPlusTy, &QToolButton::clicked,this,&GLWidget::pressedTranslateCartesianPlusY);

  _buttonCartesianMinusTz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusTz->setAutoRepeat(true);
  _buttonCartesianMinusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusTz->setText("-Tz");
  layoutCartesian->addWidget(_buttonCartesianMinusTz, 3, 0);
  QObject::connect(_buttonCartesianMinusTz, &QToolButton::clicked,this,&GLWidget::pressedTranslateCartesianMinusZ);

  _buttonCartesianPlusTz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusTz->setAutoRepeat(true);
  _buttonCartesianPlusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusTz->setText("+Tz");
  layoutCartesian->addWidget(_buttonCartesianPlusTz, 3, 1);
  QObject::connect(_buttonCartesianPlusTz, &QToolButton::clicked,this,&GLWidget::pressedTranslateCartesianPlusZ);



  _buttonCartesianMinusRx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusRx->setAutoRepeat(true);
  _buttonCartesianMinusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusRx->setText("-Rx");
  layoutCartesian->addWidget(_buttonCartesianMinusRx, 4, 0);
  QObject::connect(_buttonCartesianMinusRx, &QToolButton::clicked,this,&GLWidget::pressedRotateCartesianMinusX);

  _buttonCartesianPlusRx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusRx->setAutoRepeat(true);
  _buttonCartesianPlusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusRx->setText("+Rx");
  layoutCartesian->addWidget(_buttonCartesianPlusRx, 4, 1);
  QObject::connect(_buttonCartesianPlusRx, &QToolButton::clicked,this,&GLWidget::pressedRotateCartesianPlusX);

  _buttonCartesianMinusRy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusRy->setAutoRepeat(true);
  _buttonCartesianMinusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusRy->setText("-Ry");
  layoutCartesian->addWidget(_buttonCartesianMinusRy, 5, 0);
  QObject::connect(_buttonCartesianMinusRy, &QToolButton::clicked,this,&GLWidget::pressedRotateCartesianMinusY);

  _buttonCartesianPlusRy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusRy->setAutoRepeat(true);
  _buttonCartesianPlusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusRy->setText("+Ry");
  layoutCartesian->addWidget(_buttonCartesianPlusRy, 5, 1);
  QObject::connect(_buttonCartesianPlusRy, &QToolButton::clicked,this,&GLWidget::pressedRotateCartesianPlusY);

  _buttonCartesianMinusRz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusRz->setAutoRepeat(true);
  _buttonCartesianMinusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusRz->setText("-Rz");
  layoutCartesian->addWidget(_buttonCartesianMinusRz, 6, 0);
  QObject::connect(_buttonCartesianMinusRz, &QToolButton::clicked,this,&GLWidget::pressedRotateCartesianMinusZ);

  _buttonCartesianPlusRz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusRz->setAutoRepeat(true);
  _buttonCartesianPlusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusRz->setText("+Rz");
  layoutCartesian->addWidget(_buttonCartesianPlusRz, 6, 1);
  QObject::connect(_buttonCartesianPlusRz, &QToolButton::clicked,this,&GLWidget::pressedRotateCartesianPlusZ);

  _controlPanel->addWidget(_controlPanelCartesian);


  QGridLayout *layoutBodyFrame = new QGridLayout(this);
  layoutBodyFrame->setSpacing(4);
  _controlPanelBodyFrame->setLayout(layoutBodyFrame);

  _buttonBodyFrameMinusTx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusTx->setAutoRepeat(true);
  _buttonBodyFrameMinusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusTx->setText("-T1");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusTx, 1, 0);
  QObject::connect(_buttonBodyFrameMinusTx, &QToolButton::clicked,this,&GLWidget::pressedTranslateBodyFrameMinusX);

  _buttonBodyFramePlusTx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusTx->setAutoRepeat(true);
  _buttonBodyFramePlusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusTx->setText("+T1");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusTx, 1, 1);
  QObject::connect(_buttonBodyFramePlusTx, &QToolButton::clicked,this,&GLWidget::pressedTranslateBodyFramePlusX);

  _buttonBodyFrameMinusTy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusTy->setAutoRepeat(true);
  _buttonBodyFrameMinusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusTy->setText("-T2");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusTy, 2, 0);
  QObject::connect(_buttonBodyFrameMinusTy, &QToolButton::clicked,this,&GLWidget::pressedTranslateBodyFrameMinusY);

  _buttonBodyFramePlusTy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusTy->setAutoRepeat(true);
  _buttonBodyFramePlusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusTy->setText("+T2");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusTy, 2, 1);
  QObject::connect(_buttonBodyFramePlusTy, &QToolButton::clicked,this,&GLWidget::pressedTranslateBodyFramePlusY);

  _buttonBodyFrameMinusTz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusTz->setAutoRepeat(true);
  _buttonBodyFrameMinusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusTz->setText("-T3");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusTz, 3, 0);
  QObject::connect(_buttonBodyFrameMinusTz, &QToolButton::clicked,this,&GLWidget::pressedTranslateBodyFrameMinusZ);

  _buttonBodyFramePlusTz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusTz->setAutoRepeat(true);
  _buttonBodyFramePlusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusTz->setText("+T3");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusTz, 3, 1);
  QObject::connect(_buttonBodyFramePlusTz, &QToolButton::clicked,this,&GLWidget::pressedTranslateBodyFramePlusZ);



  _buttonBodyFrameMinusRx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusRx->setAutoRepeat(true);
  _buttonBodyFrameMinusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusRx->setText("-R1");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusRx, 4, 0);
  QObject::connect(_buttonBodyFrameMinusRx, &QToolButton::clicked,this,&GLWidget::pressedRotateBodyFrameMinusX);

  _buttonBodyFramePlusRx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusRx->setAutoRepeat(true);
  _buttonBodyFramePlusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusRx->setText("+R1");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusRx, 4, 1);
  QObject::connect(_buttonBodyFramePlusRx, &QToolButton::clicked,this,&GLWidget::pressedRotateBodyFramePlusX);

  _buttonBodyFrameMinusRy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusRy->setAutoRepeat(true);
  _buttonBodyFrameMinusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusRy->setText("-R2");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusRy, 5, 0);
  QObject::connect(_buttonBodyFrameMinusRy, &QToolButton::clicked,this,&GLWidget::pressedRotateBodyFrameMinusY);

  _buttonBodyFramePlusRy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusRy->setAutoRepeat(true);
  _buttonBodyFramePlusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusRy->setText("+R2");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusRy, 5, 1);
  QObject::connect(_buttonBodyFramePlusRy, &QToolButton::clicked,this,&GLWidget::pressedRotateBodyFramePlusY);

  _buttonBodyFrameMinusRz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusRz->setAutoRepeat(true);
  _buttonBodyFrameMinusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusRz->setText("-R3");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusRz, 6, 0);
  QObject::connect(_buttonBodyFrameMinusRz, &QToolButton::clicked,this,&GLWidget::pressedRotateBodyFrameMinusZ);

  _buttonBodyFramePlusRz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusRz->setAutoRepeat(true);
  _buttonBodyFramePlusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusRz->setText("+R3");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusRz, 6, 1);
  QObject::connect(_buttonBodyFramePlusRz, &QToolButton::clicked,this,&GLWidget::pressedRotateBodyFramePlusZ);

  _controlPanel->addWidget(_controlPanelBodyFrame);

  _controlPanel->setHidden(true);
}

void GLWidget::setControlPanel(bool iskeyAltOn)
{
  if(iskeyAltOn)
    _controlPanel->setCurrentIndex(1);
  else
    _controlPanel->setCurrentIndex(0);
}

void GLWidget::updateControlPanel()
{
  for(std::vector<std::shared_ptr<RKRenderStructure>> renderStructureVector : _renderStructures)
  {
    for(std::shared_ptr<RKRenderStructure> structure : renderStructureVector)
    {
      if(structure->hasSelectedAtoms())
      {
        _controlPanel->setHidden(false);
        return;
      }
    }
  }
  _controlPanel->setHidden(true);
}

GLWidget::~GLWidget()
{
  _bondShader.deletePermanentBuffers();
}

void GLWidget::setLogReportingWidget(LogReporting *logReporting)
{
  _logReporter = logReporting;

  _energySurfaceShader.setLogReportingWidget(logReporting);

  if(!_logData.isEmpty())
  {
    _logReporter->insert(_logData);
  }
}

void GLWidget::setRenderDataSource(std::shared_ptr<RKRenderDataSource> source)
{
  makeCurrent();

  _dataSource = source;
  _boundingBoxShader.setRenderDataSource(source);
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

void GLWidget::setRenderStructures(std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> structures)
{
  makeCurrent();
  _renderStructures = structures;

  _energySurfaceShader.setRenderStructures(structures);

  _atomShader.setRenderStructures(structures);
  _bondShader.setRenderStructures(structures);
   _objectShader.setRenderStructures(structures);
  _unitCellShader.setRenderStructures(structures);

  _selectionShader.setRenderStructures(structures);
  _pickingShader.setRenderStructures(structures);

  _textShader.setRenderStructures(structures);
}


void GLWidget::redraw()
{
  update();
}

void GLWidget::redrawWithQuality(RKRenderQuality quality)
{
  _quality = quality;
  update();
}




void GLWidget::invalidateCachedAmbientOcclusionTextures(std::vector<std::shared_ptr<RKRenderStructure>> structures)
{
  makeCurrent();
  _atomShader.invalidateCachedAmbientOcclusionTexture(structures);
}

void GLWidget::invalidateCachedIsosurfaces(std::vector<std::shared_ptr<RKRenderStructure>> structures)
{
  makeCurrent();
  _energySurfaceShader.invalidateIsosurface(structures);
}

std::array<int,4> GLWidget::pickTexture(int x, int y, int width, int height)
{
  makeCurrent();
  return _pickingShader.pickTexture(x,y,width,height);
}

void GLWidget::initializeGL()
{
  cl_context _clContext = nullptr;
  cl_device_id _clDeviceId = nullptr;
  cl_command_queue _clCommandQueue = nullptr;

  if(!context())
  {
    QMessageBox messageBox;
    messageBox.setFixedSize(650, 200);
    messageBox.critical(nullptr, "Critical error", "No OpenGL context present, install OpenGL drivers (>=3.3)");
    QApplication::quit();
  }

  if (!initializeOpenGLFunctions()) 
	  qFatal("Failed to initialize OpenGL functions");

  _backgroundShader.initializeOpenGLFunctions();
  _blurShader.initializeOpenGLFunctions();
  _energySurfaceShader.initializeOpenGLFunctions();
  _boundingBoxShader.initializeOpenGLFunctions();

  _atomShader.initializeOpenGLFunctions();
  _bondShader.initializeOpenGLFunctions();
  _objectShader.initializeOpenGLFunctions();
  _unitCellShader.initializeOpenGLFunctions();

  _selectionShader.initializeOpenGLFunctions();
  _pickingShader.initializeEmbeddedOpenGLFunctions();

  _textShader.initializeOpenGLFunctions();

  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenGL initialized");
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenGL initialized"));
  }


  GLint majorVersion = 0;
  GLint minorVersion = 0;
  GLint profile = 0;
  GLint flags = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
  glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
  glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenGL major version: " + QString::number(majorVersion));
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenGL minor version: " + QString::number(minorVersion));
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                    + "OpenGL major version: " + QString::number(majorVersion)));
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                    + "OpenGL major version: " + QString::number(minorVersion)));
  }

  if(majorVersion <= 3 && minorVersion < 3)
  {
    QMessageBox messageBox;
    messageBox.setFixedSize(650, 200);
    messageBox.critical(nullptr, "Critical error", "OpenGL version error, install OpenGL drivers (>=3.3)");
    QApplication::quit();
  }

  if(profile & GL_CONTEXT_CORE_PROFILE_BIT)
  {
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenGL core profile");
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                        + "OpenGL core profile"));
    }
  }

  // get OpenGL capabilities
  glGetIntegerv(GL_MAX_CLIP_DISTANCES, &_maxNumberOfClipPlanes);
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);

  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "max texture size: " + QString::number(_maxTextureSize));
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                        + "max texture size: " + QString::number(_maxTextureSize)));
  }

  glGetIntegerv(GLenum(GL_MAX_SAMPLES), &_maxSampleCount);
  glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &_maxSampleColorCount);
  glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &_maxSampleDepthCount);
  _maxMultiSampling = std::min(_maxSampleCount, std::min(_maxSampleColorCount, _maxSampleDepthCount));
  _multiSampling = std::max(1,std::min(8, std::min(_maxSampleCount, std::min(_maxSampleColorCount, _maxSampleDepthCount))));

  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "multisampling: " + QString::number(_multiSampling));
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                          + "multisampling: " + QString::number(_multiSampling)));
  }

  _devicePixelRatio = QPaintDevice:: devicePixelRatio();
  _width = std::max(512, this->width());
  _height = std::max(512, this->height());

  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "pixel device ratio: " + QString::number(_devicePixelRatio));
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                            + "pixel device ratio: " + QString::number(_devicePixelRatio)));
  }

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


  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  check_gl_error();

  glActiveTexture(GL_TEXTURE0);
  check_gl_error();

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _glowSelectionTexture);
  check_gl_error();

  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glActiveTexture(GL_TEXTURE1);
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

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  _blurShader.initializeFramebuffers();

  _pickingShader.generateFrameBuffers();

  cl_int err;
  cl_uint n;
  err = clGetPlatformIDs(0, nullptr, &n);
  QString platformNumberMessage = QString("Number of OpenCL platforms found: %1").arg(QString::number(n));
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, platformNumberMessage);
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + platformNumberMessage));
  }
  if (n > 0)
  {
    QVector<cl_platform_id> platformIds;
    platformIds.resize(n);
    if (clGetPlatformIDs(n, platformIds.data(), 0) == CL_SUCCESS)
    {
      for (cl_uint i = 0; i < n; ++i)
      {
        QByteArray name;
        name.resize(1024);
        clGetPlatformInfo(platformIds[i], CL_PLATFORM_NAME, name.size(), name.data(), 0);
        QString platformNameMessage = QString("platform found: %1").arg(QString(name.constData()));
        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, platformNameMessage);
        }
        else
        {
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + platformNameMessage));
        }
      }
    }

    #if defined (Q_OS_OSX)
      initializeCL_Mac(_clContext, _clDeviceId, _clCommandQueue);
    #elif defined(Q_OS_WIN)
      initializeCL_Windows(_clContext, _clDeviceId, _clCommandQueue);
    #elif defined(Q_OS_LINUX)
      initializeCL_Linux(_clContext, _clDeviceId, _clCommandQueue);
    #endif
  }

  _energySurfaceShader.initializeOpenCL(_isOpenCLInitialized, _clContext, _clDeviceId, _clCommandQueue, _logData);

  // Set the clear color to white
  glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

  this->loadShader();
  _backgroundShader.loadShader();
  _blurShader.loadShader();
  _energySurfaceShader.loadShader();
  _boundingBoxShader.loadShader();
  _atomShader.loadShader();
  _bondShader.loadShader();
  _objectShader.loadShader();
  _unitCellShader.loadShader();
  _selectionShader.loadShader();
  _pickingShader.loadShader();
  _textShader.loadShader();

  _energySurfaceShader.generateBuffers();
  _boundingBoxShader.generateBuffers();
  _unitCellShader.generateBuffers();
  _bondShader.generatePermanentBuffers();
  _textShader.generateTextures();

  _backgroundShader.initializeVertexArrayObject();
  _energySurfaceShader.initializeVertexArrayObject();
  _blurShader.initializeVertexArrayObject();
  _boundingBoxShader.initializeVertexArrayObject();
  _atomShader.initializeVertexArrayObject();
  _bondShader.initializeVertexArrayObject();
  _objectShader.initializeVertexArrayObject();
  _unitCellShader.initializeVertexArrayObject();
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

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture);
  check_gl_error();

  glBindVertexArray(0);
  check_gl_error();

  _isOpenGLInitialized = true;
}

#if defined (Q_OS_OSX)
void GLWidget::initializeCL_Mac(cl_context &_clContext, cl_device_id &_clDeviceId, cl_command_queue &_clCommandQueue)
{
  cl_int err;

  // Set up the context with OpenCL/OpenGL interop.
  CGLContextObj cgl_current_context = CGLGetCurrentContext();
  CGLShareGroupObj cgl_share_group = CGLGetShareGroup(cgl_current_context);
  cl_context_properties properties[] = { CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
                                         cl_context_properties(cgl_share_group),
                                         0 };

  _clContext = clCreateContext(properties, 0, nullptr, nullptr, nullptr, &err);
  if (err == CL_SUCCESS)
  {
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "Created OpenCL context");
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "Created OpenCL context"));
    }

    if (clGetGLContextInfoAPPLE(_clContext, static_cast<void *>(CGLGetCurrentContext()),
                                 CL_CGL_DEVICE_FOR_CURRENT_VIRTUAL_SCREEN_APPLE,
                                 sizeof(cl_device_id), &_clDeviceId, nullptr) == CL_SUCCESS)
    {
      _clCommandQueue = clCreateCommandQueue(_clContext, _clDeviceId, 0, &err);
      if(err==CL_SUCCESS)
      {
        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL initialized");
        }
        else
        {
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL initialized"));
        }
        printDeviceInformation(_clDeviceId);
        _isOpenCLInitialized = true;
      }
      else
      {
        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::error, "OpenCL command-queue creation failed");
        }
        else
        {
          _logData.append(QString("<font color=\"Red\">error (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                                            + "OpenCL command-queue creation failed"));
        }
      }
    }
    else
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, "Failed to get OpenCL device for current screen, error code: " + QString::number(err));
      }
      else
      {
        _logData.append(QString("<font color=\"Red\">error (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                                            + "Failed to get OpenCL device for current screen, error code: " + QString::number(err)));
      }
    }
  }
  else
  {
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, "Failed to create OpenCL context, error code: " + QString::number(err));
    }
    else
    {
      _logData.append(QString("<font color=\"Red\">error (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>"
                                            + "Failed to create OpenCL context, error code: " + QString::number(err)));
    }
  }
}
#endif

#if defined(Q_OS_WIN)
void GLWidget::initializeCL_Windows(cl_context &_clContext, cl_device_id &_clDeviceId, cl_command_queue &_clCommandQueue)
{
  cl_int err;

  // do not use OpenGL/OpenCL for linux
  _energySurfaceShader.setGLInteroperability(false);

  // see first if there is a suitable GPU device for OpenCL
  std::optional<cl_device_id> bestGPUDevice = bestOpenCLDevice(CL_DEVICE_TYPE_GPU);

  if(bestGPUDevice)
  {
    cl_device_id cllDeviceId = *bestGPUDevice;

    cl_context context = clCreateContext(nullptr, 1, &cllDeviceId, nullptr, nullptr, &err);
    if(err == CL_SUCCESS)
    {
      cl_command_queue clCommandQueue = clCreateCommandQueue(context, cllDeviceId, 0, &err);
      if(err == CL_SUCCESS)
      {
        _clContext = context;
        _clDeviceId = cllDeviceId;
        _clCommandQueue = clCommandQueue;
        _isOpenCLInitialized = true;

        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL device context created");
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL command-queue created");
        }
        else
        {
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL device context created"));
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL command-queue created"));
        }
        printDeviceInformation(cllDeviceId);
        return;
      }
      clReleaseContext(context);
    }
  }

  // see next if there is a suitable CPU device for OpenCL if no GPU is available
  std::optional<cl_device_id> bestCPUDevice = bestOpenCLDevice(CL_DEVICE_TYPE_CPU);

  if(bestCPUDevice)
  {
    cl_device_id cllDeviceId = *bestCPUDevice;

    cl_context context = clCreateContext(nullptr, 1, &cllDeviceId, nullptr, nullptr, &err);
    if(err == CL_SUCCESS)
    {
      cl_command_queue clCommandQueue = clCreateCommandQueue(context, cllDeviceId, 0, &err);
      if(err == CL_SUCCESS)
      {
        _clContext = context;
        _clDeviceId = cllDeviceId;
        _clCommandQueue = clCommandQueue;
        _isOpenCLInitialized = true;

        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL device context created");
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL command-queue created");
        }
        else
        {
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL device context created"));
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL command-queue created"));
        }
        printDeviceInformation(cllDeviceId);
        return;
      }
      clReleaseContext(context);
    }
  }
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::info, "OpenCL no usable devices found");
  }
  else
  {
    _logData.append(QString("<font color=\"ForestGreen\">info (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL no usable devices found"));
  }
}
#endif

#if defined(Q_OS_LINUX)
void GLWidget::initializeCL_Linux(cl_context &_clContext, cl_device_id &_clDeviceId, cl_command_queue &_clCommandQueue)
{
  cl_int err;

  // do not use OpenGL/OpenCL for linux
  _energySurfaceShader.setGLInteroperability(false);

  // see first if there is a suitable GPU device for OpenCL
  std::optional<cl_device_id> bestGPUDevice = bestOpenCLDevice(CL_DEVICE_TYPE_GPU);

  if(bestGPUDevice)
  {
    cl_device_id cllDeviceId = *bestGPUDevice;

    cl_context context = clCreateContext(nullptr, 1, &cllDeviceId, nullptr, nullptr, &err);
    if(err == CL_SUCCESS)
    {
      cl_command_queue clCommandQueue = clCreateCommandQueue(context, cllDeviceId, 0, &err);
      if(err == CL_SUCCESS)
      {
        _clContext = context;
        _clDeviceId = cllDeviceId;
        _clCommandQueue = clCommandQueue;
        _isOpenCLInitialized = true;

        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL device context created");
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL command-queue created");
        }
        else
        {
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL device context created"));
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL command-queue created"));
        }
        printDeviceInformation(cllDeviceId);
        return;
      }
      clReleaseContext(context);
    }
  }

  // see next if there is a suitable CPU device for OpenCL if no GPU is available
  std::optional<cl_device_id> bestCPUDevice = bestOpenCLDevice(CL_DEVICE_TYPE_CPU);

  if(bestCPUDevice)
  {
    cl_device_id cllDeviceId = *bestCPUDevice;

    cl_context context = clCreateContext(nullptr, 1, &cllDeviceId, nullptr, nullptr, &err);
    if(err == CL_SUCCESS)
    {
      cl_command_queue clCommandQueue = clCreateCommandQueue(context, cllDeviceId, 0, &err);
      if(err == CL_SUCCESS)
      {
        _clContext = context;
        _clDeviceId = cllDeviceId;
        _clCommandQueue = clCommandQueue;
        _isOpenCLInitialized = true;

        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL device context created");
          _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "OpenCL command-queue created");
        }
        else
        {
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL device context created"));
          _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL command-queue created"));
        }
        printDeviceInformation(cllDeviceId);
        return;
      }
      clReleaseContext(context);
    }
  }

  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::info, "OpenCL no usable devices found");
  }
  else
  {
    _logData.append(QString("<font color=\"ForestGreen\">info (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + "OpenCL no usable devices found"));
  }
}
#endif

void GLWidget::printDeviceInformation(cl_device_id &clDeviceId)
{
  cl_int err;
  size_t valueSize = 0;

  // print device name
  clGetDeviceInfo(clDeviceId, CL_DEVICE_NAME, 0, nullptr, &valueSize);
  QByteArray deviceName;
  deviceName.resize(valueSize);
  err = clGetDeviceInfo(clDeviceId, CL_DEVICE_NAME, deviceName.size(), deviceName.data(), nullptr);
  if(err == CL_SUCCESS)
  {
    QString deviceNameMessage = QString("OpenCL device name: %1").arg(QString(deviceName));
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, deviceNameMessage);
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + deviceNameMessage));
    }
  }

  // print hardware device version
  clGetDeviceInfo(clDeviceId, CL_DEVICE_VERSION, 0, nullptr, &valueSize);
  QByteArray deviceVersion;
  deviceVersion.resize(valueSize);
  err = clGetDeviceInfo(clDeviceId, CL_DEVICE_VERSION, deviceVersion.size(), deviceVersion.data(), nullptr);
  if(err == CL_SUCCESS)
  {
    QString deviceVersionMessage = QString("OpenCL device version: %1").arg(QString(deviceVersion));
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, deviceVersionMessage);
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + deviceVersionMessage));
    }
  }

  // print software driver version
  clGetDeviceInfo(clDeviceId, CL_DRIVER_VERSION, 0, nullptr, &valueSize);
  QByteArray deviceDriverVersion;
  deviceDriverVersion.resize(valueSize);
  err = clGetDeviceInfo(clDeviceId, CL_DRIVER_VERSION, deviceDriverVersion.size(), deviceDriverVersion.data(), nullptr);
  if(err == CL_SUCCESS)
  {
    QString deviceDriverVersionMessage = QString("OpenCL device driver version: %1").arg(QString(deviceDriverVersion));
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, deviceDriverVersionMessage);
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + deviceDriverVersionMessage));
    }
  }

  // print c version supported by compiler for device
  clGetDeviceInfo(clDeviceId, CL_DEVICE_OPENCL_C_VERSION, 0, nullptr, &valueSize);
  QByteArray deviceOpenCLVersion;
  deviceOpenCLVersion.resize(valueSize);
  err = clGetDeviceInfo(clDeviceId, CL_DEVICE_OPENCL_C_VERSION, deviceOpenCLVersion.size(), deviceOpenCLVersion.data(), nullptr);
  if(err == CL_SUCCESS)
  {
    QString deviceOpenCLVersionMessage = QString("OpenCL OpenCL version: %1").arg(QString(deviceOpenCLVersion));
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, deviceOpenCLVersionMessage);
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + deviceOpenCLVersionMessage));
    }
  }

  // print parallel compute units
  cl_uint maxComputeUnits;
  err = clGetDeviceInfo(clDeviceId, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, nullptr);
  if(err == CL_SUCCESS)
  {
    QString deviceMaxComputeUnitsMessage = QString("OpenCL max compute units: %1").arg(QString::number(maxComputeUnits));
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::verbose, deviceMaxComputeUnitsMessage);
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + deviceMaxComputeUnitsMessage));
    }
  }

  // check image support
  cl_bool image_support = false;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, nullptr);
  QString imageSupportString = image_support ? "true" : "false";
  QString imageSupportMessage = QString("OpenCL image support: %1").arg(imageSupportString);

  if(!image_support)
  {
    if(_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::error, imageSupportMessage);
    }
    else
    {
      _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + imageSupportMessage));
    }
    return;
  }
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, imageSupportMessage);
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + imageSupportMessage));
  }

  cl_uint imageMaxRead = 0;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_MAX_READ_IMAGE_ARGS, sizeof(imageMaxRead), &imageMaxRead, nullptr);
  QString imageMaxReadMessage = QString("OpenCL image max read: %1").arg(QString::number(imageMaxRead));
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, imageMaxReadMessage);
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + imageMaxReadMessage));
  }

  cl_uint imageMaxWrite = 0;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_MAX_WRITE_IMAGE_ARGS, sizeof(image_support), &imageMaxWrite, nullptr);
  QString imageMaxWriteMessage = QString("OpenCL image max write: %1").arg(QString::number(imageMaxWrite));
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, imageMaxWriteMessage);
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + imageMaxWriteMessage));
  }

  size_t image3dMaxWidth = 0;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(image3dMaxWidth), &image3dMaxWidth, nullptr);
  QString image3dMaxWidthMessage = QString("OpenCL image3D max width: %1").arg(QString::number(image3dMaxWidth));
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, image3dMaxWidthMessage);
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + image3dMaxWidthMessage));
  }

  size_t image3dMaxHeight = 0;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(image3dMaxHeight), &image3dMaxHeight, nullptr);
  QString image3dMaxHeightMessage = QString("OpenCL image3D max height: %1").arg(QString::number(image3dMaxWidth));
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, image3dMaxHeightMessage);
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + image3dMaxHeightMessage));
  }

  size_t image3dMaxDepth = 0;
  clGetDeviceInfo(clDeviceId, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(image3dMaxDepth), &image3dMaxDepth, nullptr);
  QString image3dMaxDepthMessage = QString("OpenCL image3D max depth: %1").arg(QString::number(image3dMaxDepth));
  if(_logReporter)
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, image3dMaxDepthMessage);
  }
  else
  {
    _logData.append(QString("<font color=\"Magenta\">verbose (" + QDateTime::currentDateTime().toString("hh:mm:ss") +  "): </font>" + image3dMaxDepthMessage));
  }
}

std::optional<cl_device_id> GLWidget::bestOpenCLDevice(cl_device_type device_type)
{
  cl_int err;

  // get the number of platforms
  cl_uint platformCount;
  err = clGetPlatformIDs(0, nullptr, &platformCount);

  if(platformCount<=0)
  {
    return std::nullopt;
  }

  // get platform ids
  QVector<cl_platform_id> platforms;
  platforms.resize(platformCount);
  err = clGetPlatformIDs(platforms.size(), platforms.data(), nullptr);
  if(err != CL_SUCCESS)
  {
    return std::nullopt;
  }

  std::optional<std::pair<cl_device_id, cl_uint>> bestDevice = std::nullopt;

  // loop over all platforms and devices of type 'device_type'
  for (cl_uint i = 0; i < platformCount; ++i)
  {
        // get the number of devices of type 'device_type' in the platform
    cl_uint deviceCount;
    err = clGetDeviceIDs(platforms[i], device_type, 0, nullptr, &deviceCount);
    if(err != CL_SUCCESS)
    {
      continue;
    }

    // get the IDs of GPU devices
    QVector<cl_device_id> devices;
    devices.resize(deviceCount);
    err = clGetDeviceIDs(platforms[i], device_type, devices.size(), devices.data(), nullptr);
    if(err != CL_SUCCESS)
    {
      continue;
    }

    // loop over all devices
    for(cl_uint j = 0; j < deviceCount; j++)
    {
      cl_uint maxComputeUnits;
      err = clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
      if(err != CL_SUCCESS)
      {
        continue;
      }

      cl_context context = clCreateContext(nullptr, 1, &devices[j], nullptr, nullptr, &err);

      if(err != CL_SUCCESS)
      {
        continue;
      }

      if(supportsImageFormatCapabilities(context, devices[j]))
      {
        cl_command_queue clCommandQueue = clCreateCommandQueue(context, devices[j], 0, &err);
        if(err != CL_SUCCESS)
        {
          clReleaseContext(context);
          continue;
        }
        clReleaseCommandQueue(clCommandQueue);

        if(bestDevice)
        {
          // use the one with the highest compute units
          if(maxComputeUnits > std::get<1>(*bestDevice))
          {
            bestDevice = std::make_pair(devices[j], maxComputeUnits);
          }
        }
        else
        {
          bestDevice = std::make_pair(devices[j], maxComputeUnits);
        }
      }
      clReleaseContext(context);
    }
  }

  if(bestDevice)
  {
    // we have a suitable device that can be used
    return std::get<0>(*bestDevice);
  }

  return std::nullopt;
}

bool GLWidget::supportsImageFormatCapabilities(cl_context &trial_clContext, cl_device_id &trial_clDeviceId)
{
  cl_int err;

  // check image support
  cl_bool image_support = false;
  clGetDeviceInfo(trial_clDeviceId, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, nullptr);

  if(!image_support)
  {
    return false;
  }

  // check the needed image formats
  cl_image_format imageFormat_RGBA_INT8{CL_RGBA,CL_UNSIGNED_INT8};
  cl_image_desc imageDescriptor_RGBA_INT8{CL_MEM_OBJECT_IMAGE3D, 256, 256, 256, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_RGBA_INT8 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_RGBA_INT8, &imageDescriptor_RGBA_INT8, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_RGBA_INT8);

  cl_image_format imageFormat_R_INT8{CL_R,CL_UNSIGNED_INT8};
  cl_image_desc imageDescriptor_R_INT8{CL_MEM_OBJECT_IMAGE3D, 256, 256, 256, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_INT8 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_INT8, &imageDescriptor_R_INT8, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_INT8);

  cl_image_format imageFormat_R_INT16{CL_R,CL_UNSIGNED_INT16};
  cl_image_desc imageDescriptor_R_INT16{CL_MEM_OBJECT_IMAGE3D, 256, 256, 256, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_INT16 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_INT16, &imageDescriptor_R_INT16, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_INT16);

  cl_image_format imageFormat_R_INT32{CL_R,CL_UNSIGNED_INT16};
  cl_image_desc imageDescriptor_R_INT32{CL_MEM_OBJECT_IMAGE3D, 128, 128, 128, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_INT32 = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_INT32, &imageDescriptor_R_INT32, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_INT32);

  cl_image_format imageFormat_R_FLOAT{CL_R,CL_FLOAT};
  cl_image_desc imageDescriptor_R_FLOAT{CL_MEM_OBJECT_IMAGE3D, 128, 128, 128, 0, 0, 0, 0, 0, nullptr};
  cl_mem image_R_FLOAT = clCreateImage(trial_clContext, CL_MEM_READ_WRITE, &imageFormat_R_FLOAT, &imageDescriptor_R_FLOAT, nullptr, &err);
  if (err != CL_SUCCESS)
  {
    return false;
  }
  clReleaseMemObject(image_R_FLOAT);

  return true;
}

void GLWidget::resizeGL( int w, int h )
{
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

  if(_sceneDepthTexture)
    glDeleteTextures(1, &_sceneDepthTexture);
  glGenTextures(1, &_sceneDepthTexture);

  if(_glowSelectionTexture)
    glDeleteTextures(1, &_glowSelectionTexture);
  glGenTextures(1, &_glowSelectionTexture);


  glBindFramebuffer(GL_FRAMEBUFFER, _sceneFrameBuffer);
  glGetError();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
  glGetError();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture, 0);
  glGetError();
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _glowSelectionTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
  glGetError();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _glowSelectionTexture, 0);
  glGetError();
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneDepthTexture);
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_S), GLint(GL_CLAMP_TO_EDGE));
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_WRAP_T), GLint(GL_CLAMP_TO_EDGE));
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MIN_FILTER), GLint(GL_NEAREST));
  glTexParameteri(GL_TEXTURE_2D, GLenum(GL_TEXTURE_MAG_FILTER), GLint(GL_NEAREST));
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_DEPTH32F_STENCIL8, _width * _devicePixelRatio, _height * _devicePixelRatio, GL_TRUE);
  glGetError();
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, _sceneDepthTexture, 0);
  glGetError();

  // check framebuffer completeness
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    qWarning("initializeSceneFrameBuffer fatal error: framebuffer incomplete: %0x",status);
    glGetError();
  }

  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if(std::shared_ptr<RKRenderDataSource> dataSource = _dataSource)
  {
    _blurShader.resizeGL(_width,_height);
  }
}


void GLWidget::paintGL()
{
  glGetError();
  updateStructureUniforms();
  glGetError();
  updateTransformUniforms();
  glGetError();
  updateIsosurfaceUniforms();
  updateLightUniforms();

  GLint prev_fbo_id;
  glGetIntegerv( GL_FRAMEBUFFER_BINDING, &prev_fbo_id );

  // pass mouse coordinates (real coordinates twice larger for retina-displays)
  _pickingShader.paintGL(_width, _height, _structureUniformBuffer);

  glViewport(0,0,_width * _devicePixelRatio,_height * _devicePixelRatio);

  drawSceneToFramebuffer(_sceneFrameBuffer);

  _blurShader.paintGL(_glowSelectionTexture, _width, _height);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,prev_fbo_id);
  glViewport(0,0,_width * _devicePixelRatio, _height * _devicePixelRatio);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);

  glUseProgram(_program);
  glBindVertexArray(_downSamplerVertexArray);

  // input scene texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _sceneTexture);
  glUniform1i(_downSampleInputTextureUniformLocation, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, _blurShader.blurredTexture());
  glUniform1i(_blurredInputTextureUniformLocation, 1);
  glUniform1i(_numberOfMultiSamplePointsUniformLocation, _multiSampling);
  glDrawElements(GL_TRIANGLE_STRIP,4,GL_UNSIGNED_SHORT, nullptr);

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(0);
  glUseProgram(0);
}

QImage GLWidget::renderSceneToImage(int width, int height, RKRenderQuality quality)
{
  GLuint sceneFrameBuffer;
  GLuint sceneDepthTexture;
  GLuint sceneTexture;
  GLuint glowSelectionTexture;

  makeCurrent();

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

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, glowSelectionTexture);
  glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, _multiSampling, GL_RGBA16F, width, height, GL_TRUE);
  check_gl_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  check_gl_error();
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glActiveTexture(GL_TEXTURE1);
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

  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

  glViewport(0,0,width,height);

  drawSceneToFramebuffer(sceneFrameBuffer);

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

 // glActiveTexture(GL_TEXTURE1);
//  glBindTexture(GL_TEXTURE_2D, _blurShader.blurredTexture());
//  glUniform1i(_blurredInputTextureUniformLocation, 1);
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

  glDeleteTextures(1, &glowSelectionTexture);
  glDeleteTextures(1, &sceneTexture);
  glDeleteTextures(1, &sceneDepthTexture);
  glDeleteFramebuffers(1, &sceneFrameBuffer);

  return img.mirrored();
}

void GLWidget::drawSceneToFramebuffer(GLuint framebuffer)
{
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,framebuffer);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  check_gl_error();

  // clear the color, depth, and stencil buffer
  // the background default color is white
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  _backgroundShader.paintGL();

  glEnable(GL_DEPTH_TEST);


  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    _atomShader.paintGL(camera, _quality, _structureUniformBuffer);
    _bondShader.paintGL(_structureUniformBuffer);
    _objectShader.paintGL(_structureUniformBuffer);
    _unitCellShader.paintGL(_structureUniformBuffer);

    _boundingBoxShader.paintGL();
    _energySurfaceShader.paintGLOpaque(_structureUniformBuffer,_isosurfaceUniformBuffer);

    _objectShader.paintGLTransparent(_structureUniformBuffer);

    _energySurfaceShader.paintGLTransparent(_structureUniformBuffer,_isosurfaceUniformBuffer);

    _selectionShader.paintGL(camera, _quality, _structureUniformBuffer);

    _textShader.paintGL(_structureUniformBuffer);
  }

  glDrawBuffer(GL_COLOR_ATTACHMENT1);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  _selectionShader.paintGLGlow(_structureUniformBuffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
}

void GLWidget::keyPressEvent( QKeyEvent* e )
{
  Q_UNUSED(e);

  makeCurrent();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
  _timer->stop();

  makeCurrent();
  _startPoint = event->pos();
  _origin = event->pos();

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


void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
  makeCurrent();
  switch(_tracking)
  {
    case Tracking::newSelection:
      // convert to dragged version
      _tracking = Tracking::draggedNewSelection;
      break;
    case Tracking::addToSelection:
       // convert to dragged version
       _tracking = Tracking::draggedAddToSelection;
       break;
    case Tracking::draggedNewSelection:
       break;
    case Tracking::draggedAddToSelection:
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

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
  makeCurrent();

  switch(_tracking)
  {
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

  update();
}


void GLWidget::wheelEvent(QWheelEvent *event)
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

void GLWidget::timeoutEventHandler()
{
  _timer->stop();
  _quality = RKRenderQuality::high;
  update();
}

void GLWidget::initializeTransformUniforms()
{
  glGenBuffers(1, &_frameUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _frameUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, _frameUniformBuffer);
  check_gl_error();


  glUniformBlockBinding(_program, glGetUniformBlockIndex(_program, "FrameUniformBlock"), 0);
  check_gl_error();

  _energySurfaceShader.initializeTransformUniforms();
  check_gl_error();
  _boundingBoxShader.initializeTransformUniforms();
  check_gl_error();
  _atomShader.initializeTransformUniforms();
  check_gl_error();
  _bondShader.initializeTransformUniforms();
  check_gl_error();
  _objectShader.initializeTransformUniforms();
  check_gl_error();
  _unitCellShader.initializeTransformUniforms();
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

void GLWidget::updateTransformUniforms()
{
  double4x4 projectionMatrix = double4x4();
  double4x4 viewMatrix = double4x4();
  double bloomLevel = 1.0;;
  double bloomPulse = 1.0;

  if (std::shared_ptr<RKCamera> camera = _camera.lock())
  {
    projectionMatrix = camera->projectionMatrix();
    viewMatrix = camera->modelViewMatrix();
    bloomLevel = camera->bloomLevel();
    bloomPulse = camera->bloomPulse();
  }

  glBindBuffer(GL_UNIFORM_BUFFER, _frameUniformBuffer);
  check_gl_error();
  RKTransformationUniforms transformationUniforms = RKTransformationUniforms(projectionMatrix, viewMatrix, bloomLevel, bloomPulse, _multiSampling);
  glBufferData (GL_UNIFORM_BUFFER, sizeof(RKTransformationUniforms), &transformationUniforms, GL_DYNAMIC_DRAW);
  check_gl_error();
  glBindBuffer(GL_UNIFORM_BUFFER,0);
}



void GLWidget::initializeStructureUniforms()
{
  glGenBuffers(1, &_structureUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _structureUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 1, _structureUniformBuffer);
  check_gl_error();

  _energySurfaceShader.initializeStructureUniforms();
  _atomShader.initializeStructureUniforms();
  _bondShader.initializeStructureUniforms();
  _objectShader.initializeStructureUniforms();
  _unitCellShader.initializeStructureUniforms();
  _selectionShader.initializeStructureUniforms();
  _pickingShader.initializeStructureUniforms();
  _textShader.initializeStructureUniforms();

  check_gl_error();
  std::vector<RKStructureUniforms> structureUniforms{RKStructureUniforms()};
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKStructureUniforms) * structureUniforms.size(), structureUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  check_gl_error();
}

void GLWidget::updateStructureUniforms()
{
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

void GLWidget::initializeIsosurfaceUniforms()
{
  glGenBuffers(1, &_isosurfaceUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _isosurfaceUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 2, _isosurfaceUniformBuffer);

  _energySurfaceShader.initializeIsosurfaceUniforms();

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  check_gl_error();
}

void GLWidget::updateIsosurfaceUniforms()
{
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


void GLWidget::initializeLightUniforms()
{
  glGenBuffers(1, &_lightsUniformBuffer);
  check_gl_error();

  glBindBuffer(GL_UNIFORM_BUFFER, _lightsUniformBuffer);
  glBindBufferBase(GL_UNIFORM_BUFFER, 3, _lightsUniformBuffer);

  _energySurfaceShader.initializeLightUniforms();
  _boundingBoxShader.initializeLightUniforms();
  _atomShader.initializeLightUniforms();
  _bondShader.initializeLightUniforms();
  _objectShader.initializeLightUniforms();
  _unitCellShader.initializeLightUniforms();
  _selectionShader.initializeLightUniforms();

  std::vector<RKLightsUniforms> lightUniforms{RKLightsUniforms()};
  glBufferData(GL_UNIFORM_BUFFER, sizeof(RKLightsUniforms) * lightUniforms.size(), lightUniforms.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  check_gl_error();
}

void GLWidget::updateLightUniforms()
{
  RKLightsUniforms lightUniforms = RKLightsUniforms(_dataSource);
  glBindBuffer(GL_UNIFORM_BUFFER, _lightsUniformBuffer);
  glBufferData(GL_UNIFORM_BUFFER, lightUniforms.lights.size() * sizeof(RKLightUniform), lightUniforms.lights.data(), GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void GLWidget::reloadData()
{
  makeCurrent();

  _energySurfaceShader.reloadData();
  _boundingBoxShader.reloadData();

  _atomShader.reloadData();
  _atomShader.reloadAmbientOcclusionData(_dataSource, RKRenderQuality::low);
  _bondShader.reloadData();
  _objectShader.reloadData();
  _unitCellShader.reloadData();
  _selectionShader.reloadData();
  _pickingShader.reloadData();
  _textShader.reloadData();

  update();

  check_gl_error();
}

void GLWidget::reloadData(RKRenderQuality quality)
{
  makeCurrent();

  _energySurfaceShader.reloadData();
  _boundingBoxShader.reloadData();

  _atomShader.reloadData();
  _atomShader.reloadAmbientOcclusionData(_dataSource, quality);
  _bondShader.reloadData();
  _objectShader.reloadData();
  _unitCellShader.reloadData();
  _selectionShader.reloadData();
  _pickingShader.reloadData();
  _textShader.reloadData();

  update();

  check_gl_error();
}

void GLWidget::reloadAmbientOcclusionData()
{
  qDebug() << "GLWidget reloadAmbientOcclusionData";
  makeCurrent();
  _atomShader.reloadAmbientOcclusionData(_dataSource, RKRenderQuality::low);
}

void GLWidget::reloadRenderData()
{
  makeCurrent();

   _energySurfaceShader.reloadData();

  _atomShader.reloadData();
  _atomShader.reloadAmbientOcclusionData(_dataSource, RKRenderQuality:: low);
  _bondShader.reloadData();
  _objectShader.reloadData();
  _unitCellShader.reloadData();
  _selectionShader.reloadData();
  _pickingShader.reloadData();
  _textShader.reloadData();

  update();
}

void GLWidget::reloadRenderMeasurePointsData()
{
  makeCurrent();
}

void GLWidget::reloadBoundingBoxData()
{
  makeCurrent();
}

void GLWidget::reloadSelectionData()
{
  makeCurrent();

  _selectionShader.reloadData();
}

void GLWidget::reloadBackgroundImage()
{
  makeCurrent();
  if(_dataSource)
  {
    _backgroundShader.reload(_dataSource);
  }
}

void GLWidget::updateVertexArrays()
{

}

void GLWidget::computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderStructure>> structures)
{
  if(_isOpenCLInitialized)
  {
    makeCurrent();
    _energySurfaceShader.computeHeliumVoidFraction(structures);
  }
  else
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "Unable to compute Helium void fraction (OpenCL not initialized)");
  }
}

void GLWidget::computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderStructure>> structures)
{
  if(_isOpenCLInitialized)
  {
    makeCurrent();
    _energySurfaceShader.computeNitrogenSurfaceArea(structures);
  }
  else
  {
    _logReporter->logMessage(LogReporting::ErrorLevel::verbose, "Unable to compute Nitrogen surface area (OpenCL not initialized)");
  }
}

void GLWidget::loadShader(void)
{
  GLuint vertexShader;
  GLuint fragmentShader;

  vertexShader=compileShaderOfType(GL_VERTEX_SHADER,GLWidget::_vertexShaderSource.c_str());
  fragmentShader=compileShaderOfType(GL_FRAGMENT_SHADER,GLWidget::_fragmentShaderSource.c_str());

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

const std::string GLWidget::_vertexShaderSource =
OpenGLVersionStringLiteral +
std::string(R"foo(
in vec4 position;

out vec2 texcoord;

void main()
{
  gl_Position = position;
  texcoord = position.xy * vec2(0.5) + vec2(0.5);
}
)foo");

const std::string GLWidget:: _fragmentShaderSource =
OpenGLVersionStringLiteral +
OpenGLFrameUniformBlockStringLiteral +
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
