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

#include <string>
#include <vector>
#include <QString>
#include <QDataStream>
#include "skrotationaloccurancetable.h"
#include "skpointsymmetryset.h"
#include "sktransformationmatrix.h"
#include <mathkit.h>
#include "skspacegroupsetting.h"

enum class Laue: qint64
{
  none = 0, laue_1 = 1, laue_2m = 2, laue_mmm = 3, laue_4m = 4, laue_4mmm = 5, laue_3 = 6, laue_3m = 7, laue_6m = 8, laue_6mmm = 9, laue_m3 = 10, laue_m3m = 11
};


class SKPointGroup
{
public:

  SKPointGroup(SKRotationalOccuranceTable table, qint64 number, QString symbol, QString schoenflies, Holohedry holohedry, Laue laue, bool centrosymmetric, bool enantiomorphic);
  SKPointGroup(SKPointSymmetrySet set);
  static std::vector<SKPointGroup> pointGroupData;

  qint64 number() {return _number;}
  Holohedry holohedry() const {return _holohedry;}
  QString holohedryString() const;
  QString LaueString() const;
  QString symbol() {return _symbol;}
  QString schoenflies() {return _schoenflies;}
  bool centrosymmetric() {return _centrosymmetric;}
  bool enantiomorphic() {return _enantiomorphic;}

  Laue laue() const {return _laue;}
  int number() const {return _number;}
  Centring computeCentering(SKTransformationMatrix basis);

  static std::optional<SKPointGroup> findPointGroup(double3x3 unitCell, std::vector<std::tuple<double3, int, double> > atoms, bool allowPartialOccupancies, double symmetryPrecision);
  SKTransformationMatrix computeBasisCorrection(SKTransformationMatrix basis, Centring &centering);
  const std::optional<SKTransformationMatrix> constructAxes(std::vector<SKRotationMatrix> rotations) const;
  const SKRotationalOccuranceTable &table() const {return _table;}
private:
  SKRotationalOccuranceTable _table;
  qint64 _number = 0;
  QString _symbol = "";
  QString _schoenflies = "";
  Holohedry _holohedry = Holohedry::none;
  Laue _laue = Laue::none;
  bool _centrosymmetric = false;
  bool _enantiomorphic = false;

  static std::map<Laue, int> rotationTypeForBasis;

  friend QDataStream &operator<<(QDataStream &, const SKPointGroup &);
  friend QDataStream &operator>>(QDataStream &, SKPointGroup &);
};
