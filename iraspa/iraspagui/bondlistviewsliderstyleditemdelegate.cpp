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

#include "bondlistviewsliderstyleditemdelegate.h"
#include <QWidget>
#include <QStyle>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QApplication>
#include <QPushButton>
#include <QDial>
#include <QDebug>
#include <skasymmetricbond.h>
#include "bondlistview.h"

BondListViewSliderStyledItemDelegate::BondListViewSliderStyledItemDelegate(QWidget *parent) : QStyledItemDelegate(parent)
{
  isOneCellInEditMode = false;

  if(BondListView *view = qobject_cast<BondListView *>(parent))
  {
     treeView = view;
     connect(treeView, SIGNAL(entered(QModelIndex)), this, SLOT(cellEntered(QModelIndex)));
  }

  slider = new QDoubleSlider(Qt::Horizontal, treeView);
  slider->hide();
  slider->setDoubleMinimum(0.0);
  slider->setDoubleMaximum(3.0);
  slider->setAutoFillBackground(true);
}

void BondListViewSliderStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
  // call super to draw selection
  QStyledItemDelegate::paint(painter, option, index);

  if (isOneCellInEditMode && (currentEditedCellIndex == index)) return;

  if(SKAsymmetricBond *asymmetricBond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    const QModelIndex bondLengthIndex = index.sibling(index.row(), index.column()-1);
    double bondLength = treeView->model()->data(bondLengthIndex).toDouble();

    painter->save();
    QStyleOptionSlider opt;
    #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      opt.init(slider);
    #else
      opt.initFrom(slider);
    #endif
    opt.orientation = Qt::Horizontal;
    opt.minimum = slider->minimum();
    opt.maximum = slider->maximum();
    opt.rect = option.rect;
    slider->setDoubleValue(bondLength);
    opt.sliderPosition = slider->sliderPosition();
    opt.sliderValue = slider->value();
    opt.subControls = QStyle::SC_SliderHandle | QStyle::SC_SliderGroove;

    #ifdef Q_OS_MAC
      painter->translate(option.rect.left(),0);
    #endif

    slider->style()->drawComplexControl(QStyle::CC_Slider, &opt, painter, slider);

    painter->restore();
  }
}

void BondListViewSliderStyledItemDelegate::cellEntered(const QModelIndex &index)
{
  if(index.column() == 7)
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

void BondListViewSliderStyledItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  Q_UNUSED(index);
  editor->setGeometry(option.rect);
}

QWidget * BondListViewSliderStyledItemDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
  if(SKAsymmetricBond *asymmetricBond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    const QModelIndex bondLengthIndex = index.sibling(index.row(), index.column()-1);
    double bondLength = treeView->model()->data(bondLengthIndex).toDouble();

    QDoubleSlider *realSlider = new QDoubleSlider(Qt::Horizontal, parent);
    realSlider->setDoubleMinimum(0);
    realSlider->setDoubleMaximum(3.0);
    realSlider->setDoubleValue(bondLength);
    realSlider->setAutoFillBackground(false);
    QSize size = option.rect.size();
    realSlider->resize(size);

    connect(realSlider, static_cast<void (QDoubleSlider::*)(double)>(&QDoubleSlider::sliderMoved), realSlider,
            [this, realSlider, index]([[maybe_unused]] double value){ setModelData(realSlider, treeView->model(), index);});

    return realSlider;
  }
  return nullptr;
}

void BondListViewSliderStyledItemDelegate::setSliderValue(double level)
{
  Q_UNUSED(level);
}


void BondListViewSliderStyledItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  Q_UNUSED(editor);
  Q_UNUSED(index);
}

void BondListViewSliderStyledItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
  if(QDoubleSlider *sliderEditor = qobject_cast<QDoubleSlider *>(editor))
  {
    model->setData(index, QVariant(sliderEditor->doubleValue()), Qt::EditRole);
  }
}

