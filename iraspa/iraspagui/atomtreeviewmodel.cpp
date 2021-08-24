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

#include "atomtreeviewmodel.h"
#include "atomtreeviewdropmovecommand.h"
#include "atomtreeviewchangedisplaynamecommand.h"
#include "atomtreeviewchangeelementcommand.h"
#include "atomtreeviewchangepositionxcommand.h"
#include "atomtreeviewchangepositionycommand.h"
#include "atomtreeviewchangepositionzcommand.h"
#include "atomtreeviewchangechargecommand.h"
#include "atomtreeviewchangeuniqueforcefieldnamecommand.h"
#include <vector>
#include <tuple>
#include <QMimeData>
#include <QCoreApplication>

char AtomTreeViewModel::mimeType[] = "application/x-qt-iraspa-mime";

AtomTreeViewModel::AtomTreeViewModel(): _iraspaStructure()
{
}

void AtomTreeViewModel::setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode)
{
  _projectTreeNode = projectTreeNode;
  _projectStructure = nullptr;
  if(std::shared_ptr<iRASPAProject> iRASPAProject = _projectTreeNode->representedObject())
  {
    if(std::shared_ptr<Project> project = iRASPAProject->project())
    {
      if (std::shared_ptr<ProjectStructure> projectStructure = std::dynamic_pointer_cast<ProjectStructure>(project))
      {
        _projectStructure = projectStructure;
      }
    }
  }
}

void AtomTreeViewModel::setMainWindow(MainWindow* mainWindow)
{
  _mainWindow = mainWindow;
}

void AtomTreeViewModel::resetData()
{
  beginResetModel();
  endResetModel();
}

void AtomTreeViewModel::setFrame(std::shared_ptr<iRASPAStructure> frame)
{
  if(_iraspaStructure != frame)
  {
    beginResetModel();
    _iraspaStructure = frame;
    endResetModel();
  }
}


QModelIndexList AtomTreeViewModel::selectedIndexes()
{
  QModelIndexList list = QModelIndexList();
  if(_iraspaStructure)
  {
    std::set<IndexPath> selectedIndexPaths = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths().second;
    for(IndexPath indexPath : selectedIndexPaths)
    {
      std::shared_ptr<SKAtomTreeNode> node = _iraspaStructure->structure()->atomsTreeController()->nodeAtIndexPath(indexPath);
      int localRow = int(indexPath.lastIndex());
      QModelIndex index = createIndex(localRow,0,node.get());
      list.push_back(index);
    }
  }
  return list;
}


SKAtomTreeNode *AtomTreeViewModel::getItem(const QModelIndex &index) const
{
  if (index.isValid())
  {
     SKAtomTreeNode *item = static_cast<SKAtomTreeNode*>(index.internalPointer());
     if (item)
     {
       return item;
     }
  }
  return _iraspaStructure->structure()->atomsTreeController()->hiddenRootNode().get();
}


QModelIndex AtomTreeViewModel::index(int row, int column, const QModelIndex &parent) const
{
  if(!hasIndex(row, column, parent))
     return QModelIndex();

  SKAtomTreeNode *parentNode = nodeForIndex(parent);
  SKAtomTreeNode *childNode = parentNode->childNodes()[row].get();
  return createIndex(row, column, childNode);
}

QModelIndex AtomTreeViewModel::parent(const QModelIndex &child) const
{
  SKAtomTreeNode *childNode = nodeForIndex(child);
  SKAtomTreeNode *parentNode = childNode->parent().get();

  if (_iraspaStructure->structure()->atomsTreeController()->isRootNode(parentNode))
    return QModelIndex();

  int row = rowForNode(parentNode);
  int column = 0;
  return createIndex(row, column, parentNode);
}

int AtomTreeViewModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);
  return 8;
}

int AtomTreeViewModel::rowCount(const QModelIndex &parent) const
{
  if(_iraspaStructure)
  {
    SKAtomTreeNode *parentItem = nodeForIndex(parent);
    if(parentItem)
      return static_cast<int>(parentItem->childCount());
  }
  return 0;
}

QVariant AtomTreeViewModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  SKAtomTreeNode *item = static_cast<SKAtomTreeNode*>(index.internalPointer());
  if(std::shared_ptr<SKAsymmetricAtom> atom = item->representedObject())
  {
    int elementIdentifier = atom->elementIdentifier();

    if(item->isGroup() && index.column() == 0)
    {
      if (role == Qt::CheckStateRole)
      {
        return atom->isVisible() ? Qt::Checked : Qt::Unchecked;
      }
    }

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
      return atom->isVisible() ? Qt::Checked : Qt::Unchecked;
    }

    if ( index.isValid() && role == Qt::ForegroundRole )
    {
      if ( index.column() == 3 )
      {
        if(_mainWindow)
        {
          ForceFieldSets& forceFieldSets = _mainWindow->forceFieldSets();
          QString currentForceFieldName = _iraspaStructure->structure()->atomForceFieldIdentifier();
          ForceFieldSet* currentForceFieldSet = forceFieldSets[currentForceFieldName];
          if(currentForceFieldSet)
          {
            if((*currentForceFieldSet)[atom->uniqueForceFieldName()])
            {
              return QVariant( QColor( Qt::black ) );
            }
          }
        }
        return QVariant( QColor( Qt::red ) );
      }
    }

    if (role != Qt::DisplayRole)
      return QVariant();

    switch(index.column())
    {
    case 0:
      return item->representedObject()->displayName();
    case 1:
      return QString::number(atom->tag());
    case 2:
      return PredefinedElements::predefinedElements[elementIdentifier]._chemicalSymbol;
    case 3:
      return atom->uniqueForceFieldName();
    case 4:
      return QString::number(item->representedObject()->position().x, 'f', 5);
    case 5:
      return QString::number(item->representedObject()->position().y, 'f', 5);
    case 6:
      return QString::number(item->representedObject()->position().z, 'f', 5);
    case 7:
      return QString::number(item->representedObject()->charge(), 'f', 5);
    }
  }

  return QVariant();
}

Qt::ItemFlags AtomTreeViewModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if ( index.column() == 0 )
  {
    flags |= Qt::ItemIsUserCheckable;
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      flags |= Qt::ItemIsTristate;
    #else
      flags |= Qt::ItemIsAutoTristate;
    #endif
    flags |= Qt::ItemIsEditable;
  }

  if ( index.column() >= 2 )
  {
    flags |= Qt::ItemIsEditable;
  }

  flags |= Qt::ItemIsDragEnabled;

  SKAtomTreeNode *node = getItem(index);
  if(node->isGroup())
  {
    flags |= Qt::ItemIsDropEnabled;
  }

  return flags;
}



QVariant AtomTreeViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation);

  if (role != Qt::DisplayRole)
    return QVariant();

  switch(section)
  {
    case 0:
      return QString("vis  name");
    case 1:
      return QString("atom id.");
    case 2:
      return QString("el");
    case 3:
      return QString("ff id.");
    case 4:
      return QString("x");
    case 5:
      return QString("y");
    case 6:
      return QString("z");
    case 7:
      return QString("q");
  }

  return QVariant();
}

bool AtomTreeViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() /*|| role != Qt::EditRole*/)
    return false;

  if (role == Qt::CheckStateRole)
  {
    qDebug() << "role == Qt::CheckStateRole";
    SKAtomTreeNode *item = static_cast<SKAtomTreeNode*>(index.internalPointer());
    if ((Qt::CheckState)value.toInt() == Qt::Checked)
    {
      qDebug() << "Checked";
      item->representedObject()->setVisibility(true);
      emit rendererReloadData();
      return true;
    }
    else
    {
      qDebug() << "UNChecked";
      item->representedObject()->setVisibility(false);
      emit rendererReloadData();
      return true;
    }
  }

  if (role != Qt::EditRole)
       return false;

  SKAtomTreeNode *item = static_cast<SKAtomTreeNode*>(index.internalPointer());
  switch(index.column())
  {
    case 0:
    {
      QString newValue = value.toString();
      AtomTreeViewChangeDisplayNameCommand*changeDisplayNameCommand = new AtomTreeViewChangeDisplayNameCommand(_mainWindow, this, _iraspaStructure, item->shared_from_this(), newValue, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(changeDisplayNameCommand);
      return true;
    }
  case 2:
  {
    QString newValueString = value.toString();
    std::map<QString, int>::iterator newValue = PredefinedElements::atomicNumberData.find(newValueString);

    if (newValue != PredefinedElements::atomicNumberData.end() )
    {
      AtomTreeViewChangeElementCommand*changeElementCommand = new AtomTreeViewChangeElementCommand(_mainWindow, this, _projectStructure, _iraspaStructure, item->shared_from_this(), newValue->second, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(changeElementCommand);
      return true;
    }
    break;
  }
  case 3:
  {
    QString newValueString = value.toString();

    AtomTreeViewChangeUniqueForceFieldNameCommand *changeUniqueForceFieldNameCommand = new AtomTreeViewChangeUniqueForceFieldNameCommand(_mainWindow, this, _projectStructure, _iraspaStructure, item->shared_from_this(), newValueString, nullptr);
    _projectTreeNode->representedObject()->undoManager().push(changeUniqueForceFieldNameCommand);
    return true;
  }
  case 4:
  {
    bool success;
    double newValue = value.toDouble(&success);
    if(success)
    {
      AtomTreeViewChangePositionXCommand*changePositionXCommand = new AtomTreeViewChangePositionXCommand(_mainWindow, this, _projectStructure, _iraspaStructure, item->shared_from_this(), newValue, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(changePositionXCommand);
      return true;
    }
    break;
  }
  case 5:
  {
    bool success;
    double newValue = value.toDouble(&success);
    if(success)
    {
      AtomTreeViewChangePositionYCommand*changePositionYCommand = new AtomTreeViewChangePositionYCommand(_mainWindow, this, _projectStructure, _iraspaStructure, item->shared_from_this(), newValue, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(changePositionYCommand);
      return true;
    }
    break;
  }
  case 6:
  {
    bool success;
    double newValue = value.toDouble(&success);
    if(success)
    {
      AtomTreeViewChangePositionZCommand*changePositionZCommand = new AtomTreeViewChangePositionZCommand(_mainWindow, this, _projectStructure, _iraspaStructure, item->shared_from_this(), newValue, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(changePositionZCommand);
      return true;
    }
    break;
  }
  case 7:
  {
    bool success;
    double newValue = value.toDouble(&success);
    if(success)
    {
      AtomTreeViewChangeChargeCommand*changeChargeCommand = new AtomTreeViewChangeChargeCommand(_mainWindow, this, _iraspaStructure, item->shared_from_this(), newValue, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(changeChargeCommand);
      return true;
    }
    break;
  }
  default:
    break;
  }

  return false;
}

Qt::DropActions AtomTreeViewModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

Qt::DropActions AtomTreeViewModel::supportedDragActions() const
{
   return Qt::CopyAction | Qt::MoveAction;
}

void AtomTreeViewModel::insertNode(SKAtomTreeNode *parentNode, int pos, std::shared_ptr<SKAtomTreeNode> node)
{
  const QModelIndex parent = indexForNode(parentNode);
  int firstRow = pos;
  int lastRow = pos;

  beginInsertRows(parent, firstRow, lastRow);
  parentNode->insertChild(pos, node);
  endInsertRows();
}

void AtomTreeViewModel::removeNode(SKAtomTreeNode *node)
{
  SKAtomTreeNode *parentNode = node->parent().get();
  const QModelIndex parent = indexForNode(parentNode);
  int pos = rowForNode(node);
  int firstRow = pos;
  int lastRow = pos;
  beginRemoveRows(parent, firstRow, lastRow);
  node->removeFromParent();
  endRemoveRows();
}

bool AtomTreeViewModel::removeRows(int position, int count, const QModelIndex &parent)
{
  SKAtomTreeNode *parentItem = getItem(parent);

  if (position < 0 || position > static_cast<int>(parentItem->childNodes().size()))
    return false;

  beginRemoveRows(parent, position, position + count - 1);
  const bool success = parentItem->removeChildren(position, count);
  endRemoveRows();

  return success;
}

bool AtomTreeViewModel::insertRows(int position, int rows, const QModelIndex &parent)
{
  SKAtomTreeNode *parentItem = getItem(parent);

  beginInsertRows(parent, position, position + rows - 1);
  for (int row = 0; row < rows; ++row)
  {
    std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
    std::shared_ptr<SKAtomTreeNode> newItem = std::make_shared<SKAtomTreeNode>(parentItem->shared_from_this(), atom);

    if (!parentItem->insertChild(position, newItem))
      break;
  }
  endInsertRows();

  return true;
}

bool AtomTreeViewModel::insertRow(int position, std::shared_ptr<SKAtomTreeNode> parentNode, std::shared_ptr<SKAtomTreeNode> atomNode)
{
  QModelIndex parentModelIndex = indexForNode(parentNode.get());
  //checkIndex(parentModelIndex);
  beginInsertRows(parentModelIndex, position, position);
  bool success = parentNode->insertChild(position, atomNode);
  endInsertRows();
  return success;
}

bool AtomTreeViewModel::removeRow(int position, std::shared_ptr<SKAtomTreeNode> parentNode)
{
  QModelIndex parent = indexForNode(parentNode.get());
  //checkIndex(parent);

  beginRemoveRows(parent, position, position);
  bool success = parentNode->removeChild(position);
  endRemoveRows();

  return success;
}

bool AtomTreeViewModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
  Q_UNUSED(sourceParent);
  Q_UNUSED(sourceRow);
  Q_UNUSED(count);
  Q_UNUSED(destinationParent);
  Q_UNUSED(destinationChild);
  return true;

  //SKAtomTreeNode *sourceParentNode = static_cast<SKAtomTreeNode*>(sourceParent.internalPointer());
  //SKAtomTreeNode *destinationParentNode = static_cast<SKAtomTreeNode*>(destinationParent.internalPointer());
  //std::shared_ptr<SKAtomTreeNode> childNode = sourceParentNode->getChildNode(sourceRow);

  /*
      // if source and destination parents are the same, move elements locally
      if(sourceParentNode == destinationParentNode)
      {
          // only process if a local move is possible
          if(sourceParentNode->IsMovePossible(sourceRow, count, destinationChild))
          {
              beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
              sourceParentNode->MoveChildren(sourceRow, count, destinationChild);
              endMoveRows();
          }
      } else {
          // otherwise, move the node under the parent
          beginMoveRows(sourceParent, sourceRow, sourceRow+count-1, destinationParent, destinationChild);
          childNode->SetParent(destinationParentNode);
          endMoveRows();
      }
      Scene::GetInstance()->GetRoot()->PrintChildren();
      */
  return true;
}


QStringList AtomTreeViewModel::mimeTypes() const
{
  return QAbstractItemModel::mimeTypes() << mimeType;
}

QMimeData* AtomTreeViewModel::mimeData(const QModelIndexList &indexes) const
{
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  QModelIndexList indexes2 = indexes;
  std::sort(indexes2.begin(), indexes2.end());

  stream << QCoreApplication::applicationPid();

  qulonglong count = static_cast<qulonglong>(indexes2.count());
  stream << count;

  for(auto iter = indexes2.constBegin(); iter != indexes2.constEnd(); ++iter)
  {
    QModelIndex index = *iter;
    if(index.isValid())
    {
      SKAtomTreeNode *atom = getItem(index);
      qulonglong ptrval(reinterpret_cast<qulonglong>(atom));
      stream << ptrval;
    }
  }
  QMimeData *mimeData = new QMimeData();
  mimeData->setData(mimeType, encodedData);

  return mimeData;
}

bool AtomTreeViewModel::hasChildren(const QModelIndex &parent) const
{
  if (!parent.isValid()) return true;

  SKAtomTreeNode *item = static_cast<SKAtomTreeNode*>(parent.internalPointer());
  return item->isGroup();
}

bool AtomTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  Q_UNUSED(column);
  if(_iraspaStructure)
  {
    if(action == Qt::IgnoreAction)
    {
      return true;
    }
    if(!data->hasFormat(mimeType))
    {
      return false;
    }

    QByteArray ddata = data->data(mimeType);
    QDataStream stream(&ddata, QIODevice::ReadOnly);

    SKAtomTreeNode *parentNode = nodeForIndex(parent);

    qint64 senderPid;
    stream >> senderPid;
    if (senderPid != QCoreApplication::applicationPid())
    {
      // Let's not cast pointers that come from another process...
      return false;
    }

    qulonglong count;
    stream >> count;

    int beginRow = row;
    if (beginRow == -1)
    {
      // valid index means: drop onto item. I chose that this should insert
      // a child item, because this is the only way to create the first child of an item...
      // This explains why Qt calls it parent: unless you just support replacing, this
      // is really the future parent of the dropped items.
      if (parent.isValid())
        beginRow = 0;
      else
        // invalid index means: append at bottom, after last toplevel
        beginRow = rowCount(parent);
    }

    if(action == Qt::DropAction::CopyAction)
    {
      insertRows(beginRow,count,parent);
    }
    else
    {
      std::vector<std::shared_ptr<SKAtomTreeNode>> newItems{};
      std::vector<std::tuple<std::shared_ptr<SKAtomTreeNode>, std::shared_ptr<SKAtomTreeNode>, size_t >> moves{};

      while (!stream.atEnd())
      {

        qlonglong nodePtr;
        stream >> nodePtr;
        SKAtomTreeNode *node = reinterpret_cast<SKAtomTreeNode *>(nodePtr);
        std::shared_ptr<SKAtomTreeNode> atom = node->shared_from_this();

        if (atom->row() < beginRow && (parentNode == atom->parent().get()))
        {
          beginRow -= 1;
        }

        moves.push_back(std::make_tuple(atom, parentNode->shared_from_this(), beginRow));

        beginRow += 1;
      }

      AtomSelectionIndexPaths selection = _iraspaStructure->structure()->atomsTreeController()->selectionIndexPaths();
      AtomTreeViewDropMoveCommand *moveCommand = new AtomTreeViewDropMoveCommand(_mainWindow, this, _iraspaStructure, moves, selection, nullptr);
      _projectTreeNode->representedObject()->undoManager().push(moveCommand);
    }

    return true;
  }
  return false;
}

/*
bool AtomTreeViewModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
  if(action == Qt::IgnoreAction)
  {
    return true;
  }
  if(!data->hasFormat(mimeType))
  {
    return false;
  }

  QByteArray ddata = data->data(mimeType);
  QDataStream stream(&ddata, QIODevice::ReadOnly);

  SKAtomTreeNode *parentNode = nodeForIndex(parent);

  qint64 senderPid;
  stream >> senderPid;
  if (senderPid != QCoreApplication::applicationPid())
  {
    // Let's not cast pointers that come from another process...
    return false;
  }

  qulonglong count;
  stream >> count;

  int beginRow = row;
  if (beginRow == -1)
  {
    // valid index means: drop onto item. I chose that this should insert
    // a child item, because this is the only way to create the first child of an item...
    // This explains why Qt calls it parent: unless you just support replacing, this
    // is really the future parent of the dropped items.
    if (parent.isValid())
      beginRow = 0;
    else
      // invalid index means: append at bottom, after last toplevel
      beginRow = rowCount(parent);
  }

  if(action == Qt::DropAction::CopyAction)
  {
     insertRows(beginRow,count,parent);
  }
  else
  {
    std::vector<std::shared_ptr<SKAtomTreeNode>> newItems{};

    emit layoutAboutToBeChanged();
    bool oldState = this->blockSignals(true);
    while (!stream.atEnd())
    {
      int localRow;
      qlonglong nodePtr;
      stream >> nodePtr;
      SKAtomTreeNode *node = reinterpret_cast<SKAtomTreeNode *>(nodePtr);
      std::shared_ptr<SKAtomTreeNode> atom = node->shared_from_this();

      localRow = node->row();
      QModelIndex index = createIndex(localRow,0,atom.get());

      removeRows(localRow,1,index.parent());


      // Adjust destination row for the case of moving an item
      // within the same parent, to a position further down.
      // Its own removal will reduce the final row number by one.
      if (index.row() < beginRow && (parentNode == atom->parent().get()))
      {
        std::cout << "Decrease beginRow" << std::endl;
         --beginRow;
      }

      beginInsertRows(parent, beginRow, beginRow);
      if (!parentNode->insertChild(beginRow, atom))
        break;
      endInsertRows();
      ++beginRow;
    }
    this->blockSignals(oldState);
  }

  _iraspaStructure->structure()->atomsTreeController()->setTags();

  emit layoutChanged();

  return true;
}*/

/*
void AtomTreeViewModel::deleteSelection(std::shared_ptr<Structure> structure, std::vector<std::shared_ptr<SKAtomTreeNode>> atoms)
{
  this->beginResetModel();

  for(std::shared_ptr<SKAtomTreeNode> atom : atoms)
  {
    atom->removeFromParent();
  }
  structure->atomsTreeController()->selectedTreeNodes().clear();
  structure->atomsTreeController()->setTags();

  this->endResetModel();
}

void AtomTreeViewModel::insertSelection(std::shared_ptr<Structure> structure, std::vector<std::shared_ptr<SKAtomTreeNode>> atoms, std::vector<IndexPath> indexPaths)
{
  this->beginResetModel();

  std::vector<std::shared_ptr<SKAtomTreeNode>> reversedAtoms;
  std::reverse_copy(std::begin(atoms), std::end(atoms), std::back_inserter(reversedAtoms));

  std::vector<IndexPath> reversedIndexPaths;
  std::reverse_copy(indexPaths.begin(), indexPaths.end(), std::back_inserter(reversedIndexPaths));

  int index=0;
  for(std::shared_ptr<SKAtomTreeNode> atom : reversedAtoms)
  {
    structure->atomsTreeController()->insertNodeAtIndexPath(atom, reversedIndexPaths[index]);
    structure->atomsTreeController()->selectedTreeNodes().insert(atom);
    index++;
  }
  structure->atomsTreeController()->setTags();
  this->endResetModel();
}
*/


// Helper functions
QModelIndex AtomTreeViewModel::indexForNode(SKAtomTreeNode *node, int column) const
{
  if(_iraspaStructure->structure()->atomsTreeController()->isRootNode(node))
  {
    return QModelIndex();
  }
  int row = rowForNode(node);
  return createIndex(row, column, node);
}

SKAtomTreeNode *AtomTreeViewModel::nodeForIndex(const QModelIndex &index) const
{
  if(index.isValid())
  {
    SKAtomTreeNode *atomNode = static_cast<SKAtomTreeNode *>(index.internalPointer());
    if(atomNode)
      return atomNode;
    else
      return nullptr;
  }
  return _iraspaStructure->structure()->atomsTreeController()->hiddenRootNode().get();
}

int AtomTreeViewModel::rowForNode(SKAtomTreeNode *node) const
{
  return node->row();
}

