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

#include "skgaussiancubeparser.h"
#include <QDebug>
#include <QString>
#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  #include <QRegExp>
#else
  #include <QRegularExpression>
#endif
#include <cmath>
#include <algorithm>
#include <qmath.h>
#include <limits>
#include "symmetrykitprotocols.h"
#include "skasymmetricatom.h"
#include "skelement.h"

SKGaussianCubeParser::SKGaussianCubeParser(QUrl url, bool onlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped): SKParser(),
  _scanner(url, charactersToBeSkipped), _onlyAsymmetricUnitCell(onlyAsymmetricUnitCell),_asMolecule(asMolecule), _frame(std::make_shared<SKStructure>()), _spaceGroupHallNumber(1)
{
  _frame->kind = SKStructure::Kind::GaussianCubeVolume;
  _frame->displayName = _scanner.displayName();
}

void SKGaussianCubeParser::startParsing()
{
  int3 dimensions;
  bool succes = false;
  double3x3 unitCell{};
  double3x3 inverseUnitCell{};

  QString scannedLine;
  QStringList termsScannedLined{};

  double bohrToAngstrom = 0.529177249;

  // skip first two lines
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  qDebug() << "CHECK1" << scannedLine;
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
qDebug() << "CHECK2" << scannedLine;

  // read number of atoms and origin
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  qDebug() << "CHECK" << scannedLine;
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.isEmpty()) {throw "Empty line in cube file (missing number of atoms and origin)";}

  int numberOfAtoms = termsScannedLined[0].toInt(&succes);
  double3 origin = double3(0.0, 0.0, 0.0);
  if(termsScannedLined.size()>=4)
  {
    double originX = termsScannedLined[1].toDouble(&succes);
    double originY = termsScannedLined[2].toDouble(&succes);
    double originZ = termsScannedLined[3].toDouble(&succes);
    origin = double3(originX, originY, originZ);
  }

  // read first lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw "Missing first lattice vector";}

  succes = false;
  dimensions.x = termsScannedLined[0].toInt(&succes);
  if(!succes) {throw "Could not read x-dimension";}

  succes = false;
  unitCell.ax = bohrToAngstrom * dimensions.x * termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw "Count not parse the x-coordinate of first lattice vector";}

  succes = false;
  unitCell.ay = bohrToAngstrom * dimensions.x * termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw "Count not parse the y-coordinate of first lattice vector";}

  succes = false;
  unitCell.az = bohrToAngstrom * dimensions.x * termsScannedLined[3].toDouble(&succes);
  if(!succes) {throw "Count not parse the z-coordinate of first lattice vector";}

  // read second lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw "Missing second lattice vector";}

  succes = false;
  dimensions.y = termsScannedLined[0].toInt(&succes);
  if(!succes) {throw "Could not read x-dimension";}

  succes = false;
  unitCell.bx = bohrToAngstrom * dimensions.y * termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw "Count not parse the x-coordinate of first lattice vector";}

  succes = false;
  unitCell.by = bohrToAngstrom * dimensions.y * termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw "Count not parse the y-coordinate of first lattice vector";}

  succes = false;
  unitCell.bz = bohrToAngstrom * dimensions.y * termsScannedLined[3].toDouble(&succes);
  if(!succes) {throw "Count not parse the z-coordinate of first lattice vector";}

  // read third lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw "Missing third lattice vector";}

  succes = false;
  dimensions.z = termsScannedLined[0].toInt(&succes);
  if(!succes) {throw "Could not read x-dimension";}

  succes = false;
  unitCell.cx = bohrToAngstrom * dimensions.z * termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw "Count not parse the x-coordinate of first lattice vector";}

  succes = false;
  unitCell.cy = bohrToAngstrom * dimensions.z * termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw "Count not parse the y-coordinate of first lattice vector";}

  succes = false;
  unitCell.cz = bohrToAngstrom * dimensions.z * termsScannedLined[3].toDouble(&succes);
  if(!succes) {throw "Count not parse the z-coordinate of first lattice vector";}

  // Create the unit cell
  _frame->cell = std::make_shared<SKCell>(unitCell);
  inverseUnitCell = (unitCell).inverse();

  // Read all the atoms
  for(int i=0; i<numberOfAtoms;i++)
  {
    _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
    #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
      termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
      termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    #else
      termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    #endif
    if(termsScannedLined.size()<5) {throw "Missing atom information";}

    int atomicNumber = termsScannedLined[0].toInt(&succes);
    if(!succes) {throw "Atomic number not an int";}

    std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
    atom->setElementIdentifier(atomicNumber);
    atom->setDisplayName(PredefinedElements::predefinedElements[atomicNumber]._chemicalSymbol);


    double3 position;
    succes = false;
    position.x = bohrToAngstrom * termsScannedLined[2].toDouble(&succes);
    position.y = bohrToAngstrom * termsScannedLined[3].toDouble(&succes);
    position.z = bohrToAngstrom * termsScannedLined[4].toDouble(&succes);

    atom->setPosition(inverseUnitCell * position);

    _frame->atoms.push_back(atom);
  }



  int numberOfElements = dimensions.x * dimensions.y * dimensions.z;
  qDebug() << "numberOfElements: " << numberOfElements;
  std::vector<float> dataPoints;
  dataPoints.resize(numberOfElements);
  float maximumValue = std::numeric_limits<float>::lowest();
  float minimumValue = std::numeric_limits<float>::max();
  double sum=0.0;
  double sumSquared=0.0;
  int index=0;
  for(int z=0;z<dimensions.z;z++) // Z is the outer loop
  {
    for(int y=0;y<dimensions.y;y++) // Y is the middle loop
    {
      for(int x=0;x<dimensions.x;x++) // X is the inner loop
      {
        double value;
        _scanner.scanDouble(value);
        dataPoints[index] = value;

        if(value>maximumValue) maximumValue = value;
        if(value<minimumValue) minimumValue = value;
        sum += value;
        sumSquared += value * value;

        index++;
      }
    }
  }

  _frame->origin = origin;
  _frame->dimensions = dimensions;
  _frame->range = std::pair(minimumValue, maximumValue);
  _frame->average = sum / (numberOfElements);
  _frame->variance = sumSquared / (numberOfElements - 1);
  _frame->byteData = QByteArray(reinterpret_cast<char*>(dataPoints.data()), dataPoints.size() * sizeof(float));

  _movies.push_back({_frame});
}
