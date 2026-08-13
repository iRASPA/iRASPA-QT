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

#pragma once

#include <QString>
#include <QUrl>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cwctype>
#include <cmath>
#include <optional>
#include <foundationkit.h>
#include "skparser.h"
#include "skasymmetricatom.h"
#include "skatomtreenode.h"
#include "skatomtreecontroller.h"
#include "skstructure.h"

class SKCIFParser: public SKParser
{
public:
  // 'separatePolymerChains' is kept for parity with the PDB reader; CIF/mmCIF has no TER records.
  SKCIFParser(QUrl url, bool onlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped, bool separatePolymerChains = false);
  void startParsing() noexcept(false) override final;
  std::optional<int> spaceGroupHallNumber() {return _spaceGroupHallNumber;}
private:
  struct ResidueKey
  {
    QChar chain;
    qint64 sequence;

    bool operator<(const ResidueKey &other) const
    {
      if(chain != other.chain) return chain < other.chain;
      return sequence < other.sequence;
    }
  };

  struct ResidueRecord
  {
    QString name{};
    bool hasNitrogen = false;
    bool hasAlphaCarbon = false;
    bool hasCarbonyl = false;
    bool water = false;
    bool nucleotide = false;
    double3 nitrogen = double3(0.0, 0.0, 0.0);
    double3 carbonyl = double3(0.0, 0.0, 0.0);
  };

  Scanner _scanner;
  bool _proteinOnlyAsymmetricUnitCell;
  bool _asMolecule;
  [[maybe_unused]] bool _separatePolymerChains;
  // shares the scanner buffer, needed to scan quoted values and multi-line text-fields by character
  QString _content;
  QString::const_iterator _previousScanLocation;
  std::optional<int> _spaceGroupHallNumber;
  int _numberOfAtoms = 0;
  int _numberOfAminoAcidAtoms = 0;
  int _numberOfNucleicAcidAtoms = 0;
  bool _proteinDetected = false;
  bool _dnaDetected = false;
  bool _cellLengthsDefined = false;
  double _a = 0.0;
  double _b = 0.0;
  double _c = 0.0;
  double _alpha = 90.0;
  double _beta = 90.0;
  double _gamma = 90.0;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> _atoms{};

  std::set<QChar> _polymerChains;
  std::set<QString> _modifiedResidues;
  std::map<ResidueKey, ResidueRecord> _residues;
  // maps '_struct_asym.id' to '_struct_asym.entity_id' for polymer-chain discovery
  std::map<QString, QString> _asymToEntity;
  // entity ids whose '_entity_poly.type' is a polypeptide or a nucleic acid
  std::set<QString> _polymerEntityIds;

  void parseAudit(QString& string);
  void parseiRASPA(QString& string);
  void parseChemical(QString& string);
  void parseCell(QString& string);
  void parseSymmetry(QString& string);
  void parseName(QString& string);
  void parseModResidue(QString& string);
  void parseLoop(QString& string);
  std::optional<QString> parseValue();
  void skipComment();
  qint64 scanInt();
  double scanDouble();
  std::optional<QString> scanString();

  void appendAtomSite(const std::map<QString,QString> &dictionary, const QString &chemicalSymbol);
  void recordEntityPolySeq(const std::map<QString,QString> &dictionary, const QString &monId);
  void recordEntityPolyType(const QString &entityId, const QString &polyType);
  void resolvePolymerChainsFromEntityTables();
  void noteResidueAtom(const std::shared_ptr<SKAsymmetricAtom> &atom);
  SKStructure::Kind kindOfCurrentPart();

  static std::optional<QString> dictionaryValue(const std::map<QString,QString> &dictionary, const std::vector<QString> &keys);
  static std::optional<QString> normalizedChemicalElement(const std::optional<QString> &symbol);
  static std::optional<double> parseCIFDouble(const std::optional<QString> &string);
  static bool isWaterResidue(const QString &residueName);
  static bool isSolventAgentResidue(const QString &residueName);
};
