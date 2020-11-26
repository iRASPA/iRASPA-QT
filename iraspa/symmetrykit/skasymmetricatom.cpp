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

#include "skasymmetricatom.h"

SKAsymmetricAtom::SKAsymmetricAtom(): _displayName("C"), _elementIdentifier(6)
{

}

SKAsymmetricAtom::SKAsymmetricAtom(QString displayName, int elementIdentifier): _displayName(displayName), _elementIdentifier(elementIdentifier)
{

}

SKAsymmetricAtom::SKAsymmetricAtom(const SKAsymmetricAtom &asymmetricAtom)
{
  _versionNumber = asymmetricAtom._versionNumber;
  _asymmetricIndex = asymmetricAtom._asymmetricIndex;
  _displayName = asymmetricAtom._displayName;
  _position = asymmetricAtom._position;
  _charge = asymmetricAtom._charge;
  _hybridization = asymmetricAtom._hybridization;
  _uniqueForceFieldName = asymmetricAtom._uniqueForceFieldName;
  _elementIdentifier = asymmetricAtom._elementIdentifier;
  _color = asymmetricAtom._color;
  _drawRadius = asymmetricAtom._drawRadius;

  _bondDistanceCriteria = asymmetricAtom._bondDistanceCriteria;
  _potentialParameters = asymmetricAtom._potentialParameters;
  _tag = asymmetricAtom._tag;
  _isFixed = asymmetricAtom._isFixed;
  _isVisible = asymmetricAtom._isVisible;
  _isVisibleEnabled = asymmetricAtom._isVisibleEnabled;

  _serialNumber = asymmetricAtom._serialNumber;
  _remotenessIndicator = asymmetricAtom._remotenessIndicator;
  _branchDesignator = asymmetricAtom._branchDesignator;
  _asymetricID = asymmetricAtom._asymetricID;
  _alternateLocationIndicator = asymmetricAtom._alternateLocationIndicator;
  _residueName = asymmetricAtom._residueName;
  _chainIdentifier = asymmetricAtom._chainIdentifier;
  _residueSequenceNumber = asymmetricAtom._residueSequenceNumber;
  _codeForInsertionOfResidues = asymmetricAtom._codeForInsertionOfResidues;
  _occupancy = asymmetricAtom._occupancy;
  _temperaturefactor = asymmetricAtom._temperaturefactor;
  _segmentIdentifier = asymmetricAtom._segmentIdentifier;

  _ligandAtom = asymmetricAtom._ligandAtom;
  _backBoneAtom = asymmetricAtom._backBoneAtom;
  _fractional = asymmetricAtom._fractional;
  _solvent = asymmetricAtom._solvent;

  _copies = std::vector<std::shared_ptr<SKAtomCopy>>{};
}

SKAsymmetricAtom::~SKAsymmetricAtom()
{
}

void SKAsymmetricAtom::toggleVisibility()
{
  _isVisible = !_isVisible;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<SKAsymmetricAtom> &asymmetricAtom)
{
  stream << asymmetricAtom->_versionNumber;
  stream << asymmetricAtom->_asymmetricIndex;
  stream << asymmetricAtom->_displayName;
  stream << asymmetricAtom->_position;
  stream << asymmetricAtom->_charge;
  stream << static_cast<typename std::underlying_type<SKAsymmetricAtom::Hybridization>::type>(asymmetricAtom->_hybridization);
  stream << asymmetricAtom->_uniqueForceFieldName;
  stream << asymmetricAtom->_elementIdentifier;
  stream << asymmetricAtom->_color;
  stream << asymmetricAtom->_drawRadius;

  stream << asymmetricAtom->_bondDistanceCriteria;
  stream << asymmetricAtom->_potentialParameters;
  stream << asymmetricAtom->_tag;
  stream << asymmetricAtom->_isFixed;
  stream << asymmetricAtom->_isVisible;
  stream << asymmetricAtom->_isVisibleEnabled;

  stream << asymmetricAtom->_serialNumber;
  stream << asymmetricAtom->_remotenessIndicator;
  stream << asymmetricAtom->_branchDesignator;
  stream << asymmetricAtom->_asymetricID;
  stream << asymmetricAtom->_alternateLocationIndicator;
  stream << asymmetricAtom->_residueName;
  stream << asymmetricAtom->_chainIdentifier;
  stream << asymmetricAtom->_residueSequenceNumber;
  stream << asymmetricAtom->_codeForInsertionOfResidues;
  stream << asymmetricAtom->_occupancy;
  stream << asymmetricAtom->_temperaturefactor;
  stream << asymmetricAtom->_segmentIdentifier;

  stream << asymmetricAtom->_ligandAtom;
  stream << asymmetricAtom->_backBoneAtom;
  stream << asymmetricAtom->_fractional;
  stream << asymmetricAtom->_solvent;

  stream << asymmetricAtom->_copies;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<SKAsymmetricAtom> &asymmetricAtom)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > asymmetricAtom->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKAsymmetricAtom");
  }

  stream >> asymmetricAtom->_asymmetricIndex;
  stream >> asymmetricAtom->_displayName;
  stream >> asymmetricAtom->_position;
  stream >> asymmetricAtom->_charge;
  if(versionNumber >= 2)
  {
    qint64 hybridization;
    stream >> hybridization;
    asymmetricAtom->_hybridization = SKAsymmetricAtom::Hybridization(hybridization);
  }
  stream >> asymmetricAtom->_uniqueForceFieldName;
  stream >> asymmetricAtom->_elementIdentifier;
  stream >> asymmetricAtom->_color;
  stream >> asymmetricAtom->_drawRadius;

  stream >> asymmetricAtom->_bondDistanceCriteria;
  stream >> asymmetricAtom->_potentialParameters;
  stream >> asymmetricAtom->_tag;
  stream >> asymmetricAtom->_isFixed;
  stream >> asymmetricAtom->_isVisible;
  stream >> asymmetricAtom->_isVisibleEnabled;

  stream >> asymmetricAtom->_serialNumber;
  stream >> asymmetricAtom->_remotenessIndicator;
  stream >> asymmetricAtom->_branchDesignator;
  stream >> asymmetricAtom->_asymetricID;
  stream >> asymmetricAtom->_alternateLocationIndicator;
  stream >> asymmetricAtom->_residueName;
  stream >> asymmetricAtom->_chainIdentifier;
  stream >> asymmetricAtom->_residueSequenceNumber;
  stream >> asymmetricAtom->_codeForInsertionOfResidues;
  stream >> asymmetricAtom->_occupancy;
  stream >> asymmetricAtom->_temperaturefactor;
  stream >> asymmetricAtom->_segmentIdentifier;

  stream >> asymmetricAtom->_ligandAtom;
  stream >> asymmetricAtom->_backBoneAtom;
  stream >> asymmetricAtom->_fractional;
  stream >> asymmetricAtom->_solvent;

  stream >> asymmetricAtom->_copies;

  for(std::shared_ptr<SKAtomCopy> atom: asymmetricAtom->_copies)
  {
    atom->_parent = asymmetricAtom;
  }
  return stream;
}
