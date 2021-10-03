#include "iraspaobject.h"
#include <QDebug>

Object::Object(): _cell(std::make_shared<SKCell>())
{

}


Object::Object(std::shared_ptr<SKStructure> frame): _cell(frame->cell)
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

// shallow copy, atoms/bonds are empty, spacegroup no symmetry
Object::Object(const Structure &structure): _cell(std::make_shared<SKCell>())
{
  _displayName = structure._displayName;

  _cell = std::make_shared<SKCell>(*structure._cell);
  _isVisible = structure._isVisible;

  _origin = structure._origin;
  _scaling = structure._scaling;
  _orientation = structure._orientation;
  _rotationDelta = structure._rotationDelta;
  _periodic = structure._periodic;

  // unit cell
  _drawUnitCell = structure._drawUnitCell;
  _unitCellScaleFactor = structure._unitCellScaleFactor;
  _unitCellDiffuseColor = structure._unitCellDiffuseColor;
  _unitCellDiffuseIntensity = structure._unitCellDiffuseIntensity;
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

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Object> &structure)
{
  stream << structure->_versionNumber;
  stream << structure->_displayName;
  stream << structure->_isVisible;

  stream << structure->_cell;
  stream << structure->_periodic;
  stream << structure->_origin;
  stream << structure->_scaling;
  stream << structure->_orientation;
  stream << structure->_rotationDelta;

  stream << structure->_drawUnitCell;
  stream << structure->_unitCellScaleFactor;
  stream << structure->_unitCellDiffuseColor;
  stream << structure->_unitCellDiffuseIntensity;

  stream << structure->_localAxes;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Object> &structure)
{
  qint64 versionNumber;
  stream >> versionNumber;

  if(versionNumber > structure->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Object");
  }

  stream >> structure->_displayName;
  stream >> structure->_isVisible;

  stream >> structure->_cell;
  stream >> structure->_periodic;
  stream >> structure->_origin;
  stream >> structure->_scaling;
  stream >> structure->_orientation;
  stream >> structure->_rotationDelta;

  stream >> structure->_drawUnitCell;
  stream >> structure->_unitCellScaleFactor;
  stream >> structure->_unitCellDiffuseColor;
  stream >> structure->_unitCellDiffuseIntensity;

  stream >> structure->_localAxes;

  return stream;
}
