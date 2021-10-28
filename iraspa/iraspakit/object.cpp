#include "iraspaobject.h"
#include <QDebug>

Object::Object(): _cell(std::make_shared<SKCell>())
{

}


Object::Object(const std::shared_ptr<const SKStructure> frame): _cell(frame->cell)
{
  std::optional<QString> displayName = frame->displayName;
  if(displayName)
  {
    _displayName = *displayName;
  }
  else
  {
    _displayName = "structure";
  }
}

std::shared_ptr<Object> Object::shallowClone()
{
  return std::make_shared<Object>(static_cast<const Object&>(*this));
}


// shallow copy, atoms/bonds are empty, spacegroup no symmetry
Object::Object(const Object &object): _cell(std::make_shared<SKCell>())
{
  _displayName = object._displayName;

  _cell = std::make_shared<SKCell>(*object._cell);
  _isVisible = object._isVisible;

  _origin = object._origin;
  _scaling = object._scaling;
  _orientation = object._orientation;
  _rotationDelta = object._rotationDelta;
  _periodic = object._periodic;

  // unit cell
  _drawUnitCell = object._drawUnitCell;
  _unitCellScaleFactor = object._unitCellScaleFactor;
  _unitCellDiffuseColor = object._unitCellDiffuseColor;
  _unitCellDiffuseIntensity = object._unitCellDiffuseIntensity;
}

Object::Object(const std::shared_ptr<const Object> object)
{
  _displayName = object->_displayName;

  _cell = std::make_shared<SKCell>(*object->_cell);
  _isVisible = object->_isVisible;

  _origin = object->_origin;
  _scaling = object->_scaling;
  _orientation = object->_orientation;
  _rotationDelta = object->_rotationDelta;
  _periodic = object->_periodic;

  // unit cell
  _drawUnitCell = object->_drawUnitCell;
  _unitCellScaleFactor = object->_unitCellScaleFactor;
  _unitCellDiffuseColor = object->_unitCellDiffuseColor;
  _unitCellDiffuseIntensity = object->_unitCellDiffuseIntensity;
}


// MARK: Bounding box
// =====================================================================

SKBoundingBox Object::boundingBox() const
{
  return SKBoundingBox();
}

SKBoundingBox Object::transformedBoundingBox() const
{
  SKBoundingBox currentBoundingBox = this->_cell->boundingBox();

  double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), currentBoundingBox.center());
  SKBoundingBox transformedBoundingBox = currentBoundingBox.adjustForTransformation(rotationMatrix);

  return transformedBoundingBox;
}

void Object::reComputeBoundingBox()
{

}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Object> &object)
{
  stream << object->_versionNumber;
  stream << object->_displayName;
  stream << object->_isVisible;

  stream << object->_cell;
  stream << object->_periodic;
  stream << object->_origin;
  stream << object->_scaling;
  stream << object->_orientation;
  stream << object->_rotationDelta;

  stream << object->_drawUnitCell;
  stream << object->_unitCellScaleFactor;
  stream << object->_unitCellDiffuseColor;
  stream << object->_unitCellDiffuseIntensity;

  stream << object->_localAxes;

  stream << object->_authorFirstName;
  stream << object->_authorMiddleName;
  stream << object->_authorLastName;
  stream << object->_authorOrchidID;
  stream << object->_authorResearcherID;
  stream << object->_authorAffiliationUniversityName;
  stream << object->_authorAffiliationFacultyName;
  stream << object->_authorAffiliationInstituteName;
  stream << object->_authorAffiliationCityName;
  stream << object->_authorAffiliationCountryName;

  stream << uint16_t(object->_creationDate.day());
  stream << uint16_t(object->_creationDate.month());
  stream << uint32_t(object->_creationDate.year());

  stream << qint64(0x6f6b6181);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Object> &object)
{
  uint16_t day,month;
  uint32_t year;
  qint64 versionNumber;
  stream >> versionNumber;

  if(versionNumber > object->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Object");
  }

  stream >> object->_displayName;
  stream >> object->_isVisible;

  stream >> object->_cell;
  stream >> object->_periodic;
  stream >> object->_origin;
  stream >> object->_scaling;
  stream >> object->_orientation;
  stream >> object->_rotationDelta;

  stream >> object->_drawUnitCell;
  stream >> object->_unitCellScaleFactor;
  stream >> object->_unitCellDiffuseColor;
  stream >> object->_unitCellDiffuseIntensity;

  stream >> object->_localAxes;

  stream >> object->_authorFirstName;
  stream >> object->_authorMiddleName;
  stream >> object->_authorLastName;
  stream >> object->_authorOrchidID;
  stream >> object->_authorResearcherID;
  stream >> object->_authorAffiliationUniversityName;
  stream >> object->_authorAffiliationFacultyName;
  stream >> object->_authorAffiliationInstituteName;
  stream >> object->_authorAffiliationCityName;
  stream >> object->_authorAffiliationCountryName;

  stream >> day;
  stream >> month;
  stream >> year;
  object->_creationDate = QDate(int(year),int(month),int(day));

  qint64 magicNumber;
  stream >> magicNumber;
  if(magicNumber != qint64(0x6f6b6181))
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Object invalid magic-number");
  }

  return stream;
}
