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

#include "framelistview.h"
#include "frameliststyleditemdelegate.h"
#include "framelistviewproxystyle.h"
#include "framelistviewinsertcommand.h"
#include "framelistviewchangeselectioncommand.h"
#include "framelistviewdeleteselectioncommand.h"
#include <optional>
#include <algorithm>

QSize FrameListView::sizeHint() const
{
  return QSize(200, 800);
}

void FrameListView::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
  _model->setMainWindow(mainWindow);
}

FrameListView::FrameListView(QWidget* parent): QListView(parent ), _model(std::make_shared<FrameListViewModel>())
{
  this->setModel(_model.get());

  this->viewport()->setMouseTracking(true);

  this->setSelectionBehavior (QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);

  this->setAttribute(Qt::WA_MacShowFocusRect, false);
  this->setStyleSheet("background-color:rgb(240, 240, 240);");
  this->setStyle(new FrameListViewProxyStyle());

  this->setItemDelegateForColumn(0,new FrameListViewStyledItemDelegate(this));

  this->setDragEnabled(true);
  this->setAcceptDrops(true);
  this->setDropIndicatorShown(true);
  this->setDragDropMode(DragDropMode::DragDrop);
  this->setDragDropOverwriteMode(false);

  QObject::connect(_model.get(), &FrameListViewModel::updateSelection, this, &FrameListView::reloadSelection);
  QObject::connect(_model.get(), &FrameListViewModel::invalidateCachedAmbientOcclusionTexture, this, &FrameListView::invalidateCachedAmbientOcclusionTexture);
  QObject::connect(_model.get(), &FrameListViewModel::rendererReloadData, this, &FrameListView::rendererReloadData);
}

void FrameListView::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _projectTreeNode = projectTreeNode;
  _sceneList = nullptr;
  if (projectTreeNode)
  {
    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if(std::shared_ptr<Project> project = iraspaProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          _model->setProject(projectTreeNode);
          _sceneList = projectStructure->sceneList();
          std::shared_ptr<Scene> scene = _sceneList->selectedScene();
          _movie = scene ? scene->selectedMovie() : nullptr;
          _model->setMovie(_movie);
          return;
        }
      }
    }
  }

  // if no project is selected or the project is not of type 'ProjectStructure'
  _sceneList = nullptr;
  _movie = nullptr;
  _model->setMovie(nullptr);
}

void FrameListView::setSelectedMovie(std::shared_ptr<Movie> movie)
{
 _movie = movie;
 _model->setMovie(_movie);
}

void FrameListView::TabItemWasSelected()
{
  qDebug() << " FrameListView::TabItemWasSelected";
  this->reloadData();
  this->reloadSelection();

  if(_sceneList)
  {
    // propagates the selected frame to the atom and bond-views
    int selectedFrameIndex = _sceneList->selectedFrameIndex();

    if(_movie)
    {
      std::shared_ptr<iRASPAStructure> iraspa_structure = _movie->frameAtIndex(selectedFrameIndex);
      emit setSelectedFrame(iraspa_structure);
    }

    // propagates the selected-frames to the renderer
    emit setSelectedRenderFrames(_sceneList->selectediRASPARenderStructures());

    // propagates the structures to the info, appearance, and cell-tabs
    emit setFlattenedSelectedFrames(_movie->selectedFramesiRASPAStructures());

    emit rendererReloadData();
  }

  update();
}

// reload the selection without side-effects
// i.e. just sets the selection of the listview to the selection of the model
void FrameListView::reloadSelection()
{
  qDebug() << "FrameListView::reloadSelection()";
  if (_movie)
  {
    whileBlocking(selectionModel())->clearSelection();

    for (int frameIndex : _movie->selectedFramesIndexSet())
    {
      QModelIndex item = model()->index(frameIndex, 0, QModelIndex());
      whileBlocking(selectionModel())->select(item, QItemSelectionModel::Select);
    }

    int selectedIndex = _sceneList->selectedFrameIndex();
    QModelIndex item = model()->index(selectedIndex, 0, QModelIndex());

    if(iRASPAStructure* iraspa_structure = static_cast<iRASPAStructure*>(item.internalPointer()))
    {
      qDebug() << "CHECK: " << iraspa_structure->structure()->displayName();
    }

    whileBlocking(selectionModel())->select(item, QItemSelectionModel::Select);
    whileBlocking(selectionModel())->setCurrentIndex(item, QItemSelectionModel::SelectionFlag::Current);

    update();
  }
}

void FrameListView::reloadData()
{
  _mainWindow->updateMenuToFrameTab();
}

bool FrameListView::isMainSelectedItem(size_t row)
{
  if(_sceneList)
  {
    return (std::min(_sceneList->selectedFrameIndex(), _movie->frames().size()-1) == row);
  }
  return false;
}


void FrameListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  QAbstractItemView::selectionChanged(selected, deselected);

  // avoid empty selection
  if (selectedIndexes().isEmpty())
  {
    reloadSelection();
    return;
  }

  if(_movie)
  {
    if(_movie->selectedFramesIndexSet().size() > 1)
    {
      // multiple frames selected, so the ambient occlusion needs to be recomputed
      emit invalidateCachedAmbientOcclusionTexture(_sceneList->selectediRASPARenderStructures());
    }

    // only when q single frame is selected, set the selected frame
    // result: extending the selection does not change the selected frame
    if(selectedIndexes().size() == 1)
    {
      QModelIndex current = selectedIndexes().front();

      if(iRASPAStructure* iraspa_structure = static_cast<iRASPAStructure*>(current.internalPointer()))
      {
        // set the overall frame-index (there is just one frame-index which is used for all movies)
        _sceneList->setSelectedFrameIndex(current.row());

        // propagates the selected frame to the atom and bond-views
        emit setSelectedFrame(iraspa_structure->shared_from_this());
      }
    }

    _movie->clearSelection();
    for(QModelIndex index : selectedIndexes())
    {
      _movie->addSelectedFrameIndex(index.row());
    }

    // set currentIndex for keyboard navigation
    if (std::optional<int> selectedIndex = _sceneList->selectedFrameIndex())
    {
      QModelIndex item = model()->index(*selectedIndex, 0, QModelIndex());
      selectionModel()->setCurrentIndex(item, QItemSelectionModel::SelectionFlag::Current);
    }

    // propagates the selected-frames to the renderer
    emit setSelectedRenderFrames(_sceneList->selectediRASPARenderStructures());

    // propagates the structures to the info, appearance, and cell-tabs
    emit setFlattenedSelectedFrames(_movie->selectedFramesiRASPAStructures());

    emit rendererReloadData();

    update();
  }
}

void FrameListView::keyPressEvent(QKeyEvent * event)
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
    QListView::keyPressEvent(event);
  }
}

void FrameListView::addFrame()
{
  qDebug() << "FrameListView::addFrame:";
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(_movie)
      {
        int row = 0;
        size_t selectedIndex = _sceneList->selectedFrameIndex();
        if(!_movie->frames().empty())
        {
          row = selectedIndex + 1;
        }
        FrameSelectionNodesAndIndexSet selection = _movie->selectedFramesNodesAndIndexSet();
        int selectedFrame = _sceneList->selectedFrameIndex();
        FrameListViewInsertCommand *addFrameCommand = new FrameListViewInsertCommand(_mainWindow, _sceneList.get(), this, _movie, row, selection, selectedFrame, nullptr);
        _projectTreeNode->representedObject()->undoManager().push(addFrameCommand);
      }
    }
  }
}

void FrameListView::deleteSelection()
{
  qDebug() << "FrameListView::deleteSelection:";
  if(_projectTreeNode)
  {
    if(_projectTreeNode->isEditable())
    {
      if(_movie)
      {
        FrameSelectionIndexSet selection = _movie->selectedFramesIndexSet();
        FrameSelectionNodesAndIndexSet deletedSelection = _movie->selectedFramesNodesAndIndexSet();

        if(!selection.empty())
        {
          FrameListViewDeleteSelectionCommand *deleteSelectionCommand = new FrameListViewDeleteSelectionCommand(_mainWindow, this, _sceneList, _movie, deletedSelection, selection, nullptr);
          _projectTreeNode->representedObject()->undoManager().push(deleteSelectionCommand);
        }
      }
    }
  }
}

void FrameListView::copy()
{
  qDebug() << "void FrameListView::copy()";
  if(FrameListViewModel* pModel = qobject_cast<FrameListViewModel*>(model()))
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

void FrameListView::paste()
{
  qDebug() << "void FrameListView::paste()";
  if(FrameListViewModel* pModel = qobject_cast<FrameListViewModel*>(model()))
  {
    if(qApp->clipboard()->mimeData()->hasFormat(iRASPAStructure::mimeType))
    {
      const QMimeData *mimeData = qApp->clipboard()->mimeData();

      QModelIndex currentIndex = selectionModel()->currentIndex();
      if(currentIndex.isValid())
      {
        if(pModel->canDropMimeData(mimeData,Qt::CopyAction, 0, 0, currentIndex))
        {
          pModel->pasteMimeData(mimeData, 0, 0, currentIndex);
        }
      }
    }
  }
}

void FrameListView::cut()
{
  copy();
  deleteSelection();
}

QPixmap FrameListView::selectionToPixmap()
{
  QModelIndexList selectionIndexes = selectionModel()->selectedRows();

  QFontMetrics fontMetrics = QFontMetrics(this->font());

  if(FrameListViewModel* pModel = qobject_cast<FrameListViewModel*>(model()))
  {
    int maxWidth=0;
    int height=4;

    for(QModelIndex index: selectionIndexes)
    {
      if(iRASPAStructure *item = static_cast<iRASPAStructure *>(index.internalPointer()))
      {
        QString text = item->structure()->displayName();
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
      if(iRASPAStructure *item = static_cast<iRASPAStructure *>(index.internalPointer()))
      {
        QString text = item->structure()->displayName();
        QRect rect = fontMetrics.boundingRect(text);
        currentHeight += rect.size().height();

        painter.save();
        painter.translate(QPointF(0,currentHeight));
        painter.drawText(rect,Qt::AlignLeft|Qt::AlignCenter, text);
        painter.restore();
      }
    }
    return pix;
  }

  return QPixmap();
}

void FrameListView::startDrag(Qt::DropActions supportedActions)
{
  qDebug() << "FrameListView::startDrag";
  setDropIndicatorShown(true);
  QModelIndex index = currentIndex();

  FrameListViewModel* pModel = qobject_cast<FrameListViewModel*>(model());
  if(iRASPAStructure *item = static_cast<iRASPAStructure *>(index.internalPointer()))
  {
    QModelIndexList selectionIndexes = selectionModel()->selectedRows();
    QMimeData* mimeData = pModel->mimeDataLazy(selectionIndexes);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);

    drag->setPixmap(selectionToPixmap());

    if(drag->exec(supportedActions))
    {
      reloadSelection();
    }
  }
}

void FrameListView::dragMoveEvent(QDragMoveEvent* event)
{
  QModelIndex index = indexAt(event->pos());

  // use the visualRect of the index to avoid dropping on the background left to items
  if (!index.isValid() || !visualRect(index).contains(event->pos()))
  {
    setDropIndicatorShown(false);
    event->ignore();
    viewport()->update();
  }
  else
  {
    setDropIndicatorShown(true);
    viewport()->update();
    event->accept();
  }

  QListView::dragMoveEvent(event);
}

