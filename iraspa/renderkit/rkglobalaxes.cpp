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


#include "rkglobalaxes.h"
#include <algorithm>

RKGlobalAxes::RKGlobalAxes()
{
  setStyle(_style);
}

double RKGlobalAxes::totalAxesSize()
{
  return _axisScale + _centerScale + _textOffset + 2.0*std::max({_textScale.x, _textScale.y, _textScale.z}) + _axesBackgroundAdditionalSize;
}

void  RKGlobalAxes::setStyle(RKGlobalAxes::Style axesStyle)
{
  _style = axesStyle;
  qDebug() << "Style set " << int(axesStyle);
  switch(axesStyle)
  {
    case RKGlobalAxes::Style::defaultStyle:
      _HDR = true;
      _exposure = 1.5;

      _centerScale = 0.125;
      _textOffset = 1.0;

      _NumberOfSectors = 4;

      _shaftLength = 1.0;
      _shaftWidth = 0.125 * sqrt(2.0);

      _tipLength = 0.0;
      _tipWidth = 1.0/6.0;

      _centerType = RKGlobalAxes::CenterType::cube;
      _tipVisibility = false;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(102,178,255,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51,255);
      _axisXDiffuseColor = QColor(102,178,255,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51,255);
      _axisYDiffuseColor = QColor(102,178,255,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(102,178,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
      break;
    case RKGlobalAxes::Style::thickRGB:
      _HDR = true;
      _exposure = 1.5;

      _NumberOfSectors = 41;

      _centerScale = 0.125;
      _textOffset = 0.0;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 3.0/48.0;

      _tipLength = 1.0/3.0;
      _tipWidth = 3.0/24.0;

      _centerType = RKGlobalAxes::CenterType::cube;
      _tipVisibility = true;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,255,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51,255);
      _axisXDiffuseColor = QColor(255,0,0,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51, 255);
      _axisYDiffuseColor = QColor(0,255,0,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(0,0,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::thick:
      _HDR = true;
      _exposure = 1.5;

      _NumberOfSectors = 41;

      _centerScale = 0.125;
      _textOffset = 0.0;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 3.0/48.0;

      _tipLength = 1.0/3.0;
      _tipWidth = 3.0/24.0;

      _centerType = RKGlobalAxes::CenterType::cube;
      _tipVisibility = true;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,255,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51,255);
      _axisXDiffuseColor = QColor(255,102,178,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51,255);
      _axisYDiffuseColor = QColor(178,255,102,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(102,178,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::thinRGB:
      _HDR = true;
      _exposure = 1.5;

      _NumberOfSectors = 41;

      _centerScale = 0.0625;
      _textOffset = 0.0;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 1.0/24.0;

      _tipLength = 1.0/3.0;
      _tipWidth = 1.0/12.0;

      _centerType = RKGlobalAxes::CenterType::sphere;
      _tipVisibility = true;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51, 255);
      _centerDiffuseColor = QColor(255,255,255,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51,255);
      _axisXDiffuseColor = QColor(255,0,0,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51, 255);
      _axisYDiffuseColor = QColor(0,255,0,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(0,0,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::thin:
      _HDR = true;
      _exposure = 1.5;

      _NumberOfSectors = 41;

      _centerScale = 0.0625;
      _textOffset = 0.0;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 1.0/24.0;

      _tipLength = 1.0/3.0;
      _tipWidth = 1.0/12.0;

      _centerType = RKGlobalAxes::CenterType::sphere;
      _tipVisibility = true;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,255,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51, 255);
      _axisXDiffuseColor = QColor(255,102,178,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51, 255);
      _axisYDiffuseColor = QColor(178,255,102,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(102,178,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::beamArrowRGB:
      _HDR = true;
      _exposure = 1.5;

      _centerScale = 0.125;
      _textOffset = 0.0;

      _NumberOfSectors = 4;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 1.0/12.0;

      _tipLength = 1.0/3.0;
      _tipWidth = 1.0/6.0;

      _centerType = RKGlobalAxes::CenterType::cube;
      _tipVisibility = true;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,0,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51,255);
      _axisXDiffuseColor = QColor(255,0,0,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51,255);
      _axisYDiffuseColor = QColor(0,255,0,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51,255);
      _axisZDiffuseColor = QColor(0,0,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::beamArrow:
      _HDR = true;
      _exposure = 1.5;

      _centerScale = 0.125;
      _textOffset = 0.0;

      _NumberOfSectors = 4;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 1.0/12.0;

      _tipLength = 1.0/3.0;
      _tipWidth = 1.0/6.0;

      _centerType = RKGlobalAxes::CenterType::cube;
      _tipVisibility = true;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,0,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51, 255);
      _axisXDiffuseColor = QColor(255,102,178,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51, 255);
      _axisYDiffuseColor = QColor(178,255,102,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(102,178,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::beamRGB:
      _HDR = true;
      _exposure = 1.5;

      _centerScale = 0.125;
      _textOffset = 0.5;

      _NumberOfSectors = 4;

      _shaftLength = 1.0;
      _shaftWidth = 0.125; // * sqrt(2.0)

      _tipLength = 0.0;
      _tipWidth = 1.0/6.0;

      _centerType = RKGlobalAxes::CenterType::cube;
      _tipVisibility = false;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,0,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51,255);
      _axisXDiffuseColor = QColor(255,0,0,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51,255);
      _axisYDiffuseColor = QColor(0,255,0,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51,255);
      _axisZDiffuseColor = QColor(0,0,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::beam:
      _HDR = true;
      _exposure = 1.5;

      _centerScale = 0.125;
      _textOffset = 0.5;

      _NumberOfSectors = 4;

      _shaftLength = 1.0;
      _shaftWidth = 0.125; // * sqrt(2.0)

      _tipLength = 0.0;
      _tipWidth = 1.0/6.0;

      _centerType = RKGlobalAxes::CenterType::cube;
      _tipVisibility = false;

      _aspectRatio = 1.0;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,0,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51, 255);
      _axisXDiffuseColor = QColor(255,102,178,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51,255);
      _axisYDiffuseColor = QColor(178,255,102,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(102,178,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::squashedRGB:
      _HDR = true;
      _exposure = 1.5;

      _NumberOfSectors = 41;

      _centerScale = 0.125;
      _textOffset = 0.0;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 3.0/48.0/2.5;

      _tipLength = 1.0/3.0;
      _tipWidth = 3.0/24.0/2.5;

      _centerType = RKGlobalAxes::CenterType::sphere;
      _tipVisibility = true;

      _aspectRatio = 0.25;

      _centerAmbientColor = QColor(051,51,51,255);
      _centerDiffuseColor = QColor(255,255,255,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(51,51,51,255);
      _axisXDiffuseColor = QColor(255,0,0,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51,255);
      _axisYDiffuseColor = QColor(0,255,0,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51,255);
      _axisZDiffuseColor = QColor(0,0,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    case RKGlobalAxes::Style::squashed:
      _HDR = true;
      _exposure = 1.5;

      _NumberOfSectors = 41;

      _centerScale = 0.125;
      _textOffset = 0.0;

      _shaftLength = 2.0/3.0;
      _shaftWidth = 3.0/48.0/2.5;

      _tipLength = 1.0/3.0;
      _tipWidth = 3.0/24.0/2.5;

      _centerType = RKGlobalAxes::CenterType::sphere;
      _tipVisibility = true;

      _aspectRatio = 0.25;

      _centerAmbientColor = QColor(51,51,51,255);
      _centerDiffuseColor = QColor(255,255,255,255);
      _centerSpecularColor = QColor(255,255,255,255);
      _centerShininess = 4.0;

      _axisXAmbientColor = QColor(5151,51,255);
      _axisXDiffuseColor = QColor(255,102,178,255);
      _axisXSpecularColor = QColor(255,255,255,255);
      _axisXShininess = 4.0;

      _axisYAmbientColor = QColor(51,51,51,255);
      _axisYDiffuseColor = QColor(178,255,102,255);
      _axisYSpecularColor = QColor(255,255,255,255);
      _axisYShininess = 4.0;

      _axisZAmbientColor = QColor(51,51,51, 255);
      _axisZDiffuseColor = QColor(102,178,255,255);
      _axisZSpecularColor = QColor(255,255,255,255);
      _axisZShininess = 4.0;
       break;
    }
  }

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<RKGlobalAxes> &axes)
{
  stream << axes->_versionNumber;
  stream << static_cast<typename std::underlying_type<RKGlobalAxes::Style>::type>(axes->_style);
  stream << static_cast<typename std::underlying_type<RKGlobalAxes::Position>::type>(axes->_position);

  stream << axes->_borderOffsetScreenFraction;
  stream << axes->_sizeScreenFraction;

  stream << axes->_axesBackgroundColor;
  stream << axes->_axesBackgroundAdditionalSize;
  stream << static_cast<typename std::underlying_type<RKGlobalAxes::BackgroundStyle>::type>(axes->_axesBackgroundStyle);

  stream << axes->_textScale;
  stream << axes->_textColorX;
  stream << axes->_textColorY;
  stream << axes->_textColorZ;
  stream << axes->_textDisplacementX;
  stream << axes->_textDisplacementY;
  stream << axes->_textDisplacementZ;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<RKGlobalAxes> &axes)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > axes->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "RKGlobalAxes");
  }

  qint64 style;
  stream >> style;
  axes->_style = RKGlobalAxes::Style(style);
  qint64 position;
  stream >> position;
  axes->_position = RKGlobalAxes::Position(position);
  stream >> axes->_borderOffsetScreenFraction;
  stream >> axes->_sizeScreenFraction;

  stream >> axes->_axesBackgroundColor;
  stream >> axes->_axesBackgroundAdditionalSize;
  qint64 axesBackgroundStyle;
  stream >> axesBackgroundStyle;
  axes->_axesBackgroundStyle = RKGlobalAxes::BackgroundStyle(axesBackgroundStyle);

  stream >> axes->_textScale;
  stream >> axes->_textColorX;
  stream >> axes->_textColorY;
  stream >> axes->_textColorZ;
  stream >> axes->_textDisplacementX;
  stream >> axes->_textDisplacementY;
  stream >> axes->_textDisplacementZ;

  axes->setStyle(axes->_style);

  return stream;
}
