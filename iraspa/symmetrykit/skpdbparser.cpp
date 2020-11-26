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

#include "skpdbparser.h"
#include <QDebug>
#include <cmath>
#include <qmath.h>
#include "symmetrykitprotocols.h"
#include "skasymmetricatom.h"
#include "skelement.h"

SKPDBParser::SKPDBParser(QString fileContent, bool onlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped): SKParser(),
  _scanner(fileContent, charactersToBeSkipped), _onlyAsymmetricUnitCell(onlyAsymmetricUnitCell),_asMolecule(asMolecule), _frame(std::make_shared<SKStructure>()), _spaceGroupHallNumber(1)
{
  _frame->kind = SKStructure::Kind::molecule;
}

void SKPDBParser::addFrameToStructure(size_t currentMovie, size_t currentFrame)
{
  if (_frame->atoms.size() > 0)
  {
    if (currentMovie >= _movies.size())
    {
      std::vector<std::shared_ptr<SKStructure>> movie = std::vector<std::shared_ptr<SKStructure>>();
      _movies.push_back(movie);
    }

    if (currentFrame >= _movies[currentMovie].size())
    {
      if(double(_numberOfAminoAcidAtoms)/double(_numberOfAtoms) > 0.5)
      {
        _frame->kind = SKStructure::Kind::protein;
        _frame->spaceGroupHallNumber = 1;
      }
      if(_cell && !_asMolecule)
      {
        _frame->cell = std::make_shared<SKCell>(*_cell);
        _frame->spaceGroupHallNumber = _spaceGroupHallNumber;
        if(double(_numberOfAminoAcidAtoms)/double(_numberOfAtoms) > 0.5)
        {
          _frame->kind = SKStructure::Kind::proteinCrystal;
        }
        else
        {
          _frame->kind = SKStructure::Kind::molecularCrystal;
        }
      }

      _movies[currentMovie].push_back(_frame);

      _frame = std::make_shared<SKStructure>();
      _frame->kind = SKStructure::Kind::molecule;
      _numberOfAminoAcidAtoms=0;
      _numberOfAtoms=0;
    }
  }
}

bool SKPDBParser::startParsing()
{

  int lineNumber = 0;
  int modelNumber = 0;
  size_t currentMovie = 0;
  size_t currentFrame = 0;

  while(!_scanner.isAtEnd())
  {
    QString scannedLine;

    // scan to first keyword
    _previousScanLocation = _scanner.scanLocation();
    if (_scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(), scannedLine) && !scannedLine.isEmpty())
    {
      lineNumber += 1;

      int length = scannedLine.size();

      if(length < 3) continue;
      QStringRef shortKeyword(&scannedLine, 0, 3);

      if(shortKeyword == "END")
      {
        addFrameToStructure(currentMovie,currentFrame);
        currentFrame += 1;
        continue;
      }

      if(shortKeyword == "TER")
      {
        addFrameToStructure(currentMovie,currentFrame);
        currentMovie += 1;
        continue;
      }

      if(length < 6) continue;
      QStringRef keyword(&scannedLine, 0, 6);


      if(keyword == "HEADER")
      {
        continue;
      }

      if(keyword == "AUTHOR")
      {
        continue;
      }

      if(keyword == "REVDAT")
      {
        continue;
      }

      if(keyword == "JRNL  ")
      {
        continue;
      }

      if(keyword == "REMARK")
      {
        continue;
      }

      if(keyword == "MODEL")
      {
        currentMovie = 0;

        if(length <= 10) continue;
        QStringRef modelString(&scannedLine, 6, length - 6);

        bool success = false;
        int integerValue = modelString.toInt(&success);
        if(success)
        {
          _frame = std::make_shared<SKStructure>();
          currentFrame = std::max(0, integerValue-1);
          currentFrame = modelNumber;
          modelNumber += 1;
        }
        continue;
      }

      if(keyword == "SCALE1")
      {
        continue;
      }

      if(keyword == "SCALE2")
      {
        continue;
      }

      if(keyword == "SCALE3")
      {
        continue;
      }

      if(keyword == "CRYST1")
      {
        QStringRef lengthAString(&scannedLine, 6, 9);
        QStringRef lengthBString(&scannedLine, 15, 9);
        QStringRef lengthCString(&scannedLine, 24, 9);
        bool succes = false;
        _a = lengthAString.toDouble(&succes);
        _b = lengthBString.toDouble(&succes);
        _c = lengthCString.toDouble(&succes);

        _alpha = 90.0;
        _beta = 90.0;
        _gamma = 90.0;
        if(scannedLine.size()>=54)
        {
          QStringRef alphaAngleString(&scannedLine, 33, 7);
          QStringRef betaAngleString(&scannedLine, 40, 7);
          QStringRef gammaAngleString(&scannedLine, 47, 7);
          _alpha = alphaAngleString.toDouble(&succes);
          _beta = betaAngleString.toDouble(&succes);
          _gamma = gammaAngleString.toDouble(&succes);
        }
        _cell = SKCell(_a, _b, _c, _alpha * M_PI/180.0, _beta*M_PI/180.0, _gamma*M_PI/180.0);
        if(!_asMolecule)
        {
          _frame->kind = SKStructure::Kind::molecularCrystal;
        }
        if(scannedLine.size()>=66)
        {
          QStringRef spaceGroupString(&scannedLine, 55, 11);

          if(_onlyAsymmetricUnitCell)
          {
            _spaceGroupHallNumber = 1;
          }
          else if(std::optional<int> spaceGroupHallNumber = SKSpaceGroup::HallNumberFromHMString(spaceGroupString.toString().simplified()))
          {
            _spaceGroupHallNumber = *spaceGroupHallNumber;
          }
        }
        continue;
      }

      if(keyword == "ORIGX1")
      {
        continue;
      }

      if(keyword == "ORIGX2")
      {
        continue;
      }

      if(keyword == "ORIGX3")
      {
        continue;
      }


      if(keyword == "ATOM  " || keyword == "HETATM")
      {
        _numberOfAtoms += 1;
        std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();


        QStringRef residueName(&scannedLine, 17, 3);

        QStringRef positionsX(&scannedLine, 30, 8);
        QStringRef positionsY(&scannedLine, 38, 8);
        QStringRef positionsZ(&scannedLine, 46, 8);
        QStringRef chemicalElementString(&scannedLine, 76, 2);

        double occupancy = 1.0;
        bool succes = false;
        QStringRef occupancyString(&scannedLine, 54, 6);

        occupancy = occupancyString.toDouble(&succes);
        if(succes)
        {
          atom->setOccupancy(occupancy);
        }

        double3 position;
        succes = false;
        position.x = positionsX.toDouble(&succes);
        position.y = positionsY.toDouble(&succes);
        position.z = positionsZ.toDouble(&succes);
        atom->setPosition(position);

        QString chemicalElement = chemicalElementString.toString().simplified().toLower();
        chemicalElement.replace(0, 1, chemicalElement[0].toUpper());
        if (std::map<QString,int>::iterator index = PredefinedElements::atomicNumberData.find(chemicalElement); index != PredefinedElements::atomicNumberData.end())
        {
           atom->setElementIdentifier(index->second);
           atom->setDisplayName(chemicalElement);
        }

        if(PredefinedElements::residueDefinitions.find(residueName.toString().toUpper()) != PredefinedElements::residueDefinitions.end())
        {
          _numberOfAminoAcidAtoms += 1;
        }

        _frame->atoms.push_back(atom);
        continue;
      }

    }
  }

  // add current frame in case last TER, ENDMDL, or END is missing
  addFrameToStructure(currentMovie,currentFrame);

  return true;
}
