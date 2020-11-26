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

#include "iraspatoolbar.h"
#include <QHBoxLayout>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include "informationpanelview.h"

iRASPAToolBar::iRASPAToolBar(QWidget * parent):QToolBar(parent)
{
  setMovable(false);
  setFloatable(false);

  InformationPanelView *frame = new InformationPanelView(this);
  //frame->setStyleSheet("background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #3783a5, stop: 1 #1e698c);");




  QHBoxLayout *layout = new QHBoxLayout;
  QWidget *separator = new QWidget();
  separator->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  addWidget(separator);

  addWidget(frame);
  layout->setAlignment(frame, Qt::AlignHCenter);

  QWidget *separator2 = new QWidget();
  separator2->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  addWidget(separator2);


  left=new QToolButton(this);
  left->setIcon(QIcon(":/iraspa/leftpanelcollapse.png"));
  left->setIconSize(QSize(left->size().height(),left->size().height()));
  left->setCheckable(true);
  left->setChecked(true);
  left->setDown(true);
  //left->setFixedWidth(24);
  //left->setFixedHeight(24);
  left->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
  addWidget(left);
  QObject::connect(left, SIGNAL(toggled(bool)),this,SLOT(reactToToggleHideLeft(bool)));

  down=new QToolButton(this);
  down->setIcon(QIcon(":/iraspa/bottompanelcollapse.png"));
  down->setIconSize(QSize(down->size().height(),down->size().height()));
  down->setCheckable(true);
  down->setChecked(true);
  down->setDown(true);
  //down->setFixedWidth(24);
  //down->setFixedHeight(24);
  down->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
  addWidget(down);
  QObject::connect(down, SIGNAL(toggled(bool)),this,SLOT(reactToToggleHideDown(bool)));

  right=new QToolButton(this);
  right->setIcon(QIcon(":/iraspa/rightpanelcollapse.png"));
  right->setIconSize(QSize(right->size().height(),right->size().height()));
  right->setCheckable(true);
  right->setChecked(true);
  right->setDown(true);
  //right->setFixedWidth(24);
  //right->setFixedHeight(24);
  right->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
  addWidget(right);
  QObject::connect(right, SIGNAL(toggled(bool)),this,SLOT(reactToToggleHideRight(bool)));

  QWidget *separator3 = new QWidget();
  separator3->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
  addWidget(separator3);
}

void iRASPAToolBar::reactToToggleHideLeft(bool checked)
{
  if (checked)
  {
    left->setDown(true);
  }
  else
  {
    left->setDown(false);
  }
}

void iRASPAToolBar::reactToToggleHideDown(bool checked)
{
  if (checked)
  {
    down->setDown(true);
  }
  else
  {
    down->setDown(false);
  }
}

void iRASPAToolBar::reactToToggleHideRight(bool checked)
{
  if (checked)
  {
    right->setDown(true);
  }
  else
  {
    right->setDown(false);
  }
}

QSize iRASPAToolBar::sizeHint() const
{
    return QSize(1200, 40);
}
