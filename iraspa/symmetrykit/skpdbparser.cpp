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
#include <algorithm>
#include <cmath>
#include <qmath.h>
#include "symmetrykitprotocols.h"
#include "skasymmetricatom.h"
#include "skelement.h"
#include "sknucleotide.h"

namespace
{
  QString pdbViewField(const QString &line, int location, int length)
  {
    if(line.size() < location + length) return QString();
    return line.mid(location, length);
  }

  QString titleCaseSymbol(const QString &field)
  {
    QString symbol = field.trimmed();
    if(symbol.isEmpty()) return symbol;
    symbol = symbol.toLower();
    symbol[0] = symbol[0].toUpper();
    return symbol;
  }

  std::optional<int> atomicNumberForSymbol(const QString &field)
  {
    const QString symbol = titleCaseSymbol(field);
    if(symbol.isEmpty()) return std::nullopt;
    const auto index = PredefinedElements::atomicNumberData.find(symbol);
    if(index == PredefinedElements::atomicNumberData.end()) return std::nullopt;
    return index->second;
  }
}

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
    reserveAtomCapacity(_scanner.content().size());
  }
}

void SKPDBParser::reserveAtomCapacity(int fileCharacterCount)
{
  const int estimatedAtoms = std::min(std::max(fileCharacterCount / 50, 256), 1000000);
  _frame->atoms.reserve(static_cast<size_t>(estimatedAtoms));
}

void SKPDBParser::parseAndAppendAtomRecord(const QString &line, bool isHetatm)
{
  _numberOfAtoms += 1;
  if(isHetatm)
  {
    _numberOfSolventAtoms++;
  }

  if(line.size() < 11) return;

  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>();
  atom->setSolvent(isHetatm);
  atom->fractional(false);

  bool success = false;
  const int atomSerialNumber = pdbViewField(line, 6, 5).trimmed().toInt(&success);
  if(success)
  {
    atom->setSerialNumber(atomSerialNumber);
  }

  QString atomName;
  if(line.size() >= 16)
  {
    atomName = pdbViewField(line, 12, 4).trimmed();
    atom->setDisplayName(atomName);
    if(line.size() >= 15)
    {
      atom->setRemotenessIndicator(line.at(14).toLatin1());
    }
    if(line.size() >= 16)
    {
      atom->setBranchDesignator(line.at(15).toLatin1());
    }

    if(const std::optional<int> atomicNumber = atomicNumberForSymbol(pdbViewField(line, 12, 2)))
    {
      atom->setElementIdentifier(*atomicNumber);
    }

    if(line.size() >= 20)
    {
      const QString residueName = pdbViewField(line, 17, 3).trimmed().toUpper();
      atom->setResidueName(residueName);

      if(PredefinedElements::residueDefinitions.find(residueName) != PredefinedElements::residueDefinitions.end())
      {
        _numberOfAminoAcidAtoms += 1;
      }
      else if(SKNucleotide::isNucleotideResidueName(residueName))
      {
        _numberOfNucleotideAtoms += 1;
      }

      const QString residueAtomKey = residueName + QLatin1Char('+') + atomName.toUpper();
      const auto elementIt = PredefinedElements::residueDefinitionsElement.find(residueAtomKey);
      if(elementIt != PredefinedElements::residueDefinitionsElement.end())
      {
        const auto index = PredefinedElements::atomicNumberData.find(elementIt->second);
        if(index != PredefinedElements::atomicNumberData.end())
        {
          atom->setElementIdentifier(index->second);
        }
      }
      const auto typeIt = PredefinedElements::residueDefinitionsType.find(residueAtomKey);
      if(typeIt != PredefinedElements::residueDefinitionsType.end())
      {
        atom->backBoneAtom(PredefinedElements::isBackboneAtomType(typeIt->second));
      }
    }
  }

  if(line.size() >= 22)
  {
    const QChar chainIdentifier = line.at(21);
    if(!chainIdentifier.isSpace())
    {
      atom->setChainIdentifier(chainIdentifier.toLatin1());
    }
  }

  if(line.size() >= 26)
  {
    success = false;
    const int residueSequenceNumber = pdbViewField(line, 22, 4).trimmed().toInt(&success);
    if(success)
    {
      atom->setResidueSequenceNumber(residueSequenceNumber);
    }
  }

  if(line.size() >= 27)
  {
    const QChar insertionCode = line.at(26);
    if(!insertionCode.isSpace())
    {
      atom->setCodeForInsertionOfResidues(insertionCode.toLatin1());
    }
  }

  double3 position{};
  if(line.size() >= 38)
  {
    success = false;
    position.x = pdbViewField(line, 30, 8).trimmed().toDouble(&success);
  }
  if(line.size() >= 46)
  {
    success = false;
    position.y = pdbViewField(line, 38, 8).trimmed().toDouble(&success);
  }
  if(line.size() >= 54)
  {
    success = false;
    position.z = pdbViewField(line, 46, 8).trimmed().toDouble(&success);
  }
  atom->setPosition(position);

  if(line.size() >= 60)
  {
    success = false;
    const double occupancy = pdbViewField(line, 54, 6).trimmed().toDouble(&success);
    if(success)
    {
      atom->setOccupancy(occupancy);
    }
  }

  if(line.size() >= 66)
  {
    success = false;
    const double temperatureFactor = pdbViewField(line, 60, 6).trimmed().toDouble(&success);
    if(success)
    {
      atom->setTemperaturefactor(temperatureFactor);
    }
  }

  if(line.size() >= 78)
  {
    if(const std::optional<int> atomicNumber = atomicNumberForSymbol(pdbViewField(line, 76, 2)))
    {
      atom->setElementIdentifier(*atomicNumber);
    }
  }

  noteResidueAtom(atom);
  _frame->atoms.push_back(atom);
}

void SKPDBParser::startParsing() noexcept(false)
{
  [[maybe_unused]] int lineNumber = 0;
  int modelNumber = 0;
  size_t currentMovie = 0;
  size_t currentFrame = 0;

  const QString &text = _scanner.content();
  reserveAtomCapacity(text.size());

  int pos = 0;
  const int n = text.size();
  while(pos < n)
  {
    int end = text.indexOf(QLatin1Char('\n'), pos);
    if(end < 0) end = n;
    QString scannedLine = text.mid(pos, end - pos);
    if(!scannedLine.isEmpty() && scannedLine.at(scannedLine.size() - 1) == QLatin1Char('\r'))
    {
      scannedLine.chop(1);
    }
    pos = (end < n) ? end + 1 : n;

    if(scannedLine.isEmpty()) continue;
    lineNumber += 1;

    const int length = scannedLine.size();
    if(length < 3) continue;

    // Cocoa checks ATOM/HETATM before the rare record types so the 58k-atom hot path
    // does not construct a QString for HEADER, AUTHOR, REMARK, ... on every line.
    if(scannedLine.startsWith(QLatin1String("ATOM  ")))
    {
      parseAndAppendAtomRecord(scannedLine, false);
      continue;
    }
    if(scannedLine.startsWith(QLatin1String("HETATM")))
    {
      parseAndAppendAtomRecord(scannedLine, true);
      continue;
    }
    if(scannedLine.startsWith(QLatin1String("TER")))
    {
      if(_separatePolymerChains && _frame->atoms.size() > 0)
      {
        addFrameToStructure(currentMovie, currentFrame);
        currentMovie += 1;
      }
      continue;
    }

    if(length < 6) continue;
    const QString keyword = scannedLine.left(6);

    if(keyword == QLatin1String("HEADER") ||
       keyword == QLatin1String("AUTHOR") ||
       keyword == QLatin1String("REVDAT") ||
       keyword == QLatin1String("JRNL  ") ||
       keyword == QLatin1String("REMARK") ||
       keyword == QLatin1String("SCALE1") ||
       keyword == QLatin1String("SCALE2") ||
       keyword == QLatin1String("SCALE3") ||
       keyword == QLatin1String("ORIGX1") ||
       keyword == QLatin1String("ORIGX2") ||
       keyword == QLatin1String("ORIGX3"))
    {
      continue;
    }

    if(keyword == QLatin1String("EXPDTA"))
    {
      const QString experiment = scannedLine.mid(6).trimmed().toUpper();
      if(experiment.contains(QLatin1String("NMR")) || experiment.contains(QLatin1String("ELECTRON MICROSCOPY")) ||
         experiment.contains(QLatin1String("SOLUTION SCATTERING")) || experiment.contains(QLatin1String("THEORETICAL MODEL")))
      {
        _experimentIsNonPeriodic = true;
        _periodic = false;
      }
      continue;
    }

    if(keyword == QLatin1String("SEQRES"))
    {
      parseSeqres(scannedLine);
      continue;
    }

    if(keyword == QLatin1String("MODRES"))
    {
      parseModres(scannedLine);
      continue;
    }

    if(keyword == QLatin1String("MODEL "))
    {
      currentMovie = 0;
      if(length <= 10) continue;

      bool success = false;
      const int integerValue = scannedLine.mid(6).trimmed().toInt(&success);
      if(success)
      {
        _frame = std::make_shared<SKStructure>();
        reserveAtomCapacity(text.size());
        currentFrame = std::max(0, integerValue - 1);
        currentFrame = modelNumber;
        modelNumber += 1;
      }
      continue;
    }

    if(keyword == QLatin1String("ENDMDL"))
    {
      addFrameToStructure(currentMovie, currentFrame);
      currentFrame += 1;
      continue;
    }

    if(keyword == QLatin1String("CRYST1"))
    {
      bool success = false;
      if(scannedLine.size() >= 17)
      {
        _a = pdbViewField(scannedLine, 6, 9).trimmed().toDouble(&success);
      }
      if(scannedLine.size() >= 24)
      {
        _b = pdbViewField(scannedLine, 15, 9).trimmed().toDouble(&success);
      }
      if(scannedLine.size() >= 33)
      {
        _c = pdbViewField(scannedLine, 24, 9).trimmed().toDouble(&success);
      }

      _alpha = 90.0;
      _beta = 90.0;
      _gamma = 90.0;
      if(scannedLine.size() >= 40)
      {
        _alpha = pdbViewField(scannedLine, 33, 7).trimmed().toDouble(&success);
      }
      if(scannedLine.size() >= 47)
      {
        _beta = pdbViewField(scannedLine, 40, 7).trimmed().toDouble(&success);
      }
      if(scannedLine.size() >= 54)
      {
        _gamma = pdbViewField(scannedLine, 47, 7).trimmed().toDouble(&success);
      }
      _cell = SKCell(_a, _b, _c, _alpha * M_PI / 180.0, _beta * M_PI / 180.0, _gamma * M_PI / 180.0);

      const bool cellIsReal = _a > 0.0 && _b > 0.0 && _c > 0.0 &&
                              !isPlaceholderCell(_a, _b, _c, _alpha, _beta, _gamma);
      _periodic = cellIsReal && !_experimentIsNonPeriodic;

      if(scannedLine.size() >= 66)
      {
        if(std::optional<int> spaceGroupHallNumber = SKSpaceGroup::HallNumberFromHMString(pdbViewField(scannedLine, 55, 11).simplified()))
        {
          _spaceGroupHallNumber = *spaceGroupHallNumber;
        }
      }
      continue;
    }
  }

  if(_frame->atoms.size() > 0)
  {
    addFrameToStructure(currentMovie, currentFrame);
  }
}
