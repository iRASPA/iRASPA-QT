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

#pragma once
#include <mathkit.h>
#include <cmath>
#include <qmath.h>

class TrackBall
{
public:
    TrackBall();
    void start(double x, double y, double originX, double originY, double width, double height);
    simd_quatd rollToTrackball(double x, double y);
private:
  double _kTol = 0.001;
  double _kRad2Deg = 180.0 / M_PI;
  double _kDeg2Rad = M_PI / 180.0;

  double _gRadiusTrackball = 0.0;
  double3 _gStartPtTrackball = double3();
  double3 _gEndPtTrackball = double3();

  double _gXCenterTrackball = 0.0;
  double _gYCenterTrackball = 0.0;
};
