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

#include "skxyzparser.h"

SKXYZParser::SKXYZParser(QString fileContent, bool onlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped, LogReporting *log): SKParser(),
    _scanner(fileContent, charactersToBeSkipped), _onlyAsymmetricUnitCell(onlyAsymmetricUnitCell), _asMolecule(asMolecule), _log(log), _frame(std::make_shared<SKStructure>())
{
  _frame->kind = SKStructure::Kind::molecule;
}



bool SKXYZParser::startParsing()
{
  int lineNumber = 0;
  int numberOfAtoms = 0;
  QString scannedLine;
  double3x3 unitCell{};

  // skip first line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  if(scannedLine.isEmpty()) return false;

  // scan second line
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine);
  if(scannedLine.isEmpty())
  {
    if (_log)
    {
      _log->logMessage(LogReporting::ErrorLevel::error, "XYZ file near empty");
    }
    return false;
  }

  QString simplifiedLine = scannedLine.simplified().toLower();
  if(simplifiedLine.startsWith("lattice=\""))
  {
    simplifiedLine.remove(0,QString("lattice=\"").size());
    simplifiedLine.replace('\"',' ');
    qDebug() << "Yes, starts with Lattice";

    // read lattice vectors
    #if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
      QStringList termsScannedLined = simplifiedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    #else
      QStringList termsScannedLined = simplifiedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    #endif

    if(termsScannedLined.size()>=9)
    {
      double3x3 cell;
      bool succes = false;

      unitCell.ax = termsScannedLined[0].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the ax-cell coordinate" + termsScannedLined[0]);
          return false;
        }
      }

      unitCell.ay = termsScannedLined[1].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the ay-cell coordinate" + termsScannedLined[1]);
          return false;
        }
      }

      unitCell.az = termsScannedLined[2].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the az-cell coordinate" + termsScannedLined[2]);
          return false;
        }
      }

      unitCell.bx = termsScannedLined[3].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the ax-cell coordinate" + termsScannedLined[3]);
          return false;
        }
      }

      unitCell.by = termsScannedLined[4].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the ay-cell coordinate" + termsScannedLined[4]);
          return false;
        }
      }

      unitCell.bz = termsScannedLined[5].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the az-cell coordinate" + termsScannedLined[5]);
          return false;
        }
      }

      unitCell.cx = termsScannedLined[6].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the ax-cell coordinate" + termsScannedLined[6]);
          return false;
        }
      }

      unitCell.cy = termsScannedLined[7].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::warning, "Count not parse the ay-cell coordinate" + termsScannedLined[7]);
          return false;
        }
      }

      unitCell.cz = termsScannedLined[8].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the az-cell coordinate" + termsScannedLined[8]);
          return false;
        }
      }

      _frame->kind = SKStructure::Kind::molecularCrystal;
      _frame->cell = std::make_shared<SKCell>(unitCell);
    }
    else
    {
      if (_log)
      {
        _log->logMessage(LogReporting::ErrorLevel::warning, "missing lattice data");
      }
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
      // read first lattice vector
      #if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        QStringList termsScannedLined = scannedLine.split(QRegExp("\\s+"), QString::SkipEmptyParts);
      #else
        QStringList termsScannedLined = scannedLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
      #endif
      if(termsScannedLined.size()<4)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "missing data on line " + QString::number(lineNumber));
        }
        return false;
      }

      numberOfAtoms += 1;
      std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();

      QString chemicalElement = termsScannedLined[0].simplified().toLower();
      chemicalElement.replace(0, 1, chemicalElement[0].toUpper());

      bool succes = false;
      position.x = termsScannedLined[1].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the x-coordinate " + termsScannedLined[1]);
          return false;
        }
      }
      position.y = termsScannedLined[2].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the y-coordinate " + termsScannedLined[2]);
          return false;
        }
      }
      position.z = termsScannedLined[3].toDouble(&succes);
      if(!succes)
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Count not parse the z-coordinate " + termsScannedLined[3]);
          return false;
        }
      }

      if (std::map<QString,int>::iterator index = PredefinedElements::atomicNumberData.find(chemicalElement); index != PredefinedElements::atomicNumberData.end())
      {
        atom->setPosition(position);
        atom->setElementIdentifier(index->second);
        atom->setDisplayName(chemicalElement);
      }
      else
      {
        if (_log)
        {
          _log->logMessage(LogReporting::ErrorLevel::error, "Unknown element " + chemicalElement);
          atom->setPosition(position);
          atom->setElementIdentifier(0);
          atom->setDisplayName("error");
        }
      }
      _frame->atoms.push_back(atom);
    }
  }
  _movies.push_back({_frame});
  return true;
}
