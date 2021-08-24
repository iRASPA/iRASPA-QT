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

#include "framelistviewmodel.h"
#include "framelistviewdropmovecommand.h"
#include "framelistviewdropcopycommand.h"
#include "framelistviewpastecommand.h"
#include "framelistviewchangedisplaynamecommand.h"
#include <iostream>
#include <QMimeData>
#include <QCoreApplication>

FrameListViewModel::FrameListViewModel(): _movie(std::make_shared<Movie>())
{

}

void FrameListViewModel::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
}

void FrameListViewModel::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
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
          _sceneList = projectStructure->sceneList();
        }
      }
    }
  }
}

void FrameListViewModel::setMovie(std::shared_ptr<Movie> movie)
{
  beginResetModel();
  _movie = movie;
  endResetModel();
}



QModelIndex FrameListViewModel::index(int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  Q_UNUSED(column);

  if(_movie)
  {
    if (_movie->frames().size() == 0)
      return QModelIndex();

    return createIndex(row, 0 , _movie->frames()[row].get());
  }
  return QModelIndex();
}

QModelIndex FrameListViewModel::parent(const QModelIndex &child) const
{
  Q_UNUSED(child);

  return QModelIndex();
}

int FrameListViewModel::rowCount(const QModelIndex &parent) const
{
  if(_movie)
  {
    if(parent == QModelIndex())
    {
      return static_cast<int>(_movie->frames().size());
    }
  }
  return 0;
}

int FrameListViewModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return 1;
}

QVariant FrameListViewModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
   return QVariant();

  if (role != Qt::DisplayRole)
   return QVariant();

  if(iRASPAStructure* iraspaStructure = static_cast<iRASPAStructure*>(index.internalPointer()))
  {
    return QString(iraspaStructure->structure()->displayName());
  }
  return QString("Unknown");
}

bool FrameListViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (role != Qt::EditRole)
    return false;

  if(iRASPAStructure* iraspaStructure = static_cast<iRASPAStructure*>(index.internalPointer()))
  {
    QString newValue = value.toString();
    if(!newValue.isEmpty())
    {
      FrameListViewChangeDisplayNameCommand *changeDisplayNameCommand = new FrameListViewChangeDisplayNameCommand(_mainWindow, this, iraspaStructure->shared_from_this(), newValue, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(changeDisplayNameCommand);
      return true;
    }
  }
  return false;
}

bool FrameListViewModel::insertRow(int position, std::shared_ptr<Movie> parent, std::shared_ptr<iRASPAStructure> iraspaStructure)
{
  if(parent)
  {
    beginInsertRows(QModelIndex(), position, position);
    parent->insertChild(position, iraspaStructure);
    endInsertRows();

    return true;
  }
  return false;
}

bool FrameListViewModel::insertRow(int position, std::shared_ptr<iRASPAStructure> iraspaStructure)
{
  if(_movie)
  {
    beginInsertRows(QModelIndex(), position, position);
    _movie->insertChild(position, iraspaStructure);
    endInsertRows();

    return true;
  }
  return false;
}

bool FrameListViewModel::removeRow(int position, std::shared_ptr<Movie> parentNode)
{
  beginRemoveRows(QModelIndex(), position, position);
  bool success = parentNode->removeChildren(position, 1);
  endRemoveRows();

  return success;
}

bool FrameListViewModel::removeRow(int position)
{
  beginRemoveRows(QModelIndex(), position, position);
  bool success = _movie->removeChildren(position, 1);
  endRemoveRows();

  return success;
}

Qt::ItemFlags FrameListViewModel::flags(const QModelIndex &index) const
{
  Qt::ItemFlags flags = Qt::ItemFlag::NoItemFlags;
  flags |= Qt::ItemIsSelectable;
  flags |= Qt::ItemIsEnabled;
  if(_projectTreeNode->isEditable())
  {
    if (!index.isValid()) return Qt::ItemIsDropEnabled;
    flags |= Qt::ItemIsEditable;
    flags |= Qt::ItemIsDragEnabled;
  }
  return flags;
}

// Helper functions
QModelIndex FrameListViewModel::indexForNode(iRASPAStructure *node, int column) const
{
  int row = int(node->row());
  return createIndex(row, column, node);
}

iRASPAStructure *FrameListViewModel::nodeForIndex(const QModelIndex &index) const
{
  iRASPAStructure *projectTreeNode = static_cast<iRASPAStructure *>(index.internalPointer());
  if(projectTreeNode)
    return projectTreeNode;
  else
    return nullptr;
}

Qt::DropActions FrameListViewModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions FrameListViewModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QMimeData* FrameListViewModel::mimeDataLazy(const QModelIndexList &indexes) const
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
  qulonglong count = static_cast<qulonglong>(sortedIndexes.count());
  stream << count;

  for(const QModelIndex &index: sortedIndexes)
  {
    if(index.isValid())
    {
      if(iRASPAStructure *item = static_cast<iRASPAStructure *>(index.internalPointer()))
      {
        qulonglong itemPtrval(reinterpret_cast<qulonglong>(item));
        stream << itemPtrval;
      }
    }
  }
  QMimeData *mimeData = new QMimeData();
  mimeData->setData(iRASPAStructure::mimeType, encodedData);

  return mimeData;
}

bool FrameListViewModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(data);
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(column);
  Q_UNUSED(parent);

  return true;
}

// drops onto existing items have row and column set to -1 and parent set to the current item
bool FrameListViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);
  Q_UNUSED(parent);

  if(action == Qt::IgnoreAction)
  {
    return true;
  }
  if(!data->hasFormat(iRASPAStructure::mimeType))
  {
    return false;
  }

  QByteArray ddata = data->data(iRASPAStructure::mimeType);
  QDataStream stream(&ddata, QIODevice::ReadOnly);

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
  qulonglong count;
  stream >> count;

  int beginRow = row;

  if(action == Qt::DropAction::CopyAction || (senderProjectTreeViewId != sourceProjectTreeViewId))
  {
    std::vector<std::pair<std::shared_ptr<iRASPAStructure>, size_t>> nodes{};

    while (!stream.atEnd())
    {
      qlonglong nodePtr;
      stream >> nodePtr;
      iRASPAStructure *item = reinterpret_cast<iRASPAStructure *>(nodePtr);
      std::shared_ptr<iRASPAStructure> node = item->clone();
      nodes.push_back(std::make_pair(node, beginRow));

      beginRow += 1;
    }

    FrameListViewDropCopyCommand *reorderCommand = new FrameListViewDropCopyCommand(_mainWindow, this, _sceneList, _movie, nodes,
                                                                                    _movie->selectedFramesIndexSet(), nullptr);
    _projectTreeNode->representedObject()->undoManager().push(reorderCommand);
  }
  else if(action == Qt::DropAction::MoveAction && (senderProjectTreeViewId == sourceProjectTreeViewId))
  {
    std::vector<std::pair<std::shared_ptr<iRASPAStructure>, size_t>> nodes{};

    while (!stream.atEnd())
    {
      qlonglong nodePtr;
      stream >> nodePtr;
      iRASPAStructure *node = reinterpret_cast<iRASPAStructure *>(nodePtr);

      if (static_cast<int>(node->row()) < beginRow)
      {
        beginRow -= 1;
      }

      nodes.push_back(std::make_pair(node->shared_from_this(), beginRow));

      beginRow += 1;
    }

    FrameListViewDropMoveCommand *reorderCommand = new FrameListViewDropMoveCommand(_mainWindow, this, _sceneList, _movie, nodes,
                                                                                    _movie->selectedFramesIndexSet(), nullptr);
    _projectTreeNode->representedObject()->undoManager().push(reorderCommand);
  }

  return true;
}


QMimeData* FrameListViewModel::mimeData(const QModelIndexList &indexes) const
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
      if(iRASPAStructure *item = static_cast<iRASPAStructure *>(index.internalPointer()))
      {
        stream << item->shared_from_this();
      }
    }
  }
  QMimeData *mimeData = new QMimeData();
  mimeData->setData(iRASPAStructure::mimeType, encodedData);

  return mimeData;
}

bool FrameListViewModel::pasteMimeData(const QMimeData *data, int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);
  Q_UNUSED(parent);

  if(!data->hasFormat(iRASPAStructure::mimeType))
  {
    return false;
  }


  QByteArray ddata = data->data(iRASPAStructure::mimeType);
  QDataStream stream(&ddata, QIODevice::ReadOnly);

  // read application id
  qint64 senderPid;
  stream >> senderPid;
  if (senderPid != QCoreApplication::applicationPid())
  {
    return false;
  }

  // determine insertion point
  int beginRow = row;

  std::vector<std::pair<std::shared_ptr<iRASPAStructure>, size_t>> nodes{};

  while (!stream.atEnd())
  {
    std::shared_ptr<iRASPAStructure> node = std::make_shared<iRASPAStructure>();
    stream >> node;

    nodes.push_back(std::make_pair(node, beginRow));

    beginRow += 1;
  }

  FrameListViewPasteCommand *copyCommand = new FrameListViewPasteCommand(_mainWindow, this, _sceneList, _movie, nodes,
                                                                         _movie->selectedFramesIndexSet(), nullptr);
  _projectTreeNode->representedObject()->undoManager().push(copyCommand);

  return true;
}
