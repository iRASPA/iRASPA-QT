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
#include <QDataStream>
#include <foundationkit.h>
#include <map>
#include <mathkit.h>
#include <QColor>



class SKColorSet
{
public:
  SKColorSet() {}

  enum class ColorScheme: qint64
  {
    jmol = 0, rasmol_modern = 1, rasmol = 2, vesta = 3, multiple_values = 4
  };

  enum class ColorSchemeOrder: qint64
  {
    elementOnly = 0, forceFieldFirst = 1, forceFieldOnly = 2, multiple_values = 3
  };

  SKColorSet(QString name, SKColorSet& from, bool editable);
  SKColorSet(ColorScheme scheme);
  const QString displayName() const {return _displayName;}
  QColor& operator[] (const QString colorName) {return _colors[colorName];}
  const QColor* operator[] (QString colorName) const;
  bool editable() {return _editable;}
private:
  qint64 _versionNumber{1};

  QString _displayName;
  bool _editable = false;
  std::map<QString, QColor> _colors{};

  static std::map<QString, QColor> jMol;
  static std::map<QString, QColor> rasmol;
  static std::map<QString, QColor> rasmolModern;
  static std::map<QString, QColor> vesta;

  friend QDataStream &operator<<(QDataStream& stream, const std::map<QString, QColor>& table);
  friend QDataStream &operator>>(QDataStream& stream, std::map<QString, QColor>& table);

  friend QDataStream &operator<<(QDataStream &, const SKColorSet &);
  friend QDataStream &operator>>(QDataStream &, SKColorSet &);
};

