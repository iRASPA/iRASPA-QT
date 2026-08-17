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

#include <QDebug>
#include <QStringList>
#include "skcifparser.h"
#include "sknucleotide.h"
#include "symmetrykitprotocols.h"
#include "skasymmetricatom.h"
#include "skelement.h"
#include "skspacegroup.h"

#include <cmath>
#include <qmath.h>

SKCIFParser::SKCIFParser(QUrl url, bool proteinOnlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped, bool separatePolymerChains): SKParser(),
  _scanner(url, charactersToBeSkipped), _proteinOnlyAsymmetricUnitCell(proteinOnlyAsymmetricUnitCell), _asMolecule(asMolecule),
  _separatePolymerChains(separatePolymerChains), _content(_scanner.string())
{
}

void SKCIFParser::startParsing() noexcept(false)
{
  while(!_scanner.isAtEnd())
  {
    QString tempString;

    // scan to first keyword
    _previousScanLocation = _scanner.scanLocation();
    if (_scanner.scanUpToCharacters(CharacterSet::whitespaceAndNewlineCharacterSet(), tempString))
    {
      QString keyword = tempString.toLower();

      if (keyword.startsWith(QString("_audit")))
      {
        parseAudit(keyword);
      }
      else if(keyword.startsWith(QString("_iraspa")))
      {

        parseiRASPA(keyword);
      }
      else if(keyword.startsWith(QString("_chemical")))
      {
        parseChemical(keyword);
      }
      else if(keyword.startsWith(QString("_cell")))
      {
        parseCell(keyword);
      }
      else if(keyword.startsWith(QString("_symmetry")))
      {
        parseSymmetry(keyword);
      }
      else if(keyword.startsWith(QString("_space_group")))
      {
        parseSymmetry(keyword);
      }
      else if(keyword.startsWith(QString("_pdbx_struct_mod_residue")))
      {
        parseModResidue(keyword);
      }
      else if(keyword.startsWith(QString("data_")))
      {
        parseName(keyword);
      }
      else if(keyword.startsWith(QString("loop_")))
      {
        parseLoop(keyword);
      }
      else if(keyword.startsWith(QString("#")))
      {
        skipComment();
      }
      else if(keyword.startsWith(QString("_")))
      {
        // unknown CIF/mmCIF data item: consume the value so that the scanner stays aligned
        parseValue();
      }
    }
  }

  resolvePolymerChainsFromEntityTables();

  std::vector<std::shared_ptr<SKStructure>> movieFrames{};
  std::shared_ptr<SKStructure> structure = std::make_shared<SKStructure>();
  structure->displayName = _scanner.displayName();
  structure->atoms = _atoms;

  const double cellA = (_a > 1e-6) ? _a : 20.0;
  const double cellB = (_b > 1e-6) ? _b : 20.0;
  const double cellC = (_c > 1e-6) ? _c : 20.0;
  structure->cell = std::make_shared<SKCell>(cellA, cellB, cellC, _alpha * M_PI/180.0, _beta*M_PI/180.0, _gamma*M_PI/180.0);

  const SKStructure::Kind kind = kindOfCurrentPart();
  structure->kind = kind;

  switch(kind)
  {
  case SKStructure::Kind::protein:
  case SKStructure::Kind::dna:
  case SKStructure::Kind::molecule:
    structure->drawUnitCell = false;
    structure->spaceGroupHallNumber = 1;
    structure->periodic = false;
    break;
  case SKStructure::Kind::proteinCrystal:
  case SKStructure::Kind::proteinCrystalSolvent:
  case SKStructure::Kind::dnaCrystal:
    structure->drawUnitCell = !_proteinOnlyAsymmetricUnitCell;
    structure->spaceGroupHallNumber = _proteinOnlyAsymmetricUnitCell ? 1 : _spaceGroupHallNumber.value_or(1);
    structure->periodic = true;
    break;
  default:
    structure->drawUnitCell = true;
    structure->spaceGroupHallNumber = _spaceGroupHallNumber.value_or(1);
    structure->periodic = true;
    break;
  }

  movieFrames.push_back(structure);
  _movies.push_back(movieFrames);
}

void SKCIFParser::parseAudit(QString& string)
{
  Q_UNUSED(string);
  parseValue();
}

void SKCIFParser::parseiRASPA(QString& string)
{
  Q_UNUSED(string);
  parseValue();
}

void SKCIFParser::parseChemical(QString& string)
{
  Q_UNUSED(string);
  parseValue();
}

void SKCIFParser::parseCell(QString& string)
{
  if (string == QString("_cell_length_a") || string == QString("_cell.length_a"))
  {
    _a = scanDouble();
    _cellLengthsDefined = true;
  }
  else if (string == QString("_cell_length_b") || string == QString("_cell.length_b"))
  {
    _b = scanDouble();
    _cellLengthsDefined = true;
  }
  else if (string == QString("_cell_length_c") || string == QString("_cell.length_c"))
  {
    _c = scanDouble();
    _cellLengthsDefined = true;
  }
  else if (string == QString("_cell_angle_alpha") || string == QString("_cell.angle_alpha"))
  {
    _alpha = scanDouble();
  }
  else if (string == QString("_cell_angle_beta") || string == QString("_cell.angle_beta"))
  {
    _beta = scanDouble();
  }
  else if (string == QString("_cell_angle_gamma") || string == QString("_cell.angle_gamma"))
  {
    _gamma = scanDouble();
  }
  else
  {
    // ignore unrecognized mmCIF/coreCIF cell tags (e.g. _cell.entry_id, *_esd)
    parseValue();
  }
}

void SKCIFParser::parseSymmetry(QString& string)
{
  if(string == QString("_symmetry_cell_setting").toLower())
  {
    parseValue();
    return;
  }

  // prefer setting spacegroup based on Hall-symbol
  if((string == QString("_space_group_name_Hall").toLower()) ||
     (string == QString("_symmetry_space_group_name_Hall").toLower()) ||
     (string == QString("_symmetry.space_group_name_Hall").toLower()))
  {
    if(std::optional<QString> possibleString = scanString())
    {
      _spaceGroupHallNumber = SKSpaceGroup::HallNumber(*possibleString);
    }
    return;
  }

  if((string == QString("_space_group_name_H-M_alt").toLower()) ||
     (string == QString("_symmetry_space_group_name_H-M").toLower()) ||
     (string == QString("_symmetry.space_group_name_h-m").toLower()) ||
     (string == QString("_symmetry.pdbx_full_space_group_name_H-M").toLower()))
  {
    std::optional<QString> possibleString = scanString();
    if(!_spaceGroupHallNumber && possibleString && *possibleString != QString("?"))
    {
      _spaceGroupHallNumber = SKSpaceGroup::HallNumberFromHMString(*possibleString);
    }
    return;
  }

  if((string == QString("_space_group_IT_number").toLower()) ||
     (string == QString("_symmetry_Int_Tables_number").toLower()) ||
     (string == QString("_symmetry.Int_Tables_number").toLower()))
  {
    const int spaceGroupNumber = static_cast<int>(scanInt());
    if(!_spaceGroupHallNumber)
    {
      _spaceGroupHallNumber = SKSpaceGroup::HallNumberFromSpaceGroupNumber(spaceGroupNumber);
    }
    return;
  }

  parseValue();
}

void SKCIFParser::parseName(QString& string)
{
  Q_UNUSED(string);
}

void SKCIFParser::parseModResidue(QString& string)
{
  const std::optional<QString> value = parseValue();
  if(!value) return;

  if(string == QString("_pdbx_struct_mod_residue.label_comp_id") ||
     string == QString("_pdbx_struct_mod_residue.auth_comp_id"))
  {
    const QString trimmed = value->trimmed().toUpper();
    if(!trimmed.isEmpty() && trimmed != QString("?") && trimmed != QString("."))
    {
      _modifiedResidues.insert(trimmed);
    }
  }
}

// <Value> = { '.' | '?' | <Numeric> | <CharString> | <TextField> }
std::optional<QString> SKCIFParser::parseValue()
{
  if (_scanner.isAtEnd())
  {
    return std::nullopt;
  }

  const QString::const_iterator end = _content.constEnd();
  QString::const_iterator location = _scanner.scanLocation();

  // skip whitespace and comments
  while(location != end)
  {
    while(location != end && location->isSpace()) ++location;
    if(location != end && *location == QChar('#'))
    {
      while(location != end && *location != QChar('\n') && *location != QChar('\r')) ++location;
      continue;
    }
    break;
  }

  if(location == end)
  {
    _scanner.setScanLocation(end);
    return std::nullopt;
  }

  const QString::const_iterator previousScanLocation = location;
  const QChar first = *location;

  // CIF text field: a semicolon at the start of a line
  if(first == QChar(';'))
  {
    ++location;
    QStringList lines{};
    while(true)
    {
      QString line;
      while(location != end && *location != QChar('\n') && *location != QChar('\r'))
      {
        line.append(*location);
        ++location;
      }
      while(location != end && (*location == QChar('\n') || *location == QChar('\r'))) ++location;

      if(!line.isEmpty() || !lines.isEmpty()) lines.append(line);

      if(location == end) break;
      if(*location == QChar(';'))
      {
        ++location;
        break;
      }
    }
    _scanner.setScanLocation(location);
    return lines.join(QChar('\n'));
  }

  // single- or double-quoted char strings (CIF allows '' / "" escapes)
  if(first == QChar('\'') || first == QChar('"'))
  {
    const QChar quote = first;
    ++location;
    QString content;
    while(location != end)
    {
      const QChar character = *location;
      ++location;
      if(character == quote)
      {
        if(location != end && *location == quote)
        {
          content.append(quote);
          ++location;
          continue;
        }
        break;
      }
      content.append(character);
    }
    _scanner.setScanLocation(location);
    return content;
  }

  QString token;
  while(location != end && !location->isSpace())
  {
    token.append(*location);
    ++location;
  }

  const QString keyword = token.toLower();
  if(keyword.startsWith(QString("_")) || keyword.startsWith(QString("loop_")) ||
     keyword.startsWith(QString("data_")) || keyword.startsWith(QString("save_")))
  {
    _scanner.setScanLocation(previousScanLocation);
    return std::nullopt;
  }

  _scanner.setScanLocation(location);
  return token;
}

// a loop can contain comments
// <DataItems> = <Tag> <WhiteSpace> <Value> | <LoopHeader> <LoopBody>    [case sensitive]
// <LoopHeader> = <LOOP_> {<WhiteSpace> <Tag>}+                          [case insensitive]
// <LoopBody> = <Value> { <WhiteSpace> <Value> }*                        [case sensitive]
void SKCIFParser::parseLoop(QString& string)
{
  Q_UNUSED(string);
  QString tempString;
  QString::const_iterator previousScanLocation;
  std::vector<QString> tags;

  // part 1: read the 'tags'
  previousScanLocation = _scanner.scanLocation();
  while(_scanner.scanUpToCharacters(CharacterSet::whitespaceAndNewlineCharacterSet(), tempString) && (tempString.size() > 0)  && (tempString.startsWith(QString("_")) || (tempString.startsWith(QString("#")))))
  {
    QString tag = tempString.toLower();

    if(tag.startsWith(QString("#")))
    {
      skipComment();
    }
    else
    {
       tags.push_back(tag);
    }
    previousScanLocation=_scanner.scanLocation();
  }

  // set scanner back to the first <value>
  _scanner.setScanLocation(previousScanLocation);

  std::optional<QString> value = std::nullopt;
  do
  {
    std::map<QString,QString> dictionary{};

    for(const QString &tag : tags)
    {
      if ((value = parseValue()))
      {
        dictionary[tag] = *value;
      }
    }

    if (value)
    {
      if (const std::optional<QString> chemicalSymbol = normalizedChemicalElement(dictionaryValue(dictionary, {QString("_atom_site_type_symbol"), QString("_atom_site.type_symbol")})))
      {
        appendAtomSite(dictionary, *chemicalSymbol);
      }
      else if (const std::optional<QString> monId = dictionaryValue(dictionary, {QString("_entity_poly_seq.mon_id")}))
      {
        recordEntityPolySeq(dictionary, *monId);
      }
      else if (const std::optional<QString> modifiedResidue = dictionaryValue(dictionary, {QString("_pdbx_struct_mod_residue.label_comp_id"),
                                                                                           QString("_pdbx_struct_mod_residue.auth_comp_id")}))
      {
        _modifiedResidues.insert(modifiedResidue->toUpper());
      }
      else if (const std::optional<QString> asymId = dictionaryValue(dictionary, {QString("_struct_asym.id")}))
      {
        if (const std::optional<QString> entityId = dictionaryValue(dictionary, {QString("_struct_asym.entity_id")}))
        {
          _asymToEntity[*asymId] = *entityId;
        }
      }
      else if (const std::optional<QString> entityId = dictionaryValue(dictionary, {QString("_entity_poly.entity_id")}))
      {
        if (const std::optional<QString> polyType = dictionaryValue(dictionary, {QString("_entity_poly.type")}))
        {
          recordEntityPolyType(*entityId, *polyType);
        }
      }
    }
  }
  while (value);
  // note: scanner-location is restored to first word after the 'loop'
}

void SKCIFParser::appendAtomSite(const std::map<QString,QString> &dictionary, const QString &chemicalSymbol)
{
  _numberOfAtoms += 1;

  // Element 0 stands for 'not identified yet': the residue dictionary below
  // claims the site first, and the chemical symbol of the site fills in the rest.
  std::shared_ptr<SKAsymmetricAtom> atom = std::make_shared<SKAsymmetricAtom>(chemicalSymbol, 0);

  if (const std::optional<QString> groupPDB = dictionaryValue(dictionary, {QString("_atom_site.group_pdb")}))
  {
    atom->setSolvent(groupPDB->toUpper() == QString("HETATM"));
  }

  if (const std::optional<QString> serialNumber = dictionaryValue(dictionary, {QString("_atom_site.id")}))
  {
    bool success = false;
    const qint64 value = serialNumber->toLongLong(&success);
    if(success) atom->setSerialNumber(value);
  }

  const QString atomName = dictionaryValue(dictionary, {QString("_atom_site.label_atom_id"), QString("_atom_site.auth_atom_id"),
                                                        QString("_atom_site_label"), QString("_atom_site.id")}).value_or(chemicalSymbol);
  atom->setDisplayName(atomName);
  if(atomName.size() >= 3)
  {
    atom->setRemotenessIndicator(atomName.at(2).toLatin1());
  }
  if(atomName.size() >= 4)
  {
    atom->setBranchDesignator(atomName.at(3).toLatin1());
  }

  const QString residueName = dictionaryValue(dictionary, {QString("_atom_site.label_comp_id"), QString("_atom_site.auth_comp_id")}).value_or(QString()).toUpper();
  atom->setResidueName(residueName);

  const QString residueAtomKey = residueName + QString("+") + atomName.toUpper();
  const std::map<QString,QString>::const_iterator elementIterator = PredefinedElements::residueDefinitionsElement.find(residueAtomKey);
  if(elementIterator != PredefinedElements::residueDefinitionsElement.end())
  {
    _numberOfAminoAcidAtoms += 1;

    const std::map<QString,QString>::const_iterator typeIterator = PredefinedElements::residueDefinitionsType.find(residueAtomKey);
    if(typeIterator != PredefinedElements::residueDefinitionsType.end())
    {
      atom->backBoneAtom(PredefinedElements::isBackboneAtomType(typeIterator->second));
    }
    if(const std::map<QString,int>::const_iterator index = PredefinedElements::atomicNumberData.find(elementIterator->second);
       index != PredefinedElements::atomicNumberData.end())
    {
      atom->setElementIdentifier(index->second);
      atom->setUniqueForceFieldName(index->first);
    }
  }
  else if(SKNucleotide::isNucleotideResidueName(residueName))
  {
    _numberOfNucleicAcidAtoms += 1;
  }
  else if(PredefinedElements::knownAminoAcidResidueCodes.find(residueName) != PredefinedElements::knownAminoAcidResidueCodes.end())
  {
    _numberOfAminoAcidAtoms += 1;
  }

  if (const std::optional<QString> chain = dictionaryValue(dictionary, {QString("_atom_site.label_asym_id"), QString("_atom_site.auth_asym_id"),
                                                                        QString("_atom_site.label_entity_id")}))
  {
    atom->setChainIdentifier(chain->at(0).toLatin1());
  }

  if (const std::optional<QString> sequenceID = dictionaryValue(dictionary, {QString("_atom_site.label_seq_id"), QString("_atom_site.auth_seq_id")}))
  {
    bool success = false;
    const qint64 value = sequenceID->toLongLong(&success);
    if(success) atom->setResidueSequenceNumber(value);
  }

  if (const std::optional<QString> insertionCode = dictionaryValue(dictionary, {QString("_atom_site.pdbx_pdb_ins_code")}))
  {
    atom->setCodeForInsertionOfResidues(insertionCode->at(0).toLatin1());
  }

  // prefer fractional for materials/crystals, Cartesian for biomolecular mmCIF sites
  const bool looksLikeProteinSite = dictionaryValue(dictionary, {QString("_atom_site.group_pdb"), QString("_atom_site.label_comp_id"),
                                                                 QString("_atom_site.auth_comp_id")}).has_value();

  const std::optional<double> cartnX = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.cartn_x"), QString("_atom_site_cartn_x")}));
  const std::optional<double> cartnY = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.cartn_y"), QString("_atom_site_cartn_y")}));
  const std::optional<double> cartnZ = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.cartn_z"), QString("_atom_site_cartn_z")}));
  const std::optional<double> fractX = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.fract_x"), QString("_atom_site_fract_x")}));
  const std::optional<double> fractY = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.fract_y"), QString("_atom_site_fract_y")}));
  const std::optional<double> fractZ = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.fract_z"), QString("_atom_site_fract_z")}));

  const bool hasCartesian = cartnX && cartnY && cartnZ;
  const bool hasFractional = fractX && fractY && fractZ;

  if(hasCartesian && (looksLikeProteinSite || !hasFractional))
  {
    atom->setPosition(double3(*cartnX, *cartnY, *cartnZ));
    atom->fractional(false);
  }
  else if(hasFractional)
  {
    atom->setPosition(double3(*fractX, *fractY, *fractZ));
    atom->fractional(true);
  }

  if (const std::optional<double> charge = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.charge"), QString("_atom_site_charge"),
                                                                                       QString("_atom_site.pdbx_formal_charge")})))
  {
    atom->setCharge(*charge);
  }

  if (const std::optional<double> occupancy = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.occupancy"), QString("_atom_site_occupancy")})))
  {
    atom->setOccupancy(*occupancy);
  }

  if (const std::optional<double> temperatureFactor = parseCIFDouble(dictionaryValue(dictionary, {QString("_atom_site.b_iso_or_equiv"), QString("_atom_site_b_iso_or_equiv"),
                                                                                                  QString("_atom_site.u_iso_or_equiv"), QString("_atom_site_u_iso_or_equiv")})))
  {
    atom->setTemperaturefactor(*temperatureFactor);
  }

  if(atom->elementIdentifier() == 0)
  {
    if(const std::map<QString,int>::const_iterator index = PredefinedElements::atomicNumberData.find(chemicalSymbol);
       index != PredefinedElements::atomicNumberData.end())
    {
      atom->setElementIdentifier(index->second);
    }
  }

  // materials CIF often stores the site label as both name and force-field type
  if(!looksLikeProteinSite)
  {
    if (const std::optional<QString> label = dictionaryValue(dictionary, {QString("_atom_site_label"), QString("_atom_site.label")}))
    {
      atom->setDisplayName(*label);
    }
    atom->setUniqueForceFieldName(dictionaryValue(dictionary, {QString("_atom_site_forcefield_label"), QString("_atom_site.forcefield_label")}).value_or(atom->displayName()));
  }
  else
  {
    atom->setUniqueForceFieldName(dictionaryValue(dictionary, {QString("_atom_site.forcefield_label"), QString("_atom_site_forcefield_label")}).value_or(chemicalSymbol));
  }

  if(atom->elementIdentifier() <= 0) return;

  if(PredefinedElements::knownAminoAcidResidueCodes.find(residueName) != PredefinedElements::knownAminoAcidResidueCodes.end() ||
     SKNucleotide::isNucleotideResidueName(residueName) ||
     _modifiedResidues.find(residueName) != _modifiedResidues.end())
  {
    _polymerChains.insert(atom->chainIdentifier());
  }

  noteResidueAtom(atom);
  _atoms.push_back(atom);
}

void SKCIFParser::recordEntityPolySeq(const std::map<QString,QString> &dictionary, const QString &monId)
{
  const QString residueName = monId.toUpper();
  if(PredefinedElements::knownAminoAcidResidueCodes.find(residueName) == PredefinedElements::knownAminoAcidResidueCodes.end() &&
     !SKNucleotide::isNucleotideResidueName(residueName) &&
     _modifiedResidues.find(residueName) == _modifiedResidues.end())
  {
    return;
  }

  if(const std::optional<QString> entityId = dictionaryValue(dictionary, {QString("_entity_poly_seq.entity_id")}))
  {
    _polymerEntityIds.insert(*entityId);

    for(const auto &[asym, entity] : _asymToEntity)
    {
      if(entity == *entityId && !asym.isEmpty())
      {
        _polymerChains.insert(asym.at(0));
      }
    }
  }
}

void SKCIFParser::recordEntityPolyType(const QString &entityId, const QString &polyType)
{
  const QString entity = entityId.trimmed();
  const QString type = polyType.trimmed().toLower();
  if(entity.isEmpty()) return;

  if(type.contains(QString("polypeptide")) || type.contains(QString("polydeoxyribonucleotide")) ||
     type.contains(QString("polyribonucleotide")) || type.contains(QString("nucleotide")))
  {
    _polymerEntityIds.insert(entity);

    for(const auto &[asym, mappedEntity] : _asymToEntity)
    {
      if(mappedEntity == entity && !asym.isEmpty())
      {
        _polymerChains.insert(asym.at(0));
      }
    }
  }
}

void SKCIFParser::resolvePolymerChainsFromEntityTables()
{
  for(const auto &[asym, entity] : _asymToEntity)
  {
    if(_polymerEntityIds.find(entity) != _polymerEntityIds.end() && !asym.isEmpty())
    {
      _polymerChains.insert(asym.at(0));
    }
  }
}

void SKCIFParser::noteResidueAtom(const std::shared_ptr<SKAsymmetricAtom> &atom)
{
  const QString residueName = atom->residueName().trimmed().toUpper();
  if(residueName.isEmpty()) return;

  const ResidueKey key{atom->chainIdentifier(), atom->residueSequenceNumber()};
  ResidueRecord &residue = _residues[key];
  residue.name = residueName;
  if(isWaterResidue(residueName)) residue.water = true;
  if(SKNucleotide::isNucleotideResidueName(residueName)) residue.nucleotide = true;

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

SKStructure::Kind SKCIFParser::kindOfCurrentPart()
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

  const bool periodic = _cellLengthsDefined && _a > 1e-6 && _b > 1e-6 && _c > 1e-6 && !_asMolecule;

  const bool isProtein = peptideResidues >= 2 && peptideBonds >= 1 && peptideResidues > otherResidues;
  if(isProtein)
  {
    _proteinDetected = true;
    return periodic ? SKStructure::Kind::proteinCrystal : SKStructure::Kind::protein;
  }

  const bool isDNA = nucleicResidues >= 2 && nucleicResidues > otherResidues && nucleicResidues >= peptideResidues;
  if(isDNA)
  {
    _dnaDetected = true;
    return periodic ? SKStructure::Kind::dnaCrystal : SKStructure::Kind::dna;
  }

  // fallback: atom-fraction heuristics for sparse residue metadata
  if(_numberOfAtoms > 0)
  {
    if(double(_numberOfAminoAcidAtoms)/double(_numberOfAtoms) > 0.5)
    {
      _proteinDetected = true;
      return periodic ? SKStructure::Kind::proteinCrystal : SKStructure::Kind::protein;
    }
    if(double(_numberOfNucleicAcidAtoms)/double(_numberOfAtoms) > 0.5)
    {
      _dnaDetected = true;
      return periodic ? SKStructure::Kind::dnaCrystal : SKStructure::Kind::dna;
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
  if(onlySolvent)
  {
    return SKStructure::Kind::proteinCrystalSolvent;
  }

  if(_asMolecule)
  {
    return SKStructure::Kind::molecule;
  }
  return SKStructure::Kind::crystal;
}

std::optional<QString> SKCIFParser::dictionaryValue(const std::map<QString,QString> &dictionary, const std::vector<QString> &keys)
{
  // tags are lower-cased when read, so the lookup keys have to be lower-cased too
  for(const QString &key : keys)
  {
    const std::map<QString,QString>::const_iterator iterator = dictionary.find(key.toLower());
    if(iterator == dictionary.end()) continue;

    const QString trimmed = iterator->second.trimmed();
    if(!trimmed.isEmpty() && trimmed != QString("?") && trimmed != QString("."))
    {
      return trimmed;
    }
  }
  return std::nullopt;
}

std::optional<QString> SKCIFParser::normalizedChemicalElement(const std::optional<QString> &symbol)
{
  if(!symbol) return std::nullopt;

  const QString strippableCharacters = QString("01234567890.+-");
  QString chemicalElement = symbol->trimmed();
  while(!chemicalElement.isEmpty() && strippableCharacters.contains(chemicalElement.at(0)))
  {
    chemicalElement.remove(0, 1);
  }
  while(!chemicalElement.isEmpty() && strippableCharacters.contains(chemicalElement.at(chemicalElement.size() - 1)))
  {
    chemicalElement.chop(1);
  }
  if(chemicalElement.isEmpty()) return std::nullopt;

  chemicalElement = chemicalElement.toLower();
  chemicalElement.replace(0, 1, chemicalElement[0].toUpper());
  return chemicalElement;
}

std::optional<double> SKCIFParser::parseCIFDouble(const std::optional<QString> &string)
{
  if(!string || string->isEmpty()) return std::nullopt;

  bool success = false;
  const double value = string->split('(').at(0).trimmed().toDouble(&success);
  if(!success) return std::nullopt;
  return value;
}

bool SKCIFParser::isWaterResidue(const QString &residueName)
{
  return residueName == QString("HOH") || residueName == QString("DOD") ||
         residueName == QString("WAT") || residueName == QString("H2O");
}

bool SKCIFParser::isSolventAgentResidue(const QString &residueName)
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

void SKCIFParser::skipComment()
{
  QString tempString;
  _scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(),tempString);
}

qint64 SKCIFParser::scanInt()
{
  QString tempString;
  if (_scanner.scanUpToCharacters(CharacterSet::whitespaceAndNewlineCharacterSet(), tempString))
  {
    bool success = false;
    return tempString.split('(').at(0).trimmed().toLongLong(&success);
  }
  return 0;
}

double SKCIFParser::scanDouble()
{
  QString tempString;
  if (_scanner.scanUpToCharacters(CharacterSet::whitespaceAndNewlineCharacterSet(),tempString))
  {
    bool success = false;
    return tempString.split('(').at(0).toDouble(&success);
  }
  return 0.0;
}

std::optional<QString> SKCIFParser::scanString()
{
  QString tempString;
  if (_scanner.scanUpToCharacters(CharacterSet::newlineCharacterSet(),tempString))
  {
    return tempString;
  }

  return std::nullopt;
}
