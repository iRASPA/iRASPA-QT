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
  void setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAStructure>> iraspa_structures);

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
  std::vector<std::shared_ptr<iRASPAStructure>> _iraspa_structures{};


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
  std::optional<QString> authorFirstName();
  void setAuthorFirstName(QString name);
  std::optional<QString> authorMiddleName();
  void setAuthorMiddleName(QString name);
  std::optional<QString> authorLastName();
  void setAuthorLastName(QString name);
  std::optional<QString> authorOrchidID();
  void setAuthorOrchidID(QString name);
  std::optional<QString> authorResearcherID();
  void setAuthorResearcherID(QString name);
  std::optional<QString> authorAffiliationUniversityName();
  void setAuthorAffiliationUniversityName(QString name);
  std::optional<QString> authorAffiliationFacultyName();
  void setAuthorAffiliationFacultyName(QString name);
  std::optional<QString> authorAffiliationInstituteName();
  void setAuthorAffiliationInstituteName(QString name);
  std::optional<QString> authorAffiliationCityName();
  void setAuthorAffiliationCityName(QString name);
  std::optional<QString> authorAffiliationCountryName();
  void setAuthorAffiliationCountryName(QString name);

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
  std::optional<QDate> creationDate();
  void setCreationDate(QDate date);
  std::optional<QString> creationTemperature();
  void setCreationTemperature(QString name);
  std::optional<Structure::TemperatureScale> creationTemperatureScale();
  void setCreationTemperatureScale(int value);
  std::optional<QString> creationPressure();
  void setCreationPressure(QString name);
  std::optional<Structure::PressureScale> creationPressureScale();
  void setCreationPressureScale(int scale);
  std::optional<Structure::CreationMethod> creationMethod();
  void setCreationMethod(int value);
  std::optional<Structure::UnitCellRelaxationMethod> creationRelaxUnitCell();
  void setCreationRelaxUnitCell(int value);
  std::optional<QString> creationAtomicPositionsSoftwarePackage();
  void setCreationAtomicPositionsSoftwarePackage(QString name);
  std::optional<Structure::IonsRelaxationAlgorithm> creationAtomicPositionsAlgorithm();
  void setCreationAtomicPositionsAlgorithm(int value);
  std::optional<Structure::IonsRelaxationCheck> creationAtomicPositionsEigenvalues();
  void setCreationAtomicPositionsEigenvalues(int value);
  std::optional<QString> creationAtomicPositionsForceField();
  void setCreationAtomicPositionsForceField(QString name);
  std::optional<QString> creationAtomicPositionsForceFieldDetails();
  void setCreationAtomicPositionsForceFieldDetails(QString name);
  std::optional<QString> creationAtomicChargesSoftwarePackage();
  void setCreationAtomicChargesSoftwarePackage(QString name);
  std::optional<QString> creationAtomicChargesAlgorithm();
  void setCreationAtomicChargesAlgorithm(QString name);
  std::optional<QString> creationAtomicChargesForceField();
  void setCreationAtomicChargesForceField(QString name);
  std::optional<QString> creationAtomicChargesForceFieldDetails();
  void setCreationAtomicChargesForceFieldDetails(QString name);

  void reloadChemicalFormulaMoiety();
  void reloadChemicalFormulaSum();
  void reloadChemicalNameSystematic();
  std::optional<QString> chemicalFormulaMoiety();
  void setChemicalFormulaMoiety(QString name);
  std::optional<QString> chemicalFormulaSum();
  void setChemicalFormulaSum(QString name);
  std::optional<QString> chemicalNameSystematic();
  void setChemicalNameSystematic(QString name);

  void reloadCitationArticleTitle();
  void reloadCitationArticleAuthors();
  void reloadCitationJournalVolume();
  void reloadCitationJournalNumber();
  void reloadCitationPublicationDate();
  void reloadCitationPublicationDOI();
  void reloadCitationPublicationDatabaseCodes();
  std::optional<QString> citationArticleTitle();
  void setCitationArticleTitle(QString name);
  std::optional<QString> citationJournalTitle();
  void setCitationJournalTitle(QString name);
  std::optional<QString> citationArticleAuthors();
  void setCitationArticleAuthors(QString name);
  std::optional<QString> citationJournalVolume();
  void setCitationJournalVolume(QString name);
  std::optional<QString> citationJournalNumber();
  void setCitationJournalNumber(QString name);
  std::optional<QDate> citationPublicationDate();
  void setCitationPublicationDate(QDate date);
  std::optional<QString> citationDOI();
  void setCitationDOI(QString name);
  std::optional<QString> citationDatabaseCodes();
  void setCitationDatabaseCodes(QString name);

private slots:
  void expandCreatorItem();
  void expandCreationItem();
  void expandChemicalItem();
  void expandCitationItem();
};
