#include "gaussiancubevolume.h"

GaussianCubeVolume::GaussianCubeVolume()
{

}



QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<GaussianCubeVolume> &volume)
{
  stream << volume->_versionNumber;

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


  std::shared_ptr<Object> object = std::static_pointer_cast<GridVolume>(volume);
  stream >> object;

  return stream;
}
