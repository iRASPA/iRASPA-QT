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

#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <optional>
#include <iraspakit.h>
#include "iraspamainwindowconsumerprotocol.h"
#include "infocreatorform.h"
#include "infocreationform.h"
#include "infochemicalform.h"
#include "infocitationform.h"

class InfoTreeWidgetController : public QTreeWidget, public ProjectConsumer, public MainWindowConsumer, public Reloadable
{
  Q_OBJECT

public:
  InfoTreeWidgetController(QWidget* parent = nullptr);
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;
  void setMainWindow(MainWindow *mainWindow) override final {_mainWindow = mainWindow;}
  void setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAObject>> iraspa_structures);

  void resetData();
private:
  InfoCreatorForm* _infoCreatorForm;
  InfoCreationForm* _infoCreationForm;
  InfoChemicalForm* _infoChemicalForm;
  InfoCitationForm* _infoCitationForm;

  QPushButton* pushButtonCreator;
  QPushButton* pushButtonCreation;
  QPushButton* pushButtonChemical;
  QPushButton* pushButtonCitation;

  MainWindow *_mainWindow;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<ProjectStructure> _projectStructure;
  std::vector<std::shared_ptr<iRASPAObject>> _iraspa_structures{};


  void reloadData() override final;
  void reloadSelection() override final {;}
  void reloadCreatorData();
  void reloadCreationData();
  void reloadChemicalData();
  void reloadCitationData();

  void reloadAuthorFirstName();
  void reloadAuthorMiddleName();
  void reloadAuthorLastName();
  void reloadAuthorOrchidID();
  void reloadAuthorResearcherID();
  void reloadAuthorAffiliationUniversityName();
  void reloadAuthorAffiliationFacultyName();
  void reloadAuthorAffiliationInstituteName();
  void reloadAuthorAffiliationCityName();
  void reloadAuthorAffiliationCountryName();
  std::optional<std::set<QString> > authorFirstName();
  void setAuthorFirstName(QString name);
  std::optional<std::set<QString> > authorMiddleName();
  void setAuthorMiddleName(QString name);
  std::optional<std::set<QString> > authorLastName();
  void setAuthorLastName(QString name);
  std::optional<std::set<QString> > authorOrchidID();
  void setAuthorOrchidID(QString name);
  std::optional<std::set<QString> > authorResearcherID();
  void setAuthorResearcherID(QString name);
  std::optional<std::set<QString> > authorAffiliationUniversityName();
  void setAuthorAffiliationUniversityName(QString name);
  std::optional<std::set<QString> > authorAffiliationFacultyName();
  void setAuthorAffiliationFacultyName(QString name);
  std::optional<std::set<QString> > authorAffiliationInstituteName();
  void setAuthorAffiliationInstituteName(QString name);
  std::optional<std::set<QString> > authorAffiliationCityName();
  void setAuthorAffiliationCityName(QString name);
  std::optional<std::set<QString> > authorAffiliationCountryName();
  void setAuthorAffiliationCountryName(const QString name);

  void reloadCreationDate();
  void reloadCreationTemperature();
  void reloadCreationTemperatureUnit();
  void reloadCreationPressure();
  void reloadCreationPressureUnit();
  void reloadCreationMethod();
  void reloadCreationRelaxUnitCell();
  void reloadCreationAtomicPositionsSoftwarePackage();
  void reloadCreationAtomicPositionsAlgorithm();
  void reloadCreationAtomicPositionsEigenvalues();
  void reloadCreationAtomicPositionsForceField();
  void reloadCreationAtomicPositionsForceFieldDetails();
  void reloadCreationAtomicChargesSoftwarePackage();
  void reloadCreationAtomicChargesAlgorithm();
  void reloadCreationAtomicChargesForceField();
  void reloadCreationAtomicChargesForceFieldDetails();
  std::optional<std::set<QDate> > creationDate();
  void setCreationDate(const QDate &date);
  std::optional<std::set<QString> > creationTemperature();
  void setCreationTemperature(QString name);
  std::optional<std::set<InfoViewer::TemperatureScale> > creationTemperatureScale();
  void setCreationTemperatureScale(int value);
  std::optional<std::set<QString> > creationPressure();
  void setCreationPressure(QString name);
  std::optional<std::set<InfoViewer::PressureScale> > creationPressureScale();
  void setCreationPressureScale(int scale);
  std::optional<std::set<InfoViewer::CreationMethod> > creationMethod();
  void setCreationMethod(int value);
  std::optional<std::set<InfoViewer::UnitCellRelaxationMethod> > creationRelaxUnitCell();
  void setCreationRelaxUnitCell(int value);
  std::optional<std::set<QString> > creationAtomicPositionsSoftwarePackage();
  void setCreationAtomicPositionsSoftwarePackage(QString name);
  std::optional<std::set<InfoViewer::IonsRelaxationAlgorithm> > creationAtomicPositionsAlgorithm();
  void setCreationAtomicPositionsAlgorithm(int value);
  std::optional<std::set<InfoViewer::IonsRelaxationCheck> > creationAtomicPositionsEigenvalues();
  void setCreationAtomicPositionsEigenvalues(int value);
  std::optional<std::set<QString> > creationAtomicPositionsForceField();
  void setCreationAtomicPositionsForceField(QString name);
  std::optional<std::set<QString> > creationAtomicPositionsForceFieldDetails();
  void setCreationAtomicPositionsForceFieldDetails(QString name);
  std::optional<std::set<QString> > creationAtomicChargesSoftwarePackage();
  void setCreationAtomicChargesSoftwarePackage(QString name);
  std::optional<std::set<QString> > creationAtomicChargesAlgorithm();
  void setCreationAtomicChargesAlgorithm(QString name);
  std::optional<std::set<QString> > creationAtomicChargesForceField();
  void setCreationAtomicChargesForceField(QString name);
  std::optional<std::set<QString> > creationAtomicChargesForceFieldDetails();
  void setCreationAtomicChargesForceFieldDetails(QString name);

  void reloadExperimentalRadiationType();
  void reloadExperimentalWaveLength();
  void reloadExperimentalMeasuermentThetaMin();
  void reloadExperimentalMeasuermentThetaMax();
  void reloadExperimentalMeasuermentHMin();
  void reloadExperimentalMeasuermentHMax();
  void reloadExperimentalMeasuermentKMin();
  void reloadExperimentalMeasuermentKMax();
  void reloadExperimentalMeasuermentLMin();
  void reloadExperimentalMeasuermentLMax();
  void reloadExperimentalNumberOfReflections();
  void reloadExperimentalSoftware();
  void reloadExperimentalDetails();
  void reloadExperimentalGoodnessOfFit();
  void reloadExperimentalFinalIndices();
  void reloadExperimentalRIndicest();
  std::optional< std::set<QString>> creationExperimentalRadiationType();
  void setCreationExperimentalRadiationType(const QString& type);
  std::optional< std::set<QString>> creationExperimentalWaveLength();
  void setCreationExperimentalWaveLength(const QString& wavelength);
  std::optional< std::set<QString>> creationExperimentalMeasuermentThetaMin();
  void setCreationExperimentalMeasuermentThetaMin(const QString& thetamin);
  std::optional< std::set<QString>> creationExperimentalMeasuermentThetaMax();
  void setCreationExperimentalMeasuermentThetaMax(const QString& thetamax);
  std::optional< std::set<QString>> creationExperimentalMeasuermentHMin();
  void setCreationExperimentalMeasuermentHMin(const QString& hmin);
  std::optional< std::set<QString>> creationExperimentalMeasuermentHMax();
  void setCreationExperimentalMeasuermentHMax(const QString& hmax);
  std::optional< std::set<QString>> creationExperimentalMeasuermentKMin();
  void setCreationExperimentalMeasuermentKMin(const QString& kmin);
  std::optional< std::set<QString>> creationExperimentalMeasuermentKMax();
  void setCreationExperimentalMeasuermentKMax(const QString& kmax);
  std::optional< std::set<QString>> creationExperimentalMeasuermentLMin();
  void setCreationExperimentalMeasuermentLMin(const QString& lmin);
  std::optional< std::set<QString>> creationExperimentalMeasuermentLMax();
  void setCreationExperimentalMeasuermentLMax(const QString& lmax);
  std::optional< std::set<QString>> creationExperimentalNumberOfReflections();
  void setCreationExperimentalNumberOfReflections(const QString& reflections);
  std::optional< std::set<QString>> creationExperimentalSoftware();
  void setCreationExperimentalSoftware(const QString& software);
  std::optional< std::set<QString>> creationExperimentalDetails();
  void setCreationExperimentalDetails(const QString& details);
  std::optional< std::set<QString>> creationExperimentalGoodnessOfFit();
  void setCreationExperimentalGoodnessOfFit(const QString& goodness);
  std::optional<std::set<QString> > creationExperimentalFinalIndices();
  void setCreationExperimentalFinalIndices(const QString& indices);
  std::optional<std::set<QString> > creationExperimentalRIndicest();
  void setCreationExperimentalRIndicest(const QString& indices);

  void reloadChemicalFormulaMoiety();
  void reloadChemicalFormulaSum();
  void reloadChemicalNameSystematic();
  std::optional<std::set<QString>> chemicalFormulaMoiety();
  void setChemicalFormulaMoiety(const QString name);
  std::optional<std::set<QString>> chemicalFormulaSum();
  void setChemicalFormulaSum(const QString name);
  std::optional<std::set<QString> > chemicalNameSystematic();
  void setChemicalNameSystematic(const QString name);

  void reloadCitationArticleTitle();
  void reloadCitationArticleAuthors();
  void reloadCitationJournalVolume();
  void reloadCitationJournalNumber();
  void reloadCitationPublicationDate();
  void reloadCitationPublicationDOI();
  void reloadCitationPublicationDatabaseCodes();
  std::optional<std::set<QString>> citationArticleTitle();
  void setCitationArticleTitle(const QString name);
  std::optional<std::set<QString>> citationJournalTitle();
  void setCitationJournalTitle(const QString name);
  std::optional<std::set<QString>> citationArticleAuthors();
  void setCitationArticleAuthors(const QString name);
  std::optional<std::set<QString>> citationJournalVolume();
  void setCitationJournalVolume(const QString name);
  std::optional<std::set<QString>> citationJournalNumber();
  void setCitationJournalNumber(const QString name);
  std::optional<std::set<QDate>> citationPublicationDate();
  void setCitationPublicationDate(QDate date);
  std::optional<std::set<QString>> citationDOI();
  void setCitationDOI(const QString name);
  std::optional<std::set<QString>> citationDatabaseCodes();
  void setCitationDatabaseCodes(const QString name);

private slots:
  void expandCreatorItem();
  void expandCreationItem();
  void expandChemicalItem();
  void expandCitationItem();
};
