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

#include "mastertoolbar.h"
#include <QLayout>
#include "mainwindow.h"

MasterToolBar::MasterToolBar(QWidget * parent):QToolBar(parent)
{
  // the first toolbutton is the "project"-tab
  buttonToProjectTab = new QToolButton(this);
  buttonToProjectTab->setGeometry(0,0,24,24);
  buttonToProjectTab->setMinimumSize(24,24);
  buttonToProjectTab->setMaximumSize(24,24);
  buttonToProjectTab->setIcon(QIcon(":/iraspa/projectlistcontroller.png"));
  buttonToProjectTab->setContentsMargins(0,0,0,0);
  buttonToProjectTab->setCheckable(true);
  buttonToProjectTab->setChecked(true);
  buttonToProjectTab->setDown(true);
  addWidget(buttonToProjectTab);

  // the second toolbutton is the "detail"-tab (scene for crystals, vasp-input files for VASP,...)
  buttonToMovieViewTab = new QToolButton(this);
  buttonToMovieViewTab->setGeometry(0,0,24,24);
  buttonToMovieViewTab->setMinimumSize(24,24);
  buttonToMovieViewTab->setMaximumSize(24,24);
  buttonToMovieViewTab->setIcon(QIcon(":/iraspa/movielistcontroller.png"));
  buttonToMovieViewTab->setContentsMargins(0,0,0,0);
  buttonToMovieViewTab->setCheckable(true);
  buttonToMovieViewTab->setChecked(false);
  addWidget(buttonToMovieViewTab);

  // the second toolbutton is the "detail"-tab (scene for crystals, vasp-input files for VASP,...)
  buttonFrameViewTab = new QToolButton(this);
  buttonFrameViewTab->setGeometry(0,0,24,24);
  buttonFrameViewTab->setMinimumSize(24,24);
  buttonFrameViewTab->setMaximumSize(24,24);
  buttonFrameViewTab->setIcon(QIcon(":/iraspa/framelistcontroller.png"));
  buttonFrameViewTab->setContentsMargins(0,0,0,0);
  buttonFrameViewTab->setCheckable(true);
  buttonFrameViewTab->setChecked(false);
  addWidget(buttonFrameViewTab);

  this->layout()->setSpacing(4);

  _mapper = new QSignalMapper(this);
  _mapper->setMapping(buttonToProjectTab, 0);
  _mapper->setMapping(buttonToMovieViewTab, 1);
  _mapper->setMapping(buttonFrameViewTab, 2);

  QObject::connect(buttonToProjectTab, &QToolButton::clicked,this,&MasterToolBar::reactToProjectToggled);
  QObject::connect(buttonToMovieViewTab, &QToolButton::clicked,this,&MasterToolBar::reactToMovieToggled);
  QObject::connect(buttonFrameViewTab, &QToolButton::clicked,this,&MasterToolBar::reactToFrameToggled);

  QObject::connect(buttonToProjectTab, &QToolButton::clicked, _mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
  QObject::connect(buttonToMovieViewTab, &QToolButton::clicked, _mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
  QObject::connect(buttonFrameViewTab, &QToolButton::clicked, _mapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
}

void MasterToolBar::reactToProjectToggled(bool checked)
{

    buttonToProjectTab->setDown(true);
    buttonToMovieViewTab->setDown(false);
    buttonFrameViewTab->setDown(false);
    buttonToProjectTab->setChecked(true);
    buttonToMovieViewTab->setChecked(false);
    buttonFrameViewTab->setChecked(false);

}

void MasterToolBar::reactToMovieToggled(bool checked)
{

    buttonToProjectTab->setDown(false);
    buttonToMovieViewTab->setDown(true);
    buttonFrameViewTab->setDown(false);
    buttonToProjectTab->setChecked(false);
    buttonToMovieViewTab->setChecked(true);
    buttonFrameViewTab->setChecked(false);

}

void MasterToolBar::reactToFrameToggled(bool checked)
{

    buttonToProjectTab->setDown(false);
    buttonToMovieViewTab->setDown(false);
    buttonFrameViewTab->setDown(true);
    buttonToProjectTab->setChecked(false);
    buttonToMovieViewTab->setChecked(false);
    buttonFrameViewTab->setChecked(true);

}

QSize MasterToolBar::sizeHint() const
{
  return QSize(200, 24);
}
