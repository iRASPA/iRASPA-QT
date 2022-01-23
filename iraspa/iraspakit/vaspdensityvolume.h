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

#include <QDataStream>
#include "structure.h"
#include "gridvolume.h"
#include "symmetrykit.h"

class VASPDensityVolume: public Structure, public VolumetricDataViewer,
                         public RKRenderUnitCellSource, public RKRenderVolumetricDataSource
{
public:
  VASPDensityVolume();
  VASPDensityVolume(std::shared_ptr<SKStructure> frame);


  // Object
  // ===============================================================================================
  VASPDensityVolume(const std::shared_ptr<Object> object);
  ObjectType structureType() override final { return ObjectType::VASPDensityVolume; }
  std::shared_ptr<Object> shallowClone()  override final;
  SKBoundingBox boundingBox() const override;
  void reComputeBoundingBox() override final;

  // RKRenderAtomSource: overwritten from Structure
  // ===============================================================================================
  std::vector<RKInPerInstanceAttributesAtoms> renderAtoms() const override final;
  std::vector<RKInPerInstanceAttributesAtoms> renderSelectedAtoms() const override final;
  std::vector<RKInPerInstanceAttributesText> atomTextData(RKFontAtlas *fontAtlas) const override final;

  // RKRenderBondSource: overwritten from Structure
  // ===============================================================================================
  std::vector<RKInPerInstanceAttributesBonds> renderInternalBonds() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderExternalBonds() const override final;

  // AtomViewer: overwritten from Structure
  // ===============================================================================================
  bool isFractional() override  {return true;}
  void expandSymmetry() override final;
  void expandSymmetry(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom);
  std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) override final;

  // BondViewer: overwritten from Structure
  // ===============================================================================================
  void computeBonds() override final;

  // Structure: reimplemented
  // ===============================================================================================
  double bondLength(std::shared_ptr<SKBond> bond) const override final;
  double3 bondVector(std::shared_ptr<SKBond> bond) const override final;
  std::pair<double3, double3> computeChangedBondLength(std::shared_ptr<SKBond> bond, double bondlength) const override final;

  // Protocol: RKRenderUnitCellSource
  // ===============================================================================================
  bool drawUnitCell() const override final {return _drawUnitCell;}
  double unitCellScaleFactor() const override final {return _unitCellScaleFactor;}
  QColor unitCellDiffuseColor() const override final {return _unitCellDiffuseColor;}
  double unitCellDiffuseIntensity() const override final {return _unitCellDiffuseIntensity;}
  std::vector<RKInPerInstanceAttributesAtoms> renderUnitCellSpheres() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderUnitCellCylinders() const override final;

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
  ProbeMolecule adsorptionSurfaceProbeMolecule() const override final {return Structure::adsorptionSurfaceProbeMolecule();}
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

private:
  qint64 _versionNumber{1};

  int3 _dimensions;
  double3 _spacing;
  double3 _aspectRatio;
  std::pair<double, double> _range;
  QByteArray _data;
  double _average;
  double _variance;

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<VASPDensityVolume> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<VASPDensityVolume> &);
};
