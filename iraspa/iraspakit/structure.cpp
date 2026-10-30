
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

#include "structure.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <unordered_set>
#include <vector>
#include <symmetrykit.h>
#include <simulationkit.h>
#include <QFontInfo>
#include <QRegularExpression>
#include "skatomtreecontroller.h"
#include "skbondsetcontroller.h"
#include "gridvolume.h"
#include "object.h"

std::optional<AtomInstancePick> decodeAtomInstancePick(RKRenderObject *object, int instanceTag)
{
  if (!object || instanceTag < 0)
  {
    return std::nullopt;
  }
  auto *viewer = dynamic_cast<AtomViewer *>(object);
  if (!viewer)
  {
    return std::nullopt;
  }

  const int replicaCount = object->cell() ? std::max(1, object->cell()->totalNumberOfReplicas()) : 1;
  std::vector<std::shared_ptr<SKAtomCopy>> copies;
  for (const std::shared_ptr<SKAtomTreeNode> &node : viewer->atomsTreeController()->flattenedLeafNodes())
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      for (const std::shared_ptr<SKAtomCopy> &copy : atom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          copies.push_back(copy);
        }
      }
    }
  }

  const int copyIndex = instanceTag / replicaCount;
  if (copyIndex < 0 || static_cast<size_t>(copyIndex) >= copies.size())
  {
    return std::nullopt;
  }

  AtomInstancePick pick;
  pick.copy = copies[static_cast<size_t>(copyIndex)];
  pick.asymmetricAtomIndex = static_cast<int>(pick.copy->asymmetricIndex());
  pick.replicaPosition = object->cell() ? object->cell()->replicaFromIndex(instanceTag) : int3(0, 0, 0);
  return pick;
}

namespace SKBondCellList
{
  namespace
  {
    enum class Mode
    {
      Cartesian,
      PeriodicCartesian,
      PeriodicFractional
    };

    constexpr int kNeighborOffsets[14][3] = {
      {0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0}, {-1, 1, 0},
      {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}, {-1, 1, 1},
      {-1, 0, 1}, {-1, -1, 1}, {0, -1, 1}, {1, -1, 1}
    };

    double covalentRadiusOfCopy(const std::shared_ptr<SKAtomCopy> &copy)
    {
      return PredefinedElements::predefinedElements[copy->parent()->elementIdentifier()]._covalentRadius;
    }

    bool occupancyPairAllowed(const std::shared_ptr<SKAtomCopy> &a, const std::shared_ptr<SKAtomCopy> &b)
    {
      const double occupancyA = a->parent()->occupancy();
      const double occupancyB = b->parent()->occupancy();
      return (occupancyA == 1.0 && occupancyB == 1.0) || (occupancyA < 1.0 && occupancyB < 1.0);
    }

    void markPeriodicDuplicate(const std::shared_ptr<SKAtomCopy> &a, const std::shared_ptr<SKAtomCopy> &b)
    {
      if (!(a->parent()->occupancy() < 1.0 || b->parent()->occupancy() < 1.0) || a->asymmetricIndex() == b->asymmetricIndex())
      {
        a->setType(SKAtomCopy::AtomCopyType::duplicate);
      }
    }

    void considerPair(std::vector<std::shared_ptr<SKBond>> &bonds,
                      const std::vector<std::shared_ptr<SKAtomCopy>> &copies,
                      const std::vector<double3> &cartesianPositions,
                      const std::vector<double> &radii,
                      SKCell *cell,
                      Mode mode,
                      size_t i,
                      size_t j)
    {
      const double bondCriteria = radii[i] + radii[j] + 0.56;

      if (mode == Mode::Cartesian)
      {
        if (!occupancyPairAllowed(copies[i], copies[j]))
        {
          return;
        }
        const double length = (cartesianPositions[i] - cartesianPositions[j]).length();
        if (length < bondCriteria)
        {
          if (length < 0.1)
          {
            copies[i]->setType(SKAtomCopy::AtomCopyType::duplicate);
          }
          bonds.push_back(std::make_shared<SKBond>(copies[i], copies[j]));
        }
        return;
      }

      const double3 separationVector = cartesianPositions[i] - cartesianPositions[j];
      const double3 periodicSeparationVector = cell->applyUnitCellBoundaryCondition(separationVector);
      const double bondLength = periodicSeparationVector.length();
      if (bondLength >= bondCriteria)
      {
        return;
      }
      if (bondLength < 0.1)
      {
        markPeriodicDuplicate(copies[i], copies[j]);
      }
      const SKBond::BoundaryType boundary = (separationVector.length() > bondCriteria)
                                                ? SKBond::BoundaryType::external
                                                : SKBond::BoundaryType::internal;
      bonds.push_back(std::make_shared<SKBond>(copies[i], copies[j], boundary));
    }

    void assignBruteForce(std::vector<std::shared_ptr<SKAtomCopy>> &copies,
                          const std::vector<double3> &cartesianPositions,
                          const std::vector<double> &radii,
                          SKCell *cell,
                          Mode mode,
                          std::vector<std::shared_ptr<SKBond>> &bonds)
    {
      const size_t count = copies.size();
      for (size_t i = 0; i < count; ++i)
      {
        for (size_t j = i + 1; j < count; ++j)
        {
          considerPair(bonds, copies, cartesianPositions, radii, cell, mode, i, j);
        }
      }
    }

    int wrapCellIndex(int index, int count)
    {
      return (index % count + count) % count;
    }

    int clampCellIndex(int index, int count)
    {
      if (index < 0)
      {
        return 0;
      }
      if (index >= count)
      {
        return count - 1;
      }
      return index;
    }

    void assignWithCellList(std::vector<std::shared_ptr<SKAtomCopy>> &copies,
                            SKBondSetController &bondSetController,
                            SKCell *cell,
                            Mode mode)
    {
      const size_t count = copies.size();
      std::vector<std::shared_ptr<SKBond>> bonds;
      if (count == 0)
      {
        bondSetController.setBonds(bonds);
        return;
      }

      std::vector<double3> cartesianPositions(count);
      std::vector<double> radii(count);
      double maxBondCriteria = 3.0;
      double3 boxMinimum(1e100, 1e100, 1e100);
      double3 boxMaximum(-1e100, -1e100, -1e100);

      for (size_t i = 0; i < count; ++i)
      {
        copies[i]->setType(SKAtomCopy::AtomCopyType::copy);
        radii[i] = covalentRadiusOfCopy(copies[i]);
        maxBondCriteria = std::max(maxBondCriteria, 2.0 * radii[i] + 0.56);
        if (mode == Mode::PeriodicFractional)
        {
          cartesianPositions[i] = cell->convertToCartesian(copies[i]->position());
        }
        else
        {
          cartesianPositions[i] = copies[i]->position();
        }
        boxMinimum = double3::min(boxMinimum, cartesianPositions[i]);
        boxMaximum = double3::max(boxMaximum, cartesianPositions[i]);
      }

      double3 widths;
      double3 binOrigin = boxMinimum;
      double cutoff = maxBondCriteria;
      double cutoffEpsilon = 0.0;

      if (mode == Mode::PeriodicFractional)
      {
        widths = cell->perpendicularWidths();
        binOrigin = double3(0.0, 0.0, 0.0);
        cutoffEpsilon = 1e-4;
      }
      else if (mode == Mode::PeriodicCartesian)
      {
        const SKBoundingBox cellBox = cell->boundingBox();
        const double3 cellWidths = cellBox.widths() + double3(0.1, 0.1, 0.1);
        if (cellWidths.x > 0.0001 && cellWidths.y > 0.0001 && cellWidths.z > 0.0001)
        {
          widths = cellWidths;
          binOrigin = cellBox.minimum();
        }
        else
        {
          widths = (boxMaximum - boxMinimum) + double3(0.1, 0.1, 0.1);
        }
      }
      else
      {
        widths = (boxMaximum - boxMinimum) + double3(0.1, 0.1, 0.1);
      }

      const int nx = static_cast<int>(widths.x / cutoff);
      const int ny = static_cast<int>(widths.y / cutoff);
      const int nz = static_cast<int>(widths.z / cutoff);

      if (nx < 3 || ny < 3 || nz < 3 || widths.x <= 0.0001 || widths.y <= 0.0001 || widths.z <= 0.0001)
      {
        assignBruteForce(copies, cartesianPositions, radii, cell, mode, bonds);
      }
      else
      {
        const size_t totalCells = static_cast<size_t>(nx) * static_cast<size_t>(ny) * static_cast<size_t>(nz);
        const double3 cutoffVector(cutoffEpsilon + widths.x / double(nx),
                                   cutoffEpsilon + widths.y / double(ny),
                                   cutoffEpsilon + widths.z / double(nz));

        std::vector<int> head(totalCells, -1);
        std::vector<int> list(count, -1);

        for (size_t i = 0; i < count; ++i)
        {
          double3 binPosition;
          if (mode == Mode::PeriodicFractional)
          {
            const double3 fractional = double3::fract(copies[i]->position());
            binPosition = double3(widths.x * fractional.x, widths.y * fractional.y, widths.z * fractional.z);
          }
          else
          {
            binPosition = cartesianPositions[i] - binOrigin;
          }

          const int ix = clampCellIndex(static_cast<int>(binPosition.x / cutoffVector.x), nx);
          const int iy = clampCellIndex(static_cast<int>(binPosition.y / cutoffVector.y), ny);
          const int iz = clampCellIndex(static_cast<int>(binPosition.z / cutoffVector.z), nz);
          const int icell = ix + iy * nx + iz * nx * ny;
          list[i] = head[static_cast<size_t>(icell)];
          head[static_cast<size_t>(icell)] = static_cast<int>(i);
        }

        for (int k1 = 0; k1 < nx; ++k1)
        {
          for (int k2 = 0; k2 < ny; ++k2)
          {
            for (int k3 = 0; k3 < nz; ++k3)
            {
              const int icellI = k1 + k2 * nx + k3 * nx * ny;
              for (int i = head[static_cast<size_t>(icellI)]; i >= 0; i = list[static_cast<size_t>(i)])
              {
                for (int offsetIndex = 0; offsetIndex < 14; ++offsetIndex)
                {
                  const int *offset = kNeighborOffsets[offsetIndex];
                  const int icellJ = wrapCellIndex(k1 + offset[0], nx)
                                     + wrapCellIndex(k2 + offset[1], ny) * nx
                                     + wrapCellIndex(k3 + offset[2], nz) * nx * ny;
                  for (int j = head[static_cast<size_t>(icellJ)]; j >= 0; j = list[static_cast<size_t>(j)])
                  {
                    if ((i < j) || (icellI != icellJ))
                    {
                      considerPair(bonds, copies, cartesianPositions, radii, cell, mode,
                                   static_cast<size_t>(i), static_cast<size_t>(j));
                    }
                  }
                }
              }
            }
          }
        }
      }

      std::vector<std::shared_ptr<SKBond>> filteredBonds;
      std::copy_if(bonds.begin(), bonds.end(), std::back_inserter(filteredBonds),
                   [](const std::shared_ptr<SKBond> &bond)
                   {
                     return bond->atom1()->type() == SKAtomCopy::AtomCopyType::copy
                            && bond->atom2()->type() == SKAtomCopy::AtomCopyType::copy;
                   });
      bondSetController.setBonds(filteredBonds);
    }
  }

  void assignCartesianBonds(std::vector<std::shared_ptr<SKAtomCopy>> &copies,
                            SKBondSetController &bondSetController)
  {
    assignWithCellList(copies, bondSetController, nullptr, Mode::Cartesian);
  }

  void assignPeriodicCartesianBonds(std::vector<std::shared_ptr<SKAtomCopy>> &copies,
                                    SKBondSetController &bondSetController,
                                    SKCell &cell)
  {
    assignWithCellList(copies, bondSetController, &cell, Mode::PeriodicCartesian);
  }

  void assignPeriodicFractionalBonds(std::vector<std::shared_ptr<SKAtomCopy>> &copies,
                                     SKBondSetController &bondSetController,
                                     SKCell &cell)
  {
    assignWithCellList(copies, bondSetController, &cell, Mode::PeriodicFractional);
  }
}

Structure::Structure(): _atomsTreeController(std::make_shared<SKAtomTreeController>()),
                        _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _legacySpaceGroup(1)
{
}

Structure::Structure(std::shared_ptr<SKAtomTreeController> atomTreeController): _atomsTreeController(atomTreeController),
                     _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _legacySpaceGroup(1)
{
}

std::shared_ptr<Object> Structure::shallowClone()
{
  return std::make_shared<Structure>(static_cast<const Structure&>(*this));
}

Structure::Structure(std::shared_ptr<SKStructure> frame): Object(frame), _atomsTreeController(std::make_shared<SKAtomTreeController>()),
  _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _legacySpaceGroup(1)
{
  for(std::shared_ptr<SKAsymmetricAtom> atom: frame->atoms)
  {
    std::shared_ptr<SKAtomTreeNode> node = std::make_shared<SKAtomTreeNode>(atom);
    _atomsTreeController->appendToRootnodes(node);
  }
}

Structure::Structure(const std::shared_ptr<Object> object): Object(object), _atomsTreeController(std::make_shared<SKAtomTreeController>()),
    _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _legacySpaceGroup(1)
{
  if (std::shared_ptr<AtomStructureViewer> atomStructureViewer = std::dynamic_pointer_cast<AtomStructureViewer>(object))
  {
    _atomRepresentationType = atomStructureViewer->atomRepresentationType();
    _atomRepresentationStyle = atomStructureViewer->atomRepresentationStyle();
    _atomEdgeCueing = atomStructureViewer->atomEdgeCueing();
    _atomColorSchemeIdentifier = atomStructureViewer->atomColorSchemeIdentifier();
    _atomColorSchemeOrder = atomStructureViewer->colorSchemeOrder();
    _atomForceFieldIdentifier = atomStructureViewer->atomForceFieldIdentifier();
    _atomForceFieldOrder = atomStructureViewer->forceFieldSchemeOrder();

    _drawAtoms = atomStructureViewer->drawAtoms();

    _atomHue = atomStructureViewer->atomHue();
    _atomSaturation = atomStructureViewer->atomSaturation();
    _atomValue = atomStructureViewer->atomValue();
    _atomScaleFactor = atomStructureViewer->atomScaleFactor();

    _atomAmbientOcclusion = atomStructureViewer->atomAmbientOcclusion();
    _atomHDR = atomStructureViewer->atomHDR();
    _atomHDRExposure = atomStructureViewer->atomHDRExposure();

    _atomAmbientColor = atomStructureViewer->atomAmbientColor();
    _atomDiffuseColor = atomStructureViewer->atomDiffuseColor();
    _atomSpecularColor = atomStructureViewer->atomSpecularColor();
    _atomAmbientIntensity = atomStructureViewer->atomAmbientIntensity();
    _atomDiffuseIntensity = atomStructureViewer->atomDiffuseIntensity();
    _atomSpecularIntensity = atomStructureViewer->atomSpecularIntensity();
    _atomShininess = atomStructureViewer->atomShininess();

    _atomSelectionStyle = atomStructureViewer->atomSelectionStyle();
    _atomSelectionIntensity = atomStructureViewer->atomSelectionIntensity();
    _atomSelectionScaling = atomStructureViewer->atomSelectionScaling();
  }

  if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(object))
  {
    _atomEdgeCueing = structure->_atomEdgeCueing;
    _atomForceFieldIdentifier = structure->_atomForceFieldIdentifier;
    _atomForceFieldOrder = structure->_atomForceFieldOrder;
    _atomColorSchemeIdentifier = structure->_atomColorSchemeIdentifier;
    _atomColorSchemeOrder = structure->_atomColorSchemeOrder;
    _atomSelectionStripesDensity = structure->_atomSelectionStripesDensity;
    _atomSelectionStripesFrequency = structure->_atomSelectionStripesFrequency;
    _atomSelectionWorleyNoise3DFrequency = structure->_atomSelectionWorleyNoise3DFrequency;
    _atomSelectionWorleyNoise3DJitter = structure->_atomSelectionWorleyNoise3DJitter;
    _bondSelectionStripesDensity = structure->_bondSelectionStripesDensity;
    _bondSelectionStripesFrequency = structure->_bondSelectionStripesFrequency;
    _bondSelectionWorleyNoise3DFrequency = structure->_bondSelectionWorleyNoise3DFrequency;
    _bondSelectionWorleyNoise3DJitter = structure->_bondSelectionWorleyNoise3DJitter;

    _blockingPockets = structure->_blockingPockets;
    _blockingPocketProbeMolecule = structure->_blockingPocketProbeMolecule;
    _blockingPocketProbeEpsilon = structure->_blockingPocketProbeEpsilon;
    _blockingPocketProbeSigma = structure->_blockingPocketProbeSigma;
    _drawBlockingPockets = structure->_drawBlockingPockets;
    _applyBlockingPockets = structure->_applyBlockingPockets;
    _blockingPocketsFrontSideHDR = structure->_blockingPocketsFrontSideHDR;
    _blockingPocketsFrontSideHDRExposure = structure->_blockingPocketsFrontSideHDRExposure;
    _blockingPocketsFrontSideAmbientColor = structure->_blockingPocketsFrontSideAmbientColor;
    _blockingPocketsFrontSideDiffuseColor = structure->_blockingPocketsFrontSideDiffuseColor;
    _blockingPocketsFrontSideSpecularColor = structure->_blockingPocketsFrontSideSpecularColor;
    _blockingPocketsFrontSideAmbientIntensity = structure->_blockingPocketsFrontSideAmbientIntensity;
    _blockingPocketsFrontSideDiffuseIntensity = structure->_blockingPocketsFrontSideDiffuseIntensity;
    _blockingPocketsFrontSideSpecularIntensity = structure->_blockingPocketsFrontSideSpecularIntensity;
    _blockingPocketsFrontSideShininess = structure->_blockingPocketsFrontSideShininess;
  }

  if (std::shared_ptr<BondStructureViewer> bondStructureViewer = std::dynamic_pointer_cast<BondStructureViewer>(object))
  {
    _drawBonds = bondStructureViewer->drawBonds();
    _bondScaleFactor = bondStructureViewer->bondScaleFactor();
    _bondColorMode = bondStructureViewer->bondColorMode();

    _bondAmbientOcclusion = bondStructureViewer->bondAmbientOcclusion();

    _bondHDR = bondStructureViewer->bondHDR();
    _bondHDRExposure = bondStructureViewer->bondHDRExposure();

    _bondHue = bondStructureViewer->bondHue();
    _bondSaturation = bondStructureViewer->bondSaturation();
    _bondValue = bondStructureViewer->bondValue();

    _bondAmbientColor = bondStructureViewer->bondAmbientColor();
    _bondDiffuseColor = bondStructureViewer->bondDiffuseColor();
    _bondSpecularColor = bondStructureViewer->bondSpecularColor();
    _bondAmbientIntensity = bondStructureViewer->bondAmbientIntensity();
    _bondDiffuseIntensity = bondStructureViewer->bondDiffuseIntensity();
    _bondSpecularIntensity = bondStructureViewer->bondSpecularIntensity();
    _bondShininess = bondStructureViewer->bondShininess();

    _bondSelectionStyle = bondStructureViewer->bondSelectionStyle();
    _bondSelectionIntensity = bondStructureViewer->bondSelectionIntensity();
    _bondSelectionScaling = bondStructureViewer->bondSelectionScaling();
  }

  if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(object))
  {
    QByteArray byteArray = QByteArray();
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream << atomViewer->atomsTreeController();

    QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
    streamRead >> _atomsTreeController;

    _atomsTreeController->setTags();
    _bondSetController->setTags();
  }

  if (std::shared_ptr<BondViewer> bondViewer = std::dynamic_pointer_cast<BondViewer>(object))
  {

  }

  if (std::shared_ptr<AnnotationViewer> annotationViewer = std::dynamic_pointer_cast<AnnotationViewer>(object))
  {
    _atomTextType = annotationViewer->renderTextType();
    _atomTextFont = annotationViewer->renderTextFont();
    _atomTextAlignment = annotationViewer->renderTextAlignment();
    _atomTextStyle = annotationViewer->renderTextStyle();
    _atomTextColor = annotationViewer->renderTextColor();
    _atomTextScaling = annotationViewer->renderTextScaling();
    _atomTextOffset = annotationViewer->renderTextOffset();
  }

  if (std::shared_ptr<InfoViewer> infoViewer = std::dynamic_pointer_cast<InfoViewer>(object))
  {
    _creationTemperature = infoViewer->creationTemperature();
    _creationTemperatureScale = infoViewer->creationTemperatureScale();
    _creationPressure = infoViewer->creationPressure();
    _creationPressureScale = infoViewer->creationPressureScale();
    _creationMethod = infoViewer->creationMethod();
    _creationUnitCellRelaxationMethod = infoViewer->creationUnitCellRelaxationMethod();
    _creationAtomicPositionsSoftwarePackage = infoViewer->creationAtomicPositionsSoftwarePackage();
    _creationAtomicPositionsIonsRelaxationAlgorithm = infoViewer->creationAtomicPositionsIonsRelaxationAlgorithm();
    _creationAtomicPositionsIonsRelaxationCheck = infoViewer->creationAtomicPositionsIonsRelaxationCheck();
    _creationAtomicPositionsForcefield = infoViewer->creationAtomicPositionsForcefield();
    _creationAtomicPositionsForcefieldDetails = infoViewer->creationAtomicPositionsForcefieldDetails();
    _creationAtomicChargesSoftwarePackage = infoViewer->creationAtomicChargesSoftwarePackage();
    _creationAtomicChargesAlgorithms = infoViewer->creationAtomicChargesAlgorithms();
    _creationAtomicChargesForcefield = infoViewer->creationAtomicChargesForcefield();
    _creationAtomicChargesForcefieldDetails = infoViewer->creationAtomicChargesForcefieldDetails();

    _experimentalMeasurementRadiation = infoViewer->experimentalMeasurementRadiation();
    _experimentalMeasurementWaveLength = infoViewer->experimentalMeasurementWaveLength();
    _experimentalMeasurementThetaMin = infoViewer->experimentalMeasurementThetaMin();
    _experimentalMeasurementThetaMax = infoViewer->experimentalMeasurementThetaMax();
    _experimentalMeasurementIndexLimitsHmin = infoViewer->experimentalMeasurementIndexLimitsHmin();
    _experimentalMeasurementIndexLimitsHmax = infoViewer->experimentalMeasurementIndexLimitsHmax();
    _experimentalMeasurementIndexLimitsKmin = infoViewer->experimentalMeasurementIndexLimitsKmin();
    _experimentalMeasurementIndexLimitsKmax = infoViewer->experimentalMeasurementIndexLimitsKmax();
    _experimentalMeasurementIndexLimitsLmin = infoViewer->experimentalMeasurementIndexLimitsLmin();
    _experimentalMeasurementIndexLimitsLmax = infoViewer->experimentalMeasurementIndexLimitsLmax();
    _experimentalMeasurementNumberOfSymmetryIndependentReflections = infoViewer->experimentalMeasurementNumberOfSymmetryIndependentReflections();
    _experimentalMeasurementSoftware = infoViewer->experimentalMeasurementSoftware();
    _experimentalMeasurementRefinementDetails = infoViewer->experimentalMeasurementRefinementDetails();
    _experimentalMeasurementGoodnessOfFit = infoViewer->experimentalMeasurementGoodnessOfFit();
    _experimentalMeasurementRFactorGt = infoViewer->experimentalMeasurementRFactorGt();
    _experimentalMeasurementRFactorAll = infoViewer->experimentalMeasurementRFactorAll();

    _chemicalFormulaMoiety = infoViewer->chemicalFormulaMoiety();
    _chemicalFormulaSum = infoViewer->chemicalFormulaSum();
    _chemicalNameSystematic = infoViewer->chemicalNameSystematic();

    _citationArticleTitle = infoViewer->citationArticleTitle();
    _citationJournalTitle = infoViewer->citationJournalTitle();
    _citationAuthors = infoViewer->citationAuthors();
    _citationJournalVolume = infoViewer->citationJournalVolume();
    _citationJournalNumber = infoViewer->citationJournalNumber();
    _citationJournalPageNumbers = infoViewer->citationJournalPageNumbers();
    _citationDOI = infoViewer->citationDOI();
    _citationPublicationDate = infoViewer->citationPublicationDate();
    _citationDatebaseCodes = infoViewer->citationDatebaseCodes();
  }
}

// shallow copy, atoms/bonds are empty, spacegroup no symmetry
Structure::Structure(const Structure &structure): Object(structure),  _atomsTreeController(std::make_shared<SKAtomTreeController>()),
  _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _legacySpaceGroup(1)
{
  qDebug() << "Base-class Copy constructor";

  _minimumGridEnergyValue = structure._minimumGridEnergyValue;

  _selectionCOMTranslation = structure._selectionCOMTranslation;
  _selectionRotationIndex = structure._selectionRotationIndex;
  _selectionBodyFixedBasis = structure._selectionBodyFixedBasis;

  _structureType = structure._structureType;
  _structureMaterialType = structure._structureMaterialType;
  _structureMass = structure._structureMass;
  _structureDensity = structure._structureDensity;
  _structureHeliumVoidFraction = structure._structureHeliumVoidFraction;
  _structureSpecificVolume = structure._structureSpecificVolume;
  _structureAccessiblePoreVolume = structure._structureAccessiblePoreVolume;
  _structureVolumetricNitrogenSurfaceArea = structure._structureVolumetricNitrogenSurfaceArea;
  _structureGravimetricNitrogenSurfaceArea = structure._structureGravimetricNitrogenSurfaceArea;
  _structureVolumetricWellSurfaceArea = structure._structureVolumetricWellSurfaceArea;
  _structureGravimetricWellSurfaceArea = structure._structureGravimetricWellSurfaceArea;
  _structureVolumetricGeometricSurfaceArea = structure._structureVolumetricGeometricSurfaceArea;
  _structureGravimetricGeometricSurfaceArea = structure._structureGravimetricGeometricSurfaceArea;
  _structureVolumetricVanDerWaalsGeometricSurfaceArea = structure._structureVolumetricVanDerWaalsGeometricSurfaceArea;
  _structureGravimetricVanDerWaalsGeometricSurfaceArea = structure._structureGravimetricVanDerWaalsGeometricSurfaceArea;
  _structureNumberOfChannelSystems = structure._structureNumberOfChannelSystems;
  _structureNumberOfInaccessiblePockets = structure._structureNumberOfInaccessiblePockets;
  _structureDimensionalityOfPoreSystem = structure._structureDimensionalityOfPoreSystem;
  _structureLargestCavityDiameter = structure._structureLargestCavityDiameter;
  _structureRestrictingPoreLimitingDiameter = structure._structureRestrictingPoreLimitingDiameter;
  _structureLargestCavityDiameterAlongAViablePath = structure._structureLargestCavityDiameterAlongAViablePath;
  _structureRestrictingPoreLimitingDiameter2 = structure._structureRestrictingPoreLimitingDiameter2;
  _structureLargestCavityDiameterAlongAViablePath2 = structure._structureLargestCavityDiameterAlongAViablePath2;
  _structureWindow1Size = structure._structureWindow1Size;
  _structureWindow2Size = structure._structureWindow2Size;
  _structureWindow3Size = structure._structureWindow3Size;
  _structureWindow1RingAtoms = structure._structureWindow1RingAtoms;
  _structureWindow2RingAtoms = structure._structureWindow2RingAtoms;
  _structureWindow3RingAtoms = structure._structureWindow3RingAtoms;

  _blockingPockets = structure._blockingPockets;
  _blockingPocketProbeMolecule = structure._blockingPocketProbeMolecule;
  _blockingPocketProbeEpsilon = structure._blockingPocketProbeEpsilon;
  _blockingPocketProbeSigma = structure._blockingPocketProbeSigma;
  _drawBlockingPockets = structure._drawBlockingPockets;
  _applyBlockingPockets = structure._applyBlockingPockets;
  _blockingPocketsFrontSideHDR = structure._blockingPocketsFrontSideHDR;
  _blockingPocketsFrontSideHDRExposure = structure._blockingPocketsFrontSideHDRExposure;
  _blockingPocketsFrontSideAmbientColor = structure._blockingPocketsFrontSideAmbientColor;
  _blockingPocketsFrontSideDiffuseColor = structure._blockingPocketsFrontSideDiffuseColor;
  _blockingPocketsFrontSideSpecularColor = structure._blockingPocketsFrontSideSpecularColor;
  _blockingPocketsFrontSideAmbientIntensity = structure._blockingPocketsFrontSideAmbientIntensity;
  _blockingPocketsFrontSideDiffuseIntensity = structure._blockingPocketsFrontSideDiffuseIntensity;
  _blockingPocketsFrontSideSpecularIntensity = structure._blockingPocketsFrontSideSpecularIntensity;
  _blockingPocketsFrontSideShininess = structure._blockingPocketsFrontSideShininess;

  _authorFirstName = structure._authorFirstName;
  _authorMiddleName = structure._authorMiddleName;
  _authorLastName = structure._authorLastName;
  _authorOrchidID = structure._authorOrchidID;
  _authorResearcherID = structure._authorResearcherID;
  _authorAffiliationUniversityName = structure._authorAffiliationUniversityName;
  _authorAffiliationFacultyName = structure._authorAffiliationFacultyName;
  _authorAffiliationInstituteName = structure._authorAffiliationInstituteName;
  _authorAffiliationCityName = structure._authorAffiliationCityName;
  _authorAffiliationCountryName = structure._authorAffiliationCountryName;

  // primitive properties
  _primitiveTransformationMatrix = structure._primitiveTransformationMatrix;
  _primitiveOrientation = structure._primitiveOrientation;
  _primitiveRotationDelta = structure._primitiveRotationDelta;

  _primitiveOpacity = structure._primitiveOpacity;
  _primitiveIsCapped = structure._primitiveIsCapped;
  _primitiveIsFractional = structure._primitiveIsFractional;
  _primitiveNumberOfSides = structure._primitiveNumberOfSides;
  _primitiveThickness = structure._primitiveThickness;

  _primitiveFrontSideHDR = structure._primitiveFrontSideHDR;
  _primitiveFrontSideHDRExposure = structure._primitiveFrontSideHDRExposure;
  _primitiveFrontSideAmbientColor = structure._primitiveFrontSideAmbientColor;
  _primitiveFrontSideDiffuseColor = structure._primitiveFrontSideDiffuseColor;
  _primitiveFrontSideSpecularColor = structure._primitiveFrontSideSpecularColor;
  _primitiveFrontSideAmbientIntensity = structure._primitiveFrontSideAmbientIntensity;
  _primitiveFrontSideDiffuseIntensity = structure._primitiveFrontSideDiffuseIntensity;
  _primitiveFrontSideSpecularIntensity = structure._primitiveFrontSideSpecularIntensity;
  _primitiveFrontSideShininess = structure._primitiveFrontSideShininess;

  _primitiveBackSideHDR = structure._primitiveBackSideHDR;
  _primitiveBackSideHDRExposure = structure._primitiveBackSideHDRExposure;
  _primitiveBackSideAmbientColor = structure._primitiveBackSideAmbientColor;
  _primitiveBackSideDiffuseColor = structure._primitiveBackSideDiffuseColor;
  _primitiveBackSideSpecularColor = structure._primitiveBackSideSpecularColor;
  _primitiveBackSideAmbientIntensity = structure._primitiveBackSideAmbientIntensity;
  _primitiveBackSideDiffuseIntensity = structure._primitiveBackSideDiffuseIntensity;
  _primitiveBackSideSpecularIntensity = structure._primitiveBackSideSpecularIntensity;
  _primitiveBackSideShininess = structure._primitiveBackSideShininess;

  // atoms
  _drawAtoms = structure._drawAtoms;

  _atomRepresentationType = structure._atomRepresentationType;
  _atomRepresentationStyle = structure._atomRepresentationStyle;
  _atomEdgeCueing = structure._atomEdgeCueing;
  _atomForceFieldIdentifier = structure._atomForceFieldIdentifier;
  _atomForceFieldOrder = structure._atomForceFieldOrder;
  _atomColorSchemeIdentifier = structure._atomColorSchemeIdentifier;
  _atomColorSchemeOrder = structure._atomColorSchemeOrder;

  _atomSelectionStyle = structure._atomSelectionStyle;
  _atomSelectionStripesDensity = structure._atomSelectionStripesDensity;
  _atomSelectionStripesFrequency = structure._atomSelectionStripesFrequency;
  _atomSelectionWorleyNoise3DFrequency = structure._atomSelectionWorleyNoise3DFrequency;
  _atomSelectionWorleyNoise3DJitter = structure._atomSelectionWorleyNoise3DJitter;
  _atomSelectionScaling = structure._atomSelectionScaling;
  _atomSelectionIntensity = structure._atomSelectionIntensity;

  _atomHue = structure._atomHue;
  _atomSaturation = structure._atomSaturation;
  _atomValue = structure._atomValue;
  _atomScaleFactor = structure._atomScaleFactor;

  _atomAmbientOcclusion = structure._atomAmbientOcclusion;
  _atomAmbientOcclusionPatchNumber = structure._atomAmbientOcclusionPatchNumber;
  _atomAmbientOcclusionTextureSize = structure._atomAmbientOcclusionTextureSize;
  _atomAmbientOcclusionPatchSize = structure._atomAmbientOcclusionPatchSize;
  //_atomCacheAmbientOcclusionTexture = clone->_atomCacheAmbientOcclusionTexture;

  _atomHDR = structure._atomHDR;
  _atomHDRExposure = structure._atomHDRExposure;
  _atomSelectionIntensity = structure._atomSelectionIntensity;

  _atomAmbientColor = structure._atomAmbientColor;
  _atomDiffuseColor = structure._atomDiffuseColor;
  _atomSpecularColor = structure._atomSpecularColor;
  _atomAmbientIntensity = structure._atomAmbientIntensity;
  _atomDiffuseIntensity = structure._atomDiffuseIntensity;
  _atomSpecularIntensity = structure._atomSpecularIntensity;
  _atomShininess = structure._atomShininess;

  // bonds
  _drawBonds = structure._drawBonds;

  _bondScaleFactor = structure._bondScaleFactor;
  _bondColorMode = structure._bondColorMode;

  _bondAmbientColor = structure._bondAmbientColor;
  _bondDiffuseColor = structure._bondDiffuseColor;
  _bondSpecularColor = structure._bondSpecularColor;
  _bondAmbientIntensity = structure._bondAmbientIntensity;
  _bondDiffuseIntensity = structure._bondDiffuseIntensity;
  _bondSpecularIntensity = structure._bondSpecularIntensity;
  _bondShininess = structure._bondShininess;

  _bondHDR = structure._bondHDR;
  _bondHDRExposure = structure._bondHDRExposure;
  _bondSelectionStyle = structure._bondSelectionStyle;
  _bondSelectionStripesDensity = structure._bondSelectionStripesDensity;
  _bondSelectionStripesFrequency = structure._bondSelectionStripesFrequency;
  _bondSelectionWorleyNoise3DFrequency = structure._bondSelectionWorleyNoise3DFrequency;
  _bondSelectionWorleyNoise3DJitter = structure._bondSelectionWorleyNoise3DJitter;
  _bondSelectionScaling = structure._bondSelectionScaling;
  _bondSelectionIntensity = structure._bondSelectionIntensity;

  _bondHue = structure._bondHue;
  _bondSaturation = structure._bondSaturation;
  _bondValue = structure._bondValue;

  _bondAmbientOcclusion = structure._bondAmbientOcclusion;

  // text properties
  _atomTextType = structure._atomTextType;
  _atomTextFont = structure._atomTextFont;
  _atomTextScaling = structure._atomTextScaling;
  _atomTextColor = structure._atomTextColor;
  _atomTextGlowColor = structure._atomTextGlowColor;
  _atomTextStyle = structure._atomTextStyle;
  _atomTextEffect = structure._atomTextEffect;
  _atomTextAlignment = structure._atomTextAlignment;
  _atomTextOffset = structure._atomTextOffset;

  // adsorption surface
  _frameworkProbeMolecule = structure._frameworkProbeMolecule;
  _frameworkProbeEpsilon = structure._frameworkProbeEpsilon;
  _frameworkProbeSigma = structure._frameworkProbeSigma;

  _drawAdsorptionSurface = structure._drawAdsorptionSurface;
  _adsorptionSurfaceOpacity = structure._adsorptionSurfaceOpacity;
  _adsorptionTransparencyThreshold = structure._adsorptionTransparencyThreshold;
  _adsorptionSurfaceIsoValue = structure._adsorptionSurfaceIsoValue;
  _encompassingPowerOfTwoCubicGridSize = structure._encompassingPowerOfTwoCubicGridSize;

  _adsorptionSurfaceRenderingMethod = structure._adsorptionSurfaceRenderingMethod;
  _adsorptionVolumeTransferFunction = structure._adsorptionVolumeTransferFunction;
  _adsorptionVolumeStepLength = structure._adsorptionVolumeStepLength;

  _adsorptionSurfaceSize = structure._adsorptionSurfaceSize;
  _adsorptionSurfaceNumberOfTriangles = structure._adsorptionSurfaceNumberOfTriangles;

  _adsorptionSurfaceProbeMolecule = structure._adsorptionSurfaceProbeMolecule;
  _adsorptionSurfaceProbeEpsilon = structure._adsorptionSurfaceProbeEpsilon;
  _adsorptionSurfaceProbeSigma = structure._adsorptionSurfaceProbeSigma;

  _adsorptionSurfaceHue = structure._adsorptionSurfaceHue;
  _adsorptionSurfaceSaturation = structure._adsorptionSurfaceSaturation;
  _adsorptionSurfaceValue = structure._adsorptionSurfaceValue;

  _adsorptionSurfaceFrontSideHDR = structure._adsorptionSurfaceFrontSideHDR;
  _adsorptionSurfaceFrontSideHDRExposure = structure._adsorptionSurfaceFrontSideHDRExposure;
  _adsorptionSurfaceFrontSideAmbientColor = structure._adsorptionSurfaceFrontSideAmbientColor;
  _adsorptionSurfaceFrontSideDiffuseColor = structure._adsorptionSurfaceFrontSideDiffuseColor;
  _adsorptionSurfaceFrontSideSpecularColor = structure._adsorptionSurfaceFrontSideSpecularColor;
  _adsorptionSurfaceFrontSideDiffuseIntensity = structure._adsorptionSurfaceFrontSideDiffuseIntensity;
  _adsorptionSurfaceFrontSideAmbientIntensity = structure._adsorptionSurfaceFrontSideAmbientIntensity;
  _adsorptionSurfaceFrontSideSpecularIntensity = structure._adsorptionSurfaceFrontSideSpecularIntensity;
  _adsorptionSurfaceFrontSideShininess = structure._adsorptionSurfaceFrontSideShininess;

  _adsorptionSurfaceBackSideHDR = structure._adsorptionSurfaceBackSideHDR;
  _adsorptionSurfaceBackSideHDRExposure = structure._adsorptionSurfaceBackSideHDRExposure;
  _adsorptionSurfaceBackSideAmbientColor = structure._adsorptionSurfaceBackSideAmbientColor;
  _adsorptionSurfaceBackSideDiffuseColor = structure._adsorptionSurfaceBackSideDiffuseColor;
  _adsorptionSurfaceBackSideSpecularColor = structure._adsorptionSurfaceBackSideSpecularColor;
  _adsorptionSurfaceBackSideDiffuseIntensity = structure._adsorptionSurfaceBackSideDiffuseIntensity;
  _adsorptionSurfaceBackSideAmbientIntensity = structure._adsorptionSurfaceBackSideAmbientIntensity;
  _adsorptionSurfaceBackSideSpecularIntensity = structure._adsorptionSurfaceBackSideSpecularIntensity;
  _adsorptionSurfaceBackSideShininess = structure._adsorptionSurfaceBackSideShininess;


  _creationDate = structure._creationDate;
  _creationTemperature = structure._creationTemperature;
  _creationTemperatureScale = structure._creationTemperatureScale;
  _creationPressure = structure._creationPressure;
  _creationPressureScale = structure._creationPressureScale;
  _creationMethod = structure._creationMethod;
  _creationUnitCellRelaxationMethod = structure._creationUnitCellRelaxationMethod;
  _creationAtomicPositionsSoftwarePackage = structure._creationAtomicPositionsSoftwarePackage;
  _creationAtomicPositionsIonsRelaxationAlgorithm = structure._creationAtomicPositionsIonsRelaxationAlgorithm;
  _creationAtomicPositionsIonsRelaxationCheck = structure._creationAtomicPositionsIonsRelaxationCheck;
  _creationAtomicPositionsForcefield = structure._creationAtomicPositionsForcefield;
  _creationAtomicPositionsForcefieldDetails = structure._creationAtomicPositionsForcefieldDetails;
  _creationAtomicChargesSoftwarePackage = structure._creationAtomicChargesSoftwarePackage;
  _creationAtomicChargesAlgorithms = structure._creationAtomicChargesAlgorithms;
  _creationAtomicChargesForcefield = structure._creationAtomicChargesForcefield;
  _creationAtomicChargesForcefieldDetails = structure._creationAtomicChargesForcefieldDetails;

  _chemicalFormulaMoiety = structure._chemicalFormulaMoiety;
  _chemicalFormulaSum = structure._chemicalFormulaSum;
  _chemicalNameSystematic = structure._chemicalNameSystematic;
  _cellFormulaUnitsZ = structure._cellFormulaUnitsZ;


  _citationArticleTitle = structure._citationArticleTitle;
  _citationJournalTitle = structure._citationJournalTitle;
  _citationAuthors = structure._citationAuthors;
  _citationJournalVolume = structure._citationJournalVolume;
  _citationJournalNumber = structure._citationJournalNumber;
  _citationJournalPageNumbers = structure._citationJournalPageNumbers;
  _citationDOI = structure._citationDOI;
  _citationPublicationDate = structure._citationPublicationDate;
  _citationDatebaseCodes = structure._citationDatebaseCodes;

  _experimentalMeasurementRadiation = structure._experimentalMeasurementRadiation;
  _experimentalMeasurementWaveLength = structure._experimentalMeasurementWaveLength;
  _experimentalMeasurementThetaMin = structure._experimentalMeasurementThetaMin;
  _experimentalMeasurementThetaMax = structure._experimentalMeasurementThetaMax;
  _experimentalMeasurementIndexLimitsHmin = structure._experimentalMeasurementIndexLimitsHmin;
  _experimentalMeasurementIndexLimitsHmax = structure._experimentalMeasurementIndexLimitsHmax;
  _experimentalMeasurementIndexLimitsKmin = structure._experimentalMeasurementIndexLimitsKmin;
  _experimentalMeasurementIndexLimitsKmax = structure._experimentalMeasurementIndexLimitsKmax;
  _experimentalMeasurementIndexLimitsLmin = structure._experimentalMeasurementIndexLimitsLmin;
  _experimentalMeasurementIndexLimitsLmax = structure._experimentalMeasurementIndexLimitsLmax;
  _experimentalMeasurementNumberOfSymmetryIndependentReflections = structure._experimentalMeasurementNumberOfSymmetryIndependentReflections;
  _experimentalMeasurementSoftware = structure._experimentalMeasurementSoftware;
  _experimentalMeasurementRefinementDetails = structure._experimentalMeasurementRefinementDetails;
  _experimentalMeasurementGoodnessOfFit = structure._experimentalMeasurementGoodnessOfFit;
  _experimentalMeasurementRFactorGt = structure._experimentalMeasurementRFactorGt;
  _experimentalMeasurementRFactorAll = structure._experimentalMeasurementRFactorAll;
}



void Structure::convertAsymmetricAtomsToCartesian()
{
  double3x3 unitCell = _cell->unitCell();
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomTreeNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &atomTreeNode: atomTreeNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = atomTreeNode->representedObject())
    {
      double3 pos = atom->position();
      atom->setPosition(unitCell * pos);
    }
  }
}

void Structure::convertAsymmetricAtomsToFractional()
{
  double3x3 inverseUnitCell = _cell->inverseUnitCell();
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomTreeNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &atomTreeNode: atomTreeNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = atomTreeNode->representedObject())
    {
      double3 pos = atom->position();
      atom->setPosition(inverseUnitCell * pos);
    }
  }
}


bool Structure::hasSelectedAtoms() const
{
  return !_atomsTreeController->selectionIndexPathSet().empty();
}

// MARK: Rendering
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> Structure::renderAtoms() const
{
  return std::vector<RKInPerInstanceAttributesAtoms>();
}

std::vector<RKInPerInstanceAttributesBonds> Structure::renderInternalBonds() const
{
  return std::vector<RKInPerInstanceAttributesBonds>();
}

std::vector<RKInPerInstanceAttributesBonds> Structure::renderExternalBonds() const
{
  return std::vector<RKInPerInstanceAttributesBonds>();
}

std::vector<RKInPerInstanceAttributesAtoms> Structure::renderUnitCellSpheres() const
{
  return std::vector<RKInPerInstanceAttributesAtoms>();
}

std::vector<RKInPerInstanceAttributesBonds> Structure::renderUnitCellCylinders() const
{
  return std::vector<RKInPerInstanceAttributesBonds>();
}

// MARK: Rendering selection
// =====================================================================

std::vector<RKInPerInstanceAttributesAtoms> Structure::renderSelectedAtoms() const
{
  return std::vector<RKInPerInstanceAttributesAtoms>();
}

std::vector<RKInPerInstanceAttributesBonds> Structure::renderSelectedInternalBonds() const
{
  return std::vector<RKInPerInstanceAttributesBonds>();
}

std::vector<RKInPerInstanceAttributesBonds> Structure::renderSelectedExternalBonds() const
{
  return std::vector<RKInPerInstanceAttributesBonds>();
}


// MARK: Filtering
// =====================================================================

std::set<int> Structure::filterCartesianAtomPositions(std::function<bool(double3)> &)
{
  return std::set<int>();
};

BondSelectionIndexSet Structure::filterCartesianBondPositions(std::function<bool(double3)> &)
{
  return BondSelectionIndexSet();
};


// MARK: Bounding box
// =====================================================================

SKBoundingBox Structure::boundingBox() const
{
  return SKBoundingBox();
}

SKBoundingBox Structure::transformedBoundingBox() const
{
  SKBoundingBox currentBoundingBox = this->_cell->boundingBox();

  double4x4 rotationMatrix = double4x4::AffinityMatrixToTransformationAroundArbitraryPoint(double4x4(_orientation), currentBoundingBox.center());
  SKBoundingBox transformedBoundingBox = currentBoundingBox.adjustForTransformation(rotationMatrix);

  return transformedBoundingBox;
}

void Structure::reComputeBoundingBox()
{
}

// MARK: Symmetry
// =====================================================================

void Structure::expandSymmetry()
{

}

double3x3 Structure::matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
{
  Q_UNUSED(atoms);
  return double3x3();
}

double3 Structure::centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
{
  Q_UNUSED(atoms);
  return double3();
}

void Structure::recomputeSelectionBodyFixedBasis()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = _atomsTreeController->selectedObjects();
  _selectionCOMTranslation = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 inertia = matrixOfInertia(atoms);

  qDebug() << "inertia";
  qDebug() << inertia.ax << ", " << inertia.bx << ", " << inertia.cx;
  qDebug() << inertia.ay << ", " << inertia.by << ", " << inertia.cy;
  qDebug() << inertia.az << ", " << inertia.bz << ", " << inertia.cz;

  double3x3 eigenvectors = inertia;
  double3 eigenvalues{};
  inertia.EigenSystemSymmetric(eigenvalues, eigenvectors);
  _selectionBodyFixedBasis = eigenvectors;

  qDebug() << "_selectionBodyFixedBasis";
  qDebug() << _selectionBodyFixedBasis.ax << ", " << _selectionBodyFixedBasis.bx << ", " << _selectionBodyFixedBasis.cx;
  qDebug() << _selectionBodyFixedBasis.ay << ", " << _selectionBodyFixedBasis.by << ", " << _selectionBodyFixedBasis.cy;
  qDebug() << _selectionBodyFixedBasis.az << ", " << _selectionBodyFixedBasis.bz << ", " << _selectionBodyFixedBasis.cz;
  qDebug() << "eigenvalues:";
  qDebug() << eigenvalues.x << ", " << eigenvalues.y << ", " << eigenvalues.z;

}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> Structure::cellForFractionalPositions()
{
  return std::nullopt;
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> Structure::cellForCartesianPositions()
{
  return std::nullopt;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Structure::asymmetricAtomsCopiedAndTransformedToCartesianPositions()
{
  return {};
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Structure::asymmetricAtomsCopiedAndTransformedToFractionalPositions()
{
  return {};
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Structure::atomsCopiedAndTransformedToCartesianPositions()
{
  return {};
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Structure::atomsCopiedAndTransformedToFractionalPositions()
{
  return {};
}

// MARK: Text
// =====================================================================

std::vector<RKInPerInstanceAttributesText> Structure::atomTextData(RKFontAtlas *fontAtlas) const
{
  Q_UNUSED(fontAtlas);
  return {};
}


std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Structure::translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(translation);
  return {};
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Structure::translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(translation);
  return {};
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Structure::rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(rotation);
  return {};
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Structure::rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(rotation);
  return {};
}


// MARK: bond-computations
// =====================================================================

double Structure::bondLength(std::shared_ptr<SKBond> bond) const
{
  Q_UNUSED(bond);
  return 0.0;
}

double3 Structure::bondVector(std::shared_ptr<SKBond> bond) const
{
  Q_UNUSED(bond);
  return double3();
}

std::pair<double3, double3> Structure::computeChangedBondLength(std::shared_ptr<SKBond>, double) const
{
  return std::pair<double3, double3>();
}

bool Structure::colorAtomsWithBondColor() const
{
  return _atomRepresentationType == RepresentationType::unity && _bondColorMode == RKBondColorMode::uniform;
}


void Structure::setAtomScaleFactor(double size)
{
  _atomScaleFactor = size;
}

void Structure::setBondScaleFactor(double value)
{
  _bondScaleFactor = value;
  if(_atomRepresentationType == RepresentationType::unity)
  {
    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
    {
      if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
      {
        atom->setDrawRadius(value);
      }
    }
  }
}

double2 Structure::adsorptionSurfaceProbeParameters() const
{
  return double2(_adsorptionSurfaceProbeEpsilon, _adsorptionSurfaceProbeSigma);
}

void Structure::setAdsorptionSurfaceProbeMolecule(ProbeMolecule value)
{
  _adsorptionSurfaceProbeMolecule = value;
  if (std::optional<double2> params = probeNamedParameters(value))
  {
    _adsorptionSurfaceProbeEpsilon = params->x;
    _adsorptionSurfaceProbeSigma = params->y;
  }
}

void Structure::setAdsorptionSurfaceProbeEpsilon(double value)
{
  _adsorptionSurfaceProbeEpsilon = value;
  _adsorptionSurfaceProbeMolecule = matchingProbeMolecule(double2(_adsorptionSurfaceProbeEpsilon, _adsorptionSurfaceProbeSigma), true);
}

void Structure::setAdsorptionSurfaceProbeSigma(double value)
{
  _adsorptionSurfaceProbeSigma = value;
  _adsorptionSurfaceProbeMolecule = matchingProbeMolecule(double2(_adsorptionSurfaceProbeEpsilon, _adsorptionSurfaceProbeSigma), true);
}

double2 Structure::frameworkProbeParameters() const
{
  return double2(_frameworkProbeEpsilon, _frameworkProbeSigma);
}

void Structure::setFrameworkProbeMolecule(ProbeMolecule value)
{
  _frameworkProbeMolecule = value;
  if (std::optional<double2> params = probeNamedParameters(value))
  {
    _frameworkProbeEpsilon = params->x;
    _frameworkProbeSigma = params->y;
  }
}

void Structure::setFrameworkProbeEpsilon(double value)
{
  _frameworkProbeEpsilon = value;
  _frameworkProbeMolecule = matchingProbeMolecule(double2(_frameworkProbeEpsilon, _frameworkProbeSigma), false);
}

void Structure::setFrameworkProbeSigma(double value)
{
  _frameworkProbeSigma = value;
  _frameworkProbeMolecule = matchingProbeMolecule(double2(_frameworkProbeEpsilon, _frameworkProbeSigma), false);
}

void Structure::setBlockingPocketProbeMolecule(ProbeMolecule value)
{
  _blockingPocketProbeMolecule = value;
  if (std::optional<double2> params = probeNamedParameters(value))
  {
    _blockingPocketProbeEpsilon = params->x;
    _blockingPocketProbeSigma = params->y;
  }
}

void Structure::setBlockingPocketProbeEpsilon(double value)
{
  _blockingPocketProbeEpsilon = value;
  _blockingPocketProbeMolecule = matchingProbeMolecule(double2(_blockingPocketProbeEpsilon, _blockingPocketProbeSigma), true);
}

void Structure::setBlockingPocketProbeSigma(double value)
{
  _blockingPocketProbeSigma = value;
  _blockingPocketProbeMolecule = matchingProbeMolecule(double2(_blockingPocketProbeEpsilon, _blockingPocketProbeSigma), true);
}

std::vector<int> Structure::atomUnitCellElementIdentifiers() const
{
  std::vector<int> result;
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (const std::shared_ptr<SKAsymmetricAtom> &atom = node->representedObject())
    {
      for (const std::shared_ptr<SKAtomCopy> &copy : atom->copies())
      {
        if (copy->type() == SKAtomCopy::AtomCopyType::copy)
        {
          result.push_back(atom->elementIdentifier());
        }
      }
    }
  }
  return result;
}

std::vector<float4> Structure::constructWellSurfaceTriangles(const std::vector<float> &wellData) const
{
  int3 dims = dimensions();
  std::vector<float4> vertices = SKComputeWellSurface::constructWellSurfaceVertexBuffer(wellData, _adsorptionSurfaceIsoValue, dims);
  if (!vertices.empty() && _cell)
  {
    float iso = SKComputeWellSurface::effectiveTrimIsovalue(wellData, _adsorptionSurfaceIsoValue, dims);
    SKComputeWellSurface::refineWellSurfaceVertices(vertices,
                                                    adsorptionSurfaceProbeParameters(),
                                                    atomUnitCellPositions(),
                                                    potentialParameters(),
                                                    _cell->unitCell(),
                                                    _cell->minimumReplicas(),
                                                    iso,
                                                    appliedBlockingPockets());
  }
  return vertices;
}

std::vector<float4> Structure::constructWellFilamentTriangles(const std::vector<float> &wellData) const
{
  if (!_cell) return {};
  return SKComputeWellSurface::constructWellFilamentVertexBuffer(wellData, _adsorptionSurfaceIsoValue, dimensions(), _cell->unitCell());
}

namespace
{
struct SubdividedSphereMesh
{
  std::vector<double3> vertices;
  std::vector<int3> triangles;
};

const SubdividedSphereMesh &unitSphereMesh()
{
  static const SubdividedSphereMesh mesh = []()
  {
    SubdividedSphereMesh m;
    const double t = (1.0 + std::sqrt(5.0)) / 2.0;
    const double3 rawVerts[12] = {
      {-1,  t,  0}, { 1,  t,  0}, {-1, -t,  0}, { 1, -t,  0},
      { 0, -1,  t}, { 0,  1,  t}, { 0, -1, -t}, { 0,  1, -t},
      { t,  0, -1}, { t,  0,  1}, {-t,  0, -1}, {-t,  0,  1}
    };
    for (const double3 &v : rawVerts)
    {
      double3 n = v;
      n.normalise();
      m.vertices.push_back(n);
    }
    m.triangles = {
      {0, 11, 5}, {0, 5, 1}, {0, 1, 7}, {0, 7, 10}, {0, 10, 11},
      {1, 5, 9}, {5, 11, 4}, {11, 10, 2}, {10, 7, 6}, {7, 1, 8},
      {3, 9, 4}, {3, 4, 2}, {3, 2, 6}, {3, 6, 8}, {3, 8, 9},
      {4, 9, 5}, {2, 4, 11}, {6, 2, 10}, {8, 6, 7}, {9, 8, 1}
    };
    for (int level = 0; level < 3; ++level)
    {
      std::map<std::pair<int, int>, int> midpointCache;
      auto midpoint = [&](int a, int b) -> int
      {
        std::pair<int, int> key = std::minmax(a, b);
        auto it = midpointCache.find(key);
        if (it != midpointCache.end()) return it->second;
        double3 mid = m.vertices[std::size_t(a)] + m.vertices[std::size_t(b)];
        mid.normalise();
        int idx = static_cast<int>(m.vertices.size());
        m.vertices.push_back(mid);
        midpointCache[key] = idx;
        return idx;
      };
      std::vector<int3> nextTris;
      nextTris.reserve(m.triangles.size() * 4);
      for (const int3 &tri : m.triangles)
      {
        int ab = midpoint(tri.x, tri.y);
        int bc = midpoint(tri.y, tri.z);
        int ca = midpoint(tri.z, tri.x);
        nextTris.push_back({tri.x, ab, ca});
        nextTris.push_back({tri.y, bc, ab});
        nextTris.push_back({tri.z, ca, bc});
        nextTris.push_back({ab, bc, ca});
      }
      m.triangles.swap(nextTris);
    }
    return m;
  }();
  return mesh;
}
} // namespace

std::vector<float4> Structure::constructGeometricSurfaceTriangles() const
{
  if (!_cell) return {};
  std::vector<double3> positions = atomUnitCellPositions();
  if (positions.empty()) return {};

  double probeSigma = adsorptionSurfaceProbeParameters().y;
  SKGeometricSurface surface;
  if (_adsorptionSurfaceRenderingMethod == RKEnergySurfaceType::vdwGeometricSurface)
  {
    std::vector<int> elementIds = atomUnitCellElementIdentifiers();
    if (elementIds.empty()) return {};
    surface = SKGeometricSurface::buildVanDerWaals(positions, elementIds, probeSigma, _cell, appliedBlockingPockets());
  }
  else
  {
    std::vector<double2> params = potentialParameters();
    if (params.empty()) return {};
    surface = SKGeometricSurface::build(positions, params, probeSigma, _cell, appliedBlockingPockets());
  }

  if (surface.patches.empty()) return {};

  const SubdividedSphereMesh &sphere = unitSphereMesh();
  double3x3 unitCell = _cell->unitCell();
  double3x3 inverseUnitCell = _cell->inverseUnitCell();
  double3x3 unitCellTranspose = double3x3::transpose(unitCell);
  bool wrapIntoCell = _periodic;

  struct SphereClipPlane
  {
    double3 axis;
    double cosAlpha;
  };

  std::vector<float4> triangleData;
  for (const SKGeometricSurfacePatch &patch : surface.patches)
  {
    if (patch.radius <= 0.0) continue;
    std::vector<SKGeometricSurfacePatchCopy> copies = wrapIntoCell
        ? patch.copiesInsideUnitCell(_cell)
        : std::vector<SKGeometricSurfacePatchCopy>{SKGeometricSurfacePatchCopy(patch.center, patch.clips, double3(0.0, 0.0, 0.0))};

    for (const SKGeometricSurfacePatchCopy &copy : copies)
    {
      std::vector<SphereClipPlane> planes;
      planes.reserve(copy.clips.size());
      bool buried = false;
      for (const SKGeometricSurfaceClip &clip : copy.clips)
      {
        double3 delta = clip.center - copy.center;
        double d = delta.length();
        if (d < 1.0e-12)
        {
          if (clip.radius >= patch.radius) { buried = true; break; }
          continue;
        }
        double cosAlpha = (patch.radius * patch.radius + d * d - clip.radius * clip.radius) / (2.0 * patch.radius * d);
        if (cosAlpha >= 1.0) continue;
        if (cosAlpha <= -1.0) { buried = true; break; }
        planes.push_back({delta / d, cosAlpha});
      }
      if (buried) continue;

      std::vector<double3> poly;
      std::vector<double3> nextPoly;
      poly.reserve(16);
      nextPoly.reserve(16);

      for (const int3 &tri : sphere.triangles)
      {
        poly.clear();
        poly.push_back(sphere.vertices[std::size_t(tri.x)]);
        poly.push_back(sphere.vertices[std::size_t(tri.y)]);
        poly.push_back(sphere.vertices[std::size_t(tri.z)]);

        for (const SphereClipPlane &pl : planes)
        {
          bool allIn = true;
          bool allOut = true;
          for (const double3 &u : poly)
          {
            double dist = pl.cosAlpha - double3::dot(u, pl.axis);
            if (dist < 0.0) allIn = false;
            if (dist >= -1.0e-6) allOut = false;
          }
          if (allOut)
          {
            poly.clear();
            break;
          }
          if (allIn) continue;

          nextPoly.clear();
          std::size_t n = poly.size();
          for (std::size_t k = 0; k < n; ++k)
          {
            const double3 &curr = poly[k];
            const double3 &prev = poly[(k + n - 1) % n];
            double dCurr = pl.cosAlpha - double3::dot(curr, pl.axis);
            double dPrev = pl.cosAlpha - double3::dot(prev, pl.axis);
            if (dCurr >= 0.0)
            {
              if (dPrev < 0.0)
              {
                double t = dPrev / (dPrev - dCurr);
                double3 inter = prev + t * (curr - prev);
                double len = inter.length();
                if (len > 1.0e-12) inter = inter / len;
                nextPoly.push_back(inter);
              }
              nextPoly.push_back(curr);
            }
            else if (dPrev >= 0.0)
            {
              double t = dPrev / (dPrev - dCurr);
              double3 inter = prev + t * (curr - prev);
              double len = inter.length();
              if (len > 1.0e-12) inter = inter / len;
              nextPoly.push_back(inter);
            }
          }
          poly.swap(nextPoly);
          if (poly.size() < 3)
          {
            poly.clear();
            break;
          }
        }
        if (poly.size() < 3) continue;

        if (wrapIntoCell)
        {
          // Clip in fractional space against [0, 1]^3
          struct VertexFracNorm
          {
            double3 frac;
            double3 unitNorm;
          };
          std::vector<VertexFracNorm> fPoly;
          std::vector<VertexFracNorm> fNext;
          fPoly.reserve(poly.size() + 6);
          fNext.reserve(poly.size() + 6);
          for (const double3 &u : poly)
          {
            double3 cart = copy.center + patch.radius * u;
            fPoly.push_back({inverseUnitCell * cart, u});
          }

          auto clipCoord = [&](auto evalDist)
          {
            if (fPoly.size() < 3) return;
            bool allIn = true;
            bool allOut = true;
            for (const VertexFracNorm &v : fPoly)
            {
              double d = evalDist(v.frac);
              if (d < 0.0) allIn = false;
              if (d >= 0.0) allOut = false;
            }
            if (allOut) { fPoly.clear(); return; }
            if (allIn) return;
            fNext.clear();
            std::size_t n = fPoly.size();
            for (std::size_t k = 0; k < n; ++k)
            {
              const VertexFracNorm &curr = fPoly[k];
              const VertexFracNorm &prev = fPoly[(k + n - 1) % n];
              double dCurr = evalDist(curr.frac);
              double dPrev = evalDist(prev.frac);
              if (dCurr >= 0.0)
              {
                if (dPrev < 0.0)
                {
                  double t = dPrev / (dPrev - dCurr);
                  double3 f = prev.frac + t * (curr.frac - prev.frac);
                  double3 u = prev.unitNorm + t * (curr.unitNorm - prev.unitNorm);
                  double len = u.length();
                  if (len > 1.0e-12) u = u / len;
                  fNext.push_back({f, u});
                }
                fNext.push_back(curr);
              }
              else if (dPrev >= 0.0)
              {
                double t = dPrev / (dPrev - dCurr);
                double3 f = prev.frac + t * (curr.frac - prev.frac);
                double3 u = prev.unitNorm + t * (curr.unitNorm - prev.unitNorm);
                double len = u.length();
                if (len > 1.0e-12) u = u / len;
                fNext.push_back({f, u});
              }
            }
            fPoly.swap(fNext);
          };

          clipCoord([](const double3 &f) { return f.x; });
          clipCoord([](const double3 &f) { return 1.0 - f.x; });
          clipCoord([](const double3 &f) { return f.y; });
          clipCoord([](const double3 &f) { return 1.0 - f.y; });
          clipCoord([](const double3 &f) { return f.z; });
          clipCoord([](const double3 &f) { return 1.0 - f.z; });

          if (fPoly.size() < 3) continue;
          for (std::size_t k = 1; k + 1 < fPoly.size(); ++k)
          {
            const VertexFracNorm *verts[3] = {&fPoly[0], &fPoly[k], &fPoly[k + 1]};
            for (int vi = 0; vi < 3; ++vi)
            {
              const double3 &f = verts[vi]->frac;
              double3 nf = unitCellTranspose * verts[vi]->unitNorm;
              triangleData.push_back(float4(float(f.x), float(f.y), float(f.z), 1.0f));
              triangleData.push_back(float4(float(nf.x), float(nf.y), float(nf.z), 0.0f));
              triangleData.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
            }
          }
        }
        else
        {
          for (std::size_t k = 1; k + 1 < poly.size(); ++k)
          {
            const double3 *verts[3] = {&poly[0], &poly[k], &poly[k + 1]};
            for (int vi = 0; vi < 3; ++vi)
            {
              const double3 &u = *verts[vi];
              double3 f = inverseUnitCell * (copy.center + patch.radius * u);
              double3 nf = unitCellTranspose * u;
              triangleData.push_back(float4(float(f.x), float(f.y), float(f.z), 1.0f));
              triangleData.push_back(float4(float(nf.x), float(nf.y), float(nf.z), 0.0f));
              triangleData.push_back(float4(0.0f, 0.0f, 0.0f, 0.0f));
            }
          }
        }
      }
    }
  }
  return triangleData;
}


void Structure::setRepresentationStyle(RepresentationStyle style)
{
  if (int(style) >= 0 && style < RepresentationStyle::multiple_values)
  {
    _atomRepresentationStyle = style;
    bool preserveAtomBondVisibility = (this->structureType() == ObjectType::protein || this->structureType() == ObjectType::proteinCrystal);

    if (style != RepresentationStyle::custom)
    {
      _atomEdgeCueing = (style == RepresentationStyle::quteMol) ? RKEdgeCueing::contoursAndHalos : RKEdgeCueing::off;
    }

    switch (style)
    {
    case RepresentationStyle::defaultStyle:
      if (!preserveAtomBondVisibility)
      {
        _drawAtoms = true;
      }
      _atomAmbientOcclusion = false;
      _atomScaleFactor = 0.7;
      _atomHue = 1.0;
      _atomSaturation = 1.0;
      _atomValue = 1.0;
      _atomAmbientColor = QColor(255, 255, 255, 255);
      _atomSpecularColor = QColor(255, 255, 255, 255);
      _atomHDR = true;
      _atomHDRExposure = 1.5;
      _atomAmbientIntensity = 0.2;
      _atomDiffuseIntensity = 1.0;
      _atomSpecularIntensity = 1.0;
      _atomShininess = 6.0;
      _atomColorSchemeIdentifier = QString("Jmol");
      _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;

      if (!preserveAtomBondVisibility)
      {
        _drawBonds = true;
      }
      _bondAmbientOcclusion = false;
      _bondColorMode = RKBondColorMode::uniform;
      _bondScaleFactor = 0.15;
      _bondAmbientColor = QColor(255, 255, 255, 255);
      _bondDiffuseColor = QColor(205, 205, 205, 255);
      _bondSpecularColor = QColor(255, 255, 255, 255);
      _bondAmbientIntensity = 0.35;
      _bondDiffuseIntensity = 1.0;
      _bondSpecularIntensity = 1.0;
      _bondShininess = 4.0;
      _bondHDR = true;
      _bondHDRExposure = 1.5;
      _bondSelectionIntensity = 1.0;
      _bondHue = 1.0;
      _bondSaturation = 1.0;
      _bondValue = 1.0;

      _atomSelectionStyle = RKSelectionStyle::WorleyNoise3D;
      _atomSelectionScaling = 1.2;
      _atomSelectionIntensity = 0.7;
      _atomSelectionStripesDensity = 0.25;
      _atomSelectionStripesFrequency = 12.0;
      _atomSelectionWorleyNoise3DFrequency = 2.0;
      _atomSelectionWorleyNoise3DJitter = 1.0;

      _bondSelectionStyle = RKSelectionStyle::WorleyNoise3D;
      _bondSelectionScaling = 1.0;
      _bondSelectionIntensity = 0.7;
      _bondSelectionStripesDensity = 0.25;
      _bondSelectionStripesFrequency = 12.0;
      _bondSelectionWorleyNoise3DFrequency = 2.0;
      _bondSelectionWorleyNoise3DJitter = 1.0;

      setRepresentationType(RepresentationType::sticks_and_balls);
      break;
    case RepresentationStyle::fancy:
    case RepresentationStyle::quteMol:
      if (!preserveAtomBondVisibility)
      {
        _drawAtoms = true;
      }
      _atomAmbientOcclusion = true;
      _atomScaleFactor = 1.0;
      _atomHue = 1.0;
      _atomSaturation = 0.5;
      _atomValue = 1.0;
      _atomAmbientColor = QColor(255, 255, 255, 255);
      _atomSpecularColor = QColor(255, 255, 255, 255);
      _atomHDR = false;
      _atomAmbientIntensity = 1.0;
      _atomDiffuseIntensity = 0.0;
      _atomSpecularIntensity = 0.2;
      _atomShininess = 4.0;
      _atomScaleFactor = 1.0;
      _atomColorSchemeIdentifier = QString("Rasmol");
      _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;

      if (!preserveAtomBondVisibility)
      {
        _drawBonds = false;
      }
      _bondAmbientOcclusion = true;

      _atomSelectionStyle = RKSelectionStyle::WorleyNoise3D;
      _atomSelectionScaling = 1.0;
      _atomSelectionIntensity = 0.4;
      _atomSelectionStripesDensity = 0.25;
      _atomSelectionStripesFrequency = 12.0;
      _atomSelectionWorleyNoise3DFrequency = 2.0;
      _atomSelectionWorleyNoise3DJitter = 1.0;

      _bondSelectionStyle = RKSelectionStyle::WorleyNoise3D;
      _bondSelectionScaling = 1.0;
      _bondSelectionIntensity = 0.4;
      _bondSelectionStripesDensity = 0.25;
      _bondSelectionStripesFrequency = 12.0;
      _bondSelectionWorleyNoise3DFrequency = 2.0;
      _bondSelectionWorleyNoise3DJitter = 1.0;

      setRepresentationType(RepresentationType::vdw);
      break;
    case RepresentationStyle::licorice:
      if (!preserveAtomBondVisibility)
      {
        _drawAtoms = true;
      }
      _atomAmbientOcclusion = false;
      _atomHue = 1.0;
      _atomSaturation = 1.0;
      _atomValue = 1.0;
      _atomAmbientColor = QColor(255, 255, 255, 255);
      _atomSpecularColor = QColor(255, 255, 255, 255);
      _atomAmbientIntensity = 0.1;
      _atomDiffuseIntensity = 1.0;
      _atomSpecularIntensity = 1.0;
      _atomShininess = 4.0;
      _atomHDR = true;
      _atomHDRExposure = 1.5;
      _atomScaleFactor = 1.0;
      _atomColorSchemeIdentifier = QString("Jmol");
      _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;

      if (!preserveAtomBondVisibility)
      {
        _drawBonds = true;
      }
      _bondAmbientOcclusion = false;
      _bondColorMode = RKBondColorMode::split;
      _bondScaleFactor = 0.25;
      _bondAmbientColor = QColor(255, 255, 255, 255);
      _bondDiffuseColor = QColor(205, 205, 205, 255);
      _bondSpecularColor = QColor(255, 255, 255, 255);
      _bondAmbientIntensity = 0.1;
      _bondDiffuseIntensity = 1.0;
      _bondSpecularIntensity = 1.0;
      _bondShininess = 4.0;
      _bondHDR = true;
      _bondHDRExposure = 1.5;
      _bondSelectionIntensity = 1.0;
      _bondHue = 1.0;
      _bondSaturation = 1.0;
      _bondValue = 1.0;

      _atomSelectionStyle = RKSelectionStyle::WorleyNoise3D;
      _atomSelectionScaling = 1.5;
      _atomSelectionIntensity = 0.8;
      _atomSelectionStripesDensity = 0.25;
      _atomSelectionStripesFrequency = 12.0;
      _atomSelectionWorleyNoise3DFrequency = 2.0;
      _atomSelectionWorleyNoise3DJitter = 1.0;

      _bondSelectionStyle = RKSelectionStyle::WorleyNoise3D;
      _bondSelectionScaling = 1.0;
      _bondSelectionIntensity = 0.8;
      _bondSelectionStripesDensity = 0.25;
      _bondSelectionStripesFrequency = 12.0;
      _bondSelectionWorleyNoise3DFrequency = 2.0;
      _bondSelectionWorleyNoise3DJitter = 1.0;

      setRepresentationType(RepresentationType::unity);
      break;
    case RepresentationStyle::objects:
      if (!preserveAtomBondVisibility)
      {
        _drawAtoms = true;
      }
      _atomScaleFactor = 1.0;
      _atomColorSchemeIdentifier = QString("Jmol");
      _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;
      _atomAmbientOcclusion = false;
      _atomHDR = true;
      _atomHDRExposure = 1.5;
      _atomAmbientIntensity = 0.1;
      _atomDiffuseIntensity = 0.6;
      _atomSpecularIntensity = 0.1;
      _atomShininess = 4.0;
      setRepresentationType(RepresentationType::unity);
      break;
    case RepresentationStyle::multiple_values:
      break;
    default:
      break;
    }
  }

  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms;
  asymmetricAtoms.reserve(asymmetricAtomNodes.size());
  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      asymmetricAtoms.push_back(atom);
    }
  }
  applyRepresentationDrawRadius(asymmetricAtoms);
}

void Structure::setRepresentationStyle(RepresentationStyle style, const SKColorSets &colorSets)
{
  setRepresentationStyle(style);

  switch (_atomRepresentationStyle)
  {
  case RepresentationStyle::defaultStyle:
    setRepresentationColorSchemeIdentifier("Jmol", colorSets);
    break;
  case RepresentationStyle::fancy:
  case RepresentationStyle::quteMol:
    setRepresentationColorSchemeIdentifier("Rasmol", colorSets);
    break;
  case RepresentationStyle::licorice:
  case RepresentationStyle::objects:
    setRepresentationColorSchemeIdentifier("Jmol", colorSets);
    break;
  case RepresentationStyle::multiple_values:
    break;
  default:
    break;
  }
}

void Structure::recheckRepresentationStyle()
{
  bool preserveAtomBondVisibility = (this->structureType() == ObjectType::protein || this->structureType() == ObjectType::proteinCrystal);
  bool atomsDrawn = preserveAtomBondVisibility || (_drawAtoms == true);
  bool bondsDrawn = preserveAtomBondVisibility || (_drawBonds == true);
  bool bondsHidden = preserveAtomBondVisibility || (_drawBonds == false);

  if (atomsDrawn &&
     (fabs(_atomHue - 1.0) < 1e-4) &&
     (fabs(_atomSaturation - 1.0) < 1e-4) &&
     (fabs(_atomValue - 1.0) < 1e-4) &&
     (_atomAmbientColor.red() == 255) &&
     (_atomAmbientColor.green() == 255) &&
     (_atomAmbientColor.blue() == 255) &&
     (_atomAmbientColor.alpha() == 255) &&
     (_atomSpecularColor.red() == 255) &&
     (_atomSpecularColor.green() == 255) &&
     (_atomSpecularColor.blue() == 255) &&
     (_atomSpecularColor.alpha() == 255) &&
     (_atomHDR == true) &&
     (fabs(_atomHDRExposure - 1.5) < 1e-4) &&
     (_atomAmbientOcclusion == false) &&
     (fabs(_atomAmbientIntensity - 0.2) < 1e-4) &&
     (fabs(_atomDiffuseIntensity - 1.0) < 1e-4) &&
     (fabs(_atomSpecularIntensity - 1.0) < 1e-4) &&
     (fabs(_atomShininess - 6.0) < 1e-4) &&
     (fabs(_atomScaleFactor - 0.7) < 1e-4) &&
     (_atomRepresentationType == RepresentationType::sticks_and_balls) &&
     (QString::compare(_atomColorSchemeIdentifier, "Jmol", Qt::CaseInsensitive) == 0) &&
     (_atomColorSchemeOrder == SKColorSet::ColorSchemeOrder::elementOnly) &&
     bondsDrawn &&
     (_bondColorMode == RKBondColorMode::uniform) &&
     (fabs(_bondScaleFactor - 0.15) < 1e-4) &&
     (_bondAmbientOcclusion == false) &&
     (_bondHDR == true) &&
     (fabs(_bondHDRExposure - 1.5) < 1e-4) &&
     (_bondAmbientColor.red() == 255) &&
     (_bondAmbientColor.green() == 255) &&
     (_bondAmbientColor.blue() == 255) &&
     (_bondAmbientColor.alpha() == 255) &&
     (_bondDiffuseColor.red() == 205) &&
     (_bondDiffuseColor.green() == 205) &&
     (_bondDiffuseColor.blue() == 205) &&
     (_bondDiffuseColor.alpha() == 255) &&
     (_bondSpecularColor.red() == 255) &&
     (_bondSpecularColor.green() == 255) &&
     (_bondSpecularColor.blue() == 255) &&
     (_bondSpecularColor.alpha() == 255) &&
     (fabs(_bondAmbientIntensity - 0.35) < 1e-4) &&
     (fabs(_bondDiffuseIntensity - 1.0) < 1e-4) &&
     (fabs(_bondSpecularIntensity - 1.0) < 1e-4) &&
     (fabs(_bondShininess - 4.0) < 1e-4) &&
     (fabs(_bondHue - 1.0) < 1e-4) &&
     (fabs(_bondSaturation - 1.0) < 1e-4) &&
     (fabs(_bondValue - 1.0) < 1e-4) &&
     (_atomSelectionStyle == RKSelectionStyle::WorleyNoise3D) &&
     (fabs(_atomSelectionScaling - 1.2) < 1e-4) &&
     (_atomEdgeCueing == RKEdgeCueing::off))
     {
       _atomRepresentationStyle = RepresentationStyle::defaultStyle;
     }
  else if ((fabs(_atomHue - 1.0) < 1e-4) &&
     (fabs(_atomSaturation - 0.5) < 1e-4) &&
     (fabs(_atomValue - 1.0) < 1e-4) &&
     (fabs(_atomAmbientColor.redF() - 1.0) < 1e-4) &&
     (fabs(_atomAmbientColor.greenF() - 1.0) < 1e-4) &&
     (fabs(_atomAmbientColor.blueF() - 1.0) < 1e-4) &&
     (fabs(_atomAmbientColor.alphaF() - 1.0) < 1e-4) &&
     (fabs(_atomSpecularColor.redF() - 1.0) < 1e-4) &&
     (fabs(_atomSpecularColor.greenF() - 1.0) < 1e-4) &&
     (fabs(_atomSpecularColor.blueF() - 1.0) < 1e-4) &&
     (fabs(_atomSpecularColor.alphaF() - 1.0) < 1e-4) &&
     atomsDrawn &&
     bondsHidden &&
     (_atomHDR == false) &&
     (_atomAmbientOcclusion == true) &&
     (_bondAmbientOcclusion == true) &&
     (fabs(_atomAmbientIntensity - 1.0) < 1e-4) &&
     (fabs(_atomDiffuseIntensity - 0.0) < 1e-4) &&
     (fabs(_atomSpecularIntensity - 0.2) < 1e-4) &&
     (fabs(_atomShininess - 4.0) < 1e-4) &&
     (fabs(_atomScaleFactor - 1.0) < 1e-4) &&
     (_atomRepresentationType == RepresentationType::vdw) &&
     (QString::compare(_atomColorSchemeIdentifier, "Rasmol", Qt::CaseInsensitive) == 0) &&
     (_atomColorSchemeOrder == SKColorSet::ColorSchemeOrder::elementOnly) &&
     (_atomSelectionStyle == RKSelectionStyle::WorleyNoise3D) &&
     (fabs(_atomSelectionScaling - 1.0) < 1e-4) &&
     (_atomEdgeCueing == RKEdgeCueing::off || _atomEdgeCueing == RKEdgeCueing::contoursAndHalos))
     {
       _atomRepresentationStyle = (_atomEdgeCueing == RKEdgeCueing::contoursAndHalos) ? RepresentationStyle::quteMol : RepresentationStyle::fancy;
     }
  else if (atomsDrawn &&
     (_atomRepresentationType == RepresentationType::unity) &&
     (QString::compare(_atomColorSchemeIdentifier, "Jmol", Qt::CaseInsensitive) == 0) &&
     (_atomColorSchemeOrder == SKColorSet::ColorSchemeOrder::elementOnly) &&
     (fabs(_atomScaleFactor - 1.0) < 1e-4) &&
     bondsDrawn &&
     (_bondColorMode == RKBondColorMode::split) &&
     (fabs(_bondScaleFactor - 0.25) < 1e-4) &&
     (_bondAmbientOcclusion == false) &&
     (_bondHDR == true) &&
     (fabs(_bondHDRExposure - 1.5) < 1e-4) &&
     (_bondAmbientColor.red() == 255) &&
     (_bondAmbientColor.green() == 255) &&
     (_bondAmbientColor.blue() == 255) &&
     (_bondAmbientColor.alpha() == 255) &&
     (_bondDiffuseColor.red() == 205) &&
     (_bondDiffuseColor.green() == 205) &&
     (_bondDiffuseColor.blue() == 205) &&
     (_bondDiffuseColor.alpha() == 255) &&
     (_bondSpecularColor.red() == 255) &&
     (_bondSpecularColor.green() == 255) &&
     (_bondSpecularColor.blue() == 255) &&
     (_bondSpecularColor.alpha() == 255) &&
     (fabs(_bondAmbientIntensity - 0.1) < 1e-4) &&
     (fabs(_bondDiffuseIntensity - 1.0) < 1e-4) &&
     (fabs(_bondSpecularIntensity - 1.0) < 1e-4) &&
     (fabs(_bondShininess - 4.0) < 1e-4) &&
     (fabs(_bondHue - 1.0) < 1e-4) &&
     (fabs(_bondSaturation - 1.0) < 1e-4) &&
     (fabs(_bondValue - 1.0) < 1e-4) &&
     (_atomSelectionStyle == RKSelectionStyle::WorleyNoise3D) &&
     (fabs(_atomSelectionScaling - 1.5) < 1e-4) &&
     (_atomEdgeCueing == RKEdgeCueing::off))
     {
       _atomRepresentationStyle = RepresentationStyle::licorice;
     }
  else if (atomsDrawn &&
     (_atomRepresentationType == RepresentationType::unity) &&
     (QString::compare(_atomColorSchemeIdentifier, "Jmol", Qt::CaseInsensitive) == 0) &&
     (_atomColorSchemeOrder == SKColorSet::ColorSchemeOrder::elementOnly) &&
     (fabs(_atomScaleFactor - 1.0) < 1e-4) &&
     (_atomAmbientOcclusion == false) &&
     (_atomHDR == true) &&
     (fabs(_atomHDRExposure - 1.5) < 1e-4) &&
     (fabs(_atomAmbientIntensity - 0.1) < 1e-4) &&
     (fabs(_atomDiffuseIntensity - 0.6) < 1e-4) &&
     (fabs(_atomSpecularIntensity - 0.1) < 1e-4) &&
     (fabs(_atomShininess - 4.0) < 1e-4) &&
     (_atomEdgeCueing == RKEdgeCueing::off))
     {
       _atomRepresentationStyle = RepresentationStyle::objects;
     }
  else
  {
    _atomRepresentationStyle = RepresentationStyle::custom;
  }
}

void Structure::setRepresentationType(RepresentationType type)
{
  if(int(type)>=0 && type < RepresentationType::multiple_values)
  {
    _atomRepresentationType = type;
    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
    std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms;
    asymmetricAtoms.reserve(asymmetricAtomNodes.size());
    for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
    {
      if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
      {
        asymmetricAtoms.push_back(atom);
      }
    }
    applyRepresentationType(type, asymmetricAtoms);
  }
}

void Structure::applyRepresentationType(RepresentationType type, const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms)
{
  switch(type)
  {
    case RepresentationType::sticks_and_balls:
    case RepresentationType::vdw:
    case RepresentationType::forcefield:
      _bondScaleFactor = 0.15;
      break;
    case RepresentationType::unity:
      _bondScaleFactor = 0.25;
      break;
    default:
      break;
  }
  applyRepresentationDrawRadius(atoms);
}

void Structure::applyRepresentationDrawRadius(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms)
{
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : atoms)
  {
    if (!atom) continue;
    switch (_atomRepresentationType)
    {
    case RepresentationType::sticks_and_balls:
      atom->setDrawRadius(PredefinedElements::predefinedElements[atom->elementIdentifier()]._covalentRadius);
      break;
    case RepresentationType::vdw:
      atom->setDrawRadius(PredefinedElements::predefinedElements[atom->elementIdentifier()]._VDWRadius);
      break;
    case RepresentationType::unity:
      atom->setDrawRadius(_bondScaleFactor);
      break;
    case RepresentationType::forcefield:
      atom->setDrawRadius(SKGeometricSurface::inflatedRadius(atom->potentialParameters().y, 0.0));
      break;
    default:
      break;
    }
  }
}

void Structure::setRepresentationColorSchemeIdentifier(const QString colorSchemeName, const SKColorSets &colorSets)
{

  if(const SKColorSet* colorSet = colorSets[colorSchemeName])
  {
    _atomColorSchemeIdentifier = colorSchemeName;
    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
    {
      if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
      {
        switch(_atomColorSchemeOrder)
        {
        case SKColorSet::ColorSchemeOrder::elementOnly:
        {
          QString chemicalElement = PredefinedElements::predefinedElements[atom->elementIdentifier()]._chemicalSymbol;
          const QColor* color = (*colorSet)[chemicalElement];
          if(color)
          {
            atom->setColor(*color);
          }
          break;
        }
        case SKColorSet::ColorSchemeOrder::forceFieldFirst:
        {
          const QColor* color = (*colorSet)[atom->uniqueForceFieldName()];
          if(color)
          {
            atom->setColor(*color);
            break;
          }
          QString chemicalElement = PredefinedElements::predefinedElements[atom->elementIdentifier()]._chemicalSymbol;
          const QColor* color2 = (*colorSet)[chemicalElement];
          if(color2)
          {
            atom->setColor(*color2);
            break;
          }
          atom->setColor(QColor(0,0,0,255));
          break;
        }
        case SKColorSet::ColorSchemeOrder::forceFieldOnly:
        {
          const QColor* color = (*colorSet)[atom->uniqueForceFieldName()];
          if(color)
          {
            atom->setColor(*color);
            break;
          }
          atom->setColor(QColor(0,0,0,255));
          break;
        }
        default:
          break;
        }

      }
    }
  }
}


void Structure::setAtomForceFieldIdentifier(QString identifier, ForceFieldSets& forceFieldSets)
{
  _atomForceFieldIdentifier = identifier;
  updateForceField(forceFieldSets);
}

std::optional<ForceFieldType> Structure::resolvedForceFieldType(const std::shared_ptr<SKAsymmetricAtom> &atom, const ForceFieldSet &forceFieldSet) const
{
  if (!atom) return std::nullopt;
  const QString elementString = PredefinedElements::predefinedElements[atom->elementIdentifier()]._chemicalSymbol;
  switch (_atomForceFieldOrder)
  {
  case ForceFieldSet::ForceFieldSchemeOrder::elementOnly:
  case ForceFieldSet::ForceFieldSchemeOrder::forceFieldFirst:
    if (const ForceFieldType *type = forceFieldSet[atom->uniqueForceFieldName()])
    {
      return *type;
    }
    if (const ForceFieldType *type = forceFieldSet[elementString])
    {
      return *type;
    }
    return ForceFieldSet::defaultType(elementString);
  case ForceFieldSet::ForceFieldSchemeOrder::forceFieldOnly:
    if (const ForceFieldType *type = forceFieldSet[atom->uniqueForceFieldName()])
    {
      return *type;
    }
    return ForceFieldSet::defaultType(elementString);
  case ForceFieldSet::ForceFieldSchemeOrder::multiple_values:
    return std::nullopt;
  }
  return std::nullopt;
}

void Structure::assignAluminosilicateOxygenTypes(const std::vector<std::shared_ptr<SKAsymmetricAtom>> &asymmetricAtoms)
{
  if (!_cell) return;
  const std::set<int> &trivalentNumbers = ForceFieldSet::trivalentTAtomAtomicNumbers;
  std::vector<double3> tPositions;
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : asymmetricAtoms)
  {
    if (!atom || trivalentNumbers.find(atom->elementIdentifier()) == trivalentNumbers.end()) continue;
    if (atom->copies().empty())
    {
      tPositions.push_back(atom->position());
    }
    else
    {
      for (const std::shared_ptr<SKAtomCopy> &copy : atom->copies())
      {
        tPositions.push_back(copy->position());
      }
    }
  }

  double3x3 unitCell = _cell->unitCell();
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : asymmetricAtoms)
  {
    if (!atom || atom->elementIdentifier() != 8) continue;
    if (atom->uniqueForceFieldName() != "O" && atom->uniqueForceFieldName() != ForceFieldSet::bridgingAluminumOxygenIdentifier)
    {
      continue;
    }
    std::vector<double3> oxygenPositions;
    if (atom->copies().empty())
    {
      oxygenPositions.push_back(atom->position());
    }
    else
    {
      for (const std::shared_ptr<SKAtomCopy> &copy : atom->copies())
      {
        oxygenPositions.push_back(copy->position());
      }
    }
    bool bridgesAluminum = false;
    for (const double3 &pos : oxygenPositions)
    {
      if (ForceFieldSet::isBridgingAluminumOxygen(pos, tPositions, unitCell))
      {
        bridgesAluminum = true;
        break;
      }
    }
    atom->setUniqueForceFieldName(bridgesAluminum ? ForceFieldSet::bridgingAluminumOxygenIdentifier : "O");
  }
}

void Structure::updateForceField(ForceFieldSets &forceFieldSets)
{
  ForceFieldSet forceFieldSet = forceFieldSets.resolvedSet(_atomForceFieldIdentifier);
  std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms;
  asymmetricAtoms.reserve(asymmetricAtomNodes.size());
  for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
  {
    if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
    {
      asymmetricAtoms.push_back(atom);
    }
  }

  bool isAluminosilicate = ForceFieldSet::isAluminosilicateFamily(forceFieldSet.displayName());
  if (isAluminosilicate)
  {
    assignAluminosilicateOxygenTypes(asymmetricAtoms);
  }
  else
  {
    for (const std::shared_ptr<SKAsymmetricAtom> &atom : asymmetricAtoms)
    {
      if (atom && atom->uniqueForceFieldName() == ForceFieldSet::bridgingAluminumOxygenIdentifier &&
          forceFieldSet[ForceFieldSet::bridgingAluminumOxygenIdentifier] == nullptr)
      {
        atom->setUniqueForceFieldName("O");
      }
    }
  }

  for (const std::shared_ptr<SKAsymmetricAtom> &atom : asymmetricAtoms)
  {
    if (!atom) continue;
    std::optional<ForceFieldType> type = resolvedForceFieldType(atom, forceFieldSet);
    atom->setPotentialParameters(type ? type->potentialParameters() : double2(0.0, 0.0));
    atom->setBondDistanceCriteria(type ? type->userDefinedRadius() : ((_atomForceFieldOrder == ForceFieldSet::ForceFieldSchemeOrder::forceFieldOnly) ? 1.0 : 0.0));
    if (isAluminosilicate)
    {
      atom->setCharge(type ? type->charge() : 0.0);
    }
  }

  applyRepresentationDrawRadius(asymmetricAtoms);
}

double Structure::atomSelectionFrequency() const
{
  switch(_atomSelectionStyle)
  {  
  case RKSelectionStyle::WorleyNoise3D:
    return _atomSelectionWorleyNoise3DFrequency;
  case RKSelectionStyle::striped:
      return _atomSelectionStripesFrequency;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
     return 0.0;
  }
  return 0.0;
}

void Structure::setAtomSelectionFrequency(double value)
{
  switch(_atomSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _atomSelectionWorleyNoise3DFrequency = value;
    break;
  case RKSelectionStyle::striped:
    _atomSelectionStripesFrequency = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}

double Structure::atomSelectionDensity() const
{
  switch(_atomSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _atomSelectionWorleyNoise3DJitter;
  case RKSelectionStyle::striped:
    return _atomSelectionStripesDensity;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    return 0.0;
  }
  return 0.0;
}
void Structure::setAtomSelectionDensity(double value)
{
  switch(_atomSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _atomSelectionWorleyNoise3DJitter = value;
    break;
  case RKSelectionStyle::striped:
    _atomSelectionStripesDensity = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}


double Structure::bondSelectionFrequency() const
{
  switch(_bondSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _bondSelectionWorleyNoise3DFrequency;
  case RKSelectionStyle::striped:
      return _bondSelectionStripesFrequency;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
     return 0.0;
  }
  return 0.0;
}

void Structure::setBondSelectionFrequency(double value)
{
  switch(_bondSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _bondSelectionWorleyNoise3DFrequency = value;
    break;
  case RKSelectionStyle::striped:
    _bondSelectionStripesFrequency = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}

double Structure::bondSelectionDensity() const
{
  switch(_bondSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _bondSelectionWorleyNoise3DJitter;
  case RKSelectionStyle::striped:
    return _bondSelectionStripesDensity;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    return 0.0;
  }
  return 0.0;
}
void Structure::setBondSelectionDensity(double value)
{
  switch(_bondSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _bondSelectionWorleyNoise3DJitter = value;
    break;
  case RKSelectionStyle::striped:
    _bondSelectionStripesDensity = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}
void  Structure::clearSelection()
{
  _atomsTreeController->clearSelection();
  _bondSetController->selectionIndexSet().clear();
}


void Structure::setAtomSelection(int asymmetricAtomId)
{
  _atomsTreeController->clearSelection();
  addAtomToSelection(asymmetricAtomId);

  _bondSetController->correctBondSelectionDueToAtomSelection();
  recomputeSelectionBodyFixedBasis();
}

void Structure::setBondSelection(int asymmetricBondId)
{
  _bondSetController->selectionIndexSet().clear();
  addBondToSelection(asymmetricBondId);
}

void Structure::addAtomToSelection(int atomId)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes = _atomsTreeController->flattenedLeafNodes();
  if (atomId < 0 || static_cast<size_t>(atomId) >= atomNodes.size())
  {
    return;
  }

  std::shared_ptr<SKAtomTreeNode> selectedAtom = atomNodes[atomId];

  _atomsTreeController->insertSelectionIndexPath(selectedAtom->indexPath());

  _bondSetController->correctBondSelectionDueToAtomSelection();
  recomputeSelectionBodyFixedBasis();
}

void Structure::addBondToSelection(int asymmetricBondId)
{
  _bondSetController->selectionIndexSet().insert(asymmetricBondId);
  _bondSetController->correctBondSelectionDueToAtomSelection();
}

void Structure::toggleAtomSelection(int asymmetricAtomId)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes = _atomsTreeController->flattenedLeafNodes();
  if (asymmetricAtomId < 0 || static_cast<size_t>(asymmetricAtomId) >= atomNodes.size())
  {
    return;
  }

  std::shared_ptr<SKAtomTreeNode> selectedAtom = atomNodes[asymmetricAtomId];
  IndexPath indexPathSelectedAtom = selectedAtom->indexPath();

  AtomSelectionIndexPaths atomSelection = _atomsTreeController->selectionIndexPaths();

  std::set<IndexPath>::const_iterator search = atomSelection.second.find(indexPathSelectedAtom);
  if (search != atomSelection.second.end())
  {
    _atomsTreeController->removeSelectionIndexPath(*search);

    // remove bonds that are connected to this atom from the selection
    atomNodes = atomsTreeController()->flattenedLeafNodes();
    selectedAtom = atomNodes[asymmetricAtomId];
    std::shared_ptr<SKAsymmetricAtom> atom = selectedAtom->representedObject();
    int bondIndex=0;
    for(const std::shared_ptr<SKAsymmetricBond> &bond : bondSetController()->arrangedObjects())
    {
      if(atom == bond->atom1() || atom == bond->atom2())
      {
        bondSetController()->selectionIndexSet().erase(bondIndex);
      }
      bondIndex++;
    }
  }
  else
  {
    _atomsTreeController->insertSelectionIndexPath(indexPathSelectedAtom);
  }

  _bondSetController->correctBondSelectionDueToAtomSelection();
  recomputeSelectionBodyFixedBasis();
}

void Structure::toggleBondSelection(int asymmetricBondId)
{
  std::set<qint64>::const_iterator search = _bondSetController->selectionIndexSet().find(asymmetricBondId);
  if (search != _bondSetController->selectionIndexSet().end())
  {
    _bondSetController->selectionIndexSet().erase(*search);
  }
  else
  {
    _bondSetController->selectionIndexSet().insert(asymmetricBondId);
  }
  _bondSetController->correctBondSelectionDueToAtomSelection();
}


void Structure::setAtomSelection(std::set<int>& atomIds)
{
  _atomsTreeController->clearSelection();
  addToAtomSelection(atomIds);
  recomputeSelectionBodyFixedBasis();
}

void Structure::addToAtomSelection(std::set<int>& atomIds)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes = _atomsTreeController->flattenedLeafNodes();

  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = _atomsTreeController->atomCopies();

  for(int atomId: atomIds)
  {
    std::shared_ptr<SKAtomTreeNode> selectedAtom = atomNodes[atomId];

    _atomsTreeController->insertSelectionIndexPath(selectedAtom->indexPath());
  }

  _bondSetController->correctBondSelectionDueToAtomSelection();
  recomputeSelectionBodyFixedBasis();
}


void Structure::setStructureNitrogenSurfaceArea(double value)
{
  _structureGravimetricNitrogenSurfaceArea = value * Constants::AvogadroConstantPerAngstromSquared / _structureMass;
  _structureVolumetricNitrogenSurfaceArea = value * 1e4 / cell()->volume();
}

void Structure::setStructureWellSurfaceArea(double value)
{
  _structureGravimetricWellSurfaceArea = value * Constants::AvogadroConstantPerAngstromSquared / _structureMass;
  _structureVolumetricWellSurfaceArea = value * 1e4 / cell()->volume();
}

void Structure::setStructureGeometricSurfaceArea(double value)
{
  _structureGravimetricGeometricSurfaceArea = value * Constants::AvogadroConstantPerAngstromSquared / _structureMass;
  _structureVolumetricGeometricSurfaceArea = value * 1e4 / cell()->volume();
}

void Structure::setStructureVanDerWaalsGeometricSurfaceArea(double value)
{
  _structureGravimetricVanDerWaalsGeometricSurfaceArea = value * Constants::AvogadroConstantPerAngstromSquared / _structureMass;
  _structureVolumetricVanDerWaalsGeometricSurfaceArea = value * 1e4 / cell()->volume();
}

double2 Structure::structureWindowSize(int index) const
{
  switch (index)
  {
  case 0: return _structureWindow1Size;
  case 1: return _structureWindow2Size;
  default: return _structureWindow3Size;
  }
}

void Structure::setStructureWindowSize(int index, double2 value)
{
  switch (index)
  {
  case 0: _structureWindow1Size = value; break;
  case 1: _structureWindow2Size = value; break;
  default: _structureWindow3Size = value; break;
  }
}

qint64 Structure::structureWindowRingAtoms(int index) const
{
  switch (index)
  {
  case 0: return _structureWindow1RingAtoms;
  case 1: return _structureWindow2RingAtoms;
  default: return _structureWindow3RingAtoms;
  }
}

void Structure::setStructureWindowRingAtoms(int index, qint64 value)
{
  switch (index)
  {
  case 0: _structureWindow1RingAtoms = value; break;
  case 1: _structureWindow2RingAtoms = value; break;
  default: _structureWindow3RingAtoms = value; break;
  }
}

std::vector<double4> Structure::parseBlockingPockets(const QString &contents)
{
  std::vector<double4> pockets;
  const QStringList lines = contents.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
  const QStringList commentMarkers = {"#", "//", "!", ";"};
  const QRegularExpression separatorRegex("[ \t,]+");

  for (const QString &line : lines)
  {
    QString strippedLine = line;
    for (const QString &marker : commentMarkers)
    {
      int idx = strippedLine.indexOf(marker);
      if (idx >= 0)
      {
        strippedLine = strippedLine.left(idx);
      }
    }
    const QStringList words = strippedLine.split(separatorRegex, Qt::SkipEmptyParts);
    if (words.size() < 4)
    {
      continue;
    }
    bool ok0 = false, ok1 = false, ok2 = false, ok3 = false;
    double x = words[0].toDouble(&ok0);
    double y = words[1].toDouble(&ok1);
    double z = words[2].toDouble(&ok2);
    double radius = words[3].toDouble(&ok3);
    if (ok0 && ok1 && ok2 && ok3)
    {
      pockets.emplace_back(x, y, z, radius);
    }
  }
  return pockets;
}

std::vector<RKInPerInstanceAttributesAtoms> Structure::renderBlockingPockets() const
{
  if (_blockingPockets.empty() || !_cell)
  {
    return {};
  }

  std::vector<RKInPerInstanceAttributesAtoms> data;
  int minimumReplicaX = _cell->minimumReplicaX();
  int minimumReplicaY = _cell->minimumReplicaY();
  int minimumReplicaZ = _cell->minimumReplicaZ();
  int maximumReplicaX = _cell->maximumReplicaX();
  int maximumReplicaY = _cell->maximumReplicaY();
  int maximumReplicaZ = _cell->maximumReplicaZ();

  auto wrapRange = [this](double center, double radius, double width, int replicaMin, int replicaMax) -> std::pair<int, int>
  {
    if (!_periodic || width <= 1.0e-12 || radius <= 0.0)
    {
      return {replicaMin, replicaMax};
    }
    double fractionalRadius = radius / width;
    double boxMin = double(replicaMin);
    double boxMax = double(replicaMax) + 1.0;
    int wrappedMin = int(std::ceil(boxMin - center - fractionalRadius + 1.0e-12));
    int wrappedMax = int(std::floor(boxMax - center + fractionalRadius - 1.0e-12));
    int lower = std::min(replicaMin, wrappedMin);
    int upper = std::max(replicaMax, wrappedMax);
    return {lower, std::max(lower, upper)};
  };

  double3 widths = _cell->perpendicularWidths();
  float4 ambient = float4(_blockingPocketsFrontSideAmbientColor.redF(),
                          _blockingPocketsFrontSideAmbientColor.greenF(),
                          _blockingPocketsFrontSideAmbientColor.blueF(),
                          _blockingPocketsFrontSideAmbientColor.alphaF());
  float4 diffuse = float4(_blockingPocketsFrontSideDiffuseColor.redF(),
                          _blockingPocketsFrontSideDiffuseColor.greenF(),
                          _blockingPocketsFrontSideDiffuseColor.blueF(),
                          _blockingPocketsFrontSideDiffuseColor.alphaF());
  float4 specular = float4(_blockingPocketsFrontSideSpecularColor.redF(),
                           _blockingPocketsFrontSideSpecularColor.greenF(),
                           _blockingPocketsFrontSideSpecularColor.blueF(),
                           _blockingPocketsFrontSideSpecularColor.alphaF());

  for (const double4 &blockingPocket : _blockingPockets)
  {
    double3 position = double3::flip(double3(blockingPocket.x, blockingPocket.y, blockingPocket.z), _cell->contentFlip(), double3(1.0, 1.0, 1.0));
    double3 base = position + _cell->contentShift();
    double radius = blockingPocket.w;

    auto [minX, maxX] = wrapRange(base.x, radius, widths.x, minimumReplicaX, maximumReplicaX);
    auto [minY, maxY] = wrapRange(base.y, radius, widths.y, minimumReplicaY, maximumReplicaY);
    auto [minZ, maxZ] = wrapRange(base.z, radius, widths.z, minimumReplicaZ, maximumReplicaZ);

    for (int k1 = minX; k1 <= maxX; ++k1)
    {
      for (int k2 = minY; k2 <= maxY; ++k2)
      {
        for (int k3 = minZ; k3 <= maxZ; ++k3)
        {
          double3 fractionalPosition = double3(position.x + double(k1), position.y + double(k2), position.z + double(k3)) + _cell->contentShift();
          double3 cartesianPosition = _cell->convertToCartesian(fractionalPosition);
          float r = float(radius);
          RKInPerInstanceAttributesAtoms instance(
              float4(float(cartesianPosition.x), float(cartesianPosition.y), float(cartesianPosition.z), 1.0f),
              ambient,
              diffuse,
              specular,
              float4(r, r, r, 1.0f),
              static_cast<int32_t>(data.size()));
          data.push_back(instance);
        }
      }
    }
  }

  return data;
}

void Structure::recomputeDensityProperties()
{
  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = _atomsTreeController->atomCopies();

  _structureMass = 0.0;
  for(const std::shared_ptr<SKAtomCopy> &atom : atomCopies)
  {
    int elementId = atom->parent()->elementIdentifier();
    _structureMass +=  PredefinedElements::predefinedElements[elementId]._mass;
  }

  _structureDensity = 1.0e-3 * _structureMass / (Constants::AvogadroConstantPerAngstromCubed * _cell->volume());
  _structureSpecificVolume = 1.0e3 / _structureDensity;
  _structureAccessiblePoreVolume = _structureHeliumVoidFraction * _structureSpecificVolume;
}


QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Structure> &structure)
{
  stream << structure->_versionNumber;

  if(structure->_versionNumber < 9)
  {
    stream << structure->_displayName;
    stream << structure->_isVisible;

    stream << structure->_legacySpaceGroup;

    stream << structure->_cell;
    stream << structure->_periodic;
    stream << structure->_origin;
    stream << structure->_scaling;
    stream << structure->_orientation;
    stream << structure->_rotationDelta;
  }

  if(structure->_versionNumber < 9)
  {
    stream << structure->_primitiveTransformationMatrix;
    stream << structure->_primitiveOrientation;
    stream << structure->_primitiveRotationDelta;

    stream << structure->_primitiveOpacity;
    stream << structure->_primitiveIsCapped;
    stream << structure->_primitiveIsFractional;
    stream << structure->_primitiveNumberOfSides;
    stream << structure->_primitiveThickness;


    stream << structure->_primitiveHue;
    stream << structure->_primitiveSaturation;
    stream << structure->_primitiveValue;

    stream << static_cast<typename std::underlying_type<RKSelectionStyle>::type>(structure->_primitiveSelectionStyle);
    stream << structure->_primitiveSelectionStripesDensity;
    stream << structure->_primitiveSelectionStripesFrequency;
    stream << structure->_primitiveSelectionWorleyNoise3DFrequency;
    stream << structure->_primitiveSelectionWorleyNoise3DJitter;
    stream << structure->_primitiveSelectionScaling;
    stream << structure->_primitiveSelectionIntensity;


    stream << structure->_primitiveFrontSideHDR;
    stream << structure->_primitiveFrontSideHDRExposure;
    stream << structure->_primitiveFrontSideAmbientColor;
    stream << structure->_primitiveFrontSideDiffuseColor;
    stream << structure->_primitiveFrontSideSpecularColor;
    stream << structure->_primitiveFrontSideDiffuseIntensity;
    stream << structure->_primitiveFrontSideAmbientIntensity;
    stream << structure->_primitiveFrontSideSpecularIntensity;
    stream << structure->_primitiveFrontSideShininess;

    stream << structure->_primitiveBackSideHDR;
    stream << structure->_primitiveBackSideHDRExposure;
    stream << structure->_primitiveBackSideAmbientColor;
    stream << structure->_primitiveBackSideDiffuseColor;
    stream << structure->_primitiveBackSideSpecularColor;
    stream << structure->_primitiveBackSideDiffuseIntensity;
    stream << structure->_primitiveBackSideAmbientIntensity;
    stream << structure->_primitiveBackSideSpecularIntensity;
    stream << structure->_primitiveBackSideShininess;
  }
  stream << static_cast<typename std::underlying_type<ProbeMolecule>::type>(structure->_frameworkProbeMolecule);

  stream << structure->_minimumGridEnergyValue;

  stream << structure->_atomsTreeController;
  stream << structure->_drawAtoms;

  stream << static_cast<typename std::underlying_type<Structure::RepresentationType>::type>(structure->_atomRepresentationType);
  stream << static_cast<typename std::underlying_type<Structure::RepresentationStyle>::type>(structure->_atomRepresentationStyle);
  stream << structure->_atomForceFieldIdentifier;
  stream << static_cast<typename std::underlying_type<ForceFieldSet::ForceFieldSchemeOrder>::type>(structure->_atomForceFieldOrder);
  stream << structure->_atomColorSchemeIdentifier;
  stream << static_cast<typename std::underlying_type<SKColorSet::ColorSchemeOrder>::type>(structure->_atomColorSchemeOrder);

  stream << static_cast<typename std::underlying_type<RKSelectionStyle>::type>(structure->_atomSelectionStyle);
  stream << structure->_atomSelectionStripesDensity;
  stream << structure->_atomSelectionStripesFrequency;
  stream << structure->_atomSelectionWorleyNoise3DFrequency;
  stream << structure->_atomSelectionWorleyNoise3DJitter;
  stream << structure->_atomSelectionScaling;
  stream << structure->_selectionIntensity;

  stream << structure->_atomHue;
  stream << structure->_atomSaturation;
  stream << structure->_atomValue;
  stream << structure->_atomScaleFactor;

  stream << structure->_atomAmbientOcclusion;
  stream << structure->_atomAmbientOcclusionPatchNumber;
  stream << structure->_atomAmbientOcclusionTextureSize;
  stream << structure->_atomAmbientOcclusionPatchSize;

  stream << structure->_atomHDR;
  stream << structure->_atomHDRExposure;
  stream << structure->_atomSelectionIntensity;

  stream << structure->_atomAmbientColor;
  stream << structure->_atomDiffuseColor;
  stream << structure->_atomSpecularColor;
  stream << structure->_atomAmbientIntensity;
  stream << structure->_atomDiffuseIntensity;
  stream << structure->_atomSpecularIntensity;
  stream << structure->_atomShininess;

  stream << static_cast<typename std::underlying_type<RKTextType>::type>(structure->_atomTextType);
  stream << structure->_atomTextFont;
  QFont font;
  font.fromString(structure->_atomTextFont);
  QFontInfo fontInfo = QFontInfo(font);
  const QString familyName = fontInfo.family();
  const QString memberName = fontInfo.styleName();
  stream << familyName;
  stream << memberName;
  stream << structure->_atomTextScaling;
  stream << structure->_atomTextColor;
  stream << structure->_atomTextGlowColor;
  stream << static_cast<typename std::underlying_type<RKTextStyle>::type>(structure->_atomTextStyle);
  stream << static_cast<typename std::underlying_type<RKTextEffect>::type>(structure->_atomTextEffect);
  stream << static_cast<typename std::underlying_type<RKTextAlignment>::type>(structure->_atomTextAlignment);
  stream << structure->_atomTextOffset;

  stream << structure->_bondSetController;

  stream << structure->_drawBonds;
  stream << structure->_bondScaleFactor;
  stream << static_cast<typename std::underlying_type<RKBondColorMode>::type>(structure->_bondColorMode);

  stream << structure->_bondAmbientColor;
  stream << structure->_bondDiffuseColor;
  stream << structure->_bondSpecularColor;
  stream << structure->_bondAmbientIntensity;
  stream << structure->_bondDiffuseIntensity;
  stream << structure->_bondSpecularIntensity;
  stream << structure->_bondShininess;

  stream << structure->_bondHDR;
  stream << structure->_bondHDRExposure;

  stream << static_cast<typename std::underlying_type<RKSelectionStyle>::type>(structure->_bondSelectionStyle);
  stream << structure->_bondSelectionStripesDensity;
  stream << structure->_bondSelectionStripesFrequency;
  stream << structure->_bondSelectionWorleyNoise3DFrequency;
  stream << structure->_bondSelectionWorleyNoise3DJitter;
  stream << structure->_bondSelectionScaling;
  stream << structure->_bondSelectionIntensity;

  stream << structure->_bondHue;
  stream << structure->_bondSaturation;
  stream << structure->_bondValue;

  stream << structure->_bondAmbientOcclusion;

  if(structure->_versionNumber < 9)
  {
    stream << structure->_drawUnitCell;
    stream << structure->_unitCellScaleFactor;
    stream << structure->_unitCellDiffuseColor;
    stream << structure->_unitCellDiffuseIntensity;

    stream << structure->_localAxes;
  }

  stream << structure->_drawAdsorptionSurface;
  stream << structure->_adsorptionSurfaceOpacity;
  stream << structure->_adsorptionTransparencyThreshold;
  stream << structure->_adsorptionSurfaceIsoValue;
  stream << structure->_adsorptionSurfaceMinimumValue;

  stream << static_cast<typename std::underlying_type<RKEnergySurfaceType>::type>(structure->_adsorptionSurfaceRenderingMethod);
  stream << static_cast<typename std::underlying_type<RKPredefinedVolumeRenderingTransferFunction>::type>(structure->_adsorptionVolumeTransferFunction);
  stream << structure->_adsorptionVolumeStepLength;

  stream << structure->_encompassingPowerOfTwoCubicGridSize;
  stream << qint64(0);

  stream << static_cast<typename std::underlying_type<ProbeMolecule>::type>(structure->_adsorptionSurfaceProbeMolecule);

  stream << structure->_adsorptionSurfaceHue;
  stream << structure->_adsorptionSurfaceSaturation;
  stream << structure->_adsorptionSurfaceValue;

  stream << structure->_adsorptionSurfaceFrontSideHDR;
  stream << structure->_adsorptionSurfaceFrontSideHDRExposure;
  stream << structure->_adsorptionSurfaceFrontSideAmbientColor;
  stream << structure->_adsorptionSurfaceFrontSideDiffuseColor;
  stream << structure->_adsorptionSurfaceFrontSideSpecularColor;
  stream << structure->_adsorptionSurfaceFrontSideAmbientIntensity;
  stream << structure->_adsorptionSurfaceFrontSideDiffuseIntensity;
  stream << structure->_adsorptionSurfaceFrontSideSpecularIntensity;
  stream << structure->_adsorptionSurfaceFrontSideShininess;

  stream << structure->_adsorptionSurfaceBackSideHDR;
  stream << structure->_adsorptionSurfaceBackSideHDRExposure;
  stream << structure->_adsorptionSurfaceBackSideAmbientColor;
  stream << structure->_adsorptionSurfaceBackSideDiffuseColor;
  stream << structure->_adsorptionSurfaceBackSideSpecularColor;
  stream << structure->_adsorptionSurfaceBackSideAmbientIntensity;
  stream << structure->_adsorptionSurfaceBackSideDiffuseIntensity;
  stream << structure->_adsorptionSurfaceBackSideSpecularIntensity;
  stream << structure->_adsorptionSurfaceBackSideShininess;

  stream << static_cast<typename std::underlying_type<Structure::StructureType>::type>(structure->_structureType);
  stream << structure->_structureMaterialType;
  stream << structure->_structureMass;
  stream << structure->_structureDensity;
  stream << structure->_structureHeliumVoidFraction;
  stream << structure->_structureSpecificVolume;
  stream << structure->_structureAccessiblePoreVolume;
  stream << structure->_structureVolumetricNitrogenSurfaceArea;
  stream << structure->_structureGravimetricNitrogenSurfaceArea;
  stream << structure->_structureNumberOfChannelSystems;
  stream << structure->_structureNumberOfInaccessiblePockets;
  stream << structure->_structureDimensionalityOfPoreSystem;
  stream << structure->_structureLargestCavityDiameter;
  stream << structure->_structureRestrictingPoreLimitingDiameter;
  stream << structure->_structureLargestCavityDiameterAlongAViablePath;

  /*
  stream << structure->_authorFirstName;
  stream << structure->_authorMiddleName;
  stream << structure->_authorLastName;
  stream << structure->_authorOrchidID;
  stream << structure->_authorResearcherID;
  stream << structure->_authorAffiliationUniversityName;
  stream << structure->_authorAffiliationFacultyName;
  stream << structure->_authorAffiliationInstituteName;
  stream << structure->_authorAffiliationCityName;
  stream << structure->_authorAffiliationCountryName;

  stream << uint16_t(structure->_creationDate.day());
  stream << uint16_t(structure->_creationDate.month());
  stream << uint32_t(structure->_creationDate.year());
  */

  stream << structure->_creationTemperature;
  stream << static_cast<typename std::underlying_type<Structure::TemperatureScale>::type>(structure->_creationTemperatureScale);
  stream << structure->_creationPressure;
  stream << static_cast<typename std::underlying_type<Structure::PressureScale>::type>(structure->_creationPressureScale);
  stream << static_cast<typename std::underlying_type<Structure::CreationMethod>::type>(structure->_creationMethod);
  stream << static_cast<typename std::underlying_type<Structure::UnitCellRelaxationMethod>::type>(structure->_creationUnitCellRelaxationMethod);
  stream << structure->_creationAtomicPositionsSoftwarePackage;
  stream << static_cast<typename std::underlying_type<Structure::IonsRelaxationAlgorithm>::type>(structure->_creationAtomicPositionsIonsRelaxationAlgorithm);
  stream << static_cast<typename std::underlying_type<Structure::IonsRelaxationCheck>::type>(structure->_creationAtomicPositionsIonsRelaxationCheck);
  stream << structure->_creationAtomicPositionsForcefield;
  stream << structure->_creationAtomicPositionsForcefieldDetails;
  stream << structure->_creationAtomicChargesSoftwarePackage;
  stream << structure->_creationAtomicChargesAlgorithms;
  stream << structure->_creationAtomicChargesForcefield;
  stream << structure->_creationAtomicChargesForcefieldDetails;

  stream << structure->_experimentalMeasurementRadiation;
  stream << structure->_experimentalMeasurementWaveLength;
  stream << structure->_experimentalMeasurementThetaMin;
  stream << structure->_experimentalMeasurementThetaMax;
  stream << structure->_experimentalMeasurementIndexLimitsHmin;
  stream << structure->_experimentalMeasurementIndexLimitsHmax;
  stream << structure->_experimentalMeasurementIndexLimitsKmin;
  stream << structure->_experimentalMeasurementIndexLimitsKmax;
  stream << structure->_experimentalMeasurementIndexLimitsLmin;
  stream << structure->_experimentalMeasurementIndexLimitsLmax;
  stream << structure->_experimentalMeasurementNumberOfSymmetryIndependentReflections;
  stream << structure->_experimentalMeasurementSoftware;
  stream << structure->_experimentalMeasurementRefinementDetails;
  stream << structure->_experimentalMeasurementGoodnessOfFit;
  stream << structure->_experimentalMeasurementRFactorGt;
  stream << structure->_experimentalMeasurementRFactorAll;

  stream << structure->_chemicalFormulaMoiety;
  stream << structure->_chemicalFormulaSum;
  stream << structure->_chemicalNameSystematic;
  stream << structure->_cellFormulaUnitsZ;

  stream << structure->_citationArticleTitle;
  stream << structure->_citationJournalTitle;
  stream << structure->_citationAuthors;
  stream << structure->_citationJournalVolume;
  stream << structure->_citationJournalNumber;
  stream << structure->_citationJournalPageNumbers;
  stream << structure->_citationDOI;
  stream << uint16_t(structure->_citationPublicationDate.day());
  stream << uint16_t(structure->_citationPublicationDate.month());
  stream << uint32_t(structure->_citationPublicationDate.year());
  stream << structure->_citationDatebaseCodes;

  stream << qint64(0x6f6b6182);

  stream << static_cast<typename std::underlying_type<RKEdgeCueing>::type>(structure->_atomEdgeCueing);

  stream << qint64(structure->_blockingPockets.size());
  for (const double4 &pocket : structure->_blockingPockets)
  {
    stream << pocket;
  }
  stream << structure->_drawBlockingPockets;
  stream << structure->_applyBlockingPockets;

  stream << structure->_blockingPocketsFrontSideHDR;
  stream << structure->_blockingPocketsFrontSideHDRExposure;
  stream << structure->_blockingPocketsFrontSideAmbientColor;
  stream << structure->_blockingPocketsFrontSideDiffuseColor;
  stream << structure->_blockingPocketsFrontSideSpecularColor;
  stream << structure->_blockingPocketsFrontSideAmbientIntensity;
  stream << structure->_blockingPocketsFrontSideDiffuseIntensity;
  stream << structure->_blockingPocketsFrontSideSpecularIntensity;
  stream << structure->_blockingPocketsFrontSideShininess;

  stream << structure->_structureVolumetricWellSurfaceArea;
  stream << structure->_structureGravimetricWellSurfaceArea;

  stream << structure->_structureVolumetricGeometricSurfaceArea;
  stream << structure->_structureGravimetricGeometricSurfaceArea;

  stream << structure->_frameworkProbeEpsilon;
  stream << structure->_frameworkProbeSigma;

  stream << structure->_adsorptionSurfaceProbeEpsilon;
  stream << structure->_adsorptionSurfaceProbeSigma;

  stream << structure->_structureVolumetricVanDerWaalsGeometricSurfaceArea;
  stream << structure->_structureGravimetricVanDerWaalsGeometricSurfaceArea;

  stream << structure->_structureWindow1Size;
  stream << structure->_structureWindow2Size;
  stream << structure->_structureWindow3Size;
  stream << qint64(structure->_structureWindow1RingAtoms);
  stream << qint64(structure->_structureWindow2RingAtoms);
  stream << qint64(structure->_structureWindow3RingAtoms);

  stream << static_cast<typename std::underlying_type<ProbeMolecule>::type>(structure->_blockingPocketProbeMolecule);
  stream << structure->_blockingPocketProbeEpsilon;
  stream << structure->_blockingPocketProbeSigma;

  stream << structure->_structureRestrictingPoreLimitingDiameter2;
  stream << structure->_structureLargestCavityDiameterAlongAViablePath2;

  // handle super class
  stream << std::static_pointer_cast<Object>(structure);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Structure> &structure)
{
  uint16_t day,month;
  uint32_t year;
  qint64 versionNumber;
  stream >> versionNumber;

  if(versionNumber > structure->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Structure");
  }

  if(versionNumber < 9)
  {
    stream >> structure->_displayName;
    stream >> structure->_isVisible;

    stream >> structure->_legacySpaceGroup;

    stream >> structure->_cell;
    stream >> structure->_periodic;
    stream >> structure->_origin;
    stream >> structure->_scaling;
    stream >> structure->_orientation;
    stream >> structure->_rotationDelta;
  }

  if(versionNumber < 9)
  {
    if(versionNumber >= 2)
    {
      stream >> structure->_primitiveTransformationMatrix;
      stream >> structure->_primitiveOrientation;
      stream >> structure->_primitiveRotationDelta;

      stream >> structure->_primitiveOpacity;
      stream >> structure->_primitiveIsCapped;
      stream >> structure->_primitiveIsFractional;
      stream >> structure->_primitiveNumberOfSides;
      stream >> structure->_primitiveThickness;

      if(versionNumber >= 6)
      {
        stream >> structure->_primitiveHue;
        stream >> structure->_primitiveSaturation;
        stream >> structure->_primitiveValue;

        qint64 primitiveSelectionStyle;
        stream >> primitiveSelectionStyle;
        structure->_primitiveSelectionStyle = RKSelectionStyle(primitiveSelectionStyle);
        stream >> structure->_primitiveSelectionStripesDensity;
        stream >> structure->_primitiveSelectionStripesFrequency;
        stream >> structure->_primitiveSelectionWorleyNoise3DFrequency;
        stream >> structure->_primitiveSelectionWorleyNoise3DJitter;
        stream >> structure->_primitiveSelectionScaling;
        stream >> structure->_primitiveSelectionIntensity;
      }

      stream >> structure->_primitiveFrontSideHDR;
      stream >> structure->_primitiveFrontSideHDRExposure;
      stream >> structure->_primitiveFrontSideAmbientColor;
      stream >> structure->_primitiveFrontSideDiffuseColor;
      stream >> structure->_primitiveFrontSideSpecularColor;
      stream >> structure->_primitiveFrontSideDiffuseIntensity;
      stream >> structure->_primitiveFrontSideAmbientIntensity;
      stream >> structure->_primitiveFrontSideSpecularIntensity;
      stream >> structure->_primitiveFrontSideShininess;

      stream >> structure->_primitiveBackSideHDR;
      stream >> structure->_primitiveBackSideHDRExposure;
      stream >> structure->_primitiveBackSideAmbientColor;
      stream >> structure->_primitiveBackSideDiffuseColor;
      stream >> structure->_primitiveBackSideSpecularColor;
      stream >> structure->_primitiveBackSideDiffuseIntensity;
      stream >> structure->_primitiveBackSideAmbientIntensity;
      stream >> structure->_primitiveBackSideSpecularIntensity;
      stream >> structure->_primitiveBackSideShininess;
    }
  }

  if(versionNumber >= 3)
  {
    qint64 probeMolecule;
    stream >> probeMolecule;
    structure->_frameworkProbeMolecule = ProbeMolecule(probeMolecule);
  }

  stream >> structure->_minimumGridEnergyValue;

  stream >> structure->_atomsTreeController;
  stream >> structure->_drawAtoms;

  qint64 atomRepresentationType;
  stream >> atomRepresentationType;
  structure->_atomRepresentationType = Structure::RepresentationType(atomRepresentationType);
  qint64 atomRepresentationStyle;
  stream >> atomRepresentationStyle;
  structure->_atomRepresentationStyle = Structure::RepresentationStyle(atomRepresentationStyle);
  stream >> structure->_atomForceFieldIdentifier;
  qint64 atomForceFieldOrder;
  stream >> atomForceFieldOrder;
  structure->_atomForceFieldOrder = ForceFieldSet::ForceFieldSchemeOrder(atomForceFieldOrder);
  stream >> structure->_atomColorSchemeIdentifier;
  qint64 atomColorSchemeOrder;
  stream >> atomColorSchemeOrder;
  structure->_atomColorSchemeOrder = SKColorSet::ColorSchemeOrder(atomColorSchemeOrder);

  qint64 atomSelectionStyle;
  stream >> atomSelectionStyle;
  structure->_atomSelectionStyle = RKSelectionStyle(atomSelectionStyle);
  stream >> structure->_atomSelectionStripesDensity;
  stream >> structure->_atomSelectionStripesFrequency;
  stream >> structure->_atomSelectionWorleyNoise3DFrequency;
  stream >> structure->_atomSelectionWorleyNoise3DJitter;
  stream >> structure->_atomSelectionScaling;
  stream >> structure->_selectionIntensity;

  stream >> structure->_atomHue;
  stream >> structure->_atomSaturation;
  stream >> structure->_atomValue;
  stream >> structure->_atomScaleFactor;

  stream >> structure->_atomAmbientOcclusion;
  stream >> structure->_atomAmbientOcclusionPatchNumber;
  stream >> structure->_atomAmbientOcclusionTextureSize;
  stream >> structure->_atomAmbientOcclusionPatchSize;

  stream >> structure->_atomHDR;
  stream >> structure->_atomHDRExposure;
  stream >> structure->_atomSelectionIntensity;

  stream >> structure->_atomAmbientColor;
  stream >> structure->_atomDiffuseColor;
  stream >> structure->_atomSpecularColor;
  stream >> structure->_atomAmbientIntensity;
  stream >> structure->_atomDiffuseIntensity;
  stream >> structure->_atomSpecularIntensity;
  stream >> structure->_atomShininess;


  qint64 atomTextType;
  stream >> atomTextType;
  structure->_atomTextType = RKTextType(atomTextType);
  stream >> structure->_atomTextFont;

  QFont font;
  font.fromString(structure->_atomTextFont);
  if(!font.exactMatch())
  {
#if defined (Q_OS_OSX)
      structure->_atomTextFont = QFont("Helvetica").toString();
#elif defined(Q_OS_WIN)
      structure->_atomTextFont = QFont("Arial").toString();
#elif defined(Q_OS_LINUX)
      structure->_atomTextFont = QFont("Arial").toString();
#endif
  }

  if(versionNumber >= 7)
  {
    QString fontFamilyName{};
    QString fontStyleName{};
    stream >> fontFamilyName;
    stream >> fontStyleName;

#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
    QFontDatabase database;
    if (!database.families().contains(fontFamilyName))
#else
    if (!QFontDatabase::families().contains(fontFamilyName))
#endif
    {
#if defined (Q_OS_OSX)
        fontFamilyName = "Helvetica";
#elif defined(Q_OS_WIN)
        fontFamilyName = "Arial";
#elif defined(Q_OS_LINUX)
        fontFamilyName = "Arial";
#endif
    }

    QFont fontFromfamily = QFont(fontFamilyName);
    fontFromfamily.setStyleName(fontStyleName);
    structure->_atomTextFont = fontFromfamily.toString();
  }
  stream >> structure->_atomTextScaling;
  stream >> structure->_atomTextColor;
  stream >> structure->_atomTextGlowColor;
  qint64 atomTextStyle;
  stream >> atomTextStyle;
  structure->_atomTextStyle = RKTextStyle(atomTextStyle);
  qint64 atomTextEffect;
  stream >> atomTextEffect;
  structure->_atomTextEffect = RKTextEffect(atomTextEffect);
  qint64 atomTextAlignment;
  stream >> atomTextAlignment;
  structure->_atomTextAlignment = RKTextAlignment(atomTextAlignment);
  stream >> structure->_atomTextOffset;

  stream >> structure->_bondSetController;

  stream >> structure->_drawBonds;
  stream >> structure->_bondScaleFactor;
  qint64 bondColorMode;
  stream >> bondColorMode;
  structure->_bondColorMode = RKBondColorMode(bondColorMode);

  stream >> structure->_bondAmbientColor;
  stream >> structure->_bondDiffuseColor;
  stream >> structure->_bondSpecularColor;
  stream >> structure->_bondAmbientIntensity;
  stream >> structure->_bondDiffuseIntensity;
  stream >> structure->_bondSpecularIntensity;
  stream >> structure->_bondShininess;

  stream >> structure->_bondHDR;
  stream >> structure->_bondHDRExposure;

  if(versionNumber >= 5) // introduced in version 5
  {
    qint64 bondSelectionStyle;
    stream >> bondSelectionStyle;
    structure->_bondSelectionStyle = RKSelectionStyle(bondSelectionStyle);
    stream >> structure->_bondSelectionStripesDensity;
    stream >> structure->_bondSelectionStripesFrequency;
    stream >> structure->_bondSelectionWorleyNoise3DFrequency;
    stream >> structure->_bondSelectionWorleyNoise3DJitter;
    stream >> structure->_bondSelectionScaling;
  }
  stream >> structure->_bondSelectionIntensity;

  stream >> structure->_bondHue;
  stream >> structure->_bondSaturation;
  stream >> structure->_bondValue;

  stream >> structure->_bondAmbientOcclusion;

  if(versionNumber < 9)
  {
    stream >> structure->_drawUnitCell;
    stream >> structure->_unitCellScaleFactor;
    stream >> structure->_unitCellDiffuseColor;
    stream >> structure->_unitCellDiffuseIntensity;

    if(versionNumber >= 8) // introduced in version 8
    {
      stream >> structure->_localAxes;
    }
  }

  stream >> structure->_drawAdsorptionSurface;
  stream >> structure->_adsorptionSurfaceOpacity;
  if(versionNumber >= 10) // introduced in version 10
  {
    stream >> structure->_adsorptionTransparencyThreshold;
  }

  stream >> structure->_adsorptionSurfaceIsoValue;
  stream >> structure->_adsorptionSurfaceMinimumValue;

  if(versionNumber >= 9) // introduced in version 9
  {
    qint64 adsorptionSurfaceRenderingMethod;
    stream >> adsorptionSurfaceRenderingMethod;
    structure->_adsorptionSurfaceRenderingMethod = RKEnergySurfaceType(adsorptionSurfaceRenderingMethod);
    qint64 adsorptionVolumeTransferFunction;
    stream >> adsorptionVolumeTransferFunction;
    structure->_adsorptionVolumeTransferFunction = RKPredefinedVolumeRenderingTransferFunction(adsorptionVolumeTransferFunction);
    stream >> structure->_adsorptionVolumeStepLength;
  }

  if(versionNumber <= 9)
  {
    qint64 temp;
    stream >> temp;
  }
  else
  {
    stream >> structure->_encompassingPowerOfTwoCubicGridSize;
  }
  stream >> structure->_adsorptionSurfaceNumberOfTriangles;

  qint64 adsorptionSurfaceProbeMolecule;
  stream >> adsorptionSurfaceProbeMolecule;
  structure->_adsorptionSurfaceProbeMolecule = ProbeMolecule(adsorptionSurfaceProbeMolecule);

  if(versionNumber >= 6) // introduced in version 6
  {
    stream >> structure->_adsorptionSurfaceHue;
    stream >> structure->_adsorptionSurfaceSaturation;
    stream >> structure->_adsorptionSurfaceValue;
  }

  stream >> structure->_adsorptionSurfaceFrontSideHDR;
  stream >> structure->_adsorptionSurfaceFrontSideHDRExposure;
  stream >> structure->_adsorptionSurfaceFrontSideAmbientColor;
  stream >> structure->_adsorptionSurfaceFrontSideDiffuseColor;
  stream >> structure->_adsorptionSurfaceFrontSideSpecularColor;
  stream >> structure->_adsorptionSurfaceFrontSideAmbientIntensity;
  stream >> structure->_adsorptionSurfaceFrontSideDiffuseIntensity;
  stream >> structure->_adsorptionSurfaceFrontSideSpecularIntensity;
  stream >> structure->_adsorptionSurfaceFrontSideShininess;

  stream >> structure->_adsorptionSurfaceBackSideHDR;
  stream >> structure->_adsorptionSurfaceBackSideHDRExposure;
  stream >> structure->_adsorptionSurfaceBackSideAmbientColor;
  stream >> structure->_adsorptionSurfaceBackSideDiffuseColor;
  stream >> structure->_adsorptionSurfaceBackSideSpecularColor;
  stream >> structure->_adsorptionSurfaceBackSideAmbientIntensity;
  stream >> structure->_adsorptionSurfaceBackSideDiffuseIntensity;
  stream >> structure->_adsorptionSurfaceBackSideSpecularIntensity;
  stream >> structure->_adsorptionSurfaceBackSideShininess;

  qint64 structureType;
  stream >> structureType;
  structure->_structureType = Structure::StructureType(structureType);
  QString decodedMaterialType;
  stream >> decodedMaterialType;
  if (std::optional<SKStructure::MaterialType> canonicalType = SKStructure::materialTypeFromDisplayName(decodedMaterialType))
  {
    structure->_structureMaterialType = SKStructure::materialTypeDisplayName(*canonicalType);
  }
  else
  {
    structure->_structureMaterialType = decodedMaterialType;
  }
  stream >> structure->_structureMass;
  stream >> structure->_structureDensity;
  stream >> structure->_structureHeliumVoidFraction;
  stream >> structure->_structureSpecificVolume;
  stream >> structure->_structureAccessiblePoreVolume;
  stream >> structure->_structureVolumetricNitrogenSurfaceArea;
  stream >> structure->_structureGravimetricNitrogenSurfaceArea;
  stream >> structure->_structureNumberOfChannelSystems;
  stream >> structure->_structureNumberOfInaccessiblePockets;
  stream >> structure->_structureDimensionalityOfPoreSystem;
  stream >> structure->_structureLargestCavityDiameter;
  stream >> structure->_structureRestrictingPoreLimitingDiameter;
  stream >> structure->_structureLargestCavityDiameterAlongAViablePath;


  if(versionNumber < 9)
  {
    stream >> structure->_authorFirstName;
    stream >> structure->_authorMiddleName;
    stream >> structure->_authorLastName;
    stream >> structure->_authorOrchidID;
    stream >> structure->_authorResearcherID;
    stream >> structure->_authorAffiliationUniversityName;
    stream >> structure->_authorAffiliationFacultyName;
    stream >> structure->_authorAffiliationInstituteName;
    stream >> structure->_authorAffiliationCityName;
    stream >> structure->_authorAffiliationCountryName;

    stream >> day;
    stream >> month;
    stream >> year;
    structure->_creationDate = QDate(int(year),int(month),int(day));
  }

  stream >> structure->_creationTemperature;
  qint64 creationTemperatureScale;
  stream >> creationTemperatureScale;
  structure->_creationTemperatureScale = Structure::TemperatureScale(creationTemperatureScale);
  stream >> structure->_creationPressure;
  qint64 creationPressureScale;
  stream >> creationPressureScale;
  structure->_creationPressureScale = Structure::PressureScale(creationPressureScale);
  qint64 creationMethod;
  stream >> creationMethod;
  structure->_creationMethod = Structure::CreationMethod(creationMethod);
  qint64 creationUnitCellRelaxationMethod;
  stream >> creationUnitCellRelaxationMethod;
  structure->_creationUnitCellRelaxationMethod = Structure::UnitCellRelaxationMethod(creationUnitCellRelaxationMethod);
  stream >> structure->_creationAtomicPositionsSoftwarePackage;
  qint64 creationAtomicPositionsIonsRelaxationAlgorithm;
  stream >> creationAtomicPositionsIonsRelaxationAlgorithm;
  structure->_creationAtomicPositionsIonsRelaxationAlgorithm = Structure::IonsRelaxationAlgorithm(creationAtomicPositionsIonsRelaxationAlgorithm);
  qint64 creationAtomicPositionsIonsRelaxationCheck;
  stream >> creationAtomicPositionsIonsRelaxationCheck;
  structure->_creationAtomicPositionsIonsRelaxationCheck = Structure::IonsRelaxationCheck(creationAtomicPositionsIonsRelaxationCheck);
  stream >> structure->_creationAtomicPositionsForcefield;
  stream >> structure->_creationAtomicPositionsForcefieldDetails;
  stream >> structure->_creationAtomicChargesSoftwarePackage;
  stream >> structure->_creationAtomicChargesAlgorithms;
  stream >> structure->_creationAtomicChargesForcefield;
  stream >> structure->_creationAtomicChargesForcefieldDetails;

  stream >> structure->_experimentalMeasurementRadiation;
  stream >> structure->_experimentalMeasurementWaveLength;
  stream >> structure->_experimentalMeasurementThetaMin;
  stream >> structure->_experimentalMeasurementThetaMax;
  stream >> structure->_experimentalMeasurementIndexLimitsHmin;
  stream >> structure->_experimentalMeasurementIndexLimitsHmax;
  stream >> structure->_experimentalMeasurementIndexLimitsKmin;
  stream >> structure->_experimentalMeasurementIndexLimitsKmax;
  stream >> structure->_experimentalMeasurementIndexLimitsLmin;
  stream >> structure->_experimentalMeasurementIndexLimitsLmax;
  stream >> structure->_experimentalMeasurementNumberOfSymmetryIndependentReflections;
  stream >> structure->_experimentalMeasurementSoftware;
  stream >> structure->_experimentalMeasurementRefinementDetails;
  stream >> structure->_experimentalMeasurementGoodnessOfFit;
  stream >> structure->_experimentalMeasurementRFactorGt;
  stream >> structure->_experimentalMeasurementRFactorAll;

  stream >> structure->_chemicalFormulaMoiety;
  stream >> structure->_chemicalFormulaSum;
  stream >> structure->_chemicalNameSystematic;
  stream >> structure->_cellFormulaUnitsZ;

  stream >> structure->_citationArticleTitle;
  stream >> structure->_citationJournalTitle;
  stream >> structure->_citationAuthors;
  stream >> structure->_citationJournalVolume;
  stream >> structure->_citationJournalNumber;
  stream >> structure->_citationJournalPageNumbers;
  stream >> structure->_citationDOI;
  stream >> day;
  stream >> month;
  stream >> year;
  structure->_citationPublicationDate = QDate(int(year),int(month),int(day));
  stream >> structure->_citationDatebaseCodes;

  if(versionNumber >= 9)
  {
    qint64 magicNumber;
    stream >> magicNumber;
    if(magicNumber != qint64(0x6f6b6182))
    {
      throw InvalidArchiveVersionException(__FILE__, __LINE__, "Structure invalid magic-number");
    }

    if (versionNumber >= 11)
    {
      qint64 atomEdgeCueing;
      stream >> atomEdgeCueing;
      structure->_atomEdgeCueing = RKEdgeCueing(atomEdgeCueing);
    }

    if (versionNumber >= 12)
    {
      qint64 numberOfBlockingPockets;
      stream >> numberOfBlockingPockets;
      structure->_blockingPockets.clear();
      structure->_blockingPockets.reserve(std::max<qint64>(0, numberOfBlockingPockets));
      for (qint64 i = 0; i < std::max<qint64>(0, numberOfBlockingPockets); ++i)
      {
        double4 pocket;
        stream >> pocket;
        structure->_blockingPockets.push_back(pocket);
      }
      stream >> structure->_drawBlockingPockets;
      stream >> structure->_applyBlockingPockets;

      stream >> structure->_blockingPocketsFrontSideHDR;
      stream >> structure->_blockingPocketsFrontSideHDRExposure;
      stream >> structure->_blockingPocketsFrontSideAmbientColor;
      stream >> structure->_blockingPocketsFrontSideDiffuseColor;
      stream >> structure->_blockingPocketsFrontSideSpecularColor;
      stream >> structure->_blockingPocketsFrontSideAmbientIntensity;
      stream >> structure->_blockingPocketsFrontSideDiffuseIntensity;
      stream >> structure->_blockingPocketsFrontSideSpecularIntensity;
      stream >> structure->_blockingPocketsFrontSideShininess;
    }

    if (versionNumber >= 13)
    {
      stream >> structure->_structureVolumetricWellSurfaceArea;
      stream >> structure->_structureGravimetricWellSurfaceArea;
    }

    if (versionNumber >= 14)
    {
      stream >> structure->_structureVolumetricGeometricSurfaceArea;
      stream >> structure->_structureGravimetricGeometricSurfaceArea;
    }

    if (versionNumber >= 15)
    {
      stream >> structure->_frameworkProbeEpsilon;
      stream >> structure->_frameworkProbeSigma;
    }

    if (versionNumber >= 16)
    {
      stream >> structure->_adsorptionSurfaceProbeEpsilon;
      stream >> structure->_adsorptionSurfaceProbeSigma;
    }

    if (versionNumber >= 17)
    {
      stream >> structure->_structureVolumetricVanDerWaalsGeometricSurfaceArea;
      stream >> structure->_structureGravimetricVanDerWaalsGeometricSurfaceArea;
    }

    if (versionNumber >= 18)
    {
      stream >> structure->_structureWindow1Size;
      stream >> structure->_structureWindow2Size;
      stream >> structure->_structureWindow3Size;
    }

    if (versionNumber >= 19)
    {
      qint64 ring1, ring2, ring3;
      stream >> ring1;
      stream >> ring2;
      stream >> ring3;
      structure->_structureWindow1RingAtoms = int(ring1);
      structure->_structureWindow2RingAtoms = int(ring2);
      structure->_structureWindow3RingAtoms = int(ring3);
    }

    if (versionNumber >= 20)
    {
      qint64 blockingPocketProbeMolecule;
      stream >> blockingPocketProbeMolecule;
      structure->_blockingPocketProbeMolecule = ProbeMolecule(blockingPocketProbeMolecule);
    }

    if (versionNumber >= 21)
    {
      stream >> structure->_blockingPocketProbeEpsilon;
      stream >> structure->_blockingPocketProbeSigma;
    }

    if (versionNumber >= 22)
    {
      stream >> structure->_structureRestrictingPoreLimitingDiameter2;
      stream >> structure->_structureLargestCavityDiameterAlongAViablePath2;
    }

    std::shared_ptr<Object> object = std::static_pointer_cast<Object>(structure);
    stream >> object;
  }

  if (versionNumber < 15)
  {
    if (std::optional<double2> parameters = probeNamedParameters(structure->_frameworkProbeMolecule))
    {
      structure->_frameworkProbeEpsilon = parameters->x;
      structure->_frameworkProbeSigma = parameters->y;
    }
  }

  if (versionNumber < 16)
  {
    if (std::optional<double2> parameters = probeNamedParameters(structure->_adsorptionSurfaceProbeMolecule))
    {
      structure->_adsorptionSurfaceProbeEpsilon = parameters->x;
      structure->_adsorptionSurfaceProbeSigma = parameters->y;
    }
  }

  if (versionNumber < 21)
  {
    if (std::optional<double2> parameters = probeNamedParameters(structure->_blockingPocketProbeMolecule))
    {
      structure->_blockingPocketProbeEpsilon = parameters->x;
      structure->_blockingPocketProbeSigma = parameters->y;
    }
  }

  RKEdgeCueing archivedAtomEdgeCueing = structure->_atomEdgeCueing;
  structure->setRepresentationStyle(structure->atomRepresentationStyle());
  if (versionNumber >= 11)
  {
    structure->_atomEdgeCueing = archivedAtomEdgeCueing;
    if (structure->_atomRepresentationStyle == Structure::RepresentationStyle::fancy &&
        archivedAtomEdgeCueing == RKEdgeCueing::contoursAndHalos)
    {
      structure->_atomRepresentationStyle = Structure::RepresentationStyle::quteMol;
    }
  }

  if(versionNumber <= 4)
  {
    structure->expandSymmetry();
    structure->computeBonds();
  }

  structure->reComputeBoundingBox();

  structure->atomsTreeController()->setTags();
  structure->bondSetController()->setTags();

  return stream;
}

FrameConsumer::~FrameConsumer()
{
  // Compulsory virtual destructor definition
}
