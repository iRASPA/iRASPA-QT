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

#include <ostream>
#include <string>
#include <vector>
#include <mathkit.h>
#include "skintegersymmetryoperationset.h"


class SKSpaceGroupSetting
{
public:  
  SKSpaceGroupSetting(qint64 number, qint64 spaceGroupNumber, qint64 order, char ext, QString qualifier, QString HM, QString oldHM, QString Hall,
                      bool inversionAtOrigin, int3 inversionCenter, Symmorphicity symmorphicity, bool standard, Centring centring,
                      std::vector<int3> latticeTranslations, qint64 pointGroupNumber, std::string schoenflies, std::string generators,
                      std::string encoding, int3x3 toPrimitiveTransformation);
  SKIntegerSymmetryOperationSet fullSeitzMatrices();

  qint64 number() const {return _spaceGroupNumber;}
  qint64 HallNumber() const {return _HallNumber;}
  QString HallString() const {return _HallString;}
  QString HMString() const {return _HMString;}
  qint64 pointGroupNumber() const {return _pointGroupNumber;}
  QString qualifier() const {return _qualifier;}
  Symmorphicity symmorphicity() const {return _symmorphicity;}
  QString symmorphicityString() const;
  QString centringString() const;

  bool inversionAtOrigin() const {return _inversionAtOrigin;}
  int3 inversionCenter() const {return _inversionCenter;}

  std::vector<int3> latticeTranslations() {return _latticeTranslations;}

  friend std::ostream& operator<<(std::ostream& os, const SKSpaceGroupSetting& setting);
private:
  qint64 _HallNumber = 1;
  qint64 _spaceGroupNumber = 1;      // space group number (1-230)
  qint64 _order;
  char _ext;                         // '1', '2', 'H', 'R' or '\0'
  QString _qualifier;                // e.g. "-cba" or "b1"
  QString _HMString;                 // H-M symbol; nul-terminated string
  QString _oldHMString;              // H-M symbol; nul-terminated string
  QString _HallString;               // Hall symbol; nul-terminated string
  std::string _encodedGenerators;    // encoded seitz matrix-generators
  std::string _encodedSeitz;         // encoded seitz matrix
  bool _inversionAtOrigin;
  int3 _inversionCenter;
  bool _standard = false;
  Symmorphicity _symmorphicity = Symmorphicity::asymmorphic;
  Centring _centring = Centring::primitive;
  std::vector<int3> _latticeTranslations;
  std::string _schoenflies;
  qint64 _pointGroupNumber;
  int3x3 _toPrimitiveTransformation;
};
