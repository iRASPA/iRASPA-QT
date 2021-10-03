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

#include <QObject>
#include <renderkit.h>
#include <symmetrykit.h>
#include <simulationkit.h>

class AtomVisualAppearanceViewer
{
public:
  enum class RepresentationType: qint64
  {
    sticks_and_balls = 0, vdw = 1, unity = 2, multiple_values = 3
  };

  enum class RepresentationStyle: qint64
  {
    custom = -1, defaultStyle = 0, fancy = 1, licorice = 2, objects = 3, multiple_values = 4
  };

  virtual ~AtomVisualAppearanceViewer() = 0;

  virtual bool drawAtoms() const = 0;
  virtual void setDrawAtoms(bool drawAtoms) = 0;

  virtual void setRepresentationType(RepresentationType) = 0;
  virtual RepresentationType atomRepresentationType() = 0;
  virtual void setRepresentationStyle(RepresentationStyle style) = 0;
  virtual void setRepresentationStyle(RepresentationStyle style, const SKColorSets &colorSets) = 0;
  virtual RepresentationStyle atomRepresentationStyle() = 0;
  virtual bool isUnity() const = 0;
  virtual void recheckRepresentationStyle() = 0;

  virtual void setRepresentationColorSchemeIdentifier(const QString colorSchemeName, const SKColorSets &colorSets) = 0;
  virtual QString atomColorSchemeIdentifier() = 0;
  virtual void setColorSchemeOrder(SKColorSet::ColorSchemeOrder order) = 0;
  virtual SKColorSet::ColorSchemeOrder colorSchemeOrder() = 0;

  virtual QString atomForceFieldIdentifier() = 0;
  virtual void setAtomForceFieldIdentifier(QString identifier, ForceFieldSets &forceFieldSets) = 0;
  virtual void updateForceField(ForceFieldSets &forceFieldSets) = 0;
  virtual void setForceFieldSchemeOrder(ForceFieldSet::ForceFieldSchemeOrder order) = 0;
  virtual ForceFieldSet::ForceFieldSchemeOrder forceFieldSchemeOrder() = 0;

  virtual QColor atomAmbientColor() const = 0;
  virtual void setAtomAmbientColor(QColor color) = 0;
  virtual QColor atomDiffuseColor() const = 0;
  virtual void setAtomDiffuseColor(QColor color) = 0;
  virtual QColor atomSpecularColor() const = 0;
  virtual void setAtomSpecularColor(QColor color) = 0;

  virtual double atomAmbientIntensity() const = 0;
  virtual void setAtomAmbientIntensity(double value) = 0;
  virtual double atomDiffuseIntensity() const = 0;
  virtual void setAtomDiffuseIntensity(double value) = 0;
  virtual double atomSpecularIntensity() const = 0;
  virtual void setAtomSpecularIntensity(double value) = 0;
  virtual double atomShininess() const = 0;
  virtual void setAtomShininess(double value) = 0;

  virtual double atomHue() const = 0;
  virtual void setAtomHue(double value) = 0;
  virtual double atomSaturation() const = 0;
  virtual void setAtomSaturation(double value) = 0;
  virtual double atomValue() const = 0;
  virtual void setAtomValue(double value) = 0;

  virtual double atomScaleFactor() const = 0;
  virtual void setAtomScaleFactor(double size) = 0;

  virtual bool atomAmbientOcclusion() const = 0;
  virtual void setAtomAmbientOcclusion(bool value) = 0;
  virtual int atomAmbientOcclusionPatchNumber() const = 0;
  virtual void setAtomAmbientOcclusionPatchNumber(int patchNumber) = 0;
  virtual int atomAmbientOcclusionPatchSize() const = 0;
  virtual void setAtomAmbientOcclusionPatchSize(int patchSize) = 0;
  virtual int atomAmbientOcclusionTextureSize() const = 0;
  virtual void setAtomAmbientOcclusionTextureSize(int size) = 0;

  virtual bool atomHDR() const = 0;
  virtual void setAtomHDR(bool value) = 0;
  virtual double atomHDRExposure() const = 0;
  virtual void setAtomHDRExposure(double value) = 0;

  virtual RKSelectionStyle atomSelectionStyle() const = 0;
  virtual void setAtomSelectionStyle(RKSelectionStyle style) = 0;
  virtual double atomSelectionScaling() const = 0;
  virtual void setAtomSelectionScaling(double scaling) = 0;
  virtual double atomSelectionIntensity() const = 0;
  virtual void setSelectionIntensity(double scaling) = 0;

  virtual double atomSelectionStripesDensity() const = 0;
  virtual void setAtomSelectionStripesDensity(double value) = 0;
  virtual double atomSelectionStripesFrequency() const = 0;
  virtual void setAtomSelectionStripesFrequency(double value) = 0;
  virtual double atomSelectionWorleyNoise3DFrequency() const = 0;
  virtual void setAtomSelectionWorleyNoise3DFrequency(double value) = 0;
  virtual double atomSelectionWorleyNoise3DJitter() const = 0;
  virtual void setAtomSelectionWorleyNoise3DJitter(double value) = 0;

  virtual double atomSelectionFrequency() const = 0;
  virtual void setAtomSelectionFrequency(double value) = 0;
  virtual double atomSelectionDensity() const = 0;
  virtual void setAtomSelectionDensity(double value) = 0;
};
