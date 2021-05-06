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

#include "bondlistpushbuttonstyleditemdelegate.h"
#include <QWidget>
#include <QStyle>
#include <QPainter>
#include <QStylePainter>
#include <QApplication>
#include <QComboBox>
#include <QCheckBox>
#include <QDial>
#include <QPushButton>
#include <QDebug>
#include <QMouseEvent>
#include "skasymmetricbond.h"
#include <QSpinBox>
#include <QToolButton>
#include <QTreeView>


BondListPushButtonStyledItemDelegate::BondListPushButtonStyledItemDelegate(QWidget* parent) : QStyledItemDelegate(parent)
{
  btn = new QPushButton();
  btn->setCheckable(true);

  btn->setStyleSheet(":!checked {margin: 1px; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                     "stop: 0 #dadbde, stop: 1 #f6f7fa); border: 1px inset #808080; }"
                     ":checked {margin: 1px; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                     "stop: 0 #77a3f4, stop: 1 #2e6af6); border: 1px inset #404040; }");
}

BondListPushButtonStyledItemDelegate::~BondListPushButtonStyledItemDelegate()
{
  delete btn;
}

void BondListPushButtonStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
  // call super to draw selection
  QStyledItemDelegate::paint(painter, option, index);

  if(SKAsymmetricBond *bond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
  {
    painter->save();
    QStyleOptionButton buttonOption;
    buttonOption.text = QString::number(bond->atom1()->tag());
    buttonOption.state = bond->atom1()->isFixed().x ? QStyle::State_On : QStyle::State_Off;
    buttonOption.rect = option.rect;
    buttonOption.rect.setWidth(option.rect.width()/2);
    btn->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, btn);
    painter->restore();

    painter->save();
    painter->translate(option.rect.width()/2,0.0);
    buttonOption.text = QString::number(bond->atom2()->tag());
    buttonOption.state = bond->atom2()->isFixed().x ? QStyle::State_On : QStyle::State_Off;
    btn->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, btn);
    painter->restore();
  }
}


bool BondListPushButtonStyledItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
  if (event->type() == QEvent::MouseButtonRelease)
  {
    QMouseEvent* pME = static_cast<QMouseEvent*>(event);
    if(pME->button() == Qt::LeftButton)
    {
      #if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
        float localXCoordinate = float(pME->localPos().toPoint().x() - option.rect.topLeft().x())/float(option.rect.size().width());
      #else
        float localXCoordinate = float(pME->position().toPoint().x() - option.rect.topLeft().x())/float(option.rect.size().width());
      #endif
      if(SKAsymmetricBond *bond = static_cast<SKAsymmetricBond*>(index.internalPointer()))
      {
        if(localXCoordinate<0.5)
        {
          if(bond->atom1()->isFixed().x == true)
          {
            bond->atom1()->setIsFixed(bool3(false,false,false));
          }
          else
          {
            bond->atom1()->setIsFixed(bool3(true,true,true));
          }
        }
        else
        {
          if(bond->atom2()->isFixed().x == true)
          {
            bond->atom2()->setIsFixed(bool3(false,false,false));
          }
          else
          {
            bond->atom2()->setIsFixed(bool3(true,true,true));
          }
        }

        // update column 2
        if(QTreeView *item = qobject_cast<QTreeView*>(this->parent()))
        {
          int numberOfRows = item->model()->rowCount();
          QModelIndex firstIndex = item->model()->index(0,2, QModelIndex());
          QModelIndex lastIndex = item->model()->index(numberOfRows,2, QModelIndex());
          item->dataChanged(firstIndex, lastIndex);
        }
      }
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}
