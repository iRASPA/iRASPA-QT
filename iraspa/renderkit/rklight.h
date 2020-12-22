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

#include <foundationkit.h>
#include <symmetrykit.h>
#include <mathkit.h>
#include <QColor>

class RKLight
{
public:
  RKLight();
  double4 position() const {return _position;}
  void setPosition(double4 position) {_position = position;}
  QColor ambientColor()  const {return _ambientColor;}
  void setAmbientColor(QColor color) {_ambientColor = color;}
  QColor diffuseColor()  const {return _diffuseColor;}
  void setDiffuseColor(QColor color) {_diffuseColor = color;}
  QColor specularColor()  const {return _specularColor;}
  void setSpecularColor(QColor color) {_specularColor = color;}
  double ambientIntensity()  const {return _ambientIntensity;}
  void setAmbientIntensity(double intensity) {_ambientIntensity = intensity;}
  double diffuseIntensity()  const {return _diffuseIntensity;}
  void setDiffuseIntensity(double intensity) {_diffuseIntensity = intensity;}
  double specularIntensity()  const {return _specularIntensity;}
  void setSpecularIntensity(double intensity) {_specularIntensity = intensity;}
  double shininess() const {return _shininess;}
  void setShininess(double shininess) {_shininess = shininess;}
  double constantAttenuation()  const {return _constantAttenuation;}
  void setConstantAttenuation(double attenuation) {_constantAttenuation = attenuation;}
  double linearAttenuation() const {return _linearAttenuation;}
  void setLinearAttenuation(double attenuation) {_linearAttenuation = attenuation;}
  double quadraticAttenuation() const {return _quadraticAttenuation;}
  void setQuadraticAttenuation(double attenuation) {_quadraticAttenuation = attenuation;}
  double3 spotDirection() const {return _spotDirection;}
  void setSpotDirection(double3 direction) {_spotDirection = direction;}
  double spotCutoff() const {return _spotCutoff;}
  void setSpotCutoff(double cutoff) {_spotCutoff = cutoff;}
  double spotExponent() const {return _spotExponent;}
  void setSpotExponent(double exponent) {_spotExponent = exponent;}
private:
  [[maybe_unused]] int _versionNumber = 1;
  double4 _position = double4(0, 0, 100.0, 0.0);
  QColor _ambientColor = QColor(255, 255, 255, 255);
  QColor _diffuseColor = QColor(255, 255, 255, 255);
  QColor _specularColor = QColor(255, 255, 255, 255);
  double _ambientIntensity = 1.0;
  double _diffuseIntensity = 1.0;
  double _specularIntensity = 1.0;
  double _shininess = 4.0;
  double _constantAttenuation = 1.0;
  double _linearAttenuation = 1.0;
  double _quadraticAttenuation = 1.0;
  double3 _spotDirection = double3(1.0, 1.0, 1.0);
  double _spotCutoff = 1.0;
  double _spotExponent = 1.0;
};
