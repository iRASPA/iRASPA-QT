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

#include <QString>
#include <QDate>
#include <mathkit.h>
#include <renderkit.h>

class PrimitiveViewer
{
public:
  virtual ~PrimitiveViewer() = 0;

  virtual simd_quatd primitiveOrientation() const = 0;
  virtual double primitiveRotationDelta() const = 0;
  virtual double3x3 primitiveTransformationMatrix() const = 0;

  virtual double primitiveOpacity() const = 0;
  virtual int primitiveNumberOfSides() const = 0;
  virtual bool primitiveIsCapped() const = 0;

  virtual RKSelectionStyle primitiveSelectionStyle() const = 0;
  virtual double primitiveSelectionScaling() const = 0;
  virtual double primitiveSelectionIntensity() const = 0;

  virtual double primitiveSelectionStripesDensity() const = 0;
  virtual double primitiveSelectionStripesFrequency() const = 0;
  virtual double primitiveSelectionWorleyNoise3DFrequency() const = 0;
  virtual double primitiveSelectionWorleyNoise3DJitter() const = 0;

  virtual double primitiveSelectionFrequency() const  = 0;
  virtual double primitiveSelectionDensity() const = 0;

  virtual double primitiveHue() const  = 0;
  virtual double primitiveSaturation() const  = 0;
  virtual double primitiveValue() const  = 0;

  virtual bool primitiveFrontSideHDR() const = 0;
  virtual double primitiveFrontSideHDRExposure() const = 0;
  virtual double primitiveFrontSideAmbientIntensity() const = 0;
  virtual double primitiveFrontSideDiffuseIntensity() const = 0;
  virtual double primitiveFrontSideSpecularIntensity() const = 0;
  virtual QColor primitiveFrontSideAmbientColor() const = 0;
  virtual QColor primitiveFrontSideDiffuseColor() const = 0;
  virtual QColor primitiveFrontSideSpecularColor() const = 0;
  virtual double primitiveFrontSideShininess() const = 0;

  virtual bool primitiveBackSideHDR() const = 0;
  virtual double primitiveBackSideHDRExposure() const = 0;
  virtual double primitiveBackSideAmbientIntensity() const = 0;
  virtual double primitiveBackSideDiffuseIntensity() const = 0;
  virtual double primitiveBackSideSpecularIntensity() const = 0;
  virtual QColor primitiveBackSideAmbientColor() const = 0;
  virtual QColor primitiveBackSideDiffuseColor() const = 0;
  virtual QColor primitiveBackSideSpecularColor() const = 0;
  virtual double primitiveBackSideShininess() const = 0;
};

class PrimitiveEditor: public PrimitiveViewer
{
public:
  virtual ~PrimitiveEditor() = 0;

  virtual void setPrimitiveOrientation(simd_quatd orientation) = 0;
  virtual void setPrimitiveRotationDelta(double angle) = 0;
  virtual void setPrimitiveTransformationMatrix(double3x3 matrix) = 0;

  virtual void setPrimitiveOpacity(double opacity) = 0;
  virtual void setPrimitiveNumberOfSides(int numberOfSides) = 0;
  virtual void setPrimitiveIsCapped(bool isCapped) = 0;

  virtual void setPrimitiveSelectionStyle(RKSelectionStyle style) = 0;
  virtual void setPrimitiveSelectionScaling(double scaling) = 0;
  virtual void setPrimitiveSelectionIntensity(double value) = 0;

  virtual void setPrimitiveSelectionFrequency(double value) = 0;
  virtual void setPrimitiveSelectionDensity(double value) = 0;

  virtual void setPrimitiveHue(double value)  = 0;
  virtual void setPrimitiveSaturation(double value)  = 0;
  virtual void setPrimitiveValue(double value) = 0;

  virtual void setPrimitiveFrontSideHDR(bool isHDR) = 0;
  virtual void setPrimitiveFrontSideHDRExposure(double exposure) = 0;
  virtual void setPrimitiveFrontSideAmbientIntensity(double intensity) = 0;
  virtual void setPrimitiveFrontSideDiffuseIntensity(double intensity) = 0;
  virtual void setPrimitiveFrontSideSpecularIntensity(double intensity) = 0;
  virtual void setPrimitiveFrontSideAmbientColor(QColor color) = 0;
  virtual void setPrimitiveFrontSideDiffuseColor(QColor color) = 0;
  virtual void setPrimitiveFrontSideSpecularColor(QColor color) = 0;
  virtual void setPrimitiveFrontSideShininess(double value) = 0;

  virtual void setPrimitiveBackSideHDR(bool isHDR) = 0;
  virtual void setPrimitiveBackSideHDRExposure(double exposure) = 0;
  virtual void setPrimitiveBackSideAmbientIntensity(double intensity) = 0;
  virtual void setPrimitiveBackSideDiffuseIntensity(double intensity) = 0;
  virtual void setPrimitiveBackSideSpecularIntensity(double intensity) = 0;
  virtual void setPrimitiveBackSideAmbientColor(QColor color) = 0;
  virtual void setPrimitiveBackSideDiffuseColor(QColor color) = 0;
  virtual void setPrimitiveBackSideSpecularColor(QColor color) = 0;
  virtual void setPrimitiveBackSideShininess(double value) = 0;
};
