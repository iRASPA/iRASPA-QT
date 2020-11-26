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

#include "projecttreeviewmodel.h"
#include "projecttreeviewchangedisplaynamecommand.h"
#include "projecttreeviewdropcopycommand.h"
#include "projecttreeviewdropmovecommand.h"
#include "projecttreeviewpasteprojectscommand.h"


ProjectTreeViewModel::ProjectTreeViewModel(): _projectTreeController(std::make_shared<ProjectTreeController>())
{
  _folderIcon = QIcon(":/iraspa/genericfoldericon.png");
  _font.setFamily("Lucida Grande");
  _font.setBold(true);
}

void ProjectTreeViewModel::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
}

void ProjectTreeViewModel::setDocumentData(std::shared_ptr<DocumentData> documentData)
{
  beginResetModel();

  for (std::shared_ptr<ProjectTreeNode> localNode : _projectTreeController->galleryProjects()->childNodes())
  {
    localNode->appendToParent(documentData->projectTreeController()->galleryProjects());
  }

  for (std::shared_ptr<ProjectTreeNode> localNode : _projectTreeController->icloudProjects()->childNodes())
  {
    localNode->appendToParent(documentData->projectTreeController()->icloudProjects());
  }

  _mainWindow->setDocumentData(documentData);
  _projectTreeController = documentData->projectTreeController();
  endResetModel();
}

void ProjectTreeViewModel::insertGalleryData(std::shared_ptr<DocumentData> database)
{
  QModelIndex index = indexForNode(_projectTreeController->galleryProjects().get());
  int numberOfChildren = database->projectTreeController()->localProjects()->childNodes().size();
  beginInsertRows(index,0,numberOfChildren-1);
  for (std::shared_ptr<ProjectTreeNode> localNode : database->projectTreeController()->localProjects()->childNodes())
  {
    localNode->appendToParent(_projectTreeController->galleryProjects());
  }
  endInsertRows();
}

void ProjectTreeViewModel::insertDatabaseCoReMOFData(std::shared_ptr<DocumentData> database)
{
  QModelIndex index = indexForNode(_projectTreeController->icloudProjects().get());
  int numberOfChildren = database->projectTreeController()->localProjects()->childNodes().size();
  beginInsertRows(index,0,numberOfChildren-1);
  for (std::shared_ptr<ProjectTreeNode> localNode : database->projectTreeController()->localProjects()->childNodes())
  {
    localNode->appendToParent(_projectTreeController->icloudProjects());
  }
  endInsertRows();
}

void ProjectTreeViewModel::insertDatabaseCoReMOFDDECData(std::shared_ptr<DocumentData> database)
{
  QModelIndex index = indexForNode(_projectTreeController->icloudProjects().get());
  int numberOfChildren = database->projectTreeController()->localProjects()->childNodes().size();
  beginInsertRows(index,0,numberOfChildren-1);
  for (std::shared_ptr<ProjectTreeNode> localNode : database->projectTreeController()->localProjects()->childNodes())
  {
    localNode->appendToParent(_projectTreeController->icloudProjects());
  }
  endInsertRows();
}

void ProjectTreeViewModel::insertDatabaseIZAFData(std::shared_ptr<DocumentData> database)
{
  QModelIndex index = indexForNode(_projectTreeController->icloudProjects().get());
  int numberOfChildren = database->projectTreeController()->localProjects()->childNodes().size();
  beginInsertRows(index,0,numberOfChildren-1);
  for (std::shared_ptr<ProjectTreeNode> localNode : database->projectTreeController()->localProjects()->childNodes())
  {
    localNode->appendToParent(_projectTreeController->icloudProjects());
  }
  endInsertRows();
}

bool ProjectTreeViewModel::isMainSelectedItem(std::shared_ptr<ProjectTreeNode> treeNode)
{
  return (_projectTreeController->selectedTreeNode() == treeNode);
}

QModelIndexList ProjectTreeViewModel::selectedIndexes()
{
  QModelIndexList list = QModelIndexList();
  if(_projectTreeController)
  {
    std::set<IndexPath> selectedIndexPaths = _projectTreeController->selectionIndexPaths().second;
    for(IndexPath indexPath : selectedIndexPaths)
    {
      std::shared_ptr<ProjectTreeNode> node = _projectTreeController->nodeAtIndexPath(indexPath);
      int localRow = indexPath.lastIndex();
      QModelIndex index = createIndex(localRow,0,node.get());
      list.push_back(index);
    }
  }
  return list;
}

bool ProjectTreeViewModel::hasChildren(const QModelIndex &parent) const
{
  if (!parent.isValid()) return true;

  ProjectTreeNode *item = nodeForIndex(parent);

  return item->representedObject()->isGroup();
}

bool ProjectTreeViewModel::removeRows(int position, int rows, const QModelIndex &parent)
{
  ProjectTreeNode *parentItem = nodeForIndex(parent);

  if (position < 0 || position > static_cast<int>(parentItem->childNodes().size()))
    return false;

  beginRemoveRows(parent, position, position + rows - 1);
  for (int row = 0; row < rows; ++row)
  {
    if (!parentItem->removeChild(position))
      break;
  }
  endRemoveRows();

  return true;
}


bool ProjectTreeViewModel::insertRows(int position, int rows, const QModelIndex &parent, std::shared_ptr<ProjectTreeNode> item)
{
  ProjectTreeNode *parentItem = nodeForIndex(parent);

	beginInsertRows(parent, position, position + rows - 1);
	for (int row = 0; row < rows; ++row)
	{
		if (!parentItem->insertChild(position, item))
			break;
	}
	endInsertRows();

	return true;
}

bool ProjectTreeViewModel::insertRows(int position, int rows, const QModelIndex &parent)
{
  ProjectTreeNode *parentItem = nodeForIndex(parent);

  beginInsertRows(parent, position, position + rows - 1);
  for (int row = 0; row < rows; ++row)
  {
    std::shared_ptr<ProjectStructure> project = std::make_shared<ProjectStructure>();
    std::shared_ptr<iRASPAProject>  iraspaproject = std::make_shared<iRASPAProject>(project);
    std::shared_ptr<ProjectTreeNode> newItem = std::make_shared<ProjectTreeNode>("New project", iraspaproject, true, true);

    if (!parentItem->insertChild(position, newItem))
      break;
  }
  endInsertRows();

  return true;
}

bool ProjectTreeViewModel::insertRow(int position, std::shared_ptr<ProjectTreeNode> parent, std::shared_ptr<ProjectTreeNode> project)
{
  if(parent)
  {
    QModelIndex parentIndex = indexForNode(parent.get());
    if(parentIndex.isValid())
    {
      beginInsertRows(parentIndex, position, position);
      parent->insertChild(position, project);
      endInsertRows();

      return true;
    }
  }
  return false;
}

bool ProjectTreeViewModel::removeRow(int position, std::shared_ptr<ProjectTreeNode> parentNode)
{
  QModelIndex parent = indexForNode(parentNode.get());
  //checkIndex(parent);

  beginRemoveRows(parent, position, position);
  bool success = parentNode->removeChild(position);
  endRemoveRows();

  return success;
}


QModelIndex ProjectTreeViewModel::index(int row, int column, const QModelIndex &parent) const
{
  if(!hasIndex(row, column, parent))
     return QModelIndex();

  ProjectTreeNode *parentNode = nodeForIndex(parent);
  ProjectTreeNode *childNode = parentNode->childNodes()[row].get();
  return createIndex(row, column, childNode);
}

QModelIndex ProjectTreeViewModel::parent(const QModelIndex &child) const
{
  ProjectTreeNode *childNode = nodeForIndex(child);
  ProjectTreeNode *parentNode = childNode->parent().get();

  if (_projectTreeController->isRootNode(parentNode))
    return QModelIndex();

  int row = rowForNode(parentNode);
  int column = 0;
  return createIndex(row, column, parentNode);
}

int ProjectTreeViewModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 1;
}


int ProjectTreeViewModel::rowCount(const QModelIndex &parent) const
{
  ProjectTreeNode *parentItem;

  if (!parent.isValid())
    parentItem = _projectTreeController->hiddenRootNode().get();
  else
    parentItem = static_cast<ProjectTreeNode*>(parent.internalPointer());

  return static_cast<int>(parentItem->childCount());
}

QVariant ProjectTreeViewModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role == Qt::ForegroundRole)
  {
    if(index.parent() == QModelIndex())
    {
      return QColor(114,114,114,255);
    }
  }

  if (role == Qt::FontRole)
  {
    if(index.parent() == QModelIndex())
    {
      return _font;
    }
  }

  if( role == Qt::DecorationRole )
  {
    if(index.parent() != QModelIndex())
    {
      if(ProjectTreeNode *item = static_cast<ProjectTreeNode*>(index.internalPointer()))
      {
        if(std::shared_ptr<iRASPAProject> iraspa_project = std::dynamic_pointer_cast<iRASPAProject>(item->representedObject()))
        {
          if (iraspa_project->isGroup())
          {
            return _folderIcon;
          }
        }
      }
    }
  }

  if (role != Qt::DisplayRole)
    return QVariant();

  ProjectTreeNode *item = static_cast<ProjectTreeNode*>(index.internalPointer());
  return item->displayName();
}


Qt::ItemFlags ProjectTreeViewModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemFlag::NoItemFlags;

  if(index.parent().isValid())
  {
    flags |= Qt::ItemIsSelectable;
    flags |= Qt::ItemIsEnabled;
  }

  if(ProjectTreeNode *node = nodeForIndex(index))
  {
    if(node->isEditable() && ( index.column() == 0 ))
    {
      flags |= Qt::ItemIsEditable;
    }

    if(node->representedObject()->isLeaf())
    {
      flags |= Qt::ItemIsDragEnabled;
    }
    if(node->representedObject()->isGroup() && node->isDropEnabled())
    {
      flags |= Qt::ItemIsDropEnabled;
    }
  }

  return flags;
}


bool ProjectTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (role != Qt::EditRole)
    return false;

  ProjectTreeNode *item = nodeForIndex(index);
  QString newValue = value.toString();
  if(!newValue.isEmpty())
  {
    ProjectTreeViewChangeDisplayNameCommand*changeDisplayNameCommand = new ProjectTreeViewChangeDisplayNameCommand(_mainWindow, this, item->shared_from_this(), newValue, nullptr);
    _undoStack->push(changeDisplayNameCommand);
    return true;
  }
  return false;
}

bool ProjectTreeViewModel::setData(const QModelIndex &index, std::shared_ptr<iRASPAProject> value, bool isGroup)
{
  ProjectTreeNode *item = nodeForIndex(index);
  item->representedObject()->setIsGroup(isGroup);
  item->setRepresentedObject(value);

  emit dataChanged(index, index);

  return true;
}

Qt::DropActions ProjectTreeViewModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions ProjectTreeViewModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QStringList ProjectTreeViewModel::mimeTypes() const
{
  return QAbstractItemModel::mimeTypes() << ProjectTreeNode::mimeType;
}

QMimeData* ProjectTreeViewModel::mimeDataLazy(const QModelIndexList &indexes) const
{
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  QModelIndexList sortedIndexes = indexes;
  std::sort(sortedIndexes.begin(), sortedIndexes.end());

  // write application id
  stream << QCoreApplication::applicationPid();

  // write application projectTreeView id
  qulonglong ptrval(reinterpret_cast<qulonglong>(this));
  stream << ptrval;

  // write the number of objects
  stream << sortedIndexes.count();

  for(const QModelIndex &index: sortedIndexes)
  {
    if(index.isValid())
    {
      if(ProjectTreeNode *projectTreeNode = nodeForIndex(index))
      {
        qulonglong ptrval(reinterpret_cast<qulonglong>(projectTreeNode));
        stream << ptrval;
      }
    }
  }
  QMimeData *mimeData = new QMimeData();
  mimeData->setData(ProjectTreeNode::mimeType, encodedData);

  return mimeData;
}

QMimeData* ProjectTreeViewModel::mimeData(const QModelIndexList &indexes) const
{
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  QModelIndexList sortedIndexes = indexes;
  std::sort(sortedIndexes.begin(), sortedIndexes.end());

  // write application id
  stream << QCoreApplication::applicationPid();

  for(const QModelIndex &index: sortedIndexes)
  {
    if(index.isValid())
    {
      if(ProjectTreeNode *projectTreeNode = nodeForIndex(index))
      {
        stream <<= projectTreeNode->shared_from_this();
      }
    }
  }
  QMimeData *mimeData = new QMimeData();
  mimeData->setData(ProjectTreeNode::mimeType, encodedData);

  return mimeData;
}

bool ProjectTreeViewModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(data);
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(column);

  if(ProjectTreeNode *parentNode = nodeForIndex(parent))
  {
    if(parentNode->isDropEnabled())
    {
      return true;
    }
  }
  return false;
}

// drops onto existing items have row and column set to -1 and parent set to the current item
bool ProjectTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);


  if(action == Qt::IgnoreAction)
  {
    return true;
  }
  if(!data->hasFormat(ProjectTreeNode::mimeType))
  {
    return false;
  }

  QByteArray ddata = data->data(ProjectTreeNode::mimeType);
  QDataStream stream(&ddata, QIODevice::ReadOnly);

  ProjectTreeNode *parentNode = nodeForIndex(parent);
  IndexPath parentIndexPath = parentNode->indexPath();

  // read application id
  qint64 senderPid;
  stream >> senderPid;
  if (senderPid != QCoreApplication::applicationPid())
  {
    return false;
  }

  // read application projectTreeView id
  qulonglong sourceProjectTreeViewId(reinterpret_cast<qulonglong>(this));
  qulonglong senderProjectTreeViewId;
  stream >> senderProjectTreeViewId;

  // read the number of objects
  int count;
  stream >> count;

  int beginRow = row;
  if (beginRow == -1)
  {
    if (parent.isValid())
      beginRow = 0;
    else
      beginRow = rowCount(parent);
  }

  if(action == Qt::DropAction::CopyAction || (senderProjectTreeViewId != sourceProjectTreeViewId))
  {
    ProjectSelectionNodesAndIndexPaths projectNodes{};
    while (!stream.atEnd())
    {
      qlonglong nodePtr;
      stream >> nodePtr;
      ProjectTreeNode *node = reinterpret_cast<ProjectTreeNode *>(nodePtr);
      std::shared_ptr<ProjectTreeNode> copiedProjectTreeNode = node->shallowClone();
      copiedProjectTreeNode->setIsEditable(true);
      copiedProjectTreeNode->setIsDropEnabled(true);
      projectNodes.second.insert(std::make_pair(copiedProjectTreeNode, parentIndexPath.appending(beginRow++)));
    }
    projectNodes.first = *projectNodes.second.begin();

    ProjectTreeViewDropCopyCommand *copyCommand = new ProjectTreeViewDropCopyCommand(_mainWindow, this, _projectTreeController, projectNodes,
                                                                                     _projectTreeController->selectionIndexPaths(), nullptr);
    _undoStack->push(copyCommand);
  }
  else if(action == Qt::DropAction::MoveAction && (senderProjectTreeViewId == sourceProjectTreeViewId))
  {
    std::vector<std::tuple<std::shared_ptr<ProjectTreeNode>, std::shared_ptr<ProjectTreeNode>, int, bool>> nodes{};

    while (!stream.atEnd())
    {
      qlonglong nodePtr;
      stream >> nodePtr;
      ProjectTreeNode *node = reinterpret_cast<ProjectTreeNode *>(nodePtr);

      if(node->isEditable())
      {
        if (static_cast<int>(node->row()) < beginRow && (parentNode == node->parent().get()))
        {
          beginRow -= 1;
        }

        nodes.push_back(std::make_tuple(node->shared_from_this(), parentNode->shared_from_this(), beginRow, true));
      }
      else
      {
        std::shared_ptr<ProjectTreeNode> copiedProjectTreeNode = node->shallowClone();
        copiedProjectTreeNode->setIsEditable(true);
        copiedProjectTreeNode->setIsDropEnabled(true);
        nodes.push_back(std::make_tuple(copiedProjectTreeNode, parentNode->shared_from_this(), beginRow, false));
      }
      beginRow += 1;
    }

    ProjectTreeViewDropMoveCommand *moveCommand = new ProjectTreeViewDropMoveCommand(_mainWindow, this, _projectTreeController.get(), nodes,
                                                                                     _projectTreeController->selectionIndexPaths(), nullptr);
    _undoStack->push(moveCommand);
  }

  return true;
}

bool ProjectTreeViewModel::pasteMimeData(const QMimeData *data, int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);

  if(!data->hasFormat(ProjectTreeNode::mimeType))
  {
    return false;
  }

  QByteArray ddata = data->data(ProjectTreeNode::mimeType);
  QDataStream stream(&ddata, QIODevice::ReadOnly);

  // read application id
  qint64 senderPid;
  stream >> senderPid;
  if (senderPid != QCoreApplication::applicationPid())
  {
    return false;
  }

  // determine insertion point
  QModelIndex beginParent = parent;
  int beginRow = row;
  if(ProjectTreeNode *parentNode = nodeForIndex(parent))
  {
    if(std::shared_ptr<iRASPAProject> iraspa_project = parentNode->representedObject())
    {
      if(std::shared_ptr<ProjectStructure> project = std::dynamic_pointer_cast<ProjectStructure>(iraspa_project->project()))
      {
        beginParent = parent.parent();
        beginRow = parent.row() + 1;
      }
    }
  }

  if (row == -1)
  {
    if (parent.isValid())
      beginRow = 0;
    else
      beginRow = rowCount(parent);
  }

  if(ProjectTreeNode *parentNode = nodeForIndex(beginParent))
  {

    std::vector<std::shared_ptr<ProjectTreeNode>> nodes{};
    while (!stream.atEnd())
    {
      std::shared_ptr<ProjectTreeNode> copiedProjectTreeNode = std::make_shared<ProjectTreeNode>();
      stream >>= copiedProjectTreeNode;
      copiedProjectTreeNode->setIsEditable(true);
      copiedProjectTreeNode->setIsDropEnabled(true);
      nodes.push_back(copiedProjectTreeNode);
    }

    ProjectTreeViewPasteProjectsCommand *copyCommand = new ProjectTreeViewPasteProjectsCommand(_mainWindow, this, parentNode->shared_from_this(), beginRow, nodes, _projectTreeController->selectionIndexPaths(), nullptr);
    _undoStack->push(copyCommand);
    return true;
  }

  return false;
}

// Helper functions
QModelIndex ProjectTreeViewModel::indexForNode(ProjectTreeNode *node, int column) const
{
  if(_projectTreeController->isRootNode(node))
  {
    return QModelIndex();
  }
  int row = rowForNode(node);
  return createIndex(row, column, node);
}

ProjectTreeNode *ProjectTreeViewModel::nodeForIndex(const QModelIndex &index) const
{
  if(index.isValid())
  {
    ProjectTreeNode *projectTreeNode = static_cast<ProjectTreeNode *>(index.internalPointer());
    if(projectTreeNode)
      return projectTreeNode;
    else
      return nullptr;
  }
  return _projectTreeController->hiddenRootNode().get();
}

int ProjectTreeViewModel::rowForNode(ProjectTreeNode *node) const
{
  return node->row();
}
