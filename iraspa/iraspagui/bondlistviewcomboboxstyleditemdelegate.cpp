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

BondListViewComboBoxStyledItemDelegate::BondListViewComboBoxStyledItemDelegate(QWidget *parent) : QStyledItemDelegate(parent), singleBondIcon(new QIcon(":/iraspa/singlebond.png"))
{
  isOneCellInEditMode = false;

  if(QTreeView *view = qobject_cast<QTreeView *>(parent))
  {
     treeView = view;
     connect(treeView, SIGNAL(entered(QModelIndex)), this, SLOT(cellEntered(QModelIndex)));
  }

  singleBondIcon = new QIcon(":/iraspa/singlebond.png");
  doubleBondIcon = new QIcon(":/iraspa/doublebond.png");
  partialDoubleBondIcon = new QIcon(":/iraspa/partialdoublebond.png");
  tripleBondIcon = new QIcon(":/iraspa/triplebond.png");

  comboBox = new QComboBox(treeView);
  comboBox->hide();
  comboBox->setAutoFillBackground(true);
  comboBox->addItem(*singleBondIcon,QString(""));
  comboBox->addItem(*doubleBondIcon, QString(""));
  comboBox->addItem(*partialDoubleBondIcon, QString(""));
  comboBox->addItem(*tripleBondIcon, QString(""));
  comboBox->setCurrentText("");
  comboBox->setCurrentIndex(0);
}

void BondListViewComboBoxStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
  // call super to draw selection
  QStyledItemDelegate::paint(painter, option, index);

  if (isOneCellInEditMode && (currentEditedCellIndex == index)) return;

  if(SKAsymmetricBond *asymmetricBond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    painter->save();

    comboBox->resize(option.rect.size());

    QStyleOptionComboBox comboBoxOption;
    comboBoxOption.initFrom(comboBox);
    comboBoxOption.rect = option.rect;
    comboBoxOption.state = option.state;

    int value = static_cast<typename std::underlying_type<SKAsymmetricBond::SKBondType>::type>(asymmetricBond->getBondType());
    comboBoxOption.currentIcon = comboBox->itemIcon(value);
    comboBoxOption.iconSize = comboBox->iconSize();

    comboBox->style()->drawComplexControl(QStyle::CC_ComboBox, &comboBoxOption, painter, nullptr);
    comboBox->style()->drawControl(QStyle::CE_ComboBoxLabel, &comboBoxOption, painter, nullptr);

    painter->restore();
  }
}

void BondListViewComboBoxStyledItemDelegate::cellEntered(const QModelIndex &index)
{
  if(index.column() == 5)
  {
    if(isOneCellInEditMode)
    {
      treeView->closePersistentEditor(currentEditedCellIndex);
    }
    treeView->openPersistentEditor(index);
    isOneCellInEditMode = true;
    currentEditedCellIndex = index;
  }
  else
  {
    if(isOneCellInEditMode)
    {
      isOneCellInEditMode = false;
      treeView->closePersistentEditor(currentEditedCellIndex);
    }
  }
}

void BondListViewComboBoxStyledItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
    realCombobox->addItem(*singleBondIcon, QString(""));
    realCombobox->addItem(*doubleBondIcon, QString(""));
    realCombobox->addItem(*partialDoubleBondIcon, QString(""));
    realCombobox->addItem(*tripleBondIcon, QString(""));
    realCombobox->setCurrentIndex(value);

    connect(realCombobox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), realCombobox,
            [this, realCombobox, index](int value){ setModelData(realCombobox, treeView->model(), index);});

    return realCombobox;
  }
  return nullptr;
}

void BondListViewComboBoxStyledItemDelegate::setEditorData([[maybe_unused]] QWidget *editor, const QModelIndex &index) const
{
  //qDebug() << "setEditorData " << index.row();
    ////setup the editor - your data are in index.data(Qt::DataRoles) - stored in a QVariant;
    //QString value = index.model()->data(index,Qt::EditRole).toString();
    //SubscriberForm *subscriberForm =  static_cast<SubscriberForm*>(editor);
}

void BondListViewComboBoxStyledItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
  if(QComboBox *comboBox = qobject_cast<QComboBox *>(editor))
  {
    model->setData(index, QVariant(comboBox->currentIndex()), Qt::EditRole);
  }
}

