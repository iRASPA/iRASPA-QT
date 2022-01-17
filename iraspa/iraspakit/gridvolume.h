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
#include "structure.h"

class GridVolume: public Object, public VolumetricDataViewer,
                  public RKRenderUnitCellSource,  public RKRenderVolumetricDataSource
{
public:
  GridVolume();
  GridVolume(std::shared_ptr<SKStructure> frame);

  // Object
  // ===============================================================================================
  GridVolume(const std::shared_ptr<Object> object);
  ObjectType structureType()  override { return ObjectType::gridVolume; }
  std::shared_ptr<Object> shallowClone() override;
  SKBoundingBox boundingBox() const override;
  void reComputeBoundingBox() override;

  // Protocol: VolumetricDataViewer (many already defined in RKRenderAdsorptionSurfaceSource)
  // ===============================================================================================
  void setDrawAdsorptionSurface(bool state) override final {_drawAdsorptionSurface = state;}
  std::pair<double,double> range() const override final {return _range;}
  double3 spacing() const override final {return _spacing;}
  QByteArray data() const override final {return _data;}
  double average() const override final {return _average;}
  double variance() const override final {return _variance;}

  void setAdsorptionSurfaceOpacity(double value) override final {_adsorptionSurfaceOpacity = value;}
  void setAdsorptionTransparencyThreshold(double value) override final {_adsorptionTransparencyThreshold = value;}
  void setAdsorptionSurfaceIsoValue(double value) override final {_adsorptionSurfaceIsoValue = value;}
  ProbeMolecule adsorptionSurfaceProbeMolecule() const override final {return ProbeMolecule::helium;} // TODO: change to 'none'
  void setAdsorptionSurfaceProbeMolecule(ProbeMolecule value) override final {_adsorptionSurfaceProbeMolecule = value;}

  void setAdsorptionSurfaceRenderingMethod(RKEnergySurfaceType type) override final {_adsorptionSurfaceRenderingMethod = type;}
  void setAdsorptionVolumeTransferFunction(RKPredefinedVolumeRenderingTransferFunction function) override final {_adsorptionVolumeTransferFunction = function;}
  void setAdsorptionVolumeStepLength(double value) override final {_adsorptionVolumeStepLength = value;}

  void setAdsorptionSurfaceHue(double value) override final {_adsorptionSurfaceHue = value;}
  void setAdsorptionSurfaceSaturation(double value) override final {_adsorptionSurfaceSaturation = value;}
  void setAdsorptionSurfaceValue(double value) override final {_adsorptionSurfaceValue = value;}

  void setAdsorptionSurfaceFrontSideHDR(bool state) override final {_adsorptionSurfaceFrontSideHDR = state;}
  void setAdsorptionSurfaceFrontSideHDRExposure(double value) override final {_adsorptionSurfaceFrontSideHDRExposure = value;}
  void setAdsorptionSurfaceFrontSideAmbientColor(QColor color) override final {_adsorptionSurfaceFrontSideAmbientColor = color;}
  void setAdsorptionSurfaceFrontSideDiffuseColor(QColor color) override final {_adsorptionSurfaceFrontSideDiffuseColor = color;}
  void setAdsorptionSurfaceFrontSideSpecularColor(QColor color) override final {_adsorptionSurfaceFrontSideSpecularColor = color;}
  void setAdsorptionSurfaceFrontSideDiffuseIntensity(double value) override final {_adsorptionSurfaceFrontSideDiffuseIntensity = value;}
  void setAdsorptionSurfaceFrontSideAmbientIntensity(double value) override final {_adsorptionSurfaceFrontSideAmbientIntensity = value;}
  void setAdsorptionSurfaceFrontSideSpecularIntensity(double value) override final {_adsorptionSurfaceFrontSideSpecularIntensity = value;}
  void setAdsorptionSurfaceFrontSideShininess(double value) override final {_adsorptionSurfaceFrontSideShininess = value;}

  void setAdsorptionSurfaceBackSideHDR(bool state) override final {_adsorptionSurfaceBackSideHDR = state;}
  void setAdsorptionSurfaceBackSideHDRExposure(double value) override final {_adsorptionSurfaceBackSideHDRExposure = value;}
  void setAdsorptionSurfaceBackSideAmbientColor(QColor color) override final {_adsorptionSurfaceBackSideAmbientColor = color;}
  void setAdsorptionSurfaceBackSideDiffuseColor(QColor color) override final {_adsorptionSurfaceBackSideDiffuseColor = color;}
  void setAdsorptionSurfaceBackSideSpecularColor(QColor color) override final {_adsorptionSurfaceBackSideSpecularColor = color;}
  void setAdsorptionSurfaceBackSideDiffuseIntensity(double value) override final {_adsorptionSurfaceBackSideDiffuseIntensity = value;}
  void setAdsorptionSurfaceBackSideAmbientIntensity(double value) override final {_adsorptionSurfaceBackSideAmbientIntensity = value;}
  void setAdsorptionSurfaceBackSideSpecularIntensity(double value) override final {_adsorptionSurfaceBackSideSpecularIntensity = value;}
  void setAdsorptionSurfaceBackSideShininess(double value) override final {_adsorptionSurfaceBackSideShininess = value;}

  // Protocol: RKRenderUnitCellSource
  // ===============================================================================================

  bool drawUnitCell() const override {return _drawUnitCell;}
  double unitCellScaleFactor() const override {return _unitCellScaleFactor;}
  QColor unitCellDiffuseColor() const override {return _unitCellDiffuseColor;}
  double unitCellDiffuseIntensity() const override {return _unitCellDiffuseIntensity;}
  std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override;
  std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override;

  // Protocol: RKRenderAdsorptionSurfaceSource
  // ===============================================================================================
  bool drawAdsorptionSurface() const override final {return _drawAdsorptionSurface;}
  int3 dimensions() const override final {return _dimensions;}
  std::vector<float> gridData() override final;
  std::vector<float4> gridValueAndGradientData() override final;
  bool isImmutable() const override final {return true;}

  RKEnergySurfaceType adsorptionSurfaceRenderingMethod() const override final {return _adsorptionSurfaceRenderingMethod;}
  RKPredefinedVolumeRenderingTransferFunction adsorptionVolumeTransferFunction() const override final {return _adsorptionVolumeTransferFunction;}
  double adsorptionVolumeStepLength() const override final {return _adsorptionVolumeStepLength;}

  double adsorptionSurfaceOpacity() const override final {return _adsorptionSurfaceOpacity;}
  double adsorptionTransparencyThreshold() const override final {return _adsorptionTransparencyThreshold;}
  double adsorptionSurfaceIsoValue() const override final {return _adsorptionSurfaceIsoValue;}
  int encompassingPowerOfTwoCubicGridSize() const override final {return _encompassingPowerOfTwoCubicGridSize;}

  double adsorptionSurfaceHue() const override final {return _adsorptionSurfaceHue;}
  double adsorptionSurfaceSaturation() const override final {return _adsorptionSurfaceSaturation;}
  double adsorptionSurfaceValue() const override final {return _adsorptionSurfaceValue;}

  bool adsorptionSurfaceFrontSideHDR() const override final {return _adsorptionSurfaceFrontSideHDR;}
  double adsorptionSurfaceFrontSideHDRExposure() const override final {return _adsorptionSurfaceFrontSideHDRExposure;}
  QColor adsorptionSurfaceFrontSideAmbientColor() const override final {return _adsorptionSurfaceFrontSideAmbientColor;}
  QColor adsorptionSurfaceFrontSideDiffuseColor() const override final {return _adsorptionSurfaceFrontSideDiffuseColor;}
  QColor adsorptionSurfaceFrontSideSpecularColor() const override final {return _adsorptionSurfaceFrontSideSpecularColor;}
  double adsorptionSurfaceFrontSideDiffuseIntensity() const override final {return _adsorptionSurfaceFrontSideDiffuseIntensity;}
  double adsorptionSurfaceFrontSideAmbientIntensity() const override final {return _adsorptionSurfaceFrontSideAmbientIntensity;}
  double adsorptionSurfaceFrontSideSpecularIntensity() const override final {return _adsorptionSurfaceFrontSideSpecularIntensity;}
  double adsorptionSurfaceFrontSideShininess() const override final {return _adsorptionSurfaceFrontSideShininess;}

  bool adsorptionSurfaceBackSideHDR() const override final {return _adsorptionSurfaceBackSideHDR;}
  double adsorptionSurfaceBackSideHDRExposure() const override final {return _adsorptionSurfaceBackSideHDRExposure;}
  QColor adsorptionSurfaceBackSideAmbientColor() const override final {return _adsorptionSurfaceBackSideAmbientColor;}
  QColor adsorptionSurfaceBackSideDiffuseColor() const override final {return _adsorptionSurfaceBackSideDiffuseColor;}
  QColor adsorptionSurfaceBackSideSpecularColor() const override final {return _adsorptionSurfaceBackSideSpecularColor;}
  double adsorptionSurfaceBackSideDiffuseIntensity() const override final {return _adsorptionSurfaceBackSideDiffuseIntensity;}
  double adsorptionSurfaceBackSideAmbientIntensity() const override final {return _adsorptionSurfaceBackSideAmbientIntensity;}
  double adsorptionSurfaceBackSideSpecularIntensity() const override final {return _adsorptionSurfaceBackSideSpecularIntensity;}
  double adsorptionSurfaceBackSideShininess() const override final {return _adsorptionSurfaceBackSideShininess;}

protected:
  int3 _dimensions;
  double3 _spacing;
  double3 _aspectRatio;
  std::pair<double, double> _range;
  QByteArray _data;
  double _average;
  double _variance;

  bool _drawAdsorptionSurface = false;

  RKEnergySurfaceType _adsorptionSurfaceRenderingMethod = RKEnergySurfaceType::isoSurface;
  RKPredefinedVolumeRenderingTransferFunction  _adsorptionVolumeTransferFunction = RKPredefinedVolumeRenderingTransferFunction::CoolWarmDiverging;
  double _adsorptionVolumeStepLength = 0.0005;

  double _adsorptionSurfaceOpacity = 1.0;
  double _adsorptionTransparencyThreshold = 0.0;
  double _adsorptionSurfaceIsoValue = 0.0;
  int64_t _encompassingPowerOfTwoCubicGridSize= 7;

  ProbeMolecule _adsorptionSurfaceProbeMolecule = ProbeMolecule::helium;

  int _adsorptionSurfaceNumberOfTriangles = 0;

   double _adsorptionSurfaceHue = 1.0;
   double _adsorptionSurfaceSaturation = 1.0;
   double _adsorptionSurfaceValue = 1.0;

   bool _adsorptionSurfaceFrontSideHDR = true;
   double _adsorptionSurfaceFrontSideHDRExposure = 2.0;
   QColor _adsorptionSurfaceFrontSideAmbientColor = QColor(0, 0, 0, 255);
   QColor _adsorptionSurfaceFrontSideDiffuseColor = QColor(255, 255, 255, 255);
   QColor _adsorptionSurfaceFrontSideSpecularColor = QColor(230, 230, 230, 1.0);
   double _adsorptionSurfaceFrontSideDiffuseIntensity = 1.0;
   double _adsorptionSurfaceFrontSideAmbientIntensity = 0.0;
   double _adsorptionSurfaceFrontSideSpecularIntensity = 0.5;
   double _adsorptionSurfaceFrontSideShininess = 4.0;

   bool _adsorptionSurfaceBackSideHDR = true;
   double _adsorptionSurfaceBackSideHDRExposure = 2.0;
   QColor _adsorptionSurfaceBackSideAmbientColor = QColor(0, 0, 0, 255);
   QColor _adsorptionSurfaceBackSideDiffuseColor = QColor(255, 255, 255, 255);
   QColor _adsorptionSurfaceBackSideSpecularColor = QColor(230, 230, 230, 255);
   double _adsorptionSurfaceBackSideDiffuseIntensity = 1.0;
   double _adsorptionSurfaceBackSideAmbientIntensity = 0.0;
   double _adsorptionSurfaceBackSideSpecularIntensity = 0.5;
   double _adsorptionSurfaceBackSideShininess = 4.0;

  inline unsigned modulo( int value, unsigned m) {
      int mod = value % (int)m;
      if (mod < 0) {
          mod += m;
      }
      return mod;
  }

  inline float Images(const std::vector<float4> &data, size_t x,size_t y,size_t z)
  {
    size_t index = modulo(x,_dimensions.x)+modulo(y,_dimensions.y)*_dimensions.y+modulo(z,_dimensions.z)*_dimensions.x*_dimensions.y;

    return data[index].x;
  }

private:
  qint64 _versionNumber{2};

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<GridVolume> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<GridVolume> &);
};
