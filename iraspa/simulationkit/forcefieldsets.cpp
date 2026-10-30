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

#include "forcefieldsets.h"
#include <algorithm>
#include <cmath>

ForceFieldSets::ForceFieldSets(): _forceFieldSets{
  ForceFieldSet(),
  ForceFieldSet::aluminosilicate(),
  ForceFieldSet::zeoliteAtlas(),
  ForceFieldSet::aluminosilicateZeoPlusPlus()
}
{

}

QString ForceFieldSets::suggestedDisplayName(SKStructure::MaterialType materialType)
{
  return SKStructure::usesAluminosilicateForceField(materialType) ? ForceFieldSet::aluminosilicateDisplayName : ForceFieldSet::defaultDisplayName;
}

QString ForceFieldSets::suggestedDisplayNameForMaterialTypeName(const QString &name)
{
  SKStructure::MaterialType materialType = SKStructure::materialTypeFromDisplayName(name).value_or(SKStructure::MaterialType::unspecified);
  return suggestedDisplayName(materialType);
}

ForceFieldSet* ForceFieldSets::operator[] (const QString name)
{
  ensurePredefinedSets();
  for(ForceFieldSet& forceFieldSet: _forceFieldSets)
  {
    if(QString::compare(forceFieldSet.displayName(), name, Qt::CaseInsensitive) == 0)
    {
      return &forceFieldSet;
    }
  }

  return nullptr;
}

const ForceFieldSet* ForceFieldSets::operator[] (const QString name) const
{
  for(const ForceFieldSet& forceFieldSet: _forceFieldSets)
  {
    if(QString::compare(forceFieldSet.displayName(), name, Qt::CaseInsensitive) == 0)
    {
      return &forceFieldSet;
    }
  }

  return nullptr;
}

void ForceFieldSets::append(ForceFieldSet forceField)
{
  if(firstIndexNamed(forceField.displayName()).has_value() &&
     (ForceFieldSet::isAluminosilicateFamily(forceField.displayName()) ||
      QString::compare(forceField.displayName(), ForceFieldSet::defaultDisplayName, Qt::CaseInsensitive) == 0))
  {
    return;
  }
  _forceFieldSets.push_back(forceField);
}

ForceFieldSet ForceFieldSets::resolvedSet(const QString &displayName)
{
  ensurePredefinedSets();
  if(ForceFieldSet::isAluminosilicateFamily(displayName))
  {
    return ForceFieldSet::predefined(displayName);
  }
  for(const ForceFieldSet &set : _forceFieldSets)
  {
    if(QString::compare(set.displayName(), displayName, Qt::CaseInsensitive) == 0)
    {
      return set;
    }
  }
  return ForceFieldSet::predefined(displayName);
}

std::optional<size_t> ForceFieldSets::firstIndexNamed(const QString &name) const
{
  for(size_t i = 0; i < _forceFieldSets.size(); ++i)
  {
    if(QString::compare(_forceFieldSets[i].displayName(), name, Qt::CaseInsensitive) == 0)
    {
      return i;
    }
  }
  return std::nullopt;
}

void ForceFieldSets::ensurePredefinedSets()
{
  _forceFieldSets.erase(
    std::remove_if(_forceFieldSets.begin(), _forceFieldSets.end(), [](const ForceFieldSet &s) {
      const QString name = s.displayName();
      return QString::compare(name, QString("Aluminosilicate"), Qt::CaseInsensitive) == 0 ||
             QString::compare(name, QString("Zeolite Atlas"), Qt::CaseInsensitive) == 0 ||
             QString::compare(name, QString("Aluminosilicate Zeolite Atlas"), Qt::CaseInsensitive) == 0;
    }),
    _forceFieldSets.end()
  );

  const std::vector<std::pair<QString, std::function<ForceFieldSet()>>> predefined = {
    {ForceFieldSet::aluminosilicateDisplayName, ForceFieldSet::aluminosilicate},
    {ForceFieldSet::zeoliteAtlasDisplayName, ForceFieldSet::zeoliteAtlas},
    {ForceFieldSet::aluminosilicateZeoPlusPlusDisplayName, ForceFieldSet::aluminosilicateZeoPlusPlus}
  };

  for(size_t pIdx = 0; pIdx < predefined.size(); ++pIdx)
  {
    const QString &name = predefined[pIdx].first;
    const auto &factory = predefined[pIdx].second;

    std::vector<size_t> matches;
    for(size_t i = 0; i < _forceFieldSets.size(); ++i)
    {
      if(QString::compare(_forceFieldSets[i].displayName(), name, Qt::CaseInsensitive) == 0)
      {
        matches.push_back(i);
      }
    }

    if(!matches.empty())
    {
      _forceFieldSets[matches.front()] = factory();
      for(size_t k = matches.size(); k > 1; --k)
      {
        _forceFieldSets.erase(_forceFieldSets.begin() + static_cast<std::ptrdiff_t>(matches[k - 1]));
      }
    }
    else
    {
      std::optional<size_t> afterPrevious = std::nullopt;
      for(size_t prev = 0; prev < pIdx; ++prev)
      {
        if(std::optional<size_t> idx = firstIndexNamed(predefined[prev].first))
        {
          size_t candidate = *idx + 1;
          if(!afterPrevious || candidate > *afterPrevious)
          {
            afterPrevious = candidate;
          }
        }
      }
      size_t insertIndex = _forceFieldSets.size();
      if(afterPrevious)
      {
        insertIndex = *afterPrevious;
      }
      else if(std::optional<size_t> defIdx = firstIndexNamed(ForceFieldSet::defaultDisplayName))
      {
        insertIndex = *defIdx + 1;
      }
      insertIndex = std::min(insertIndex, _forceFieldSets.size());
      _forceFieldSets.insert(_forceFieldSets.begin() + static_cast<std::ptrdiff_t>(insertIndex), factory());
    }
  }

  restoreDefaultFrameworkTypesIfTraPPE();
}

void ForceFieldSets::restoreDefaultFrameworkTypesIfTraPPE()
{
  std::optional<size_t> defIdx = firstIndexNamed(ForceFieldSet::defaultDisplayName);
  if(!defIdx) return;
  ForceFieldSet &defaultSet = _forceFieldSets[*defIdx];
  ForceFieldType *oxygen = defaultSet[QString("O")];
  if(!oxygen) return;
  bool looksLikeTraPPEOxygen = std::fabs(oxygen->potentialParameters().x - 53.0) < 1.0e-6 &&
                               std::fabs(oxygen->potentialParameters().y - 3.30) < 1.0e-6;
  if(!looksLikeTraPPEOxygen) return;

  for(const QString &symbol : {QString("O"), QString("Si"), QString("Al")})
  {
    std::optional<ForceFieldType> canonical = ForceFieldSet::defaultType(symbol);
    if(!canonical) continue;
    for(ForceFieldType &ft : defaultSet.atomTypeList())
    {
      if(ft.forceFieldStringIdentifier() == symbol)
      {
        ft = *canonical;
        break;
      }
    }
  }
}

QDataStream &operator<<(QDataStream& stream, const std::vector<ForceFieldSet>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const ForceFieldSet& singleVal : val)
    stream << singleVal;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::vector<ForceFieldSet>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);
  ForceFieldSet tempVal;
  while(vecSize--)
  {
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}


QDataStream &operator<<(QDataStream &stream, const ForceFieldSets &forcefieldTypes)
{
  stream << forcefieldTypes._versionNumber;

  stream << forcefieldTypes._forceFieldSets;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, ForceFieldSets &forcefieldTypes)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > forcefieldTypes._versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "ForceFieldSets");
  }

  stream >> forcefieldTypes._forceFieldSets;
  forcefieldTypes.ensurePredefinedSets();

  return stream;
}

