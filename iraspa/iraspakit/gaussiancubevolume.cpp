#include "gaussiancubevolume.h"

GaussianCubeVolume::GaussianCubeVolume(): GridVolume()
{

}



QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<GaussianCubeVolume> &volume)
{
  stream << volume->_versionNumber;

  stream << qint64(0x6f6b6199);

  // handle super class
  stream << std::static_pointer_cast<GridVolume>(volume);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<GaussianCubeVolume> &volume)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "GaussianCubeVolume");
  }

  qint64 magicNumber;
  stream >> magicNumber;
  if(magicNumber != qint64(0x6f6b6199))
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "VASPDensityVolume invalid magic-number");
  }

  std::shared_ptr<GridVolume> object = std::static_pointer_cast<GridVolume>(volume);
  stream >> object;

  return stream;
}
