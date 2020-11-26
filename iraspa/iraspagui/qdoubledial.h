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

#pragma once

#include <QDial>
#include <QWheelEvent>

class QDoubleDial : public QDial
{
  Q_OBJECT
public:
  explicit QDoubleDial(QWidget *parent = nullptr);

  void wheelEvent(QWheelEvent *event) override final;

  double doubleMinimum()
  {
    return m_DoubleMin;
  }

  double doubleMaximum()
  {
    return m_DoubleMax;
  }

  double doubleSingleStep()
  {
    return m_DoubleStep;
  }

  void setDoubleMinimum(double value)
  {
    m_DoubleMin = value;
    updateRange();
  }

  void setDoubleMaximum(double value)
  {
    m_DoubleMax = value;
    updateRange();
  }

  void setDoubleSingleStep(double value)
  {
    m_DoubleStep = value;
    updateRange();
  }

  double doubleValue();

  void setDoubleValue(double x);

signals:
  void sliderMoved(double value);
  void valueChanged(double value);
public slots:

private:
  double m_DoubleMin;
  double m_DoubleMax;
  double m_DoubleStep;
  double m_DoubleValue;

  int m_CorrespondingIntValue;

  void updateRange();

  void convertIntToDouble(int value);

  void updateStep()
  {
    QDial::setSingleStep((int)(1000 * m_DoubleStep / (m_DoubleMax - m_DoubleMin)));
  }
};
