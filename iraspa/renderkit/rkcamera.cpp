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

#include "rkcamera.h"
#include "QtDebug"

#include <cmath>
#include <qmath.h>

RKCamera::RKCamera()
{
  _zNear = 1.0;
  _zFar = 1000.0;

  _distance = double3(0.0, 0.0, 30.0);
  _orthoScale = 10.0;
  _centerOfScene = double3(10,10,10);
  _centerOfRotation = double3(10,10,10);
  _panning = double3(0.0,0.0,0.0);
  _trucking = double3(0.0,0.0,0.0);
  _eye = _centerOfScene + _distance + _panning;
  _windowWidth = 1160;
  _windowHeight = 720;
  _angleOfView = 60.0 * M_PI / 180.0;
  _resetFraction = 0.85;
  _aspectRatio = _windowWidth / _windowHeight;
  _boundingBox = SKBoundingBox(double3(0.0, 0.0, 0.0), double3(10.0, 10.0, 10.0));
  _boundingBoxAspectRatio = 1.0;
  _frustrumType = FrustrumType::orthographic;
  _resetDirectionType = ResetDirectionType::plus_Z;

  _viewMatrix = RKCamera::GluLookAt(_eye, _centerOfScene, double3(0.0, 1.0, 0.0));
}

SKBoundingBox RKCamera::cameraBoundingBox()
{
  // use at least 5,5,5 as the minimum-size
  double3 center = _boundingBox.minimum() + (_boundingBox.maximum() - _boundingBox.minimum()) * 0.5;
  double3 width = double3::max(double3(5.0),_boundingBox.maximum() - _boundingBox.minimum());
  return SKBoundingBox(center, width, 1.0);
}

SKBoundingBox RKCamera::boundingBox()
{
  return _boundingBox;
}

void RKCamera::setBoundingBox(SKBoundingBox boundingBox)
{
    _boundingBox = boundingBox;
}

double4x4 RKCamera::projectionMatrix()
{
  switch(_frustrumType)
  {
    case FrustrumType::orthographic:
      return RKCamera::glFrustumfOrthographic(_left, _right, _bottom, _top, _zNear, _zFar);
    case FrustrumType::perspective:
      return RKCamera::glFrustumfPerspective(_left, _right, _bottom, _top, _zNear, _zFar);
  }
}

double3 RKCamera::position()
{
  double4 cameraPosition = double4x4::inverse(modelViewMatrix()) * double4(0.0, 0.0, 0.0, 1.0);

  return double3(cameraPosition.x, cameraPosition.y, cameraPosition.z);
}

simd_quatd RKCamera::referenceDirection()
{
  switch(_resetDirectionType)
  {
    case ResetDirectionType::plus_Z:
      return simd_quatd(1.0, double3(0.0, 0.0, 0.0));
    case ResetDirectionType::plus_Y:
      return simd_quatd(cos(M_PI / 4.0), double3(sin(M_PI / 4.0), 0.0, 0.0));
    case ResetDirectionType::plus_X:
      return simd_quatd(cos(M_PI / 4.0), double3(0.0, sin(M_PI / 4.0), 0.0));
    case ResetDirectionType::minus_Z:
      return simd_quatd(cos(M_PI / 2.0), double3(0.0, sin(M_PI / 2.0), 0.0));
    case ResetDirectionType::minus_Y:
      return simd_quatd(cos(-M_PI / 4.0), double3(sin(-M_PI / 4.0), 0.0, 0.0));
    case ResetDirectionType::minus_X:
      return simd_quatd(cos(-M_PI / 4.0), double3(0.0, sin(-M_PI / 4.0), 0.0));
   }
}

double3 RKCamera::EulerAngles()
{
  return (_trackBallRotation * _worldRotation).EulerAngles();
}

double4x4 RKCamera::modelMatrix()
{
  return double4x4::TransformationAroundArbitraryPoint(double4x4(_trackBallRotation * _worldRotation * referenceDirection()), _centerOfRotation);
}

double4x4 RKCamera::modelViewMatrix()
{
  // first rotate the actors, and then construct the modelView-matrix
  return _viewMatrix * double4x4::TransformationAroundArbitraryPoint(double4x4(_trackBallRotation * _worldRotation * referenceDirection()), _centerOfRotation);
}

void RKCamera::setPanning(double panx, double pany)
{
  _panning.x += panx;
  _panning.y += pany;
  _eye = _centerOfScene + _distance + _panning;

  _viewMatrix = RKCamera::GluLookAt(_eye + _trucking, _centerOfScene + _trucking, double3(0.0, 1.0, 0.0));
  updateCameraForWindowResize(_windowWidth, _windowHeight);
}

void RKCamera::setTrucking(double truckx, double trucky)
{
  _trucking.x += truckx;
  _trucking.y += trucky;
  _eye = _centerOfScene + _distance + _panning;

  _viewMatrix = RKCamera::GluLookAt(_eye + _trucking, _centerOfScene + _trucking, double3(0.0, 1.0, 0.0));
  updateCameraForWindowResize(_windowWidth, _windowHeight);
}

void RKCamera::increaseDistance(double deltaDistance)
{
  SKBoundingBox transformedBoundingBox = cameraBoundingBox().adjustForTransformation(modelMatrix());
  double3 delta = double3();
  delta.x = fabs(transformedBoundingBox.maximum().x-transformedBoundingBox.minimum().x);
  delta.y = fabs(transformedBoundingBox.maximum().y-transformedBoundingBox.minimum().y);
  delta.z = fabs(transformedBoundingBox.maximum().z-transformedBoundingBox.minimum().z);
  double focalLength = 1.0 / tan(0.5 * _angleOfView);

  switch(_frustrumType)
  {
    case FrustrumType::orthographic:
      if ((_orthoScale - 0.25 * deltaDistance) * focalLength > 0.0)
      {
        _orthoScale -= 0.25 * deltaDistance;
        _distance.z = _orthoScale * focalLength + 0.5 * delta.z;
      }
      break;
    case FrustrumType::perspective:
      if (_distance.z - 0.25 * deltaDistance > 0.0)
      {
        _distance.z -= 0.25 * deltaDistance;
        _orthoScale = (_distance.z - 0.5 * delta.z) * tan(0.5 * _angleOfView);
      }
      break;
  }

  _eye = _centerOfScene + _distance + _panning;

  _viewMatrix = RKCamera::GluLookAt(_eye + _trucking, _centerOfScene + _trucking, double3(0.0, 1.0, 0.0));
  updateCameraForWindowResize(_windowWidth, _windowHeight);
}

void RKCamera::updateFieldOfView()
{
  updateCameraForWindowResize(_windowWidth, _windowHeight);
}

void RKCamera::updateCameraForWindowResize(double width, double height)
{
  _windowWidth = width;
  _windowHeight = height;

  _aspectRatio = _windowWidth/_windowHeight;

  switch(_frustrumType)
  {
    case FrustrumType::perspective:
      setCameraToPerspective();
      break;
    case FrustrumType::orthographic:
      setCameraToOrthographic();
      break;
  }
}

 void RKCamera::setCameraToPerspective()
{
  _frustrumType = FrustrumType::perspective;

  SKBoundingBox transformedBoundingBox = cameraBoundingBox().adjustForTransformation(modelMatrix());

  double3 delta = double3(fabs(transformedBoundingBox.maximum().x-transformedBoundingBox.minimum().x),
                          fabs(transformedBoundingBox.maximum().y-transformedBoundingBox.minimum().y),
                          fabs(transformedBoundingBox.maximum().z-transformedBoundingBox.minimum().z));

  double inverseFocalPoint = tan(_angleOfView * 0.5);
  double focalPoint = 1.0 / inverseFocalPoint;

  _distance.z = std::max(_orthoScale * focalPoint + 0.5 * delta.z, 0.25);

  _zNear = std::max(1.0, position().z - delta.length() * delta.length());
  _zFar = position().z + delta.length() * delta.length();

  // halfHeight  half of frustum height at znear  znear∗tan(fov/2)
  // halfWidth   half of frustum width at znear   halfHeight×aspect
  // depth       depth of view frustum            zfar−znear

  if (_aspectRatio > _boundingBoxAspectRatio)
  {
    _left =  -_aspectRatio * _zNear * inverseFocalPoint  / _boundingBoxAspectRatio;
    _right = _aspectRatio * _zNear * inverseFocalPoint  / _boundingBoxAspectRatio;
    _top = _zNear * inverseFocalPoint  / _boundingBoxAspectRatio;
    _bottom = -_zNear * inverseFocalPoint  / _boundingBoxAspectRatio;
  }
  else
  {
    _left = -_zNear * inverseFocalPoint;
    _right = _zNear * inverseFocalPoint;
    _top = _zNear * inverseFocalPoint  / _aspectRatio;
    _bottom = -_zNear * inverseFocalPoint  / _aspectRatio;
  }
}

void RKCamera::setCameraToOrthographic()
{
  _frustrumType = FrustrumType::orthographic;

  SKBoundingBox transformedBoundingBox = cameraBoundingBox().adjustForTransformation(modelMatrix());

  double3 delta = double3(fabs(transformedBoundingBox.maximum().x-transformedBoundingBox.minimum().x),
                          fabs(transformedBoundingBox.maximum().y-transformedBoundingBox.minimum().y),
                          fabs(transformedBoundingBox.maximum().z-transformedBoundingBox.minimum().z));

  double inverseFocalPoint = tan(_angleOfView * 0.5);
  _orthoScale = std::max(_distance.z - 0.5 * delta.z, 0.25) * inverseFocalPoint;

  _zNear = std::max(1.0, position().z - delta.length() * delta.length());
  _zFar = position().z + delta.length() * delta.length();

  if (_aspectRatio > _boundingBoxAspectRatio)
  {
    _left =  -_aspectRatio*_orthoScale/_boundingBoxAspectRatio;
    _right = _aspectRatio*_orthoScale/_boundingBoxAspectRatio;
    _top = _orthoScale/_boundingBoxAspectRatio;
    _bottom = -_orthoScale/_boundingBoxAspectRatio;
  }
  else
  {
    _left = -_orthoScale;
    _right = _orthoScale;
    _top = _orthoScale/_aspectRatio;
    _bottom = -_orthoScale/_aspectRatio;
  }
}


void RKCamera::resetForNewBoundingBox(SKBoundingBox box)
{
  SKBoundingBox transformedBoundingBoxOld = cameraBoundingBox().adjustForTransformation(modelMatrix());

  double3 deltaOld = double3(fabs(transformedBoundingBoxOld.maximum().x-transformedBoundingBoxOld.minimum().x),
                             fabs(transformedBoundingBoxOld.maximum().y-transformedBoundingBoxOld.minimum().y),
                             fabs(transformedBoundingBoxOld.maximum().z-transformedBoundingBoxOld.minimum().z));

  _boundingBox = box;
  SKBoundingBox transformedBoundingBoxNew = cameraBoundingBox().adjustForTransformation(modelMatrix());

  double3 deltaNew = double3(fabs(transformedBoundingBoxNew.maximum().x-transformedBoundingBoxNew.minimum().x),
                             fabs(transformedBoundingBoxNew.maximum().y-transformedBoundingBoxNew.minimum().y),
                             fabs(transformedBoundingBoxNew.maximum().z-transformedBoundingBoxNew.minimum().z));

  double inverseFocalLength = tan(0.5 * _angleOfView);
  double focalLength = 1.0 / inverseFocalLength;
  _boundingBoxAspectRatio = deltaNew.x / deltaNew.y;
  double distanceNew = 0.5 * deltaNew.z + 0.5 * deltaNew.x * focalLength;
  double distanceOld = 0.5 * deltaOld.z + 0.5 * deltaOld.x * focalLength;
  _distance.z += (distanceNew - distanceOld);
  _orthoScale = (_distance.z - 0.5 * deltaNew.z) * inverseFocalLength;

  _centerOfScene = cameraBoundingBox().minimum() + (cameraBoundingBox().maximum() - cameraBoundingBox().minimum()) * 0.5;
  _centerOfRotation = _centerOfScene;
  _eye = _centerOfScene + _distance + _panning;
  _viewMatrix = RKCamera::GluLookAt(_eye + _trucking, _centerOfScene + _trucking, double3(0.0, 1.0, 0.0));

   updateCameraForWindowResize(_windowWidth, _windowHeight);
}


void RKCamera::resetCameraToDirection()
{
  _initialized = true;

  _panning = double3(0.0,0.0,0.0);
  _trucking = double3(0.0,0.0,0.0);

  _centerOfScene = cameraBoundingBox().minimum() + (cameraBoundingBox().maximum() - cameraBoundingBox().minimum()) * 0.5;
  _centerOfRotation = _centerOfScene;

  _worldRotation = simd_quatd(1.0, double3(0.0, 0.0, 0.0));

  resetCameraDistance();
}

void RKCamera::resetCameraDistance()
{
  double3 delta = double3();

  _initialized = true;

  _centerOfScene = cameraBoundingBox().minimum() + (cameraBoundingBox().maximum() - cameraBoundingBox().minimum()) * 0.5;
  _centerOfRotation = _centerOfScene;

  double4x4 matrix = double4x4::TransformationAroundArbitraryPoint(double4x4(referenceDirection() * _worldRotation), _centerOfRotation);
  SKBoundingBox transformedBoundingBox = cameraBoundingBox().adjustForTransformation(matrix);

  delta.x = fabs(transformedBoundingBox.maximum().x-transformedBoundingBox.minimum().x);
  delta.y = fabs(transformedBoundingBox.maximum().y-transformedBoundingBox.minimum().y);
  delta.z = fabs(transformedBoundingBox.maximum().z-transformedBoundingBox.minimum().z);

  double focalPoint = 1.0 / tan(0.5 * _angleOfView);
  _boundingBoxAspectRatio = delta.x / delta.y;
  _orthoScale = 0.5 * delta.x / _resetFraction;
  _distance.z = _orthoScale * focalPoint  + 0.5 * delta.z;

  //_panning = double3(0.0,0.0,0.0);
  //_trucking = double3(0.0,0.0,0.0);
  _trackBallRotation = simd_quatd(1.0, double3(0.0, 0.0, 0.0));

  _eye = _centerOfScene + _distance + _panning;
  _viewMatrix = RKCamera::GluLookAt(_eye + _trucking, _centerOfScene + _trucking, double3(0.0, 1.0, 0.0));

  updateCameraForWindowResize(_windowWidth, _windowHeight);
}

void RKCamera::setAngleOfView(double d)
{
  _angleOfView = d;

  SKBoundingBox transformedBoundingBox = cameraBoundingBox().adjustForTransformation(modelMatrix());
  double3 delta = double3();
  delta.x = fabs(transformedBoundingBox.maximum().x-transformedBoundingBox.minimum().x);
  delta.y = fabs(transformedBoundingBox.maximum().y-transformedBoundingBox.minimum().y);
  delta.z = fabs(transformedBoundingBox.maximum().z-transformedBoundingBox.minimum().z);
  double focalLength = 1.0 / tan(0.5 * _angleOfView);

  switch(_frustrumType)
  {
    case FrustrumType::orthographic:
      _distance.z = _orthoScale * focalLength + 0.5 * delta.z;
      break;
    case FrustrumType::perspective:
      _orthoScale = (_distance.z - 0.5 * delta.z) * tan(0.5 * _angleOfView);
      break;
  }

  updateCameraForWindowResize(_windowWidth, _windowHeight);
}


// This function computes the inverse camera transform according to its parameters
double4x4 RKCamera::GluLookAt(double3 eye, double3 center, double3 oldup)
{
  double3 up = oldup;
  double3 forward = double3();
  double3 side = double3();
  double4x4 viewMatrix;
  double inv_length;

  forward.x = center.x - eye.x;
  forward.y = center.y - eye.y;
  forward.z = center.z - eye.z;
  inv_length=1.0/sqrt((forward.x*forward.x)+(forward.y*forward.y)+(forward.z*forward.z));
  forward.x *= inv_length;
  forward.y *= inv_length;
  forward.z *= inv_length;

  inv_length = 1.0/sqrt((up.x*up.x)+(up.y*up.y)+(up.z*up.z));
  up.x *= inv_length;
  up.y *= inv_length;
  up.z *= inv_length;

  /* Side = forward x up */
  side.x = forward.y * up.z - forward.z * up.y;
  side.y = forward.z * up.x - forward.x * up.z;
  side.z = forward.x * up.y - forward.y * up.x;
  inv_length=1.0/sqrt((side.x*side.x)+(side.y*side.y)+(side.z*side.z));
  side.x *= inv_length;
  side.y *= inv_length;
  side.z *= inv_length;

  /* Up = side x forward */
  up.x = side.y * forward.z - side.z * forward.y;
  up.y = side.z * forward.x - side.x * forward.z;
  up.z = side.x * forward.y - side.y * forward.x;


  // note that the inverse matrix is setup, i.e. the transpose
  viewMatrix=double4x4(double4(side.x, up.x, -forward.x, 0.0), // 1th column
      double4(side.y, up.y, -forward.y, 0.0), // 2nd column
      double4(side.z, up.z, -forward.z, 0.0), // 3rd column
      double4(0, 0, 0, 1)) ;                // 4th column


  // set translation part
  viewMatrix[3][0] = -(viewMatrix[0][0]*eye.x+viewMatrix[1][0]*eye.y+viewMatrix[2][0]*eye.z);
  viewMatrix[3][1] = -(viewMatrix[0][1]*eye.x+viewMatrix[1][1]*eye.y+viewMatrix[2][1]*eye.z);
  viewMatrix[3][2] = -(viewMatrix[0][2]*eye.x+viewMatrix[1][2]*eye.y+viewMatrix[2][2]*eye.z);

  viewMatrix[3][0] = -eye.x;
  viewMatrix[3][1] = -eye.y;
  viewMatrix[3][2] = -eye.z;

  return viewMatrix;
}



// http://www.songho.ca/opengl/gl_projectionmatrix.html
double4x4 RKCamera::glFrustumfPerspective(double left, double right, double bottom, double top, double near, double far)
{
  double4x4 m = double4x4();

  double _2n = 2.0 * near;
  double _1over_rml = 1.0 / (right - left);
  double _1over_fmn = 1.0 / (far - near);
  double _1over_tmb = 1.0 / (top - bottom);

  m=double4x4(double4(_2n * _1over_rml, 0.0, 0.0, 0.0),
      double4(0.0, _2n * _1over_tmb, 0.0, 0.0),
      double4((right + left) * _1over_rml, (top + bottom) * _1over_tmb, (-(far + near)) * _1over_fmn, -1.0),
      double4(0.0, 0.0, -(_2n * far * _1over_fmn), 0.0));

  return m;
}




double4x4 RKCamera::glFrustumfPerspective(double fov, double aspectratio, [[maybe_unused]] double boundingBoxAspectRatio, double near, double far)
{
  double4x4 m;
  double _1over_fmn = 1.0 / (near - far);
  double focolPoint = 1.0/tan(0.5*fov);

  // When the Aspect Ratio is larger than unity, gluPerspective fixes the height, lengthens/shortens the width
  // When the Aspect Ratio is less than unity, gluPerspective fixes the width, lengthens/shortens the height
  m=double4x4(double4(focolPoint / (aspectratio > 1.0 ? aspectratio : 1.0), 0.0, 0.0, 0.0),
                double4(0.0, focolPoint * (aspectratio < 1.0 ? aspectratio : 1.0), 0.0, 0.0),
                double4(0.0, 0.0, ((far + near)) * _1over_fmn, -1.0),
                double4(0.0, 0.0, (2.0 * near * far * _1over_fmn), 0.0));

  return m;
}

double4x4 RKCamera::glFrustumfOrthographic(double left, double right, double  bottom, double top, double near, double far)
{
  double4x4 m;
  double _1over_rml  = 1.0 / (right - left);
  double _1over_fmn  = 1.0 / (far - near);
  double _1over_tmb = 1.0 / (top - bottom);

  m=double4x4(double4(2.0 * _1over_rml, 0.0, 0.0, 0.0),
      double4(0.0, 2.0 * _1over_tmb, 0.0, 0.0),
      double4(0.0, 0.0, -2.0 * _1over_fmn, 0.0),
      double4(-(right + left) * _1over_rml, -(top + bottom) * _1over_tmb, (-(far + near)) * _1over_fmn, 1.0));
  return m;
}


// projection: point in “world” -> model-view -> projection -> viewport -> point on “screen”
// unprojection: point on “screen” -> viewport^(–1) -> projection^(–1) -> model-view^(–1) -> point in “world”

// It turns out that, the way OpenGL calculates things, winZ == 0.0 (the screen) corresponds to objZ == –N (the near plane),
// and winZ == 1.0 corresponds to objZ == –F (the far plane)

// Since two points determine a line, we actually need to call gluUnProject() twice: once with winZ == 0.0, then again with winZ == 1.0
// this will give us the world points that correspond to the mouse click on the near and far planes, respectively
// https://www.opengl.org/wiki/GluProject_and_gluUnProject_code
double3 RKCamera::myGluUnProject(double3 position, QRect viewPort)
{
  double4x4 finalMatrix;
  double4 inVector = double4();

  // Map x and y from window coordinates
  inVector.x = (position.x - double(viewPort.left())) / double(viewPort.width());
  inVector.y = (position.y - double(viewPort.top())) / double(viewPort.height());
  inVector.z = double(position.z);
  inVector.w = 1.0;

  // Map to range -1 to 1 NDC-coordinates
  inVector.x = inVector.x * 2.0 - 1.0;
  inVector.y = inVector.y * 2.0 - 1.0;
  inVector.z = inVector.z * 2.0 - 1.0;
  inVector.w = 1.0;


  // Coordinate space ranges from -Wc to Wc in all three axes, where Wc is the Clip Coordinate W value.
  // OpenGL clips all coordinates outside this range.

  // transform from clip-coordinates to object coordinates
  finalMatrix = double4x4::inverse(projectionMatrix()* modelViewMatrix());
  double4 outVector = finalMatrix * inVector;

  if(fabs(outVector.w ) < DBL_MIN)
  {
     return double3(0.0, 0.0, 0.0);
  }
  else
  {
    return double3(outVector.x/outVector.w, outVector.y/outVector.w, outVector.z/outVector.w);
  }
}

double3 RKCamera::myGluUnProject(double3 position, double4x4 mMatrix, QRect viewPort)
{
  double4x4 finalMatrix;
  double4 inVector = double4();

  // Map x and y from window coordinates
  inVector.x = (position.x - double(viewPort.left())) / double(viewPort.width());
  inVector.y = (position.y - double(viewPort.bottom())) / double(viewPort.height());
  inVector.z = double(position.z);
  inVector.w = 1.0;

  // Map to range -1 to 1 NDC-coordinates
  inVector.x = inVector.x * 2.0 - 1.0;
  inVector.y = inVector.y * 2.0 - 1.0;
  inVector.z = inVector.z * 2.0 - 1.0;
  inVector.w = 1.0;


  // Coordinate space ranges from -Wc to Wc in all three axes, where Wc is the Clip Coordinate W value.
  // OpenGL clips all coordinates outside this range.

  // transform from clip-coordinates to object coordinates
  //double4x4 mvpMatrix = projectionMatrix() * modelViewMatrix() * modelMatrix();
  double4x4 mvpMatrix = projectionMatrix() * modelViewMatrix() * mMatrix;
  finalMatrix = double4x4::inverse(mvpMatrix);
  double4 outVector = finalMatrix * inVector;

  if(fabs(outVector.w ) < DBL_MIN)
  {
    return double3(0.0, 0.0, 0.0);
  }
  else
  {
    return double3(outVector.x/outVector.w, outVector.y/outVector.w, outVector.z/outVector.w);
  }
}


double3 RKCamera::myGluProject(double3 position, QRect viewPort)
{
  double4 outVector = double4();
  double3 finalVector = double3();

  // convert to clip-coordinates
  double4x4 mvpMatrix = projectionMatrix() * modelViewMatrix();
  outVector = mvpMatrix * double4(position.x, position.y, position.z, 1.0);

  // perform perspective division
  double factor = 1.0 / outVector.w;
  outVector.x *= factor;
  outVector.y *= factor;
  outVector.z *= factor;

  // Map x, y to range 0-1
  finalVector.x = (0.5*outVector.x + 0.5) * double(viewPort.width()) + double(viewPort.left());
  finalVector.y = (0.5*outVector.y + 0.5) * double(viewPort.height()) + double(viewPort.bottom());
  finalVector.z = 0.5*outVector.z + 0.5 ;   // Between 0 and 1, this is only correct when glDepthRange(0.0, 1.0)

  return finalVector;
}


// http://www.3dkingdoms.com/selection.html
std::vector<int> RKCamera::selectPositionsInRectangle(std::vector<double3> &positions, QRect rect, double3 origin, QRect viewPortBounds)
{
  qDebug() << "viewPortBounds: " << viewPortBounds;
  qDebug() << "rect: " << rect;

  double3 Points0 = myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.top()), 0.0), viewPortBounds);
  double3 Points1 = myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.top()), 1.0), viewPortBounds);

  double3 Points2 = myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.bottom()), 0.0), viewPortBounds);
  double3 Points3 = myGluUnProject(double3(double(rect.left()), double(viewPortBounds.size().height() - rect.bottom()), 1.0), viewPortBounds);

  double3 Points4 = myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.bottom()), 0.0), viewPortBounds);
  double3 Points5 = myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.bottom()), 1.0), viewPortBounds);

  double3 Points6 = myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.top()), 0.0), viewPortBounds);
  double3 Points7 = myGluUnProject(double3(double(rect.right()), double(viewPortBounds.size().height() - rect.top()), 1.0), viewPortBounds);

  double3 FrustrumPlane0 = double3::cross(Points0 - Points1, Points0 - Points2).normalise();
  double3 FrustrumPlane1 = double3::cross(Points2 - Points3, Points2 - Points4).normalise();
  double3 FrustrumPlane2 = double3::cross(Points4 - Points5, Points4 - Points6).normalise();
  double3 FrustrumPlane3 = double3::cross(Points6 - Points7, Points6 - Points0).normalise();

  std::vector<int> indexSet = std::vector<int>();
  int numberOfObjects = positions.size();
  for(int j=0;j<numberOfObjects;j++)
  {
    double3 position = positions[j] + origin;
    if((double3::dot(position-Points0,FrustrumPlane0)<0) &&
      (double3::dot(position-Points2,FrustrumPlane1)<0) &&
      (double3::dot(position-Points4,FrustrumPlane2)<0) &&
      (double3::dot(position-Points6,FrustrumPlane3)<0))
      {
        indexSet.push_back(j);
      }
  }

  return indexSet;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<RKCamera> &camera)
{
  stream << camera->_versionNumber;
  stream << camera->_zNear;
  stream << camera->_zFar;
  stream << camera->_left;
  stream << camera->_right;
  stream << camera->_bottom;
  stream << camera->_top;

  stream << camera->_boundingBox;

  stream << camera->_boundingBoxAspectRatio;
  stream << camera->_centerOfScene;
  stream << camera->_panning;
  stream << camera->_centerOfRotation;
  stream << camera->_eye;
  stream << camera->_distance;
  stream << camera->_orthoScale;
  stream << camera->_angleOfView;
  stream << static_cast<typename std::underlying_type<FrustrumType>::type>(camera->_frustrumType);
  stream << static_cast<typename std::underlying_type<ResetDirectionType>::type>(camera->_resetDirectionType);

  stream << camera->_resetFraction;

  stream << camera->_worldRotation;
  stream << camera->_rotationDelta;

  stream << camera->_bloomLevel;
  stream << camera->_bloomPulse;

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<RKCamera> &camera)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > camera->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "RKCamera");
  }

  stream >> camera->_zNear;
  stream >> camera->_zFar;
  stream >> camera->_left;
  stream >> camera->_right;
  stream >> camera->_bottom;
  stream >> camera->_top;

  stream >> camera->_boundingBox;

  stream >> camera->_boundingBoxAspectRatio;
  stream >> camera->_centerOfScene;
  stream >> camera->_panning;
  stream >> camera->_centerOfRotation;
  stream >> camera->_eye;
  stream >> camera->_distance;
  stream >> camera->_orthoScale;
  stream >> camera->_angleOfView;
  qint64 frustrumType;
  stream >> frustrumType;
  camera->_frustrumType = FrustrumType(frustrumType);
  qint64 resetDirectionType;
  stream >> resetDirectionType;
  camera->_resetDirectionType = ResetDirectionType(resetDirectionType);

  stream >> camera->_resetFraction;

  stream >> camera->_worldRotation;
  stream >> camera->_rotationDelta;

  stream >> camera->_bloomLevel;
  stream >> camera->_bloomPulse;

  return stream;
}
