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

#include <QObject>
#include <QWidget>
#include <QSpinBox>
#include <QWheelEvent>

class CustomIntSpinBox: public QSpinBox
{
  Q_OBJECT

public:
  CustomIntSpinBox(QWidget* parent = nullptr);

  enum SpinBoxStateFlag { Int = 0x00, Text = 0x01};

  void setText(QString text);

  void setValue(int value);

  void focusInEvent(QFocusEvent *event) override final;
  void wheelEvent(QWheelEvent *event) override final;
  bool focusNextPrevChild(bool next) override final;

  QString textFromValue(int value) const override final;
  int valueFromText(const QString &text) const override final;
  QValidator::State validate(QString& input, int& pos) const override final;

  void stepBy(int steps) override final;
private:
  int _intValue;
  mutable QString _textValue;
  SpinBoxStateFlag _state = Int;
  void privateEditingFinished();
signals:
  void textOnEditingFinished(const QString &value);
};

