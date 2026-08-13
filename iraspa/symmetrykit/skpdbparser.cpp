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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  #include <QStringRef>
#else
  #include<QStringView>
#endif
#include <cmath>
#include <qmath.h>
#include "symmetrykitprotocols.h"
#include "skasymmetricatom.h"
#include "skelement.h"
#include "sknucleotide.h"

SKPDBParser::SKPDBParser(QUrl url, bool proteinOnlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped, bool separatePolymerChains): SKParser(),
  _scanner(url, charactersToBeSkipped), _proteinOnlyAsymmetricUnitCell(proteinOnlyAsymmetricUnitCell), _asMolecule(asMolecule),
  _separatePolymerChains(separatePolymerChains), _frame(std::make_shared<SKStructure>()), _spaceGroupHallNumber(1)
{
  _frame->kind = SKStructure::Kind::molecule;
  _frame->displayName = _scanner.displayName();
}

QString SKPDBParser::pdbField(const QString &line, int location, int length)
{
  if(line.size() < location + length) return QString();
  return line.mid(location, length);
}

bool SKPDBParser::isWaterResidue(const QString &residueName)
{
  return residueName == QString("HOH") || residueName == QString("DOD") ||
         residueName == QString("WAT") || residueName == QString("H2O");
}

bool SKPDBParser::isSolventAgentResidue(const QString &residueName)
{
  static const std::set<QString> agents =
  {
    QString("SO4"), QString("PO4"), QString("GOL"), QString("EDO"), QString("MPD"), QString("PEG"),
    QString("PG4"), QString("ACT"), QString("ACY"), QString("DMS"), QString("TRS"), QString("MES"),
    QString("EPE"), QString("IMD"), QString("FMT"), QString("NA"), QString("K"), QString("MG"),
    QString("CA"), QString("ZN"), QString("MN"), QString("FE"), QString("NI"), QString("CU"),
    QString("CD"), QString("CL"), QString("BR"), QString("IOD"), QString("F"), QString("CO")
  };
  return agents.find(residueName) != agents.end();
}

// A cell of 1 Angstrom on a side with right angles is the PDB placeholder when there is no crystal.
bool SKPDBParser::isPlaceholderCell(double a, double b, double c, double alpha, double beta, double gamma)
{
  auto isOne = [](double value) {return std::fabs(value - 1.0) < 1.0e-3;};
  auto isRight = [](double value) {return std::fabs(value - 90.0) < 1.0e-3;};
  return isOne(a) && isOne(b) && isOne(c) && isRight(alpha) && isRight(beta) && isRight(gamma);
}

void SKPDBParser::noteResidueAtom(const std::shared_ptr<SKAsymmetricAtom> &atom)
{
  const QString residueName = atom->residueName().trimmed().toUpper();
  if(residueName.isEmpty()) return;

  const ResidueKey key{atom->chainIdentifier(), atom->residueSequenceNumber()};
  ResidueRecord &residue = _residues[key];
  residue.name = residueName;
  residue.water = isWaterResidue(residueName);
  residue.nucleotide = SKNucleotide::isNucleotideResidueName(residueName);

  const QString atomName = atom->displayName().trimmed().toUpper();
  if(atomName == QString("N"))
  {
    residue.hasNitrogen = true;
    residue.nitrogen = atom->position();
  }
  else if(atomName == QString("CA"))
  {
    residue.hasAlphaCarbon = true;
  }
  else if(atomName == QString("C"))
  {
    residue.hasCarbonyl = true;
    residue.carbonyl = atom->position();
  }
}

void SKPDBParser::parseSeqres(const QString &line)
{
  if(line.size() < 19) return;

  const QString chainField = pdbField(line, 11, 1);
  if(chainField.isEmpty()) return;
  const QChar chainIdentifier = chainField.at(0);

  for(int start = 19; start + 3 <= line.size(); start += 4)
  {
    const QString residueName = pdbField(line, start, 3).trimmed().toUpper();
    if(residueName.isEmpty() || isWaterResidue(residueName)) continue;

    if(PredefinedElements::knownAminoAcidResidueCodes.find(residueName) != PredefinedElements::knownAminoAcidResidueCodes.end() ||
       SKNucleotide::isNucleotideResidueName(residueName))
    {
      _polymerChains.insert(chainIdentifier);
      return;
    }
  }
}

void SKPDBParser::parseModres(const QString &line)
{
  if(line.size() < 15) return;

  const QString residueName = pdbField(line, 12, 3).trimmed().toUpper();
  if(!residueName.isEmpty())
  {
    _modifiedResidues.insert(residueName);
  }
}

SKStructure::Kind SKPDBParser::kindOfCurrentPart()
{
  int peptideResidues = 0;
  int nucleicResidues = 0;
  int waterResidues = 0;
  int otherResidues = 0;

  // std::map is ordered on (chain, sequence), which is the order needed for peptide-bond detection
  for(const auto &[key, residue] : _residues)
  {
    const bool declaredPolymer = _polymerChains.find(key.chain) != _polymerChains.end() &&
      (_modifiedResidues.find(residue.name) != _modifiedResidues.end() ||
       PredefinedElements::knownAminoAcidResidueCodes.find(residue.name) != PredefinedElements::knownAminoAcidResidueCodes.end() ||
       SKNucleotide::isNucleotideResidueName(residue.name));

    if(residue.water)
    {
      waterResidues += 1;
    }
    else if(residue.nucleotide || (declaredPolymer && SKNucleotide::isNucleotideResidueName(residue.name)))
    {
      nucleicResidues += 1;
    }
    else if((residue.hasNitrogen && residue.hasAlphaCarbon && residue.hasCarbonyl) ||
            (declaredPolymer && !isWaterResidue(residue.name) && !SKNucleotide::isNucleotideResidueName(residue.name)))
    {
      peptideResidues += 1;
    }
    else
    {
      otherResidues += 1;
    }
  }

  int peptideBonds = 0;
  const ResidueRecord *previous = nullptr;
  std::optional<QChar> previousChain{};
  for(const auto &[key, residue] : _residues)
  {
    if(previous && previousChain && key.chain == *previousChain &&
       previous->hasCarbonyl && residue.hasNitrogen)
    {
      if((previous->carbonyl - residue.nitrogen).length() < 2.0)
      {
        peptideBonds += 1;
      }
    }
    previous = &residue;
    previousChain = key.chain;
  }

  const bool isProtein = peptideResidues >= 2 && peptideBonds >= 1 && peptideResidues > otherResidues;
  if(isProtein)
  {
    _proteinDetected = true;
    return (_periodic && !_asMolecule) ? SKStructure::Kind::proteinCrystal : SKStructure::Kind::protein;
  }

  const bool isDNA = nucleicResidues >= 2 && nucleicResidues > otherResidues && nucleicResidues >= peptideResidues;
  if(isDNA)
  {
    _dnaDetected = true;
    return (_periodic && !_asMolecule) ? SKStructure::Kind::dnaCrystal : SKStructure::Kind::dna;
  }

  // fallback: atom-fraction heuristics for sparse residue metadata
  if(_numberOfAtoms > 0)
  {
    if(double(_numberOfAminoAcidAtoms)/double(_numberOfAtoms) > 0.5)
    {
      _proteinDetected = true;
      return (_periodic && !_asMolecule) ? SKStructure::Kind::proteinCrystal : SKStructure::Kind::protein;
    }
    if(double(_numberOfNucleotideAtoms)/double(_numberOfAtoms) > 0.5)
    {
      _dnaDetected = true;
      return (_periodic && !_asMolecule) ? SKStructure::Kind::dnaCrystal : SKStructure::Kind::dna;
    }
  }

  bool onlySolvent = waterResidues > 0 && peptideResidues == 0 && nucleicResidues == 0;
  if(onlySolvent)
  {
    for(const auto &[key, residue] : _residues)
    {
      Q_UNUSED(key);
      if(!residue.water && !isSolventAgentResidue(residue.name))
      {
        onlySolvent = false;
        break;
      }
    }
  }

  if(_proteinDetected && onlySolvent)
  {
    return (_periodic && !_asMolecule) ? SKStructure::Kind::proteinCrystalSolvent : SKStructure::Kind::molecule;
  }
  if(_dnaDetected && onlySolvent)
  {
    return (_periodic && !_asMolecule) ? SKStructure::Kind::dnaCrystal : SKStructure::Kind::dna;
  }

  return (_periodic && !_asMolecule) ? SKStructure::Kind::molecularCrystal : SKStructure::Kind::molecule;
}

void SKPDBParser::addFrameToStructure(size_t currentMovie, size_t currentFrame)
{
  if (currentMovie >= _movies.size())
  {
    std::vector<std::shared_ptr<SKStructure>> movie = std::vector<std::shared_ptr<SKStructure>>();
    _movies.push_back(movie);
  }

  if (currentFrame >= _movies[currentMovie].size())
  {
    if(_cell)
    {
      _frame->cell = std::make_shared<SKCell>(*_cell);
    }

    const SKStructure::Kind kind = kindOfCurrentPart();
    _frame->kind = kind;

    switch(kind)
    {
    case SKStructure::Kind::proteinCrystal:
    case SKStructure::Kind::proteinCrystalSolvent:
    case SKStructure::Kind::dnaCrystal:
      _frame->drawUnitCell = !_proteinOnlyAsymmetricUnitCell;
      _frame->spaceGroupHallNumber = _proteinOnlyAsymmetricUnitCell ? 1 : _spaceGroupHallNumber;
      _frame->periodic = true;
      break;
    case SKStructure::Kind::molecularCrystal:
      _frame->drawUnitCell = true;
      _frame->spaceGroupHallNumber = _spaceGroupHallNumber;
      _frame->periodic = true;
      break;
    default:
      _frame->drawUnitCell = false;
      _frame->spaceGroupHallNumber = 1;
      _frame->periodic = false;
      break;
    }

    _movies[currentMovie].push_back(_frame);

    _frame = std::make_shared<SKStructure>();
    _frame->atoms.clear();
    _frame->displayName = _scanner.displayName();
    _frame->kind = SKStructure::Kind::molecule;
    _numberOfAminoAcidAtoms=0;
    _numberOfNucleotideAtoms=0;
    _numberOfSolventAtoms=0;
    _numberOfAtoms=0;
    _residues.clear();
  }
}

void SKPDBParser::startParsing() noexcept(false)
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

      #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QStringRef shortKeyword(&scannedLine, 0, 3);
      #else
        QStringView shortKeyword(&scannedLine[0], 3);
      #endif

      if(shortKeyword == QString("TER"))
      {
        // TER marks the end of a polymer chain. Splitting on it puts each chain in its own movie;
        // with 'separate polymer chains' off, keep reading into the current structure.
        if(_separatePolymerChains && _frame->atoms.size() > 0)
        {
          addFrameToStructure(currentMovie,currentFrame);
          currentMovie += 1;
        }
        continue;
      }

      if(length < 6) continue;

      #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        QStringRef keyword(&scannedLine, 0, 6);
      #else
        QStringView keyword(&scannedLine[0], 6);
      #endif

      if(keyword == QString("HEADER"))
      {
        continue;
      }

      if(keyword == QString("AUTHOR"))
      {
        continue;
      }

      if(keyword == QString("REVDAT"))
      {
        continue;
      }

      if(keyword == QString("JRNL  "))
      {
        continue;
      }

      if(keyword == QString("REMARK"))
      {
        continue;
      }

      if(keyword == QString("EXPDTA"))
      {
        const QString experiment = scannedLine.mid(6).trimmed().toUpper();
        if(experiment.contains(QString("NMR")) || experiment.contains(QString("ELECTRON MICROSCOPY")) ||
           experiment.contains(QString("SOLUTION SCATTERING")) || experiment.contains(QString("THEORETICAL MODEL")))
        {
          _experimentIsNonPeriodic = true;
          _periodic = false;
        }
        continue;
      }

      if(keyword == QString("SEQRES"))
      {
        parseSeqres(scannedLine);
        continue;
      }

      if(keyword == QString("MODRES"))
      {
        parseModres(scannedLine);
        continue;
      }

      if(keyword == QString("MODEL"))
      {
        currentMovie = 0;

        if(length <= 10) continue;

        #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
          QStringRef modelString(&scannedLine, 6, length - 6);
        #else
          QStringView modelString(&scannedLine[6], length - 6);
        #endif

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
      if(keyword == QString("ENDMDL"))
      {
        // also frames with zero atoms are allowed in PDB movies from RASPA. This happens in grand-canonical ensembles at low fugacities.
        addFrameToStructure(currentMovie,currentFrame);
        currentFrame += 1;
        continue;
      }
      if(keyword == QString("SCALE1"))
      {
        continue;
      }

      if(keyword == QString("SCALE2"))
      {
        continue;
      }

      if(keyword == QString("SCALE3"))
      {
        continue;
      }

      if(keyword == QString("CRYST1"))
      {
        #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
          QStringRef lengthAString{};
          QStringRef lengthBString{};
          QStringRef lengthCString{};
        #else
          QStringView lengthAString{};
          QStringView lengthBString{};
          QStringView lengthCString{};
        #endif

        if(scannedLine.size()>=17)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            lengthAString = QStringRef(&scannedLine, 6, 9);
          #else
            lengthAString = QStringView(&scannedLine[6], 9);
          #endif
        }
        if(scannedLine.size()>=24)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            lengthBString = QStringRef(&scannedLine, 15, 9);
          #else
            lengthBString = QStringView(&scannedLine[15], 9);
          #endif
        }
        if(scannedLine.size()>=33)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            lengthCString = QStringRef(&scannedLine, 24, 9);
          #else
            lengthCString = QStringView(&scannedLine[24], 9);
          #endif
        }

        bool succes = false;
        _a = lengthAString.toDouble(&succes);
        _b = lengthBString.toDouble(&succes);
        _c = lengthCString.toDouble(&succes);

        _alpha = 90.0;
        _beta = 90.0;
        _gamma = 90.0;
        if(scannedLine.size()>=54)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QStringRef alphaAngleString{};
            QStringRef betaAngleString{};
            QStringRef gammaAngleString{};
          #else
            QStringView alphaAngleString{};
            QStringView betaAngleString{};
            QStringView gammaAngleString{};
          #endif
          if(scannedLine.size()>=40)
          {
            #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
              alphaAngleString = QStringRef(&scannedLine, 33, 7);
            #else
              alphaAngleString = QStringView(&scannedLine[33], 7);
            #endif
          }
          if(scannedLine.size()>=47)
          {
            #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
              betaAngleString = QStringRef(&scannedLine, 40, 7);
            #else
              betaAngleString = QStringView(&scannedLine[40], 7);
            #endif
          }
          if(scannedLine.size()>=54)
          {
            #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
              gammaAngleString = QStringRef(&scannedLine, 47, 7);
            #else
              gammaAngleString = QStringView(&scannedLine[47], 7);
            #endif
          }

          _alpha = alphaAngleString.toDouble(&succes);
          _beta = betaAngleString.toDouble(&succes);
          _gamma = gammaAngleString.toDouble(&succes);
        }
        _cell = SKCell(_a, _b, _c, _alpha * M_PI/180.0, _beta*M_PI/180.0, _gamma*M_PI/180.0);

        // structures without a crystal carry a 1x1x1, 90-degree placeholder cell
        const bool cellIsReal = _a > 0.0 && _b > 0.0 && _c > 0.0 &&
                                !isPlaceholderCell(_a, _b, _c, _alpha, _beta, _gamma);
        _periodic = cellIsReal && !_experimentIsNonPeriodic;

        if(scannedLine.size()>=66)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QStringRef spaceGroupString(&scannedLine, 55, 11);
          #else
            QStringView spaceGroupString(&scannedLine[55], 11);
          #endif

          if(std::optional<int> spaceGroupHallNumber = SKSpaceGroup::HallNumberFromHMString(spaceGroupString.toString().simplified()))
          {
            _spaceGroupHallNumber = *spaceGroupHallNumber;
          }
        }
        continue;
      }

      if(keyword == QString("ORIGX1"))
      {
        continue;
      }

      if(keyword == QString("ORIGX2"))
      {
        continue;
      }

      if(keyword == QString("ORIGX3"))
      {
        continue;
      }


      if(keyword == QString("ATOM  ") || keyword == QString("HETATM"))
      {
         //  COLUMNS   LENGHT  DATA TYPE       CONTENTS
         //  --------------------------------------------------------------------------------
         //   0 -  5   6       Record name     "ATOM  "
         //   6 - 10   5       Integer         Atom serial number.
         //  11        1
         //  12 - 15   4       Atom            Atom name.
         //  16        1       Character       Alternate location indicator.
         //  17 - 19   3       Residue name    Residue name.
         //  20        1
         //  21        1       Character       Chain identifier.
         //  22 - 25   4       Integer         Residue sequence number.
         //  26        1       AChar           Code for insertion of residues.
         //  27 - 29   3
         //  30 - 37   8       Real(8.3)       Orthogonal coordinates for X in Angstroms.
         //  38 - 45   8       Real(8.3)       Orthogonal coordinates for Y in Angstroms.
         //  46 - 53   8       Real(8.3)       Orthogonal coordinates for Z in Angstroms.
         //  54 - 59   6       Real(6.2)       Occupancy.
         //  60 - 65   6       Real(6.2)       Temperature factor (Default = 0.0).
         //  66 - 71   6
         //  72 - 75   4       LString(4)      Segment identifier, left-justified.
         //  76 - 77   2       LString(2)      Element symbol, right-justified.
         //  78 - 79   2       LString(2)      Charge on the atom.

        _numberOfAtoms += 1;
        const bool isHetatm = keyword == QString("HETATM");
        if(isHetatm)
        {
          _numberOfSolventAtoms++;
        }
        std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
        atom->setSolvent(isHetatm);
        atom->fractional(false);

        #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
          QStringRef positionsX{};
          QStringRef positionsY{};
          QStringRef positionsZ{};
        #else
          QStringView positionsX{};
          QStringView positionsY{};
          QStringView positionsZ{};
        #endif

        if(scannedLine.size()>=11)
        {
          bool succes = false;
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QString atomSerialNumberString = QStringRef(&scannedLine, 6, 5).toString().simplified();
          #else
            QString atomSerialNumberString = QStringView(&scannedLine[6], 5).toString().simplified();
          #endif
          int atomSerialNumber = atomSerialNumberString.toInt(&succes);
          if(succes)
          {
            atom->setSerialNumber(atomSerialNumber);
          }
        }

        QString atomName;
        if(scannedLine.size()>=16)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            atomName = QStringRef(&scannedLine, 12, 4).toString().simplified();
            atom->setDisplayName(atomName);
            atom->setRemotenessIndicator(QStringRef(&scannedLine, 14, 1).toString().toUtf8().at(0));
            atom->setBranchDesignator(QStringRef(&scannedLine, 15, 1).toString().toUtf8().at(0));
          #else
            atomName = QStringView(&scannedLine[12], 4).toString().simplified();
            atom->setDisplayName(atomName);
            atom->setRemotenessIndicator(QStringView(&scannedLine[14],1).toString().toUtf8().at(0));
            atom->setBranchDesignator(QStringView(&scannedLine[15],1).toString().toUtf8().at(0));
          #endif

          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QString elementString = QStringRef(&scannedLine, 12, 2).toString().simplified().toLower();
          #else
            QString elementString = QStringView(&scannedLine[12], 2).toString().simplified().toLower();
          #endif
          if(!elementString.isEmpty())
          {
            elementString.replace(0, 1, elementString[0].toUpper());
          }
          if (std::map<QString,int>::iterator index = PredefinedElements::atomicNumberData.find(elementString); index != PredefinedElements::atomicNumberData.end())
          {
             atom->setElementIdentifier(index->second);
          }

          if(scannedLine.size()>=20)
          {
            #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
              QString residueName = QStringRef(&scannedLine, 17, 3).toString().simplified().toUpper();
            #else
              QString residueName = QStringView(&scannedLine[17], 3).toString().simplified().toUpper();
            #endif

            atom->setResidueName(residueName);

            if(PredefinedElements::residueDefinitions.find(residueName) != PredefinedElements::residueDefinitions.end())
            {
              _numberOfAminoAcidAtoms += 1;
            }
            else if(SKNucleotide::isNucleotideResidueName(residueName))
            {
              _numberOfNucleotideAtoms += 1;
            }

            const QString backboneLookupName = atomName.toUpper();
            auto it = PredefinedElements::residueDefinitionsElement.find(residueName + "+" + backboneLookupName);
            if(it !=  PredefinedElements::residueDefinitionsElement.end())
            {
              if (std::map<QString,int>::iterator index = PredefinedElements::atomicNumberData.find(it->second); index != PredefinedElements::atomicNumberData.end())
              {
                 atom->setElementIdentifier(index->second);
              }
            }
            auto typeIt = PredefinedElements::residueDefinitionsType.find(residueName + "+" + backboneLookupName);
            if (typeIt != PredefinedElements::residueDefinitionsType.end())
            {
              atom->backBoneAtom(PredefinedElements::isBackboneAtomType(typeIt->second));
            }
          }
        }

        if(scannedLine.size() >= 22)
        {
          const QChar chainIdentifier = scannedLine.at(21);
          if (!chainIdentifier.isSpace())
          {
            atom->setChainIdentifier(chainIdentifier.toLatin1());
          }
        }

        if(scannedLine.size() >= 26)
        {
          bool success = false;
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            const int residueSequenceNumber = QStringRef(&scannedLine, 22, 4).toString().trimmed().toInt(&success);
          #else
            const int residueSequenceNumber = QStringView(&scannedLine[22], 4).toString().trimmed().toInt(&success);
          #endif
          if (success)
          {
            atom->setResidueSequenceNumber(residueSequenceNumber);
          }
        }

        if(scannedLine.size() >= 27)
        {
          const QChar insertionCode = scannedLine.at(26);
          if (!insertionCode.isSpace())
          {
            atom->setCodeForInsertionOfResidues(insertionCode.toLatin1());
          }
        }

        if(scannedLine.size()>=38)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            positionsX = QStringRef(&scannedLine, 30, 8);
          #else
            positionsX = QStringView(&scannedLine[30], 8);
          #endif
        }
        if(scannedLine.size()>=46)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            positionsY = QStringRef(&scannedLine, 38, 8);
          #else
            positionsY = QStringView(&scannedLine[38], 8);
          #endif
        }
        if(scannedLine.size()>=54)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            positionsZ = QStringRef(&scannedLine, 46, 8);
          #else
            positionsZ = QStringView(&scannedLine[46], 8);
          #endif
        }
        if(scannedLine.size()>=60)
        {
          bool succes = false;
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QStringRef occupancyString = QStringRef(&scannedLine, 54, 6);
          #else
            QStringView occupancyString = QStringView(&scannedLine[54], 6);
          #endif
          double occupancy = occupancyString.toDouble(&succes);
          if(succes)
          {
            atom->setOccupancy(occupancy);
          }
        }
        if(scannedLine.size()>=78)
        {
          #if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QString chemicalElement = QStringRef(&scannedLine, 76, 2).toString().simplified().toLower();
          #else
            QString chemicalElement = QStringView(&scannedLine[76], 2).toString().simplified().toLower();
          #endif

          if(!chemicalElement.isEmpty())
          {
            chemicalElement.replace(0, 1, chemicalElement[0].toUpper());
            if (std::map<QString,int>::iterator index = PredefinedElements::atomicNumberData.find(chemicalElement); index != PredefinedElements::atomicNumberData.end())
            {
               atom->setElementIdentifier(index->second);
            }
          }
        }

        double3 position;
        bool succes = false;
        position.x = positionsX.toDouble(&succes);
        position.y = positionsY.toDouble(&succes);
        position.z = positionsZ.toDouble(&succes);
        atom->setPosition(position);

        noteResidueAtom(atom);

        _frame->atoms.push_back(atom);
        continue;
      }
    }
  }

  // add current frame in case last TER, ENDMDL, or END is missing
  if(_frame->atoms.size()>0)
  {
    addFrameToStructure(currentMovie,currentFrame);
  }
}
