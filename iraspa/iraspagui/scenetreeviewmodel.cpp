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

#include "scenetreeviewmodel.h"
#include "scenetreeviewdropmovecommand.h"
#include "scenetreeviewdropcopycommand.h"
#include "scenetreeviewchangescenedisplaynamecommand.h"
#include "scenetreeviewchangemoviedisplaynamecommand.h"
#include "scenetreeviewpasteprojectscommand.h"
#include <optional>

SceneTreeViewModel::SceneTreeViewModel(): _sceneList(std::make_shared<SceneList>())
{

}

void SceneTreeViewModel::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
}

void SceneTreeViewModel::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  beginResetModel();
  _projectTreeNode = projectTreeNode;
  _projectStructure = nullptr;
  _sceneList = nullptr;
  if(projectTreeNode)
  {
    if(std::shared_ptr<iRASPAProject> iraspaProject = projectTreeNode->representedObject())
    {
      if(std::shared_ptr<Project> project = iraspaProject->project())
      {
        if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
        {
          _projectStructure = projectStructure;
          _sceneList = projectStructure->sceneList();
        }
      }
    }
  }
  endResetModel();
}


std::shared_ptr<Scene> SceneTreeViewModel::parentForMovie(const std::shared_ptr<Movie> movie) const
{
  //return movie->parent().lock();


  if(_sceneList)
  {
    for(std::shared_ptr<Scene> currentScene: _sceneList->scenes())
    {
      for(std::shared_ptr<Movie> currentMovie: currentScene->movies())
      {
        if (currentMovie == movie)
        {
          return currentScene;
        }
      }
    }
  }
  return nullptr;
}

DisplayableProtocol *SceneTreeViewModel::getItem(const QModelIndex &index) const
{
  if(index.isValid())
  {
    DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer());
    if (item)
      return item;
    else
      return nullptr;
   }
   return _sceneList.get();
}


bool SceneTreeViewModel::isMainSelectedItem(std::shared_ptr<Movie> movie)
{
  if(_sceneList)
  {
    if(std::shared_ptr<Scene> selectedScene = _sceneList->selectedScene())
    {
      return selectedScene->selectedMovie() == movie;
    }
  }
  return false;
}

QModelIndex SceneTreeViewModel::indexOfMainSelected() const
{
  if(_sceneList)
  {
    if(std::shared_ptr<Scene> selectedScene = _sceneList->selectedScene())
    {
      if(std::shared_ptr<Movie> selectedMovie = selectedScene->selectedMovie())
      {
        if (std::optional<int> row = selectedScene->findChildIndex(selectedMovie))
        {
          return createIndex(*row, 0, selectedMovie.get());
        }
      }
    }
  }
  return QModelIndex();
}

QModelIndex SceneTreeViewModel::indexForItem(std::shared_ptr<Movie> movie)
{
  std::shared_ptr<Scene> parent = parentForMovie(movie);
  if (std::optional<int> row = parent->findChildIndex(movie))
  {
    return createIndex(*row, 0, movie.get());
  }
  qDebug() << "CAN NOT FIND ITEM";
  return QModelIndex();
}

QModelIndex SceneTreeViewModel::indexForItem(std::shared_ptr<Scene> scene)
{
  if (std::optional<int> row = _sceneList->findChildIndex(scene))
  {
    return index(*row,0, QModelIndex());
  }
  return QModelIndex();
}



// Returns the index of the item in the model specified by the given row, column and parent index.
QModelIndex SceneTreeViewModel::index(int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(column);

  if (parent.isValid() && parent.column() != 0)
    return QModelIndex();

  if(_sceneList)
  {
    DisplayableProtocol *parentItem = getItem(parent);

    if (!parentItem)
      return QModelIndex();

    if(dynamic_cast<SceneList*>(parentItem))
    {
      Scene* scene = _sceneList->scenes()[row].get();
      return createIndex(row, 0, scene);
    }

    if(Scene* scene = dynamic_cast<Scene*>(parentItem))
    {
      Movie* movie = scene->movies()[row].get();
      return createIndex(row, 0, movie);
    }
  }
  return QModelIndex();
}


QModelIndex SceneTreeViewModel::parent(const QModelIndex &index) const
{
  if (!index.isValid())
    return QModelIndex();

  if(!index.internalPointer())
    return QModelIndex();

  if(DisplayableProtocol *item = getItem(index))
  {
    if(Movie* movie = dynamic_cast<Movie*>(item))
    {
      if(std::shared_ptr<Scene> scene = parentForMovie(movie->shared_from_this()))
      {
        std::optional<int> row = _sceneList->findChildIndex(scene);

        if(row)
        {
          return createIndex(*row,0,scene.get());
        }
      }
    }
  }

  return QModelIndex();
}


int SceneTreeViewModel::rowCount(const QModelIndex &parent) const
{
  //checkIndex(parent);
  if(_sceneList)
  {
    if(!parent.isValid())
    {
      return static_cast<int>(_sceneList->scenes().size());
    }

    if(DisplayableProtocol *parentItem = static_cast<DisplayableProtocol*>(parent.internalPointer()))
    {
      if(SceneList* parentSceneList = dynamic_cast<SceneList*>(parentItem))
      {
        return static_cast<int>(parentSceneList->scenes().size());
      }
      if(Scene* scene = dynamic_cast<Scene*>(parentItem))
      {
        return static_cast<int>(scene->movies().size());
      }
    }
  }
  return 0;
}

int SceneTreeViewModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return 1;
}

QVariant SceneTreeViewModel::data(const QModelIndex &index, int role) const
{
  //checkIndex(index);
  if (!index.isValid())
   return QVariant();

  DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer());

  if (role == Qt::CheckStateRole)
  {
    if(Movie* movie = dynamic_cast<Movie*>(item))
    {
      return movie->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
  }

  if (role == Qt::ForegroundRole)
  {
    DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer());
    if(Scene* scene = dynamic_cast<Scene*>(item))
    {
      return QColor(114,114,114,255);
    }
  }

  if (role == Qt::FontRole)
  {
    if(Scene* scene = dynamic_cast<Scene*>(item))
    {
      QFont font = QFont();
      font.setFamily("Lucida Grande");
      font.setBold(true);
      return font;
    }
  }

  if (role != Qt::DisplayRole)
   return QVariant();


  if(Movie* movie = dynamic_cast<Movie*>(item))
  {
    return movie->displayName();
  }
  if(Scene* scene = dynamic_cast<Scene*>(item))
  {
    return scene->displayName().toUpper();
  }
  return QString(item->displayName());
}

bool SceneTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid())
  {
    return false;
  }

  if(DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer()))
  {
    if(Movie* movie = dynamic_cast<Movie*>(item))
    {
      if (role == Qt::CheckStateRole)
      {
        std::shared_ptr<Scene> scene = parentForMovie(movie->shared_from_this());
        std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures = _sceneList->selectediRASPARenderStructures();
        if ((Qt::CheckState)value.toInt() == Qt::Checked)
        {
          movie->setVisibility(true);
          emit invalidateCachedAmbientOcclusionTexture(structures);
          emit rendererReloadData();
          return true;
        }
        else
        {
          movie->setVisibility(false);
          emit invalidateCachedAmbientOcclusionTexture(structures);
          emit rendererReloadData();
          return true;
        }
      }
    }

    if (role != Qt::EditRole)
      return false;

    QString newValue = value.toString();
    if(!newValue.isEmpty())
    {
      if(Movie* movie = dynamic_cast<Movie*>(item))
      {
        SceneTreeViewChangeMovieDisplayNameCommand*changeDisplayNameCommand = new SceneTreeViewChangeMovieDisplayNameCommand(_mainWindow, this, movie->shared_from_this(), newValue, nullptr);
        _projectTreeNode->representedObject()->undoManager().push(changeDisplayNameCommand);
        return true;
      }
      if(Scene* scene = dynamic_cast<Scene*>(item))
      {
        SceneTreeViewChangeSceneDisplayNameCommand*changeDisplayNameCommand = new SceneTreeViewChangeSceneDisplayNameCommand(_mainWindow, this, scene->shared_from_this(), newValue, nullptr);
        _projectTreeNode->representedObject()->undoManager().push(changeDisplayNameCommand);
        return true;
      }
    }
  }
  return false;
}


Qt::ItemFlags SceneTreeViewModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
  {
    return Qt::ItemIsDropEnabled;
  }

  Qt::ItemFlags flags = Qt::NoItemFlags;
  flags |= Qt::ItemIsEnabled;

  if(_projectTreeNode->isEditable())
  {
    flags |= Qt::ItemIsEditable;
  }

  if(DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer()))
  {
    if(Scene* scene = dynamic_cast<Scene*>(item))
    {
      flags |= Qt::ItemIsDropEnabled;
      return flags;
    }

    if(Movie* movie = dynamic_cast<Movie*>(item))
    {
      flags |= Qt::ItemIsSelectable;
      flags |= Qt::ItemIsUserCheckable;
      flags |= Qt::ItemIsDragEnabled;
      return flags;
    }
  }

  return Qt::NoItemFlags;
}

bool SceneTreeViewModel::hasChildren(const QModelIndex &parent) const
{
  if (!parent.isValid()) return true;

  if(DisplayableProtocol *item = static_cast<DisplayableProtocol*>(parent.internalPointer()))
  {
    if(Movie* movie = dynamic_cast<Movie*>(item))
    {
      return false;
    }
  }
  return true;
}

bool SceneTreeViewModel::removeRows(int position, int count, const QModelIndex &parent)
{
  qDebug() << "SceneTreeViewModel::removeRows";
  if(parent.isValid())
  {

    DisplayableProtocol *item = static_cast<DisplayableProtocol*>(parent.internalPointer());


    qDebug() << "SceneTreeViewModel::removeRows valid" << item;
    if(Movie* movie = dynamic_cast<Movie*>(item))
    {
      std::shared_ptr<Scene> scene = parentForMovie(movie->shared_from_this());
      qDebug() << "is movie!! " << position << ", " << count;
      if (position < 0 || position > static_cast<int>(scene->movies().size()))
        return false;

      beginRemoveRows(parent, position, position + count - 1);
      const bool success = scene->removeChildren(position, count);
      endRemoveRows();
       qDebug() << "endRemoveRows end ";
      return success;
    }

    if(Scene* scene = dynamic_cast<Scene*>(item))
    {
      qDebug() << "SceneTreeViewModel::removeRows bot valid 1";
      if (position < 0 || position > static_cast<int>(scene->movies().size()))
        return false;

      qDebug() << "scene parent" + scene->displayName() + ", " << position << ", " << count;
      beginRemoveRows(parent, position, position + count - 1);
      const bool success = scene->removeChildren(position, count);
      endRemoveRows();
       qDebug() << "endRemoveRows end ";

      return success;
    }
  }
  else
  {
    qDebug() << "SceneTreeViewModel::removeRows NOT valid";

    if (position < 0 || position > static_cast<int>(_sceneList->scenes().size()))
      return false;

    qDebug() << "root parent, " << position << ", " << count;
    beginRemoveRows(parent, position, position + count - 1);
    const bool success = _sceneList->removeChildren(position, count);
    endRemoveRows();
    qDebug() << "endRemoveRows end ";

    return success;
  }
  return false;
}

bool SceneTreeViewModel::insertRows(int position, int rows, const QModelIndex &parent)
{
  Q_UNUSED(position);
  Q_UNUSED(rows);
  Q_UNUSED(parent);
  qFatal("fatal error");
  return false;
}

bool SceneTreeViewModel::insertRow(int position, std::shared_ptr<Scene> scene, std::shared_ptr<Movie> movie)
{
  QModelIndex parentModelIndex = indexForItem(scene);
  //checkIndex(parentModelIndex);
  beginInsertRows(parentModelIndex, position, position);
  scene->insertChild(position, movie);
  endInsertRows();

  return true;
}

bool SceneTreeViewModel::insertRow(int position, std::shared_ptr<Scene> scene)
{
  QModelIndex parent = QModelIndex();
  beginInsertRows(parent, position, position);
  _sceneList->insertChild(position, scene);
  endInsertRows();

  return true;
}

bool SceneTreeViewModel::removeRow(int position)
{
  QModelIndex parent = QModelIndex();
  beginRemoveRows(parent, position, position);
  bool success = _sceneList->removeChild(position);
  endRemoveRows();

  return success;
}

bool SceneTreeViewModel::removeRow(int position, std::shared_ptr<Scene> scene, std::shared_ptr<Movie> movie)
{
  Q_UNUSED(movie);
  QModelIndex parent = indexForItem(scene);

  beginRemoveRows(parent, position, position);
  bool success = scene->removeChild(position);
  endRemoveRows();

  return success;
}

QStringList SceneTreeViewModel::mimeTypes() const
{
  return QAbstractItemModel::mimeTypes() << Movie::mimeType;
}


Qt::DropActions SceneTreeViewModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions SceneTreeViewModel::supportedDragActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QMimeData* SceneTreeViewModel::mimeData(const QModelIndexList &indexes) const
{
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  QModelIndexList sortedIndexes = indexes;
  std::sort(sortedIndexes.begin(), sortedIndexes.end());

  stream << QCoreApplication::applicationPid();
  stream << sortedIndexes.count();

  for(const QModelIndex &index: sortedIndexes)
  {
    if(index.isValid())
    {
      DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer());
      if(Movie* movie = dynamic_cast<Movie*>(item))
      {
        stream << movie->shared_from_this();
      }
    }
  }
  QMimeData *mimeData = new QMimeData();
  mimeData->setData(Movie::mimeType, encodedData);

  return mimeData;
}

QMimeData* SceneTreeViewModel::mimeDataLazy(const QModelIndexList &indexes) const
{
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  QModelIndexList sortedIndexes = indexes;
  std::sort(sortedIndexes.begin(), sortedIndexes.end());

  stream << QCoreApplication::applicationPid();
  stream << sortedIndexes.count();

  for(const QModelIndex &index: sortedIndexes)
  {
    if(index.isValid())
    {
      DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer());
      if(Movie* movie = dynamic_cast<Movie*>(item))
      {
        qulonglong ptrval(reinterpret_cast<qulonglong>(movie));
        stream << ptrval;
      }
    }
  }
  QMimeData *mimeData = new QMimeData();
  mimeData->setData(Movie::mimeType, encodedData);

  return mimeData;
}

bool SceneTreeViewModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(data);
  Q_UNUSED(action);
  Q_UNUSED(row);
  Q_UNUSED(column);
  Q_UNUSED(parent);

  //qDebug() << "Can drop on row: " << row << ", " << column << ", " << parent;


  return true;
}

// drops onto existing items have row and column set to -1 and parent set to the current item
bool SceneTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);

  qDebug() << "row: " << row << ", " << column << ", " << parent;

  if(action == Qt::IgnoreAction)
  {
    return true;
  }
  if(!data->hasFormat(Movie::mimeType))
  {
    return false;
  }

  QByteArray ddata = data->data(Movie::mimeType);
  QDataStream stream(&ddata, QIODevice::ReadOnly);

  qint64 senderPid;
  stream >> senderPid;
  if (senderPid != QCoreApplication::applicationPid())
  {
    return false;
  }

  DisplayableProtocol *item = static_cast<DisplayableProtocol*>(parent.internalPointer());
  Scene* scene = dynamic_cast<Scene*>(item);

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

  if(action == Qt::DropAction::CopyAction)
  {
    qDebug() << "Qt::DropAction::CopyAction";

    std::shared_ptr<Scene> realParent=nullptr;
    int realRow=0;

    if(scene)
    {
      realParent = scene->shared_from_this();
      realRow = beginRow;
    }

    if(row == -1 && column == -1 && !parent.isValid())
    {
      realParent = nullptr;
    }

    std::vector<std::shared_ptr<Movie>> movies{};
    while (!stream.atEnd())
    {
      qlonglong nodePtr;
      stream >> nodePtr;
      Movie *moviePtr = reinterpret_cast<Movie *>(nodePtr);
      std::shared_ptr<Movie> copiedMovie = moviePtr->shallowClone();
      // FIX
      //std::optional<int> selectionIndex = _sceneList->selectedFrameIndex();
      //if(selectionIndex)
      //{
      //  copiedMovie->setSelectedFrameIndex(*selectionIndex);
      //}
      movies.push_back(copiedMovie);
    }

    SceneTreeViewDropCopyCommand *moveCommand = new SceneTreeViewDropCopyCommand(_mainWindow, this, _frameModel, _sceneList, realParent, realRow, movies, _sceneList->allSelection(), nullptr);
    _projectTreeNode->representedObject()->undoManager().push(moveCommand);
  }
  else if(action == Qt::DropAction::MoveAction)
  {
    qDebug() << "Qt::DropAction::MoveAction";

    std::shared_ptr<Scene> realParent=nullptr;
    int realRow=0;

    if(scene)
    {
      realParent = scene->shared_from_this();
      realRow = beginRow;
    }

    if(row == -1 && column == -1 && !parent.isValid())
    {
      realParent = nullptr;
    }

    std::vector<std::shared_ptr<Movie>> movies{};
    while (!stream.atEnd())
    {
      qlonglong nodePtr;
      stream >> nodePtr;
      Movie *node = reinterpret_cast<Movie *>(nodePtr);
      movies.push_back(node->shared_from_this());
    }

    SceneTreeViewDropMoveCommand *moveCommand = new SceneTreeViewDropMoveCommand(_mainWindow, this, _frameModel, _sceneList, realParent, realRow, movies, _sceneList->allSelection(), nullptr);
    _projectTreeNode->representedObject()->undoManager().push(moveCommand);
  }

  return true;
}

bool SceneTreeViewModel::pasteMimeData(const QMimeData *data, int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);

  if(!data->hasFormat(Movie::mimeType))
  {
    return false;
  }

  QByteArray ddata = data->data(Movie::mimeType);
  QDataStream stream(&ddata, QIODevice::ReadOnly);

  qint64 senderPid;
  stream >> senderPid;
  if (senderPid != QCoreApplication::applicationPid())
  {
    return false;
  }

  DisplayableProtocol *parentItem = getItem(parent);

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

  std::vector<std::shared_ptr<Movie>> movies{};
  while (!stream.atEnd())
  {
    std::shared_ptr<Movie> copiedMovie = std::make_shared<Movie>();
    stream >> copiedMovie;
    // FIX
    //copiedMovie->setSelectedFrameIndex(copiedMovie->selectedFrameIndex());
    movies.push_back(copiedMovie);
  }

  SceneTreeViewPasteProjectsCommand *moveCommand = new SceneTreeViewPasteProjectsCommand(_mainWindow, this, _projectStructure, _sceneList.get(), parentItem, beginRow, movies, _sceneList->allSelection(), nullptr);
  _projectTreeNode->representedObject()->undoManager().push(moveCommand);

  return true;
}
