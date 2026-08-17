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

#include "arrowygeometry.h"

ArrowYGeometry::ArrowYGeometry(double offset, double arrowHeight, double arrowRadius, float4 arrowColor,
                               double tipHeight, double tipRadius, float4 tipColor, bool tipVisibility,
                               double aspectRatio, int sectorCount)
{
  double sectorStep = 2.0 * M_PI / double(sectorCount);

  // bottom cap arrow
  size_t ref_cap_1 = _vertices.size();
  _vertices.push_back(RKPrimitiveVertex(float4(0.0, offset, 0.0, 0.0), float4(0.0, -1.0, 0.0, 0.0), arrowColor, float2(0.5,0.5)));
  for(int i=0;i<sectorCount;i++)
  {
    double sectorAngle = i * sectorStep + 0.25 * M_PI;
    float4 position = float4(cos(sectorAngle) * arrowRadius / aspectRatio, offset, sin(sectorAngle) * arrowRadius, 0.0);
    float4 normal = float4(0.0, -1.0, 0.0, 0.0);
    _vertices.push_back(RKPrimitiveVertex(position, normal, arrowColor, float2()));
  }

  // arrow shaft
  size_t ref_cap_2 = _vertices.size();
  for(int i=0;i<sectorCount;i++)
  {
    double sectorAngle = (i+1) * sectorStep + 0.25 * M_PI;
    double cosTheta = cos(sectorAngle);
    double sinTheta = sin(sectorAngle);

    double sectorAngle2 = i * sectorStep + 0.25 * M_PI;
    double cosTheta2 = cos(sectorAngle2);
    double sinTheta2 = sin(sectorAngle2);

    float4 position1 = float4(arrowRadius * cosTheta2 / aspectRatio, offset + arrowHeight, arrowRadius * sinTheta2, 0.0);
    float4 position2 = float4(arrowRadius * cosTheta / aspectRatio,  offset + arrowHeight, arrowRadius * sinTheta,  0.0);
    float4 position3 = float4(arrowRadius * cosTheta / aspectRatio,  offset,               arrowRadius * sinTheta,  0.0);


    float4 v1 = position2 - position3;
    float4 w1 = position2 - position1;
    float3 normal1 = float3::cross(float3(v1.x,v1.y,v1.z), float3(w1.x,w1.y,w1.z)).normalise();

    if(sectorCount < 20 || aspectRatio < 1.0)
    {
      _vertices.push_back(RKPrimitiveVertex(position1, float4(normal1.x,normal1.y,normal1.z,0.0), tipColor,   float2()));
      _vertices.push_back(RKPrimitiveVertex(position2, float4(normal1.x,normal1.y,normal1.z,0.0), tipColor,   float2()));
      _vertices.push_back(RKPrimitiveVertex(position3, float4(normal1.x,normal1.y,normal1.z,0.0), arrowColor, float2()));
    }
    else
    {
      _vertices.push_back(RKPrimitiveVertex(position1, float4(position1.x,0.0,position1.z,0.0), tipColor,   float2()));
      _vertices.push_back(RKPrimitiveVertex(position2, float4(position2.x,0.0,position2.z,0.0), tipColor,   float2()));
      _vertices.push_back(RKPrimitiveVertex(position3, float4(position3.x,0.0,position3.z,0.0), arrowColor, float2()));
    }

    float4 position4 = float4(arrowRadius * cosTheta / aspectRatio,  offset,               arrowRadius * sinTheta,  0.0);
    float4 position5 = float4(arrowRadius * cosTheta2 / aspectRatio, offset,               arrowRadius * sinTheta2, 0.0);
    float4 position6 = float4(arrowRadius * cosTheta2 / aspectRatio, offset + arrowHeight, arrowRadius * sinTheta2, 0.0);


    float4 v2 = position5 - position6;
    float4 w2 = position5 - position4;
    float3 normal2 = float3::cross(float3(v2.x,v2.y,v2.z), float3(w2.x,w2.y,w2.z)).normalise();

    if(sectorCount < 20 || aspectRatio < 1.0)
    {
      _vertices.push_back(RKPrimitiveVertex(position4, float4(normal2.x,normal2.y,normal2.z,0.0), arrowColor, float2()));
      _vertices.push_back(RKPrimitiveVertex(position5, float4(normal2.x,normal2.y,normal2.z,0.0), arrowColor, float2()));
      _vertices.push_back(RKPrimitiveVertex(position6, float4(normal2.x,normal2.y,normal2.z,0.0), tipColor,   float2()));
    }
    else
    {
      _vertices.push_back(RKPrimitiveVertex(position4, float4(position4.x,0.0,position4.z,0.0), arrowColor, float2()));
      _vertices.push_back(RKPrimitiveVertex(position5, float4(position5.x,0.0,position5.z,0.0), arrowColor, float2()));
      _vertices.push_back(RKPrimitiveVertex(position6, float4(position6.x,0.0,position6.z,0.0), tipColor,   float2()));
    }
  }

  // bottom cap arrow tip
  size_t ref_cap_3 = _vertices.size();
  if(tipVisibility)
  {
    _vertices.push_back(RKPrimitiveVertex(float4(0.0, offset + arrowHeight, 0.0, 0.0), float4(0.0, -1.0, 0.0, 0.0), tipColor, float2(0.5,0.5)));
    for(int i=0;i<sectorCount;i++)
    {
      double sectorAngle = i * sectorStep + 0.25 * M_PI;
      float4 position = float4(cos(sectorAngle) * tipRadius / aspectRatio, offset + arrowHeight, sin(sectorAngle) * tipRadius, 0.0);
      float4 normal = float4(0.0,-1.0, 0.0, 0.0);
      _vertices.push_back(RKPrimitiveVertex(position, normal, tipColor, float2()));
    }
  }
  else
  {
    _vertices.push_back(RKPrimitiveVertex(float4(0.0, offset + arrowHeight, 0.0, 0.0), float4(0.0, 1.0, 0.0, 0.0), tipColor, float2(0.5,0.5)));
    for(int i=0;i<sectorCount;i++)
    {
      double sectorAngle = i * sectorStep + 0.25 * M_PI;
      float4 position = float4(cos(sectorAngle) * arrowRadius / aspectRatio,offset + arrowHeight, sin(sectorAngle) * arrowRadius, 0.0);
      float4 normal = float4(0.0, 1.0, 0.0, 0.0);
      _vertices.push_back(RKPrimitiveVertex(position, normal, tipColor, float2()));
    }
  }

  // arrow tip
  size_t ref_cap_4 = _vertices.size();
  if(tipVisibility)
  {
    double angleY = atan2(tipRadius, tipHeight);
    double nx = cos(angleY);
    double ny = sin(angleY);
    double nz = 0;

    for(int i=0;i<=sectorCount;i++)
    {
      double sectorAngle = (i+1) * sectorStep + 0.25 * M_PI;
      double sectorAngle2 = i * sectorStep + 0.25 * M_PI;

      float4 position1 = float4(cos(sectorAngle) * tipRadius / aspectRatio,  offset + arrowHeight, sin(sectorAngle) * tipRadius,  0.0);
      float4 position2 = float4(cos(sectorAngle2) * tipRadius / aspectRatio, offset + arrowHeight, sin(sectorAngle2) * tipRadius, 0.0);
      float4 position3 = float4(0.0, offset + arrowHeight + tipHeight,  0.0, 0.0);

      float3 normal1; float3 normal2; float3 normal3;
      if(sectorCount < 20 || aspectRatio < 1.0)
      {
        float4 v1 = position2 - position3;
        float4 w1 = position2 - position1;
        normal1 = float3::cross(float3(v1.x,v1.y,v1.z), float3(w1.x,w1.y,w1.z)).normalise();
        normal2 = normal1;
        normal3 = normal1;
      }
      else
      {
        normal1 = float3((cos(sectorAngle)*nx - sin(sectorAngle)*nz) / aspectRatio,   ny,  sin(sectorAngle)*nx + cos(sectorAngle)*nz).normalise();
        normal2 = float3((cos(sectorAngle2)*nx - sin(sectorAngle2)*nz) / aspectRatio, ny,  sin(sectorAngle2)*nx + cos(sectorAngle2)*nz).normalise();
        normal3 = float3((cos(sectorAngle)*nx - sin(sectorAngle)*nz) / aspectRatio,   ny,  sin(sectorAngle)*nx + cos(sectorAngle)*nz).normalise();
      }

      _vertices.push_back(RKPrimitiveVertex(position1, float4(normal1.x,normal1.y,normal1.z,0.0), tipColor, float2()));
      _vertices.push_back(RKPrimitiveVertex(position2, float4(normal2.x,normal2.y,normal2.z,0.0), tipColor, float2()));
      _vertices.push_back(RKPrimitiveVertex(position3, float4(normal3.x,normal3.y,normal3.z,0.0), tipColor, float2()));
    }
  }



  // bottom cap arrow
  for(int i=0;i<sectorCount;i++)
  {
    _indices.push_back(short(ref_cap_1));
    _indices.push_back(short(ref_cap_1 + 1 + i % sectorCount));
    _indices.push_back(short(ref_cap_1 + 1 + (i+1) % sectorCount));
  }

  // arrow shaft
  for(int i=0;i<6*sectorCount;i++)
  {
    _indices.push_back(short(ref_cap_2 + i));
  }

  // bottom cap arrow tip
  if(tipVisibility)
  {
    for(int i=0;i<sectorCount;i++)
    {
      _indices.push_back(short(ref_cap_3));
      _indices.push_back(short(ref_cap_3 + 1 + i % sectorCount));
      _indices.push_back(short(ref_cap_3 + 1 + (i+1) % sectorCount));
    }
  }
  else
  {
    for(int i=0;i<sectorCount;i++)
    {
      _indices.push_back(short(ref_cap_3));
      _indices.push_back(short(ref_cap_3 + 1 + (i+1) % sectorCount));
      _indices.push_back(short(ref_cap_3 + 1 + i % sectorCount));
    }
  }

  // arrow tip
  if(tipVisibility)
  {
    for(int i=0;i<3*sectorCount;i++)
    {
      _indices.push_back(short(ref_cap_4 + i));
    }
  }

  _numberOfVertices = _vertices.size();
  _numberOfIndices = _indices.size();
}
