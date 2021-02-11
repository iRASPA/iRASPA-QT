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

#include "bondlistviewcomboboxstyleditemdelegate.h"

#include <QWidget>
#include <QStyle>
#include <QPainter>
#include <QStylePainter>
#include <QApplication>
#include <QPushButton>
#include <QDial>
#include <QDebug>
#include <skasymmetricbond.h>
#include <QProxyStyle>
#include <type_traits>

BondListViewComboBoxStyledItemDelegate::BondListViewComboBoxStyledItemDelegate(QWidget *parent) : QStyledItemDelegate(parent), _singleBondIcon(new QIcon(":/iraspa/singlebond.png"))
{
  _isOneCellInEditMode = false;

  if(QTreeView *view = qobject_cast<QTreeView *>(parent))
  {
     _treeView = view;
     connect(_treeView, SIGNAL(entered(QModelIndex)), this, SLOT(cellEntered(QModelIndex)));
  }

  _singleBondIcon = new QIcon(":/iraspa/singlebond.png");
  _doubleBondIcon = new QIcon(":/iraspa/doublebond.png");
  _partialDoubleBondIcon = new QIcon(":/iraspa/partialdoublebond.png");
  _tripleBondIcon = new QIcon(":/iraspa/triplebond.png");

  _comboBox = new QComboBox(_treeView);
  _comboBox->hide();
  _comboBox->setAutoFillBackground(true);
  _comboBox->addItem(*_singleBondIcon,QString(""));
  _comboBox->addItem(*_doubleBondIcon, QString(""));
  _comboBox->addItem(*_partialDoubleBondIcon, QString(""));
  _comboBox->addItem(*_tripleBondIcon, QString(""));
  _comboBox->setCurrentText("");
  _comboBox->setCurrentIndex(0);
}

void BondListViewComboBoxStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
  // call super to draw selection
  QStyledItemDelegate::paint(painter, option, index);

  if (_isOneCellInEditMode && (_currentEditedCellIndex == index)) return;

  if(SKAsymmetricBond *asymmetricBond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    painter->save();

    _comboBox->resize(option.rect.size());

    QStyleOptionComboBox comboBoxOption;
    comboBoxOption.initFrom(_comboBox);
    comboBoxOption.rect = option.rect;
    comboBoxOption.state = option.state;

    int value = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType());
    comboBoxOption.currentIcon = _comboBox->itemIcon(value);
    comboBoxOption.iconSize = _comboBox->iconSize();

    _comboBox->style()->drawComplexControl(QStyle::CC_ComboBox, &comboBoxOption, painter, nullptr);
    _comboBox->style()->drawControl(QStyle::CE_ComboBoxLabel, &comboBoxOption, painter, nullptr);

    painter->restore();
  }
}

void BondListViewComboBoxStyledItemDelegate::cellEntered(const QModelIndex &index)
{
  if(index.column() == 5)
  {
    if(_isOneCellInEditMode)
    {
      _treeView->closePersistentEditor(_currentEditedCellIndex);
    }
    _treeView->openPersistentEditor(index);
    _isOneCellInEditMode = true;
    _currentEditedCellIndex = index;
  }
  else
  {
    if(_isOneCellInEditMode)
    {
      _isOneCellInEditMode = false;
      _treeView->closePersistentEditor(_currentEditedCellIndex);
    }
  }
}

void BondListViewComboBoxStyledItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, [[maybe_unused]] const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}

QWidget * BondListViewComboBoxStyledItemDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
  if(SKAsymmetricBond *asymmetricBond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    int value = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType());

    QComboBox *realCombobox = new QComboBox(parent);
    realCombobox->setAutoFillBackground(false);
    QSize size = option.rect.size();
    realCombobox->resize(size);

    realCombobox->setAutoFillBackground(true);
    realCombobox->addItem(*_singleBondIcon, QString(""));
    realCombobox->addItem(*_doubleBondIcon, QString(""));
    realCombobox->addItem(*_partialDoubleBondIcon, QString(""));
    realCombobox->addItem(*_tripleBondIcon, QString(""));
    realCombobox->setCurrentIndex(value);

    connect(realCombobox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), realCombobox,
            [this, realCombobox, index]([[maybe_unused]] int value){ setModelData(realCombobox, _treeView->model(), index);});

    return realCombobox;
  }
  return nullptr;
}

void BondListViewComboBoxStyledItemDelegate::setEditorData([[maybe_unused]] QWidget *editor, [[maybe_unused]] const QModelIndex &index) const
{

}

void BondListViewComboBoxStyledItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
  if(QComboBox *comboBox = qobject_cast<QComboBox *>(editor))
  {
    model->setData(index, QVariant(comboBox->currentIndex()), Qt::EditRole);
  }
}

