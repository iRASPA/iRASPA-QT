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

#include "skelfcarparser.h"
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

SKELFCARParser::SKELFCARParser(QUrl url, bool proteinOnlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped): SKParser(),
  _scanner(url, charactersToBeSkipped), _proteinOnlyAsymmetricUnitCell(proteinOnlyAsymmetricUnitCell),_asMolecule(asMolecule), _frame(std::make_shared<SKStructure>()), _spaceGroupHallNumber(1)
{
  _frame->kind = SKStructure::Kind::VASPDensityVolume;
  _frame->displayName = _scanner.displayName();
  _frame->drawUnitCell = true;
}

void SKELFCARParser::startParsing()
{
  double3x3 unitCell{};
  double3x3 inverseUnitCell{};

  QString scannedLine;
  QStringList termsScannedLined{};

  // skip first line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  if(scannedLine.isEmpty()) {throw "Empty file";}

  // skip first line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  if(scannedLine.isEmpty()) {throw "POSCAR file near empty";}

  // read first lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw "Missing first lattice vector in POSCAR";}

  bool succes = false;
  unitCell.ax = termsScannedLined[0].toDouble(&succes);
  if(!succes) {throw "Could not parse the x-coordinate of first lattice vector";}

  unitCell.ay = termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw "Could not parse the y-coordinate of first lattice vector";}

  unitCell.az = termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw "Could not parse the z-coordinate of first lattice vector";}


  // read second lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw "Missing second lattice vector in POSCAR";}

  unitCell.bx = termsScannedLined[0].toDouble(&succes);
  if(!succes) {throw "Could not parse the x-coordinate of second lattice vector";}

  unitCell.by = termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw "Could not parse the y-coordinate of second lattice vector";}

  unitCell.bz = termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw "Could not parse the z-coordinate of second lattice vector";}


  // read third lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw "Missing third lattice vector in POSCAR";}

  unitCell.cx = termsScannedLined[0].toDouble(&succes);
  if(!succes) {throw "Could not parse the x-coordinate of third lattice vector";}

  unitCell.cy = termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw "Could not parse the y-coordinate of third lattice vector";}

  unitCell.cz = termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw "Could not parse the z-coordinate of third lattice vector";}


  _frame->cell = std::make_shared<SKCell>(unitCell);
  inverseUnitCell = unitCell.inverse();

  // read elements
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    QStringList elementList = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QStringList elementList = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    QStringList elementList = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(elementList.empty()) {throw "List of elements is empty";}

  // read amount of atoms per element
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    QStringList amountList = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QStringList amountList = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    QStringList amountList = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(amountList.empty()) {throw "List of amount of atoms is empty";}

  if(elementList.size() != amountList.size()) {throw "The number of atoms list does not match the element list";}

  // skip first line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    QStringList directOrCartesian = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QStringList directOrCartesian = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    QStringList directOrCartesian = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(elementList[0].toLower() == "selective") // skip Selective dynamics line
  {
    _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
    #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
      directOrCartesian = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
      directOrCartesian = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    #else
      directOrCartesian = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    #endif
  }

  bool cartesian = false;
  if(directOrCartesian[0].toLower() == "cartesian")
  {
    cartesian = true;
  }

  for(int k=0; k<std::min(amountList.size(),elementList.size()); k++)
  {
    int numberOfAtoms = amountList[k].toInt(&succes);
    QString chemicalElementString = elementList[k];

    for(int i=0; i<numberOfAtoms;i++)
    {
      std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();

      // read atom
      _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
      #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
        termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
      #else
        termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      #endif
      if(termsScannedLined.empty()) {throw "Error reading atoms";}

      QString chemicalElement = chemicalElementString.simplified().toLower();
      chemicalElement.replace(0, 1, chemicalElement[0].toUpper());
      if (std::map<QString,int>::iterator index = PredefinedElements::atomicNumberData.find(chemicalElement); index != PredefinedElements::atomicNumberData.end())
      {
         atom->setElementIdentifier(index->second);
         atom->setDisplayName(chemicalElement);
      }
      else
      {
        atom->setElementIdentifier(0);
        atom->setDisplayName("Unknown");
      }

      double3 position;
      position.x = termsScannedLined[0].toDouble(&succes);
      if(!succes) {throw "Could not parse the x-coordinate";}

      position.y = termsScannedLined[1].toDouble(&succes);
      if(!succes) {throw "Could not parse the y-coordinate";}

      position.z = termsScannedLined[2].toDouble(&succes);
      if(!succes) {throw "Could not parse the z-coordinate";}

      // convert to fractional position if Cartesian coordinates are specified
      if(cartesian)
      {
        atom->setPosition(inverseUnitCell * position);
      }
      else
      {
        atom->setPosition(position);
      }

      _frame->atoms.push_back(atom);
    }
  }

  // skip line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw "Missing dimensions";}

  int3 dimensions;
  dimensions.x = termsScannedLined[0].toInt(&succes);
  if(!succes) {throw "Could not parse x-dimensions";}

  dimensions.y = termsScannedLined[1].toInt(&succes);
  if(!succes) {throw "Could not parse y-dimensions";}

  dimensions.z = termsScannedLined[2].toInt(&succes);
  if(!succes) {throw "Could not parse z-dimensions";}

  int numberOfElements = dimensions.x * dimensions.y * dimensions.z;
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
  _frame->dimensions = dimensions;
  _frame->range = std::pair(minimumValue, maximumValue);
  _frame->average = sum / (numberOfElements);
  _frame->variance = sumSquared / (numberOfElements - 1);
  _frame->byteData = QByteArray(reinterpret_cast<char*>(dataPoints.data()), dataPoints.size() * sizeof(float));

  _movies.push_back({_frame});
}
