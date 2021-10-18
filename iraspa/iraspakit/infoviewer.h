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

/*
class BasicInfoViewer
{
public:
  virtual ~BasicInfoViewer() = 0;
  virtual QString authorFirstName() = 0;
  virtual void setAuthorFirstName(QString name) = 0;
  virtual QString authorMiddleName() = 0;
  virtual void setAuthorMiddleName(QString name) = 0;
  virtual QString authorLastName() = 0;
  virtual void setAuthorLastName(QString name) = 0;
  virtual QString authorOrchidID() = 0;
  virtual void setAuthorOrchidID(QString name) = 0;
  virtual QString authorResearcherID() = 0;
  virtual void setAuthorResearcherID(QString name) = 0;
  virtual QString authorAffiliationUniversityName() = 0;
  virtual void setAuthorAffiliationUniversityName(QString name) = 0;
  virtual QString authorAffiliationFacultyName() = 0;
  virtual void setAuthorAffiliationFacultyName(QString name) = 0;
  virtual QString authorAffiliationInstituteName() = 0;
  virtual void setAuthorAffiliationInstituteName(QString name) = 0;
  virtual QString authorAffiliationCityName() = 0;
  virtual void setAuthorAffiliationCityName(QString name) = 0;
  virtual QString authorAffiliationCountryName() = 0;
  virtual void setAuthorAffiliationCountryName(QString name) = 0;
};*/

class InfoViewer
{
public:
  enum class TemperatureScale: qint64
  {
    Kelvin = 0, Celsius = 1, multiple_values = 2
  };

  enum class PressureScale: qint64
  {
    Pascal = 0, bar = 1, multiple_values = 2
  };

  enum class CreationMethod: qint64
  {
    unknown = 0, simulation = 1, experimental = 2, multiple_values = 3
  };

  enum class UnitCellRelaxationMethod: qint64
  {
    unknown = 0, allFree = 1, fixedAnglesIsotropic = 2, fixedAnglesAnistropic = 3, betaAnglefixed = 4, fixedVolumeFreeAngles = 5, allFixed = 6, multiple_values = 7
  };

  enum class IonsRelaxationAlgorithm: qint64
  {
    unknown = 0, none = 1, simplex = 2, simulatedAnnealing = 3, geneticAlgorithm = 4, steepestDescent = 5, conjugateGradient = 6,
    quasiNewton = 7, NewtonRaphson = 8, BakersMinimization = 9, multiple_values = 10
  };

  enum class IonsRelaxationCheck: qint64
  {
    unknown = 0, none = 1, allPositiveEigenvalues = 2, someSmallNegativeEigenvalues = 3, someSignificantNegativeEigenvalues = 4,
    manyNegativeEigenvalues = 5, multiple_values = 6
  };

  virtual ~InfoViewer() = 0;
  //virtual QDate creationDate() = 0;
  //virtual void setCreationDate(QDate date) = 0;
  virtual QString creationTemperature() = 0;
  virtual void setCreationTemperature(QString name) = 0;
  virtual TemperatureScale creationTemperatureScale() = 0;
  virtual void setCreationTemperatureScale(TemperatureScale scale) = 0;
  virtual QString creationPressure() = 0;
  virtual void setCreationPressure(QString pressure) = 0;
  virtual PressureScale creationPressureScale() = 0;
  virtual void setCreationPressureScale(PressureScale scale) = 0;
  virtual CreationMethod creationMethod() = 0;
  virtual void setCreationMethod(CreationMethod method) = 0;
  virtual UnitCellRelaxationMethod creationUnitCellRelaxationMethod() = 0;
  virtual void setCreationUnitCellRelaxationMethod(UnitCellRelaxationMethod method) = 0;
  virtual QString creationAtomicPositionsSoftwarePackage() = 0;
  virtual void setCreationAtomicPositionsSoftwarePackage(QString name) = 0;
  virtual IonsRelaxationAlgorithm creationAtomicPositionsIonsRelaxationAlgorithm() = 0;
  virtual void setCreationAtomicPositionsIonsRelaxationAlgorithm(IonsRelaxationAlgorithm algorithm) = 0;
  virtual IonsRelaxationCheck creationAtomicPositionsIonsRelaxationCheck() = 0;
  virtual void setCreationAtomicPositionsIonsRelaxationCheck(IonsRelaxationCheck check) = 0;
  virtual QString creationAtomicPositionsForcefield() = 0;
  virtual void setCreationAtomicPositionsForcefield(QString name) = 0;
  virtual QString creationAtomicPositionsForcefieldDetails() = 0;
  virtual void setCreationAtomicPositionsForcefieldDetails(QString name) = 0;
  virtual QString creationAtomicChargesSoftwarePackage() = 0;
  virtual void setCreationAtomicChargesSoftwarePackage(QString name) = 0;
  virtual QString creationAtomicChargesAlgorithms() = 0;
  virtual void setCreationAtomicChargesAlgorithms(QString name) = 0;
  virtual QString creationAtomicChargesForcefield() = 0;
  virtual void setCreationAtomicChargesForcefield(QString name) = 0;
  virtual QString creationAtomicChargesForcefieldDetails() = 0;
  virtual void setCreationAtomicChargesForcefieldDetails(QString name) = 0;

  virtual QString experimentalMeasurementRadiation() = 0;
  virtual void setExperimentalMeasurementRadiation(QString name) = 0;
  virtual QString experimentalMeasurementWaveLength() = 0;
  virtual void setExperimentalMeasurementWaveLength(QString name) = 0;
  virtual QString experimentalMeasurementThetaMin() = 0;
  virtual void setExperimentalMeasurementThetaMin(QString name) = 0;
  virtual QString experimentalMeasurementThetaMax() = 0;
  virtual void setExperimentalMeasurementThetaMax(QString name) = 0;
  virtual QString experimentalMeasurementIndexLimitsHmin() = 0;
  virtual void setExperimentalMeasurementIndexLimitsHmin(QString name) = 0;
  virtual QString experimentalMeasurementIndexLimitsHmax() = 0;
  virtual void setExperimentalMeasurementIndexLimitsHmax(QString name) = 0;
  virtual QString experimentalMeasurementIndexLimitsKmin() = 0;
  virtual void setExperimentalMeasurementIndexLimitsKmin(QString name) = 0;
  virtual QString experimentalMeasurementIndexLimitsKmax() = 0;
  virtual void setExperimentalMeasurementIndexLimitsKmax(QString name) = 0;
  virtual QString experimentalMeasurementIndexLimitsLmin() = 0;
  virtual void setExperimentalMeasurementIndexLimitsLmin(QString name) = 0;
  virtual QString experimentalMeasurementIndexLimitsLmax() = 0;
  virtual void setExperimentalMeasurementIndexLimitsLmax(QString name) = 0;
  virtual QString experimentalMeasurementNumberOfSymmetryIndependentReflections() = 0;
  virtual void setExperimentalMeasurementNumberOfSymmetryIndependentReflections(QString name) = 0;
  virtual QString experimentalMeasurementSoftware() = 0;
  virtual void setExperimentalMeasurementSoftware(QString name) = 0;
  virtual QString experimentalMeasurementRefinementDetails() = 0;
  virtual void setExperimentalMeasurementGoodnessOfFit(QString goodness) = 0;
  virtual QString experimentalMeasurementGoodnessOfFit() = 0;
  virtual void setExperimentalMeasurementRefinementDetails(QString name) = 0;
  virtual QString experimentalMeasurementRFactorGt() = 0;
  virtual void setExperimentalMeasurementRFactorGt(QString name) = 0;
  virtual QString experimentalMeasurementRFactorAll() = 0;
  virtual void setExperimentalMeasurementRFactorAll(QString name) = 0;

  virtual QString chemicalFormulaMoiety() = 0;
  virtual void setChemicalFormulaMoiety(QString name) = 0;
  virtual QString chemicalFormulaSum() = 0;
  virtual void setChemicalFormulaSum(QString name) = 0;
  virtual QString chemicalNameSystematic() = 0;
  virtual void setChemicalNameSystematic(QString name) = 0;

  virtual QString citationArticleTitle() = 0;
  virtual void setCitationArticleTitle(QString name) = 0;
  virtual QString citationJournalTitle() = 0;
  virtual void setCitationJournalTitle(QString name) = 0;
  virtual QString citationAuthors() = 0;
  virtual void setCitationAuthors(QString name) = 0;
  virtual QString citationJournalVolume() = 0;
  virtual void setCitationJournalVolume(QString name) = 0;
  virtual QString citationJournalNumber() = 0;
  virtual void setCitationJournalNumber(QString name) = 0;
  virtual QString citationJournalPageNumbers() = 0;
  virtual void setCitationJournalPageNumbers(QString name) = 0;
  virtual QString citationDOI() = 0;
  virtual void setCitationDOI(QString name) = 0;
  virtual QDate citationPublicationDate() = 0;
  virtual void setCitationPublicationDate(QDate date) = 0;
  virtual QString citationDatebaseCodes() = 0;
  virtual void setCitationDatebaseCodes(QString name) = 0;
};
