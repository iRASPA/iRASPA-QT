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

#include "toolbarwidget.h"
#include <QDebug>
#include <QKeyEvent>
#include <QToolBar>
#include <QGridLayout>
#include <QToolButton>

ToolbarWidget::ToolbarWidget(RenderStackedWidget *parent): QStackedWidget(parent)
{
  this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QFrame *_controlPanelCartesian(new QFrame(this));
  _controlPanelCartesian->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _controlPanelCartesian->installEventFilter(this);
  _controlPanelCartesian->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

  QFrame *_controlPanelBodyFrame(new QFrame(this));
  _controlPanelBodyFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _controlPanelBodyFrame->installEventFilter(this);
  _controlPanelBodyFrame->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

  QGridLayout *layoutCartesian = new QGridLayout(_controlPanelCartesian);
  layoutCartesian->setSpacing(2);

  QToolButton *_buttonCartesianMinusTx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusTx->setAutoRepeat(true);
  _buttonCartesianMinusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusTx->setText("-Tx");
  layoutCartesian->addWidget(_buttonCartesianMinusTx, 1, 0);
  QObject::connect(_buttonCartesianMinusTx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateCartesianMinusX);

  QToolButton *_buttonCartesianPlusTx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusTx->setAutoRepeat(true);
  _buttonCartesianPlusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusTx->setText("+Tx");
  layoutCartesian->addWidget(_buttonCartesianPlusTx, 1, 1);
  QObject::connect(_buttonCartesianPlusTx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateCartesianPlusX);

  QToolButton *_buttonCartesianMinusTy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusTy->setAutoRepeat(true);
  _buttonCartesianMinusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusTy->setText("-Ty");
  layoutCartesian->addWidget(_buttonCartesianMinusTy, 2, 0);
  QObject::connect(_buttonCartesianMinusTy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateCartesianMinusY);

  QToolButton *_buttonCartesianPlusTy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusTy->setAutoRepeat(true);
  _buttonCartesianPlusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusTy->setText("+Ty");
  layoutCartesian->addWidget(_buttonCartesianPlusTy, 2, 1);
  QObject::connect(_buttonCartesianPlusTy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateCartesianPlusY);

  QToolButton *_buttonCartesianMinusTz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusTz->setAutoRepeat(true);
  _buttonCartesianMinusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusTz->setText("-Tz");
  layoutCartesian->addWidget(_buttonCartesianMinusTz, 3, 0);
  QObject::connect(_buttonCartesianMinusTz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateCartesianMinusZ);

  QToolButton *_buttonCartesianPlusTz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusTz->setAutoRepeat(true);
  _buttonCartesianPlusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusTz->setText("+Tz");
  layoutCartesian->addWidget(_buttonCartesianPlusTz, 3, 1);
  QObject::connect(_buttonCartesianPlusTz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateCartesianPlusZ);


  QToolButton *_buttonCartesianMinusRx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusRx->setAutoRepeat(true);
  _buttonCartesianMinusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusRx->setText("-Rx");
  layoutCartesian->addWidget(_buttonCartesianMinusRx, 4, 0);
  QObject::connect(_buttonCartesianMinusRx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateCartesianMinusX);

  QToolButton *_buttonCartesianPlusRx = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusRx->setAutoRepeat(true);
  _buttonCartesianPlusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusRx->setText("+Rx");
  layoutCartesian->addWidget(_buttonCartesianPlusRx, 4, 1);
  QObject::connect(_buttonCartesianPlusRx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateCartesianPlusX);

  QToolButton *_buttonCartesianMinusRy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusRy->setAutoRepeat(true);
  _buttonCartesianMinusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusRy->setText("-Ry");
  layoutCartesian->addWidget(_buttonCartesianMinusRy, 5, 0);
  QObject::connect(_buttonCartesianMinusRy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateCartesianMinusY);

  QToolButton *_buttonCartesianPlusRy = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusRy->setAutoRepeat(true);
  _buttonCartesianPlusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusRy->setText("+Ry");
  layoutCartesian->addWidget(_buttonCartesianPlusRy, 5, 1);
  QObject::connect(_buttonCartesianPlusRy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateCartesianPlusY);

  QToolButton *_buttonCartesianMinusRz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianMinusRz->setAutoRepeat(true);
  _buttonCartesianMinusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianMinusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianMinusRz->setText("-Rz");
  layoutCartesian->addWidget(_buttonCartesianMinusRz, 6, 0);
  QObject::connect(_buttonCartesianMinusRz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateCartesianMinusZ);

  QToolButton *_buttonCartesianPlusRz = new QToolButton(_controlPanelCartesian);
  _buttonCartesianPlusRz->setAutoRepeat(true);
  _buttonCartesianPlusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonCartesianPlusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonCartesianPlusRz->setText("+Rz");
  layoutCartesian->addWidget(_buttonCartesianPlusRz, 6, 1);
  QObject::connect(_buttonCartesianPlusRz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateCartesianPlusZ);

  this->addWidget(_controlPanelCartesian);

  QGridLayout *layoutBodyFrame = new QGridLayout(_controlPanelBodyFrame);
  layoutBodyFrame->setSpacing(2);

  QToolButton *_buttonBodyFrameMinusTx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusTx->setAutoRepeat(true);
  _buttonBodyFrameMinusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusTx->setText("-T1");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusTx, 1, 0);
  QObject::connect(_buttonBodyFrameMinusTx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateBodyFrameMinusX);

  QToolButton *_buttonBodyFramePlusTx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusTx->setAutoRepeat(true);
  _buttonBodyFramePlusTx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusTx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusTx->setText("+T1");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusTx, 1, 1);
  QObject::connect(_buttonBodyFramePlusTx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateBodyFramePlusX);

  QToolButton *_buttonBodyFrameMinusTy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusTy->setAutoRepeat(true);
  _buttonBodyFrameMinusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusTy->setText("-T2");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusTy, 2, 0);
  QObject::connect(_buttonBodyFrameMinusTy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateBodyFrameMinusY);

  QToolButton *_buttonBodyFramePlusTy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusTy->setAutoRepeat(true);
  _buttonBodyFramePlusTy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusTy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusTy->setText("+T2");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusTy, 2, 1);
  QObject::connect(_buttonBodyFramePlusTy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateBodyFramePlusY);

  QToolButton *_buttonBodyFrameMinusTz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusTz->setAutoRepeat(true);
  _buttonBodyFrameMinusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusTz->setText("-T3");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusTz, 3, 0);
  QObject::connect(_buttonBodyFrameMinusTz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateBodyFrameMinusZ);

  QToolButton *_buttonBodyFramePlusTz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusTz->setAutoRepeat(true);
  _buttonBodyFramePlusTz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusTz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusTz->setText("+T3");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusTz, 3, 1);
  QObject::connect(_buttonBodyFramePlusTz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedTranslateBodyFramePlusZ);

  QToolButton *_buttonBodyFrameMinusRx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusRx->setAutoRepeat(true);
  _buttonBodyFrameMinusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusRx->setText("-R1");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusRx, 4, 0);
  QObject::connect(_buttonBodyFrameMinusRx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateBodyFrameMinusX);

  QToolButton *_buttonBodyFramePlusRx = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusRx->setAutoRepeat(true);
  _buttonBodyFramePlusRx->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusRx->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusRx->setText("+R1");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusRx, 4, 1);
  QObject::connect(_buttonBodyFramePlusRx, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateBodyFramePlusX);

  QToolButton *_buttonBodyFrameMinusRy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusRy->setAutoRepeat(true);
  _buttonBodyFrameMinusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusRy->setText("-R2");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusRy, 5, 0);
  QObject::connect(_buttonBodyFrameMinusRy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateBodyFrameMinusY);

  QToolButton *_buttonBodyFramePlusRy = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusRy->setAutoRepeat(true);
  _buttonBodyFramePlusRy->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusRy->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusRy->setText("+R2");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusRy, 5, 1);
  QObject::connect(_buttonBodyFramePlusRy, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateBodyFramePlusY);

  QToolButton *_buttonBodyFrameMinusRz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFrameMinusRz->setAutoRepeat(true);
  _buttonBodyFrameMinusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFrameMinusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFrameMinusRz->setText("-R3");
  layoutBodyFrame->addWidget(_buttonBodyFrameMinusRz, 6, 0);
  QObject::connect(_buttonBodyFrameMinusRz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateBodyFrameMinusZ);

  QToolButton *_buttonBodyFramePlusRz = new QToolButton(_controlPanelBodyFrame);
  _buttonBodyFramePlusRz->setAutoRepeat(true);
  _buttonBodyFramePlusRz->setStyleSheet("height: 24px; width: 24px; font: bold; color: black; background-color: rgba(220, 220, 220, 128); border: 1px solid grey; font-size: 14px; ");
  _buttonBodyFramePlusRz->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  _buttonBodyFramePlusRz->setText("+R3");
  layoutBodyFrame->addWidget(_buttonBodyFramePlusRz, 6, 1);
  QObject::connect(_buttonBodyFramePlusRz, &QToolButton::clicked,parent,&RenderStackedWidget::pressedRotateBodyFramePlusZ);

  this->addWidget(_controlPanelBodyFrame);
}

bool ToolbarWidget::eventFilter(QObject *object, QEvent *event)
{
  Q_UNUSED(object);

  if(QKeyEvent * keyEvent = dynamic_cast<QKeyEvent*>(event))
  {
    if (event->type() == QEvent::KeyPress)
    {
      if (keyEvent->modifiers().testFlag(Qt::AltModifier))
      {
        this->setCurrentIndex(1);
        event->ignore();
      }

    }
    if (event->type() == QEvent::KeyRelease)
    {
      if (keyEvent->key() == Qt::Key_Alt)
      {
        this->setCurrentIndex(0);
        event->ignore();
      }
    }
  }
  return QStackedWidget::eventFilter(object, event);
}
