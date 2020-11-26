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

#include "frameliststyleditemdelegate.h"
#include <qtreeview.h>
#include "iraspastructure.h"
#include "framelistview.h"
#include "framelistviewmodel.h"

FrameListViewStyledItemDelegate::FrameListViewStyledItemDelegate(QWidget* parent) : QStyledItemDelegate(parent)
{

}


void FrameListViewStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if(!index.isValid())
    return;

  QStyledItemDelegate::paint(painter, option, index);

  if(iRASPAStructure *iraspa_structure = static_cast<iRASPAStructure*>(index.internalPointer()))
  {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QStyleOptionViewItem opt = option;
    QStyledItemDelegate::initStyleOption(&opt, index);
    QRect rect = opt.rect;

    // handle selection
    if(option.state & QStyle::State_Selected)
    {
      if(FrameListView *item = qobject_cast<FrameListView*>(this->parent()))
      {
        if(item->isMainSelectedItem(index.row()))
        {
          painter->save();

          #if defined(Q_OS_WIN)
            painter->setPen(Qt::blue);
          #elif defined(Q_OS_MAC)
            painter->setPen(Qt::white);
          #elif defined(Q_OS_LINUX)
            painter->setPen(Qt::white);
          #endif

          painter->drawRoundedRect(rect.adjusted(1,1,-1,-1), 5, 5);

          painter->restore();
        }
      }
    }

    painter->restore();
  }
}
