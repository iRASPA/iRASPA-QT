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
#include "structure.h"

class StructuralPropertyViewer
{
public:
  virtual ~StructuralPropertyViewer() = 0;

  virtual void recomputeDensityProperties() = 0;

  virtual double computeVoidFractionAccelerated() const noexcept(false) = 0;
  virtual double computeNitrogenSurfaceAreaAccelerated() const noexcept(false) = 0;

  virtual double computeVoidFraction() const noexcept = 0 ;
  virtual double computeNitrogenSurfaceArea() const noexcept = 0;

  //   var structureType: Structure.StructureType {get}
  virtual QString structureMaterialType() const = 0;
  virtual ProbeMolecule frameworkProbeMolecule() const = 0;
  virtual double structureMass() const = 0;
  virtual double structureDensity() const = 0;
  virtual double structureHeliumVoidFraction() const = 0;
  virtual double structureSpecificVolume() const = 0;
  virtual double structureAccessiblePoreVolume() const = 0;
  virtual double structureVolumetricNitrogenSurfaceArea() const = 0;
  virtual double structureGravimetricNitrogenSurfaceArea() const = 0;
  virtual int structureNumberOfChannelSystems() const = 0;
  virtual int structureNumberOfInaccessiblePockets() const = 0;
  virtual int structureDimensionalityOfPoreSystem() const = 0;
  virtual double structureLargestCavityDiameter() const = 0;
  virtual double structureRestrictingPoreLimitingDiameter() const = 0;
  virtual double structureLargestCavityDiameterAlongAViablePath() const = 0;
};

class StructuralPropertyEditor: public StructuralPropertyViewer
{
public:
  virtual ~StructuralPropertyEditor() = 0;

  //   var structureType: Structure.StructureType {get}
  virtual void setStructureMaterialType(QString value) = 0;
  virtual void setFrameworkProbeMolecule(ProbeMolecule molecule) = 0;
  virtual void setStructureMass(double value) = 0;
  virtual void setStructureDensity(double value) = 0;
  virtual void setStructureHeliumVoidFraction(double value) = 0;
  virtual void setStructureSpecificVolume(double value) = 0;
  virtual void setStructureAccessiblePoreVolume(double value) = 0;
  virtual void setStructureVolumetricNitrogenSurfaceArea(double value) = 0;
  virtual void setStructureGravimetricNitrogenSurfaceArea(double value) = 0;
  virtual void setStructureNumberOfChannelSystems(int value) = 0;
  virtual void setStructureNumberOfInaccessiblePockets(int value) = 0;
  virtual void setStructureDimensionalityOfPoreSystem(int value) = 0;
  virtual void setStructureLargestCavityDiameter(double value) = 0;
  virtual void setStructureRestrictingPoreLimitingDiameter(double value) = 0;
  virtual void setStructureLargestCavityDiameterAlongAViablePath(double value) = 0;
};

