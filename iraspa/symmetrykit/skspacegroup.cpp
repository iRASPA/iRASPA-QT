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
#include "skpointgroup.h"

SKSpaceGroup::SKSpaceGroup(int HallNumber)
{
  _spaceGroupSetting = SKSpaceGroupDataBase::spaceGroupData[HallNumber];
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
    QString storedSpaceGroupString = SKSpaceGroupDataBase::spaceGroupData[i].HallString().simplified().toLower();

    if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
    {
      return i;
    }

    storedSpaceGroupString = SKSpaceGroupDataBase::spaceGroupData[i].HallString().simplified().toLower().remove(' ');
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
    QString storedSpaceGroupString = SKSpaceGroupDataBase::spaceGroupData[i].HMString().simplified().toLower();

    if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
    {
      return i;
    }

    if(SKSpaceGroupDataBase::spaceGroupData[i].qualifier() == "H")
    {
      storedSpaceGroupString = storedSpaceGroupString.replace(0,1,"h");
      if(SKSpaceGroup::matchSpacegroup(spaceSearchGroupString, storedSpaceGroupString))
      {
        return i;
      }
    }

    storedSpaceGroupString = SKSpaceGroupDataBase::spaceGroupData[i].HMString().simplified().toLower().remove(' ');
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
    std::vector<int> hall_numbers = SKSpaceGroupDataBase::spaceGroupHallData[number];
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

  SKSpaceGroupSetting setting = SKSpaceGroupDataBase::spaceGroupData[HallNumber];
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
  SKSpaceGroupSetting setting = SKSpaceGroupDataBase::spaceGroupData[HallNumber];
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

std::optional<int> SKSpaceGroup::findSpaceGroupGroup(double3x3 unitCell, std::vector<std::tuple<double3, int, double> > atoms, bool allowPartialOccupancies, double symmetryPrecision = 1e-2)
{
  std::vector<std::tuple<double3, int, double> > reducedAtoms{};

  if(allowPartialOccupancies)
  {
    reducedAtoms = atoms;
  }
  else
  {
    std::map<int, int> histogram{};
    for(const std::tuple<double3, int, double> &atom: atoms)
    {
      histogram[std::get<1>(atom)] = histogram[std::get<1>(atom)] + 1;
    }
    std::map<int,int>::iterator index = std::min_element(histogram.begin(), histogram.end(),
                                       [](const auto& l, const auto& r) { return l.second < r.second; });
    int leastOccuringAtomType = index->first;

    std::copy_if (atoms.begin(), atoms.end(), std::back_inserter(reducedAtoms), [leastOccuringAtomType](std::tuple<double3, int, double> a){return std::get<1>(a) == leastOccuringAtomType;} );
  }

  double3x3 smallestUnitCell = SKSymmetryCell::findSmallestPrimitiveCell(reducedAtoms, atoms, unitCell, allowPartialOccupancies, symmetryPrecision);

  std::optional<double3x3> primitiveDelaunayUnitCell = SKSymmetryCell::computeDelaunayReducedCell(smallestUnitCell, symmetryPrecision);

  if(primitiveDelaunayUnitCell)
  {
    SKPointSymmetrySet latticeSymmetries = SKSymmetryCell::findLatticeSymmetry(*primitiveDelaunayUnitCell, symmetryPrecision);

    std::vector<std::tuple<double3, int, double> >positionInPrimitiveCell = SKSymmetryCell::trim(atoms, unitCell, *primitiveDelaunayUnitCell, allowPartialOccupancies, symmetryPrecision);

    SKSymmetryOperationSet spaceGroupSymmetries = SKSpaceGroup::findSpaceGroupSymmetry(unitCell, positionInPrimitiveCell, positionInPrimitiveCell, latticeSymmetries, allowPartialOccupancies, symmetryPrecision);

    SKPointSymmetrySet pointSymmetry = SKPointSymmetrySet(spaceGroupSymmetries.rotations());

    std::optional<SKPointGroup> pointGroup = SKPointGroup(pointSymmetry);

    if(pointGroup)
    {
      // Use the axes directions of the Laue group-specific symmetry as a new basis
      std::optional<SKTransformationMatrix> Mprime = pointGroup->constructAxes(spaceGroupSymmetries.rotations());

      if(Mprime)
      {
        // adjustment of (M',0) to (M,0) for certain combination of Laue and centring types
        switch(pointGroup->laue())
        {
        case Laue::laue_1:
        {
          SKSymmetryCell::createFromUnitCell(*primitiveDelaunayUnitCell * Mprime->int3x3);
          std::optional<std::pair<SKSymmetryCell, SKTransformationMatrix >> symmetryCell = SKSymmetryCell::createFromUnitCell(*primitiveDelaunayUnitCell * Mprime->int3x3).computeReducedNiggliCellAndChangeOfBasisMatrix();
          if(!symmetryCell)
          {
            return std::nullopt;
          }
          Mprime = std::get<1>(*symmetryCell);
        }
        case Laue::laue_2m:
        {
          // Change the basis for this monoclinic centrosymmetric point group using Delaunay reduction in 2D (algorithm of Atsushi Togo used)
          // The unique axis is chosen as b, choose shortest a, c lattice vectors (|a| < |c|)
          std::optional<double3x3> computedDelaunayReducedCell2D = SKSymmetryCell::computeDelaunayReducedCell2D(*primitiveDelaunayUnitCell * Mprime->int3x3, symmetryPrecision);
          if(!computedDelaunayReducedCell2D)
          {
            return std::nullopt;
          }
          Mprime = SKTransformationMatrix(primitiveDelaunayUnitCell->inverse() *  *computedDelaunayReducedCell2D);
        }
        default:
          break;
        }

        Centring centering = pointGroup->computeCentering(*Mprime);

        SKTransformationMatrix correctedBasis = pointGroup->computeBasisCorrection(*Mprime, centering);

        double3x3 primitiveLattice = *primitiveDelaunayUnitCell * correctedBasis.int3x3;

        // transform the symmetries (rotation and translation) from the primtive cell to the conventional cell
        // the centering is used to add the additional translations
        SKSymmetryOperationSet symmetryInConventionalCell = spaceGroupSymmetries.changedBasis(correctedBasis).addingCenteringOperations(centering);

        for(int i=0;i<=230;i++)
        {
          int HallNumber = SKSpaceGroupDataBase::spaceGroupHallData[i].front();
          if(SKSpaceGroupDataBase::spaceGroupData[HallNumber].pointGroupNumber() == pointGroup->number())
          {
            std::optional<std::pair<double3, SKRotationalChangeOfBasis>> foundSpaceGroup = SKSpaceGroup::matchSpaceGroup(HallNumber, primitiveLattice, centering, symmetryInConventionalCell.operations(), symmetryPrecision);
            if(foundSpaceGroup)
            {
              return HallNumber;
            }
          }
        }
      }
    }
  }
  return std::nullopt;
}


std::optional<std::pair<double3, SKRotationalChangeOfBasis>> SKSpaceGroup::matchSpaceGroup(int HallNumber, double3x3 lattice, Centring centering, std::vector<SKSeitzMatrix> seitzMatrices, double symmetryPrecision = 1e-2)
{
  int pointGroupNumber = SKSpaceGroupDataBase::spaceGroupData[HallNumber].pointGroupNumber();
  switch(SKPointGroup::pointGroupData[pointGroupNumber].holohedry())
  {
  case Holohedry::none:
     break;
  case Holohedry::triclinic:
  case Holohedry::tetragonal:
  case Holohedry::trigonal:
  case Holohedry::hexagonal:
  case Holohedry::cubic:
  {
    std::optional<double3> originShift = getOriginShift(HallNumber, centering, SKRotationalChangeOfBasis(SKRotationMatrix::identity), seitzMatrices, symmetryPrecision);
    if(originShift)
    {
      return std::make_pair(*originShift, SKRotationalChangeOfBasis(SKRotationMatrix::identity));
    }
    return std::nullopt;
  }
  case Holohedry::monoclinic:
    for(const SKRotationalChangeOfBasis& changeOfMonoclinicCentering: SKRotationalChangeOfBasis::changeOfMonoclinicCentering)
    {
      std::optional<double3> originShift = getOriginShift(HallNumber, centering, changeOfMonoclinicCentering, seitzMatrices, symmetryPrecision);
      if(originShift)
      {
        return std::make_pair(*originShift, SKRotationalChangeOfBasis(SKRotationMatrix::identity));
      }
    }
    return std::nullopt;
    case Holohedry::orthorhombic:
      for(const SKRotationalChangeOfBasis& changeOfOrthorhombicCentering: SKRotationalChangeOfBasis::changeOfOrthorhombicCentering)
      {
        std::optional<double3> originShift = getOriginShift(HallNumber, centering, changeOfOrthorhombicCentering, seitzMatrices, symmetryPrecision);
        if(originShift)
        {
          return std::make_pair(*originShift, SKRotationalChangeOfBasis(SKRotationMatrix::identity));
        }
      }
      return std::nullopt;
  }

  return std::nullopt;
}

std::optional<double3> SKSpaceGroup::getOriginShift(int HallNumber, Centring centering, SKRotationalChangeOfBasis changeOfBasis, std::vector<SKSeitzMatrix> seitzMatrices, double symmetryPrecision = 1e-2)
{
  int3x3 translations = int3x3();
  double3x3 translationsnew = double3x3();

  SKSpaceGroup dataBaseSpaceGroup = SKSpaceGroup(HallNumber);
  //dataBaseSpaceGroup.spaceGroupSetting().encodedGenerators();
    /*



        var dataBaseSpaceGroupGenerators = SKSeitzIntegerMatrix.SeitzMatrices(generatorEncoding: dataBaseSpaceGroup.spaceGroupSetting.encodedGenerators)

        // apply change-of-basis to generators
        for i in 0..<dataBaseSpaceGroupGenerators.count
        {
          dataBaseSpaceGroupGenerators[i] = changeOfBasis * dataBaseSpaceGroupGenerators[i]
        }

        // apply change-of-basis to lattice translations
        var spaceGroupLatticeTranslations = dataBaseSpaceGroup.spaceGroupSetting.latticeTranslations
        for i in 0..<spaceGroupLatticeTranslations.count
        {
          spaceGroupLatticeTranslations[i] = changeOfBasis * spaceGroupLatticeTranslations[i]
        }

        // apply change-of-basis to centring
        var dataBaseSpaceGroupCentering = dataBaseSpaceGroup.spaceGroupSetting.centring
        switch(dataBaseSpaceGroupCentering)
        {
        case .a_face, .b_face, .c_face:
          if spaceGroupLatticeTranslations[1].x == 0
          {
            dataBaseSpaceGroupCentering = .a_face
          }
          if spaceGroupLatticeTranslations[1].y == 0
          {
            dataBaseSpaceGroupCentering = .b_face
          }
          if spaceGroupLatticeTranslations[1].z == 0
          {
            dataBaseSpaceGroupCentering = .c_face
          }
          break
        default:
          break
        }

        // return if the centring is not equal to the spacegroup one
        if centering != dataBaseSpaceGroupCentering
        {
          return nil
        }

        // apply change-of-basis to the Seitz-matrices
        var dataBaseSpaceGroupSeitzMatrices: [SKSeitzIntegerMatrix] =  dataBaseSpaceGroup.spaceGroupSetting.SeitzMatricesWithoutTranslation
        for i in 0..<dataBaseSpaceGroupSeitzMatrices.count
        {
          dataBaseSpaceGroupSeitzMatrices[i] = changeOfBasis * dataBaseSpaceGroupSeitzMatrices[i]
        }

        for i in 0..<dataBaseSpaceGroupGenerators.count
        {
          guard let index: Int = seitzMatrices.firstIndex(where: {$0.rotation == dataBaseSpaceGroupGenerators[i].rotation}) else {return nil}
          translations[i] = SKSeitzIntegerMatrix(SeitzMatrx: seitzMatrices[index]).translation
          translationsnew[i] = seitzMatrices[index].translation
        }

        var transformation: SKTransformationMatrix = SKTransformationMatrix.identity
        switch(dataBaseSpaceGroupCentering)
        {
        case .primitive:
          transformation = SKTransformationMatrix.identity
        case .body:
          transformation = SKTransformationMatrix.primitiveToBodyCentered
        case .face:
          transformation = SKTransformationMatrix.primitiveToFaceCentered
        case .a_face:
          transformation = SKTransformationMatrix.primitiveToACentered
        case .b_face:
          transformation = SKTransformationMatrix.primitiveToBCentered
        case .c_face:
          transformation = SKTransformationMatrix.primitiveToCCentered
        case .r:
          transformation = SKTransformationMatrix.primitiveToRhombohedral
        case .h:
          transformation = SKTransformationMatrix.primitiveToHexagonal
        default:
          break
        }

        let changeToPrimitive: SKIntegerChangeOfBasis = SKIntegerChangeOfBasis(inversionTransformation: transformation)

        let r1: SKRotationMatrix = dataBaseSpaceGroupGenerators[0].rotation
        let r2: SKRotationMatrix = dataBaseSpaceGroupGenerators.count > 1 ? dataBaseSpaceGroupGenerators[1].rotation : SKRotationMatrix.identity
        let r3: SKRotationMatrix = dataBaseSpaceGroupGenerators.count > 2 ? dataBaseSpaceGroupGenerators[2].rotation : SKRotationMatrix.identity

        let t1: SKTransformationMatrix = changeToPrimitive * SKTransformationMatrix(r1 - SKRotationMatrix.identity)
        let t2: SKTransformationMatrix = changeToPrimitive * SKTransformationMatrix(r2 - SKRotationMatrix.identity)
        let t3: SKTransformationMatrix = changeToPrimitive * SKTransformationMatrix(r3 - SKRotationMatrix.identity)


        // m is a 9x3 matrix
        let m: RingMatrix = RingMatrix(Int3x3: [t1.int3x3,t2.int3x3,t3.int3x3])

        // The system M * cp = b (mod Z) can be solved by computing the Smith normal form D = PMQ.
        // b is the translation difference, cp the origin shift
        // D is a matrix in diagonal form with diagonal entries d1, . . . , dn.
        // P is square, 9x9, invertible matrix
        // Q is square, 3x3, invertible matrix
        let sol:  (P: RingMatrix, Q: RingMatrix, D: RingMatrix) = try m.SmithNormalForm()

        var b: Matrix = Matrix(rows: 9, columns: 1, repeatedValue: 0.0)
        for i in 0..<dataBaseSpaceGroupGenerators.count
        {
          let seitzMatrix: SKSeitzIntegerMatrix? = dataBaseSpaceGroupSeitzMatrices.filter{$0.rotation == dataBaseSpaceGroupGenerators[i].rotation}.first
          guard seitzMatrix != nil else {return nil}

          let transPrimitive: SIMD3<Double> = changeToPrimitive * translationsnew[i]

          let dataBaseTranslation: SIMD3<Int32>  = changeToPrimitive * dataBaseSpaceGroupGenerators[i].translation

          let translationDifference: SIMD3<Double> = fract(transPrimitive - SIMD3<Double>(dataBaseTranslation) / 24.0)
          b[3*i,0] = translationDifference.x
          b[3*i+1,0] = translationDifference.y
          b[3*i+2,0] = translationDifference.z
        }

        // v (9x1) =  P (9x9) x b (9,1)
        let v: Matrix = sol.P <*> b

        // The system P * b = v, v = [v1,...,vn] has solutions(mod Z) if and only if vi==0 whenever di=0
        if (sol.D[0,0] == 0 && abs(v[0,0] - rint(v[0,0])) > symmetryPrecision) ||
           (sol.D[1,1] == 0 && abs(v[1,0] - rint(v[1,0])) > symmetryPrecision) ||
           (sol.D[2,2] == 0 && abs(v[2,0] - rint(v[2,0])) > symmetryPrecision)
        {
          return nil
        }
        for i in 3..<9
        {
          if abs(v[i,0] - rint(v[i,0])) > symmetryPrecision
          {
            return nil
          }
        }

        var Dinv: Matrix = Matrix(rows: 3, columns: 9, repeatedValue: 0.0)
        for i in 0..<3
        {
          if (sol.D[i,i] != 0)
          {
            Dinv[i,i] = 1.0 / Double(sol.D[i,i])
          }
        }

        // sol.Q (3x3), T (3x9), sol.P (9x9), bm (9x1) -> (3x1)
        let cp: Matrix = (sol.Q <*> Dinv <*> sol.P) <*> b

        let originShift: SIMD3<Double> = fract(SIMD3<Double>(cp[0], cp[1], cp[2]))
        let basis: SKIntegerChangeOfBasis = SKIntegerChangeOfBasis(inverse: changeToPrimitive)
        return fract(changeOfBasis.inverse * (basis * originShift))
                    */
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
  spaceGroup._spaceGroupSetting = SKSpaceGroupDataBase::spaceGroupData[HallNumber];
  return stream;
}

