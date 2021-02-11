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

#include "projecttreeview.h"
#include "projecttreeviewstyleditemdelegate.h"
#include "projecttreeviewproxystyle.h"
#include "projecttreeviewinsertprojectstructurecommand.h"
#include "projecttreeviewinsertprojectgroupcommand.h"
#include "projecttreeviewdeleteselectioncommand.h"
#include "projecttreeviewchangeselectioncommand.h"
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QFont>
#include <QFontMetrics>

ProjectTreeView::ProjectTreeView(QWidget* parent): iRASPATreeView(parent),
  _model(std::make_shared<ProjectTreeViewModel>()),
  _undoStack()
{
  this->setModel(_model.get());
  this->_model->setUndoStack(&_undoStack);
  this->setHeaderHidden(true);

  this->setAttribute(Qt::WA_MacShowFocusRect, false);
  this->setStyleSheet("background-color:rgb(240, 240, 240);");
  this->setStyle(new ProjectTreeViewProxyStyle());

  this->setSelectionBehavior (QAbstractItemView::SelectRows);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);

  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  setDragDropMode(DragDropMode::DragDrop);
  setDragDropOverwriteMode(false);

  setItemsExpandable(true);
  setRootIsDecorated(false);
  setExpandsOnDoubleClick(false);

  this->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(this, &QWidget::customContextMenuRequested, this, &ProjectTreeView::ShowContextMenu);

  _dropIndicatorRect = QRect();

  this->setItemDelegateForColumn(0,new ProjectTreeViewStyledItemDelegate(this));

  QObject::connect(_model.get(), &ProjectTreeViewModel::updateSelection, this, &ProjectTreeView::reloadSelection);
  QObject::connect(_model.get(), &ProjectTreeViewModel::collapse, this, &ProjectTreeView::collapse);
  QObject::connect(_model.get(), &ProjectTreeViewModel::expand, this, &ProjectTreeView::expand);
}

void ProjectTreeView::setLogReportingWidget(LogReporting *logReporting)
{
  _logReporting = logReporting;
}

void ProjectTreeView::setDocumentData(std::shared_ptr<DocumentData> documentData)
{
  _model->setDocumentData(documentData);
  _projectTreeController = documentData->projectTreeController();

  QModelIndex index1 = model()->index(0,0,QModelIndex());
  QModelIndex index2 = model()->index(2,0,QModelIndex());
  QModelIndex index3 = model()->index(4,0,QModelIndex());
  expand(index1);
  expand(index2);
  expand(index3);

  QModelIndex index4 = model()->index(0,0,index2);
  expand(index4);

  _mainWindow->propagateProject(nullptr, _mainWindow);
}

void ProjectTreeView::insertGalleryData(std::shared_ptr<DocumentData> database)
{
  _model->insertGalleryData(database);
}

void ProjectTreeView::insertDatabaseCoReMOFData(std::shared_ptr<DocumentData> database)
{
  _model->insertDatabaseCoReMOFData(database);
}

void ProjectTreeView::insertDatabaseCoReMOFDDECData(std::shared_ptr<DocumentData> database)
{
  _model->insertDatabaseCoReMOFDDECData(database);
}

void ProjectTreeView::insertDatabaseIZAFData(std::shared_ptr<DocumentData> database)
{
  _model->insertDatabaseIZAFData(database);
}

void ProjectTreeView::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
  _model->setMainWindow(mainWindow);
}

void ProjectTreeView::TabItemWasSelected()
{
  this->reloadData();
  this->reloadSelection();
}

// Use the general undoManager for changes to the project-treeView.
void ProjectTreeView::focusInEvent( QFocusEvent* )
{
  QAction *newUndoAction = this->undoManager().createUndoAction(this, tr("&Undo"));
  _mainWindow->setUndoAction(newUndoAction);

  QAction *newRedoAction = this->undoManager().createRedoAction(this, tr("&Redo"));
  _mainWindow->setRedoAction(newRedoAction);
}

// Use the undoManger of the project for all changes inside a project. Each project has its own undoManager.
void ProjectTreeView::focusOutEvent( QFocusEvent* )
{
  QModelIndex index = this->selectionModel()->currentIndex();

  if(index.isValid())
  {
    if(ProjectTreeNode* item = static_cast<ProjectTreeNode*>(index.internalPointer()))
    {
       if(std::shared_ptr<iRASPAProject> iraspaProject =  item->representedObject())
       {
         QAction *newUndoAction = iraspaProject->undoManager().createUndoAction(this, tr("&Undo"));
         _mainWindow->setUndoAction(newUndoAction);

         QAction *newRedoAction = iraspaProject->undoManager().createRedoAction(this, tr("&Redo"));
         _mainWindow->setRedoAction(newRedoAction);
       }
    }
  }
}

bool ProjectTreeView::hasSelection() const
{
  QModelIndexList selection = selectionModel()->selectedIndexes();
  for(QModelIndex index : selection)
  {
    if(ProjectTreeNode *selectedTreeNode = static_cast<ProjectTreeNode*>(index.internalPointer()))
    {
      if(std::shared_ptr<iRASPAProject> iraspa_project = selectedTreeNode->representedObject())
      {
        if(std::shared_ptr<ProjectStructure> structureProject = std::dynamic_pointer_cast<ProjectStructure>(iraspa_project->project()))
        {
          return true;
        }
      }
    }
  }
  return false;
}

void ProjectTreeView::paintEvent(QPaintEvent *event)
{
  QTreeView:: paintEvent(event);
}


QPixmap ProjectTreeView::selectionToPixmap()
{
  QModelIndexList selectionIndexes = selectionModel()->selectedRows();

  QFontMetrics fontMetrics = QFontMetrics(this->font());

  if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model()))
  {
    int maxWidth=0;
    int height=4;

    for(QModelIndex index: selectionIndexes)
    {
      if(ProjectTreeNode* item = pModel->nodeForIndex(index))
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
      if(ProjectTreeNode* item = pModel->nodeForIndex(index))
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

void ProjectTreeView::startDrag(Qt::DropActions supportedActions)
{
  setDropIndicatorShown(true);
  QModelIndex index = currentIndex();

  ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model());
  if(ProjectTreeNode* item = pModel->nodeForIndex(index))
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

void ProjectTreeView::dragMoveEvent(QDragMoveEvent* event)
{
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  QModelIndex index = indexAt(event->pos());
#else
  QModelIndex index = indexAt(event->position().toPoint());
#endif

  // use the visualRect of the index to avoid dropping on the background left to items
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  if (!index.isValid() || !visualRect(index).contains(event->pos()))
#else
  if (!index.isValid() || !visualRect(index).contains(event->position().toPoint()))
#endif
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

  QTreeView::dragMoveEvent(event);
}


void ProjectTreeView::reloadSelection()
{
  ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model());
  if (pModel)
  {
    if(_projectTreeController)
    {
      ProjectSelectionIndexPaths selection = _projectTreeController->selectionIndexPaths();

      whileBlocking(selectionModel())->clearSelection();

      for(QModelIndex index : pModel->selectedIndexes())
      {
        whileBlocking(selectionModel())->select(index, QItemSelectionModel::Select|QItemSelectionModel::Rows);
      }

      if(std::shared_ptr<ProjectTreeNode> project = _projectTreeController->selectedTreeNode())
      {
        QModelIndex index = pModel->indexForNode(project.get());
        whileBlocking(selectionModel())->select(index, QItemSelectionModel::Select);
        whileBlocking(selectionModel())->setCurrentIndex(index, QItemSelectionModel::Select);
      }
    }
    update();
  }
}

void ProjectTreeView::reloadData()
{
  _mainWindow->updateMenuToProjectTab();
}


bool ProjectTreeView::insertRows(int position, int rows, const QModelIndex &parent, std::shared_ptr<ProjectTreeNode> item)
{
	ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model());
	if (pModel)
	{
		return pModel->insertRows(position, rows, parent, item);
	}
	return false;
}


void ProjectTreeView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  // avoid empty selection
  if (selectedIndexes().isEmpty())
  {
    reloadSelection();
    return;
  }

  QAbstractItemView::selectionChanged(selected, deselected);

  if(_projectTreeController)
  {
    ProjectSelectionIndexPaths previousSelection = _projectTreeController->selectionIndexPaths();

    _projectTreeController->clearSecondSelection();

    // the selection should be set when the selection-size is 1, and selecting other projects will not change the set project.
    // (this avoids that selecting multiple project continuously change the shown main project)
    if(selectedIndexes().size() == 1)
    {
      QModelIndex current = selectedIndexes().front();
      if(ProjectTreeNode *selectedTreeNode = static_cast<ProjectTreeNode*>(current.internalPointer()))
      {
        qDebug() << "SELECTED PROJECT: " << selectedTreeNode;
        IndexPath selectedTreeNodeIndexPath = IndexPath::indexPath(current);
        _projectTreeController->setSelectionIndexPaths(ProjectSelectionIndexPaths(selectedTreeNodeIndexPath,{selectedTreeNodeIndexPath}));
        selectedTreeNode->representedObject()->unwrapIfNeeded(_logReporting);

        if(std::shared_ptr<iRASPAProject> iraspa_project = selectedTreeNode->representedObject())
        {
          if(std::shared_ptr<Project> project = iraspa_project->project())
          {
            project->setInitialSelectionIfNeeded();
          }
        }
      }
    }

    // loop over all selected indexes and add them to the 'second-selection'.
    for(QModelIndex index : selectedIndexes())
    {
      IndexPath indexPath = IndexPath::indexPath(index);
      _projectTreeController->insertSelectionIndexPath(indexPath);
    }

    ProjectSelectionIndexPaths currentSelection = _projectTreeController->selectionIndexPaths();
    ProjectChangeSelectionCommand *changeSelectionCommand = new ProjectChangeSelectionCommand(_mainWindow, this, currentSelection, previousSelection,  nullptr);
    undoManager().push(changeSelectionCommand);

    update();
  }
}

QSize ProjectTreeView::sizeHint() const
{
  return QSize(200, 800);
}

void ProjectTreeView::ShowToolButtonContextMenu(const QPoint &pos)
{
  QMenu contextMenu(tr("Context menu"), this);

  QAction actionAddStructureProject("Add structure project", this);
  actionAddStructureProject.setEnabled(true);
  connect(&actionAddStructureProject, &QAction::triggered, [this](void) {
     this->addStructureProject(QModelIndex());
  });
  contextMenu.addAction(&actionAddStructureProject);


  QAction actionGroupProject("Add group project", this);
  actionGroupProject.setEnabled(true);
  connect(&actionGroupProject, &QAction::triggered, [this](void) {
     this->addGroupProject(QModelIndex());
  });
  contextMenu.addAction(&actionGroupProject);

  contextMenu.exec(pos);
}

void ProjectTreeView::ShowContextMenu(const QPoint &pos)
{
  QModelIndex index = indexAt(pos);


  if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model()))
  {
    QMenu contextMenu(tr("Context menu"), this);

    QAction actionAddStructureProject("Add structure project", this);
    actionAddStructureProject.setEnabled(true);
    connect(&actionAddStructureProject, &QAction::triggered, [this, index](void) {
       this->addStructureProject(index);
    });
    contextMenu.addAction(&actionAddStructureProject);


    QAction actionGroupProject("Add group project", this);
    actionGroupProject.setEnabled(true);
    connect(&actionGroupProject, &QAction::triggered, [this, index](void) {
       this->addGroupProject(index);
    });
    contextMenu.addAction(&actionGroupProject);

    contextMenu.exec(viewport()->mapToGlobal(pos));
  }
}

void ProjectTreeView::keyPressEvent(QKeyEvent * event)
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

void ProjectTreeView::deleteSelection()
{
  if(_projectTreeController)
  {
    ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
    ProjectSelectionNodesAndIndexPaths deletedSelection = _projectTreeController->selectionEditableNodesAndIndexPaths();

    if(!deletedSelection.second.empty())
    {
      ProjectTreeViewDeleteSelectionCommand *deleteSelectionCommand = new ProjectTreeViewDeleteSelectionCommand(_mainWindow, this->_model.get(), _projectTreeController,
                                                                                                                deletedSelection, selection, nullptr);
      undoManager().push(deleteSelectionCommand);
    }
  }
}

void ProjectTreeView::copy()
{
  if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model()))
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

void ProjectTreeView::paste()
{
  if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model()))
  {
    if(qApp->clipboard()->mimeData()->hasFormat(ProjectTreeNode::mimeType))
    {
      const QMimeData *mimeData = qApp->clipboard()->mimeData();

      QModelIndex currentIndex = selectionModel()->currentIndex();
      if(currentIndex.isValid())
      {
        if(pModel->canDropMimeData(mimeData,Qt::CopyAction, 0, 0, currentIndex))
        {
          pModel->pasteMimeData(mimeData, 0, 0, currentIndex);
        }
        else
        {
          if(ProjectTreeNode *projectTreeNode = pModel->nodeForIndex(currentIndex.parent()))
          {
            _logReporting->logMessage(LogReporting::ErrorLevel::warning, "Not allowed to paste into container " + projectTreeNode->displayName());
          }
        }
      }
    }
  }
}

void ProjectTreeView::cut()
{
  copy();
  deleteSelection();
}

void ProjectTreeView::addProjectStructure()
{
  if(_projectTreeController)
  {
    if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model()))
    {
      std::shared_ptr<ProjectTreeNode> parent = _projectTreeController->localProjects();
      QModelIndex parentIndex = pModel->indexForNode(parent.get());
      if(parentIndex.isValid())
      {
        ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
        ProjectTreeViewInsertProjectStructureCommand *insertProjectCommand = new ProjectTreeViewInsertProjectStructureCommand(_mainWindow, this, parent, int(parent->childCount()),
                                                                                                                              selection, nullptr);
        undoManager().push(insertProjectCommand);
      }
    }
  }
}

void ProjectTreeView::addStructureProject(QModelIndex index)
{
  if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model()))
  {
    if(index.isValid())
    {
      ProjectTreeNode *item = pModel->nodeForIndex(index);

      if(item->isDropEnabled())
      {
        if(item->representedObject()->isLeaf())
        {
          ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
          ProjectTreeViewInsertProjectStructureCommand *insertProjectCommand = new ProjectTreeViewInsertProjectStructureCommand(_mainWindow, this, item->parent(),
                                                                                                                               int(item->row() + 1), selection, nullptr);
          undoManager().push(insertProjectCommand);
        }
        else
        {
          ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
          ProjectTreeViewInsertProjectStructureCommand *insertProjectCommand = new ProjectTreeViewInsertProjectStructureCommand(_mainWindow, this, item->shared_from_this(),
                                                                                                                               0, selection, nullptr);
          undoManager().push(insertProjectCommand);
        }
      }
    }
    else
    {
      std::shared_ptr<ProjectTreeNode> parent = _projectTreeController->localProjects();
      ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
      ProjectTreeViewInsertProjectStructureCommand *insertProjectCommand = new ProjectTreeViewInsertProjectStructureCommand(_mainWindow, this, parent,
                                                                                                                            int(parent->childCount()), selection, nullptr);
      undoManager().push(insertProjectCommand);
    }
  }
}

void ProjectTreeView::addGroupProject(QModelIndex index)
{
  if(ProjectTreeViewModel* pModel = qobject_cast<ProjectTreeViewModel*>(model()))
  {
    if(index.isValid())
    {
      ProjectTreeNode *item = pModel->nodeForIndex(index);

      if(item->isDropEnabled())
      {
        if(item->representedObject()->isLeaf())
        {
          ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
          ProjectTreeViewInsertProjectGroupCommand *insertProjectCommand = new ProjectTreeViewInsertProjectGroupCommand(_mainWindow, this, item->parent(),
                                                                                                                               item->row() + 1, selection, nullptr);
          undoManager().push(insertProjectCommand);
        }
        else
        {
          ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
          ProjectTreeViewInsertProjectGroupCommand *insertProjectCommand = new ProjectTreeViewInsertProjectGroupCommand(_mainWindow, this, item->shared_from_this(),
                                                                                                                               0, selection, nullptr);
          undoManager().push(insertProjectCommand);
        }
      }
    }
    else
    {
      std::shared_ptr<ProjectTreeNode> parent = _projectTreeController->localProjects();
      ProjectSelectionNodesAndIndexPaths selection = _projectTreeController->selectionNodesAndIndexPaths();
      ProjectTreeViewInsertProjectGroupCommand *insertProjectCommand = new ProjectTreeViewInsertProjectGroupCommand(_mainWindow, this, parent,
                                                                                                                            int(parent->childCount()), selection, nullptr);
      undoManager().push(insertProjectCommand);
    }
  }
}

