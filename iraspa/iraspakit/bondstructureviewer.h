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

#include <QColor>
#include <renderkit.h>


class BondStructureViewer
{
public:
  virtual ~BondStructureViewer() = 0;

  virtual bool drawBonds() const = 0;
  virtual double bondScaleFactor() const = 0;
  virtual RKBondColorMode bondColorMode() const = 0;

  virtual bool bondAmbientOcclusion() const = 0;

  virtual bool bondHDR() const = 0;
  virtual double bondHDRExposure() const = 0;

  virtual double bondHue() const = 0;
  virtual double bondSaturation() const = 0;
  virtual double bondValue() const = 0;

  virtual QColor bondAmbientColor() const = 0;
  virtual QColor bondDiffuseColor() const = 0;
  virtual QColor bondSpecularColor() const = 0;
  virtual double bondAmbientIntensity() const = 0;
  virtual double bondDiffuseIntensity() const = 0;
  virtual double bondSpecularIntensity() const = 0;
  virtual double bondShininess() const = 0;

  virtual RKSelectionStyle bondSelectionStyle() const = 0;
  virtual double bondSelectionFrequency() const = 0;
  virtual double bondSelectionDensity() const  = 0;
  virtual double bondSelectionIntensity() const = 0;
  virtual double bondSelectionScaling() const = 0;
};

class BondStructureEditor: public BondStructureViewer
{
public:
  virtual ~BondStructureEditor() = 0;

  virtual void setDrawBonds(bool drawBonds) = 0;
  virtual void setBondScaleFactor(double value) = 0;
  virtual void setBondColorMode(RKBondColorMode value) = 0;

  virtual void setBondAmbientOcclusion(bool state) = 0;

  virtual void setBondHDR(bool value) = 0;
  virtual void setBondHDRExposure(double value) = 0;

  virtual void setBondHue(double value) = 0;
  virtual void setBondSaturation(double value) = 0;
  virtual void setBondValue(double value) = 0;

  virtual void setBondAmbientColor(QColor color) = 0;
  virtual void setBondDiffuseColor(QColor color) = 0;
  virtual void setBondSpecularColor(QColor color) = 0;
  virtual void setBondAmbientIntensity(double value) = 0;
  virtual void setBondDiffuseIntensity(double value) = 0;
  virtual void setBondSpecularIntensity(double value) = 0;
  virtual void setBondShininess(double value) = 0;

  virtual void setBondSelectionStyle(RKSelectionStyle style) = 0;
  virtual void setBondSelectionFrequency(double value) = 0;
  virtual void setBondSelectionDensity(double value) = 0;
  virtual void setBondSelectionIntensity(double value) = 0;
  virtual void setBondSelectionScaling(double scaling) = 0;
};
