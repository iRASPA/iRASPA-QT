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
#include <vector>
#include "skboundingbox.h"

class SKCell
{
public:
  SKCell();
  SKCell(double a, double b, double c, double alpha, double beta, double gamma);
  SKCell(double3 v1, double3 v2, double3 v3);
  SKCell(double3x3 m);
  SKCell(SKBoundingBox boundingBox);
  SKCell(SKCell superCell, int3 minimumReplica, int3 maximumReplica);
  double3x3 unitCell() const {return _unitCell;}
  double3x3 inverseUnitCell() const {return _inverseUnitCell;}
  void setUnitCell(const double3x3& unitCell);
  int3 minimumReplicas() const;
  void setMinimumReplicas(const int3& minimumReplicas);
  int3 maximumReplicas() const;
  void setMaximumReplicas(const int3& maximumReplicas);
  SKBoundingBox enclosingBoundingBox();
  std::array<double3,8> corners();
  int3 replicaFromIndex(int index);
  double3x3 box() const;
  void setBox(const double3x3& fullCell);
  int numberOfReplicas() const;

  std::shared_ptr<SKCell> superCell() const;

  int minimumReplicaX() const;
  void setMinimumReplicaX(const int newValue);
  int maximumReplicaX() const;
  void setMaximumReplicaX(const int newValue);
  int minimumReplicaY() const;
  void setMinimumReplicaY(const int newValue);
  int maximumReplicaY() const;
  void setMaximumReplicaY(const int newValue);
  int minimumReplicaZ() const;
  void setMinimumReplicaZ(const int newValue);
  int maximumReplicaZ() const;
  void setMaximumReplicaZ(const int newValue);

  double a() const;
  void setLengthA(double value);
  void a(const double& newValue);
  double b() const;
  void setLengthB(double value);
  void b(const double& newValue);
  double c() const;
  void setLengthC(double value);
  void c(const double& newValue);
  double alpha() const;
  void setAlphaAngle(double value);
  void alpha(const double& newValue);
  double beta() const;
  void setBetaAngle(double value);
  void beta(const double& newValue);
  double gamma() const;
  void setGammaAngle(double value);
  void gamma(const double& newValue);

  bool orthorhombic() const;
  double volume() const;
  double3 perpendicularWidths() const;

  int zValue() {return _zValue;}

  double3 applyFullCellBoundaryCondition(double3 dr);
  double3 applyUnitCellBoundaryCondition(double3 dr);

  inline double3 convertToCartesian(double3 s) {return _unitCell * s;}
  inline double3 convertToFractional(double3 r) {return _inverseUnitCell * r;}
  inline double3 convertToNormalizedFractional(double3 r) {return (_inverseFullCell * r).fract();}

  int3 numberOfReplicas(double cutoff);

  SKBoundingBox boundingBox() const {return _boundingBox;}
  void setBoundingBox(SKBoundingBox boundingBox) {_boundingBox = boundingBox;}

  std::vector<float4> renderTranslationVectors();

  double precision() const {return _precision;}
  void setPrecision(double precision) {_precision = precision;}

  double3 contentShift() {return _contentShift;}
  void setContentShift(double3 value) {_contentShift = value;}
  void setContentShiftX(double value) {_contentShift.x = value;}
  void setContentShiftY(double value) {_contentShift.y = value;}
  void setContentShiftZ(double value) {_contentShift.z = value;}
  bool3 contentFlip() {return _contentFlip;}
  void setContentFlip(bool3 value) {_contentFlip = value;}
  void setContentFlipX(bool value) {_contentFlip.x = value;}
  void setContentFlipY(bool value) {_contentFlip.y = value;}
  void setContentFlipZ(bool value) {_contentFlip.z = value;}
private:
  qint64 _versionNumber{2};

  int _zValue{1};

  int3 _minimumReplica = int3(0,0,0);
  int3 _maximumReplica = int3(0,0,0);

  double3x3 _unitCell = double3x3();
  double3x3 _inverseUnitCell = double3x3();

  double3x3 _fullCell = double3x3();
  double3x3 _inverseFullCell = double3x3();

  SKBoundingBox _boundingBox = SKBoundingBox();

  double3 _contentShift = double3(0.0,0.0,0.0);
  bool3 _contentFlip = bool3(false,false,false);

  double3 _origin = double3(0.0, 0.0, 0.0);

  double _precision = 1e-2;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<SKCell> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<SKCell> &);
};

