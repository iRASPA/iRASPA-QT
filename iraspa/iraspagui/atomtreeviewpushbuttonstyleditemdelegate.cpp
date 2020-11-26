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

#include "atomtreeviewpushbuttonstyleditemdelegate.h"
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
#include <QSizePolicy>

 AtomTreeViewPushButtonStyledItemDelegate::AtomTreeViewPushButtonStyledItemDelegate(QWidget* parent): QStyledItemDelegate(parent)
 {
   btn = new QPushButton();
   btn->setCheckable(true);

   btn->setStyleSheet(tr(":!checked {margin: 1px; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                         "stop: 0 #eaebee, stop: 1 #f8f9fc); border: 1px inset #808080 ; }"
                         ":checked {margin: 1px; border-radius: 5px; background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
                         "stop: 0 #77a3f4, stop: 1 #2e6af6); border: 1px inset #404040; }"));
 }

 AtomTreeViewPushButtonStyledItemDelegate::~AtomTreeViewPushButtonStyledItemDelegate()
 {
   delete btn;
 }

 void AtomTreeViewPushButtonStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
 {
   // call super to draw selection
   QStyledItemDelegate::paint(painter, option, index);

   if(SKAtomTreeNode *atom = static_cast<SKAtomTreeNode*>(index.internalPointer()))
   {
     painter->save();
     QStyleOptionButton buttonOption;
     buttonOption.text = QString::number(atom->representedObject()->tag());
     buttonOption.state = atom->representedObject()->isFixed().x ? QStyle::State_On : QStyle::State_Off;
     buttonOption.rect = option.rect;
     btn->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, btn);
     painter->restore();
   }
 }


 bool AtomTreeViewPushButtonStyledItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
 {
   if (index.column() == 1)
   {
     if (event->type() == QEvent::MouseButtonRelease)
     {
       QMouseEvent* pME = static_cast<QMouseEvent*>(event);
       if(pME->button() == Qt::LeftButton)
       {
         if(SKAtomTreeNode *atom = static_cast<SKAtomTreeNode*>(index.internalPointer()))
         {
           if(atom->representedObject()->isFixed().x == true)
           {
             atom->representedObject()->setIsFixed(bool3(false,false,false));
           }
           else
           {
             atom->representedObject()->setIsFixed(bool3(true,true,true));
           }

           // update column 2
           if(QTreeView *item = qobject_cast<QTreeView*>(this->parent()))
           {
             int numberOfRows = item->model()->rowCount();
             QModelIndex firstIndex = item->model()->index(0,1, QModelIndex());
             QModelIndex lastIndex = item->model()->index(numberOfRows,1, QModelIndex());
             item->dataChanged(firstIndex, lastIndex);
           }
         }
       }
     }
   }
   return QStyledItemDelegate::editorEvent(event, model, option, index);
 }
