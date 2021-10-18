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

#include <QObject>
#include <renderkit.h>
#include "skboundingbox.h"
#include "skcell.h"
#include "displayable.h"


enum class ObjectType : qint64
{
  none = -1, object = 0, structure = 1, crystal = 2, molecularCrystal = 3, molecule = 4, protein = 5, proteinCrystal = 6,
  proteinCrystalSolvent = 7, crystalSolvent = 8, molecularCrystalSolvent = 9,
  crystalEllipsoidPrimitive = 10, crystalCylinderPrimitive = 11, crystalPolygonalPrismPrimitive = 12,
  ellipsoidPrimitive = 13, cylinderPrimitive = 14, polygonalPrismPrimitive = 15,
  gridVolume = 16, RASPADensityVolume = 17, VTKDensityVolume = 18, VASPDensityVolume = 19, GaussianCubeVolume = 20
};

class Object: public DisplayableProtocol, public RKRenderStructure
{
public:
  Object();
  virtual ~Object() {;}

  virtual std::shared_ptr<Object> shallowClone();

  virtual ObjectType structureType()  {return ObjectType::object;}

  Object(const std::shared_ptr<const SKStructure> structure);
  Object(const Object &object);
  Object(const std::shared_ptr<const Object> object);

  QString displayName() const override final {return _displayName;}
  void setDisplayName(QString name) override final {_displayName = name;}

  bool isVisible() const override  final{return _isVisible;}
  void setVisibility(bool visibility) {_isVisible = visibility;}

  std::shared_ptr<SKCell> cell() const override final {return _cell;}

  void setCell(std::shared_ptr<SKCell> cell) {_cell = cell;}

  virtual SKBoundingBox boundingBox()  const override;
  SKBoundingBox transformedBoundingBox() const override;
  virtual void reComputeBoundingBox();

  double rotationDelta() {return _rotationDelta;}
  void setRotationDelta(double angle) {_rotationDelta = angle;}
  void setOrientation(simd_quatd orientation) {_orientation = orientation;}
  simd_quatd orientation() const override final {return _orientation;}
  double3 origin() const override final {return _origin;}
  void setOrigin(double3 value) {_origin = value;}
  void setOriginX(double value) {_origin.x = value;}
  void setOriginY(double value) {_origin.y = value;}
  void setOriginZ(double value) {_origin.z = value;}

  // unit cell
  bool drawUnitCell() const override {return _drawUnitCell;}
  void setDrawUnitCell(bool state) {_drawUnitCell = state;}
  double unitCellScaleFactor() const override {return _unitCellScaleFactor;}
  void setUnitCellScaleFactor(double value) {_unitCellScaleFactor = value;}
  QColor unitCellDiffuseColor() const override {return _unitCellDiffuseColor;}
  void setUnitCellDiffuseColor(QColor color) {_unitCellDiffuseColor = color;}
  double unitCellDiffuseIntensity() const override {return _unitCellDiffuseIntensity;}
  void setUnitCellDiffuseIntensity(double value) {_unitCellDiffuseIntensity = value;}

  // local axes
  RKLocalAxes &renderLocalAxes() override {return _localAxes;}

  std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override  {return {};}
  std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override {return {};}

  // info
  QString authorFirstName() {return _authorFirstName;}
  void setAuthorFirstName(QString name) {_authorFirstName = name;}
  QString authorMiddleName() {return _authorMiddleName;}
  void setAuthorMiddleName(QString name) {_authorMiddleName = name;}
  QString authorLastName()  {return _authorLastName;}
  void setAuthorLastName(QString name) {_authorLastName = name;}
  QString authorOrchidID() {return _authorOrchidID;}
  void setAuthorOrchidID(QString name) {_authorOrchidID = name;}
  QString authorResearcherID() {return _authorResearcherID;}
  void setAuthorResearcherID(QString name) {_authorResearcherID = name;}
  QString authorAffiliationUniversityName() {return _authorAffiliationUniversityName;}
  void setAuthorAffiliationUniversityName(QString name) {_authorAffiliationUniversityName = name;}
  QString authorAffiliationFacultyName() {return _authorAffiliationFacultyName;}
  void setAuthorAffiliationFacultyName(QString name) {_authorAffiliationFacultyName = name;}
  QString authorAffiliationInstituteName()  {return _authorAffiliationInstituteName;}
  void setAuthorAffiliationInstituteName(QString name) {_authorAffiliationInstituteName = name;}
  QString authorAffiliationCityName() {return _authorAffiliationCityName;}
  void setAuthorAffiliationCityName(QString name) {_authorAffiliationCityName = name;}
  QString authorAffiliationCountryName() {return _authorAffiliationCountryName;}
  void setAuthorAffiliationCountryName(QString name) {_authorAffiliationCountryName = name;}

  QDate creationDate() {return _creationDate;}
  void setCreationDate(QDate date) {_creationDate = date;}
protected:
  QString _displayName;
  bool _isVisible = true;

  double3 _origin = double3(0.0, 0.0, 0.0);
  double3 _scaling = double3(1.0, 1.0, 1.0);
  bool _periodic = false;
  simd_quatd _orientation = simd_quatd(1.0, double3(0.0, 0.0, 0.0));
  double _rotationDelta = 5.0;
  std::shared_ptr<SKCell> _cell;

  bool _drawUnitCell = true;
  double _unitCellScaleFactor = 1.0;
  QColor _unitCellDiffuseColor = QColor(255, 255, 255, 255);
  double _unitCellDiffuseIntensity = 1.0;

  RKLocalAxes _localAxes;

  QString _authorFirstName = QString("");
  QString _authorMiddleName = QString("");
  QString _authorLastName = QString("");
  QString _authorOrchidID = QString("");
  QString _authorResearcherID = QString("");
  QString _authorAffiliationUniversityName = QString("");
  QString _authorAffiliationFacultyName = QString("");
  QString _authorAffiliationInstituteName = QString("");
  QString _authorAffiliationCityName = QString("");
  QString _authorAffiliationCountryName = QString("Netherlands");

  QDate _creationDate = QDate().currentDate();
private:
  qint64 _versionNumber{1};


  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Object> &object);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Object> &object);
};
