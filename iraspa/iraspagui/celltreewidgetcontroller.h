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
#include <unordered_set>
#include <vector>
#include <iraspakit.h>
#include "iraspamainwindowconsumerprotocol.h"
#include "celltreewidgetchangespacegroupcommand.h"
#include "cellcellform.h"
#include "celltransformcontentform.h"
#include "cellstructuralform.h"
#include "cellsymmetryform.h"
#include "logreporting.h"

class CellTreeWidgetController : public QTreeWidget, public MainWindowConsumer, public ProjectConsumer, public Reloadable, public LogReportingConsumer
{
  Q_OBJECT

public:
  CellTreeWidgetController(QWidget* parent = nullptr);

  void setMainWindow(MainWindow *mainWindow)  override{_mainWindow = mainWindow;}
  void setFlattenedSelectedFrames(std::vector<std::shared_ptr<iRASPAObject>> iraspa_structures);
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override;

  void resetData();
  void reloadData() override;
  void reloadSelection() override;

  void setLogReportingWidget(LogReporting *logReporting) override final {_logReporter = logReporting;}
  LogReporting* logReporter() const override final {return _logReporter;};
private:
  CellCellForm* _cellCellForm;
  CellTransformContentForm* _cellTransformContentForm;
  CellStructuralForm* _cellStructuralForm;
  CellSymmetryForm* _cellSymmetryForm;

  QPushButton* pushButtonCell;
  QPushButton* pushButtonTransformContent;
  QPushButton* pushButtonStructural;
  QPushButton* pushButtonSymmetry;

  LogReporting* _logReporter = nullptr;
  MainWindow *_mainWindow;
  std::shared_ptr<ProjectTreeNode> _projectTreeNode;
  std::shared_ptr<ProjectStructure> _projectStructure;
  std::vector<std::shared_ptr<iRASPAObject>> _iraspa_structures{};
  void reloadCellProperties();

  void reloadStructureType();
  void reloadBoundingBox();
  void reloadUnitCell();
  void reloadUnitCellLengths();
  void reloadUnitCellAngles();
  void reloadCellVolume();
  void reloadCellPerpendicularWidthX();
  void reloadCellPerpendicularWidthY();
  void reloadCellPerpendicularWidthZ();
  void reloadCellMaximumReplicasX();
  void reloadCellMinimumReplicasX();
  void reloadCellMaximumReplicasY();
  void reloadCellMinimumReplicasY();
  void reloadCellMaximumReplicasZ();
  void reloadCellMinimumReplicasZ();
  void reloadRotationAngle();
  void reloadEulerAngles();
  void reloadCellOriginX();
  void reloadCellOriginY();
  void reloadCellOriginZ();

  void setStructureType(int value);
  std::optional<ObjectType>  structureType();
  SKBoundingBox boundingBox();

  std::optional<double> unitCellAX();
  std::optional<double> unitCellAY();
  std::optional<double> unitCellAZ();

  std::optional<double> unitCellBX();
  std::optional<double> unitCellBY();
  std::optional<double> unitCellBZ();

  std::optional<double> unitCellCX();
  std::optional<double> unitCellCY();
  std::optional<double> unitCellCZ();

  std::optional<double> unitCellLengthA();
  void setUnitCellLengthA(double value);
  std::optional<double> unitCellLengthB();
  void setUnitCellLengthB(double value);
  std::optional<double> unitCellLengthC();
  void setUnitCellLengthC(double value);

  std::optional<double> unitCellAngleAlpha();
  void setUnitCellAngleAlpha(double value);
  std::optional<double> unitCellAngleBeta();
  void setUnitCellAngleBeta(double value);
  std::optional<double> unitCellAngleGamma();
  void setUnitCellAngleGamma(double value);

  std::optional<double> unitCellVolume();
  std::optional<double> unitCellPerpendicularWidthX();
  std::optional<double> unitCellPerpendicularWidthY();
  std::optional<double> unitCellPerpendicularWidthZ();

  std::optional<int> maximumReplicasX();
  void setMaximumReplicasX(int value);
  std::optional<int> maximumReplicasY();
  void setMaximumReplicasY(int value);
  std::optional<int> maximumReplicasZ();
  void setMaximumReplicasZ(int value);
  std::optional<int> minimumReplicasX();
  void setMinimumReplicasX(int value);
  std::optional<int> minimumReplicasY();
  void setMinimumReplicasY(int value);
  std::optional<int> minimumReplicasZ();
  void setMinimumReplicasZ(int value);

  std::optional<double> rotationAngle();
  void setRotationAngle(double angle);
  void rotateYawPlus();
  void rotateYawMinus();
  void rotatePitchPlus();
  void rotatePitchMinus();
  void rotateRollPlus();
  void rotateRollMinus();

  std::optional<double> EulerAngleX();
  void setEulerAngleXIntermediate(double angle);
  void setEulerAngleX(double angle);
  std::optional<double> EulerAngleY();
  void setEulerAngleYIntermediate(double angle);
  void setEulerAngleY(double angle);
  std::optional<double> EulerAngleZ();
  void setEulerAngleZIntermediate(double angle);
  void setEulerAngleZ(double angle);

  std::optional<double> originX();
  void setOriginX(double value);
  std::optional<double> originY();
  void setOriginY(double value);
  std::optional<double> originZ();
  void setOriginZ(double value);


  void reloadTransformContentProperties();

  void reloadFlipAxesProperties();
  void reloadShiftAxesProperties();

  void applyTransformContent();

  std::optional<bool> flipAxisA();
  void setFlipAxisA(bool state);
  std::optional<bool> flipAxisB();
  void setFlipAxisB(bool state);
  std::optional<bool> flipAxisC();
  void setFlipAxisC(bool state);

  std::optional<double> shiftAxisA();
  void setShiftAxisA(double value);
  std::optional<double> shiftAxisB();
  void setShiftAxisB(double value);
  std::optional<double> shiftAxisC();
  void setShiftAxisC(double value);


  void reloadStructureProperties();

  void reloadStructuralMaterialType();
  void reloadStructuralMass();
  void reloadStructuralDensity();
  void reloadStructuralHeliumVoidFraction();
  void reloadStructuralSpecificVolume();
  void reloadStructuralAccessiblePoreVolume();
  void reloadFrameworkProbeMoleculePopupBox();
  void reloadStructuralVolumetricSurfaceArea();
  void reloadStructuralGravimetricSurfaceArea();
  void reloadStructuralNumberOfChannelSystems();
  void reloadStructuralNumberOfInaccessiblePockets();
  void reloadStructuralDimensionalityOfPoreSystem();
  void reloadStructuralLargestOverallCavityDiameter();
  void reloadStructuralRestrictingPoreDiameter();
  void reloadStructuralLargestDiamtereAlongAViablePath();

  std::optional<std::unordered_set<double>> structuralMass();
  std::optional<std::unordered_set<double>> structuralDensity();
  std::optional<std::unordered_set<double>> structureHeliumVoidFraction();
  std::optional<std::unordered_set<double>> structureSpecificVolume();
  std::optional<std::unordered_set<double>> structureAccessiblePoreVolume();
  void setFrameworkProbeMolecule(int value);
  std::optional<std::unordered_set<ProbeMolecule, enum_hash>> frameworkProbeMolecule();
  std::optional<std::unordered_set<double>> structureVolumetricNitrogenSurfaceArea();
  std::optional<std::unordered_set<double>> structureGravimetricNitrogenSurfaceArea();
  std::optional<std::unordered_set<int>> structureNumberOfChannelSystems();
  void setStructureNumberOfChannelSystems(int value);
  std::optional<std::unordered_set<int>> structureNumberOfInaccessiblePockets();
  void setStructureNumberOfInaccessiblePockets(int value);
  std::optional<std::unordered_set<int>> structureDimensionalityOfPoreSystem();
  void setStructureDimensionalityOfPoreSystem(int value);
  std::optional<std::unordered_set<double>> structureLargestCavityDiameter();
  void setStructureLargestCavityDiameter(double value);
  std::optional<std::unordered_set<double>> structureRestrictingPoreLimitingDiameter();
  void setStructureRestrictingPoreLimitingDiameter(double value);
  std::optional<std::unordered_set<double>> structureLargestCavityDiameterAlongAViablePath();
  void setStructureLargestCavityDiameterAlongAviablePath(double value);

  void reloadSymmetryProperties();

  void reloadSpaceGroupHallName();
  void reloadSpaceGroupITNumber();
  void reloadSpaceGroupHolohedry();
  void reloadSpaceGroupQualifier();
  void reloadSpaceGroupPrecision();
  void reloadSpaceGroupCenteringType();
  void reloadSpaceGroupCenteringVectors();
  void reloadSpaceGroupInversion();
  void reloadSpaceGroupInversionCenter();
  void reloadSpaceGroupCentroSymmetric();
  void reloadSpaceGroupEnantiomorphic();
  void reloadSpaceGroupLaueGroup();
  void reloadSpaceGroupPointGroup();
  void reloadSpaceGroupSchoenfliesSymbol();
  void reloadSpaceGroupSymmorphicity();

  std::optional<std::unordered_set<int> > symmetrySpaceGroupHallNumber();
  void setSymmetrySpaceGroupHallNumber(int value);
  std::optional<std::unordered_set<int> > symmetrySpaceGroupStamdardNumber();
  void setSymmetrySpaceGroupStandardNumber(int value);
  std::optional<std::unordered_set<double> > symmetryPrecision();
  void setSymmetryPrecision(double value);
  std::optional<std::set<QString> > symmetryCenteringString();
  std::optional<std::set<QString> > symmetryHolohedryString();
  std::optional<std::set<QString> > symmetryQualifierString();

  std::optional<std::unordered_set<bool> > symmetryInversion();
  std::optional<std::set<QString> > symmetryInversionCenterString();
  std::optional<std::unordered_set<bool> > symmetryCentrosymmetric();
  std::optional<std::unordered_set<bool>> symmetryEnantiomorphic();
  std::optional<std::unordered_set<int> > symmetryPointGroup();
  std::optional<std::set<QString> > symmetrySymmorphicity();

private slots:
  void expandCellItem();
  void expandTransformContentItem();
  void expandStructuralItem();
  void expandSymmetryItem();

  void computeHeliumVoidFractionPushButton();
  void computeGravimetricSurfaceAreaPushButton();
signals:
  void rendererReloadData();
  void rendererReloadBoundingBoxData();
  void reloadAllViews();
  void redrawWithQuality(RKRenderQuality quality);
  void invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};
