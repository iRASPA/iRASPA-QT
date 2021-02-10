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

#include <cmath>
#include <QRect>
#include <QSize>
#include <vector>
#include <cfloat>
#include <algorithm>
#include <symmetrykit.h>
#include <foundationkit.h>
#include <type_traits>

enum class ResetDirectionType: qint64
{
  plus_X = 0, plus_Y = 1, plus_Z = 2, minus_X = 3, minus_Y = 4, minus_Z = 5
};

enum class FrustrumType: qint64
{
  perspective = 0, orthographic = 1
};


class RKCamera
{
public:
  RKCamera();
  SKBoundingBox cameraBoundingBox();
  double4x4 projectionMatrix();
  double3 position();
  simd_quatd referenceDirection();
  double3 EulerAngles();
  double4x4 modelMatrix();
  double4x4 modelViewMatrix();
  double3 myGluUnProject(double3 position, QRect viewPort);
  double3 myGluUnProject(double3 position, double4x4 modelMatrix, QRect viewPort);
  double3 myGluProject(double3 position, QRect viewPort);
  void setTrucking(double truckx, double trucky);
  void setPanning(double panx, double pany);
  double3 distance() {return _distance;}
  void increaseDistance(double deltaDistance);
  void updateFieldOfView();
  void updateCameraForWindowResize(double width, double height);

  void setCameraToOrthographic();
  bool isOrthographic() {return _frustrumType == FrustrumType::orthographic;}
  void setCameraToPerspective();
  bool isPerspective() {return _frustrumType == FrustrumType::perspective;}

  void resetForNewBoundingBox(SKBoundingBox box);
  void resetCameraToDirection();
  void resetCameraDistance();

  double resetFraction() {return _resetFraction;}
  void setResetFraction(double fraction) {_resetFraction = fraction;}
  ResetDirectionType resetCameraDirection() {return _resetDirectionType;}
  double3 centerOfScene() {return _centerOfScene;}
  double angleOfView() {return _angleOfView;}
  void setAngleOfView(double d);
  double rotationAngle() {return _rotationDelta;}
  void setRotationAngle(double angle) {_rotationDelta=angle;}

  double bloomLevel() const {return _bloomLevel;}
  void setBloomLevel(double level) {_bloomLevel = level;}
  double bloomPulse() const {return _bloomPulse;}
  void setBloomPulse(double pulse) {_bloomPulse = pulse;}

  void setBoundingBox(SKBoundingBox boundingBox);
  SKBoundingBox boundingBox();
  simd_quatd worldRotation() {return _worldRotation;}
  void setWorldRotation(simd_quatd rotation) {_worldRotation = rotation;}
  simd_quatd trackBallRotation() {return _trackBallRotation;}
  void setTrackBallRotation(simd_quatd trackBallRotation) {_trackBallRotation = trackBallRotation;}

  void setResetDirectionType(ResetDirectionType resetDirectionType) {_resetDirectionType = resetDirectionType;}

  static double4x4 GluLookAt(double3 eye, double3 center, double3 oldup);
  static double4x4 glFrustumfPerspective(double left, double right, double bottom, double top, double near, double far);
  static double4x4 glFrustumfPerspective(double fov, double aspectratio, double boundingBoxAspectRatio, double near, double far);
  static double4x4 glFrustumfOrthographic(double left, double right, double  bottom, double top, double near, double far);
  std::vector<size_t> selectPositionsInRectangle(std::vector<double3> &positions, QRect rect, double3 origin, QRect viewPortBounds);
private:
  qint64 _versionNumber{1};
  double _zNear=0.0;
  double _zFar=0.0;
  double _left = -10.0;
  double _right = 10.0;
  double _bottom = -10.0;
  double _top = 10.0;
  double _windowWidth = 100.0;
  double _windowHeight = 100.0;
  double _aspectRatio = 1.0;
  SKBoundingBox _boundingBox = SKBoundingBox();

  double _boundingBoxAspectRatio = 1.0;
  double3 _centerOfScene = double3(0,0,0);
  double3 _panning = double3(0.0,0.0,0.0);   // pan/tilt
  double3 _trucking= double3(0.0,0.0,0.0);   // truck/pedestal
  double3 _centerOfRotation = double3(0,0,0);
  double3 _eye = double3(0,0,0);
  double3 _distance = double3(0.0,0.0,50.0);
  double _orthoScale = 1.0;
  double _angleOfView = 60.0;
  FrustrumType _frustrumType = FrustrumType::orthographic;
  ResetDirectionType _resetDirectionType = ResetDirectionType::plus_Z;

  double _resetFraction = 0.85;
  bool _initialized = false;

  simd_quatd _worldRotation = simd_quatd(1.0,double3(0.0, 0.0, 0.0));
  simd_quatd _trackBallRotation = simd_quatd(1.0,double3(0.0, 0.0, 0.0));
  double _rotationDelta = 15.0;

  double _bloomLevel = 1.0;
  double _bloomPulse = 1.0;

  double4x4 _viewMatrix = double4x4();

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<RKCamera> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<RKCamera> &);
};
