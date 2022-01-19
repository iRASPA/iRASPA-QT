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

#include <QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  #include <QRegExp>
#else
  #include <QRegularExpression>
#endif
#include "skxyzparser.h"

SKXYZParser::SKXYZParser(QUrl url, bool proteinOnlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped): SKParser(),
    _scanner(url, charactersToBeSkipped), _proteinOnlyAsymmetricUnitCell(proteinOnlyAsymmetricUnitCell), _asMolecule(asMolecule), _frame(std::make_shared<SKStructure>())
{
  _frame->kind = SKStructure::Kind::molecule;
  _frame->displayName = _scanner.displayName();
}

void SKXYZParser::startParsing()
{
  int lineNumber = 0;
  int numberOfAtoms = 0;
  QString scannedLine;
  double3x3 unitCell{};

  // skip first line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  if(scannedLine.isEmpty()) {throw "Empty file";}

  // scan second line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  if(scannedLine.isEmpty()) {throw "XYZ file near empty";}

  QString simplifiedLine = scannedLine.simplified().toLower();
  if(simplifiedLine.startsWith("lattice=\""))
  {
    simplifiedLine.remove(0,QString("lattice=\"").size());
    simplifiedLine.replace('\"',' ');

    // read lattice vectors
    #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
      QStringList termsScannedLined = simplifiedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
      QStringList termsScannedLined = simplifiedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    #else
      QStringList termsScannedLined = simplifiedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    #endif

    if(termsScannedLined.size()>=9)
    {
      double3x3 cell;
      bool succes = false;

      unitCell.ax = termsScannedLined[0].toDouble(&succes);
      if(!succes) {throw "Count not parse the ax-cell coordinate";}

      unitCell.ay = termsScannedLined[1].toDouble(&succes);
      if(!succes) {throw "Count not parse the ay-cell coordinate";}

      unitCell.az = termsScannedLined[2].toDouble(&succes);
      if(!succes) {throw "Count not parse the az-cell coordinate";}


      unitCell.bx = termsScannedLined[3].toDouble(&succes);
      if(!succes) {throw "Count not parse the bx-cell coordinate";}

      unitCell.by = termsScannedLined[4].toDouble(&succes);
      if(!succes) {throw "Count not parse the by-cell coordinate";}

      unitCell.bz = termsScannedLined[5].toDouble(&succes);
      if(!succes) {throw "Count not parse the bz-cell coordinate";}


      unitCell.cx = termsScannedLined[6].toDouble(&succes);
      if(!succes) {throw "Count not parse the cx-cell coordinate";}

      unitCell.cy = termsScannedLined[7].toDouble(&succes);
      if(!succes) {throw "Count not parse the cy-cell coordinate";}

      unitCell.cz = termsScannedLined[8].toDouble(&succes);
      if(!succes) {throw "Count not parse the cz-cell coordinate";}

      _frame->drawUnitCell = false;
      if(!_asMolecule)
      {
        _frame->kind = SKStructure::Kind::molecularCrystal;
        _frame->drawUnitCell = true;
      }
      _frame->cell = std::make_shared<SKCell>(unitCell);
    }
  }

  while(!_scanner.isAtEnd())
  {
    // scan to first keyword
    _previousScanLocation = _scanner.scanLocation();
    if (_scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine) && !scannedLine.isEmpty())
    {
      lineNumber += 1;

      // handle reading in all atoms
      double3 position;

      // read chemical element, and x,y,z position
      #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
        QStringList termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      #elif (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QStringList termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
      #else
        QStringList termsScannedLined = scannedLine.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      #endif
      if(termsScannedLined.size()<4) {throw "Missing data";}

      numberOfAtoms += 1;
      std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();

      QString chemicalElement = termsScannedLined[0].simplified().toLower();
      chemicalElement.replace(0, 1, chemicalElement[0].toUpper());

      bool succes = false;
      position.x = termsScannedLined[1].toDouble(&succes);
      if(!succes) {throw "Could not parse the x-coordinate";}

      position.y = termsScannedLined[2].toDouble(&succes);
      if(!succes) {throw "Could not parse the y-coordinate";}

      position.z = termsScannedLined[3].toDouble(&succes);
      if(!succes) {throw "Could not parse the z-coordinate";}


      if (std::map<QString,int>::iterator index = PredefinedElements::atomicNumberData.find(chemicalElement); index != PredefinedElements::atomicNumberData.end())
      {
        atom->setPosition(position);
        atom->setElementIdentifier(index->second);
        atom->setUniqueForceFieldName(chemicalElement);
        atom->setDisplayName(chemicalElement);
      }
      else
      {
        atom->setPosition(position);
        atom->setElementIdentifier(0);
        atom->setDisplayName("Unknown");
      }
      _frame->atoms.push_back(atom);
    }
  }
  _movies.push_back({_frame});
}
