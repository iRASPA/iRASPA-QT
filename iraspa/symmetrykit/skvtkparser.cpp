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

#include <QtGlobal>
#include <QSysInfo>
#include <QFileInfo>
#include <QTextStream>
#include <sstream>
#include "skvtkparser.h"

SKVTKParser::SKVTKParser(QUrl url, QDataStream::ByteOrder byteorder, LogReporting *log): _url(url), _byteOrder(byteorder), _log(log), _frame(std::make_shared<SKStructure>())
{
  _frame->kind = SKStructure::Kind::densityVolume;
}

template <typename T> static QByteArray readData(QTextStream &stream, size_t numberOfElements)
{
  std::vector<T> dataPoints;
  dataPoints.reserve(numberOfElements);

  for (size_t i=0; i<numberOfElements; i++)
  {
    T dataPoint;
    QString line;
    stream.readLineInto(&line);
    std::stringstream ss(line.toStdString());
    ss >> dataPoint;
    dataPoints.push_back(dataPoint);
  }
  return QByteArray(reinterpret_cast<char*>(dataPoints.data()), dataPoints.size() * sizeof(T));
}

bool SKVTKParser::startParsing()
{
  QFile file(_url.toLocalFile());
  QFileInfo info(file);

  if (file.open(QIODevice::ReadOnly))
  {
    QTextStream stream(&file);

    QString line;
    stream.readLineInto(&line);
    if(line.startsWith("# vtk"))
    {
      const size_t maxHeaderLineCount = 20;
      std::vector<QString> headerData{};

      // read header
      bool pointDataFound = false;
      for (size_t i=0; i<maxHeaderLineCount; i++)
      {
        stream.readLineInto(&line);
        if (line.isEmpty())
        {
          file.close();
          if(_log)
          {
            _log->logMessage(LogReporting::ErrorLevel::error, "Cannot read VTK header");
          }
          return false;
        }
        else
        {
          headerData.push_back(line);
          if(line.toUpper().startsWith("LOOKUP_TABLE"))
          {
            pointDataFound = true;
            break;
          }
        }
      }

      for(const QString &headerString : headerData)
      {
        QString string = headerString.toUpper();
        if (string.startsWith("DATASET"))
        {
          QStringList strSplited = headerString.split(' ');
          if(strSplited.size() >= 2)
          {
            QString geometryType = strSplited[1].toUpper();
            if(geometryType != "STRUCTURED_POINTS")
            {
              file.close();
              if(_log)
              {
                _log->logMessage(LogReporting::ErrorLevel::error, "VTK DataSet type must be STRUCTURED_POINTS");
              }
              return false;
            }
          }
        }

        if (string.startsWith("DIMENSIONS"))
        {
          QStringList strSplited = headerString.split(' ');
          if(strSplited.size() >= 4)
          {
            _dimensions.x = strSplited[1].toInt();
            _dimensions.y = strSplited[2].toInt();
            _dimensions.z = strSplited[3].toInt();
          }
        }
        if (string.startsWith("ORIGIN"))
        {
          QStringList strSplited = headerString.split(' ');
          if(strSplited.size() >= 4)
          {
            _origin.x = strSplited[1].toDouble();
            _origin.y = strSplited[2].toDouble();
            _origin.z = strSplited[3].toDouble();
          }
        }
        if (string.startsWith("SPACING"))
        {
          QStringList strSplited = headerString.split(' ');
          if(strSplited.size() >= 4)
          {
            _spacing.x = strSplited[1].toDouble();
            _spacing.y = strSplited[2].toDouble();
            _spacing.z = strSplited[3].toDouble();
          }
        }
        if (string.startsWith("SCALARS"))
        {
          QStringList strSplited = headerString.split(' ');
          if(strSplited.size()>=3)
          {
            QString dataTypeString = strSplited[2].toLower();

            if (dataTypeString == "unsigned_char")
            {
             _dataType = SKStructure::DataType::Uint8;
            }
            else if (dataTypeString == "char")
            {
              _dataType = SKStructure::DataType::Int8;
            }
            else if (dataTypeString == "unsigned_short")
            {
              _dataType = SKStructure::DataType::Uint16;
            }
            else if (dataTypeString == "short")
            {
              _dataType = SKStructure::DataType::Int16;
            }
            else if (dataTypeString == "unsigned_int")
            {
              _dataType = SKStructure::DataType::Uint32;
            }
            else if (dataTypeString == "int")
            {
              _dataType = SKStructure::DataType::Int32;
            }
            else if (dataTypeString == "unsigned_long")
            {
              _dataType = SKStructure::DataType::Uint64;
            }
            else if (dataTypeString == "long")
            {
              _dataType = SKStructure::DataType::Int64;
            }
            else if (dataTypeString == "float")
            {
              _dataType = SKStructure::DataType::Float;
            }
            else if (dataTypeString == "double")
            {
              _dataType = SKStructure::DataType::Double;
            }
            else
            {
              file.close();
              if(_log)
              {
                _log->logMessage(LogReporting::ErrorLevel::error, "Cannot read VTK SCALARS type");
              }
              return false;
            }
          }
        }
        if (string.startsWith("UNIT_CELL"))
        {
          qDebug() << "Unit cell found";
          QStringList strSplited = headerString.split(' ');
          if(strSplited.size() >= 6)
          {
            _a = strSplited[1].toDouble();
            _b = strSplited[2].toDouble();
            _c = strSplited[3].toDouble();
            _alpha = strSplited[4].toDouble();
            _beta  = strSplited[5].toDouble();
            _gamma = strSplited[6].toDouble();
            _frame->cell = std::make_shared<SKCell>(_a, _b, _c, _alpha * M_PI/180.0, _beta*M_PI/180.0, _gamma*M_PI/180.0);
          }
        }
      }

      size_t numberOfElements = _dimensions.x * _dimensions.y * _dimensions.z;
      bool isBinary = false;
      for(const QString &headerString : headerData)
      {
        if (headerString.startsWith("BINARY"))
        {
          isBinary = true;
          break;
        }
        else if (headerString.startsWith("ASCII"))
        {
          isBinary = false;
          break;
        }
      }

      if(isBinary)
      {
        // skip newline and read the remainder as binary
        file.seek(stream.pos()+1);
        _byteData = file.readAll();
      }
      else
      {
        switch(_dataType)
        {
        case SKStructure::DataType::Uint8:
          _byteData = readData<uint8_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Int8:
          _byteData = readData<int8_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Uint16:
          _byteData = readData<uint16_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Int16:
          _byteData = readData<int16_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Uint32:
          _byteData = readData<uint32_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Int32:
          _byteData = readData<int32_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Uint64:
          _byteData = readData<uint64_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Int64:
          _byteData = readData<int64_t>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Float:
          _byteData = readData<float>(stream, numberOfElements);
          break;
        case SKStructure::DataType::Double:
          _byteData = readData<double>(stream, numberOfElements);
          break;
        }
        file.close();
      }

      _frame->displayName = info.baseName();
      _frame->dimensions = _dimensions;
      _frame->origin = _origin;
      _frame->spacing = _spacing;
      _frame->byteData = _byteData;
      _frame->dataType = _dataType;
      _frame->kind = SKStructure::Kind::densityVolume;
      _movies.push_back({_frame});
      return true;
    }
    if(_log)
    {
      _log->logMessage(LogReporting::ErrorLevel::error, "No VTK file found");
    }
    return false;
  }
  return false;
}

