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

#include <QString>
#include <QUrl>
#include <string>
#include <vector>
//#include <string_view>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include "characterset.h"

class Scanner
{
public:
  Scanner(QUrl &url, CharacterSet charactersToBeSkipped);
  QString string() const {return _string;}
  QString::const_iterator scanLocation() const {return _scanLocation;}
  void setScanLocation(QString::const_iterator location) {_scanLocation = location;}
  bool scanCharacters(CharacterSet set, QString &into);
  bool scanUpToCharacters(CharacterSet set, QString &into);
  bool isAtEnd();
  bool scanDouble(double& value);
  bool scanInt(int& value);
  const QString displayName() {return _displayName;}
private:
  CharacterSet _charactersToBeSkipped;
  QString _displayName;
  QString _string;
  QString::const_iterator _scanLocation;

  QString::const_iterator find_first_not_of(const QString & chars, const QString & text, QString::const_iterator location);
  QString::const_iterator find_first_of(const QString & chars, const QString & text, QString::const_iterator location);
};
