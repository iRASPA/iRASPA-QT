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
#include <skcell.h>
#include "object.h"
#include "adsorptionsurfacevisualappearanceviewer.h"

class CellViewer
{
public:

  virtual ~CellViewer() = 0;

  //virtual void setCell(std::shared_ptr<SKCell> cell) = 0;
  //virtual std::shared_ptr<SKCell> cell() const = 0;

  virtual ProbeMolecule frameworkProbeMolecule() const  = 0;
  virtual void setFrameworkProbeMolecule(ProbeMolecule value)  = 0;
 // virtual void recomputeDensityProperties() = 0;
  virtual QString structureMaterialType() = 0;
  virtual double structureMass() = 0;
  virtual double structureDensity() = 0;
  virtual double structureHeliumVoidFraction() = 0;
  virtual void setStructureHeliumVoidFraction(double value) = 0;
 // virtual void setStructureNitrogenSurfaceArea(double value) = 0;
 // virtual double2 frameworkProbeParameters() = 0;
  virtual double structureSpecificVolume() = 0;
  virtual double structureAccessiblePoreVolume () = 0;
  virtual double structureVolumetricNitrogenSurfaceArea() = 0;
  virtual double structureGravimetricNitrogenSurfaceArea() = 0;
  virtual int structureNumberOfChannelSystems() = 0;
  virtual void setStructureNumberOfChannelSystems(int value) = 0;
  virtual int structureNumberOfInaccessiblePockets() = 0;
  virtual void setStructureNumberOfInaccessiblePockets(int value) = 0;
  virtual int structureDimensionalityOfPoreSystem() = 0;
  virtual void setStructureDimensionalityOfPoreSystem(int value) = 0;
  virtual double structureLargestCavityDiameter() = 0;
  virtual void setStructureLargestCavityDiameter(double value) = 0;
  virtual double structureRestrictingPoreLimitingDiameter() = 0;
  virtual void setStructureRestrictingPoreLimitingDiameter(double value) = 0;
  virtual double structureLargestCavityDiameterAlongAviablePath() = 0;
  virtual void setStructureLargestCavityDiameterAlongAviablePath(double value) = 0;
};
