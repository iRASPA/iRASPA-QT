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

#include "skspacegroup.h"
#include "sksymmetrycell.h"
#include "skseitzmatrix.h"
#include <iostream>
#include <QDebug>

SKSpaceGroup::SKSpaceGroup(int HallNumber)
{
  _spaceGroupSetting = SKSpaceGroup::_spaceGroupData[HallNumber];
}

bool SKSpaceGroup::matchSpacegroup(QString spaceSearchGroupString, QString storedSpaceGroupString)
{
  if(storedSpaceGroupString == spaceSearchGroupString)
  {
    return true;
  }

  if("'" + storedSpaceGroupString + "'" == spaceSearchGroupString)
  {
    return true;
  }

  if("\"" + storedSpaceGroupString + "\"" == spaceSearchGroupString)
  {
    return true;
  }
  return false;
}

std::optional<int> SKSpaceGroup::HallNumber(QString string)
{
  QString spaceSearchGroupString = string.simplified().toLower();

  for(int i=0;i<=530;i++)
  {
    QString storedSpaceGroupString = SKSpaceGroup::_spaceGroupData[i].HallString().simplified().toLower();

    if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
    {
      return i;
    }

    storedSpaceGroupString = SKSpaceGroup::_spaceGroupData[i].HallString().simplified().toLower().remove(' ');
    if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
    {
      return i;
    }
  }

  return std::nullopt;
}

std::optional<int> SKSpaceGroup::HallNumberFromHMString(QString string)
{
  QString spaceSearchGroupString = string.simplified().toLower();

  for(int i=0;i<=530;i++)
  {
    QString storedSpaceGroupString = SKSpaceGroup::_spaceGroupData[i].HMString().simplified().toLower();

    if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
    {
      return i;
    }

    if(SKSpaceGroup::_spaceGroupData[i].qualifier() == "H")
    {
      storedSpaceGroupString = storedSpaceGroupString.replace(0,1,"h");
      if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
      {
        return i;
      }
    }

    storedSpaceGroupString = SKSpaceGroup::_spaceGroupData[i].HMString().simplified().toLower().remove(' ');
    if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
    {
      return i;
    }
  }

  return std::nullopt;
}

std::optional<int> SKSpaceGroup::HallNumberFromSpaceGroupNumber([[maybe_unused]] int number)
{
  if(number > 0 && number <= 230)
  {
    std::vector<int> hall_numbers = SKSpaceGroup::spaceGroupHallData[number];
    if(hall_numbers.size()> 0)
    {
      return hall_numbers.front();
    }
  }
  return std::nullopt;
}


std::vector<double3> SKSpaceGroup::listOfSymmetricPositions(double3 pos)
{
  std::unordered_set<SKSeitzIntegerMatrix> seitzMatrices = _spaceGroupSetting.fullSeitzMatrices().operations();
  size_t m = seitzMatrices.size();

  std::vector<double3> positions = std::vector<double3>{};
  positions.reserve(m);

  for (const auto& elem: seitzMatrices)
  {
     positions.push_back(elem * pos);
  }
  return positions;
}

std::vector<QString> SKSpaceGroup::latticeTranslationStrings(int HallNumber)
{
  std::vector<QString> latticeStrings{"","","",""};

  SKSpaceGroupSetting setting = SKSpaceGroup::_spaceGroupData[HallNumber];
  std::vector<int3> latticeVectors = setting.latticeTranslations();

  int index=0;
  for(int3 latticeVector: latticeVectors)
  {
    int3 gcd = greatestCommonDivisor(latticeVector, 12);
    QString latticeStringX = latticeVector.x == 0 ? "0" : QString::number(latticeVector.x/gcd.x) + "/" + QString::number(12/gcd.x);
    QString latticeStringY = latticeVector.y == 0 ? "0" : QString::number(latticeVector.y/gcd.y) + "/" + QString::number(12/gcd.y);
    QString latticeStringZ = latticeVector.z == 0 ? "0" : QString::number(latticeVector.z/gcd.z) + "/" + QString::number(12/gcd.z);
    latticeStrings[index] = "(" + latticeStringX + "," + latticeStringY + "," + latticeStringZ + ")";
    index++;
  }

  return latticeStrings;
}

QString SKSpaceGroup::inversionCenterString(int HallNumber)
{
  SKSpaceGroupSetting setting = SKSpaceGroup::_spaceGroupData[HallNumber];
  int3 inversionCenter = setting.inversionCenter();
  int3 gcd = greatestCommonDivisor(inversionCenter, 12);
  QString latticeStringX = inversionCenter.x == 0 ? "0" : QString::number(inversionCenter.x/gcd.x) + "/" + QString::number(12/gcd.x);
  QString latticeStringY = inversionCenter.y == 0 ? "0" : QString::number(inversionCenter.y/gcd.y) + "/" + QString::number(12/gcd.y);
  QString latticeStringZ = inversionCenter.z == 0 ? "0" : QString::number(inversionCenter.z/gcd.z) + "/" + QString::number(12/gcd.z);
  return "(" + latticeStringX + "," + latticeStringY + "," + latticeStringZ + ")";
}

SKSymmetryOperationSet SKSpaceGroup::findSpaceGroupSymmetry(double3x3 unitCell, std::vector<std::tuple<double3, int, double>> reducedAtoms, std::vector<std::tuple<double3, int, double>> atoms, SKPointSymmetrySet latticeSymmetries, bool allowPartialOccupancies, double symmetryPrecision= 1e-2)
{
  std::vector<SKSeitzMatrix> spaceGroupSymmetries{};

  for(const SKRotationMatrix &rotationMatrix: latticeSymmetries.rotations())
  {
    std::vector<double3> translations = SKSymmetryCell::primitiveTranslationVectors(unitCell, reducedAtoms, atoms, rotationMatrix, allowPartialOccupancies, symmetryPrecision);

    for(const double3& translation: translations)
    {
      SKSeitzMatrix matrix = SKSeitzMatrix(rotationMatrix, translation);

      // avoid duplicate Seitz-matrices
      if(!(std::find(spaceGroupSymmetries.begin(),spaceGroupSymmetries.end(), matrix) != spaceGroupSymmetries.end()))
      {
        spaceGroupSymmetries.push_back(matrix);
      }
    }
  }
  return SKSymmetryOperationSet(spaceGroupSymmetries);
}

QDataStream &operator<<(QDataStream &stream, const SKSpaceGroup &spaceGroup)
{
  stream << spaceGroup._spaceGroupSetting.HallNumber();
  return stream;
}

QDataStream &operator>>(QDataStream &stream, SKSpaceGroup &spaceGroup)
{
  qint64 HallNumber;
  stream >> HallNumber;
  spaceGroup._spaceGroupSetting = spaceGroup._spaceGroupData[HallNumber];
  return stream;
}


const std::array<SKSpaceGroupSetting,531> SKSpaceGroup::_spaceGroupData = std::array<SKSpaceGroupSetting,531>{
    SKSpaceGroupSetting(  0,  0, 0,0, "abc",            "", "", "unknown",false, int3(0,0,0), Symmorphicity::symmorphic, false, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 0, "", "012", "", int3x3(1)),
    
    // TRICLINIC GROUPS
    // ================
    
    SKSpaceGroupSetting(  1,  1, 1,0, "abc",         "P 1", "", "P 1",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  1,""      , "","012", int3x3(1)),
    
    SKSpaceGroupSetting(  2,  2, 2,0, "abc",        "P -1", "", "-P 1",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  2,"C1^1"  , "012345","012", int3x3(1)),
    
    
    // MONOCLINIC GROUPS
    // =================
    
    SKSpaceGroupSetting(  3,  3, 2,0,   "b",     "P 1 2 1", "", "P 2y", false, int3(0,0,0), Symmorphicity::symmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)},  3,"Ci^1"  , "315","012315", int3x3(1)),
    SKSpaceGroupSetting(  4,  3, 2,0,   "c",     "P 1 1 2", "", "P 2", false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  3,"C2^1"  , "342","012342", int3x3(1)),
    SKSpaceGroupSetting(  5,  3, 2,0,   "a",     "P 2 1 1", "", "P 2x", false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  3,"C2^1"  , "045","012045", int3x3(1)),
    
    SKSpaceGroupSetting(  6,  4, 2,0,   "b",    "P 1 21 1", "", "P 2yb", false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  3,"C2^2"  , "3=5","0123=5", int3x3(1)),
    SKSpaceGroupSetting(  7,  4, 2,0,   "c",    "P 1 1 21", "", "P 2c", false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  3,"C2^2"  , "34B","01234B", int3x3(1)),
    SKSpaceGroupSetting(  8,  4, 2,0,   "a",    "P 21 1 1", "", "P 2xa", false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  3,"C2^2"  , "845","012845", int3x3(1)),
    
    SKSpaceGroupSetting(  9,  5, 4,0,   "b1",    "C 1 2 1", "", "C 2y",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  3,"C2^3"  , "315","012315", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 10,  5, 4,0,   "b2",    "A 1 2 1", "", "A 2y",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  3,"C2^3"  , "315","012315", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 11,  5, 4,0,   "b3",    "I 1 2 1", "", "I 2y",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  3,"C2^3"  , "315","012315", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 12,  5, 4,0,   "c1",    "A 1 1 2", "", "A 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  3,"C2^3"  , "342","012342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 13,  5, 4,0,   "c2",    "B 1 1 2", "", "B 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  3,"C2^3"  , "342","012342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 14,  5, 4,0,   "c3",    "I 1 1 2", "", "I 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  3,"C2^3"  , "342","012342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 15,  5, 4,0,   "a1",    "B 2 1 1", "", "B 2x",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  3,"C2^3"  , "045","012045", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 16,  5, 4,0,   "a2",    "C 2 1 1", "", "C 2x",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  3,"C2^3"  , "045","012045", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 17,  5, 4,0,   "a3",    "I 2 1 1", "", "I 2x",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  3,"C2^3"  , "045","012045", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting( 18,  6, 2,0,    "b",    "P 1 m 1", "", "P -2y",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^1"  , "042","012042", int3x3(1)),
    SKSpaceGroupSetting( 19,  6, 2,0,    "c",    "P 1 1 m", "", "P -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^1"  , "015","012015", int3x3(1)),
    SKSpaceGroupSetting( 20,  6, 2,0,    "a",    "P m 1 1", "", "P -2x",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^1"  , "312","012312", int3x3(1)),
    
    SKSpaceGroupSetting( 21,  7, 2,0,   "b1",    "P 1 c 1", "", "P -2yc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "04B","01204B", int3x3(1)),
    SKSpaceGroupSetting( 22,  7, 2,0,   "b2",    "P 1 n 1", "", "P -2yac",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "84B","01284B", int3x3(1)),
    SKSpaceGroupSetting( 23,  7, 2,0,   "b3",    "P 1 a 1", "", "P -2ya",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "842","012842", int3x3(1)),
    SKSpaceGroupSetting( 24,  7, 2,0,   "c1",    "P 1 1 a", "", "P -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "815","012815", int3x3(1)),
    SKSpaceGroupSetting( 25,  7, 2,0,   "c2",    "P 1 1 n", "", "P -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "8=5","0128=5", int3x3(1)),
    SKSpaceGroupSetting( 26,  7, 2,0,   "c3",    "P 1 1 b", "", "P -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "0=5","0120=5", int3x3(1)),
    SKSpaceGroupSetting( 27,  7, 2,0,   "a1",    "P b 1 1", "", "P -2xb",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "3=2","0123=2", int3x3(1)),
    SKSpaceGroupSetting( 28,  7, 2,0,   "a2",    "P n 1 1", "", "P -2xbc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "3=B","0123=B", int3x3(1)),
    SKSpaceGroupSetting( 29,  7, 2,0,   "a3",    "P c 1 1", "", "P -2xc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  4,"Cs^2"  , "31B","01231B", int3x3(1)),
    
    SKSpaceGroupSetting( 30,  8, 4,0,   "b1",    "C 1 m 1", "", "C -2y",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  4,"Cs^3"  , "042","012042", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 31,  8, 4,0,   "b2",    "A 1 m 1", "", "A -2y",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  4,"Cs^3"  , "042","012042", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 32,  8, 4,0,   "b3",    "I 1 m 1", "", "I -2y",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^3"  , "042","012042", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 33,  8, 4,0,   "c1",    "A 1 1 m", "", "A -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  4,"Cs^3"  , "015","012015", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 34,  8, 4,0,   "c2",    "B 1 1 m", "", "B -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  4,"Cs^3"  , "015","012015", int3x3(int3(-1, 0, 1), int3(0, -1, 1), int3(1, 0, 1))),
    SKSpaceGroupSetting( 35,  8, 4,0,   "c3",    "I 1 1 m", "", "I -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^3"  , "015","012015", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 36,  8, 4,0,   "a1",    "B m 1 1", "", "B -2x",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  4,"Cs^3"  , "312","012312", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 37,  8, 4,0,   "a2",    "C m 1 1", "", "C -2x",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  4,"Cs^3"  , "312","012312", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 38,  8, 4,0,   "a3",    "I m 1 1", "", "I -2x",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^3"  , "312","012312", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting( 39,  9, 4,0,   "b1",    "C 1 c 1", "", "C -2yc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  4,"Cs^4"  , "04B","01204B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 40,  9, 4,0,   "b2",    "A 1 n 1", "", "A -2yab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  4,"Cs^4"  , "8N2","0128N2", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 41,  9, 4,0,   "b3",    "I 1 a 1", "", "I -2ya",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^4"  , "842","012842", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 42,  9, 4,0,  "-b1",    "A 1 a 1", "", "A -2ya",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  4,"Cs^4"  , "842","012842", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 43,  9, 4,0,  "-b2",    "C 1 n 1", "", "C -2yac",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  4,"Cs^4"  , "84B","01284B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 44,  9, 4,0,  "-b3",    "I 1 c 1", "", "I -2yc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^4"  , "04B","01204B", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 45,  9, 4,0,   "c1",    "A 1 1 a", "", "A -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  4,"Cs^4"  , "815","012815", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 46,  9, 4,0,   "c2",    "B 1 1 n", "", "B -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  4,"Cs^4"  , "8=5","0128=5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 47,  9, 4,0,   "c3",    "I 1 1 b", "", "I -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^4"  , "0=5","0120=5", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 48,  9, 4,0,  "-c1",    "B 1 1 b", "", "B -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  4,"Cs^4"  , "0=5","0120=5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 49,  9, 4,0,  "-c2",    "A 1 1 n", "", "A -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  4,"Cs^4"  , "8=5","0128=5", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 50,  9, 4,0,  "-c3",    "I 1 1 a", "", "I -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^4"  , "815","012815", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 51,  9, 4,0,   "a1",    "B b 1 1", "", "B -2xb",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  4,"Cs^4"  , "3=2","0123=2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 52,  9, 4,0,   "a2",    "C n 1 1", "", "C -2xac",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  4,"Cs^4"  , "I1B","012I1B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 53,  9, 4,0,   "a3",    "I c 1 1", "", "I -2xc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^4"  , "31B","01231B", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 54,  9, 4,0,  "-a1",    "C c 1 1", "", "C -2xc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  4,"Cs^4"  , "31B","01231B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 55,  9, 4,0,  "-a2",    "B n 1 1", "", "B -2xab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  4,"Cs^4"  , "I=2","012I=2", int3x3(int3(-1, 0, 1), int3(0, -1, 1), int3(1, 0, 1))),
    SKSpaceGroupSetting( 56,  9, 4,0,  "-a3",    "I b 1 1", "", "I -2xb",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  4,"Cs^4"  , "3=2","0123=2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting( 57, 10, 4,0,    "b",  "P 1 2/m 1", "", "-P 2y",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^1" , "315345","012315", int3x3(1)),
    SKSpaceGroupSetting( 58, 10, 4,0,    "c",  "P 1 1 2/m", "", "-P 2",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^1" , "342345","012342", int3x3(1)),
    SKSpaceGroupSetting( 59, 10, 4,0,    "a",  "P 2/m 1 1", "", "-P 2x",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^1" , "045345","012045", int3x3(1)),
    
    SKSpaceGroupSetting( 60, 11, 4,0,    "b", "P 1 21/m 1", "", "-P 2yb",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^2" , "3=5345","0123=5", int3x3(1)),
    SKSpaceGroupSetting( 61, 11, 4,0,    "c", "P 1 1 21/m", "", "-P 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^2" , "34B345","01234B", int3x3(1)),
    SKSpaceGroupSetting( 62, 11, 4,0,    "a", "P 21/m 1 1", "", "-P 2xa",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^2" , "845345","012845", int3x3(1)),
    
    SKSpaceGroupSetting( 63, 12, 8,0,   "b1",  "C 1 2/m 1", "", "-C 2y",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  5,"C2h^3" , "315345","012315", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 64, 12, 8,0,   "b2",  "A 1 2/m 1", "", "-A 2y",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  5,"C2h^3" , "315345","012315", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 65, 12, 8,0,   "b3",  "I 1 2/m 1", "", "-I 2y",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^3" , "315345","012315", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 66, 12, 8,0,   "c1",  "A 1 1 2/m", "", "-A 2",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  5,"C2h^3" , "342345","012342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 67, 12, 8,0,   "c2",  "B 1 1 2/m", "", "-B 2",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  5,"C2h^3" , "342345","012342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 68, 12, 8,0,   "c3",  "I 1 1 2/m", "", "-I 2",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^3" , "342345","012342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 69, 12, 8,0,   "a1",  "B 2/m 1 1", "", "-B 2x",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  5,"C2h^3" , "045345","012045", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 70, 12, 8,0,   "a2",  "C 2/m 1 1", "", "-C 2x",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  5,"C2h^3" , "045345","012045", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 71, 12, 8,0,   "a3",  "I 2/m 1 1", "", "-I 2x",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^3" , "045345","012045", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting( 72, 13, 4,0,   "b1",  "P 1 2/c 1", "", "-P 2yc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "31S345","01231S", int3x3(1)),
    SKSpaceGroupSetting( 73, 13, 4,0,   "b2",  "P 1 2/n 1", "", "-P 2yac",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "I1S345","012I1S", int3x3(1)),
    SKSpaceGroupSetting( 74, 13, 4,0,   "b3",  "P 1 2/a 1", "", "-P 2ya",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "I15345","012I15", int3x3(1)),
    SKSpaceGroupSetting( 75, 13, 4,0,   "c1",  "P 1 1 2/a", "", "-P 2a",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "I42345","012I42", int3x3(1)),
    SKSpaceGroupSetting( 76, 13, 4,0,   "c2",  "P 1 1 2/n", "", "-P 2ab",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "IN2345","012IN2", int3x3(1)),
    SKSpaceGroupSetting( 77, 13, 4,0,   "c3",  "P 1 1 2/b", "", "-P 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "3N2345","0123N2", int3x3(1)),
    SKSpaceGroupSetting( 78, 13, 4,0,   "a1",  "P 2/b 1 1", "", "-P 2xb",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "0N5345","0120N5", int3x3(1)),
    SKSpaceGroupSetting( 79, 13, 4,0,   "a2",  "P 2/n 1 1", "", "-P 2xbc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "0NS345","0120NS", int3x3(1)),
    SKSpaceGroupSetting( 80, 13, 4,0,   "a3",  "P 2/c 1 1", "", "-P 2xc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^4" , "04S345","01204S", int3x3(1)),
    
    SKSpaceGroupSetting( 81, 14, 4,0,   "b1", "P 1 21/c 1", "", "-P 2ybc",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "3=S345","0123=S", int3x3(1)),
    SKSpaceGroupSetting( 82, 14, 4,0,   "b2", "P 1 21/n 1", "", "-P 2yn",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "I=S345","012I=S", int3x3(1)),
    SKSpaceGroupSetting( 83, 14, 4,0,   "b3", "P 1 21/a 1", "", "-P 2yab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "I=5345","012I=5", int3x3(1)),
    SKSpaceGroupSetting( 84, 14, 4,0,   "c1", "P 1 1 21/a", "", "-P 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "I4B345","012I4B", int3x3(1)),
    SKSpaceGroupSetting( 85, 14, 4,0,   "c2", "P 1 1 21/n", "", "-P 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "INB345","012INB", int3x3(1)),
    SKSpaceGroupSetting( 86, 14, 4,0,   "c3", "P 1 1 21/b", "", "-P 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "3NB345","0123NB", int3x3(1)),
    SKSpaceGroupSetting( 87, 14, 4,0,   "a1", "P 21/b 1 1", "", "-P 2xab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "8N5345","0128N5", int3x3(1)),
    SKSpaceGroupSetting( 88, 14, 4,0,   "a2", "P 21/n 1 1", "", "-P 2xn",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "8NS345","0128NS", int3x3(1)),
    SKSpaceGroupSetting( 89, 14, 4,0,   "a3", "P 21/c 1 1", "", "-P 2xac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  5,"C2h^5" , "84S345","01284S", int3x3(1)),
    
    SKSpaceGroupSetting( 90, 15, 8,0,   "b1",  "C 1 2/c 1", "", "-C 2yc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  5,"C2h^6" , "31S345","01231S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 91, 15, 8,0,   "b2",  "A 1 2/n 1", "", "-A 2yab",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  5,"C2h^6" , "I=5345","012I=5", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 92, 15, 8,0,   "b3",  "I 1 2/a 1", "", "-I 2ya",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^6" , "I15345","012I15", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 93, 15, 8,0,  "-b1",  "A 1 2/a 1", "", "-A 2ya",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  5,"C2h^6" , "I15345","012I15", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 94, 15, 8,0,  "-b2",  "C 1 2/n 1", "", "-C 2yac",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  5,"C2h^6" , "I1S345","012I1S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting( 95, 15, 8,0,  "-b3",  "I 1 2/c 1", "", "-I 2yc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^6" , "31S345","01231S", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 96, 15, 8,0,   "c1",  "A 1 1 2/a", "", "-A 2a",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  5,"C2h^6" , "I42345","012I42", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting( 97, 15, 8,0,   "c2",  "B 1 1 2/n", "", "-B 2ab",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  5,"C2h^6" , "IN2345","012IN2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting( 98, 15, 8,0,   "c3",  "I 1 1 2/b", "", "-I 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^6" , "3N2345","0123N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting( 99, 15, 8,0,  "-c1",  "B 1 1 2/b", "", "-B 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  5,"C2h^6" , "3N2345","0123N2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(100, 15, 8,0,  "-c2",  "A 1 1 2/n", "", "-A 2ab",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  5,"C2h^6" , "IN2345","012IN2", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(101, 15, 8,0,  "-c3",  "I 1 1 2/a", "", "-I 2a",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^6" , "I42345","012I42", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(102, 15, 8,0,   "a1",  "B 2/b 1 1", "", "-B 2xb",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  5,"C2h^6" , "0N5345","0120N5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(103, 15, 8,0,   "a2",  "C 2/n 1 1", "", "-C 2xac",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  5,"C2h^6" , "84S345","01284S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(104, 15, 8,0,   "a3",  "I 2/c 1 1", "", "-I 2xc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^6" , "04S345","01204S", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(105, 15, 8,0,  "-a1",  "C 2/c 1 1", "", "-C 2xc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  5,"C2h^6" , "04S345","01204S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(106, 15, 8,0,  "-a2",  "B 2/n 1 1", "", "-B 2xab",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  5,"C2h^6" , "8N5345","0128N5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(107, 15, 8,0,  "-a3",  "I 2/b 1 1", "", "-I 2xb",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  5,"C2h^6" , "0N5345","0120N5", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    // ORTHORHOMBIC GROUPS
    // ===================
    
    SKSpaceGroupSetting(108, 16, 4,0,  "abc",     "P 2 2 2", "", "P 2 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^1"  , "342045","012045315342", int3x3(1)),
    
    SKSpaceGroupSetting(109, 17, 4,0,  "abc",    "P 2 2 21", "", "P 2c 2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^2"  , "34B045","01204531S34B", int3x3(1)),
    SKSpaceGroupSetting(110, 17, 4,0,  "cab",    "P 21 2 2", "", "P 2a 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^2"  , "I42845","012845315I42", int3x3(1)),
    SKSpaceGroupSetting(111, 17, 4,0,  "bca",    "P 2 21 2", "", "P 2 2b",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^2"  , "3420N5","0120N53=5342", int3x3(1)),
    
    SKSpaceGroupSetting(112, 18, 4,0,  "abc",   "P 21 21 2", "", "P 2 2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^3"  , "3428N5","0128N5I=5342", int3x3(1)),
    SKSpaceGroupSetting(113, 18, 4,0,  "cab",   "P 2 21 21", "", "P 2bc 2",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^3"  , "3NB045","0120453=S3NB", int3x3(1)),
    SKSpaceGroupSetting(114, 18, 4,0,  "bca",   "P 21 2 21", "", "P 2ac 2ac",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^3"  , "I4B84S","01284S315I4B", int3x3(1)),
    
    SKSpaceGroupSetting(115, 19, 4,0,  "abc",  "P 21 21 21", "", "P 2ac 2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  6,"D2^4"  , "I4B8N5","0128N53=SI4B", int3x3(1)),
    
    SKSpaceGroupSetting(116, 20, 8,0,  "abc",    "C 2 2 21", "", "C 2c 2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  6,"D2^5"  , "34B045","01204531S34B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(117, 20, 8,0,  "cab",    "A 21 2 2", "", "A 2a 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  6,"D2^5"  , "I42845","012845315I42", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(118, 20, 8,0,  "bca",    "B 2 21 2", "", "B 2 2b",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  6,"D2^5"  , "3420N5","0120N53=5342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(119, 21, 8,0,  "abc",     "C 2 2 2", "", "C 2 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  6,"D2^6"  , "342045","012045315342", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(120, 21, 8,0,  "cab",     "A 2 2 2", "", "A 2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  6,"D2^6"  , "342045","012045315342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(121, 21, 8,0,  "bca",     "B 2 2 2", "", "B 2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  6,"D2^6"  , "342045","012045315342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(122, 22,16,0,  "abc",     "F 2 2 2", "", "F 2 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  6,"D2^7"  , "342045","012045315342", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(123, 23, 8,0,  "abc",     "I 2 2 2", "", "I 2 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  6,"D2^8"  , "342045","012045315342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(124, 24, 8,0,  "abc",  "I 21 21 21", "", "I 2b 2c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  6,"D2^9"  , "3N204S","01204SI153N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(125, 25, 4,0,  "abc",     "P m m 2", "", "P 2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^1" , "342312","012342312042", int3x3(1)),
    SKSpaceGroupSetting(126, 25, 4,0,  "cab",     "P 2 m m", "", "P -2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^1" , "015045","012045042015", int3x3(1)),
    SKSpaceGroupSetting(127, 25, 4,0,  "bca",     "P m 2 m", "", "P -2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^1" , "015312","012315312015", int3x3(1)),
    
    SKSpaceGroupSetting(128, 26, 4,0,  "abc",    "P m c 21", "", "P 2c -2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^2" , "34B312","01234B31204B", int3x3(1)),
    SKSpaceGroupSetting(129, 26, 4,0, "ba-c",    "P c m 21", "", "P 2c -2c",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^2" , "34B31B","01234B31B042", int3x3(1)),
    SKSpaceGroupSetting(130, 26, 4,0,  "cab",    "P 21 m a", "", "P -2a 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^2" , "815845","012845042815", int3x3(1)),
    SKSpaceGroupSetting(131, 26, 4,0, "-cba",    "P 21 a m", "", "P -2 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^2" , "015845","012845842015", int3x3(1)),
    SKSpaceGroupSetting(132, 26, 4,0,  "bca",    "P b 21 m", "", "P -2 -2b",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^2" , "0153=2","0123=53=2015", int3x3(1)),
    SKSpaceGroupSetting(133, 26, 4,0, "a-cb",    "P m 21 b", "", "P -2b -2",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^2" , "0=5312","0123=53120=5", int3x3(1)),
    
    SKSpaceGroupSetting(134, 27, 4,0,  "abc",     "P c c 2", "", "P 2 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^3" , "34231B","01234231B04B", int3x3(1)),
    SKSpaceGroupSetting(135, 27, 4,0,  "cab",     "P 2 a a", "", "P -2a 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^3" , "815045","012045842815", int3x3(1)),
    SKSpaceGroupSetting(136, 27, 4,0,  "bca",     "P b 2 b", "", "P -2b -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^3" , "0=53=2","0123153=20=5", int3x3(1)),
    
    SKSpaceGroupSetting(137, 28, 4,0,  "abc",     "P m a 2", "", "P 2 -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^4" , "342I12","012342I12842", int3x3(1)),
    SKSpaceGroupSetting(138, 28, 4,0, "ba-c",     "P b m 2", "", "P 2 -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^4" , "3423=2","0123423=20N2", int3x3(1)),
    SKSpaceGroupSetting(139, 28, 4,0,  "cab",     "P 2 m b", "", "P -2b 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^4" , "0=5045","0120450N20=5", int3x3(1)),
    SKSpaceGroupSetting(140, 28, 4,0, "-cba",     "P 2 c m", "", "P -2c 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^4" , "01S045","01204504B01S", int3x3(1)),
    SKSpaceGroupSetting(141, 28, 4,0,  "bca",     "P c 2 m", "", "P -2c -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^4" , "01S31B","01231531B01S", int3x3(1)),
    SKSpaceGroupSetting(142, 28, 4,0, "a-cb",     "P m 2 a", "", "P -2a -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^4" , "815I12","012315I12815", int3x3(1)),
    
    SKSpaceGroupSetting(143, 29, 4,0,  "abc",    "P c a 21", "", "P 2c -2ac",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^1" , "34BI1B","01234BI1B842", int3x3(1)),
    SKSpaceGroupSetting(144, 29, 4,0, "ba-c",    "P b c 21", "", "P 2c -2b",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^5" , "34B3=2","01234B3=20NB", int3x3(1)),
    SKSpaceGroupSetting(145, 29, 4,0,  "cab",    "P 21 a b", "", "P -2b 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^5" , "0=5845","0128458N20=5", int3x3(1)),
    SKSpaceGroupSetting(146, 29, 4,0, "-cba",    "P 21 c a", "", "P -2ac 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^5" , "81S845","01284504B81S", int3x3(1)),
    SKSpaceGroupSetting(147, 29, 4,0,  "bca",    "P c 21 b", "", "P -2bc -2c",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^5" , "0=S31B","0123=531B0=S", int3x3(1)),
    SKSpaceGroupSetting(148, 29, 4,0, "a-cb",    "P b 21 a", "", "P -2a -2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^5" , "815I=2","0123=5I=2815", int3x3(1)),
    
    SKSpaceGroupSetting(149, 30, 4,0,  "abc",     "P n c 2", "", "P 2 -2bc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^6" , "3423=B","0123423=B0NB", int3x3(1)),
    SKSpaceGroupSetting(150, 30, 4,0, "ba-c",     "P c n 2", "", "P 2 -2ac",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^6" , "342I1B","012342I1B84B", int3x3(1)),
    SKSpaceGroupSetting(151, 30, 4,0,  "cab",     "P 2 n a", "", "P -2ac 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^6" , "81S045","01204584B81S", int3x3(1)),
    SKSpaceGroupSetting(152, 30, 4,0, "-cba",     "P 2 a n", "", "P -2ab 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^6" , "8=5045","0120458N28=5", int3x3(1)),
    SKSpaceGroupSetting(153, 30, 4,0,  "bca",     "P b 2 n", "", "P -2ab -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^6" , "8=5I=2","012315I=28=5", int3x3(1)),
    SKSpaceGroupSetting(154, 30, 4,0, "a-cb",     "P n 2 b", "", "P -2bc -2bc",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^6" , "0=S3=B","0123153=B0=S", int3x3(1)),
    
    SKSpaceGroupSetting(155, 31, 4,0,  "abc",    "P m n 21", "", "P 2ac -2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^7" , "I4B312","012I4B31284B", int3x3(1)),
    SKSpaceGroupSetting(156, 31, 4,0, "ba-c",    "P n m 21", "", "P 2bc -2bc",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^7" , "3NB3=B","0123NB3=B042", int3x3(1)),
    SKSpaceGroupSetting(157, 31, 4,0,  "cab",    "P 21 m n", "", "P -2ab 2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^7" , "8=58N5","0128N50428=5", int3x3(1)),
    SKSpaceGroupSetting(158, 31, 4,0, "-cba",    "P 21 n m", "", "P -2 2ac",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^7" , "01584S","01284S84B015", int3x3(1)),
    SKSpaceGroupSetting(159, 31, 4,0,  "bca",    "P n 21 m", "", "P -2 -2bc",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^7" , "0153=B","0123=S3=B015", int3x3(1)),
    SKSpaceGroupSetting(160, 31, 4,0, "a-cb",    "P m 21 n", "", "P -2ab -2",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^7" , "8=5312","012I=53128=5", int3x3(1)),
    
    SKSpaceGroupSetting(161, 32, 4,0,  "abc",     "P b a 2", "", "P 2 -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^8" , "342I=2","012342I=28N2", int3x3(1)),
    SKSpaceGroupSetting(162, 32, 4,0,  "cab",     "P 2 c b", "", "P -2bc 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^8" , "0=S045","0120450NB0=S", int3x3(1)),
    SKSpaceGroupSetting(163, 32, 4,0,  "bca",     "P c 2 a", "", "P -2ac -2ac",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^8" , "81SI1B","012315I1B81S", int3x3(1)),
    
    SKSpaceGroupSetting(164, 33, 4,0,  "abc",    "P n a 21", "", "P 2c -2n",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^9" , "34BI=B","01234BI=B8N2", int3x3(1)),
    SKSpaceGroupSetting(165, 33, 4,0, "ba-c",    "P b n 21", "", "P 2c -2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^9" , "34BI=2","01234BI=28NB", int3x3(1)),
    SKSpaceGroupSetting(166, 33, 4,0,  "cab",    "P 21 n b", "", "P -2bc 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^9" , "0=S845","0128458NB0=S", int3x3(1)),
    SKSpaceGroupSetting(167, 33, 4,0, "-cba",    "P 21 c n", "", "P -2n 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^9" , "8=S845","0128450NB8=S", int3x3(1)),
    SKSpaceGroupSetting(168, 33, 4,0,  "bca",    "P c 21 n", "", "P -2n -2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^9" , "8=SI1B","0123=5I1B8=S", int3x3(1)),
    SKSpaceGroupSetting(169, 33, 4,0, "a-cb",    "P n 21 a", "", "P -2ac -2n",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^9" , "81SI=B","0123=5I=B81S", int3x3(1)),
    
    SKSpaceGroupSetting(170, 34, 4,0,  "abc",     "P n n 2", "", "P 2 -2n",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^10", "342I=B","012342I=B8NB", int3x3(1)),
    SKSpaceGroupSetting(171, 34, 4,0,  "cab",     "P 2 n n", "", "P -2n 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^10", "8=S045","0120458NB8=S", int3x3(1)),
    SKSpaceGroupSetting(172, 34, 4,0,  "bca",     "P n 2 n", "", "P -2n -2n",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  7,"C2v^10", "8=SI=B","012315I=B8=S", int3x3(1)),
    
    SKSpaceGroupSetting(173, 35, 8,0,  "abc",     "C m m 2", "", "C 2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0),int3(6,6,0)},  7,"C2v^11", "342312","012342312042", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(174, 35, 8,0,  "cab",     "A 2 m m", "", "A -2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0),int3(0,6,6)},  7,"C2v^11", "015045","012045042015", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(175, 35, 8,0,  "bca",     "B m 2 m", "", "B -2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0),int3(6,0,6)},  7,"C2v^11", "015312","012315312015", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(176, 36, 8,0,  "abc",    "C m c 21", "", "C 2c -2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^12", "34B312","01234B31204B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(177, 36, 8,0, "ba-c",    "C c m 21", "", "C 2c -2c",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^12", "34B31B","01234B31B042", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(178, 36, 8,0,  "cab",    "A 21 m a", "", "A -2a 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^12", "815845","012845042815", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(179, 36, 8,0, "-cba",    "A 21 a m", "", "A -2 2a",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^12", "015845","012845842015", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(180, 36, 8,0,  "bca",    "B b 21 m", "", "B -2 -2b",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^12", "0153=2","0123=53=2015", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(181, 36, 8,0, "a-cb",    "B m 21 b", "", "B -2b -2",false, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^12", "0=5312","0123=53120=5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(182, 37, 8,0,  "abc",     "C c c 2", "", "C 2 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^13", "34231B","01234231B04B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(183, 37, 8,0,  "cab",     "A 2 a a", "", "A -2a 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^13", "815045","012045842815", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(184, 37, 8,0,  "bca",     "B b 2 b", "", "B -2b -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^13", "0=53=2","0123153=20=5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(185, 38, 8,0,  "abc",     "A m m 2", "", "A 2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^14", "342312","012342312042", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(186, 38, 8,0, "ba-c",     "B m m 2", "", "B 2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^14", "342312","012342312042", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(187, 38, 8,0,  "cab",     "B 2 m m", "", "B -2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^14", "015045","012045042015", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(188, 38, 8,0, "-cba",     "C 2 m m", "", "C -2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^14", "015045","012045042015", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(189, 38, 8,0,  "bca",     "C m 2 m", "", "C -2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^14", "015312","012315312015", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(190, 38, 8,0, "a-cb",     "A m 2 m", "", "A -2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^14", "015312","012315312015", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    
    SKSpaceGroupSetting(191, 39, 8,0,  "abc",     "A b m 2", "", "A 2 -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^15", "3423=2","0123423=20N2", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(192, 39, 8,0, "ba-c",     "B m a 2", "", "B 2 -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^15", "342I12","012342I12842", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(193, 39, 8,0,  "cab",     "B 2 c m", "", "B -2a 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^15", "815045","012045842815", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(194, 39, 8,0, "-cba",     "C 2 m b", "", "C -2a 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^15", "815045","012045842815", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(195, 39, 8,0,  "bca",     "C m 2 a", "", "C -2a -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^15", "815I12","012315I12815", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(196, 39, 8,0, "a-cb",     "A c 2 m", "", "A -2b -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^15", "0=53=2","0123153=20=5", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    
    SKSpaceGroupSetting(197, 40, 8,0,  "abc",     "A m a 2", "", "A 2 -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^16", "342I12","012342I12842", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(198, 40, 8,0, "ba-c",     "B b m 2", "", "B 2 -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^16", "3423=2","0123423=20N2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(199, 40, 8,0,  "cab",     "B 2 m b", "", "B -2b 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^16", "0=5045","0120450N20=5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(200, 40, 8,0, "-cba",     "C 2 c m", "", "C -2c 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^16", "01S045","01204504B01S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(201, 40, 8,0,  "bca",     "C c 2 m", "", "C -2c -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^16", "01S31B","01231531B01S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(202, 40, 8,0, "a-cb",     "A m 2 a", "", "A -2a -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^16", "815I12","012315I12815", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    
    SKSpaceGroupSetting(203, 41, 8,0,  "abc",     "A b a 2", "", "A 2 -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^17", "342I=2","012342I=28N2", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(204, 41, 8,0, "ba-c",     "B b a 2", "", "B 2 -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^17", "342I=2","012342I=28N2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(205, 41, 8,0,  "cab",     "B 2 c b", "", "B -2ab 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  7,"C2v^17", "8=5045","0120458N28=5", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(206, 41, 8,0, "-cba",     "C 2 c b", "", "C -2ac 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^17", "81S045","01204584B81S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(207, 41, 8,0,  "bca",     "C c 2 a", "", "C -2ac -2ac",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  7,"C2v^17", "81SI1B","012315I1B81S", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(208, 41, 8,0, "a-cb",     "A c 2 a", "", "A -2ab -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  7,"C2v^18", "8=5I=2","012315I=28=5", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    
    SKSpaceGroupSetting(209, 42,16,0,  "abc",     "F m m 2", "", "F 2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  7,"C2v^18", "342312","012342312042", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(210, 42,16,0,  "cab",     "F 2 m m", "", "F -2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  7,"C2v^18", "015045","012045042015", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(211, 42,16,0,  "bca",     "F m 2 m", "", "F -2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  7,"C2v^18", "015312","012315312015", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(212, 43,16,0,     "",     "F d d 2", "", "F 2 -2d",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  7,"C2v^19", "342K?D","012342K?D:PD", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(213, 43,16,0,  "cab",     "F 2 d d", "", "F -2d 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  7,"C2v^19", ":?U045","012045:PD:?U", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(214, 43,16,0,  "bca",     "F d 2 d", "", "F -2d -2d",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  7,"C2v^19", ":?UK?D","012315K?D:?U", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(215, 44, 8,0,  "abc",     "I m m 2", "", "I 2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^20", "342312","012342312042", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(216, 44, 8,0,  "cab",     "I 2 m m", "", "I -2 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^20", "015045","012045042015", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(217, 44, 8,0,  "bca",     "I m 2 m", "", "I -2 -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^20", "015312","012315312015", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(218, 45, 8,0,  "abc",     "I b a 2", "", "I 2 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^21", "34231B","01234231B04B", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(219, 45, 8,0,  "cab",     "I 2 c b", "", "I -2a 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^21", "815045","012045842815", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(220, 45, 8,0,  "bca",     "I c 2 a", "", "I -2b -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^21", "0=53=2","0123153=20=5", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(221, 46, 8,0,  "abc",     "I m a 2", "", "I 2 -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^22", "342I12","012342I12842", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(222, 46, 8,0, "ba-c",     "I b m 2", "", "I 2 -2b",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^22", "3423=2","0123423=20N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(223, 46, 8,0,  "cab",     "I 2 m b", "", "I -2b 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^22", "0=5045","0120450N20=5", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(224, 46, 8,0, "-cba",     "I 2 c m", "", "I -2c 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^22", "01S045","01204504B01S", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(225, 46, 8,0,  "bca",     "I c 2 m", "", "I -2c -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^22", "01S31B","01231531B01S", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(226, 46, 8,0, "a-cb",     "I m 2 a", "", "I -2a -2a",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  7,"C2v^22", "815I12","012315I12815", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(227, 47, 8,0,     "",     "P m m m", "", "-P 2 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^1" , "342045345","012045315342", int3x3(1)),
    
    SKSpaceGroupSetting(228, 48, 8,1,  "abc",     "P n n n", "", "P 2 2 -1n",false, int3(6,6,6), Symmorphicity::hemisymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^2" , "342045INS","012045315342", int3x3(1)),
    SKSpaceGroupSetting(229, 48, 8,2,  "abc",     "P n n n", "", "-P 2ab 2bc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^2" , "IN20NS345","0120NSI1SIN2", int3x3(1)),
    
    SKSpaceGroupSetting(230, 49, 8,0,  "abc",     "P c c m", "", "-P 2 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^3" , "34204S345","01204S31S342", int3x3(1)),
    SKSpaceGroupSetting(231, 49, 8,0,  "cab",     "P m a a", "", "-P 2a 2",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^3" , "I42045345","012045I15I42", int3x3(1)),
    SKSpaceGroupSetting(232, 49, 8,0,  "bca",     "P b m b", "", "-P 2b 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^3" , "3N20N5345","0120N53153N2", int3x3(1)),
    
    SKSpaceGroupSetting(233, 50, 8,1,  "abc",     "P b a n", "", "P 2 2 -1ab",false, int3(6,6,0), Symmorphicity::hemisymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^4" , "342045IN5","012045315342", int3x3(1)),
    SKSpaceGroupSetting(234, 50, 8,2,  "abc",     "P b a n", "", "-P 2ab 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^4" , "IN20N5345","0120N5I15IN2", int3x3(1)),
    SKSpaceGroupSetting(235, 50, 8,1,  "cab",     "P n c b", "", "P 2 2 -1bc",false, int3(0,6,6), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^4" , "3420453NS","012045315342", int3x3(1)),
    SKSpaceGroupSetting(236, 50, 8,2,  "cab",     "P n c b", "", "-P 2b 2bc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^4" , "3N20NS345","0120NS31S3N2", int3x3(1)),
    SKSpaceGroupSetting(237, 50, 8,1,  "bca",     "P c n a", "", "P 2 2 -1ac",false, int3(6,0,6), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^4" , "342045I4S","012045315342", int3x3(1)),
    SKSpaceGroupSetting(238, 50, 8,2,  "bca",     "P c n a", "", "-P 2a 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^4" , "I4204S345","01204SI1SI42", int3x3(1)),
    
    SKSpaceGroupSetting(239, 51, 8,0,     "",     "P m m a", "", "-P 2a 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^5" , "I42845345","012845315I42", int3x3(1)),
    SKSpaceGroupSetting(240, 51, 8,0, "ba-c",     "P m m b", "", "-P 2b 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^5" , "3N2045345","0120453=53N2", int3x3(1)),
    SKSpaceGroupSetting(241, 51, 8,0,  "cab",     "P b m m", "", "-P 2 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^5" , "3420N5345","0120N53=5342", int3x3(1)),
    SKSpaceGroupSetting(242, 51, 8,0, "-cba",     "P c m m", "", "-P 2c 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^5" , "34B04S345","01204S31534B", int3x3(1)),
    SKSpaceGroupSetting(243, 51, 8,0,  "bca",     "P m c m", "", "-P 2c 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^5" , "34B045345","01204531S34B", int3x3(1)),
    SKSpaceGroupSetting(244, 51, 8,0, "a-cb",     "P m a m", "", "-P 2 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^5" , "342845345","012845I15342", int3x3(1)),
    
    SKSpaceGroupSetting(245, 52, 8,0,  "abc",     "P n n a", "", "-P 2a 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^6" , "I420NS345","0120NSI=SI42", int3x3(1)),
    SKSpaceGroupSetting(246, 52, 8,0, "ba-c",     "P n n b", "", "-P 2b 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^6" , "3N28NS345","0128NSI1S3N2", int3x3(1)),
    SKSpaceGroupSetting(247, 52, 8,0,  "cab",     "P b n n", "", "-P 2n 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^6" , "INB0N5345","0120N5I1SINB", int3x3(1)),
    SKSpaceGroupSetting(248, 52, 8,0, "-cba",     "P c n n", "", "-P 2ab 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^6" , "IN204S345","01204SI=SIN2", int3x3(1)),
    SKSpaceGroupSetting(249, 52, 8,0,  "bca",     "P n c n", "", "-P 2ab 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^6" , "IN28NS345","0128NS31SIN2", int3x3(1)),
    SKSpaceGroupSetting(250, 52, 8,0, "a-cb",     "P n a n", "", "-P 2n 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^6" , "INB0NS345","0120NSI15INB", int3x3(1)),
    
    SKSpaceGroupSetting(251, 53, 8,0,  "abc",     "P m n a", "", "-P 2ac 2",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^7" , "I4B045345","012045I1SI4B", int3x3(1)),
    SKSpaceGroupSetting(252, 53, 8,0, "ba-c",     "P n m b", "", "-P 2bc 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^7" , "3NB0NS345","0120NS3153NB", int3x3(1)),
    SKSpaceGroupSetting(253, 53, 8,0,  "cab",     "P b m n", "", "-P 2ab 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^7" , "IN28N5345","0128N5315IN2", int3x3(1)),
    SKSpaceGroupSetting(254, 53, 8,0, "-cba",     "P c n m", "", "-P 2 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^7" , "34284S345","01284SI1S342", int3x3(1)),
    SKSpaceGroupSetting(255, 53, 8,0,  "bca",     "P n c m", "", "-P 2 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^7" , "3420NS345","0120NS3=S342", int3x3(1)),
    SKSpaceGroupSetting(256, 53, 8,0, "a-cb",     "P m a n", "", "-P 2ab 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^7" , "IN2045345","012045I=5IN2", int3x3(1)),
    
    SKSpaceGroupSetting(257, 54, 8,0,  "abc",     "P c c a", "", "-P 2a 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^8" , "I4284S345","01284S31SI42", int3x3(1)),
    SKSpaceGroupSetting(258, 54, 8,0, "ba-c",     "P c c b", "", "-P 2b 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^8" , "3N204S345","01204S3=S3N2", int3x3(1)),
    SKSpaceGroupSetting(259, 54, 8,0,  "cab",     "P b a a", "", "-P 2a 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^8" , "I420N5345","0120N5I=5I42", int3x3(1)),
    SKSpaceGroupSetting(260, 54, 8,0, "-cba",     "P c a a", "", "-P 2ac 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^8" , "I4B04S345","01204SI15I4B", int3x3(1)),
    SKSpaceGroupSetting(261, 54, 8,0,  "bca",     "P b c b", "", "-P 2bc 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^8" , "3NB0N5345","0120N531S3NB", int3x3(1)),
    SKSpaceGroupSetting(262, 54, 8,0, "a-cb",     "P b a b", "", "-P 2b 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^8" , "3N28N5345","0128N5I153N2", int3x3(1)),
    
    SKSpaceGroupSetting(263, 55, 8,0,  "abc",     "P b a m", "", "-P 2 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^9" , "3428N5345","0128N5I=5342", int3x3(1)),
    SKSpaceGroupSetting(264, 55, 8,0,  "cab",     "P m c b", "", "-P 2bc 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^9" , "3NB045345","0120453=S3NB", int3x3(1)),
    SKSpaceGroupSetting(265, 55, 8,0,  "bca",     "P c m a", "", "-P 2ac 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^9" , "I4B84S345","01284S315I4B", int3x3(1)),
    
    SKSpaceGroupSetting(266, 56, 8,0,  "abc",     "P c c n", "", "-P 2ab 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^10", "IN284S345","01284S3=SIN2", int3x3(1)),
    SKSpaceGroupSetting(267, 56, 8,0,  "cab",     "P n a a", "", "-P 2ac 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^10", "I4B0NS345","0120NSI=5I4B", int3x3(1)),
    SKSpaceGroupSetting(268, 56, 8,0,  "bca",     "P b n b", "", "-P 2bc 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^10", "3NB8N5345","0128N5I1S3NB", int3x3(1)),
    
    SKSpaceGroupSetting(269, 57, 8,0,  "abc",     "P b c m", "", "-P 2c 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^11", "34B0N5345","0120N53=S34B", int3x3(1)),
    SKSpaceGroupSetting(270, 57, 8,0, "ba-c",     "P c a m", "", "-P 2c 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^11", "34B84S345","01284SI1534B", int3x3(1)),
    SKSpaceGroupSetting(271, 57, 8,0,  "cab",     "P m c a", "", "-P 2ac 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^11", "I4B845345","01284531SI4B", int3x3(1)),
    SKSpaceGroupSetting(272, 57, 8,0, "-cba",     "P m a b", "", "-P 2b 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^11", "3N2845345","012845I=53N2", int3x3(1)),
    SKSpaceGroupSetting(273, 57, 8,0,  "bca",     "P b m a", "", "-P 2a 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^11", "I428N5345","0128N53=5I42", int3x3(1)),
    SKSpaceGroupSetting(274, 57, 8,0, "a-cb",     "P c m b", "", "-P 2bc 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^11", "3NB04S345","01204S3=53NB", int3x3(1)),
    
    SKSpaceGroupSetting(275, 58, 8,0,  "abc",     "P n n m", "", "-P 2 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^12", "3428NS345","0128NSI=S342", int3x3(1)),
    SKSpaceGroupSetting(276, 58, 8,0,  "cab",     "P m n n", "", "-P 2n 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^12", "INB045345","012045I=SINB", int3x3(1)),
    SKSpaceGroupSetting(277, 58, 8,0,  "bca",     "P n m n", "", "-P 2n 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^12", "INB8NS345","0128NS315INB", int3x3(1)),
    
    SKSpaceGroupSetting(278, 59, 8,1,  "abc",     "P m m n", "", "P 2 2ab -1ab",false, int3(6,6,0), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^13", "3428N5IN5","0128N5I=5342", int3x3(1)),
    SKSpaceGroupSetting(279, 59, 8,2,  "abc",     "P m m n", "", "-P 2ab 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^13", "IN2845345","0128453=5IN2", int3x3(1)),
    SKSpaceGroupSetting(280, 59, 8,1,  "cab",     "P n m m", "", "P 2bc 2 -1bc",false, int3(0,6,6), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^13", "3NB0453NS","0120453=S3NB", int3x3(1)),
    SKSpaceGroupSetting(281, 59, 8,2,  "cab",     "P n m m", "", "-P 2c 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^13", "34B0NS345","0120NS3=534B", int3x3(1)),
    SKSpaceGroupSetting(282, 59, 8,1,  "bca",     "P m n m", "", " P 2ac 2ac -1ac",false, int3(6,0,6), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^13", "I4B84SI4S","01284S315I4B", int3x3(1)),
    SKSpaceGroupSetting(283, 59, 8,2,  "bca",     "P m n m", "", "-P 2c 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^13", "34B845345","012845I1S34B", int3x3(1)),
    
    SKSpaceGroupSetting(284, 60, 8,0,  "abc",     "P b c n", "", "-P 2n 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^14", "INB8N5345","0128N531SINB", int3x3(1)),
    SKSpaceGroupSetting(285, 60, 8,0, "ba-c",     "P c a n", "", "-P 2n 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^14", "INB04S345","01204SI=5INB", int3x3(1)),
    SKSpaceGroupSetting(286, 60, 8,0,  "cab",     "P n c a", "", "-P 2a 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^14", "I428NS345","0128NS3=SI42", int3x3(1)),
    SKSpaceGroupSetting(287, 60, 8,0, "-cba",     "P n a b", "", "-P 2bc 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^14", "3NB8NS345","0128NSI153NB", int3x3(1)),
    SKSpaceGroupSetting(288, 60, 8,0,  "bca",     "P b n a", "", "-P 2ac 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^14", "I4B0N5345","0120N5I=SI4B", int3x3(1)),
    SKSpaceGroupSetting(289, 60, 8,0, "a-cb",     "P c n b", "", "-P 2b 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^14", "3N284S345","01284SI=S3N2", int3x3(1)),
    
    SKSpaceGroupSetting(290, 61, 8,0,  "abc",     "P b c a", "", "-P 2ac 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^15", "I4B8N5345","0128N53=SI4B", int3x3(1)),
    SKSpaceGroupSetting(291, 61, 8,0, "ba-c",     "P c a b", "", "-P 2bc 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^15", "3NB84S345","01284SI=53NB", int3x3(1)),
    
    SKSpaceGroupSetting(292, 62, 8,0,  "abc",     "P n m a", "", "-P 2ac 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^16", "I4B8NS345","0128NS3=5I4B", int3x3(1)),
    SKSpaceGroupSetting(293, 62, 8,0, "ba-c",     "P m n b", "", "-P 2bc 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^16", "3NB845345","012845I=S3NB", int3x3(1)),
    SKSpaceGroupSetting(294, 62, 8,0,  "cab",     "P b n m", "", "-P 2c 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^16", "34B8N5345","0128N5I=S34B", int3x3(1)),
    SKSpaceGroupSetting(295, 62, 8,0, "-cba",     "P c m n", "", "-P 2n 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^16", "INB84S345","01284S3=5INB", int3x3(1)),
    SKSpaceGroupSetting(296, 62, 8,0,  "bca",     "P m c n", "", "-P 2n 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^16", "INB845345","0128453=SINB", int3x3(1)),
    SKSpaceGroupSetting(297, 62, 8,0, "a-cb",     "P n a m", "", "-P 2c 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)},  8,"D2h^16", "34B8NS345","0128NSI=534B", int3x3(1)),
    
    SKSpaceGroupSetting(298, 63,16,0,  "abc",     "C m c m", "", "-C 2c 2",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^17", "34B045345","01204531S34B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(299, 63,16,0, "ba-c",     "C c m m", "", "-C 2c 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^17", "34B04S345","01204S31534B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(300, 63,16,0,  "cab",     "A m m a", "", "-A 2a 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^17", "I42845345","012845315I42", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(301, 63,16,0, "-cba",     "A m a m", "", "-A 2 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^17", "342845345","012845I15342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(302, 63,16,0,  "bca",     "B b m m", "", "-B 2 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^17", "3420N5345","0120N53=5342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(303, 63,16,0, "a-cb",     "B m m b", "", "-B 2b 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^17", "3N2045345","0120453=53N2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(304, 64,16,0,  "abc",     "C m c a", "", "-C 2ac 2",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^18", "I4B045345","012045I1SI4B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(305, 64,16,0, "ba-c",     "C c m b", "", "-C 2ac 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^18", "I4B84S345","01284S315I4B", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(306, 64,16,0,  "cab",     "A b m a", "", "-A 2ab 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^18", "IN28N5345","0128N5315IN2", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(307, 64,16,0, "-cba",     "A c a m", "", "-A 2 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^18", "3428N5345","0128N5I=5342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(308, 64,16,0,  "bca",     "B b c m", "", "-B 2 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^18", "3428N5345","0128N5I=5342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(309, 64,16,0, "a-cb",     "B m a b", "", "-B 2ab 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^18", "IN2045345","012045I=5IN2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(310, 65,16,0,  "abc",     "C m m m", "", "-C 2 2",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^19", "342045345","012045315342", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(311, 65,16,0,  "cab",     "A m m m", "", "-A 2 2",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^19", "342045345","012045315342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(312, 65,16,0,  "bca",     "B m m m", "", "-B 2 2",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^19", "342045345","012045315342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(313, 66,16,0,  "abc",     "C c c m", "", "-C 2 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^20", "34204S345","01204S31S342", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(314, 66,16,0,  "cab",     "A m a a", "", "-A 2a 2",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^20", "I42045345","012045I15I42", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(315, 66,16,0,  "bca",     "B b m b", "", "-B 2b 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^20", "3N20N5345","0120N53153N2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(316, 67,16,0,  "abc",     "C m m a", "", "-C 2a 2",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^21", "I42045345","012045I15I42", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(317, 67,16,0, "ba-c",     "C m m b", "", "-C 2a 2a",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^21", "I42845345","012845315I42", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(318, 67,16,0,  "cab",     "A b m m", "", "-A 2b 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^21", "3N20N5345","0120N53153N2", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(319, 67,16,0, "-cba",     "A c m m", "", "-A 2 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^21", "3420N5345","0120N53=5342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(320, 67,16,0,  "bca",     "B m c m", "", "-B 2 2a",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^21", "342845345","012845I15342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(321, 67,16,0, "a-cb",     "B m a m", "", "-B 2a 2",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^21", "I42045345","012045I15I42", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(322, 68,16,1,  "abc",     "C c c a", "", "C 2 2 -1ac",false, int3(0,6,6), Symmorphicity::hemisymmorphic,false,  Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^22", "3420453NS","012045315342", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(323, 68,16,2,  "abc",     "C c c a", "", "-C 2a 2ac",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^22", "I4284S345","01284S31SI42", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(324, 68,16,1, "ba-c",     "C c c b", "", "C 2 2 -1ac",false, int3(0,6,6), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^22", "3420453NS","012045315342", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(325, 68,16,2, "ba-c",     "C c c b", "", "-C 2a 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::c_face,   std::vector<int3>{int3(0,0,0), int3(6,6,0)},  8,"D2h^22", "I4204S345","01204SI1SI42", int3x3(int3(1, 1, 0), int3(1, -1, 0), int3(0, 0, -1))),
    SKSpaceGroupSetting(326, 68,16,1,  "cab",     "A b a a", "", "A 2 2 -1ab",false, int3(6,0,6), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^22", "342045I4S","012045315342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(327, 68,16,2,  "cab",     "A b a a", "", "-A 2a 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^22", "I420N5345","0120N5I=5I42", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(328, 68,16,1, "-cba",     "A c a a", "", "A 2 2 -1ab",false, int3(6,0,6), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^22", "342045I4S","012045315342", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(329, 68,16,2, "-cba",     "A c a a", "", "-A 2ab 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::a_face,   std::vector<int3>{int3(0,0,0), int3(0,6,6)},  8,"D2h^22", "IN20N5345","0120N5I15IN2", int3x3(int3(-1, 0, 0), int3(0, -1, 1), int3(0, 1, 1))),
    SKSpaceGroupSetting(330, 68,16,1,  "bca",     "B b c b", "", "B 2 2 -1ab",false, int3(0,6,6), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^22", "3420453NS","012045315342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(331, 68,16,2,  "bca",     "B b c b", "", "-B 2ab 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^22", "IN20N5345","0120N5I15IN2", int3x3(int3(-1, 0, 1), int3(0, -1, 1), int3(1, 0, 1))),
    SKSpaceGroupSetting(332, 68,16,1, "a-cb",     "B b a b", "", "B 2 2 -1ab",false, int3(0,6,6), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^22", "3420453NS","012045315342", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    SKSpaceGroupSetting(333, 68,16,2, "a-cb",     "B b a b", "", "-B 2b 2ab",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0), int3(6,0,6)},  8,"D2h^22", "3N28N5345","0128N5I153N2", int3x3(int3(-1, 0, 1), int3(0, -1, 0), int3(1, 0, 1))),
    
    SKSpaceGroupSetting(334, 69,32,0,  "abc",     "F m m m", "", "-F 2 2",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  8,"D2h^23", "342045345","012045315342", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(335, 70,32,1,  "abc",     "F d d d", "", "F 2 2 -1d",false, int3(3,3,3), Symmorphicity::hemisymmorphic,false,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  8,"D2h^24", "342045KPU","012045315342", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(336, 70,32,2,  "abc",     "F d d d", "", "-F 2uv 2vw",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)},  8,"D2h^24", "KP20PU345","0120PUK1UKP2", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(337, 71,16,0,  "abc",     "I m m m", "", "-I 2 2",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^25", "342045345","012045315342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(338, 72,16,0,  "abc",     "I b a m", "", "-I 2 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^26", "34204S345","01204S31S342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(339, 72,16,0,  "cab",     "I m c b", "", "-I 2a 2",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^26", "I42045345","012045I15I42", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(340, 72,16,0,  "bca",     "I c m a", "", "-I 2b 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^26", "3N20N5345","0120N53153N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(341, 73,16,0,     "",     "I b c a", "", "-I 2b 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^27", "3N204S345","01204SI153N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(342, 73,16,0, "ba-c",     "I c a b", "", "-I 2a 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^27", "I420N5345","0120N531SI42", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(343, 74,16,0,  "abc",     "I m m a", "", "-I 2b 2",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^28", "3N2045345","0120453=53N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(344, 74,16,0, "ba-c",     "I m m b", "", "-I 2a 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^28", "I42845345","012845315I42", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(345, 74,16,0,  "cab",     "I b m m", "", "-I 2c 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^28", "34B04S345","01204S31534B", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(346, 74,16,0, "-cba",     "I c m m", "", "-I 2 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^28", "3420N5345","0120N53=5342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(347, 74,16,0,  "bca",     "I m c m", "", "-I 2 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^28", "342845345","012845I15342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(348, 74,16,0, "a-cb",     "I m a m", "", "-I 2c 2",true, int3(0,0,0), Symmorphicity::asymmorphic,false, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  8,"D2h^28", "34B045345","01204531S34B", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    // TETRAGONAL GROUPS
    // =================
    
    SKSpaceGroupSetting(349, 75, 4,0,     "",         "P 4", "", "P 4",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  9,"C4^1"  , "402","012402132342", int3x3(1)),
    
    SKSpaceGroupSetting(350, 76, 4,0,     "",        "P 41", "", "P 4w",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  9,"C4^2"  , "40D","01240D13G34B", int3x3(1)),
    
    SKSpaceGroupSetting(351, 77, 4,0,     "",        "P 42", "", "P 4c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  9,"C4^3"  , "40B","01240B13B342", int3x3(1)),
    
    SKSpaceGroupSetting(352, 78, 4,0,     "",        "P 43", "", "P 4cw",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)},  9,"C4^4"  , "40G","01240G13D34B", int3x3(1)),
    
    SKSpaceGroupSetting(353, 79, 8,0,     "",         "I 4", "", "I 4",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  9,"C4^5"  , "402","012402132342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(354, 80, 8,0,     "",        "I 41", "", "I 4bw",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)},  9,"C4^6"  , "N0G","012N0G=3G342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(355, 81, 4,0,     "",        "P -4", "", "P -4",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 10,"S4^1"  , "135","012135405342", int3x3(1)),
    
    SKSpaceGroupSetting(356, 82, 8,0,     "",        "I -4", "", "I -4",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 10,"S4^2"  , "135","012135405342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(357, 83, 8,0,     "",       "P 4/m", "", "-P 4",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 11,"C4h^1" , "402345","012402132342", int3x3(1)),
    
    SKSpaceGroupSetting(358, 84, 8,0,     "",      "P 42/m", "", "-P 4c",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 11,"C4h^2" , "40B345","01240B13B342", int3x3(1)),
    
    SKSpaceGroupSetting(359, 85, 8,1,  "abc",       "P 4/n", "", "P 4ab -1ab",false, int3(6,6,0), Symmorphicity::hemisymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 11,"C4h^3" , "N82IN5","012N82=I2342", int3x3(1)),
    SKSpaceGroupSetting(360, 85, 8,2,  "abc",       "P 4/n", "", "-P 4a",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 11,"C4h^3" , "N02345","012N021I2IN2", int3x3(1)),
    
    SKSpaceGroupSetting(361, 86, 8,1,  "abc",      "P 42/n", "", "P 4n -1n",false, int3(6,6,6), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 11,"C4h^4" , "N8BINS","012N8B=IB342", int3x3(1)),
    SKSpaceGroupSetting(362, 86, 8,2,  "abc",      "P 42/n", "", "-P 4bc",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 11,"C4h^4" , "48B345","01248B=3BIN2", int3x3(1)),
    
    SKSpaceGroupSetting(363, 87,16,0,     "",       "I 4/m", "", "-I 4",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 11,"C4h^5" , "402345","012402132342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(364, 88,16,1,  "abc",      "I 41/a", "", "I 4bw -1bw",false, int3(0,6,3), Symmorphicity::asymmorphic,false,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 11,"C4h^6" , "N0G3NU","012N0G=3G342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(365, 88,16,2,  "abc",      "I 41/a", "", "-I 4ad",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 11,"C4h^6" , "P<G345","012P<G?KD3N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(366, 89, 8,0,     "",     "P 4 2 2", "", "P 4 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^1"  , "402045","012402132045315342105435", int3x3(1)),
    
    SKSpaceGroupSetting(367, 90, 8,0,     "",    "P 4 21 2", "", "P 4ab 2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^2"  , "N828N5","012N82=I28N5I=5342105435", int3x3(1)),
    
    SKSpaceGroupSetting(368, 91, 8,0,     "",    "P 41 2 2", "", "P 4w 2c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^3"  , "40D04S","01240D13G04S31534B10X43U", int3x3(1)),
    
    SKSpaceGroupSetting(369, 92, 8,0,     "",   "P 41 21 2", "", "P 4abw 2nw",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^4"  , "N8D8NX","012N8D=IG8NXI=U34B10543S", int3x3(1)),
    
    SKSpaceGroupSetting(370, 93, 8,0,     "",    "P 42 2 2", "", "P 4c 2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^5"  , "40B045","01240B13B04531534210S43S", int3x3(1)),
    
    SKSpaceGroupSetting(371, 94, 8,0,     "",   "P 42 21 2", "", "P 4n 2n",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^6"  , "N8B8NS","012N8B=IB8NSI=S342105435", int3x3(1)),
    
    SKSpaceGroupSetting(372, 95, 8,0,     "",    "P 43 2 2", "", "P 4cw 2c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^7"  , "40G04S","01240G13D04S31534B10U43X", int3x3(1)),
    
    SKSpaceGroupSetting(373, 96, 8,0,     "",   "P 43 21 2", "", "P 4nw 2abw",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 12,"D4^8"  , "N8G8NU","012N8G=ID8NUI=X34B10543S", int3x3(1)),
    
    SKSpaceGroupSetting(374, 97,16,0,     "",     "I 4 2 2", "", "I 4 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 12,"D4^9"  , "402045","012402132045315342105435",  int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(375, 98,16,0,     "",    "I 41 2 2", "", "I 4bw 2bw",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 12,"D4^10" , "N0G84X","012N0G=3G84XI1X342105435", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(376, 99, 8,0,     "",     "P 4 m m", "", "P 4 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^1" , "402312","012402132342312042432102", int3x3(1)),
    
    SKSpaceGroupSetting(377,100, 8,0,     "",     "P 4 b m", "", "P 4 -2ab",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^2" , "402I=2","012402132342I=28N2NI2=82", int3x3(1)),
    
    SKSpaceGroupSetting(378,101, 8,0,     "",    "P 42 c m", "", "P 4c -2c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^3" , "40B31B","01240B13B34231B04B432102", int3x3(1)),
    
    SKSpaceGroupSetting(379,102, 8,0,     "",    "P 42 n m", "", "P 4n -2n",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^4" , "N8BI=B","012N8B=IB342I=B8NB432102", int3x3(1)),
    
    SKSpaceGroupSetting(380,103, 8,0,     "",     "P 4 c c", "", "P 4 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^5" , "40231B","01240213234231B04B43B10B", int3x3(1)),
    
    SKSpaceGroupSetting(381,104, 8,0,     "",     "P 4 n c", "", "P 4 -2n",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^6" , "402I=B","012402132342I=B8NBNIB=8B", int3x3(1)),
    
    SKSpaceGroupSetting(382,105, 8,0,     "",    "P 42 m c", "", "P 4c -2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^7" , "40B312","01240B13B34231204243B10B", int3x3(1)),
    
    SKSpaceGroupSetting(383,106, 8,0,     "",    "P 42 b c", "", "P 4c -2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 13,"C4v^8" , "40BI=2","01240B13B342I=28N2NIB=8B", int3x3(1)),
    
    SKSpaceGroupSetting(384,107,16,0,     "",     "I 4 m m", "", "I 4 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 13,"C4v^9" , "402312","012402132342312042432102", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(385,108,16,0,     "",     "I 4 c m", "", "I 4 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 13,"C4v^10", "40231B","01240213234231B04B43B10B", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(386,109,16,0,     "",    "I 41 m d", "", "I 4bw -2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 13,"C4v^11", "N0G312","012N0G=3G342312042N3G=0G", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(387,110,16,0,     "",    "I 41 c d", "", "I 4bw -2c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 13,"C4v^12", "N0G31B","012N0G=3G34231B04BN3D=0D", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(388,111, 8,0,     "",    "P -4 2 m", "", "P -4 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^1" , "135045","012135405045315342432102", int3x3(1)),
    
    SKSpaceGroupSetting(389,112, 8,0,     "",    "P -4 2 c", "", "P -4 2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^2" , "13504S","01213540504S31S34243B10B", int3x3(1)),
    
    SKSpaceGroupSetting(390,113, 8,0,     "",   "P -4 21 m", "", "P -4 2ab",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^3" , "1358N5","0121354058N5I=5342NI2=82", int3x3(1)),
    
    SKSpaceGroupSetting(391,114, 8,0,     "",   "P -4 21 c", "", "P -4 2n",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^4" , "1358NS","0121354058NSI=S342NIB=8B", int3x3(1)),
    
    SKSpaceGroupSetting(392,115, 8,0,     "",    "P -4 m 2", "", "P -4 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^5" , "135312","012135405342105435312042", int3x3(1)),
    
    SKSpaceGroupSetting(393,116, 8,0,     "",    "P -4 c 2", "", "P -4 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^6" , "13531B","01213540534210S43S31B04B", int3x3(1)),
    
    SKSpaceGroupSetting(394,117, 8,0,     "",    "P -4 b 2", "", "P -4 -2ab",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^7" , "135I=2","012135405342=85NI5I=28N2", int3x3(1)),
    
    SKSpaceGroupSetting(395,118, 8,0,     "",    "P -4 n 2", "", "P -4 -2n",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 14,"D2d^8" , "135I=B","012135405342=8SNISI=B8NB", int3x3(1)),
    
    SKSpaceGroupSetting(396,119,16,0,     "",    "I -4 m 2", "", "I -4 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 14,"D2d^9" , "135312","012135405342105435312042", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(397,120,16,0,     "",    "I -4 c 2", "", "I -4 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 14,"D2d^10", "13531B","01213540534210S43S31B04B", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(398,121,16,0,     "",    "I -4 2 m", "", "I -4 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 14,"D2d^11", "135045","012135405045315342432102", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(399,122,16,0,     "",    "I -4 2 d", "", "I -4 2bw",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 14,"D2d^12", "13584X","01213540584XI1X342N3G=0G", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(400,123,16,0,     "",   "P 4/m m m", "", "-P 4 2",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^1" , "402045345","012402132045315342105435", int3x3(1)),
    
    SKSpaceGroupSetting(401,124,16,0,     "",   "P 4/m c c", "", "-P 4 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^2" , "40204S345","01240213204S31S34210S43S", int3x3(1)),
    
    SKSpaceGroupSetting(402,125,16,1,  "abc",   "P 4/n b m", "", "P 4 2 -1ab",false, int3(6,6,0), Symmorphicity::hemisymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^3" , "402045IN5","012402132045315342105435", int3x3(1)),
    SKSpaceGroupSetting(403,125,16,2,  "abc",   "P 4/n b m", "", "-P 4a 2b",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^3" , "N020N5345","012N021I20N5I15IN2105NI5", int3x3(1)),
    
    SKSpaceGroupSetting(404,126,16,1,  "abc",   "P 4/n n c", "", "P 4 2 -1n",false, int3(6,6,6), Symmorphicity::hemisymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^4" , "402045INS","012402132045315342105435", int3x3(1)),
    SKSpaceGroupSetting(405,126,16,2,  "abc",   "P 4/n n c", "", "-P 4a 2bc",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^4" , "N020NS345","012N021I20NSI1SIN210SNIS", int3x3(1)),
    
    SKSpaceGroupSetting(406,127,16,0,     "",   "P 4/m b m", "", "-P 4 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^5" , "4028N5345","0124021328N5I=5342=85NI5", int3x3(1)),
    
    SKSpaceGroupSetting(407,128,16,0,     "",   "P 4/m n c", "", "-P 4 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^6" , "4028NS345","0124021328NSI=S342=8SNIS", int3x3(1)),
    
    SKSpaceGroupSetting(408,129,16,1,  "abc",   "P 4/n m m", "", "P 4ab 2ab -1ab",false, int3(6,6,0), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^7" , "N828N5IN5","012N82=I28N5I=5342105435", int3x3(1)),
    SKSpaceGroupSetting(409,129,16,2,  "abc",   "P 4/n m m", "", "-P 4a 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^7" , "N02845345","012N021I28453=5IN2=85435", int3x3(1)),
    
    SKSpaceGroupSetting(410,130,16,1,  "abc",   "P 4/n c c", "", "P 4ab 2n -1ab",false, int3(6,6,0), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^8" , "N828NSIN5","012N82=I28NSI=S34210S43S", int3x3(1)),
    SKSpaceGroupSetting(411,130,16,2,  "abc",   "P 4/n c c", "", "-P 4a 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^8" , "N0284S345","012N021I284S3=SIN2=8S43S", int3x3(1)),
    
    SKSpaceGroupSetting(412,131,16,0,     "",  "P 42/m m c", "", "-P 4c 2",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^9" , "40B045345","01240B13B04531534210S43S", int3x3(1)),
    
    SKSpaceGroupSetting(413,132,16,0,     "",  "P 42/m c m", "", "-P 4c 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^10", "40B04S345","01240B13B04S31S342105435", int3x3(1)),
    
    SKSpaceGroupSetting(414,133,16,1,  "abc",  "P 42/n b c", "", "P 4n 2c -1n",false, int3(6,6,6), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^11", "N8B04SINS","012N8B=IB04S31S342=85NI5", int3x3(1)),
    SKSpaceGroupSetting(415,133,16,2,  "abc",  "P 42/n b c", "", "-P 4ac 2b",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^11", "N0B0N5345","012N0B1IB0N5I15IN210SNIS", int3x3(1)),
    
    SKSpaceGroupSetting(416,134,16,1,  "abc",  "P 42/n n m", "", "P 4n 2 -1n",false, int3(6,6,6), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^12", "N8B045INS","012N8B=IB045315342=8SNIS", int3x3(1)),
    SKSpaceGroupSetting(417,134,16,2,  "abc",  "P 42/n n m", "", "-P 4ac 2bc",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^12", "N0B0NS345","012N0B1IB0NSI1SIN2105NI5", int3x3(1)),
    
    SKSpaceGroupSetting(418,135,16,0,     "",  "P 42/m b c", "", "-P 4c 2ab",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^13", "40B8N5345","01240B13B8N5I=5342=8SNIS", int3x3(1)),
    
    SKSpaceGroupSetting(419,136,16,0,     "",  "P 42/m n m", "", "-P 4n 2n",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^14", "N8B8NS345","012N8B=IB8NSI=S342105435", int3x3(1)),
    
    SKSpaceGroupSetting(420,137,16,1,  "abc",  "P 42/n m c", "", "P 4n 2n -1n",false, int3(6,6,6), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^15", "N8B8NSINS","012N8B=IB8NSI=S342105435", int3x3(1)),
    SKSpaceGroupSetting(421,137,16,2,  "abc",  "P 42/n m c", "", "-P 4ac 2a",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^15", "N0B845345","012N0B1IB8453=5IN2=8S43S", int3x3(1)),
    
    SKSpaceGroupSetting(422,138,16,1,  "abc",  "P 42/n c m", "", "P 4n 2ab -1n",false, int3(6,6,6), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^16", "N8B8N5INS","012N8B=IB8N5I=534210S43S", int3x3(1)),
    SKSpaceGroupSetting(423,138,16,2,  "abc",  "P 42/n c m", "", "-P 4ac 2ac",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 15,"D4h^16", "N0B84S345","012N0B1IB84S3=SIN2=85435", int3x3(1)),
    
    SKSpaceGroupSetting(424,139,32,0,     "",   "I 4/m m m", "",  "-I 4 2",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 15,"D4h^17", "402045345","012402132045315342105435", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(425,140,32,0,     "",   "I 4/m c m", "",  "-I 4 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 15,"D4h^18", "40204S345","01240213204S31S34210S43S", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(426,141,32,1,  "abc",  "I 41/a m d", "", "I 4bw 2bw -1bw",false, int3(0,6,3), Symmorphicity::asymmorphic,false,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 15,"D4h^19", "N0G84X3NU","012N0G=3G84XI1X342105435", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(427,141,32,2,  "abc",  "I 41/a m d", "", "-I 4bd 2",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 15,"D4h^19", "P<D045345","012P<D?KG0453=53N2?<UPKX", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(428,142,32,1,  "abc",  "I 41/a c d", "", "I 4bw 2aw -1bw",false, int3(0,6,3), Symmorphicity::asymmorphic,false,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 15,"D4h^20", "N0G84U3NU","012N0G=3G84UI1U34210S43S", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    SKSpaceGroupSetting(429,142,32,2,  "abc",  "I 41/a c d", "", "-I 4bd 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 15,"D4h^20", "P<D04S345","012P<D?KG04SI153N2?<XPKU", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    // TRIGONAL GROUPS
    // ===============
    
    SKSpaceGroupSetting(430,143, 3,0,     "",         "P 3", "", "P 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 16,"C3^1"  , "462","012462732", int3x3(1)),
    
    SKSpaceGroupSetting(431,144, 3,0,     "",        "P 31", "", "P 31",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 16,"C3^2"  , "46C","01246C73F", int3x3(1)),
    
    SKSpaceGroupSetting(432,145, 3,0,     "",        "P 32", "", "P 32",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 16,"C3^3"  , "46F","01246F73C", int3x3(1)),
    
    SKSpaceGroupSetting(433,146, 9,0,    "H",         "R 3", "", "R 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,   Centring::r, std::vector<int3>{int3(0,0,0), int3(8,4,4), int3(4,8,8)}, 16,"C3^4"  , "462","012462732", int3x3(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1))),
    SKSpaceGroupSetting(434,146, 3,0,    "R",         "R 3", "", "P 3*",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 16,"C3^4"  , "201","012201120", int3x3(1)),
    
    SKSpaceGroupSetting(435,147, 6,0,     "",        "P -3", "", "-P 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 17,"C3i^1" , "462345","012462732", int3x3(1)),
    
    SKSpaceGroupSetting(436,148,18,0,    "H",        "R -3", "", "-R 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,   Centring::r, std::vector<int3>{int3(0,0,0), int3(8,4,4), int3(4,8,8)}, 17,"C3i^2" , "462345","012462732", int3x3(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1))),
    SKSpaceGroupSetting(437,148, 6,0,    "R",        "R -3", "", "-P 3*",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 17,"C3i^2" , "201345","012201120", int3x3(1)),
    
    SKSpaceGroupSetting(438,149, 6,0,     "",     "P 3 1 2", "", "P 3 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 18,"D3^1"  , "462435","012462732435715065", int3x3(1)),
    
    SKSpaceGroupSetting(439,150, 6,0,     "",     "P 3 2 1", "", "P 3 2\"",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 18,"D3^2"  , "462105","012462732645375105", int3x3(1)),
    
    SKSpaceGroupSetting(440,151, 6,0,     "",    "P 31 1 2", "", "P 31 2 (0 0 4)",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 18,"D3^3"  , "46C43W","01246C73F43W71T065", int3x3(1)),
    
    SKSpaceGroupSetting(441,152, 6,0,     "",    "P 31 2 1", "", "P 31 2\"",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 18,"D3^4"  , "46C105","01246C73F64W37T105", int3x3(1)),
    
    SKSpaceGroupSetting(442,153, 6,0,     "",    "P 32 1 2", "", "P 32 2 (0 0 2)",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 18,"D3^5"  , "46F43T","01246F73C43T71W065", int3x3(1)),
    
    SKSpaceGroupSetting(443,154, 6,0,     "",    "P 32 2 1", "", "P 32 2\"",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 18,"D3^6"  , "46F105","01246F73C64T37W105", int3x3(1)),
    
    SKSpaceGroupSetting(444,155,18,0,    "H",       "R 3 2", "", "R 3 2\"",false, int3(0,0,0), Symmorphicity::symmorphic,true,   Centring::r, std::vector<int3>{int3(0,0,0), int3(8,4,4), int3(4,8,8)}, 18,"D3^7"  , "462105","012462732645375105", int3x3(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1))),
    SKSpaceGroupSetting(445,155, 6,0,    "R",       "R 3 2", "", "P 3* 2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 18,"D3^7"  , "201435","012201120435543354", int3x3(1)),
    
    SKSpaceGroupSetting(446,156, 6,0,     "",     "P 3 m 1", "", "P 3 -2\"",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 19,"C3v^1" , "462432","012462732712062432", int3x3(1)),
    
    SKSpaceGroupSetting(447,157, 6,0,     "",     "P 3 1 m", "", "P 3 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 19,"C3v^2" , "462102","012462732102642372", int3x3(1)),
    
    SKSpaceGroupSetting(448,158, 6,0,     "",     "P 3 c 1", "", "P 3 -2\"c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 19,"C3v^3" , "46243B","01246273271B06B43B", int3x3(1)),
    
    SKSpaceGroupSetting(449,159, 6,0,     "",     "P 3 1 c", "", "P 3 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 19,"C3v^4" , "46210B","01246273210B64B37B", int3x3(1)),
    
    SKSpaceGroupSetting(450,160,18,0,    "H",       "R 3 m", "", "R 3 -2\"",true, int3(0,0,0), Symmorphicity::symmorphic,true,   Centring::r, std::vector<int3>{int3(0,0,0), int3(8,4,4), int3(4,8,8)}, 19,"C3v^5" , "462432","012462732712062432", int3x3(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1))),
    SKSpaceGroupSetting(451,160, 6,0,    "R",       "R 3 m", "", "P 3* -2",false, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 19,"C3v^5" , "201102","012201120102210021", int3x3(1)),
    
    SKSpaceGroupSetting(452,161,18,0,    "H",       "R 3 c", "", "R 3 -2\"c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,   Centring::r, std::vector<int3>{int3(0,0,0), int3(8,4,4), int3(4,8,8)}, 19,"C3v^6" , "46243B","01246273271B06B43B", int3x3(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1))),
    SKSpaceGroupSetting(453,161, 6,0,    "R",       "R 3 c", "", "P 3* -2n",false, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 19,"C3v^6" , "201=8B","012201120=8BB=88B=", int3x3(1)),
    
    SKSpaceGroupSetting(454,162,12,0,     "",    "P -3 1 m", "", "-P 3 2",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 20,"D3d^1" , "462435345","012462732435715065", int3x3(1)),
    
    SKSpaceGroupSetting(455,163,12,0,     "",    "P -3 1 c", "", "-P 3 2c",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 20,"D3d^2" , "46243S345","01246273243S71S06S", int3x3(1)),
    
    SKSpaceGroupSetting(456,164,12,0,     "",    "P -3 m 1", "", "-P 3 2\"",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 20,"D3d^3" , "462105345","012462732645375105", int3x3(1)),
    
    SKSpaceGroupSetting(457,165,12,0,     "",    "P -3 c 1", "", "-P 3 2\"c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 20,"D3d^4" , "46210S345","01246273264S37S10S", int3x3(1)),
    
    SKSpaceGroupSetting(458,166,36,0,    "H",      "R -3 m", "", "-R 3 2\"",true, int3(0,0,0), Symmorphicity::symmorphic,true,   Centring::r, std::vector<int3>{int3(0,0,0), int3(8,4,4), int3(4,8,8)}, 20,"D3d^5" , "462105345","012462732645375105", int3x3(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1))),
    SKSpaceGroupSetting(459,166,12,0,    "R",      "R -3 m", "", "-P 3* 2",true, int3(0,0,0), Symmorphicity::symmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 20,"D3d^5" , "201435345","012201120435543354", int3x3(1)),
    
    SKSpaceGroupSetting(460,167,36,0,    "H",      "R -3 c", "", "-R 3 2\"c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,   Centring::r, std::vector<int3>{int3(0,0,0), int3(8,4,4), int3(4,8,8)},  20,"D3d^6" , "46210S345","01246273264S37S10S", int3x3(int3(1, 0, 1), int3(-1, 1, 1), int3(0, -1, 1))),
    SKSpaceGroupSetting(461,167,12,0,    "R",      "R -3 c", "", "-P 3* 2n",true, int3(0,0,0), Symmorphicity::hemisymmorphic,false, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 20,"D3d^6" , "201NIS345","012201120NISSNIISN", int3x3(1)),
    
    // HEXAGONAL GROUPS
    // ================
    
    SKSpaceGroupSetting(462,168, 6,0,     "",         "P 6", "", "P 6",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 21,"C6^1"  , "602","012602172462732342", int3x3(1)),
    
    SKSpaceGroupSetting(463,169, 6,0,     "",        "P 61", "", "P 61",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 21,"C6^2"  , "60E","01260E17H46C73F34B", int3x3(1)),
    
    SKSpaceGroupSetting(464,170, 6,0,     "",        "P 65", "", "P 65",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 21,"C6^3"  , "60H","01260H17E46F73C34B", int3x3(1)),
    
    SKSpaceGroupSetting(465,171, 6,0,     "",        "P 62", "", "P 62",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 21,"C6^4"  , "60C","01260C17F46F73C342", int3x3(1)),
    
    SKSpaceGroupSetting(466,172, 6,0,     "",        "P 64", "", "P 64",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 21,"C6^5"  , "60F","01260F17C46C73F342", int3x3(1)),
    
    SKSpaceGroupSetting(467,173, 6,0,     "",        "P 63", "", "P 6c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 21,"C6^6"  , "60B","01260B17B46273234B", int3x3(1)),
    
    SKSpaceGroupSetting(468,174, 6,0,     "",        "P -6", "", "P -6",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 22,"C3h^1" , "735","012735465462732015", int3x3(1)),
    
    SKSpaceGroupSetting(469,175,12,0,     "",       "P 6/m", "", "-P 6" ,true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 23,"C6h^1" , "602345","012602172462732342", int3x3(1)),
    
    SKSpaceGroupSetting(470,176,12,0,     "",      "P 63/m", "", "-P 6c",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 23,"C6h^2" , "60B345","01260B17B46273234B", int3x3(1)),
    
    SKSpaceGroupSetting(471,177, 12,0,    "",     "P 6 2 2", "", "P 6 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 24,"D6^1"  , "602105","012602172462732645375342105435715065", int3x3(1)),
    
    SKSpaceGroupSetting(472,178,12,0,     "",    "P 61 2 2", "", "P 61 2 (0 0 5)",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 24,"D6^2"  , "60E10T","01260E17H46C73F64537W34B10T43Y71S06V", int3x3(1)),
    
    SKSpaceGroupSetting(473,179,12,0,     "",    "P 65 2 2", "", "P 65 2 (0 0 1)",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 24,"D6^3"  , "60H10W","01260H17E46F73C64537T34B10W43V71S06Y", int3x3(1)),
    
    SKSpaceGroupSetting(474,180,12,0,     "",    "P 62 2 2", "", "P 62 2 (0 0 4)",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 24,"D6^4"  , "60C10W","01260C17F46F73C64537T34210W43W71506T", int3x3(1)),
    
    SKSpaceGroupSetting(475,181,12,0,     "",    "P 64 2 2", "", "P 64 2 (0 0 2)",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 24,"D6^5"  , "60F10T","01260F17C46C73F64537W34210T43T71506W", int3x3(1)),
    
    SKSpaceGroupSetting(476,182,12,0,     "",    "P 63 2 2", "", "P 6c 2c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 24,"D6^6"  , "60B105","01260B17B46273264537534B10543S71S06S", int3x3(1)),
    
    SKSpaceGroupSetting(477,183,12,0,     "",     "P 6 m m", "", "P 6 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 25,"C6v^1" , "602432","012602172462732342712062432102642372", int3x3(1)),
    
    SKSpaceGroupSetting(478,184,12,0,     "",     "P 6 c c", "", "P 6 -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 25,"C6v^2" , "60243B","01260217246273234271B06B43B10B64B37B", int3x3(1)),
    
    SKSpaceGroupSetting(479,185,12,0,     "",    "P 63 c m", "", "P 6c -2",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 25,"C6v^3" , "60B43B","01260B17B46273234B71B06B43B102642372", int3x3(1)),
    
    SKSpaceGroupSetting(480,186,12,0,     "",    "P 63 m c", "", "P 6c -2c",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 25,"C6v^4" , "60B432","01260B17B46273234B71206243210B64B37B", int3x3(1)),
    
    SKSpaceGroupSetting(481,187,12,0,     "",    "P -6 m 2", "", "P -6 2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 26,"D3h^1" , "735432","012735465462732435715065712062015432", int3x3(1)),
    
    SKSpaceGroupSetting(482,188,12,0,     "",    "P -6 c 2", "", "P -6c 2",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 26,"D3h^2" , "73S43B","01273S46S46273243571506571B06B01S43B", int3x3(1)),
    
    SKSpaceGroupSetting(483,189,12,0,     "",    "P -6 2 m", "", "P -6 -2",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 26,"D3h^3" , "735105","012735465462732645375105015102642372", int3x3(1)),
    
    SKSpaceGroupSetting(484,190,12,0,     "",    "P -6 2 c", "", "P -6c -2c",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 26,"D3h^4" , "73S105","01273S46S46273264537510501S10B64B37B", int3x3(1)),
    
    SKSpaceGroupSetting(485,191,24,0,     "",   "P 6/m m m", "", "-P 6 2",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 27,"D6h^1" , "602105345","012602172462732645375342105435715065", int3x3(1)),
    
    SKSpaceGroupSetting(486,192,24,0,     "",   "P 6/m c c", "", "-P 6 2c",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 27,"D6h^2" , "60210S345","01260217246273264S37S34210S43S71S06S", int3x3(1)),
    
    SKSpaceGroupSetting(487,193,24,0,     "",  "P 63/m c m", "", "-P 6c 2",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 27,"D6h^3" , "60B10S345","01260B17B46273264S37S34B10S435715065", int3x3(1)),
    
    SKSpaceGroupSetting(488,194,24,0,     "",  "P 63/m m c", "", "-P 6c 2c",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 27,"D6h^4" , "60B105345","01260B17B46273264537534B10543S71S06S", int3x3(1)),
    
    // CUBIC GROUPS
    // ============
    
    SKSpaceGroupSetting(489,195,12,0,     "",       "P 2 3", "", "P 2 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 28,"T^1"   , "342201","012201120450234423531504153045315342", int3x3(1)),
    
    SKSpaceGroupSetting(490,196,48,0,     "",       "F 2 3", "", "F 2 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 28,"T^2"   , "342201","012201120450234423531504153045315342", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(491,197,24,0,     "",       "I 2 3", "", "I 2 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 28,"T^3"   , "342201","012201120450234423531504153045315342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(492,198,12,0,     "",      "P 21 3", "", "P 2ac 2ab 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 28,"T^4"   , "I4B201","012201120N58BI44BIS3=58N=S38N53=SI4B", int3x3(1)),
    
    SKSpaceGroupSetting(493,199,24,0,     "",      "I 21 3", "", "I 2b 2c 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 28,"T^5"   , "3N2201","0122011204S023NN235I1S0415I04SI153N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(494,200,24,0,     "",      "P m -3", "", "-P 2 2 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 29,"Th^1"  , "342201345","012201120450234423531504153045315342", int3x3(1)),
    
    SKSpaceGroupSetting(495,201,24,1,  "abc",      "P n -3", "", "P 2 2 3 -1n",false, int3(6,6,6), Symmorphicity::hemisymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 29,"Th^2"  , "342201INS","012201120450234423531504153045315342", int3x3(1)),
    SKSpaceGroupSetting(496,201,24,2,  "abc",      "P n -3", "", "-P 2ab 2bc 3",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 29,"Th^2"  , "IN2201345","012201120NS02INN2ISI1S0N1SI0NSI1SIN2", int3x3(1)),
    
    SKSpaceGroupSetting(497,202,96,0,     "",      "F m -3", "", "-F 2 2 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 29,"Th^3"  , "342201345","012201120450234423531504153045315342", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(498,203,96,1,  "abc",      "F d -3", "", "F 2 2 3 -1d",false, int3(3,3,3), Symmorphicity::hemisymmorphic,false,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 29,"Th^4"  , "342201KPU","012201120450234423531504153045315342", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(499,203,96,2,  "abc",      "F d -3", "", "-F 2uv 2vw 3",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 29,"Th^4"  , "KP2201345","012201120PU02KPP2KUK1U0P1UK0PUK1UKP2", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(500,204,48,0,     "",      "I m -3", "", "-I 2 2 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 29,"Th^5"  , "342201345","012201120450234423531504153045315342", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(501,205,24,0,     "",      "P a -3", "", "-P 2ac 2ab 3",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 29,"Th^6"  , "I4B201345","012201120N58BI44BIS3=58N=S38N53=SI4B", int3x3(1)),
    
    SKSpaceGroupSetting(502,206,48,0,     "",      "I a -3", "", "-I 2b 2c 3",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 29,"Th^7"  , "3N2201345","0122011204S023NN235I1S0415I04SI153N2", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(503,207,24,0,     "",     "P 4 3 2", "", "P 4 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 30,"O^1"   , "402201","012051024213510402132201120450234423531504153045315342105435240543321354", int3x3(1)),
    
    SKSpaceGroupSetting(504,208,24,0,     "",    "P 42 3 2", "", "P 4n 2 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 30,"O^2"   , "N8B201","0128S=8BNB=IS=8N8B=IB201120450234423531504153045315342=8SNISBN8SNIIB=ISN", int3x3(1)),
    
    SKSpaceGroupSetting(505,209,96,0,     "",     "F 4 3 2", "", "F 4 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 30,"O^3"   , "402201","012051024213510402132201120450234423531504153045315342105435240543321354", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(506,210,96,0,     "",    "F 41 3 2", "", "F 4d 2 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 30,"O^4"   , "P:D201","012:U?:DPD?KU?:P:D?KD201120450234423531504153045315342?:UPKUDP:UPKKD?KUP", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(507,211,48,0,     "",     "I 4 3 2", "", "I 4 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 30,"O^5"   , "402201","012051024213510402132201120450234423531504153045315342105435240543321354", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(508,212,24,0,     "",    "P 43 3 2", "", "P 4acd 2ab 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 30,"O^6"   , "R:G201","012<X?:GRDAMX?<R:GAMD201120N58BI44BIS3=58N=S38N53=SI4B?<XPKUGR:UPKMDAKUP", int3x3(1)),
    
    SKSpaceGroupSetting(509,213,24,0,     "",    "P 41 3 2", "", "P 4bd 2ab 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 30,"O^7"   , "P<D201","012:UA<DPG?KUA:P<D?KG201120N58BI44BIS3=58N=S38N53=SI4BA:URMXDP<XRMKG?MXR", int3x3(1)),
    
    SKSpaceGroupSetting(510,214,48,0,     "",    "I 41 3 2", "", "I 4bd 2c 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 30,"O^8"   , "P<D201","012:UA:GRDAMUA:P<D?KG2011204S023NN235I1S0415I04SI153N2?<XPKUDP<UPKKG?KUP", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(511,215,24,0,     "",    "P -4 3 m", "", "P -4 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 31,"Td^1"  , "135201","012324351540243135405201120450234423531504153045315342432102513210054021", int3x3(1)),
    
    SKSpaceGroupSetting(512,216,96,0,     "",    "F -4 3 m", "", "F -4 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 31,"Td^2"  , "135201","012324351540243135405201120450234423531504153045315342432102513210054021", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(513,217,48,0,     "",    "I -4 3 m", "", "I -4 2 3",false, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 31,"Td^3"  , "135201","012324351540243135405201120450234423531504153045315342432102513210054021", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(514,218,24,0,     "",    "P -4 3 n", "", "P -4n 2 3",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 31,"Td^4"  , "=IS201","012IBNIS=SN8BNI=ISN8S201120450234423531504153045315342NIB=8BS=IB=88SN8B=", int3x3(1)),
    
    SKSpaceGroupSetting(515,219,96,0,     "",    "F -4 3 c", "", "F -4a 2 3",false, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 31,"Td^5"  , "=35201","012I24I51S40B43=35N05201120450234423531504153045315342N32=02S13B10854821", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(516,220,48,0,     "",    "I -4 3 d", "", "I -4bd 2c 3",false, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 31,"Td^6"  , "?MU201","012KDRKXAUR<DRK?MUP:X2011204S023NN235I1S0415I04SI153N2PMG?:DU?MD?::XP:D?", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(517,221,48,0,     "",    "P m -3 m", "", "-P 4 2 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 32,"Oh^1"  , "402201345","012051024213510402132201120450234423531504153045315342105435240543321354", int3x3(1)),
    
    SKSpaceGroupSetting(518,222,48,1,  "abc",    "P n -3 n", "", "P 4 2 3 -1n",false, int3(6,6,6), Symmorphicity::hemisymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 32,"Oh^2"  , "402201INS","012051024213510402132201120450234423531504153045315342105435240543321354", int3x3(1)),
    SKSpaceGroupSetting(519,222,48,2,  "abc",    "P n -3 n", "", "-P 4a 2bc 3",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 32,"Oh^2"  , "N02201345","0120S102N21IS10N021I2201120NS02INN2ISI1S0N1SI0NSI1SIN210SNIS2N0SNII21ISN", int3x3(1)),
    
    SKSpaceGroupSetting(520,223,48,0,     "",    "P m -3 n", "", "-P 4n 2 3",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 32,"Oh^3"  , "N8B201345","0128S=8BNB=IS=8N8B=IB201120450234423531504153045315342=8SNISBN8SNIIB=ISN", int3x3(1)),
    
    SKSpaceGroupSetting(521,224,48,1,  "abc",    "P n -3 m", "", "P 4n 2 3 -1n",false, int3(6,6,6), Symmorphicity::asymmorphic,false,  Centring::primitive,std::vector<int3>{int3(0,0,0)}, 32,"Oh^4"  , "N8B201INS","0128S=8BNB=IS=8N8B=IB201120450234423531504153045315342=8SNISBN8SNIIB=ISN", int3x3(1)),
    SKSpaceGroupSetting(522,224,48,2,  "abc",    "P n -3 m", "", "-P 4bc 2bc 3",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::primitive,std::vector<int3>{int3(0,0,0)}, 32,"Oh^4"  , "48B201345","01285=8B4B=35=848B=3B201120NS02INN2ISI1S0N1SI0NSI1SIN2=85435B485433B=354", int3x3(1)),
    
    SKSpaceGroupSetting(523,225,192,0,    "",    "F m -3 m", "", "-F 4 2 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 32,"Oh^5"  , "402201345","012051024213510402132201120450234423531504153045315342105435240543321354", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(524,226,192,0,    "",    "F m -3 c", "", "-F 4a 2 3",true, int3(0,0,0), Symmorphicity::hemisymmorphic,true,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 32,"Oh^6"  , "N02201345","012851824B13S10N02=32201120450234423531504153045315342=05N35B40S43I21I54", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(525,227,192,1, "abc",    "F d -3 m", "", "F 4d 2 3 -1d",false, int3(3,3,3), Symmorphicity::asymmorphic,false,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 32,"Oh^7"  , "P:D201KPU","012:U?:DPD?KU?:P:D?KD201120450234423531504153045315342?:UPKUDP:UPKKD?KUP", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(526,227,192,2, "abc",    "F d -3 m", "", "-F 4vw 2vw 3",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 32,"Oh^7"  , "4:D201345","012:5?:D4D?35?:4:D?3D201120PU02KPP2KUK1U0P1UK0PUK1UKP2?:5435D4:5433D?354", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(527,228,192,1, "abc",    "F d -3 c", "", "F 4d 2 3 -1ad",false, int3(3,3,-3), Symmorphicity::asymmorphic,false,  Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 32,"Oh^8"  , "P:D201KPX","012:U?:DPD?KU?:P:D?KD201120450234423531504153045315342?:UPKUDP:UPKKD?KUP", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    SKSpaceGroupSetting(528,228,192,2, "abc",    "F d -3 c", "", "-F 4ud 2vw 3",true, int3(0,0,0), Symmorphicity::asymmorphic,true, Centring::face,     std::vector<int3>{int3(0,0,0), int3(0,6,6), int3(6,0,6), int3(6,6,0)}, 32,"Oh^8"  , "4:G201345","012:5A:G4DA35?<4:G?3G201120PU02KPP2KUK1U0P1UK0PUK1UKP2?<5N35D4<S433DAI54", int3x3(int3(-1, 1, 1), int3(1, -1, 1), int3(1, 1, -1))),
    
    SKSpaceGroupSetting(529,229,96,0,     "",    "I m -3 m", "", "-I 4 2 3",true, int3(0,0,0), Symmorphicity::symmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 32,"Oh^9"  , "402201345","012051024213510402132201120450234423531504153045315342105435240543321354", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
    
    SKSpaceGroupSetting(530,230,96,0,     "",    "I a -3 d", "", "-I 4bd 2c 3",true, int3(0,0,0), Symmorphicity::asymmorphic,true,  Centring:: body,     std::vector<int3>{int3(0,0,0), int3(6,6,6)}, 32,"Oh^10" , "P<D201345","012:UA:GRDAMUA:P<D?KG2011204S023NN235I1S0415I04SI153N2?<XPKUDP<UPKKG?KUP", int3x3(int3(0, 1, 1), int3(1, 0, 1), int3(1, 1, 0))),
};

const std::vector<std::vector<int>> SKSpaceGroup::spaceGroupHallData =
{
  {0},
  {1},
  {2},
  {3, 4, 5},
  {6, 7, 8},
  {9, 10, 11, 12, 13, 14, 15, 16, 17},
  {18, 19, 20},
  {21, 22, 23, 24, 25, 26, 27, 28, 29},
  {30, 31, 32, 33, 34, 35, 36, 37, 38},
  {39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56},
  {57, 58, 59},
  {60, 61, 62},
  {63, 64, 65, 66, 67, 68, 69, 70, 71},
  {72, 73, 74, 75, 76, 77, 78, 79, 80},
  {81, 82, 83, 84, 85, 86, 87, 88, 89},
  {90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107},
  {108},
  {109, 110, 111},
  {112, 113, 114},
  {115},
  {116, 117, 118},
  {119, 120, 121},
  {122},
  {123},
  {124},
  {125, 126, 127},
  {128, 129, 130, 131, 132, 133},
  {134, 135, 136},
  {137, 138, 139, 140, 141, 142},
  {143, 144, 145, 146, 147, 148},
  {149, 150, 151, 152, 153, 154},
  {155, 156, 157, 158, 159, 160},
  {161, 162, 163},
  {164, 165, 166, 167, 168, 169},
  {170, 171, 172},
  {173, 174, 175},
  {176, 177, 178, 179, 180, 181},
  {182, 183, 184},
  {185, 186, 187, 188, 189, 190},
  {191, 192, 193, 194, 195, 196},
  {197, 198, 199, 200, 201, 202},
  {203, 204, 205, 206, 207, 208},
  {209, 210, 211},
  {212, 213, 214},
  {215, 216, 217},
  {218, 219, 220},
  {221, 222, 223, 224, 225, 226},
  {227},
  {229, 228}, // second origin as default
  {230, 231, 232},
  {234, 233, 236, 235, 238, 237}, // second origin as default
  {239, 240, 241, 242, 243, 244},
  {245, 246, 247, 248, 249, 250},
  {251, 252, 253, 254, 255, 256},
  {257, 258, 259, 260, 261, 262},
  {263, 264, 265},
  {266, 267, 268},
  {269, 270, 271, 272, 273, 274},
  {275, 276, 277},
  {279, 278, 281, 280, 283, 282}, // second origin as default
  {284, 285, 286, 287, 288, 289},
  {290, 291},
  {292, 293, 294, 295, 296, 297},
  {298, 299, 300, 301, 302, 303},
  {304, 305, 306, 307, 308, 309},
  {310, 311, 312},
  {313, 314, 315},
  {316, 317, 318, 319, 320, 321},
  {323, 322, 325, 324, 327, 326, 329, 328, 331, 330, 333, 332}, // second origin as default
  {334},
  {336, 335}, // second origin as default
  {337},
  {338, 339, 340},
  {341, 342},
  {343, 344, 345, 346, 347, 348},
  {349},
  {350},
  {351},
  {352},
  {353},
  {354},
  {355},
  {356},
  {357},
  {358},
  {360, 359}, // second origin as default
  {362, 361}, // second origin as default
  {363},
  {365, 364}, // second origin as default
  {366},
  {367},
  {368},
  {369},
  {370},
  {371},
  {372},
  {373},
  {374},
  {375},
  {376},
  {377},
  {378},
  {379},
  {380},
  {381},
  {382},
  {383},
  {384},
  {385},
  {386},
  {387},
  {388},
  {389},
  {390},
  {391},
  {392},
  {393},
  {394},
  {395},
  {396},
  {397},
  {398},
  {399},
  {400},
  {401},
  {403, 402}, // second origin as default
  {405, 404}, // second origin as default
  {406},
  {407},
  {409, 408}, // second origin as default
  {411, 410}, // second origin as default
  {412},
  {413},
  {415, 414}, // second origin as default
  {417, 416}, // second origin as default
  {418},
  {419},
  {421, 420}, // second origin as default
  {423, 422}, // second origin as default
  {424},
  {425},
  {427, 426}, // second origin as default
  {429, 428}, // second origin as default
  {430},
  {431},
  {432},
  {433, 434},
  {435},
  {436, 437},
  {438},
  {439},
  {440},
  {441},
  {442},
  {443},
  {444, 445},
  {446},
  {447},
  {448},
  {449},
  {450, 451},
  {452, 453},
  {454},
  {455},
  {456},
  {457},
  {458, 459},
  {460, 461},
  {462},
  {463},
  {464},
  {465},
  {466},
  {467},
  {468},
  {469},
  {470},
  {471},
  {472},
  {473},
  {474},
  {475},
  {476},
  {477},
  {478},
  {479},
  {480},
  {481},
  {482},
  {483},
  {484},
  {485},
  {486},
  {487},
  {488},
  {489},
  {490},
  {491},
  {492},
  {493},
  {494},
  {496, 495}, // second origin as default
  {497},
  {499, 498}, // second origin as default
  {500},
  {501},
  {502},
  {503},
  {504},
  {505},
  {506},
  {507},
  {508},
  {509},
  {510},
  {511},
  {512},
  {513},
  {514},
  {515},
  {516},
  {517},
  {519, 518}, // second origin as default
  {520},
  {522, 521}, // second origin as default
  {523},
  {524},
  {526, 525},  // second origin as default
  {527, 528},
  {529},
  {530}
};
