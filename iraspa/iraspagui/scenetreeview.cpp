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

#include "scenetreeview.h"
#include "scenetreeviewstyleditemdelegate.h"
#include "scenetreeviewproxystyle.h"
#include "scenetreeviewinsertmoviecommand.h"
#include "scenetreeviewinsertinitialscenecommand.h"
#include "scenetreeviewdeleteselectioncommand.h"
#include "scenetreeviewchangeselectioncommand.h"
#include <QModelIndexList>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QClipboard>
#include <map>
#include <unordered_set>

SceneTreeView::SceneTreeView(QWidget* parent): QTreeView(parent ), _model(std::make_shared<SceneTreeViewModel>())
{
  this->setModel(_model.get());

  this->setSelectionBehavior (QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  //this->viewport()->setMouseTracking(true);

  this->setRootIsDecorated(false);
  this->setItemsExpandable(false);
  this->setHeaderHidden(true);
  this->setIndentation(4);

  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  setDragDropMode(DragDropMode::DragDrop);
  setDragDropOverwriteMode(false);

  this->setAttribute(Qt::WA_MacShowFocusRect, false);
  this->setStyleSheet("background-color:rgb(240, 240, 240);");
  this->setStyle(new SceneTreeViewProxyStyle());

  this->setItemDelegateForColumn(0,new SceneTreeViewStyledItemDelegate(this));

  QObject::connect(_model.get(), &SceneTreeViewModel::updateSelection, this, &SceneTreeView::reloadSelection);
  QObject::connect(_model.get(), &SceneTreeViewModel::expandAll, this, &SceneTreeView::expandAll);
  QObject::connect(_model.get(), &SceneTreeViewModel::invalidateCachedAmbientOcclusionTexture, this, &SceneTreeView::invalidateCachedAmbientOcclusionTexture);
  QObject::connect(_model.get(), &SceneTreeViewModel::rendererReloadData, this, &SceneTreeView::rendererReloadData);
}

void SceneTreeView::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
  _model->setMainWindow(mainWindow);
}

void SceneTreeView::setFrameModel(std::shared_ptr<FrameListViewModel> model)
{
  _frameModel = model;
  _model->setFrameModel(model);
}

void SceneTreeView::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _projectTreeNode = projectTreeNode;
  if (projectTreeNode)
  {
    setDragEnabled(projectTreeNode->isEditable());
    setAcceptDrops(projectTreeNode->isEditable());

    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if(std::shared_ptr<Project> project = iraspaProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          _sceneList = projectStructure->sceneList();
          _model->setProject(projectTreeNode);
          expandAll();
          return;
        }
      }
    }
  }

  // if no project is selected or the project is not of type 'ProjectStructure'
  setDragEnabled(false);
  setAcceptDrops(false);
  _sceneList = nullptr;
  _model->setProject(nullptr);
}


QSize SceneTreeView::sizeHint() const
{
  return QSize(200, 800);
}

void SceneTreeView::TabItemWasSelected()
{
  this->reloadData();
  this->reloadSelection();
}

// reload the selection without side-effects
// i.e. just sets the selection of the treeview to the selection of the model
void SceneTreeView::reloadSelection()
{
  qDebug() << "SceneTreeView::reloadSelection()";
  expandAll();
  if(_sceneList)
  {
    whileBlocking(selectionModel())->clearSelection();

    std::set<std::shared_ptr<Scene>> selectedScenes = _sceneList->selectedScenes();
    for(std::shared_ptr<Scene> scene: selectedScenes)
    {
      std::optional<int> sceneIndex = _sceneList->findChildIndex(scene);
      if(sceneIndex)
      {
        QModelIndex parentItem = model()->index(*sceneIndex,0,QModelIndex());
        std::set<std::shared_ptr<Movie>> selectedMovies = scene->selectedMovies();
        for(const std::shared_ptr<Movie> &movie: selectedMovies)
        {
          std::optional<int> movieIndex = scene->findChildIndex(movie);
          if(movieIndex)
          {
            QModelIndex selectedMovie = model()->index(*movieIndex,0, parentItem);
            whileBlocking(selectionModel())->select(selectedMovie, QItemSelectionModel::Select);
          }
        }
      }
    }
    qDebug() << "emmiting " << _sceneList->selectediRASPARenderStructures().size();
    emit setSelectedRenderFrames(_sceneList->selectediRASPARenderStructures());
    emit setFlattenedSelectedFrames(_sceneList->selectedMoviesIRASPAStructures());

    std::optional<int> sceneIndex = _sceneList->selectedSceneIndex();
    if(sceneIndex)
    {
      qDebug() << "sceneIndex" << *sceneIndex;
      QModelIndex parentItem = model()->index(*sceneIndex,0,QModelIndex());

      if(std::shared_ptr<Scene> selectedScene = _sceneList->selectedScene())
      {
        std::shared_ptr<Movie> movie =  selectedScene->selectedMovie();
        std::optional<int> movieIndex = selectedScene->selectMovieIndex();

        if(movieIndex)
        {
          QModelIndex selectedMovie = model()->index(*movieIndex,0,parentItem);

          whileBlocking(selectionModel())->select(selectedMovie, QItemSelectionModel::Select);

          // set currentIndex for keyboard navigation
          whileBlocking(selectionModel())->setCurrentIndex(selectedMovie, QItemSelectionModel::SelectionFlag::Current);

          emit setSelectedMovie(movie);
          emit setSelectedFrame(movie->frameAtIndex(_sceneList->selectedFrameIndex()));
        }
      }
    }
    else
    {
      qDebug() << "empty selection";
      _mainWindow->recheckRemovalButtons();
      emit setSelectedMovie(nullptr);
      emit setSelectedFrame(nullptr);
      emit setSelectedRenderFrames(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>>{});
      emit setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAStructure>>{});
    }
  }
  update();
}

void SceneTreeView::reloadData()
{
  _mainWindow->updateMenuToSceneTab();
}

void SceneTreeView::keyPressEvent(QKeyEvent *event)
{
  if( event->matches(QKeySequence::Delete))
  {
    deleteSelection();
  }
  else if(event->matches(QKeySequence::Copy))
  {
    copy();
  }
  else if(event->matches(QKeySequence::Paste))
  {
    paste();
  }
  else if (event->matches(QKeySequence::Cut))
  {
    cut();
  }
  else
  {
    QTreeView::keyPressEvent(event);
  }
}

void SceneTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  QAbstractItemView::selectionChanged(selected, deselected);

  // avoid empty selection
  if (selectedIndexes().isEmpty())
  {
    reloadSelection();
    return;
  }

  SceneListSelection previousSelection = _sceneList->allSelection();

  if(_sceneList)
  {
    // clear scene-movie selection
    _sceneList->selectedScenes().clear();
    for(std::shared_ptr<Scene> scene : _sceneList->scenes())
    {
      scene->selectedMovies().clear();
    }

    if(selectedIndexes().size() == 1)
    {
      QModelIndex current = selectedIndexes().front();

      DisplayableProtocol *currentItem = static_cast<DisplayableProtocol*>(current.internalPointer());
      if(Movie* movie = dynamic_cast<Movie*>(currentItem))
      {
        QModelIndex parentItem = current.parent();
        DisplayableProtocol *item = static_cast<DisplayableProtocol*>(parentItem.internalPointer());

        if(Scene* scene = dynamic_cast<Scene*>(item))
        {
          _sceneList->setSelectedScene(scene->shared_from_this());
          scene->setSelectedMovie(movie->shared_from_this());
        }

        // propagates the selected movie to the frame-view
        emit setSelectedMovie(movie->shared_from_this());

        // propagates the selected frame to the atom and bond-views
        emit setSelectedFrame(movie->frameAtIndex(_sceneList->selectedFrameIndex()));
      }
    }

    // loop over all selected indexes
    for(QModelIndex index : selectedIndexes())
    {
      DisplayableProtocol *currentItem = static_cast<DisplayableProtocol*>(index.internalPointer());
      if(Movie* movie = dynamic_cast<Movie*>(currentItem))
      {
        QModelIndex parentItem = index.parent();
        DisplayableProtocol *item = static_cast<DisplayableProtocol*>(parentItem.internalPointer());

        if(Scene* scene = dynamic_cast<Scene*>(item))
        {
          _sceneList->selectedScenes().insert(scene->shared_from_this());
          scene->selectedMovies().insert(movie->shared_from_this());
        }
      }
    }

    // set currentIndex for keyboard navigation
    SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(model());
    QModelIndex selectedIndex = pModel->indexOfMainSelected();
    if (selectedIndex.isValid())
    {
      selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::SelectionFlag::Current);
    }

    // propagates the selected-frames to the renderer
    emit setSelectedRenderFrames(_sceneList->selectediRASPARenderStructures());

    // propagates the structures to the info, appearance, and cell-tabs
    emit setFlattenedSelectedFrames(_sceneList->selectedMoviesIRASPAStructures());

    emit updateRenderer();

    SceneListSelection currentSelection = _sceneList->allSelection();
    SceneChangeSelectionCommand *changeSelectionCommand = new SceneChangeSelectionCommand(_mainWindow, this, currentSelection, previousSelection,  nullptr);
    _projectTreeNode->representedObject()->undoManager().push(changeSelectionCommand);

    update();
  }
}




QString SceneTreeView::nameOfItem(const QModelIndex &current)
{
  DisplayableProtocol *currentItem = static_cast<DisplayableProtocol*>(current.internalPointer());

  if(Movie* movie = dynamic_cast<Movie*>(currentItem))
  {
    QModelIndex parentItem = current.parent();
    DisplayableProtocol *item = static_cast<DisplayableProtocol*>(parentItem.internalPointer());

    if(Scene* scene = dynamic_cast<Scene*>(item))
    {
      _sceneList->setSelectedScene(scene->shared_from_this());
      scene->setSelectedMovie(movie->shared_from_this());
      return movie->displayName();
    }
  }
  return QString("unknown");
}

QPixmap SceneTreeView::selectionToPixmap()
{
  QModelIndexList selectionIndexes = selectionModel()->selectedRows();

  QFontMetrics fontMetrics = QFontMetrics(this->font());

  if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(model()))
  {
    int maxWidth=0;
    int height=4;

    for(QModelIndex index: selectionIndexes)
    {
      if(DisplayableProtocol *item = pModel->getItem(index))
      {
        QString text = item->displayName();
        QRect textBoundingRect = fontMetrics.boundingRect(text);
        if(textBoundingRect.width() > maxWidth) maxWidth = textBoundingRect.size().width();
        height += textBoundingRect.size().height();
      }
    }

    QRect rect = QRect(QPoint(0,0), QSize(maxWidth, height));
    QPixmap pix = QPixmap(maxWidth, height);
    pix.fill(Qt::transparent);

    QPainter painter( &pix );
    painter.setFont(this->font());
    painter.setOpacity(1.0);
    painter.setPen(Qt::black);

    int currentHeight=0;
    for(QModelIndex index: selectionIndexes)
    {
      if(DisplayableProtocol* item = pModel->getItem(index))
      {
        QString text = item->displayName();
        QRect fontRect = fontMetrics.boundingRect(text);
        currentHeight += fontRect.size().height();

        painter.save();
        painter.translate(QPointF(0,currentHeight));
        painter.drawText(fontRect,Qt::AlignLeft|Qt::AlignCenter, text);
        painter.restore();
      }
    }
    return pix;
  }

  return QPixmap();
}

void SceneTreeView::startDrag(Qt::DropActions supportedActions)
{
  if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(model()))
  {
    QMimeData* mimeData = pModel->mimeDataLazy(selectedIndexes());

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(selectionToPixmap());

    if(drag->exec(supportedActions))
    {
      reloadSelection();
    }
  }
}

void SceneTreeView::dragMoveEvent(QDragMoveEvent* event)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  QModelIndex index = indexAt(event->pos());
#else
  QModelIndex index = indexAt(event->position().toPoint());
#endif

  // use the visualRect of the index to avoid dropping on the background left to items
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  if (index.isValid() && !visualRect(index).contains(event->pos()))
#else
  if (index.isValid() && !visualRect(index).contains(event->position().toPoint()))
#endif
  {
    event->ignore();
  }
  else
  {
    event->accept();
  }

  QTreeView::dragMoveEvent(event);
}



void SceneTreeView::newCrystal()
{
  std::shared_ptr<Crystal> crystal = std::make_shared<Crystal>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  crystal->atomsTreeController()->appendToRootnodes(atomtreeNode);
  crystal->expandSymmetry();

  crystal->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  crystal->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(crystal);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newMolecularCrystal()
{
  std::shared_ptr<MolecularCrystal> molecularCrystal = std::make_shared<MolecularCrystal>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  molecularCrystal->atomsTreeController()->appendToRootnodes(atomtreeNode);
  molecularCrystal->expandSymmetry();

  molecularCrystal->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  molecularCrystal->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(molecularCrystal);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newProteinCrystal()
{
  std::shared_ptr<ProteinCrystal> proteinCrystal = std::make_shared<ProteinCrystal>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  proteinCrystal->atomsTreeController()->appendToRootnodes(atomtreeNode);
  proteinCrystal->expandSymmetry();

  proteinCrystal->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  proteinCrystal->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(proteinCrystal);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newMolecule()
{
  std::shared_ptr<Molecule> molecule = std::make_shared<Molecule>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  molecule->atomsTreeController()->appendToRootnodes(atomtreeNode);
  molecule->expandSymmetry();

  molecule->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  molecule->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(molecule);

  insertMovie(iraspaStructure);
}


void SceneTreeView::newProtein()
{
  std::shared_ptr<Protein> protein = std::make_shared<Protein>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  protein->atomsTreeController()->appendToRootnodes(atomtreeNode);
  protein->expandSymmetry();

  protein->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  protein->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(protein);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newCrystalEllipsoid()
{
  std::shared_ptr<CrystalEllipsoidPrimitive> crystalEllipsoid = std::make_shared<CrystalEllipsoidPrimitive>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  crystalEllipsoid->atomsTreeController()->appendToRootnodes(atomtreeNode);
  crystalEllipsoid->expandSymmetry();

  crystalEllipsoid->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  crystalEllipsoid->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(crystalEllipsoid);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newCrystalCylinder()
{
  std::shared_ptr<CrystalCylinderPrimitive> crystalCylinder = std::make_shared<CrystalCylinderPrimitive>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  crystalCylinder->atomsTreeController()->appendToRootnodes(atomtreeNode);
  crystalCylinder->expandSymmetry();

  crystalCylinder->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  crystalCylinder->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(crystalCylinder);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newCrystalPolygonalPrism()
{
  std::shared_ptr<CrystalPolygonalPrismPrimitive> crystalPolygonalPrismPrimitive = std::make_shared<CrystalPolygonalPrismPrimitive>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  crystalPolygonalPrismPrimitive->atomsTreeController()->appendToRootnodes(atomtreeNode);
  crystalPolygonalPrismPrimitive->expandSymmetry();

  crystalPolygonalPrismPrimitive->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  crystalPolygonalPrismPrimitive->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(crystalPolygonalPrismPrimitive);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newEllipsoid()
{
  std::shared_ptr<EllipsoidPrimitive> ellipsoidPrimitive = std::make_shared<EllipsoidPrimitive>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  ellipsoidPrimitive->atomsTreeController()->appendToRootnodes(atomtreeNode);
  ellipsoidPrimitive->expandSymmetry();

  ellipsoidPrimitive->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  ellipsoidPrimitive->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(ellipsoidPrimitive);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newCylinder()
{
  std::shared_ptr<CylinderPrimitive> cylinderPrimitive = std::make_shared<CylinderPrimitive>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  cylinderPrimitive->atomsTreeController()->appendToRootnodes(atomtreeNode);
  cylinderPrimitive->expandSymmetry();

  cylinderPrimitive->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  cylinderPrimitive->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(cylinderPrimitive);

  insertMovie(iraspaStructure);
}

void SceneTreeView::newPolygonalPrism()
{
  std::shared_ptr<PolygonalPrismPrimitive> polygonalPrismPrimitive = std::make_shared<PolygonalPrismPrimitive>();
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  std::shared_ptr<SKAtomTreeNode> atomtreeNode = std::make_shared<SKAtomTreeNode>(atom);
  polygonalPrismPrimitive->atomsTreeController()->appendToRootnodes(atomtreeNode);
  polygonalPrismPrimitive->expandSymmetry();

  polygonalPrismPrimitive->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, _mainWindow->colorSets());
  polygonalPrismPrimitive->setAtomForceFieldIdentifier("Default", _mainWindow->forceFieldSets());

  std::shared_ptr<iRASPAStructure> iraspaStructure = std::make_shared<iRASPAStructure>(polygonalPrismPrimitive);

  insertMovie(iraspaStructure);
}



void SceneTreeView::insertMovie(std::shared_ptr<iRASPAStructure> iraspaStructure)
{
  static int number = 0;
  if(_sceneList)
  {
    if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(model()))
    {
      if(selectedIndexes().empty())
      {
        std::shared_ptr<Movie> movie = Movie::create(iraspaStructure);
        std::shared_ptr<Scene> scene = Scene::create(movie);
        scene->setSelectedMovie(movie);
        std::map<std::shared_ptr<Scene>, std::unordered_set<std::shared_ptr<Movie>>> selection = {{scene, {movie}}};
        SceneTreeViewInsertInitialSceneCommand *insertSceneCommand = new SceneTreeViewInsertInitialSceneCommand(_mainWindow, this, _sceneList, scene, movie, 0, nullptr);
        _projectTreeNode->representedObject()->undoManager().push(insertSceneCommand);
      }
      else
      {
        QModelIndex current = currentIndex();

        DisplayableProtocol *item = pModel->getItem(current);

        if(Movie* selectedMovie = dynamic_cast<Movie*>(item))
        {
          if(std::shared_ptr<Scene> scene = pModel->parentForMovie(selectedMovie->shared_from_this()))
          {
            int row = current.row() + 1;

            std::shared_ptr<Movie> movie = Movie::create(iraspaStructure);
            movie->setDisplayName(QString::number(number));
            number++;

            if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
            {
              SceneTreeViewInsertMovieCommand *insertMovieCommand = new SceneTreeViewInsertMovieCommand(_mainWindow, this, _frameModel, projectStructure, _sceneList, movie, scene, row, _sceneList->allSelection(), nullptr);
              _projectTreeNode->representedObject()->undoManager().push(insertMovieCommand);
            }
          }
        }
      }
    }
  }
}


void SceneTreeView::restoreSelection(SceneListSelection selection)
{
  _sceneList->setSelection(selection);
}


void SceneTreeView::deleteSelection()
{
  if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(model()))
  {
    if(!selectedIndexes().empty())
    {
      std::vector<std::shared_ptr<Movie>> selectedMovies{};
      for(QModelIndex index : selectedIndexes())
      {
        DisplayableProtocol *item = pModel->getItem(index);

        if(Movie *movie = dynamic_cast<Movie*>(item))
        {
          selectedMovies.push_back(movie->shared_from_this());
        }
      }

      if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(_projectTreeNode->representedObject()->project()))
      {
        SceneTreeViewDeleteSelectionCommand *deleteSelectionCommand = new SceneTreeViewDeleteSelectionCommand(_mainWindow, this, _frameModel, projectStructure,
                                                                                                              _sceneList, selectedMovies, _sceneList->allSelection(), nullptr);
        _projectTreeNode->representedObject()->undoManager().push(deleteSelectionCommand);
      }
    }
  }
}

void SceneTreeView::copy()
{
  if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(model()))
  {
    if(QClipboard *clipboard = QApplication::clipboard())
    {
      QModelIndexList indexes = this->selectionModel()->selectedIndexes();

      if(indexes.isEmpty())
        return;

      std::sort(indexes.begin(), indexes.end());

      QMimeData *mimeData = pModel->mimeData(indexes);
      clipboard->setMimeData(mimeData);
    }
  }
}

void SceneTreeView::paste()
{
  if(SceneTreeViewModel* pModel = qobject_cast<SceneTreeViewModel*>(model()))
  {
    if(qApp->clipboard()->mimeData()->hasFormat(Movie::mimeType))
    {
      const QMimeData *mimeData = qApp->clipboard()->mimeData();

      QModelIndex currentIndex = selectionModel()->currentIndex();
      if(currentIndex.isValid())
      {
        if(pModel->canDropMimeData(mimeData,Qt::CopyAction, currentIndex.row() + 1, currentIndex.column(), currentIndex.parent()))
        {
          qDebug() << "pasting movie";
          pModel->pasteMimeData(mimeData, currentIndex.row() + 1, currentIndex.column(), currentIndex.parent());
        }
        else
        {
          //if(Movie *movie = pModel->getItem(currentIndex.parent()))
          //{
          //  _logReporting->logMessage(LogReporting::ErrorLevel::warning, "Not allowed to paste into container " + projectTreeNode->displayName());
          //}
        }
      }
      else
      {
        // case: nothing present or selected yet
        if(pModel->canDropMimeData(mimeData,Qt::CopyAction, 0, 0, QModelIndex()))
        {
          pModel->pasteMimeData(mimeData, 0, 0, QModelIndex());
        }
      }
    }
  }
}

void SceneTreeView::cut()
{
  copy();
  deleteSelection();
}


