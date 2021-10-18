#include "vaspdensityvolume.h"

VASPDensityVolume::VASPDensityVolume()
{

}



QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<VASPDensityVolume> &volume)
{
  stream << volume->_versionNumber;

  // handle super class
  stream << std::static_pointer_cast<GridVolume>(volume);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<VASPDensityVolume> &volume)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "VASPDensityVolume");
  }

  std::shared_ptr<Object> object = std::static_pointer_cast<GridVolume>(volume);
  stream >> object;

  return stream;
}
