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

#include "skposcarlegacyparser.h"
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
#include "symmetrykitprotocols.h"
#include "skasymmetricatom.h"
#include "skelement.h"

SKPOSCARLegacyParser::SKPOSCARLegacyParser(QUrl url, bool proteinOnlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped, LogReporting *log): SKParser(),
  _scanner(url, charactersToBeSkipped), _proteinOnlyAsymmetricUnitCell(proteinOnlyAsymmetricUnitCell), _asMolecule(asMolecule), _log(log), _frame(std::make_shared<SKStructure>()), _spaceGroupHallNumber(1)
{
  _frame->kind = SKStructure::Kind::crystal;
}

SKPOSCARLegacyParser::SKPOSCARLegacyParser(QString content, bool proteinOnlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped, LogReporting *log): SKParser(),
  _scanner(content, charactersToBeSkipped), _proteinOnlyAsymmetricUnitCell(proteinOnlyAsymmetricUnitCell), _asMolecule(asMolecule), _log(log), _frame(std::make_shared<SKStructure>()), _spaceGroupHallNumber(1)
{
  _frame->kind = SKStructure::Kind::crystal;
}

void SKPOSCARLegacyParser::startParsing() noexcept(false)
{
  double3x3 unitCell{};
  double3x3 inverseUnitCell{};

  QString scannedLine;
  QStringList termsScannedLined{};

  // skip first line
  _scanner.scanLine(scannedLine);
  if(scannedLine.isEmpty()) {throw std::runtime_error("Empty file");}

  // skip first line
  _scanner.scanLine(scannedLine);
  if(scannedLine.isEmpty()) {throw std::runtime_error("Empty file");}

  // read first lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw std::runtime_error("Missing first lattice vector in POSCAR");}

  bool succes = false;
  unitCell.ax = termsScannedLined[0].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the x-coordinate of first lattice vector");}

  unitCell.ay = termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the y-coordinate of first lattice vector");}

  unitCell.az = termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the z-coordinate of first lattice vector");}


  // read second lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw std::runtime_error("Missing second lattice vector in POSCAR");}

  unitCell.bx = termsScannedLined[0].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the x-coordinate of second lattice vector");}

  unitCell.by = termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the y-coordinate of second lattice vector");}

  unitCell.bz = termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the z-coordinate of second lattice vector");}

  // read third lattice vector
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(termsScannedLined.size()<3) {throw std::runtime_error("Missing third lattice vector in POSCAR");}


  unitCell.cx = termsScannedLined[0].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the x-coordinate of third lattice vector");}

  unitCell.cy = termsScannedLined[1].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the y-coordinate of third lattice vector");}

  unitCell.cz = termsScannedLined[2].toDouble(&succes);
  if(!succes) {throw std::runtime_error("Could not parse the z-coordinate of third lattice vector");}

  _frame->cell = std::make_shared<SKCell>(unitCell);
  inverseUnitCell = unitCell.inverse();

  /*
  // read elements
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    QStringList elementList = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QStringList elementList = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    QStringList elementList = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(elementList.empty())
  {
    if (_log)
    {
      _log->logMessage(LogReporting::ErrorLevel::error, "list of elements is empty");
      return false;
    }
  }*/

  // read amount of atoms per element
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    QStringList amountList = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QStringList amountList = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    QStringList amountList = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif
  if(amountList.empty()) {throw std::runtime_error("List of amount of atoms is empty");}

  /*
  if(elementList.size() != amountList.size())
  {
    if (_log)
    {
      _log->logMessage(LogReporting::ErrorLevel::error, "the number of atoms list does not match the element list");
    }
  }*/

  // skip first line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
    QStringList directOrCartesian = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
  #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QStringList directOrCartesian = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
  #else
    QStringList directOrCartesian = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
  #endif

    /*
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
  }*/

  bool cartesian = false;
  if(directOrCartesian[0].toLower() == "cartesian")
  {
    cartesian = true;
  }

  for(int k=0; k<amountList.size(); k++)
  {
    int numberOfAtoms = amountList[k].toInt(&succes);

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
      if(termsScannedLined.empty()) {throw std::runtime_error("Error reading atoms");}


      atom->setElementIdentifier(k+1);
      atom->setDisplayName(QString::number(k+1));

      double3 position;
      position.x = termsScannedLined[0].toDouble(&succes);
      if(!succes) {throw std::runtime_error("Could not parse the x-coordinate");}

      position.y = termsScannedLined[1].toDouble(&succes);
      if(!succes) {throw std::runtime_error("Could not parse the y-coordinate");}

      position.z = termsScannedLined[2].toDouble(&succes);
      if(!succes) {throw std::runtime_error("Could not parse the z-coordinate");}


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

  _movies.push_back({_frame});
}
