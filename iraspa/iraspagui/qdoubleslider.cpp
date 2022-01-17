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

#include "qdoubleslider.h"
#include <cmath>
#include <QtGlobal>

QDoubleSlider::QDoubleSlider(QWidget *parent) :
  QSlider(parent)
{
  m_DoubleMin = 0.0;
  m_DoubleMax = 1.0;
  m_DoubleStep = 0.01;
  updateRange();

  connect(this, &QSlider::sliderMoved, this, &QDoubleSlider::convertIntToDouble);
  connect(this, &QSlider::valueChanged, this, &QDoubleSlider::convertIntToDouble);


  #if defined (Q_OS_OSX)
    // Fixed in QT 6.2.2
    #if QT_VERSION < QT_VERSION_CHECK(6, 2, 2)
    // https://stackoverflow.com/questions/69890284/qslider-in-qt-misbehaves-in-new-macos-monterey-v12-0-1-any-workaround/69890285#69890285
    this->setStyleSheet("\
                           QSlider::groove:horizontal {\
                               height: 8px; /* the groove expands to the size of the slider by default. by giving it a height, it has a fixed size */ \
                               background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #B1B1B1, stop:1 #c4c4c4);\
                               margin: 2px 0;\
                           }\
                           \
                           QSlider::handle:horizontal {\
                               background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #b4b4b4, stop:1 #8f8f8f);\
                               border: 1px solid #5c5c5c;\
                               width: 18px;\
                               margin: -2px 0; /* handle is placed by default on the contents rect of the groove. Expand outside the groove */ \
                               border-radius: 3px;\
                           }\
                       ");
    #endif
  #endif
}

QDoubleSlider::QDoubleSlider(Qt::Orientation orientation, QWidget *parent) : QSlider(orientation, parent)
{
  m_DoubleMin = 0.0;
  m_DoubleMax = 1.0;
  m_DoubleStep = 0.01;
  updateRange();

  connect(this, &QSlider::sliderMoved, this, &QDoubleSlider::convertIntToDouble);
  connect(this, &QSlider::valueChanged, this, &QDoubleSlider::convertIntToDouble);
}

void QDoubleSlider::mouseReleaseEvent(QMouseEvent* event)
{
  QSlider::mouseReleaseEvent(event);
  emit sliderReleaseValue(m_DoubleValue);
}


void QDoubleSlider::convertIntToDouble(int value)
{
  double t = value * 1.0 / this->maximum();
  double dValue = m_DoubleMin + t * (m_DoubleMax - m_DoubleMin);
  emit sliderMoved(dValue);
}
void QDoubleSlider::updateRange()
{
  int mymax = ceil((m_DoubleMax - m_DoubleMin) / m_DoubleStep);
  this->setMinimum(0);
  this->setMaximum(mymax);
  this->setSingleStep(1);

  this->setDoubleValue(m_DoubleValue);
}

void QDoubleSlider::setDoubleValue(double x)
{
  m_DoubleValue = x;
  double t = (m_DoubleValue - m_DoubleMin) / (m_DoubleMax - m_DoubleMin);
  t = std::max(0.0, std::min(1.0, t));
  int p = (int)(0.5 + this->maximum() * t);
  if(this->value() != p)
    this->setValue(p);
  m_CorrespondingIntValue = p;
}

double QDoubleSlider::doubleValue()
{
  if(this->value() != m_CorrespondingIntValue)
  {
    double t = this->value() * 1.0 / this->maximum();
    m_DoubleValue = m_DoubleMin + t * (m_DoubleMax - m_DoubleMin);
    m_CorrespondingIntValue = this->value();
  }

  return m_DoubleValue;
}

void QDoubleSlider::wheelEvent(QWheelEvent *event)
{
  if (!hasFocus())
  {
    event->ignore();
  }
  else
  {
    QSlider::wheelEvent(event);
  }
}
