#include "vtkdensityvolume.h"

VTKDensityVolume::VTKDensityVolume()
{

}


VTKDensityVolume::VTKDensityVolume(std::shared_ptr<SKStructure> frame): GridVolume(frame)
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

  std::vector<float4> dataGrid(_dimensions.x*_dimensions.y*_dimensions.z);

  switch(frame->dataType)
  {
  case SKStructure::DataType::Uint8:
    {
      auto *p = reinterpret_cast<uint8_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i])/float(std::numeric_limits<uint8_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Int8:
    {
      auto *p = reinterpret_cast<int8_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i]-std::numeric_limits<int8_t>::lowest())/float(std::numeric_limits<int8_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Uint16:
    {
      auto *p = reinterpret_cast<uint16_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i])/float(std::numeric_limits<uint16_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Int16:
    {
      auto *p = reinterpret_cast<int16_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i]-std::numeric_limits<int16_t>::lowest())/float(std::numeric_limits<int16_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Uint32:
    {
      auto *p = reinterpret_cast<uint32_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i])/float(std::numeric_limits<uint32_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Int32:
    {
      auto *p = reinterpret_cast<int32_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i]-std::numeric_limits<int32_t>::lowest())/float(std::numeric_limits<int32_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Uint64:
    {
      auto *p = reinterpret_cast<uint64_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i])/float(std::numeric_limits<uint16_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Int64:
    {
      auto *p = reinterpret_cast<int64_t*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i]-std::numeric_limits<int64_t>::lowest())/float(std::numeric_limits<int64_t>::max()),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Float:
    {
      auto *p = reinterpret_cast<float*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i]),0.0,0.0,0.0);
      }
    }
    break;
  case SKStructure::DataType::Double:
    {
      auto *p = reinterpret_cast<double*>(frame->byteData.data());
      for(int i=0;i<_dimensions.x * _dimensions.y * _dimensions.z; i++)
      {
        dataGrid[i] = float4(float(p[i]),0.0,0.0,0.0);
      }
    }
    break;
  }

  for(int z=0;z<_dimensions.z;z++)
  {
    for(int y=0;y<_dimensions.y;y++)
    {
      for(int x=0;x<_dimensions.x;x++)
      {
        float value = Images(dataGrid,x,y,z);

        // x
        int xi = (int)(x + 0.5f);
        float xf = x + 0.5f - xi;
        float xd0 = Images(dataGrid,x-1,y,z);
        float xd1 = Images(dataGrid,x,y,z);
        float xd2 = Images(dataGrid,x+1,y,z);
        float gx = (xd1 - xd0) * (1.0f - xf) + (xd2 - xd1) * xf; // lerp

        // y
        int yi = (int)(y + 0.5f);
        float yf = y + 0.5f - yi;
        float yd0 = Images(dataGrid,x,y-1,z);
        float yd1 = Images(dataGrid,x,y,z);
        float yd2 = Images(dataGrid,x,y+1,z);
        float gy = (yd1 - yd0) * (1.0f - yf) + (yd2 - yd1) * yf; // lerp

        // z
        int zi = (int)(z + 0.5f);
        float zf = z + 0.5f - zi;
        float zd0 =  Images(dataGrid,x,y,z-1);
        float zd1 =  Images(dataGrid,x,y,z);
        float zd2 =  Images(dataGrid,x,y,z+1);
        float gz =  (zd1 - zd0) * (1.0f - zf) + (zd2 - zd1) * zf; // lerp

        dataGrid[x+y*_dimensions.y+z*_dimensions.x*_dimensions.y] = float4(value, gx, gy, gz);
      }
    }
  }

  _data =  QByteArray(reinterpret_cast<const char*>(dataGrid.data()), dataGrid.size()*sizeof(float4));
}


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<VTKDensityVolume> &volume)
{
  stream << volume->_versionNumber;

  // handle super class
  stream << std::static_pointer_cast<GridVolume>(volume);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<VTKDensityVolume> &volume)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > volume->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "VTKDensityVolume");
  }


  std::shared_ptr<Object> object = std::static_pointer_cast<GridVolume>(volume);
  stream >> object;

  return stream;
}
