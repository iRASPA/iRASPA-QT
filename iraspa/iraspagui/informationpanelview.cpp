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

#include "informationpanelview.h"
#include <QPainter>
#include <QPainterPath>

InformationPanelView::InformationPanelView(QWidget * parent):QLabel(parent)
{
  setFixedWidth(340);
  setFixedHeight(30);
    //frame->setFrameStyle(QFrame::Box | QFrame::Raised);
  setFrameStyle(QFrame::Box);
  setLineWidth(2);
}

void InformationPanelView::paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  QLinearGradient grad1(0,0,0,30);
  grad1.setColorAt(0.0,QColor(237,241,225));
  grad1.setColorAt(0.5,QColor(230,235,213));
  grad1.setColorAt(0.5,QColor(222,228,199));
  grad1.setColorAt(1.0,QColor(242,245,224));

  QRectF rectangle(0,0,340,30);
  QPainterPath path;
  path.addRoundedRect(rectangle, 3.5, 3.5);
  QPen pen(Qt::darkGray, 2);
  painter.setPen(pen);
  painter.fillPath(path, grad1);
  painter.drawRoundedRect(rectangle, 3.5, 3.5);
}
