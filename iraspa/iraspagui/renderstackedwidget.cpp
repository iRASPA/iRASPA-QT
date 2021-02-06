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
#include <QMenu>
#include <QObject>
#include <QAction>
#include <QtDebug>
#include <iraspakit.h>
#include "moviemaker.h"
#include "fixedaspectratiolayoutitem.h"
#include "renderviewdeleteselectioncommand.h"
#include "renderviewchangeselectioncommand.h"
#include "renderviewchangeselectionsubcommand.h"
#include "renderviewtranslatepositionscartesiancommand.h"
#include "renderviewtranslatepositionsbodyframecommand.h"
#include "renderviewrotatepositionscartesiancommand.h"
#include "renderviewrotatepositionsbodyframecommand.h"

RenderStackedWidget::RenderStackedWidget(QWidget* parent ): QStackedWidget(parent ),
    _rubberBand(new QRubberBand(QRubberBand::Rectangle, this))
{
  this->setContextMenuPolicy(Qt::PreventContextMenu);

  setMouseTracking(true);
}

void RenderStackedWidget::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  this->_projectTreeNode = projectTreeNode;
  this->_project.reset();


  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> render_structures{};
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
          _iraspa_structures = projectStructure->sceneList()->selectediRASPARenderStructures();

          for(std::vector<std::shared_ptr<iRASPAStructure>> v : _iraspa_structures)
          {
            std::vector<std::shared_ptr<RKRenderStructure>> r{};
            std::transform(v.begin(),v.end(),std::back_inserter(r),
                           [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});
            render_structures.push_back(r);
          }
        }
      }
    }
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->setRenderStructures(render_structures);
  }
}

void RenderStackedWidget::setLogReportingWidget(LogReporting *logReporting)
{
  _logReporter = logReporting;
}

void RenderStackedWidget::setSelectedRenderFrames(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> iraspa_structures)
{
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> renderStructures{};
  for(std::vector<std::shared_ptr<iRASPAStructure>> v : iraspa_structures)
  {
    std::vector<std::shared_ptr<RKRenderStructure>> r{};
    std::transform(v.begin(),v.end(),std::back_inserter(r),
                   [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});
    renderStructures.push_back(r);
  }

  _iraspa_structures = iraspa_structures;
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
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
  emit rendererWidgetResized();
}


bool RenderStackedWidget::eventFilter(QObject *obj, QEvent *event)
{
  Q_UNUSED(obj);
  if(event->type() == QEvent::MouseMove || event->type() == QEvent::Wheel)
  {
    emit updateCameraModelViewMatrix();
    emit updateCameraEulerAngles();
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

    if(isSHIFT)   // contains shift
    {
      // Using the shift key means a new selection is chosen. If later a drag occurs it is modified to 'draggedNewSelection'
      _tracking = Tracking::newSelection;
      _rubberBand->setGeometry(QRect(_origin, QSize()));
      _rubberBand->show();
    }
    else if(isCTRL)  // command, not option
    {
      // Using the command key means the selection is extended. If later a drag occurs it is modified to 'draggedAddToSelection'
      _tracking = Tracking::addToSelection;
      _rubberBand->setGeometry(QRect(_origin, QSize()));
      _rubberBand->show();
    }
    else if(isALT && isCTRL)  // option and command
    {
      _tracking = Tracking::translateSelection;
    }
    else if(isALT) // option, not command
    {
      _tracking = Tracking::measurement;
    }
    else
    {
      qDebug() << "Tracking::backgroundClick";
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
        _rubberBand->setGeometry(QRect(_origin, me->pos()).normalized());
        break;
      case Tracking::addToSelection:
        // convert to dragged version
        _tracking = Tracking::draggedAddToSelection;
        _rubberBand->setGeometry(QRect(_origin, me->pos()).normalized());
        break;
      case Tracking::draggedNewSelection:
        _rubberBand->setGeometry(QRect(_origin, me->pos()).normalized());
        break;
      case Tracking::draggedAddToSelection:
        _rubberBand->setGeometry(QRect(_origin, me->pos()).normalized());
        break;
      case Tracking::translateSelection:
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
    const QMouseEvent* const me = static_cast<const QMouseEvent*>( event );
    std::array<int,4> pixel{0,0,0,0};
    QRect rect = QRect(_origin, me->pos()).normalized();

    _rubberBand->hide();

    switch(_tracking)
    {
      case Tracking::newSelection:
        if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
        {
          pixel = widget->pickTexture(me->x(), me->y(), this->width(), this->height());

          int structureIdentifier = pixel[1];
          int movieIdentifier = pixel[2];
          int pickedObject  = pixel[3];

          switch(pixel[0])
          {
          case 1:
            _iraspa_structures[structureIdentifier][movieIdentifier]->structure()->setAtomSelection(pickedObject);
            reloadData();
            emit updateAtomSelection();
            emit updateBondSelection();
            break;
          case 2:
            _iraspa_structures[structureIdentifier][movieIdentifier]->structure()->setBondSelection(pickedObject);
            reloadData();
            emit updateBondSelection();
            break;
          default:
            break;
          }
        }
        _mainWindow->updateControlPanel();
        break;
      case Tracking::addToSelection:
        if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
        {
          pixel = widget->pickTexture(me->x(), me->y(), this->width(), this->height());
          int structureIdentifier = pixel[1];
          int movieIdentifier = pixel[2];
          int pickedObject  = pixel[3];
          switch(pixel[0])
          {
          case 1:
            _iraspa_structures[structureIdentifier][movieIdentifier]->structure()->toggleAtomSelection(pickedObject);
            reloadData();
            emit updateAtomSelection();
            emit updateBondSelection();
            break;
          case 2:
            _iraspa_structures[structureIdentifier][movieIdentifier]->structure()->toggleBondSelection(pickedObject);
            reloadData();
            emit updateBondSelection();
            break;
          default:
            break;
          }
        }
        _mainWindow->updateControlPanel();
        break;
      case Tracking::draggedNewSelection:
        if (std::shared_ptr<RKCamera> camera = _camera.lock())
        {
          selectAsymetricAtomsInRectangle(rect, false);
        }
        _mainWindow->updateControlPanel();
        break;
      case Tracking::draggedAddToSelection:
        if (std::shared_ptr<RKCamera> camera = _camera.lock())
        {
          selectAsymetricAtomsInRectangle(rect, true);
        }
        _mainWindow->updateControlPanel();
        break;
      case Tracking::translateSelection:
        break;
      case Tracking::measurement:
        break;
      case Tracking::backgroundClick:
        for(size_t i=0; i< _iraspa_structures.size();i++)
        {
          for(size_t j=0; j<_iraspa_structures[i].size(); j++)
          {
            _iraspa_structures[i][j]->structure()->clearSelection();
          }
        }
        if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
        {
          pixel = widget->pickTexture(me->x(), me->y(), this->width(), this->height());

          int structureIdentifier = pixel[1];
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
            _iraspa_structures[structureIdentifier][movieIdentifier]->structure()->setAtomSelection(pickedObject);
            reloadData();
            emit updateAtomSelection();
            emit updateBondSelection();
            break;
          case 2:
            _iraspa_structures[structureIdentifier][movieIdentifier]->structure()->setBondSelection(pickedObject);
            reloadData();
            emit updateBondSelection();
            break;
          default:
            break;
          }
        }
        _mainWindow->updateControlPanel();
        break;
      default:
        break;
    }
  }

  return QStackedWidget::eventFilter(obj, event);
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
            std::shared_ptr<SKAtomTreeController> atomTreeController = _iraspa_structures[i][j]->structure()->atomsTreeController();
            std::shared_ptr<SKBondSetController> bondSetController = _iraspa_structures[i][j]->structure()->bondSetController();

            AtomSelectionIndexPaths previousAtomSelection = atomTreeController->selectionIndexPaths();
            std::set<int> previousBondSelection = bondSetController->selectionIndexSet();

            std::set<int> indexSetSelectedAtoms = _iraspa_structures[i][j]->structure()->filterCartesianAtomPositions(closure);
            std::set<int> indexSetSelectedBonds = _iraspa_structures[i][j]->structure()->filterCartesianBondPositions(closure);

            if(extend)
            {
              _iraspa_structures[i][j]->structure()->addToAtomSelection(indexSetSelectedAtoms);
              _iraspa_structures[i][j]->structure()->bondSetController()->addSelectedObjects(indexSetSelectedBonds);
            }
            else
            {
              _iraspa_structures[i][j]->structure()->setAtomSelection(indexSetSelectedAtoms);
              _iraspa_structures[i][j]->structure()->bondSetController()->setSelectionIndexSet(indexSetSelectedBonds);
            }

            AtomSelectionIndexPaths atomSelection = atomTreeController->selectionIndexPaths();
            std::set<int> bondSelection = bondSetController->selectionIndexSet();

            new RenderViewChangeSelectionSubCommand(_iraspa_structures[i][j]->structure(), atomSelection, previousAtomSelection,
                                                    bondSelection, previousBondSelection, changeSelectionCommand);
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
  _mainWindow->updateControlPanel();
}


void RenderStackedWidget::ShowContextMenu(const QPoint &pos)
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    QMenu contextMenu(tr("Context menu"), this);

    QAction actionResetCameraDistance("Reset camera distance", this);
    connect(&actionResetCameraDistance, &QAction::triggered, this, &RenderStackedWidget::resetCameraDistance);
    contextMenu.addAction(&actionResetCameraDistance);

    QMenu* subMenuResetCamera = contextMenu.addMenu( "Reset camera to" );
    QActionGroup* cameraResetDirectionGroup = new QActionGroup(this);
    QAction actionResetToZ("Z-direction", this);
    cameraResetDirectionGroup->addAction(&actionResetToZ);
    subMenuResetCamera->addAction(&actionResetToZ);
    connect(&actionResetToZ, &QAction::triggered, this, &RenderStackedWidget::resetCameraToZ);
    QAction actionResetToY("Y-direction", this);
    subMenuResetCamera->addAction(&actionResetToY);
    cameraResetDirectionGroup->addAction(&actionResetToY);
    connect(&actionResetToY, &QAction::triggered, this, &RenderStackedWidget::resetCameraToY);
    QAction actionResetToX("X-direction", this);
    cameraResetDirectionGroup->addAction(&actionResetToX);
    connect(&actionResetToX, &QAction::triggered, this, &RenderStackedWidget::resetCameraToX);
    subMenuResetCamera->addAction(&actionResetToX);


    QMenu* subMenuCameraProjection = contextMenu.addMenu( "Camera projection" );
    QActionGroup* cameraProjectionGroup = new QActionGroup(this);
    QAction actionOrthographic("Orthographic", this);
    actionOrthographic.setCheckable(true);
    cameraProjectionGroup->addAction(&actionOrthographic);
    subMenuCameraProjection->addAction(&actionOrthographic);
    actionOrthographic.setChecked(project->camera()->isOrthographic());
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::setCameraToOrthographic);
    QAction actionPerspective("Perspective", this);
    actionPerspective.setCheckable(true);
    cameraProjectionGroup->addAction(&actionPerspective);
    subMenuCameraProjection->addAction(&actionPerspective);
    connect(&actionPerspective, &QAction::triggered, this, &RenderStackedWidget::setCameraToPerspective);
    actionPerspective.setChecked(project->camera()->isPerspective());

    QAction _actionShowBoundingBox("Show bounding box", this);
    _actionShowBoundingBox.setCheckable(true);
    _actionShowBoundingBox.setChecked(project->showBoundingBox());
    connect(&_actionShowBoundingBox, &QAction::toggled, this, &RenderStackedWidget::showBoundingBox);
    contextMenu.addAction(&_actionShowBoundingBox);

    QAction _actionComputeAOHighQuality("Compute AO high quality", this);
    connect(&_actionComputeAOHighQuality, &QAction::triggered, this, &RenderStackedWidget::computeAOHighQuality);
    contextMenu.addAction(&_actionComputeAOHighQuality);

    QMenu* subMenuExport = contextMenu.addMenu( "Export to" );
    QActionGroup* exportToGroup = new QActionGroup(this);
    QAction actionExportToPDB("PDB", this);
    exportToGroup->addAction(&actionExportToPDB);
    subMenuExport->addAction(&actionExportToPDB);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToPDB);
    QAction actionExportToMMCIF("mmCIF", this);
    exportToGroup->addAction(&actionExportToMMCIF);
    subMenuExport->addAction(&actionExportToMMCIF);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToMMCIF);
    QAction actionExportToCIF("CIF", this);
    exportToGroup->addAction(&actionExportToCIF);
    subMenuExport->addAction(&actionExportToCIF);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToCIF);
    QAction actionExportToXYZ("XYZ", this);
    exportToGroup->addAction(&actionExportToXYZ);
    subMenuExport->addAction(&actionExportToXYZ);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToXYZ);
    QAction actionExportToPOSCAR("VASP POSCAR", this);
    exportToGroup->addAction(&actionExportToPOSCAR);
    subMenuExport->addAction(&actionExportToPOSCAR);
    connect(&actionOrthographic, &QAction::triggered, this, &RenderStackedWidget::exportToPOSCAR);

    contextMenu.exec(mapToGlobal(pos));
  }
}


// reload-data but also injects the selected frames
void RenderStackedWidget::reloadRenderData()
{
  _iraspa_structures = _project.lock()->sceneList()->selectediRASPARenderStructures();

  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> render_structures{};
  for(std::vector<std::shared_ptr<iRASPAStructure>> v : _iraspa_structures)
  {
    std::vector<std::shared_ptr<RKRenderStructure>> r{};
    std::transform(v.begin(),v.end(),std::back_inserter(r),
                   [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});
    render_structures.push_back(r);
  }

  qDebug() << "RenderStackedWidget::reloadRenderData(): " << render_structures.size();
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->setRenderStructures(render_structures);
    widget->reloadRenderData();
  }
}

void RenderStackedWidget::resetData()
{
  std::vector<std::vector<std::shared_ptr<RKRenderStructure>>> render_structures{};

  for(std::vector<std::shared_ptr<iRASPAStructure>> v : _iraspa_structures)
  {
    std::vector<std::shared_ptr<RKRenderStructure>> r{};
    std::transform(v.begin(),v.end(),std::back_inserter(r),
                   [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});
    render_structures.push_back(r);
  }

  if(std::shared_ptr<ProjectStructure> project = _project.lock())
  {
    SKBoundingBox box = project->renderBoundingBox();
    project->camera()->resetForNewBoundingBox(box);
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->setRenderStructures(render_structures);
    widget->reloadData();
  }
  update();
}

void RenderStackedWidget::reloadData()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->reloadData();
  }
  update();
}

void RenderStackedWidget::reloadAmbientOcclusionData()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->reloadAmbientOcclusionData();
  }
  update();
}

void RenderStackedWidget::reloadSelectionData()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->reloadData();
  }
  update();
}

void RenderStackedWidget::reloadBackgroundImage()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->reloadBackgroundImage();
  }
  update();
}


void RenderStackedWidget::resetCameraDistance()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
     project->camera()->resetCameraDistance();
     emit updateCameraModelViewMatrix();
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
  }
}

void RenderStackedWidget::setCameraToOrthographic()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->camera()->setCameraToOrthographic();
    emit updateCameraProjection();
  }
}

void RenderStackedWidget::setCameraToPerspective()
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->camera()->setCameraToPerspective();
    emit updateCameraProjection();
  }
}

void RenderStackedWidget::showBoundingBox(bool checked)
{
  if(std::shared_ptr<ProjectStructure> project = this->_project.lock())
  {
    project->setShowBoundingBox(checked);
    emit reloadData();
  }
}

void RenderStackedWidget::computeAOHighQuality()
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    // invalidate all lower quality caches
    if(std::shared_ptr<ProjectStructure> project = _project.lock())
    {
      invalidateCachedAmbientOcclusionTextures(project->sceneList()->allIRASPAStructures());
    }
    qDebug() << "computeAOHighQuality";
    widget->reloadData(RKRenderQuality::picture);
    widget->redraw();
  }
}




void RenderStackedWidget::createPicture(QUrl fileURL, int width, int height)
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    // invalidate all lower quality caches
    if(std::shared_ptr<ProjectStructure> project = _project.lock())
    {
      invalidateCachedAmbientOcclusionTextures(project->sceneList()->allIRASPAStructures());
    }
    std::cout << "Saving file: " << fileURL.path().toStdString() << std::endl;
    QImage image = widget->renderSceneToImage(width, height, RKRenderQuality::picture);
    image.save(fileURL.toLocalFile());
  }
}

int nearestEvenInt(int to)
{
  return (to % 2 == 0) ? to : (to + 1);
}

void RenderStackedWidget::createMovie(QUrl fileURL, int width, int height, MovieWriter::Type type)
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    if (std::shared_ptr<ProjectStructure> project = _project.lock())
    {
      MovieWriter movie(nearestEvenInt(width), nearestEvenInt(height), project->movieFramesPerSecond(), _logReporter, type);
      int ret = movie.initialize(fileURL.toLocalFile().toStdString());
      if (ret < 0)
      {
        _logReporter->logMessage(LogReporting::ErrorLevel::error, "failed to create movie");
        return;
      }

      int numberOfFrames = project->maxNumberOfMoviesFrames();
      for (int iframe = 0; iframe < numberOfFrames; iframe++)
      {
        project->sceneList()->setSelectedFrameIndex(iframe);
        _mainWindow->propagateProject(_projectTreeNode.lock(), _mainWindow);
        widget->reloadData();
        QImage image = widget->renderSceneToImage(nearestEvenInt(width), nearestEvenInt(height), RKRenderQuality::picture);

        movie.addFrame(image.bits(), iframe);
      }
      movie.finalize();
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
  currentWidget()->update();
}

void RenderStackedWidget::redrawWithQuality(RKRenderQuality quality)
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->redrawWithQuality(quality);
  }
}


void RenderStackedWidget::invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures)
{
  qDebug() << "invalidateCachedAmbientOcclusionTextures: " << structures.size();
  std::vector<std::shared_ptr<RKRenderStructure>> renderStructures{};
  for (const std::vector<std::shared_ptr<iRASPAStructure>> &v : structures)
  {
    std::transform(v.begin(),v.end(),std::back_inserter(renderStructures),
                   [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->invalidateCachedAmbientOcclusionTextures(renderStructures);
  }
}

void RenderStackedWidget::invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures)
{
  std::vector<std::shared_ptr<RKRenderStructure>> renderStructures{};
  for (const std::vector<std::shared_ptr<iRASPAStructure>> &v : structures)
  {
    std::transform(v.begin(),v.end(),std::back_inserter(renderStructures),
                   [](std::shared_ptr<iRASPAStructure> iraspastructure) -> std::shared_ptr<RKRenderStructure> {return iraspastructure->structure();});
  }

  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->invalidateCachedIsosurfaces(renderStructures);
  }
}


void RenderStackedWidget::computeHeliumVoidFraction(std::vector<std::shared_ptr<RKRenderStructure>> structures)
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->computeHeliumVoidFraction(structures);
  }
}

void RenderStackedWidget::computeNitrogenSurfaceArea(std::vector<std::shared_ptr<RKRenderStructure>> structures)
{
  if (RKRenderViewController* widget = dynamic_cast<RKRenderViewController*>(currentWidget()))
  {
    widget->computeNitrogenSurfaceArea(structures);
  }
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
