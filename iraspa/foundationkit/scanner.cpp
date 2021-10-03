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

#include "scanner.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

Scanner::Scanner(QUrl &url, CharacterSet charactersToBeSkipped): _charactersToBeSkipped(charactersToBeSkipped)
{
  QFile file(url.toLocalFile());
  QFileInfo info(file);

  if (file.open(QIODevice::ReadOnly))
  {
    QTextStream in(&file);

    _displayName = info.baseName();

    _string = in.readAll();
  }
   _scanLocation = _string.constBegin();
}

QString::const_iterator Scanner::find_first_not_of(const QString & chars, const QString & text, QString::const_iterator location)
{
  QString::const_iterator it = location;
  QString::const_iterator end = text.constEnd();
  while(it != end)
  {
    if(chars.contains(*it) == false)
        return it;
    ++it;
  }

  return  text.constEnd();
}


QString::const_iterator Scanner::find_first_of(const QString & chars, const QString & text, QString::const_iterator location)
{
  QString::const_iterator it = location;
  QString::const_iterator end = text.constEnd();
  while(it != end)
  {
    if(chars.contains(*it))
        return it;
    ++it;
  }

  return  text.constEnd();
}

bool Scanner::scanCharacters(CharacterSet set, QString &into)
{
  QString::const_iterator found = find_first_not_of(set.string(), _string, _scanLocation);

  if (found>=_string.constEnd())
  {
    _scanLocation = _string.constEnd();
    into = QString("");
    return false;
  }

  if(found < _string.constEnd())
  {
    into = QString(_scanLocation, found - _scanLocation);
    _scanLocation = found;
    return true;
  }
  _scanLocation = _string.constEnd();
  into = QString("");
  return false;

}

bool Scanner::scanUpToCharacters(CharacterSet set, QString &into)
{
  QString::const_iterator found = find_first_not_of(_charactersToBeSkipped.string(), _string, _scanLocation);

  if (found>=_string.constEnd())
  {
    _scanLocation = _string.constEnd();
    into = QString("");
    return false;
  }

  _scanLocation = found;

  found = find_first_of(set.string(), _string, _scanLocation);

  if(found < _string.constEnd())
  {
    into = QString(_scanLocation, found - _scanLocation);
    _scanLocation = found;
    return true;
  }
  _scanLocation = _string.constEnd();
  into = QString("");
  return false;

}

bool Scanner::isAtEnd()
{
  return _scanLocation >= _string.constEnd();
}

bool Scanner::scanDouble(double &value)
{
  QString::const_iterator found = find_first_not_of(_charactersToBeSkipped.string(), _string, _scanLocation);

  if (found>=_string.constEnd())
  {
    _scanLocation = _string.constEnd();
    return false;
  }

  _scanLocation = found;

  found = find_first_of(_charactersToBeSkipped.string(), _string, _scanLocation);

  if(found < _string.constEnd())
  {
    QString into = QString(_scanLocation, found - _scanLocation);
    _scanLocation = found;
    bool success = false;
    value = into.toDouble(&success);
    return success;
  }
  _scanLocation = _string.constEnd();
  return false;
}

bool Scanner::scanInt(int &value)
{
    QString::const_iterator found = find_first_not_of(_charactersToBeSkipped.string(), _string, _scanLocation);

    if (found>=_string.constEnd())
    {
      _scanLocation = _string.constEnd();
      return false;
    }

    _scanLocation = found;

    found = find_first_of(_charactersToBeSkipped.string(), _string, _scanLocation);

    if(found < _string.constEnd())
    {
      QString into = QString(_scanLocation, found - _scanLocation);
      _scanLocation = found;
      bool success = false;
      value = into.toInt(&success);
      return success;
    }
    _scanLocation = _string.constEnd();
    return false;
}
