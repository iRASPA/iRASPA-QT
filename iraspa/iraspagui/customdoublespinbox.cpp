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

#include "customdoublespinbox.h"
#include <iostream>
#include <QLineEdit>
#include <cmath>
#include <iostream>

CustomDoubleSpinBox::CustomDoubleSpinBox(QWidget* parent) : QDoubleSpinBox(parent)
{
  QObject::connect(this, &CustomDoubleSpinBox::editingFinished, this, &CustomDoubleSpinBox::privateEditingFinished);
}


void CustomDoubleSpinBox::privateEditingFinished()
{
  if(!isReadOnly())
  {
    bool success = false;
    double value = _textValue.toDouble(&success);
    _state=Text;
    setPrefix(_textValue);

    if(success)
    {
      _state = Double;
      _doubleValue = value;
      setPrefix("");
      lineEdit()->deselect();
      clearFocus();
      emit textOnEditingFinished(text());
    }
  }
}

void CustomDoubleSpinBox::setValue(double value)
{
  setPrefix("");
  _state = Double;
  _doubleValue = value;
  QDoubleSpinBox::setValue(value);
}



void CustomDoubleSpinBox::setText(QString text)
{
  _textValue = text;
  _state = Text;
  setPrefix(text);
}

bool CustomDoubleSpinBox::focusNextPrevChild(bool next)
{
  Q_UNUSED(next);

  return false;
}

void CustomDoubleSpinBox::wheelEvent(QWheelEvent *event)
{
  if (!hasFocus())
  {
    event->ignore();
  }
  else
  {
    QDoubleSpinBox::wheelEvent(event);
  }
}

void CustomDoubleSpinBox::focusInEvent(QFocusEvent *event)
{
  if(_state == Text && !isReadOnly())
  {
    setPrefix("");
    _state = Double;
    _textValue = "";
    lineEdit()->setText("");
  }
  QDoubleSpinBox::focusInEvent(event);
}

QValidator::State CustomDoubleSpinBox::validate(QString& input, int& pos) const
{
  if(_state == Text)
  {
    return QValidator::Acceptable;
  }
  else
  {
    bool ok = false;
    locale().toDouble(input, &ok);
    return ok ? QValidator::Acceptable : QValidator::Invalid;
  }
}

QString CustomDoubleSpinBox::textFromValue(double value) const
{
  switch(_state)
  {
  case Double:
    return QDoubleSpinBox::textFromValue(value);
  case Text:
    return "";
  }
  return "";
}

double CustomDoubleSpinBox::valueFromText(const QString &text) const
{
  _textValue = text;
  return QDoubleSpinBox::valueFromText(text);
}

void CustomDoubleSpinBox::stepBy(int steps)
{
  QDoubleSpinBox::stepBy(steps);
  _textValue = text();
  lineEdit()->deselect();
}
