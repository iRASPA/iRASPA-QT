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

#include "skboundingbox.h"
#include <mathkit.h>
#include <cmath>
#include <algorithm>

SKBoundingBox::SKBoundingBox(): _minimum(double3(0,0,0)),_maximum(double3(20,20,20))
{

}

SKBoundingBox::SKBoundingBox(double3 minimum, double3 maximum): _minimum(minimum), _maximum(maximum)
{
}

SKBoundingBox::SKBoundingBox(const double3 center, const double3 width, const double scale):
     _minimum(center - (0.5 * scale) * width), _maximum(center + (0.5 * scale) * width)
{
}



double3 SKBoundingBox::widths() const
{
  return double3(_maximum.x-_minimum.x, _maximum.y-_minimum.y, _maximum.z-_minimum.z);
}

std::array<double3,8> const SKBoundingBox::corners() const
{
   std::array<double3,8> temp = {{
      double3(_minimum.x, _minimum.y, _minimum.z), // c[0]
      double3(_maximum.x, _minimum.y, _minimum.z), // c[1]
      double3(_maximum.x, _maximum.y, _minimum.z), // c[2]
      double3(_minimum.x, _maximum.y, _minimum.z), // c[3]
      double3(_minimum.x, _minimum.y, _maximum.z), // c[4]
      double3(_maximum.x, _minimum.y, _maximum.z), // c[5]
      double3(_maximum.x, _maximum.y, _maximum.z), // c[6]
      double3(_minimum.x, _maximum.y, _maximum.z)  // c[7]
   }};
   return temp;
}

std::array<std::pair<double3, double3>, 12> const SKBoundingBox::sides() const
{
  return std::array<std::pair<double3,double3>,12>{
      // bottom ring
      std::make_pair<double3,double3>( double3(_minimum.x, _minimum.y, _minimum.z), double3(_maximum.x, _minimum.y, _minimum.z)),
      std::make_pair<double3,double3>( double3(_maximum.x, _minimum.y, _minimum.z), double3(_maximum.x, _maximum.y, _minimum.z)),
      std::make_pair<double3,double3>( double3(_maximum.x, _maximum.y, _minimum.z), double3(_minimum.x, _maximum.y, _minimum.z)),
      std::make_pair<double3,double3>( double3(_minimum.x, _maximum.y, _minimum.z), double3(_minimum.x, _minimum.y, _minimum.z)),

      // top ring
      std::make_pair<double3,double3>( double3(_minimum.x, _minimum.y, _maximum.z), double3(_maximum.x, _minimum.y, _maximum.z)),
      std::make_pair<double3,double3>( double3(_maximum.x, _minimum.y, _maximum.z), double3(_maximum.x, _maximum.y, _maximum.z)),
      std::make_pair<double3,double3>( double3(_maximum.x, _maximum.y, _maximum.z), double3(_minimum.x, _maximum.y, _maximum.z)),
      std::make_pair<double3,double3>( double3(_minimum.x, _maximum.y, _maximum.z), double3(_minimum.x, _minimum.y, _maximum.z)),

      // sides
      std::make_pair<double3,double3>( double3(_minimum.x, _minimum.y, _minimum.z), double3(_minimum.x, _minimum.y, _maximum.z)),
      std::make_pair<double3,double3>( double3(_maximum.x, _minimum.y, _minimum.z), double3(_maximum.x, _minimum.y, _maximum.z)),
      std::make_pair<double3,double3>( double3(_maximum.x, _maximum.y, _minimum.z), double3(_maximum.x, _maximum.y, _maximum.z)),
      std::make_pair<double3,double3>( double3(_minimum.x, _maximum.y, _minimum.z), double3(_minimum.x, _maximum.y, _maximum.z))};
}

double3 SKBoundingBox::center()
{
  return _minimum + (_maximum - _minimum) * 0.5;
}

double SKBoundingBox::volume()
{
  double3x3 axes = double3x3(double3(_maximum.x - _minimum.x,0.0,0.0),double3(0.0,_maximum.y - _minimum.y,0.0),double3(0.0,0.0,_maximum.z - _minimum.z));

  return abs(axes.determinant());
}

double SKBoundingBox::shortestEdge()
{
  double3 edgeLengths = _maximum - _minimum;
  return std::min({edgeLengths.x, edgeLengths.y, edgeLengths.z});
}

double SKBoundingBox::boundingSphereRadius()
{
  std::array<double3,8> corners = {{
     double3(_minimum.x, _minimum.y, _minimum.z),
     double3(_maximum.x, _minimum.y, _minimum.z),
     double3(_maximum.x, _maximum.y, _minimum.z),
     double3(_minimum.x, _maximum.y, _minimum.z),
     double3(_minimum.x, _minimum.y, _maximum.z),
     double3(_maximum.x, _minimum.y, _maximum.z),
     double3(_maximum.x, _maximum.y, _maximum.z),
     double3(_minimum.x, _maximum.y, _maximum.z)
  }};

  double3 centerOfScene  = _minimum + (_maximum - _minimum) * 0.5;
  double radius = 0.0;
  for(double3 corner : corners)
  {
    double3 distanceVector = centerOfScene-corner;
    double cornerRadius = distanceVector.length();
    if (cornerRadius > radius)
    {
      radius = cornerRadius;
    }
  }
  return radius;
}

SKBoundingBox SKBoundingBox::adjustForTransformation(double4x4 transformation)
{
  double3 centerOfScene  = _minimum + (_maximum - _minimum) * 0.5;
  double3 min = double3();
  double3 max = double3();

  if (transformation[0][0] > 0.0)
  {
    min.x += transformation[0][0] * (this->_minimum.x - centerOfScene.x);
    max.x += transformation[0][0] * (this->_maximum.x - centerOfScene.x);
  }
  else
  {
    min.x += transformation[0][0] * (this->_maximum.x - centerOfScene.x);
    max.x += transformation[0][0] * (this->_minimum.x - centerOfScene.x);
  }

  if (transformation[0][1] > 0.0)
  {
    min.y += transformation[0][1] * (this->_minimum.x - centerOfScene.x);
    max.y += transformation[0][1] * (this->_maximum.x - centerOfScene.x);
  }
  else
  {
    min.y += transformation[0][1] * (this->_maximum.x - centerOfScene.x);
    max.y += transformation[0][1] * (this->_minimum.x - centerOfScene.x);
  }

  if (transformation[0][2] > 0.0)
  {
    min.z += transformation[0][2] * (this->_minimum.x - centerOfScene.x);
    max.z += transformation[0][2] * (this->_maximum.x - centerOfScene.x);
  }
  else
  {
    min.z += transformation[0][2] * (this->_maximum.x - centerOfScene.x);
    max.z += transformation[0][2] * (this->_minimum.x - centerOfScene.x);
  }

  if (transformation[1][0] > 0.0)
  {
    min.x += transformation[1][0] * (this->_minimum.y - centerOfScene.y);
    max.x += transformation[1][0] * (this->_maximum.y - centerOfScene.y);
  }
  else
  {
    min.x += transformation[1][0] * (this->_maximum.y - centerOfScene.y);
    max.x += transformation[1][0] * (this->_minimum.y - centerOfScene.y);
  }

  if (transformation[1][1] > 0.0)
  {
    min.y += transformation[1][1] * (this->_minimum.y - centerOfScene.y);
    max.y += transformation[1][1] * (this->_maximum.y - centerOfScene.y);
  }
  else
  {
    min.y += transformation[1][1] * (this->_maximum.y - centerOfScene.y);
    max.y += transformation[1][1] * (this->_minimum.y - centerOfScene.y);
  }

  if (transformation[1][2] > 0.0)
  {
    min.z += transformation[1][2] * (this->_minimum.y - centerOfScene.y);
    max.z += transformation[1][2] * (this->_maximum.y - centerOfScene.y);
  }
  else
  {
    min.z += transformation[1][2] * (this->_maximum.y - centerOfScene.y);
    max.z += transformation[1][2] * (this->_minimum.y - centerOfScene.y);
  }

  if (transformation[2][0] > 0.0)
  {
    min.x += transformation[2][0] * (this->_minimum.z - centerOfScene.z);
    max.x += transformation[2][0] * (this->_maximum.z - centerOfScene.z);
  }
  else
  {
    min.x += transformation[2][0] * (this->_maximum.z - centerOfScene.z);
    max.x += transformation[2][0] * (this->_minimum.z - centerOfScene.z);
  }

  if (transformation[2][1] > 0.0)
  {
    min.y += transformation[2][1] * (this->_minimum.z - centerOfScene.z);
    max.y += transformation[2][1] * (this->_maximum.z - centerOfScene.z);
  }
  else
  {
    min.y += transformation[2][1] * (this->_maximum.z - centerOfScene.z);
    max.y += transformation[2][1] * (this->_minimum.z - centerOfScene.z);
  }

  if (transformation[2][2] > 0.0)
  {
    min.z += transformation[2][2] * (this->_minimum.z - centerOfScene.z);
    max.z += transformation[2][2] * (this->_maximum.z - centerOfScene.z);
  }
  else
  {
    min.z += transformation[2][2] * (this->_maximum.z - centerOfScene.z);
    max.z += transformation[2][2] * (this->_minimum.z - centerOfScene.z);
  }
  return SKBoundingBox(min + centerOfScene, max + centerOfScene);
}

SKBoundingBox operator+(const SKBoundingBox left, const SKBoundingBox right)
{
  return SKBoundingBox(left._minimum + right.minimum(), left._maximum + right._maximum);
}

SKBoundingBox operator+(const SKBoundingBox left, double3 right)
{
  return SKBoundingBox(left._minimum + right, left._maximum + right);
}

SKBoundingBox operator-(const SKBoundingBox left, double3 right)
{
  return SKBoundingBox(left._minimum - right, left._maximum - right);
}

QDataStream &operator<<(QDataStream &stream, const SKBoundingBox &box)
{
  stream << box._versionNumber;
  stream << box._minimum;
  stream << box._maximum;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, SKBoundingBox &box)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > box._versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "SKBoundingBox");
  }
  stream >> box._minimum;
  stream >> box._maximum;
  return stream;
}
