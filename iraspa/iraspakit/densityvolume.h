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

#pragma  once

#include <vector>
#include <tuple>
#include <mathkit.h>
#include <renderkit.h>
#include "object.h"
#include "infoviewer.h"

class DensityVolume: public Object, public RKRenderDensityVolumeSource, public BasicInfoViewer
{
public:
  DensityVolume();
  DensityVolume(std::shared_ptr<SKStructure> frame);

  //std::shared_ptr<Object> clone() override final;
  ObjectType structureType() override final { return ObjectType::densityVolume; }

  SKBoundingBox boundingBox()  const override;
  void reComputeBoundingBox() final override;

  int3 dimension() override {return _dimensions;}
  double3 spacing() override {return _spacing;}
  double3 aspectRatio() override;
  std::pair<double, double> range() override {return _range;}
  std::vector<float4> data() override final {return _data;}

  std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override final;

  // info
  QString authorFirstName() override final {return _authorFirstName;}
  void setAuthorFirstName(QString name) override final {_authorFirstName = name;}
  QString authorMiddleName() override final {return _authorMiddleName;}
  void setAuthorMiddleName(QString name) override final {_authorMiddleName = name;}
  QString authorLastName() override final {return _authorLastName;}
  void setAuthorLastName(QString name) override final  {_authorLastName = name;}
  QString authorOrchidID() override final {return _authorOrchidID;}
  void setAuthorOrchidID(QString name) override final {_authorOrchidID = name;}
  QString authorResearcherID() override final {return _authorResearcherID;}
  void setAuthorResearcherID(QString name) override final {_authorResearcherID = name;}
  QString authorAffiliationUniversityName() override final {return _authorAffiliationUniversityName;}
  void setAuthorAffiliationUniversityName(QString name) override final {_authorAffiliationUniversityName = name;}
  QString authorAffiliationFacultyName() override final {return _authorAffiliationFacultyName;}
  void setAuthorAffiliationFacultyName(QString name) override final {_authorAffiliationFacultyName = name;}
  QString authorAffiliationInstituteName() override final {return _authorAffiliationInstituteName;}
  void setAuthorAffiliationInstituteName(QString name) override final {_authorAffiliationInstituteName = name;}
  QString authorAffiliationCityName() override final {return _authorAffiliationCityName;}
  void setAuthorAffiliationCityName(QString name) override final {_authorAffiliationCityName = name;}
  QString authorAffiliationCountryName() override final {return _authorAffiliationCountryName;}
  void setAuthorAffiliationCountryName(QString name) override final {_authorAffiliationCountryName = name;}

private:
  qint64 _versionNumber{1};

  inline unsigned modulo( int value, unsigned m) {
      int mod = value % (int)m;
      if (mod < 0) {
          mod += m;
      }
      return mod;
  }

  inline float Images(size_t x,size_t y,size_t z)
  {
    size_t index = modulo(x,_dimensions.x)+modulo(y,_dimensions.y)*_dimensions.y+modulo(z,_dimensions.z)*_dimensions.x*_dimensions.y;

    return _data[index].x;
  }

  //inline float4 operator() (int i,int j, int k) {return _data[i+j*_dimensions.y+k*_dimensions.x*_dimensions.y];}

  int3 _dimensions;
  double3 _spacing;
  double3 _aspectRatio;
  std::pair<double, double> _range;
  std::vector<float4> _data;

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

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<DensityVolume> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<DensityVolume> &);
};
