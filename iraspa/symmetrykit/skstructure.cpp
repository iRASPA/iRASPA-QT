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

#include "skstructure.h"
#include <QStringList>
#include <algorithm>
#include <unordered_set>

namespace
{
  const std::unordered_set<int> ignoredAtomicNumbers = {1, 2, 10, 18, 36, 54, 86};
  const std::unordered_set<int> extraFrameworkAtomicNumbers = {3, 11, 12, 19, 20, 37, 38, 55, 56};
  const std::unordered_set<int> halogenAtomicNumbers = {9, 17, 35, 53, 85};
  const std::unordered_set<int> zifMetalAtomicNumbers = {27, 28, 30, 48};
  const std::unordered_set<int> nonmetalAtomicNumbers = {
    1, 2, 5, 6, 7, 8, 9, 10, 14, 15, 16, 17, 18, 33, 34, 35, 36, 52, 53, 54, 85, 86
  };

  std::unordered_set<int> frameworkMetals(const std::unordered_set<int> &present)
  {
    std::unordered_set<int> metals;
    for (int z : present)
    {
      if (nonmetalAtomicNumbers.find(z) == nonmetalAtomicNumbers.end())
      {
        metals.insert(z);
      }
    }
    return metals;
  }

  bool hasNonAlFrameworkMetal(const std::unordered_set<int> &present)
  {
    for (int z : present)
    {
      if (z != 13 &&
          nonmetalAtomicNumbers.find(z) == nonmetalAtomicNumbers.end() &&
          extraFrameworkAtomicNumbers.find(z) == extraFrameworkAtomicNumbers.end() &&
          halogenAtomicNumbers.find(z) == halogenAtomicNumbers.end())
      {
        return true;
      }
    }
    return false;
  }

  bool isCarbonMaterial(const std::vector<int> &elementIdentifiers)
  {
    const std::unordered_set<int> allowed = {6, 8, 9, 17};
    std::unordered_set<int> present;
    for (int z : elementIdentifiers)
    {
      if (z > 1 && ignoredAtomicNumbers.find(z) == ignoredAtomicNumbers.end())
      {
        present.insert(z);
      }
    }
    if (present.find(6) == present.end())
    {
      return false;
    }
    for (int z : present)
    {
      if (allowed.find(z) == allowed.end())
      {
        return false;
      }
    }
    int nC = int(std::count(elementIdentifiers.begin(), elementIdentifiers.end(), 6));
    int nH = int(std::count(elementIdentifiers.begin(), elementIdentifiers.end(), 1));
    return nH == 0 || nC >= nH * 2;
  }

  bool isAllSilicaComposition(const std::vector<int> &elementIdentifiers)
  {
    std::unordered_set<int> present;
    for (int z : elementIdentifiers)
    {
      if (z > 0 && ignoredAtomicNumbers.find(z) == ignoredAtomicNumbers.end())
      {
        present.insert(z);
      }
    }
    return present.find(14) != present.end() &&
           present.find(8) != present.end() &&
           present.find(13) == present.end() &&
           present.find(15) == present.end();
  }

  bool isMesoporousSilicaName(const std::vector<QString> &names)
  {
    QStringList list;
    for (const QString &name : names)
    {
      list.append(name);
    }
    QString joined = list.join(" ").toUpper();
    if (joined.isEmpty())
    {
      return false;
    }
    const std::vector<QString> tokens = {"MCM", "SBA", "FSM", "HMS", "KIT-6", "KIT6", "FDU", "TUD-1", "MESOPOROUS"};
    for (const QString &token : tokens)
    {
      if (joined.contains(token))
      {
        return true;
      }
    }
    return false;
  }

  std::optional<SKStructure::MaterialType> inferFromNames(const std::vector<QString> &names)
  {
    QStringList list;
    for (const QString &name : names)
    {
      list.append(name);
    }
    QString joined = list.join(" ").toUpper();
    if (joined.isEmpty())
    {
      return std::nullopt;
    }

    if (isMesoporousSilicaName(names)) { return SKStructure::MaterialType::silica; }
    if (joined.contains("ZIF")) { return SKStructure::MaterialType::zif; }
    if (joined.contains("COF")) { return SKStructure::MaterialType::cof; }
    if (joined.contains("MOF")) { return SKStructure::MaterialType::mof; }
    if (joined.contains("SAPO") || joined.contains("SILICOALUMINOPHOSPHATE")) { return SKStructure::MaterialType::silicoaluminophosphate; }
    if (joined.contains("ALPO") || joined.contains("ALUMINOPHOSPHATE")) { return SKStructure::MaterialType::aluminophosphate; }
    if (joined.contains("ZEOLITE")) { return SKStructure::MaterialType::zeolite; }
    return std::nullopt;
  }

  SKStructure::MaterialType inferComposition(const std::vector<int> &elementIdentifiers)
  {
    std::unordered_set<int> present;
    for (int z : elementIdentifiers)
    {
      if (z > 0 && ignoredAtomicNumbers.find(z) == ignoredAtomicNumbers.end())
      {
        present.insert(z);
      }
    }
    if (present.empty())
    {
      return SKStructure::MaterialType::unspecified;
    }

    bool hasC = present.find(6) != present.end();
    bool hasN = present.find(7) != present.end();
    bool hasO = present.find(8) != present.end();
    bool hasB = present.find(5) != present.end();
    bool hasAl = present.find(13) != present.end();
    bool hasSi = present.find(14) != present.end();
    bool hasP = present.find(15) != present.end();
    bool hasGe = present.find(32) != present.end();
    bool hasGa = present.find(31) != present.end();
    bool hasAs = present.find(33) != present.end();
    bool hasBe = present.find(4) != present.end();

    bool isZeoliteFamily = hasO && (
      hasSi ||
      (hasAl && hasP) ||
      (hasAl && hasSi) ||
      hasGe || hasGa || hasAs || hasBe ||
      (hasB && !hasC)
    );

    if (isZeoliteFamily)
    {
      if (hasSi && hasAl && hasP)
      {
        return SKStructure::MaterialType::silicoaluminophosphate;
      }
      if (hasAl && hasP && !hasSi)
      {
        if (hasNonAlFrameworkMetal(present))
        {
          return SKStructure::MaterialType::metallophosphate;
        }
        return SKStructure::MaterialType::aluminophosphate;
      }
      if (hasSi && hasAl && !hasP)
      {
        return SKStructure::MaterialType::aluminosilicate;
      }
      if (hasSi && hasO && !hasAl && !hasP)
      {
        return SKStructure::MaterialType::zeolite;
      }
      if (hasP && hasO && hasNonAlFrameworkMetal(present))
      {
        return SKStructure::MaterialType::metallophosphate;
      }
      return SKStructure::MaterialType::zeolite;
    }

    if (hasC)
    {
      std::unordered_set<int> metals = frameworkMetals(present);
      if (metals.empty())
      {
        if (hasB || hasN)
        {
          return SKStructure::MaterialType::cof;
        }
        if (isCarbonMaterial(elementIdentifiers))
        {
          return SKStructure::MaterialType::carbon;
        }
        return SKStructure::MaterialType::molecularCrystal;
      }
      bool hasZifMetal = false;
      for (int m : metals)
      {
        if (zifMetalAtomicNumbers.find(m) != zifMetalAtomicNumbers.end())
        {
          hasZifMetal = true;
          break;
        }
      }
      if (hasZifMetal && hasN && !hasSi && !hasAl && !hasP)
      {
        return SKStructure::MaterialType::zif;
      }
      return SKStructure::MaterialType::mof;
    }

    if (hasO && !frameworkMetals(present).empty())
    {
      return SKStructure::MaterialType::oxide;
    }

    if (hasO)
    {
      return SKStructure::MaterialType::molecularCrystal;
    }

    return SKStructure::MaterialType::unspecified;
  }
}

QString SKStructure::materialTypeDisplayName(MaterialType type)
{
  switch (type)
  {
  case MaterialType::unspecified: return "Unspecified";
  case MaterialType::molecule: return "Molecule";
  case MaterialType::protein: return "Protein";
  case MaterialType::dnaRna: return "DNA/RNA";
  case MaterialType::molecularCrystal: return "Molecular crystal";
  case MaterialType::silica: return "Silica";
  case MaterialType::aluminosilicate: return "Aluminosilicate";
  case MaterialType::aluminophosphate: return "Aluminophosphate";
  case MaterialType::metallophosphate: return "Metallophosphate";
  case MaterialType::silicoaluminophosphate: return "Silicoaluminophosphate";
  case MaterialType::zeolite: return "Zeolite";
  case MaterialType::mof: return "MOF";
  case MaterialType::zif: return "ZIF";
  case MaterialType::cof: return "COF";
  case MaterialType::carbon: return "Carbon";
  case MaterialType::oxide: return "Oxide";
  case MaterialType::hof: return "HOF";
  case MaterialType::paf: return "PAF";
  case MaterialType::pim: return "PIM";
  case MaterialType::polymer: return "Polymer";
  case MaterialType::ionicLiquid: return "Ionic liquid";
  case MaterialType::clay: return "Clay";
  case MaterialType::perovskite: return "Perovskite";
  case MaterialType::alloy: return "Alloy";
  case MaterialType::glass: return "Glass";
  }
  return "Unspecified";
}

std::vector<QString> SKStructure::allMaterialTypeDisplayNames()
{
  const std::vector<MaterialType> order = {
    MaterialType::unspecified, MaterialType::molecule, MaterialType::protein, MaterialType::dnaRna, MaterialType::molecularCrystal,
    MaterialType::silica, MaterialType::aluminosilicate, MaterialType::aluminophosphate, MaterialType::metallophosphate, MaterialType::silicoaluminophosphate, MaterialType::zeolite,
    MaterialType::mof, MaterialType::zif, MaterialType::cof,
    MaterialType::carbon, MaterialType::oxide,
    MaterialType::hof, MaterialType::paf, MaterialType::pim, MaterialType::polymer, MaterialType::ionicLiquid, MaterialType::clay, MaterialType::perovskite, MaterialType::alloy, MaterialType::glass
  };
  std::vector<QString> names;
  names.reserve(order.size());
  for (MaterialType type : order)
  {
    names.push_back(materialTypeDisplayName(type));
  }
  return names;
}

std::optional<SKStructure::MaterialType> SKStructure::materialTypeFromDisplayName(const QString &name)
{
  QString trimmed = name.trimmed();
  if (trimmed.compare("Silicialuminophosphate", Qt::CaseInsensitive) == 0)
  {
    return MaterialType::silicoaluminophosphate;
  }
  for (int i = 0; i <= 24; ++i)
  {
    MaterialType candidate = static_cast<MaterialType>(i);
    if (materialTypeDisplayName(candidate).compare(trimmed, Qt::CaseInsensitive) == 0)
    {
      return candidate;
    }
  }
  return std::nullopt;
}

bool SKStructure::usesAluminosilicateForceField(MaterialType type)
{
  switch (type)
  {
  case MaterialType::zeolite:
  case MaterialType::silica:
  case MaterialType::aluminosilicate:
  case MaterialType::aluminophosphate:
  case MaterialType::metallophosphate:
  case MaterialType::silicoaluminophosphate:
    return true;
  default:
    return false;
  }
}

SKStructure::MaterialType SKStructure::inferMaterialType(const std::vector<int> &elementIdentifiers, Kind kind, const std::vector<QString> &names)
{
  switch (kind)
  {
  case Kind::protein:
  case Kind::proteinCrystal:
    return MaterialType::protein;
  case Kind::dna:
  case Kind::dnaCrystal:
    return MaterialType::dnaRna;
  case Kind::molecule:
    return MaterialType::molecule;
  case Kind::molecularCrystal:
  case Kind::molecularCrystalSolvent:
    return MaterialType::molecularCrystal;
  default:
    break;
  }

  if (isMesoporousSilicaName(names) && isAllSilicaComposition(elementIdentifiers))
  {
    return MaterialType::silica;
  }

  MaterialType composition = inferComposition(elementIdentifiers);
  if (composition != MaterialType::unspecified)
  {
    return composition;
  }
  return inferFromNames(names).value_or(MaterialType::unspecified);
}

void SKStructure::applyInferredMaterialType(const std::vector<QString> &extraNames, std::optional<Kind> kindOverride)
{
  std::vector<QString> names = extraNames;
  if (displayName && !displayName->isEmpty())
  {
    names.push_back(*displayName);
  }
  std::vector<int> elementIdentifiers;
  elementIdentifiers.reserve(atoms.size());
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    if (atom)
    {
      elementIdentifiers.push_back(atom->elementIdentifier());
    }
  }
  materialType = inferMaterialType(elementIdentifiers, kindOverride.value_or(this->kind), names);
}
