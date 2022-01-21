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

#include "bondlistviewmodel.h"
#include "bondlistpushbuttonstyleditemdelegate.h"

BondListViewModel::BondListViewModel(): _iraspaStructure(std::make_shared<iRASPAObject>())
{
}

void BondListViewModel::resetData()
{
  beginResetModel();
  endResetModel();
}

QModelIndex BondListViewModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  if(_iraspaStructure)
  {
    if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<Structure>(_iraspaStructure->object()))
    {
      const std::vector<std::shared_ptr<SKAsymmetricBond>> bonds = bondViewer->bondSetController()->arrangedObjects();
      if(bonds.empty())
        return QModelIndex();

      return createIndex(row, column, bonds[row].get());
    }
  }
  return QModelIndex();
}

QModelIndex BondListViewModel::parent(const QModelIndex &index) const
{
  Q_UNUSED(index);

  return QModelIndex();
}

void BondListViewModel::setFrame(std::shared_ptr<iRASPAObject> frame)
{
  if(_iraspaStructure != frame)
  {
    beginResetModel();
    _iraspaStructure = frame;
    endResetModel();
  }
}


int BondListViewModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  return 8;
}

int BondListViewModel::rowCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent);

  if(_iraspaStructure)
  {
    if (std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
    {
      return static_cast<int>(bondViewer->bondSetController()->arrangedObjects().size());
    }
    return 0;
  }
  else
    return 0;
}

QVariant BondListViewModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role == Qt::CheckStateRole && index.column() == 0)
  {
    if(SKAsymmetricBond *bond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
    {
      return bond->isVisible() ? Qt::Checked : Qt::Unchecked;
    }
  }

  if (role != Qt::DisplayRole)
    return QVariant();

 //DisplayableProtocol *item = static_cast<DisplayableProtocol*>(index.internalPointer());

  if(SKAsymmetricBond *asymmetricBond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    switch(index.column())
    {
    case 0:
      return QVariant();
    case 1:
      return QString::number(index.row());
    case 2:
      return QVariant();
    case 3:
    {
      int atomicNumber = asymmetricBond->atom1()->elementIdentifier();
      SKElement element = PredefinedElements::predefinedElements[atomicNumber];
      return QString(element._chemicalSymbol);
    }
    case 4:
    {
      int atomicNumber = asymmetricBond->atom2()->elementIdentifier();
      SKElement element = PredefinedElements::predefinedElements[atomicNumber];
      return QString(element._chemicalSymbol);
    }
    case 5:
      return QVariant();
    case 6:
      if(asymmetricBond->copies().size()>0)
      {
        std::shared_ptr<SKBond> bond = asymmetricBond->copies().front();
        if(_iraspaStructure)
        {
          double bondLength = 0.0;
          if (std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
          {
             bondLength = bondViewer->bondLength(bond);
          }
          #if (QT_VERSION >= QT_VERSION_CHECK(5,5,0))
            return QString::asprintf("%5.4f", bondLength);
          #else
            return QString::number(bondLength);
          #endif
        }
        else
        {
          return QString();
        }
      }
      return  0.0;
    case 7:
      return QVariant();
    }
  }

  return QVariant();
}

Qt::ItemFlags BondListViewModel::flags(const QModelIndex &index) const
{
  if (!index.isValid()) return Qt::NoItemFlags;

  Qt::ItemFlags flags = Qt::ItemIsSelectable;

  if ( index.column() == 0)
  {
    flags |= Qt::ItemIsUserCheckable;
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      flags |= Qt::ItemIsTristate;
    #else
      flags |= Qt::ItemIsAutoTristate;
    #endif
    flags |= Qt::ItemIsEditable;
    flags |= Qt::ItemIsEnabled;
  }

  if (std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(_iraspaStructure->object()))
  {
    if ( index.column() != 2 && index.column() != 6)
    {
      flags |= Qt::ItemIsEditable;
    }
    flags |= Qt::ItemIsEnabled;
  }

  return flags;
}

QVariant BondListViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation);
  if (role != Qt::DisplayRole)
    return QVariant();

  switch(section)
  {
    case 0:
      return QString("vis");
    case 1:
      return QString("id");
    case 2:
      return QString("atom ids");
    case 3:
      return QString("el(A)");
    case 4:
      return QString("el(B)");
    case 5:
      return QString("type");
    case 6:
      return QString("length");
    case 7:
      return QString("length");
  }

  return QVariant();
}

bool BondListViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() /*|| role != Qt::EditRole*/)
    return false;

  if(SKAsymmetricBond *bond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    switch(index.column())
    {
    case 0:
      if (role == Qt::CheckStateRole)
      {
        if ((Qt::CheckState)value.toInt() == Qt::Checked)
        {
          bond->setIsVisible(true);
          emit rendererReloadData();
          return true;
        }
        else
        {
          bond->setIsVisible(false);
          emit rendererReloadData();
          return true;
        }
        break;
      case 2:
          if ((Qt::CheckState)value.toInt() == Qt::Checked)
          {
            qDebug() << "checked";
          }
        case 5:
          if (std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(_iraspaStructure->object()))
          {
            if (role == Qt::EditRole)
            {
              bond->setBondType(SKAsymmetricBond::SKBondType(value.toInt()));
              emit rendererReloadData();
              return true;
            }
          }
      default:
        break;
      }
    }
  }

  return false;
}

bool BondListViewModel::insertRow(int position, std::shared_ptr<SKAsymmetricBond> bondNode)
{
  if (std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(_iraspaStructure->object()))
  {
    beginInsertRows(QModelIndex(), position, position);
    bool success = bondEditor->bondSetController()->insertBond(bondNode, position);
    endInsertRows();
    return success;
  }
  return false;
}

bool BondListViewModel::removeRow(int position)
{
  if (std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(_iraspaStructure->object()))
  {
    beginRemoveRows(QModelIndex(), position, position);
    bool success = bondEditor->bondSetController()->removeBond(position);
    endRemoveRows();
    return success;
  }
  return false;
}

QModelIndexList BondListViewModel::selectedIndexes()
{
  QModelIndexList list = QModelIndexList();
  if(_iraspaStructure)
  {
    if(std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(_iraspaStructure->object()))
    {
      for(int localRow : bondViewer->bondSetController()->selectionIndexSet())
      {
        std::shared_ptr<SKAsymmetricBond> node = bondViewer->bondSetController()->arrangedObjects()[localRow];
        QModelIndex index = createIndex(localRow,0,node.get());
        list.push_back(index);
      }
    }
  }
  return list;
}

void BondListViewModel::deleteSelection(std::shared_ptr<Object> object, BondSelectionIndexSet indexSet)
{
  this->beginResetModel();

  if(std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(object))
  {
    bondEditor->bondSetController()->deleteBonds(indexSet);
    bondEditor->bondSetController()->selectionIndexSet().clear();
    bondEditor->bondSetController()->setTags();
  }

  this->endResetModel();
}

void BondListViewModel::deleteSelection(std::shared_ptr<Object> object, BondSelectionNodesAndIndexSet selection)
{
  this->beginResetModel();

  if(std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(object))
  {
    bondEditor->bondSetController()->deleteBonds(selection);
    bondEditor->bondSetController()->selectionIndexSet().clear();
    bondEditor->bondSetController()->setTags();
  }

  this->endResetModel();
}

void BondListViewModel::insertSelection(std::shared_ptr<Object> object, std::vector<std::shared_ptr<SKAsymmetricBond>> bonds, BondSelectionIndexSet indexSet)
{
  this->beginResetModel();

  if(std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(object))
  {
    bondEditor->bondSetController()->insertBonds(bonds, indexSet);
    bondEditor->bondSetController()->selectionIndexSet().insert(indexSet.begin(), indexSet.end());
    bondEditor->bondSetController()->setTags();
  }

  this->endResetModel();
}

void BondListViewModel::insertSelection(std::shared_ptr<Object> object, BondSelectionNodesAndIndexSet selection)
{
  this->beginResetModel();

  if(std::shared_ptr<BondEditor> bondEditor = std::dynamic_pointer_cast<BondEditor>(object))
  {
    bondEditor->bondSetController()->insertSelection(selection);
    bondEditor->bondSetController()->setTags();
  }

  this->endResetModel();
}
