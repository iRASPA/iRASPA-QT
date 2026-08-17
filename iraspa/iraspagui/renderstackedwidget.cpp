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

#include "renderstackedwidget.h"
#include <iostream>
#include <optional>
#include <QMenu>
#include <QApplication>
#include <QObject>
#include <QAction>
#include <QWidgetAction>
#include <QtDebug>
#include <QToolButton>
#include <QActionGroup>
#include <QMainWindow>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSettings>
#include <QUndoCommand>
#include <iraspakit.h>
#include "moviemaker.h"
#include "toolbarwidget.h"
#include "toolbarmenu.h"
#include "fixedaspectratiolayoutitem.h"
#include "renderviewdeleteselectioncommand.h"
#include "renderviewchangeselectioncommand.h"
#include "renderviewchangeselectionsubcommand.h"
#include "renderviewtranslatepositionscartesiancommand.h"
#include "renderviewtranslatepositionscartesiansubcommand.h"
#include "renderviewtranslatepositionsbodyframecommand.h"
#include "renderviewrotatepositionscartesiancommand.h"
#include "renderviewrotatepositionsbodyframecommand.h"
#include <exception>
#include <QHBoxLayout>
#include <QPainter>
#include <QPointer>
#include <QtConcurrent>
#include "iraspaexport.h"
#include "rkrendererbackend.h"

#if defined (USE_OPENGL)
  #include "openglwindow.h"
  #include "ribbonaolayout.h"
#endif
#if defined (USE_VULKAN)
  #include "vulkanwindow.h"
  #include "ribbonaolayout.h"
#endif

#include "proteinribbonmixin.h"
#include "proteinribbonsegmentsupport.h"

namespace
{

bool sceneMovieForFlatStructureIndex(const std::vector<std::vector<std::shared_ptr<iRASPAObject>>> &structures,
                                     int flatIndex, int &sceneId, int &movieId)
{
  int counter = 0;
  for (size_t i = 0; i < structures.size(); ++i)
  {
    for (size_t j = 0; j < structures[i].size(); ++j)
    {
      if (counter == flatIndex)
      {
        sceneId = static_cast<int>(i);
        movieId = static_cast<int>(j);
        return true;
      }
      ++counter;
    }
  }
  return false;
}

void finalizeRibbonSelection(const std::shared_ptr<Object> &object)
{
  if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(object))
  {
    structure->bondSetController()->correctBondSelectionDueToAtomSelection();
    structure->recomputeSelectionBodyFixedBasis();
  }
}

bool applyRibbonPickToObject(const std::shared_ptr<Object> &object,
                             int segmentIndex,
                             int residueIndex,
                             ProteinRibbonMixin::RibbonPickAction action,
                             bool selectSegment)
{
  if (ProteinRibbonMixin *ribbon = dynamic_cast<ProteinRibbonMixin*>(object.get()))
  {
    if (ribbon->applyRibbonPick(segmentIndex, residueIndex, action, selectSegment))
    {
      finalizeRibbonSelection(object);
      return true;
    }
  }
  return false;
}

bool applyAtomInstancePick(const std::shared_ptr<Object> &object, int instanceTag, bool toggle)
{
  if (!object)
  {
    return false;
  }
  const std::optional<AtomInstancePick> pick = decodeAtomInstancePick(object.get(), instanceTag);
  if (!pick)
  {
    return false;
  }
  if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(object))
  {
    if (toggle)
    {
      atomViewer->toggleAtomSelection(pick->asymmetricAtomIndex);
    }
    else
    {
      atomViewer->setAtomSelection(pick->asymmetricAtomIndex);
    }
    return true;
  }
  return false;
}

} // namespace

namespace
{
class SelectionRubberBandOverlay : public QWidget
{
public:
  explicit SelectionRubberBandOverlay(QWidget *parent) : QWidget(parent)
  {
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_NativeWindow);
    hide();
  }

  void setSelectionRect(const QRect &rect, bool dashed)
  {
    _rect = rect;
    _dashed = dashed;
    if (_rect.isNull() || _rect.width() < 2 || _rect.height() < 2)
    {
      hide();
      return;
    }
    show();
    raise();
    update();
  }

protected:
  void paintEvent(QPaintEvent *) override
  {
    if (_rect.isNull())
    {
      return;
    }
    QPainter painter(this);
    QColor color(Qt::lightGray);
    color.setAlpha(100);
    QPen pen;
    pen.setColor(Qt::darkGray);
    if (_dashed)
    {
      pen.setDashPattern(QVector<qreal>{6.0, 3.0});
    }
    else
    {
      pen.setStyle(Qt::SolidLine);
    }
    pen.setWidthF(2.0);
    painter.setPen(pen);
    painter.drawRect(_rect);
    painter.fillRect(_rect, QBrush(color));
  }

private:
  QRect _rect;
  bool _dashed = false;
};
}  // namespace

RenderStackedWidget::RenderStackedWidget(QWidget* parent ): QWidget(parent )
{
  this->setContextMenuPolicy(Qt::PreventContextMenu);

  _layout = new QHBoxLayout(this);
  _layout->setSpacing(0);
  _layout->setContentsMargins(0,0,0,0);

  createRenderer(RKRendererAvailability::preferredBackend());

  setMouseTracking(true);

  _controlPanel = new ToolbarWidget(this);

  toolBar = new ToolbarMenu(this);
  toolBar->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
  QWidgetAction *action = new QWidgetAction(toolBar);
  action->setDefaultWidget(_controlPanel);
  toolBar->addAction(action);
  toolBar->setContentsMargins(0, 0, 0, 0);

  if (renderWidget)
  {
    renderWidget->setMinimumHeight(270);
    renderWidget->setMinimumWidth(120);
  }
}

void RenderStackedWidget::createOpenGLRenderer()
{
#if defined (USE_OPENGL)
  OpenGLWindow *w = new OpenGLWindow(this);
  if(!w)
  {
    qFatal("OPENGL WINDOW IS NULL");
  }
  renderWindow = w;
  w->installEventFilter(this);
  renderViewController = w;
  renderWidget = QWidget::createWindowContainer(w);
  renderWidget->setMouseTracking(true);
  renderWidget->setAttribute( Qt::WA_OpaquePaintEvent );
  renderWidget->setFocusPolicy(Qt::StrongFocus);
  renderWidget->installEventFilter(this);
  _layout->addWidget(renderWidget);
  connect(w, &OpenGLWindow::ribbonAODebugModeChanged, this, &RenderStackedWidget::updateRibbonDebugOverlay);
  _backend = RKRendererBackend::OpenGL;
#endif
}

void RenderStackedWidget::createVulkanRenderer()
{
#if defined (USE_VULKAN)
  VulkanWindow* w = new VulkanWindow(nullptr);
  renderWindow = w;
  w->installEventFilter(this);
  renderViewController = w;
  renderWidget = QWidget::createWindowContainer(w, this);
  renderWidget->setMouseTracking(true);
  renderWidget->setAttribute(Qt::WA_OpaquePaintEvent);
  renderWidget->setFocusPolicy(Qt::StrongFocus);
  renderWidget->installEventFilter(this);
  _layout->addWidget(renderWidget);

  _selectionOverlay = new SelectionRubberBandOverlay(this);
  _selectionOverlay->raise();
  _backend = RKRendererBackend::Vulkan;
#endif
}

void RenderStackedWidget::destroyRenderer()
{
  if (renderWindow)
  {
    renderWindow->removeEventFilter(this);
#if defined(USE_VULKAN)
    if (VulkanWindow *vulkanWindow = dynamic_cast<VulkanWindow *>(renderWindow))
    {
      vulkanWindow->prepareForDestruction();
    }
#endif
  }

  QPointer<QWindow> window = renderWindow;
  if (renderWidget)
  {
    renderWidget->removeEventFilter(this);
    if (_layout)
    {
      _layout->removeWidget(renderWidget);
    }
    renderWidget->hide();
    delete renderWidget;
    renderWidget = nullptr;
  }
  if (window)
  {
    delete window.data();
  }
  renderWindow = nullptr;
  renderViewController = nullptr;

  delete _selectionOverlay;
  _selectionOverlay = nullptr;
}

void RenderStackedWidget::createRenderer(RKRendererBackend backend)
{
#if defined(USE_VULKAN)
  if (backend == RKRendererBackend::Vulkan)
  {
    createVulkanRenderer();
    return;
  }
#endif
#if defined(USE_OPENGL)
  if (backend == RKRendererBackend::OpenGL)
  {
    createOpenGLRenderer();
    return;
  }
#endif
#if defined(USE_VULKAN)
  createVulkanRenderer();
#elif defined(USE_OPENGL)
  createOpenGLRenderer();
#endif
}

void RenderStackedWidget::rebindCurrentProject()
{
  if (std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    setProject(projectTreeNode);
  }
  else
  {
    setProject(nullptr);
  }
}

void RenderStackedWidget::setRendererBackend(RKRendererBackend backend)
{
  if (backend == _backend && renderViewController)
  {
    return;
  }
  if (backend == RKRendererBackend::OpenGL && !RKRendererAvailability::isOpenGLAvailable())
  {
    if (_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::warning, "OpenGL renderer is not available");
    }
    return;
  }
  if (backend == RKRendererBackend::Vulkan && !RKRendererAvailability::isVulkanAvailable())
  {
    if (_logReporter)
    {
      _logReporter->logMessage(LogReporting::ErrorLevel::warning, "Vulkan renderer is not available");
    }
    return;
  }

  destroyRenderer();
  createRenderer(backend);
  if (renderWidget)
  {
    renderWidget->setMinimumHeight(270);
    renderWidget->setMinimumWidth(120);
    renderWidget->show();
  }
  rebindCurrentProject();
  redraw();

  if (_logReporter)
  {
    const QString name = (_backend == RKRendererBackend::Vulkan) ? QString("Vulkan") : QString("OpenGL");
    _logReporter->logMessage(LogReporting::ErrorLevel::info, QString("Switched renderer to %1").arg(name));
  }
}

void RenderStackedWidget::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  this->_projectTreeNode = projectTreeNode;
  this->_project.reset();

  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> render_structures{};
  if (projectTreeNode)
  {
    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if(std::shared_ptr<Project> project = iraspaProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          QSize size = frameSize();
          double ratio = double(size.width()) / double(size.height());
          projectStructure->setImageAspectRatio(ratio);
          emit rendererWidgetResized();

          this->_project = projectStructure;
          this->_camera = projectStructure->camera();
          _iraspa_structures = projectStructure->sceneList()->selectediRASPAStructures();

          for(std::vector<std::shared_ptr<iRASPAObject>> v : _iraspa_structures)
          {
            std::vector<std::shared_ptr<RKRenderObject>> r{};
            std::transform(v.begin(),v.end(),std::back_inserter(r),
                           [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::shared_ptr<RKRenderObject> {return iraspastructure->object();});
            render_structures.push_back(r);
            qDebug() << "Number of structures: " << r.size() << v.size();
          }
        }
      }
    }
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    renderViewController->setRenderStructures(render_structures);
    // Bind the new camera before any present. Vulkan draws synchronously, so
    // reload/redraw with the previous project's camera would flash one frame.
    if (std::shared_ptr<ProjectStructure> projectStructure = this->_project.lock())
    {
      renderViewController->setRenderDataSource(projectStructure);
    }
    else if (projectTreeNode)
    {
      widget->reloadData();
      widget->redraw();
    }
  }
}

void RenderStackedWidget::setLogReportingWidget(LogReporting *logReporting)
{
  _logReporter = logReporting;
  if(_logReporter)
  {
    if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
    {
      _logReporter->insert(widget->logData());
    }
  }
}

void RenderStackedWidget::setSelectedRenderFrames(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> iraspa_structures)
{
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> renderStructures{};
  for(std::vector<std::shared_ptr<iRASPAObject>> v : iraspa_structures)
  {
    std::vector<std::shared_ptr<RKRenderObject>> r{};
    std::transform(v.begin(),v.end(),std::back_inserter(r),
                   [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::shared_ptr<RKRenderObject> {return iraspastructure->object();});
    renderStructures.push_back(r);
  }

  _iraspa_structures = iraspa_structures;
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    widget->setRenderStructures(renderStructures);
  }

  reloadData();
}

void RenderStackedWidget::setAtomModel(std::shared_ptr<AtomTreeViewModel> atomModel)
{
  _atomModel = atomModel;
}

void RenderStackedWidget::setBondModel(std::shared_ptr<BondListViewModel> bondModel)
{
  _bondModel = bondModel;
}



void RenderStackedWidget::resizeEvent(QResizeEvent *event)
{
  QSize size = event->size();
  double ratio = double(size.width())/double(size.height());
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->setImageAspectRatio(ratio);
  }
  if (_selectionOverlay && renderWidget)
  {
    _selectionOverlay->setGeometry(renderWidget->geometry());
  }
  emit rendererWidgetResized();
}

void RenderStackedWidget::updateSelectionOverlay(const QPoint &globalPos, bool dragging, bool dashed)
{
  auto *overlay = static_cast<SelectionRubberBandOverlay *>(_selectionOverlay);
  if (!overlay)
  {
    return;
  }
  if (renderWidget)
  {
    overlay->setGeometry(renderWidget->geometry());
  }
  if (!dragging)
  {
    overlay->setSelectionRect(QRect(), dashed);
    return;
  }
  const QPoint originLocal = overlay->mapFromGlobal(_originGlobal);
  const QPoint currentLocal = overlay->mapFromGlobal(globalPos);
  overlay->setSelectionRect(QRect(originLocal, currentLocal).normalized(), dashed);
}


bool RenderStackedWidget::eventFilter(QObject *object, QEvent *event)
{
  static int coalesce = 0;
  Q_UNUSED(object);

  if(event->type() == QEvent::MouseMove)
  {
    coalesce++;
    if(coalesce%10 == 0)
    {
      emit updateCameraMovement();
    }
  }

  if(event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::Wheel)
  {
    emit updateCameraMovement();
  }


  if(object == renderWindow || object == renderWidget || object == this)
  {
    if(QKeyEvent * keyEvent = dynamic_cast<QKeyEvent*>(event))
    {
      if (event->type() == QEvent::KeyPress)
      {
        if (keyEvent->modifiers().testFlag(Qt::AltModifier) && keyEvent->key() == Qt::Key_D)
        {
#if defined(USE_OPENGL)
          if (OpenGLWindow *openGLWindow = dynamic_cast<OpenGLWindow*>(renderViewController))
          {
            openGLWindow->cycleRibbonAODebugMode();
            event->accept();
            return true;
          }
#endif
#if defined(USE_VULKAN)
          if (VulkanWindow *vulkanWindow = dynamic_cast<VulkanWindow*>(renderViewController))
          {
            vulkanWindow->cycleRibbonAODebugMode();
            event->accept();
            return true;
          }
#endif
        }
        if (keyEvent->modifiers().testFlag(Qt::AltModifier))
        {
          setControlPanel(true);
          event->ignore();
        }
      }
      if (event->type() == QEvent::KeyRelease)
      {
        if (keyEvent->key() == Qt::Key_Alt)
        {
          setControlPanel(false);
          event->ignore();
        }
      }
    }
  }

  if( event->type() == QEvent::KeyPress )
  {
    QKeyEvent * keyEvent = dynamic_cast<QKeyEvent*>(event);
    if( keyEvent->key() == Qt::Key_Delete )
    {
      deleteSelection();
    }
  }

  Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
  bool isSHIFT = keyMod.testFlag(Qt::ShiftModifier);
  bool isCTRL = keyMod.testFlag(Qt::ControlModifier);
  bool isALT = keyMod.testFlag(Qt::AltModifier);

  if(event->type() == QEvent::MouseButtonPress)
  {
    const QMouseEvent* const me = static_cast<const QMouseEvent*>( event );
    _origin = me->pos();
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    _originGlobal = me->globalPos();
#else
    _originGlobal = me->globalPosition().toPoint();
#endif

    if(isSHIFT)   // contains shift
    {
      // Using the shift key means a new selection is chosen. If later a drag occurs it is modified to 'draggedNewSelection'
      _tracking = Tracking::newSelection;
    }
    else if(isALT && isCTRL)  // option and command
    {
      _tracking = Tracking::translateSelection;
      _pickedDepth = std::nullopt;
      if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
      {
        _pickedDepth = widget->pickDepth(_origin.x(), _origin.y(), this->width(), this->height());
      }
    }
    else if(isCTRL)  // command, not option
    {
      // Using the command key means the selection is extended. If later a drag occurs it is modified to 'draggedAddToSelection'
      _tracking = Tracking::addToSelection;
    }
    else if(isALT) // option, not command
    {
      _tracking = Tracking::measurement;
    }
    else
    {
      _tracking = Tracking::backgroundClick;
    }

    // show context menu when right-click _without_ any modifiers
    if(me->button() == Qt::RightButton )
    {
      if(keyMod.testFlag(Qt::NoModifier))
      {
        ShowContextMenu(_origin);
        return true;
      }
      else if(keyMod == Qt::AltModifier)
      {
        _tracking = Tracking::panning;
        _panStartPoint = _origin;
      }
      else if(keyMod == Qt::ControlModifier)
      {
        _tracking = Tracking::trucking;
        _truckStartPoint = _origin;
      }
    }
  }

  if(event->type() == QEvent::MouseMove)
  {
    const QMouseEvent* const me = static_cast<const QMouseEvent*>( event );
    switch(_tracking)
    {
      case Tracking::newSelection:
        // convert to dragged version
        _tracking = Tracking::draggedNewSelection;
        updateSelectionOverlay(
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            me->globalPos(),
#else
            me->globalPosition().toPoint(),
#endif
            true, false);
        break;
      case Tracking::addToSelection:
        // convert to dragged version
        _tracking = Tracking::draggedAddToSelection;
        updateSelectionOverlay(
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            me->globalPos(),
#else
            me->globalPosition().toPoint(),
#endif
            true, true);
        break;
      case Tracking::draggedNewSelection:
        updateSelectionOverlay(
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            me->globalPos(),
#else
            me->globalPosition().toPoint(),
#endif
            true, false);
        break;
      case Tracking::draggedAddToSelection:
        updateSelectionOverlay(
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            me->globalPos(),
#else
            me->globalPosition().toPoint(),
#endif
            true, true);
        break;
      case Tracking::translateSelection:
        if (_pickedDepth)
        {
          shiftSelection(me->pos(), _origin, static_cast<double>(*_pickedDepth));
        }
        break;
      case Tracking::measurement:
        break;
      case Tracking::panning:
        {
          QPoint location = me->pos();
          QPoint panDelta = _panStartPoint - location;
          if (std::shared_ptr<RKCamera> camera = _camera.lock())
          {
            double distance = camera->distance().z;
            camera->setPanning(panDelta.x() * distance/1500.0, -panDelta.y() * distance/1500.0);
            _panStartPoint = location;
          }
        }
        break;
      case Tracking::trucking:
        {
          QPoint location = me->pos();
          QPoint truckDelta = _truckStartPoint - location;
          if (std::shared_ptr<RKCamera> camera = _camera.lock())
          {
            double distance = camera->distance().z;
            camera->setTrucking(truckDelta.x() * distance/1500.0, -truckDelta.y() * distance/1500.0);
            _truckStartPoint = location;
          }
        }
        break;
      default:
        QPoint location = me->pos();
        QPoint delta = _origin - location;
        if(delta != QPoint(0,0))
        {
          _tracking =Tracking::other;
        }
        break;
    }
  }

  if(event->type() == QEvent::MouseButtonRelease)
  {
    updateSelectionOverlay(QPoint(), false, false);
    const QMouseEvent* const me = static_cast<const QMouseEvent*>( event );
    std::array<int,4> pixel{0,0,0,0};
    QRect rect = QRect(_origin, me->pos()).normalized();

    switch(_tracking)
    {
      case Tracking::newSelection:
        if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            pixel = widget->pickTexture(me->pos().x(), me->pos().y(), this->width(), this->height());
          #else
            pixel = widget->pickTexture(me->position().x(), me->position().y(), this->width(), this->height());
          #endif

          int sceneIdentifier = pixel[1];
          int movieIdentifier = pixel[2];
          int pickedObject  = pixel[3];

          switch(pixel[0])
          {
          case 1:
            if (applyAtomInstancePick(_iraspa_structures[sceneIdentifier][movieIdentifier]->object(), pickedObject, false))
            {
              reloadData();
              emit updateAtomSelection();
              emit updateBondSelection();
            }
            break;
          case 2:
            if (std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspa_structures[sceneIdentifier][movieIdentifier]->object()))
            {
              bondViewer->setBondSelection(pickedObject);
              reloadData();
              emit updateBondSelection();
            }
            break;
          case ProteinRibbonSegmentSupport::ribbonPickObjectType:
          {
            int ribbonSceneId = 0;
            int ribbonMovieId = 0;
            if (sceneMovieForFlatStructureIndex(_iraspa_structures, pixel[1], ribbonSceneId, ribbonMovieId))
            {
              if (applyRibbonPickToObject(_iraspa_structures[ribbonSceneId][ribbonMovieId]->object(),
                                          pixel[2], pixel[3],
                                          ProteinRibbonMixin::RibbonPickAction::replaceResidue, false))
              {
                reloadData();
                emit updateAtomSelection();
                emit updateBondSelection();
              }
            }
            break;
          }
          default:
            break;
          }
        }
        this->updateControlPanel();
        break;
      case Tracking::addToSelection:
        if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            pixel = widget->pickTexture(me->pos().x(), me->pos().y(), this->width(), this->height());
          #else
            pixel = widget->pickTexture(me->position().x(), me->position().y(), this->width(), this->height());
          #endif
          int sceneIdentifier = pixel[1];
          int movieIdentifier = pixel[2];
          int pickedObject  = pixel[3];


          switch(pixel[0])
          {
          case 1:
            if (applyAtomInstancePick(_iraspa_structures[sceneIdentifier][movieIdentifier]->object(), pickedObject, true))
            {
              reloadData();
              emit updateAtomSelection();
              emit updateBondSelection();
            }
            break;
          case 2:
            if (std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspa_structures[sceneIdentifier][movieIdentifier]->object()))
            {
              bondViewer->toggleBondSelection(pickedObject);
              reloadData();
              emit updateBondSelection();
            }
            break;
          case ProteinRibbonSegmentSupport::ribbonPickObjectType:
          {
            int ribbonSceneId = 0;
            int ribbonMovieId = 0;
            if (sceneMovieForFlatStructureIndex(_iraspa_structures, pixel[1], ribbonSceneId, ribbonMovieId))
            {
              if (applyRibbonPickToObject(_iraspa_structures[ribbonSceneId][ribbonMovieId]->object(),
                                          pixel[2], pixel[3],
                                          ProteinRibbonMixin::RibbonPickAction::toggleResidue, false))
              {
                reloadData();
                emit updateAtomSelection();
                emit updateBondSelection();
              }
            }
            break;
          }
          default:
            break;
          }
        }
        this->updateControlPanel();
        break;
      case Tracking::draggedNewSelection:
        if (std::shared_ptr<RKCamera> camera = _camera.lock())
        {
          selectAsymetricAtomsInRectangle(rect, false);
        }
        this->updateControlPanel();
        break;
      case Tracking::draggedAddToSelection:
        if (std::shared_ptr<RKCamera> camera = _camera.lock())
        {
          selectAsymetricAtomsInRectangle(rect, true);
        }
        this->updateControlPanel();
        break;
      case Tracking::translateSelection:
        if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
        {
          const QPoint releasePos = me->pos();
          const QPoint delta = releasePos - _origin;
          const bool isClick = (delta.x() * delta.x() + delta.y() * delta.y()) <= 64;
          #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            pixel = widget->pickTexture(me->pos().x(), me->pos().y(), this->width(), this->height());
          #else
            pixel = widget->pickTexture(me->position().x(), me->position().y(), this->width(), this->height());
          #endif
          if (isClick && pixel[0] == ProteinRibbonSegmentSupport::ribbonPickObjectType)
          {
            int ribbonSceneId = 0;
            int ribbonMovieId = 0;
            if (sceneMovieForFlatStructureIndex(_iraspa_structures, pixel[1], ribbonSceneId, ribbonMovieId))
            {
              if (applyRibbonPickToObject(_iraspa_structures[ribbonSceneId][ribbonMovieId]->object(),
                                          pixel[2], pixel[3],
                                          ProteinRibbonMixin::RibbonPickAction::toggleSecondaryStructureSegment, true))
              {
                reloadData();
                emit updateAtomSelection();
                emit updateBondSelection();
              }
            }
          }
          else if (_pickedDepth)
          {
            finalizeShiftSelection(releasePos, _origin, static_cast<double>(*_pickedDepth));
          }
        }
        _pickedDepth = std::nullopt;
        this->updateControlPanel();
        break;
      case Tracking::measurement:
        if (std::shared_ptr<ProjectStructure> project = _project.lock())
        {
          if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
          {
            #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
              pixel = widget->pickTexture(me->pos().x(), me->pos().y(), this->width(), this->height());
            #else
              pixel = widget->pickTexture(me->position().x(), me->position().y(), this->width(), this->height());
            #endif

            const int sceneIdentifier = pixel[1];
            const int movieIdentifier = pixel[2];
            const int pickedObject = pixel[3];
            const bool validAtomPick =
                pixel[0] == 1 &&
                sceneIdentifier >= 0 && static_cast<size_t>(sceneIdentifier) < _iraspa_structures.size() &&
                movieIdentifier >= 0 && static_cast<size_t>(movieIdentifier) < _iraspa_structures[static_cast<size_t>(sceneIdentifier)].size() &&
                _iraspa_structures[static_cast<size_t>(sceneIdentifier)][static_cast<size_t>(movieIdentifier)];

            if (validAtomPick)
            {
              std::shared_ptr<RKRenderObject> renderObject =
                  _iraspa_structures[static_cast<size_t>(sceneIdentifier)][static_cast<size_t>(movieIdentifier)]->object();
              if (dynamic_cast<RKRenderAtomSource *>(renderObject.get()) && pickedObject >= 0)
              {
                if (project->addAtomToMeasurement(renderObject, pickedObject))
                {
                  const QString message = project->measurementLogMessage();
                  if (_logReporter && !message.isEmpty())
                  {
                    _logReporter->logMessage(LogReporting::ErrorLevel::info, message);
                  }
                }
                else
                {
                  project->clearMeasurement();
                }
              }
              else
              {
                project->clearMeasurement();
              }
            }
            else
            {
              project->clearMeasurement();
            }

            widget->reloadRenderMeasurePointsData();
            widget->redraw();
          }
        }
        break;
      case Tracking::backgroundClick:
        for(size_t i=0; i< _iraspa_structures.size();i++)
        {
          for(size_t j=0; j<_iraspa_structures[i].size(); j++)
          {
            if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(_iraspa_structures[i][j]->object()))
            {
              atomViewer->clearSelection();
            }
          }
        }
        if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
            pixel = widget->pickTexture(me->pos().x(), me->pos().y(), this->width(), this->height());
          #else
            pixel = widget->pickTexture(me->position().x(), me->position().y(), this->width(), this->height());
          #endif

          int sceneIdentifier = pixel[1];
          int movieIdentifier = pixel[2];
          int pickedObject  = pixel[3];

          switch(pixel[0])
          {
          case 0:
            reloadData();
            emit updateAtomSelection();
            emit updateBondSelection();
            break;
          case 1:

            if (applyAtomInstancePick(_iraspa_structures[sceneIdentifier][movieIdentifier]->object(), pickedObject, false))
            {
              reloadData();
              emit updateAtomSelection();
              emit updateBondSelection();
            }
            break;
          case 2:
            if (std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspa_structures[sceneIdentifier][movieIdentifier]->object()))
            {
              bondViewer->setBondSelection(pickedObject);
              reloadData();
              emit updateBondSelection();
            }
            break;
          case ProteinRibbonSegmentSupport::ribbonPickObjectType:
          {
            int ribbonSceneId = 0;
            int ribbonMovieId = 0;
            if (sceneMovieForFlatStructureIndex(_iraspa_structures, pixel[1], ribbonSceneId, ribbonMovieId))
            {
              if (applyRibbonPickToObject(_iraspa_structures[ribbonSceneId][ribbonMovieId]->object(),
                                          pixel[2], pixel[3],
                                          ProteinRibbonMixin::RibbonPickAction::replaceResidue, false))
              {
                reloadData();
                emit updateAtomSelection();
                emit updateBondSelection();
              }
            }
            break;
          }
          default:
            break;
          }
        }
        this->updateControlPanel();
        break;
      default:
        break;
    }
  }

  update();

  return QWidget::eventFilter(object, event);
}

void RenderStackedWidget::selectAsymetricAtomsInRectangle(QRect rect, bool extend)
{
  if (std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if (std::shared_ptr<RKCamera> camera = _camera.lock())
      {
        QRect viewPortBounds = QRect(QPoint(0.0,0.0), this->size());
        double3 Points0 = camera->myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.top()), 0.0), viewPortBounds);
        double3 Points1 = camera->myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.top()), 1.0), viewPortBounds);

        double3 Points2 = camera->myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.bottom()), 0.0), viewPortBounds);
        double3 Points3 = camera->myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.bottom()), 1.0), viewPortBounds);

        double3 Points4 = camera->myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.bottom()), 0.0), viewPortBounds);
        double3 Points5 = camera->myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.bottom()), 1.0), viewPortBounds);

        double3 Points6 = camera->myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.top()), 0.0), viewPortBounds);
        double3 Points7 = camera->myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.top()), 1.0), viewPortBounds);

        double3 FrustrumPlane0 = double3::cross(Points0 - Points1, Points0 - Points2).normalise();
        double3 FrustrumPlane1 = double3::cross(Points2 - Points3, Points2 - Points4).normalise();
        double3 FrustrumPlane2 = double3::cross(Points4 - Points5, Points4 - Points6).normalise();
        double3 FrustrumPlane3 = double3::cross(Points6 - Points7, Points6 - Points0).normalise();


        std::function<bool(double3)> closure = [=](double3 position) -> bool {
          return (double3::dot(position-Points0,FrustrumPlane0)<0) &&
              (double3::dot(position-Points2,FrustrumPlane1)<0) &&
              (double3::dot(position-Points4,FrustrumPlane2)<0) &&
              (double3::dot(position-Points6,FrustrumPlane3)<0);
        };


        RenderViewChangeSelectionCommand *changeSelectionCommand = new RenderViewChangeSelectionCommand(_mainWindow, _atomModel.get(), _bondModel.get());

        for(size_t i=0; i< _iraspa_structures.size();i++)
        {
          for(size_t j=0; j<_iraspa_structures[i].size(); j++)
          {
            std::shared_ptr<Object> object = _iraspa_structures[i][j]->object();
            if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(object))
            {
              std::shared_ptr<SKAtomTreeController> atomTreeController = atomViewer->atomsTreeController();


              AtomSelectionIndexPaths previousAtomSelection = atomTreeController->selectionIndexPaths();

              std::set<int> indexSetSelectedAtoms = atomViewer->filterCartesianAtomPositions(closure);

              if(extend)
              {
                atomViewer->addToAtomSelection(indexSetSelectedAtoms);
              }
              else
              {
                atomViewer->setAtomSelection(indexSetSelectedAtoms);
              }

              AtomSelectionIndexPaths atomSelection = atomTreeController->selectionIndexPaths();

              BondSelectionIndexSet previousBondSelection{};
              BondSelectionIndexSet bondSelection{};
              if (std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(object))
              {
                std::shared_ptr<SKBondSetController> bondSetController = bondViewer->bondSetController();

                previousBondSelection = bondSetController->selectionIndexSet();

                BondSelectionIndexSet indexSetSelectedBonds = bondViewer->filterCartesianBondPositions(closure);

                if(extend)
                {
                  bondViewer->bondSetController()->addSelectedObjects(indexSetSelectedBonds);
                }
                else
                {
                  bondViewer->bondSetController()->setSelectionIndexSet(indexSetSelectedBonds);
                }

                bondSelection = bondSetController->selectionIndexSet();
              }

              new RenderViewChangeSelectionSubCommand(object, atomSelection, previousAtomSelection,
                                                      bondSelection, previousBondSelection, changeSelectionCommand);
            }
          }
        }
        iraspaProject->undoManager().push(changeSelectionCommand);
      }
    }
  }
}

void RenderStackedWidget::deleteSelection()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            RenderViewDeleteSelectionCommand *deleteSelectionCommand = new RenderViewDeleteSelectionCommand(_mainWindow, _atomModel.get(), _bondModel.get(),
                                                                                                            projectStructure, nullptr);
            iRASPAProject->undoManager().push(deleteSelectionCommand);
          }
        }
      }
    }
  }
  this->updateControlPanel();
}


void RenderStackedWidget::ShowContextMenu(const QPoint &pos)
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    QMenu contextMenu("Context menu", this);

    QAction actionResetCameraDistance(tr("Reset Camera Distance"), this);
    connect(&actionResetCameraDistance, &QAction::triggered, this, &RenderStackedWidget::resetCameraDistance);
    contextMenu.addAction(&actionResetCameraDistance);

    QMenu* subMenuResetCamera = contextMenu.addMenu(tr("Reset Camera To"));
    QActionGroup* cameraResetDirectionGroup = new QActionGroup(this);
    QAction actionResetToZ(tr("Z-Direction"), this);
    cameraResetDirectionGroup->addAction(&actionResetToZ);
    subMenuResetCamera->addAction(&actionResetToZ);
    connect(&actionResetToZ, &QAction::triggered, this, &RenderStackedWidget::resetCameraToZ);
    QAction actionResetToY(tr("Y-Direction"), this);
    subMenuResetCamera->addAction(&actionResetToY);
    cameraResetDirectionGroup->addAction(&actionResetToY);
    connect(&actionResetToY, &QAction::triggered, this, &RenderStackedWidget::resetCameraToY);
    QAction actionResetToX(tr("X-Direction"), this);
    cameraResetDirectionGroup->addAction(&actionResetToX);
    connect(&actionResetToX, &QAction::triggered, this, &RenderStackedWidget::resetCameraToX);
    subMenuResetCamera->addAction(&actionResetToX);


    QMenu* subMenuCameraProjection = contextMenu.addMenu(tr("Camera Projection"));
    QActionGroup* cameraProjectionGroup = new QActionGroup(this);
    QAction actionOrthographic(tr("Orthographic"), this);
    actionOrthographic.setCheckable(true);
    cameraProjectionGroup->addAction(&actionOrthographic);
    subMenuCameraProjection->addAction(&actionOrthographic);
    actionOrthographic.setChecked(project->camera()->isOrthographic());
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::setCameraToOrthographic);
    QAction actionPerspective(tr("Perspective"), this);
    actionPerspective.setCheckable(true);
    cameraProjectionGroup->addAction(&actionPerspective);
    subMenuCameraProjection->addAction(&actionPerspective);
    connect(&actionPerspective, &QAction::triggered, this, &RenderStackedWidget::setCameraToPerspective);
    actionPerspective.setChecked(project->camera()->isPerspective());

    QAction _actionShowBoundingBox(tr("Show Bounding Box"), this);
    _actionShowBoundingBox.setCheckable(true);
    _actionShowBoundingBox.setChecked(project->showBoundingBox());
    connect(&_actionShowBoundingBox, &QAction::toggled, this, &RenderStackedWidget::showBoundingBox);
    contextMenu.addAction(&_actionShowBoundingBox);

    QAction _actionComputeAOHighQuality(tr("Compute AO High Quality"), this);
    connect(&_actionComputeAOHighQuality, &QAction::triggered, this, &RenderStackedWidget::computeAOHighQuality);
    contextMenu.addAction(&_actionComputeAOHighQuality);

    QMenu* subMenuExport = contextMenu.addMenu(tr("Export To"));
    QActionGroup* exportToGroup = new QActionGroup(this);
    QAction actionExportToPDB(tr("PDB"), this);
    exportToGroup->addAction(&actionExportToPDB);
    subMenuExport->addAction(&actionExportToPDB);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToPDB);
    QAction actionExportToMMCIF(tr("mmCIF"), this);
    exportToGroup->addAction(&actionExportToMMCIF);
    subMenuExport->addAction(&actionExportToMMCIF);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToMMCIF);
    QAction actionExportToCIF(tr("CIF"), this);
    exportToGroup->addAction(&actionExportToCIF);
    subMenuExport->addAction(&actionExportToCIF);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToCIF);
    QAction actionExportToXYZ(tr("XYZ"), this);
    exportToGroup->addAction(&actionExportToXYZ);
    subMenuExport->addAction(&actionExportToXYZ);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToXYZ);
    QAction actionExportToPOSCAR(tr("VASP POSCAR"), this);
    exportToGroup->addAction(&actionExportToPOSCAR);
    subMenuExport->addAction(&actionExportToPOSCAR);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToPOSCAR);

    contextMenu.exec(mapToGlobal(pos));
  }
}


// reload-data but also injects the selected frames
void RenderStackedWidget::reloadRenderData()
{
  if(std::shared_ptr<ProjectStructure> project = _project.lock())
  {
    _iraspa_structures = project->sceneList()->selectediRASPAStructures();

    std::vector<std::vector<std::shared_ptr<RKRenderObject>>> render_structures{};
    for(std::vector<std::shared_ptr<iRASPAObject>> v : _iraspa_structures)
    {
      std::vector<std::shared_ptr<RKRenderObject>> r{};
      std::transform(v.begin(),v.end(),std::back_inserter(r),
                     [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::shared_ptr<RKRenderObject> {return iraspastructure->object();});
      render_structures.push_back(r);
    }

    if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
    {
      try
      {
        widget->setRenderStructures(render_structures);
        widget->reloadRenderData();
        widget->redraw();
      }
      catch(std::exception const& e)
      {
        if(_logReporter)
        {
          _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
        }
      }
    }
  }
}

void RenderStackedWidget::resetData()
{
  std::vector<std::vector<std::shared_ptr<RKRenderObject>>> render_structures{};

  for(std::vector<std::shared_ptr<iRASPAObject>> v : _iraspa_structures)
  {
    std::vector<std::shared_ptr<RKRenderObject>> r{};
    std::transform(v.begin(),v.end(),std::back_inserter(r),
                   [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::shared_ptr<RKRenderObject> {return iraspastructure->object();});
    render_structures.push_back(r);
  }

  if(std::shared_ptr<ProjectStructure> project = _project.lock())
  {
    SKBoundingBox box = project->renderBoundingBox();
    project->camera()->resetForNewBoundingBox(box);
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    try
    {
      widget->setRenderStructures(render_structures);
      widget->reloadData();
    }
    catch(std::exception const& e)
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
      }
    }
  }
  redraw();
}

void RenderStackedWidget::reloadData()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    try
    {
      widget->reloadData();
    }
    catch(std::exception const& e)
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
      }
    }
  }
  redraw();
}

void RenderStackedWidget::reloadStructureUniforms()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    try
    {
      widget->reloadStructureUniforms();
    }
    catch(std::exception const& e)
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
      }
    }
  }
  updateRibbonDebugOverlay();
  redraw();
}

void RenderStackedWidget::updateRibbonDebugOverlay()
{
#if defined(USE_OPENGL)
  if (OpenGLWindow *openGLWindow = dynamic_cast<OpenGLWindow*>(renderViewController))
  {
    openGLWindow->update();
  }
#endif
#if defined(USE_VULKAN)
  if (VulkanWindow *vulkanWindow = dynamic_cast<VulkanWindow*>(renderViewController))
  {
    vulkanWindow->redraw();
  }
#endif
}

void RenderStackedWidget::reloadBoundingBoxData()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    try
    {
      widget->reloadBoundingBoxData();
    }
    catch(std::exception const& e)
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
      }
    }
  }
  redraw();
}

void RenderStackedWidget::reloadAmbientOcclusionData()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    try
    {
      widget->reloadAmbientOcclusionData();
    }
    catch(std::exception const& e)
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
      }
    }
  }
  redraw();
}

void RenderStackedWidget::reloadSelectionData()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    try
    {
      widget->reloadData();
    }
    catch(std::exception const& e)
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
      }
    }
  }
  redraw();
}

void RenderStackedWidget::reloadBackgroundImage()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    try
    {
      widget->reloadBackgroundImage();
    }
    catch(std::exception const& e)
    {
      if(_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, e.what());
      }
    }
  }
  redraw();
}


void RenderStackedWidget::resetCameraDistance()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
     project->camera()->resetCameraDistance();
     emit updateCameraModelViewMatrix();
     redraw();
  }
}

void RenderStackedWidget::resetCameraToZ()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->camera()->setBoundingBox(project->renderBoundingBox());
    project->camera()->setResetDirectionType( ResetDirectionType::plus_Z);
    project->camera()->resetCameraToDirection();
    project->camera()->resetCameraDistance();
    emit updateCameraResetDirection();
    redraw();
  }
}

void RenderStackedWidget::resetCameraToY()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->camera()->setBoundingBox(project->renderBoundingBox());
    project->camera()->setResetDirectionType( ResetDirectionType::plus_Y);
    project->camera()->resetCameraToDirection();
    project->camera()->resetCameraDistance();
    emit updateCameraResetDirection();
    redraw();
  }
}

void RenderStackedWidget::resetCameraToX()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->camera()->setBoundingBox(project->renderBoundingBox());
    project->camera()->setResetDirectionType( ResetDirectionType::plus_X);
    project->camera()->resetCameraToDirection();
    project->camera()->resetCameraDistance();
    emit updateCameraResetDirection();
    redraw();
  }
}

void RenderStackedWidget::setCameraToOrthographic()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->camera()->setCameraToOrthographic();
    emit updateCameraProjection();
    redraw();
  }
}

void RenderStackedWidget::setCameraToPerspective()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->camera()->setCameraToPerspective();
    emit updateCameraProjection();
    redraw();
  }
}

void RenderStackedWidget::showBoundingBox(bool checked)
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->setShowBoundingBox(checked);
    reloadData();
    redraw();
  }
}

void RenderStackedWidget::computeAOHighQuality()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    // invalidate all lower quality caches
    if(std::shared_ptr<ProjectStructure> project = _project.lock())
    {
      invalidateCachedAmbientOcclusionTextures(project->sceneList()->allIRASPAStructures());
    }
    widget->reloadData(RKRenderQuality::picture);
    widget->redraw();
  }
}




void RenderStackedWidget::createPicture(QUrl fileURL, int width, int height)
{
  if (std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    try
    {
      std::shared_ptr<ProjectTreeNode> nodeCopy = projectTreeNode->shallowClone();
      const RKRendererBackend backend = _backend;
      QtConcurrent::run([nodeCopy, fileURL, width, height, backend]() {
        runOffscreenPictureExport(nodeCopy, fileURL, width, height, backend);
      });
    }
    catch (const std::exception &e)
    {
      if (_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, QString("failed to copy project for picture export: %1").arg(e.what()));
      }
    }
    return;
  }

  if (RKRenderViewController *widget = dynamic_cast<RKRenderViewController *>(renderViewController))
  {
    if (std::shared_ptr<ProjectStructure> project = _project.lock())
    {
      invalidateCachedAmbientOcclusionTextures(project->sceneList()->allIRASPAStructures());
    }
    QImage image = widget->renderSceneToImage(width, height, RKRenderQuality::picture);
    image.save(fileURL.toLocalFile());
  }
}

int nearestEvenInt(int to)
{
  return (to % 2 == 0) ? to : (to + 1);
}

void RenderStackedWidget::createMovie(QUrl fileURL, int width, int height, MovieWriter::Format format, ProjectStructure::MovieType movieType)
{
  if (std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    try
    {
      std::shared_ptr<ProjectTreeNode> nodeCopy = projectTreeNode->shallowClone();
      const RKRendererBackend backend = _backend;
      QtConcurrent::run([nodeCopy, fileURL, width, height, format, movieType, backend]() {
        runOffscreenMovieExport(nodeCopy, fileURL, width, height, format, movieType, backend);
      });
    }
    catch (const std::exception &e)
    {
      if (_logReporter)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, QString("failed to copy project for movie export: %1").arg(e.what()));
      }
    }
    return;
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    if (std::shared_ptr<ProjectStructure> project = _project.lock())
    {
      MovieWriter movie(nearestEvenInt(width), nearestEvenInt(height), project->movieFramesPerSecond(), _logReporter, format);
      int ret = movie.initialize(fileURL.toLocalFile().toStdString());
      if (ret < 0)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to create movie");
        return;
      }

      switch(movieType)
      {
      case ProjectStructure::MovieType::frames:
        {
          size_t numberOfFrames = project->maxNumberOfMoviesFrames();
          for (size_t iframe = 0; iframe < numberOfFrames; iframe++)
          {
            project->sceneList()->setSelectedFrameIndex(iframe);

            std::vector<std::vector<std::shared_ptr<RKRenderObject>>> render_structures = _project.lock()->sceneList()->selectediRASPARenderStructures();

            renderViewController->setRenderStructures(render_structures);
            renderViewController->reloadData();

            invalidateCachedAmbientOcclusionTextures(project->sceneList()->allIRASPAStructures());
            QImage image = widget->renderSceneToImage(nearestEvenInt(width), nearestEvenInt(height), RKRenderQuality::picture);

            movie.addFrame(image.bits(), iframe);
          }
        }
        break;
      case ProjectStructure::MovieType::rotationY:
        {
          double theta = -3.0 * M_PI/180.0;
          std::shared_ptr<RKCamera> camera = project->camera();
          for(size_t iframe=0; iframe<120;iframe++)
          {
            QImage image = widget->renderSceneToImage(nearestEvenInt(width), nearestEvenInt(height), RKRenderQuality::picture);
            movie.addFrame(image.bits(), iframe);
            camera->rotateCameraAroundAxisY(theta);
          }
        }
        break;
      default:
        break;
      }
      movie.finalize();

      std::vector<std::vector<std::shared_ptr<RKRenderObject>>> render_structures = _project.lock()->sceneList()->selectediRASPARenderStructures();
      renderViewController->setRenderStructures(render_structures);
      renderViewController->reloadData();
    }
  }
}

void RenderStackedWidget::pressedTranslateCartesianMinusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(-0.1,0.0,0.0);
            RenderViewTranslatePositionsCartesianCommand *translationCommand = new RenderViewTranslatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateCartesianPlusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.1,0.0,0.0);
            RenderViewTranslatePositionsCartesianCommand *translationCommand = new RenderViewTranslatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateCartesianMinusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,-0.1,0.0);
            RenderViewTranslatePositionsCartesianCommand *translationCommand = new RenderViewTranslatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateCartesianPlusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,0.1,0.0);
            RenderViewTranslatePositionsCartesianCommand *translationCommand = new RenderViewTranslatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateCartesianMinusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,0.0,-0.1);
            RenderViewTranslatePositionsCartesianCommand *translationCommand = new RenderViewTranslatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateCartesianPlusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,0.0,0.1);
            RenderViewTranslatePositionsCartesianCommand *translationCommand = new RenderViewTranslatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateCartesianMinusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(-2.5 * M_PI/180.0,double3(1.0,0.0,0.0));
            RenderViewRotatePositionsCartesianCommand *translationCommand = new RenderViewRotatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateCartesianPlusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(2.5 * M_PI/180.0,double3(1.0,0.0,0.0));
            RenderViewRotatePositionsCartesianCommand *translationCommand = new RenderViewRotatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateCartesianMinusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(-2.5 * M_PI/180.0,double3(0.0,1.0,0.0));
            RenderViewRotatePositionsCartesianCommand *translationCommand = new RenderViewRotatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateCartesianPlusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(2.5 * M_PI/180.0,double3(0.0,1.0,0.0));
            RenderViewRotatePositionsCartesianCommand *translationCommand = new RenderViewRotatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateCartesianMinusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(-2.5 * M_PI/180.0,double3(0.0,0.0,1.0));
            RenderViewRotatePositionsCartesianCommand *translationCommand = new RenderViewRotatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateCartesianPlusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(2.5 * M_PI/180.0,double3(0.0,0.0,1.0));
            RenderViewRotatePositionsCartesianCommand *translationCommand = new RenderViewRotatePositionsCartesianCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}


void RenderStackedWidget::pressedTranslateBodyFrameMinusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(-0.1,0.0,0.0);
            RenderViewTranslatePositionsBodyFrameCommand *translationCommand = new RenderViewTranslatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateBodyFramePlusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.1,0.0,0.0);
            RenderViewTranslatePositionsBodyFrameCommand *translationCommand = new RenderViewTranslatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateBodyFrameMinusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,-0.1,0.0);
            RenderViewTranslatePositionsBodyFrameCommand *translationCommand = new RenderViewTranslatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateBodyFramePlusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,0.1,0.0);
            RenderViewTranslatePositionsBodyFrameCommand *translationCommand = new RenderViewTranslatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateBodyFrameMinusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,0.0,-0.1);
            RenderViewTranslatePositionsBodyFrameCommand *translationCommand = new RenderViewTranslatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedTranslateBodyFramePlusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            double3 translation = double3(0.0,0.0,0.1);
            RenderViewTranslatePositionsBodyFrameCommand *translationCommand = new RenderViewTranslatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, translation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateBodyFrameMinusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(-2.5 * M_PI/180.0,double3(1.0,0.0,0.0));
            RenderViewRotatePositionsBodyFrameCommand *translationCommand = new RenderViewRotatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateBodyFramePlusX()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(2.5 * M_PI/180.0,double3(1.0,0.0,0.0));
            RenderViewRotatePositionsBodyFrameCommand *translationCommand = new RenderViewRotatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateBodyFrameMinusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(-2.5 * M_PI/180.0,double3(0.0,1.0,0.0));
            RenderViewRotatePositionsBodyFrameCommand *translationCommand = new RenderViewRotatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateBodyFramePlusY()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(2.5 * M_PI/180.0,double3(0.0,1.0,0.0));
            RenderViewRotatePositionsBodyFrameCommand *translationCommand = new RenderViewRotatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateBodyFrameMinusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(-2.5 * M_PI/180.0,double3(0.0,0.0,1.0));
            RenderViewRotatePositionsBodyFrameCommand *translationCommand = new RenderViewRotatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::pressedRotateBodyFramePlusZ()
{
  if(std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock())
  {
    if(projectTreeNode->isEditable())
    {
      if(std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject())
      {
        if(std::shared_ptr<Project> project = iRASPAProject->project())
        {
          if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
          {
            simd_quatd rotation = simd_quatd::fromAxisAngle(2.5 * M_PI/180.0,double3(0.0,0.0,1.0));
            RenderViewRotatePositionsBodyFrameCommand *translationCommand = new RenderViewRotatePositionsBodyFrameCommand(_mainWindow, projectStructure, _iraspa_structures, rotation, nullptr);
            iRASPAProject->undoManager().push(translationCommand);
          }
        }
      }
    }
  }
}

void RenderStackedWidget::redraw()
{
  if(renderViewController)
  {
    renderViewController->redraw();
  }
}

void RenderStackedWidget::redrawWithQuality(RKRenderQuality quality)
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    widget->redrawWithQuality(quality);
  }
}


void RenderStackedWidget::invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures)
{
  std::vector<std::shared_ptr<RKRenderObject>> renderStructures{};
  for (const std::vector<std::shared_ptr<iRASPAObject>> &v : structures)
  {
    std::transform(v.begin(),v.end(),std::back_inserter(renderStructures),
                   [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::shared_ptr<RKRenderObject> {return iraspastructure->object();});
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    widget->invalidateCachedAmbientOcclusionTextures(renderStructures);
  }
}

void RenderStackedWidget::invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures)
{
  std::vector<std::shared_ptr<RKRenderObject>> renderStructures{};
  for (const std::vector<std::shared_ptr<iRASPAObject>> &v : structures)
  {
    std::transform(v.begin(),v.end(),std::back_inserter(renderStructures),
                   [](std::shared_ptr<iRASPAObject> iraspastructure) -> std::shared_ptr<RKRenderObject> {return iraspastructure->object();});
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(renderViewController))
  {
    widget->invalidateCachedIsosurfaces(renderStructures);
  }
}

void RenderStackedWidget::setControlPanel(bool iskeyAltOn)
{
  if(iskeyAltOn)
  {
    _controlPanel->setCurrentIndex(1);
  }
  else
  {
    _controlPanel->setCurrentIndex(0);
  }
}


void RenderStackedWidget::hideToolBarMenuMenu()
{
  if(toolBar)
  {
    toolBar->hide();
  }
}
void RenderStackedWidget::showToolBarMenuMenu()
{
  if(toolBar)
  {
     if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
     {
       if(project->hasSelectedObjects())
       {
         toolBar->move(mapToGlobal(QPoint(10,10)));
         toolBar->show();
         return;
       }
    }
    toolBar->hide();
  }
}

void RenderStackedWidget::updateControlPanel()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
     if(project->hasSelectedObjects())
     {
       showToolBarMenuMenu();
       return;
     }
  }

  hideToolBarMenuMenu();
}

void RenderStackedWidget::exportToPDB() const
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    //project->camera()->setCameraToPerspective();
    //emit updateCameraProjection();
  }
}

void RenderStackedWidget::exportToMMCIF() const
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    //project->camera()->setCameraToPerspective();
    //emit updateCameraProjection();
  }
}

void RenderStackedWidget::applySelectionDisplacement(const std::shared_ptr<RKRenderObject> &object, double3 shift)
{
  std::shared_ptr<SKAtomTreeController> controller;
  if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(object))
  {
    controller = atomViewer->atomsTreeController();
  }
  else if (std::shared_ptr<AtomEditor> atomEditor = std::dynamic_pointer_cast<AtomEditor>(object))
  {
    controller = atomEditor->atomsTreeController();
  }
  if (!controller)
  {
    return;
  }
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : controller->selectedObjects())
  {
    atom->setDisplacement(shift);
  }
}

double3 RenderStackedWidget::unprojectedSelectionShift(const QPoint &to, const QPoint &origin, double depth,
                                                       const std::shared_ptr<RKRenderObject> &object) const
{
  std::shared_ptr<RKCamera> camera = _camera.lock();
  if (!camera || !object || !object->cell())
  {
    return double3();
  }

  const QRect viewPort(0, 0, width(), height());
  const double3 toPoint(static_cast<double>(to.x()), static_cast<double>(viewPort.height() - to.y()), depth);
  const double3 originPoint(static_cast<double>(origin.x()), static_cast<double>(viewPort.height() - origin.y()), depth);
  const double4x4 modelMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(
      double4x4(object->orientation()), object->cell()->boundingBox().center());
  return camera->myGluUnProject(toPoint, modelMatrix, viewPort) - camera->myGluUnProject(originPoint, modelMatrix, viewPort);
}

void RenderStackedWidget::shiftSelection(const QPoint &to, const QPoint &origin, double depth)
{
  std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock();
  if (!projectTreeNode || !projectTreeNode->isEditable())
  {
    return;
  }

  for (const std::vector<std::shared_ptr<iRASPAObject>> &movie : _iraspa_structures)
  {
    for (const std::shared_ptr<iRASPAObject> &iraspaObject : movie)
    {
      if (!iraspaObject)
      {
        continue;
      }
      applySelectionDisplacement(iraspaObject->object(), unprojectedSelectionShift(to, origin, depth, iraspaObject->object()));
    }
  }
  reloadRenderData();
}

void RenderStackedWidget::finalizeShiftSelection(const QPoint &to, const QPoint &origin, double depth)
{
  std::shared_ptr<ProjectTreeNode> projectTreeNode = _projectTreeNode.lock();
  if (!projectTreeNode || !projectTreeNode->isEditable())
  {
    return;
  }
  std::shared_ptr<iRASPAProject> iRASPAProject = projectTreeNode->representedObject();
  if (!iRASPAProject)
  {
    return;
  }
  std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(iRASPAProject->project());
  if (!projectStructure)
  {
    return;
  }

  QUndoCommand *parentCommand = new QUndoCommand(QString("Translate atom selection"));
  bool any = false;
  for (const std::vector<std::shared_ptr<iRASPAObject>> &movie : _iraspa_structures)
  {
    for (const std::shared_ptr<iRASPAObject> &iraspaObject : movie)
    {
      if (!iraspaObject)
      {
        continue;
      }

      std::shared_ptr<SKAtomTreeController> controller;
      if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(iraspaObject->object()))
      {
        controller = atomViewer->atomsTreeController();
      }
      else if (std::shared_ptr<AtomEditor> atomEditor = std::dynamic_pointer_cast<AtomEditor>(iraspaObject->object()))
      {
        controller = atomEditor->atomsTreeController();
      }
      if (!controller || controller->selectedObjects().empty())
      {
        applySelectionDisplacement(iraspaObject->object(), double3());
        continue;
      }

      const double3 shift = unprojectedSelectionShift(to, origin, depth, iraspaObject->object());
      new RenderViewTranslatePositionsCartesianSubCommand(_mainWindow, iraspaObject, shift, parentCommand);
      any = true;
    }
  }

  if (any)
  {
    iRASPAProject->undoManager().push(parentCommand);
    for (const std::vector<std::shared_ptr<iRASPAObject>> &movie : _iraspa_structures)
    {
      for (const std::shared_ptr<iRASPAObject> &iraspaObject : movie)
      {
        if (iraspaObject)
        {
          applySelectionDisplacement(iraspaObject->object(), double3());
        }
      }
    }
    if (_mainWindow)
    {
      emit _mainWindow->invalidateCachedAmbientOcclusionTextures(projectStructure->sceneList()->invalidatediRASPAStructures());
      emit _mainWindow->invalidateCachedIsoSurfaces(projectStructure->sceneList()->invalidatediRASPAStructures());
      emit _mainWindow->rendererReloadData();
      _mainWindow->documentWasModified();
    }
  }
  else
  {
    delete parentCommand;
    reloadRenderData();
  }
}

void RenderStackedWidget::exportToCIF() const
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    //project->camera()->setCameraToPerspective();
    //emit updateCameraProjection();
  }
}

void RenderStackedWidget::exportToXYZ() const
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    //project->camera()->setCameraToPerspective();
    //emit updateCameraProjection();
  }
}

void RenderStackedWidget::exportToPOSCAR() const
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    //project->camera()->setCameraToPerspective();
    //emit updateCameraProjection();
  }
}
