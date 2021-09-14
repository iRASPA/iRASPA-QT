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
#include <iostream>
#include <unordered_set>
#include <symmetrykit.h>
#include <simulationkit.h>
#include <QFontInfo>
#include "skatomtreecontroller.h"
#include "skbondsetcontroller.h"

Structure::Structure(): _atomsTreeController(std::make_shared<SKAtomTreeController>()),
                        _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _spaceGroup(1), _cell(std::make_shared<SKCell>())
{
}

Structure::Structure(std::shared_ptr<SKAtomTreeController> atomTreeController): _atomsTreeController(atomTreeController),
                     _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _spaceGroup(1), _cell(std::make_shared<SKCell>())
{
}

Structure::Structure(std::shared_ptr<SKStructure> structure): _atomsTreeController(std::make_shared<SKAtomTreeController>()),
  _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _spaceGroup(1), _cell(structure->cell)
{
  if(structure->spaceGroupHallNumber)
  {
    this->_spaceGroup = *(structure->spaceGroupHallNumber);
  }

  for(std::shared_ptr<SKAsymmetricAtom> atom: structure->atoms)
  {
    std::shared_ptr<SKAtomTreeNode> node = std::make_shared<SKAtomTreeNode>(atom);
    _atomsTreeController->appendToRootnodes(node);
  }
}

// shallow copy, atoms/bonds are empty, spacegroup no symmetry
Structure::Structure(const Structure &structure): _atomsTreeController(std::make_shared<SKAtomTreeController>()),
  _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _spaceGroup(1), _cell(std::make_shared<SKCell>())
{
  qDebug() << "Base-class Copy constructor";
  _displayName = structure._displayName;

  _origin = structure._origin;
  _scaling = structure._scaling;
  _orientation = structure._orientation;
  _rotationDelta = structure._rotationDelta;
  _periodic = structure._periodic;
  _isVisible = structure._isVisible;
  _cell = std::make_shared<SKCell>(*structure._cell);
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
  _structureNumberOfChannelSystems = structure._structureNumberOfChannelSystems;
  _structureNumberOfInaccessiblePockets = structure._structureNumberOfInaccessiblePockets;
  _structureDimensionalityOfPoreSystem = structure._structureDimensionalityOfPoreSystem;
  _structureLargestCavityDiameter = structure._structureLargestCavityDiameter;
  _structureRestrictingPoreLimitingDiameter = structure._structureRestrictingPoreLimitingDiameter;
  _structureLargestCavityDiameterAlongAViablePath = structure._structureLargestCavityDiameterAlongAViablePath;

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

  // unit cell
  _drawUnitCell = structure._drawUnitCell;
  _unitCellScaleFactor = structure._unitCellScaleFactor;
  _unitCellDiffuseColor = structure._unitCellDiffuseColor;
  _unitCellDiffuseIntensity = structure._unitCellDiffuseIntensity;

  // adsorption surface
  _frameworkProbeMolecule = structure._frameworkProbeMolecule;

  _drawAdsorptionSurface = structure._drawAdsorptionSurface;
  _adsorptionSurfaceOpacity = structure._adsorptionSurfaceOpacity;
  _adsorptionSurfaceIsoValue = structure._adsorptionSurfaceIsoValue;

  _adsorptionSurfaceSize = structure._adsorptionSurfaceSize;
  _adsorptionSurfaceNumberOfTriangles = structure._adsorptionSurfaceNumberOfTriangles;

  _adsorptionSurfaceProbeMolecule = structure._adsorptionSurfaceProbeMolecule;

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

  /*
  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << structure._atomsTreeController;

  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> _atomsTreeController;

  setRepresentationStyle(_atomRepresentationStyle);

  reComputeBoundingBox();

  _atomsTreeController->setTags();
  _bondSetController->setTags();
  */
}

Structure::Structure(std::shared_ptr<Structure> clone): _atomsTreeController(std::make_shared<SKAtomTreeController>()),
   _bondSetController(std::make_shared<SKBondSetController>(_atomsTreeController)), _spaceGroup(1), _cell(std::make_shared<SKCell>())
{
  qDebug() << "Copy constructor Structure";
  _displayName = clone->_displayName;

  _origin = clone->_origin;
  _scaling = clone->_scaling;
  _orientation = clone->_orientation;
  _rotationDelta = clone->_rotationDelta;
  _periodic = clone->_periodic;
  _isVisible = clone->_isVisible;
  _cell = std::make_shared<SKCell>(*clone->_cell);
  _minimumGridEnergyValue = clone->_minimumGridEnergyValue;
  _spaceGroup = clone->_spaceGroup;

  _selectionCOMTranslation = clone->_selectionCOMTranslation;
  _selectionRotationIndex = clone->_selectionRotationIndex;
  _selectionBodyFixedBasis = clone->_selectionBodyFixedBasis;

  _structureType = clone->_structureType;
  _structureMaterialType = clone->_structureMaterialType;
  _structureMass = clone->_structureMass;
  _structureDensity = clone->_structureDensity;
  _structureHeliumVoidFraction = clone->_structureHeliumVoidFraction;
  _structureSpecificVolume = clone->_structureSpecificVolume;
  _structureAccessiblePoreVolume = clone->_structureAccessiblePoreVolume;
  _structureVolumetricNitrogenSurfaceArea = clone->_structureVolumetricNitrogenSurfaceArea;
  _structureGravimetricNitrogenSurfaceArea = clone->_structureGravimetricNitrogenSurfaceArea;
  _structureNumberOfChannelSystems = clone->_structureNumberOfChannelSystems;
  _structureNumberOfInaccessiblePockets = clone->_structureNumberOfInaccessiblePockets;
  _structureDimensionalityOfPoreSystem = clone->_structureDimensionalityOfPoreSystem;
  _structureLargestCavityDiameter = clone->_structureLargestCavityDiameter;
  _structureRestrictingPoreLimitingDiameter = clone->_structureRestrictingPoreLimitingDiameter;
  _structureLargestCavityDiameterAlongAViablePath = clone->_structureLargestCavityDiameterAlongAViablePath;

  _authorFirstName = clone->_authorFirstName;
  _authorMiddleName = clone->_authorMiddleName;
  _authorLastName = clone->_authorLastName;
  _authorOrchidID = clone->_authorOrchidID;
  _authorResearcherID = clone->_authorResearcherID;
  _authorAffiliationUniversityName = clone->_authorAffiliationUniversityName;
  _authorAffiliationFacultyName = clone->_authorAffiliationFacultyName;
  _authorAffiliationInstituteName = clone->_authorAffiliationInstituteName;
  _authorAffiliationCityName = clone->_authorAffiliationCityName;
  _authorAffiliationCountryName = clone->_authorAffiliationCountryName;

  // primitive properties
  _primitiveTransformationMatrix = clone->_primitiveTransformationMatrix;
  _primitiveOrientation = clone->_primitiveOrientation;
  _primitiveRotationDelta = clone->_primitiveRotationDelta;

  _primitiveOpacity = clone->_primitiveOpacity;
  _primitiveIsCapped = clone->_primitiveIsCapped;
  _primitiveIsFractional = clone->_primitiveIsFractional;
  _primitiveNumberOfSides = clone->_primitiveNumberOfSides;
  _primitiveThickness = clone->_primitiveThickness;

  _primitiveFrontSideHDR = clone->_primitiveFrontSideHDR;
  _primitiveFrontSideHDRExposure = clone->_primitiveFrontSideHDRExposure;
  _primitiveFrontSideAmbientColor = clone->_primitiveFrontSideAmbientColor;
  _primitiveFrontSideDiffuseColor = clone->_primitiveFrontSideDiffuseColor;
  _primitiveFrontSideSpecularColor = clone->_primitiveFrontSideSpecularColor;
  _primitiveFrontSideAmbientIntensity = clone->_primitiveFrontSideAmbientIntensity;
  _primitiveFrontSideDiffuseIntensity = clone->_primitiveFrontSideDiffuseIntensity;
  _primitiveFrontSideSpecularIntensity = clone->_primitiveFrontSideSpecularIntensity;
  _primitiveFrontSideShininess = clone->_primitiveFrontSideShininess;

  _primitiveBackSideHDR = clone->_primitiveBackSideHDR;
  _primitiveBackSideHDRExposure = clone->_primitiveBackSideHDRExposure;
  _primitiveBackSideAmbientColor = clone->_primitiveBackSideAmbientColor;
  _primitiveBackSideDiffuseColor = clone->_primitiveBackSideDiffuseColor;
  _primitiveBackSideSpecularColor = clone->_primitiveBackSideSpecularColor;
  _primitiveBackSideAmbientIntensity = clone->_primitiveBackSideAmbientIntensity;
  _primitiveBackSideDiffuseIntensity = clone->_primitiveBackSideDiffuseIntensity;
  _primitiveBackSideSpecularIntensity = clone->_primitiveBackSideSpecularIntensity;
  _primitiveBackSideShininess = clone->_primitiveBackSideShininess;

  // atoms
  _drawAtoms = clone->_drawAtoms;

  _atomRepresentationType = clone->_atomRepresentationType;
  _atomRepresentationStyle = clone->_atomRepresentationStyle;
  _atomForceFieldIdentifier = clone->_atomForceFieldIdentifier;
  _atomForceFieldOrder = clone->_atomForceFieldOrder;
  _atomColorSchemeIdentifier = clone->_atomColorSchemeIdentifier;
  _atomColorSchemeOrder = clone->_atomColorSchemeOrder;

  _atomSelectionStyle = clone->_atomSelectionStyle;
  _atomSelectionStripesDensity = clone->_atomSelectionStripesDensity;
  _atomSelectionStripesFrequency = clone->_atomSelectionStripesFrequency;
  _atomSelectionWorleyNoise3DFrequency = clone->_atomSelectionWorleyNoise3DFrequency;
  _atomSelectionWorleyNoise3DJitter = clone->_atomSelectionWorleyNoise3DJitter;
  _atomSelectionScaling = clone->_atomSelectionScaling;
  _atomSelectionIntensity = clone->_atomSelectionIntensity;

  _atomHue = clone->_atomHue;
  _atomSaturation = clone->_atomSaturation;
  _atomValue = clone->_atomValue;
  _atomScaleFactor = clone->_atomScaleFactor;

  _atomAmbientOcclusion = clone->_atomAmbientOcclusion;
  _atomAmbientOcclusionPatchNumber = clone->_atomAmbientOcclusionPatchNumber;
  _atomAmbientOcclusionTextureSize = clone->_atomAmbientOcclusionTextureSize;
  _atomAmbientOcclusionPatchSize = clone->_atomAmbientOcclusionPatchSize;
  //_atomCacheAmbientOcclusionTexture = clone->_atomCacheAmbientOcclusionTexture;

  _atomHDR = clone->_atomHDR;
  _atomHDRExposure = clone->_atomHDRExposure;
  _atomSelectionIntensity = clone->_atomSelectionIntensity;

  _atomAmbientColor = clone->_atomAmbientColor;
  _atomDiffuseColor = clone->_atomDiffuseColor;
  _atomSpecularColor = clone->_atomSpecularColor;
  _atomAmbientIntensity = clone->_atomAmbientIntensity;
  _atomDiffuseIntensity = clone->_atomDiffuseIntensity;
  _atomSpecularIntensity = clone->_atomSpecularIntensity;
  _atomShininess = clone->_atomShininess;

  // bonds
  _drawBonds = clone->_drawBonds;

  _bondScaleFactor = clone->_bondScaleFactor;
  _bondColorMode = clone->_bondColorMode;

  _bondAmbientColor = clone->_bondAmbientColor;
  _bondDiffuseColor = clone->_bondDiffuseColor;
  _bondSpecularColor = clone->_bondSpecularColor;
  _bondAmbientIntensity = clone->_bondAmbientIntensity;
  _bondDiffuseIntensity = clone->_bondDiffuseIntensity;
  _bondSpecularIntensity = clone->_bondSpecularIntensity;
  _bondShininess = clone->_bondShininess;

  _bondHDR = clone->_bondHDR;
  _bondHDRExposure = clone->_bondHDRExposure;
  _bondSelectionIntensity = clone->_bondSelectionIntensity;

  _bondHue = clone->_bondHue;
  _bondSaturation = clone->_bondSaturation;
  _bondValue = clone->_bondValue;

  _bondAmbientOcclusion = clone->_bondAmbientOcclusion;

  // text properties
  _atomTextType = clone->_atomTextType;
  _atomTextFont = clone->_atomTextFont;
  _atomTextScaling = clone->_atomTextScaling;
  _atomTextColor = clone->_atomTextColor;
  _atomTextGlowColor = clone->_atomTextGlowColor;
  _atomTextStyle = clone->_atomTextStyle;
  _atomTextEffect = clone->_atomTextEffect;
  _atomTextAlignment = clone->_atomTextAlignment;
  _atomTextOffset = clone->_atomTextOffset;

  // unit cell
  _drawUnitCell = clone->_drawUnitCell;
  _unitCellScaleFactor = clone->_unitCellScaleFactor;
  _unitCellDiffuseColor = clone->_unitCellDiffuseColor;
  _unitCellDiffuseIntensity = clone->_unitCellDiffuseIntensity;

  // adsorption surface
  _frameworkProbeMolecule = clone->_frameworkProbeMolecule;

  _drawAdsorptionSurface = clone->_drawAdsorptionSurface;
  _adsorptionSurfaceOpacity = clone->_adsorptionSurfaceOpacity;
  _adsorptionSurfaceIsoValue = clone->_adsorptionSurfaceIsoValue;

  _adsorptionSurfaceSize = clone->_adsorptionSurfaceSize;
  _adsorptionSurfaceNumberOfTriangles = clone->_adsorptionSurfaceNumberOfTriangles;

  _adsorptionSurfaceProbeMolecule = clone->_adsorptionSurfaceProbeMolecule;

  _adsorptionSurfaceHue = clone->_adsorptionSurfaceHue;
  _adsorptionSurfaceSaturation = clone->_adsorptionSurfaceSaturation;
  _adsorptionSurfaceValue = clone->_adsorptionSurfaceValue;

  _adsorptionSurfaceFrontSideHDR = clone->_adsorptionSurfaceFrontSideHDR;
  _adsorptionSurfaceFrontSideHDRExposure = clone->_adsorptionSurfaceFrontSideHDRExposure;
  _adsorptionSurfaceFrontSideAmbientColor = clone->_adsorptionSurfaceFrontSideAmbientColor;
  _adsorptionSurfaceFrontSideDiffuseColor = clone->_adsorptionSurfaceFrontSideDiffuseColor;
  _adsorptionSurfaceFrontSideSpecularColor = clone->_adsorptionSurfaceFrontSideSpecularColor;
  _adsorptionSurfaceFrontSideDiffuseIntensity = clone->_adsorptionSurfaceFrontSideDiffuseIntensity;
  _adsorptionSurfaceFrontSideAmbientIntensity = clone->_adsorptionSurfaceFrontSideAmbientIntensity;
  _adsorptionSurfaceFrontSideSpecularIntensity = clone->_adsorptionSurfaceFrontSideSpecularIntensity;
  _adsorptionSurfaceFrontSideShininess = clone->_adsorptionSurfaceFrontSideShininess;

  _adsorptionSurfaceBackSideHDR = clone->_adsorptionSurfaceBackSideHDR;
  _adsorptionSurfaceBackSideHDRExposure = clone->_adsorptionSurfaceBackSideHDRExposure;
  _adsorptionSurfaceBackSideAmbientColor = clone->_adsorptionSurfaceBackSideAmbientColor;
  _adsorptionSurfaceBackSideDiffuseColor = clone->_adsorptionSurfaceBackSideDiffuseColor;
  _adsorptionSurfaceBackSideSpecularColor = clone->_adsorptionSurfaceBackSideSpecularColor;
  _adsorptionSurfaceBackSideDiffuseIntensity = clone->_adsorptionSurfaceBackSideDiffuseIntensity;
  _adsorptionSurfaceBackSideAmbientIntensity = clone->_adsorptionSurfaceBackSideAmbientIntensity;
  _adsorptionSurfaceBackSideSpecularIntensity = clone->_adsorptionSurfaceBackSideSpecularIntensity;
  _adsorptionSurfaceBackSideShininess = clone->_adsorptionSurfaceBackSideShininess;


  _creationDate = clone->_creationDate;
  _creationTemperature = clone->_creationTemperature;
  _creationTemperatureScale = clone->_creationTemperatureScale;
  _creationPressure = clone->_creationPressure;
  _creationPressureScale = clone->_creationPressureScale;
  _creationMethod = clone->_creationMethod;
  _creationUnitCellRelaxationMethod = clone->_creationUnitCellRelaxationMethod;
  _creationAtomicPositionsSoftwarePackage = clone->_creationAtomicPositionsSoftwarePackage;
  _creationAtomicPositionsIonsRelaxationAlgorithm = clone->_creationAtomicPositionsIonsRelaxationAlgorithm;
  _creationAtomicPositionsIonsRelaxationCheck = clone->_creationAtomicPositionsIonsRelaxationCheck;
  _creationAtomicPositionsForcefield = clone->_creationAtomicPositionsForcefield;
  _creationAtomicPositionsForcefieldDetails = clone->_creationAtomicPositionsForcefieldDetails;
  _creationAtomicChargesSoftwarePackage = clone->_creationAtomicChargesSoftwarePackage;
  _creationAtomicChargesAlgorithms = clone->_creationAtomicChargesAlgorithms;
  _creationAtomicChargesForcefield = clone->_creationAtomicChargesForcefield;
  _creationAtomicChargesForcefieldDetails = clone->_creationAtomicChargesForcefieldDetails;

  _chemicalFormulaMoiety = clone->_chemicalFormulaMoiety;
  _chemicalFormulaSum = clone->_chemicalFormulaSum;
  _chemicalNameSystematic = clone->_chemicalNameSystematic;
  _cellFormulaUnitsZ = clone->_cellFormulaUnitsZ;


  _citationArticleTitle = clone->_citationArticleTitle;
  _citationJournalTitle = clone->_citationJournalTitle;
  _citationAuthors = clone->_citationAuthors;
  _citationJournalVolume = clone->_citationJournalVolume;
  _citationJournalNumber = clone->_citationJournalNumber;
  _citationJournalPageNumbers = clone->_citationJournalPageNumbers;
  _citationDOI = clone->_citationDOI;
  _citationPublicationDate = clone->_citationPublicationDate;
  _citationDatebaseCodes = clone->_citationDatebaseCodes;

  _experimentalMeasurementRadiation = clone->_experimentalMeasurementRadiation;
  _experimentalMeasurementWaveLength = clone->_experimentalMeasurementWaveLength;
  _experimentalMeasurementThetaMin = clone->_experimentalMeasurementThetaMin;
  _experimentalMeasurementThetaMax = clone->_experimentalMeasurementThetaMax;
  _experimentalMeasurementIndexLimitsHmin = clone->_experimentalMeasurementIndexLimitsHmin;
  _experimentalMeasurementIndexLimitsHmax = clone->_experimentalMeasurementIndexLimitsHmax;
  _experimentalMeasurementIndexLimitsKmin = clone->_experimentalMeasurementIndexLimitsKmin;
  _experimentalMeasurementIndexLimitsKmax = clone->_experimentalMeasurementIndexLimitsKmax;
  _experimentalMeasurementIndexLimitsLmin = clone->_experimentalMeasurementIndexLimitsLmin;
  _experimentalMeasurementIndexLimitsLmax = clone->_experimentalMeasurementIndexLimitsLmax;
  _experimentalMeasurementNumberOfSymmetryIndependentReflections = clone->_experimentalMeasurementNumberOfSymmetryIndependentReflections;
  _experimentalMeasurementSoftware = clone->_experimentalMeasurementSoftware;
  _experimentalMeasurementRefinementDetails = clone->_experimentalMeasurementRefinementDetails;
  _experimentalMeasurementGoodnessOfFit = clone->_experimentalMeasurementGoodnessOfFit;
  _experimentalMeasurementRFactorGt = clone->_experimentalMeasurementRFactorGt;
  _experimentalMeasurementRFactorAll = clone->_experimentalMeasurementRFactorAll;

  clone->atomsTreeController()->setTags();

  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << clone->_atomsTreeController;

  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> _atomsTreeController;

  setRepresentationStyle(_atomRepresentationStyle);

  reComputeBoundingBox();

  _atomsTreeController->setTags();
  _bondSetController->setTags();
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

std::set<int> Structure::filterCartesianBondPositions(std::function<bool(double3)> &)
{
  return std::set<int>();
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
  SKBoundingBox boundingBox = this->boundingBox();

  // store in the cell datastructure
  _cell->setBoundingBox(boundingBox);
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

double Structure::primitiveSelectionFrequency() const
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _primitiveSelectionWorleyNoise3DFrequency;
  case RKSelectionStyle::striped:
      return _primitiveSelectionStripesFrequency;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
     return 0.0;
  }
  return 0.0;
}

void Structure::setPrimitiveSelectionFrequency(double value)
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _primitiveSelectionWorleyNoise3DFrequency = value;
    break;
  case RKSelectionStyle::striped:
    _primitiveSelectionStripesFrequency = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}

double Structure::primitiveSelectionDensity() const
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _primitiveSelectionWorleyNoise3DJitter;
  case RKSelectionStyle::striped:
    return _primitiveSelectionStripesDensity;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    return 0.0;
  }
  return 0.0;
}
void Structure::setPrimitiveSelectionDensity(double value)
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _primitiveSelectionWorleyNoise3DJitter = value;
    break;
  case RKSelectionStyle::striped:
    _primitiveSelectionStripesDensity = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
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
  switch(_adsorptionSurfaceProbeMolecule)
  {
    case ProbeMolecule::helium:
      return double2(10.9, 2.64);
    case ProbeMolecule::nitrogen:
      return double2(36.0,3.31);
    case ProbeMolecule::methane:
      return double2(158.5,3.72);
    case ProbeMolecule::hydrogen:
      return double2(36.7,2.958);
    case ProbeMolecule::water:
      return double2(89.633,3.097);
    case ProbeMolecule::co2:
      // Y. Iwai, H. Higashi, H. Uchida, Y. Arai, Fluid Phase Equilibria 127 (1997) 251-261.
      return double2(236.1,3.72);
    case ProbeMolecule::xenon:
      // Gábor Rutkai, Monika Thol, Roland Span & Jadran Vrabec (2017), Molecular Physics, 115:9-12, 1104-1121
      return double2(226.14,3.949);
    case ProbeMolecule::krypton:
      // Gábor Rutkai, Monika Thol, Roland Span & Jadran Vrabec (2017), Molecular Physics, 115:9-12, 1104-1121
      return double2(162.58,3.6274);
    case ProbeMolecule::argon:
      return double2(119.8, 3.34);
    case ProbeMolecule::multiple_values:
      return double2();
  }
  return double2();
}

double2 Structure::frameworkProbeParameters() const
{
  switch(_frameworkProbeMolecule)
  {
    case ProbeMolecule::helium:
      return double2(10.9, 2.64);
    case ProbeMolecule::nitrogen:
      return double2(36.0,3.31);
    case ProbeMolecule::methane:
      return double2(158.5,3.72);
    case ProbeMolecule::hydrogen:
      return double2(36.7,2.958);
    case ProbeMolecule::water:
      return double2(89.633,3.097);
    case ProbeMolecule::co2:
      // Y. Iwai, H. Higashi, H. Uchida, Y. Arai, Fluid Phase Equilibria 127 (1997) 251-261.
      return double2(236.1,3.72);
    case ProbeMolecule::xenon:
      // Gábor Rutkai, Monika Thol, Roland Span & Jadran Vrabec (2017), Molecular Physics, 115:9-12, 1104-1121
      return double2(226.14,3.949);
    case ProbeMolecule::krypton:
      // Gábor Rutkai, Monika Thol, Roland Span & Jadran Vrabec (2017), Molecular Physics, 115:9-12, 1104-1121
      return double2(162.58,3.6274);
    case ProbeMolecule::argon:
      return double2(119.8, 3.34);
    case ProbeMolecule::multiple_values:
      return double2();
  }
  return double2();
}


void Structure::setRepresentationStyle(RepresentationStyle style)
{
	if (int(style) >= 0 && style < RepresentationStyle::multiple_values)
	{
		_atomRepresentationStyle = style;

		switch (style)
		{
		case RepresentationStyle::defaultStyle:
			_drawAtoms = true;
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
			_atomForceFieldIdentifier = QString("Default");
			_atomColorSchemeIdentifier = QString("Jmol");
      _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;

			_drawBonds = true;
      _bondAmbientOcclusion = false;
			_bondColorMode = RKBondColorMode::uniform;;
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

      _atomSelectionStyle = RKSelectionStyle::striped;
      _atomSelectionScaling = 1.0;
      _atomSelectionIntensity = 0.7;
      _atomSelectionStripesDensity = 0.25;
      _atomSelectionStripesFrequency = 12.0;

      _bondSelectionStyle = RKSelectionStyle::striped;
      _bondSelectionScaling = 1.0;
      _bondSelectionIntensity = 0.7;
      _bondSelectionStripesDensity = 0.25;
      _bondSelectionStripesFrequency = 12.0;

			setRepresentationType(RepresentationType::sticks_and_balls);

			break;
		case RepresentationStyle::fancy:
			_drawAtoms = true;
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
			_atomForceFieldIdentifier = QString("Default");
			_atomColorSchemeIdentifier = QString("Rasmol");
      _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;

			_drawBonds = false;
			_bondAmbientOcclusion = false;

      _atomSelectionStyle = RKSelectionStyle::striped;
      _atomSelectionScaling = 1.0;
      _atomSelectionIntensity = 0.4;
      _atomSelectionStripesDensity = 0.25;
      _atomSelectionStripesFrequency = 12.0;

      _bondSelectionStyle = RKSelectionStyle::striped;
      _bondSelectionScaling = 1.0;
      _bondSelectionIntensity = 0.4;
      _bondSelectionStripesDensity = 0.25;
      _bondSelectionStripesFrequency = 12.0;

			setRepresentationType(RepresentationType::vdw);
			break;
		case RepresentationStyle::licorice:
      _drawAtoms = true;
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
			_atomForceFieldIdentifier = QString("Default");
			_atomColorSchemeIdentifier = QString("Jmol");
      _atomColorSchemeOrder = SKColorSet::ColorSchemeOrder::elementOnly;

			_drawBonds = true;
      _bondAmbientOcclusion = false;
			_bondColorMode = RKBondColorMode::split;;
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
			_bondAmbientOcclusion = false;

      _atomSelectionStyle = RKSelectionStyle::striped;
      _atomSelectionScaling = 1.0;
      _atomSelectionIntensity = 0.8;
      _atomSelectionStripesDensity = 0.25;
      _atomSelectionStripesFrequency = 12.0;

      _bondSelectionStyle = RKSelectionStyle::striped;
      _bondSelectionScaling = 1.0;
      _bondSelectionIntensity = 0.8;
      _bondSelectionStripesDensity = 0.25;
      _bondSelectionStripesFrequency = 12.0;

			setRepresentationType(RepresentationType::unity);
			break;
    case RepresentationStyle::objects:
      break;
		case RepresentationStyle::multiple_values:
			break;
		default:
			break;
		}
	}

	std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    for (const std::shared_ptr<SKAtomTreeNode> &node : asymmetricAtomNodes)
	{
		if (std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
		{
			double radius = 0.0;
			switch (_atomRepresentationType)
			{
			case RepresentationType::sticks_and_balls:
				radius = PredefinedElements::predefinedElements[atom->elementIdentifier()]._covalentRadius;
				atom->setDrawRadius(radius);
				break;
			case RepresentationType::vdw:
				radius = PredefinedElements::predefinedElements[atom->elementIdentifier()]._VDWRadius;
				atom->setDrawRadius(radius);
				break;
			case RepresentationType::unity:
        atom->setDrawRadius(_bondScaleFactor);
				break;
			default:
				break;
			}
		}
	}
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
		setRepresentationColorSchemeIdentifier("Rasmol", colorSets);
		break;
	case RepresentationStyle::licorice:
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
  if (_drawAtoms == true &&
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
     (fabs(_atomSpecularIntensity - 1.0)) < 1e-4 &&
     (fabs(_atomShininess - 6.0) < 1e-4) &&
     (_atomRepresentationType == RepresentationType::sticks_and_balls) &&
     (QString::compare(_atomForceFieldIdentifier, "Default", Qt::CaseInsensitive) == 0) &&
     (QString::compare(_atomColorSchemeIdentifier, "Jmol", Qt::CaseInsensitive) == 0) &&
     (_atomColorSchemeOrder == SKColorSet::ColorSchemeOrder::elementOnly) &&
     (_drawBonds == true) &&
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
     (fabs(_bondValue - 1.0) < 1e-4)  &&
     (_atomSelectionStyle == RKSelectionStyle::WorleyNoise3D) &&
     (fabs(_atomSelectionScaling - 1.2) < 1e-4))
     {
       qDebug() << "FOUND DEFAULT";
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
     (_drawAtoms == true) &&
     (_drawBonds == false) &&
     (_atomHDR == false) &&
     (_atomAmbientOcclusion == true) &&
     (fabs(_atomAmbientIntensity - 1.0) < 1e-4) &&
     (fabs(_atomDiffuseIntensity - 0.0) < 1e-4) &&
     (fabs(_atomSpecularIntensity - 0.2) < 1e-4) &&
     (fabs(_atomShininess - 6.0) < 1e-4) &&
     (fabs(_atomScaleFactor - 1.0) < 1e-4) &&
     (_atomRepresentationType == RepresentationType::vdw) &&
     (QString::compare(_atomForceFieldIdentifier, "Default", Qt::CaseInsensitive) == 0) &&
     (QString::compare(_atomColorSchemeIdentifier, "Rasmol", Qt::CaseInsensitive) == 0) &&
     (_atomColorSchemeOrder == SKColorSet::ColorSchemeOrder::elementOnly) &&
     (_atomSelectionStyle == RKSelectionStyle::WorleyNoise3D) &&
     (fabs(_atomSelectionScaling - 1.0) < 1e-4))
     {
       qDebug() << "FOUND FANCY";
       _atomRepresentationStyle = RepresentationStyle::fancy;
     }
  else if ((_drawAtoms == true) &&
     (_atomRepresentationType == RepresentationType::unity) &&
     (QString::compare(_atomForceFieldIdentifier, "Default", Qt::CaseInsensitive) == 0) &&
     (QString::compare(_atomColorSchemeIdentifier, "Jmol", Qt::CaseInsensitive) == 0) &&
     (_atomColorSchemeOrder == SKColorSet::ColorSchemeOrder::elementOnly) &&
		(fabs(_atomScaleFactor - 1.0) < 1e-4) &&
     (_drawBonds == true) &&
     (_bondColorMode == RKBondColorMode::split) &&
     (fabs(_bondScaleFactor - 0.25) < 1e-4) &&
     (_bondAmbientOcclusion == false) &&
     (_bondHDR == true) &&
     ((_bondHDRExposure - 1.5) < 1e-4) &&
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
     ((_bondAmbientIntensity - 0.1) < 1e-4) &&
     ((_bondDiffuseIntensity - 1.0) < 1e-4) &&
     ((_bondSpecularIntensity - 1.0) < 1e-4) &&
     (fabs(_bondShininess - 4.0) < 1e-4) &&
     (fabs(_bondHue - 1.0) < 1e-4) &&
     (fabs(_bondSaturation - 1.0) < 1e-4) &&
     (fabs(_bondValue - 1.0) < 1e-4) &&
     (_atomSelectionStyle == RKSelectionStyle::WorleyNoise3D) &&
     (fabs(_atomSelectionScaling - 1.5) < 1e-4))
     {
       qDebug() << "FOUND LICORICE";
       _atomRepresentationStyle = RepresentationStyle::licorice;
     }
  else
  {
		qDebug() << "FOUND CUSTOM";
    _atomRepresentationStyle = RepresentationStyle::custom;
  }

  qDebug() << "FOUND: " << int(_atomRepresentationStyle);

	
}

void Structure::setRepresentationType(RepresentationType type)
{
  if(int(type)>=0 && type < RepresentationType::multiple_values)
  {
    _atomRepresentationType = type;
    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
    {
      if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
      {
        double radius = 0.0;
        switch(type)
        {
          case RepresentationType::sticks_and_balls:
            radius = PredefinedElements::predefinedElements[atom->elementIdentifier()]._covalentRadius;
            atom->setDrawRadius(radius);
            _atomScaleFactor = 0.7;
            _bondScaleFactor = 0.15;
            break;
          case RepresentationType::vdw:
            radius = PredefinedElements::predefinedElements[atom->elementIdentifier()]._VDWRadius;
            atom->setDrawRadius(radius);
            _atomScaleFactor = 1.0;
            _bondScaleFactor = 0.15;
            break;
          case RepresentationType::unity:
            _atomScaleFactor = 1.0;
            _bondScaleFactor = 0.25;
            atom->setDrawRadius(_bondScaleFactor);
            break;
          default:
            break;
        }
      }
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


void Structure::updateForceField(ForceFieldSets &forceFieldSets)
{
  qDebug() << "Structure::updateForceField";
  if(ForceFieldSet* forceField = forceFieldSets[_atomForceFieldIdentifier])
  {
    std::vector<std::shared_ptr<SKAtomTreeNode>> asymmetricAtomNodes = _atomsTreeController->flattenedLeafNodes();

    switch(_atomForceFieldOrder)
    {
    case ForceFieldSet::ForceFieldSchemeOrder::elementOnly:
      for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
      {
        if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
        {
          SKElement& element = PredefinedElements::predefinedElements[atom->elementIdentifier()];
          const QString chemicalElement = element._chemicalSymbol;
          ForceFieldType* forceFieldType = (*forceField)[chemicalElement];
          if(forceFieldType)
          {
            atom->setPotentialParameters(forceFieldType->potentialParameters());
            atom->setBondDistanceCriteria(forceFieldType->userDefinedRadius());
          }
        }
      }
      break;
    case ForceFieldSet::ForceFieldSchemeOrder::forceFieldFirst:
      for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
      {
        if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
        {
          ForceFieldType* forceFieldType = (*forceField)[atom->uniqueForceFieldName()];
          if(forceFieldType)
          {
            atom->setPotentialParameters(forceFieldType->potentialParameters());
            atom->setBondDistanceCriteria(forceFieldType->userDefinedRadius());
            break;
          }
          else
          {
            SKElement& element = PredefinedElements::predefinedElements[atom->elementIdentifier()];
            const QString chemicalElement = element._chemicalSymbol;
            forceFieldType = (*forceField)[chemicalElement];
            if(forceFieldType)
            {
              atom->setPotentialParameters(forceFieldType->potentialParameters());
              atom->setBondDistanceCriteria(forceFieldType->userDefinedRadius());
            }
          }
        }
      }
      break;
    case ForceFieldSet::ForceFieldSchemeOrder::forceFieldOnly:
      for(const std::shared_ptr<SKAtomTreeNode> &node: asymmetricAtomNodes)
      {
        if(std::shared_ptr<SKAsymmetricAtom> atom = node->representedObject())
        {
          ForceFieldType* forceFieldType = (*forceField)[atom->uniqueForceFieldName()];
          if(forceFieldType)
          {
            qDebug() << "Setting force field for " << atom->uniqueForceFieldName() << " to: " << forceFieldType->potentialParameters().x;
            atom->setPotentialParameters(forceFieldType->potentialParameters());
            atom->setBondDistanceCriteria(forceFieldType->userDefinedRadius());
          }
          else
          {
            atom->setPotentialParameters(double2(0.0, 1.0));
          }
        }
      }
      break;
    case ForceFieldSet::ForceFieldSchemeOrder::multiple_values:
      break;
    }
  }
  else
  {
      std::cout << "No force field present" << std::endl;
  }
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
  std::set<int>::const_iterator search = _bondSetController->selectionIndexSet().find(asymmetricBondId);
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
  std::cout << "Surface area: " << value << std::endl;
  _structureGravimetricNitrogenSurfaceArea = value * Constants::AvogadroConstantPerAngstromSquared / _structureMass;
  _structureVolumetricNitrogenSurfaceArea = value * 1e4 / cell()->volume();
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
  stream << structure->_displayName;
  stream << structure->_isVisible;

  stream << structure->_spaceGroup;
  stream << structure->_cell;
  stream << structure->_periodic;
  stream << structure->_origin;
  stream << structure->_scaling;
  stream << structure->_orientation;
  stream << structure->_rotationDelta;

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

  stream << static_cast<typename std::underlying_type<Structure::ProbeMolecule>::type>(structure->_frameworkProbeMolecule);

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


  stream << structure->_drawUnitCell;
  stream << structure->_unitCellScaleFactor;
  stream << structure->_unitCellDiffuseColor;
  stream << structure->_unitCellDiffuseIntensity;

  stream << structure->_localAxes;

  stream << structure->_drawAdsorptionSurface;
  stream << structure->_adsorptionSurfaceOpacity;
  stream << structure->_adsorptionSurfaceIsoValue;
  stream << structure->_adsorptionSurfaceMinimumValue;

  stream << structure->_adsorptionSurfaceSize;
  stream << structure->_adsorptionSurfaceNumberOfTriangles;

  stream << static_cast<typename std::underlying_type<Structure::ProbeMolecule>::type>(structure->_adsorptionSurfaceProbeMolecule);

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

  stream >> structure->_displayName;
  stream >> structure->_isVisible;

  stream >> structure->_spaceGroup;
  stream >> structure->_cell;
  stream >> structure->_periodic;
  stream >> structure->_origin;
  stream >> structure->_scaling;
  stream >> structure->_orientation;
  stream >> structure->_rotationDelta;

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

  if(versionNumber >= 3)
  {
    qint64 probeMolecule;
    stream >> probeMolecule;
    structure->_frameworkProbeMolecule = Structure::ProbeMolecule(probeMolecule);
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

  stream >> structure->_drawUnitCell;
  stream >> structure->_unitCellScaleFactor;
  stream >> structure->_unitCellDiffuseColor;
  stream >> structure->_unitCellDiffuseIntensity;

  if(versionNumber >= 8) // introduced in version 8
  {
    stream >> structure->_localAxes;
  }

  stream >> structure->_drawAdsorptionSurface;
  stream >> structure->_adsorptionSurfaceOpacity;
  stream >> structure->_adsorptionSurfaceIsoValue;
  stream >> structure->_adsorptionSurfaceMinimumValue;

  stream >> structure->_adsorptionSurfaceSize;
  stream >> structure->_adsorptionSurfaceNumberOfTriangles;

  qint64 adsorptionSurfaceProbeMolecule;
  stream >> adsorptionSurfaceProbeMolecule;
  structure->_adsorptionSurfaceProbeMolecule = Structure::ProbeMolecule(adsorptionSurfaceProbeMolecule);

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
  stream >> structure->_structureMaterialType;
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

  structure->setRepresentationStyle(structure->atomRepresentationStyle());

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
