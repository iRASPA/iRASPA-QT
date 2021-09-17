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

#include "customintspinbox.h"
#include <climits>
#include <QApplication>
#include <iostream>
#include <QLineEdit>
#include <cmath>
#include <iostream>

CustomIntSpinBox::CustomIntSpinBox(QWidget* parent) : QSpinBox(parent)
{
  QObject::connect(this, &CustomIntSpinBox::editingFinished, this, &CustomIntSpinBox::privateEditingFinished);
}

void CustomIntSpinBox::privateEditingFinished()
{
  if(!isReadOnly())
  {
    bool success = false;
    int value = _textValue.toInt(&success);
    _state=Text;
    setPrefix(_textValue);

    if(success)
    {
      _state = Int;
      _intValue = value;
      setPrefix("");
      lineEdit()->deselect();
      clearFocus();
      emit textOnEditingFinished(text());
    }
  }
}

bool CustomIntSpinBox::focusNextPrevChild([[maybe_unused]] bool next)
{
  return false;
}

void CustomIntSpinBox::wheelEvent(QWheelEvent *event)
{
  if (!hasFocus())
  {
    event->ignore();
  }
  else
  {
    QSpinBox::wheelEvent(event);
  }
}

void CustomIntSpinBox::setText(QString text)
{
  _textValue = text;
  _state = Text;
  QSpinBox::setPrefix(text);
}

void CustomIntSpinBox::setValue(int value)
{
  setPrefix("");
  _state = Int;
  _intValue = value;
  QSpinBox::setValue(value);
}

void CustomIntSpinBox::focusInEvent(QFocusEvent *event)
{
  if(_state == Text && !isReadOnly())
  {
    setPrefix("");
    _state = Int;
    _textValue = "";
    lineEdit()->setText("");
  }
  QSpinBox::focusInEvent(event);
}

QValidator::State CustomIntSpinBox::validate(QString& input, int& pos) const
{
  if(_state == Text)
  {
    return QValidator::Acceptable;
  }
  else
  {
    return QSpinBox::validate(input,pos);
  }
}


QString CustomIntSpinBox::textFromValue(int value) const
{
   switch(_state)
   {
   case Int:
       return QSpinBox::textFromValue(value);
   case Text:
       return "";
   }
   return "";
}

int CustomIntSpinBox::valueFromText(const QString &text) const
{
  _textValue = text;
  return QSpinBox::valueFromText(text);
}

void CustomIntSpinBox::stepBy(int steps)
{
  QSpinBox::stepBy(steps);
  _textValue = text();
  lineEdit()->deselect();
}

