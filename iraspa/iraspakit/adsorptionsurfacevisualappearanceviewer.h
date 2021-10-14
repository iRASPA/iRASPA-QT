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

#include <mathkit.h>
#include "rkrenderkitprotocols.h"

enum class ProbeMolecule: qint64
{
  helium = 0, methane = 1, nitrogen = 2, hydrogen = 3, water = 4, co2 = 5, xenon = 6, krypton = 7, argon = 8, multiple_values = 9
};


class AdsorptionSurfaceVisualAppearanceViewer
{
public:

  virtual  ~AdsorptionSurfaceVisualAppearanceViewer() = 0;

  virtual RKEnergySurfaceType adsorptionSurfaceRenderingMethod() = 0;
  virtual void setAdsorptionSurfaceRenderingMethod(RKEnergySurfaceType type) = 0;
  virtual RKPredefinedVolumeRenderingTransferFunction adsorptionVolumeTransferFunction() = 0;
  virtual void setAdsorptionVolumeTransferFunction(RKPredefinedVolumeRenderingTransferFunction function) = 0;
  virtual double adsorptionVolumeStepLength() = 0;
  virtual void setAdsorptionVolumeStepLength(double value) = 0;


  virtual std::vector<double3> atomUnitCellPositions() const = 0;
  virtual bool drawAdsorptionSurface() const  = 0;
  virtual void setDrawAdsorptionSurface(bool state)  = 0;
  virtual double adsorptionSurfaceOpacity() const  = 0;
  virtual void setAdsorptionSurfaceOpacity(double value)  = 0;
  virtual double adsorptionSurfaceIsoValue() const  = 0;
  virtual void setAdsorptionSurfaceIsoValue(double value)  = 0;
  virtual double adsorptionSurfaceMinimumValue() const  = 0;
  virtual void setAdsorptionSurfaceMinimumValue(double value)  = 0;

  virtual int adsorptionSurfaceSize() const = 0;
  virtual double2 adsorptionSurfaceProbeParameters() const = 0;
  // int adsorptionSurfaceNumberOfTriangles() const = 0;

  virtual ProbeMolecule adsorptionSurfaceProbeMolecule() const = 0;
  virtual void setAdsorptionSurfaceProbeMolecule(ProbeMolecule value) = 0;

  virtual bool adsorptionSurfaceFrontSideHDR() const = 0;
  virtual void setAdsorptionSurfaceFrontSideHDR(bool state) = 0;
  virtual double adsorptionSurfaceFrontSideHDRExposure() const = 0;
  virtual void setAdsorptionSurfaceFrontSideHDRExposure(double value) = 0;
  virtual QColor adsorptionSurfaceFrontSideAmbientColor() const = 0;
  virtual void setAdsorptionSurfaceFrontSideAmbientColor(QColor color) = 0;
  virtual QColor adsorptionSurfaceFrontSideDiffuseColor() const = 0;
  virtual void setAdsorptionSurfaceFrontSideDiffuseColor(QColor color) = 0;
  virtual QColor adsorptionSurfaceFrontSideSpecularColor() const = 0;
  virtual void setAdsorptionSurfaceFrontSideSpecularColor(QColor color) = 0;
  virtual double adsorptionSurfaceFrontSideDiffuseIntensity() const = 0;
  virtual void setAdsorptionSurfaceFrontSideDiffuseIntensity(double value) = 0;
  virtual double adsorptionSurfaceFrontSideAmbientIntensity() const = 0;
  virtual void setAdsorptionSurfaceFrontSideAmbientIntensity(double value) = 0;
  virtual double adsorptionSurfaceFrontSideSpecularIntensity() const = 0;
  virtual void setAdsorptionSurfaceFrontSideSpecularIntensity(double value) = 0;
  virtual double adsorptionSurfaceFrontSideShininess() const = 0;
  virtual void setAdsorptionSurfaceFrontSideShininess(double value) = 0;

  virtual double adsorptionSurfaceHue() const = 0;
  virtual void setAdsorptionSurfaceHue(double value) = 0;
  virtual double adsorptionSurfaceSaturation() const = 0;
  virtual void setAdsorptionSurfaceSaturation(double value) = 0;
  virtual double adsorptionSurfaceValue() const = 0;
  virtual void setAdsorptionSurfaceValue(double value) = 0;

  virtual bool adsorptionSurfaceBackSideHDR() const = 0;
  virtual void setAdsorptionSurfaceBackSideHDR(bool state) = 0;
  virtual double adsorptionSurfaceBackSideHDRExposure() const = 0;
  virtual void setAdsorptionSurfaceBackSideHDRExposure(double value) = 0;
  virtual QColor adsorptionSurfaceBackSideAmbientColor() const = 0;
  virtual void setAdsorptionSurfaceBackSideAmbientColor(QColor color) = 0;
  virtual QColor adsorptionSurfaceBackSideDiffuseColor() const = 0;
  virtual void setAdsorptionSurfaceBackSideDiffuseColor(QColor color) = 0;
  virtual QColor adsorptionSurfaceBackSideSpecularColor() const = 0;
  virtual void setAdsorptionSurfaceBackSideSpecularColor(QColor color) = 0;
  virtual double adsorptionSurfaceBackSideDiffuseIntensity() const = 0;
  virtual void setAdsorptionSurfaceBackSideDiffuseIntensity(double value) = 0;
  virtual double adsorptionSurfaceBackSideAmbientIntensity() const = 0;
  virtual void setAdsorptionSurfaceBackSideAmbientIntensity(double value) = 0;
  virtual double adsorptionSurfaceBackSideSpecularIntensity() const = 0;
  virtual void setAdsorptionSurfaceBackSideSpecularIntensity(double value) = 0;
  virtual double adsorptionSurfaceBackSideShininess() const = 0;
  virtual void setAdsorptionSurfaceBackSideShininess(double value) = 0;
};
