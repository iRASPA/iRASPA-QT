#include "raspadensityvolume.h"

RASPADensityVolume::RASPADensityVolume(): GridVolume()
{

}

RASPADensityVolume::RASPADensityVolume(const std::shared_ptr<Object> object): GridVolume(object)
{

}

std::shared_ptr<Object> RASPADensityVolume::shallowClone()
{
  return std::make_shared<RASPADensityVolume>(static_cast<const RASPADensityVolume&>(*this));
}

RASPADensityVolume::RASPADensityVolume(std::shared_ptr<SKStructure> frame): GridVolume(frame)
{
  std::optional<QString> displayName = frame->displayName;
  if(displayName)
  {
    _displayName = *displayName;
  }
  _dimensions = frame->dimensions;
  _spacing = frame->spacing;
  _origin = frame->origin;
  _cell = frame->cell;

  std::vector<float> dataGrid(_dimensions.x*_dimensions.y*_dimensions.z);
  switch(frame->dataType)
  {
  case SKStructure::DataType::Uint8:
    {
      auto *p = reinterpret_cast<uint8_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i])/float(std::numeric_limits<uint8_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Int8:
    {
      auto *p = reinterpret_cast<int8_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i]-std::numeric_limits<int8_t>::lowest())/float(std::numeric_limits<int8_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Uint16:
    {
      auto *p = reinterpret_cast<uint16_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i])/float(std::numeric_limits<uint16_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Int16:
    {
      auto *p = reinterpret_cast<int16_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i]-std::numeric_limits<int16_t>::lowest())/float(std::numeric_limits<int16_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Uint32:
    {
      auto *p = reinterpret_cast<uint32_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i])/float(std::numeric_limits<uint32_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Int32:
    {
      auto *p = reinterpret_cast<int32_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i]-std::numeric_limits<int32_t>::lowest())/float(std::numeric_limits<int32_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Uint64:
    {
      auto *p = reinterpret_cast<uint64_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i])/float(std::numeric_limits<uint16_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Int64:
    {
      auto *p = reinterpret_cast<int64_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i]-std::numeric_limits<int64_t>::lowest())/float(std::numeric_limits<int64_t>::max()));
      }
    }
    break;
  case SKStructure::DataType::Float:
    {
      auto *p = reinterpret_cast<float*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i]));
      }
    }
    break;
  case SKStructure::DataType::Double:
    {
      auto *p = reinterpret_cast<double*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float(float(p[i]));
      }
    }
    break;
  }

  int largestSize = std::max({_dimensions.x,_dimensions.y,_dimensions.z});
  int k = 1;
  while(largestSize > pow(2,k))
  {
    k += 1;
  }
  _encompassingPowerOfTwoCubicGridSize = k;
  _adsorptionVolumeStepLength = 0.5 / pow(2,k);

  double maximumValue = std::numeric_limits<float>::lowest();
  double minimumValue = std::numeric_limits<float>::max();
  double sum=0.0;
  double sumSquared=0.0;
  size_t numberOfElements = _dimensions.x * _dimensions.y * _dimensions.z;
  for(size_t i=0;i<numberOfElements;i++)
  {
    float dataPoint = dataGrid[i];
    if(dataPoint>maximumValue) maximumValue = dataPoint;
    if(dataPoint<minimumValue) minimumValue = dataPoint;
    sum += dataPoint;
    sumSquared += dataPoint * dataPoint;
  }

  _average = sum / (numberOfElements);
  _variance = sumSquared / (numberOfElements - 1);

  _range = std::pair<double,double>(minimumValue, maximumValue);
  _adsorptionSurfaceIsoValue = _average;
  _adsorptionSurfaceRenderingMethod = RKEnergySurfaceType::isoSurface;
  _adsorptionVolumeTransferFunction = RKPredefinedVolumeRenderingTransferFunction::CoolWarmDiverging;
  _drawAdsorptionSurface = true;

  _data = QByteArray(reinterpret_cast<const char*>(dataGrid.data()), dataGrid.size()*sizeof(float));
}




QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<RASPADensityVolume> &volume)
{
  stream << volume->_versionNumber;

  stream << qint64(0x6f6b6196);

  // handle super class
  stream << std::static_pointer_cast<GridVolume>(volume);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<RASPADensityVolume> &volume)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "RASPADensityVolume");
  }

  qint64 magicNumber;
  stream >> magicNumber;
  if(magicNumber != qint64(0x6f6b6196))
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "RASPADensityVolume invalid magic-number");
  }

  std::shared_ptr<GridVolume> object = std::static_pointer_cast<GridVolume>(volume);
  stream >> object;

  return stream;
}
