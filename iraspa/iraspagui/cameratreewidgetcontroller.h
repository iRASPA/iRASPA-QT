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
#include <QUrl>
#include <optional>
#include <iraspakit.h>
#include <moviemaker.h>
#include "iraspamainwindowconsumerprotocol.h"
#include "cameracameraform.h"
#include "cameraselectionform.h"
#include "cameraaxesform.h"
#include "cameralightsform.h"
#include "camerapicturesform.h"
#include "camerabackgroundform.h"

class CameraTreeWidgetController : public QTreeWidget, public ProjectConsumer, public MainWindowConsumer, Reloadable
{
  Q_OBJECT

public:
  CameraTreeWidgetController(QWidget* parent = nullptr);
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;
  void setMainWindow(MainWindow *mainWindow) override final;

  void resetData();
  void reloadData() override final;
  void reloadSelection() override final;
private:
  CameraCameraForm* _cameraCameraForm;
  CameraSelectionForm* _cameraSelectionForm;
  CameraAxesForm* _cameraAxesForm;
  CameraLightsForm* _cameraLightsForm;
  CameraPicturesForm* _cameraPicturesForm;
  CameraBackgroundForm* _cameraBackgroundForm;

  QPushButton* pushButtonCamera;
  QPushButton* pushButtonSelection;
  QPushButton* pushButtonAxes;
  QPushButton* pushButtonLights;
  QPushButton* pushButtonPictures;
  QPushButton* pushButtonBackground;

  MainWindow *_mainWindow;
  std::weak_ptr<RKCamera> _camera;
  std::shared_ptr<ProjectStructure> _project;

  MovieWriter::Format _movieFormat = MovieWriter::Format::h265;
private slots:
  void expandCameraItem();
  void expandSelectionItem();
  void expandAxesItem();
  void expandLightsItem();
  void expandPicturesItem();
  void expandBackgroundItem();

  void setResetPercentage(double resetPercentage);
  void resetCameraToDefaultDirection();
  void setResetDirectionToMinusX(bool checked = false);
  void setResetDirectionToPlusX(bool checked = false);
  void setResetDirectionToMinusY(bool checked = false);
  void setResetDirectionToPlusY(bool checked = false);
  void setResetDirectionToMinusZ(bool checked = false);
  void setResetDirectionToPlusZ(bool checked = false);
  void setCameraToOrthographic(bool checked = false);
  void setCameraToPerspective(bool checked = false);
  void setAngleOfView(double d);
  void setRotationAngle(double angle);
  void rotateYawPlus();
  void rotateYawMinus();
  void rotatePitchPlus();
  void rotatePitchMinus();
  void rotateRollPlus();
  void rotateRollMinus();
  void setEulerAngleX(int angle);
  void setEulerAngleX(double angle);
  void setEulerAngleZ(int angle);
  void setEulerAngleZ(double angle);
  void setEulerAngleY(int angle);
  void setEulerAngleY(double angle);

  void setAxesPosition(int value);
  void setAxesStyle(int value);
  void setAxesSize(double value);
  void setAxesOffset(double value);

  void setAxesBackgroundStyle(int value);
  void setAxesBackgroundColor();
  void setAxesBackgroundAdditionalSize(double value);

  void setAxesTextScalingX(double value);
  void setAxesTextScalingY(double value);
  void setAxesTextScalingZ(double value);
  void setAxesTextolorX();
  void setAxesTextolorY();
  void setAxesTextolorZ();
  void setAxesXTextDisplacementX(double value);
  void setAxesXTextDisplacementY(double value);
  void setAxesXTextDisplacementZ(double value);
  void setAxesYTextDisplacementX(double value);
  void setAxesYTextDisplacementY(double value);
  void setAxesYTextDisplacementZ(double value);
  void setAxesZTextDisplacementX(double value);
  void setAxesZTextDisplacementY(double value);
  void setAxesZTextDisplacementZ(double value);

  void setSelectionIntensity(double level);
  void setAmbientLightIntensity(double intensity);
  void setDiffuseLightIntensity(double intensity);
  void setSpecularLightIntensity(double intensity);
  void setAmbientColor();
  void setDiffuseColor();
  void setSpecularColor();

  void setPictureDotsPerInch(int value);
  void setPictureQuality(int value);
  void setPictureNumberOfPixels(int width);
  void setPicturePhysicalSize(double width);
  void setPictureEditDimensions(int dimensions);
  void setPictureEditUnits(int units);
  void setMovieFramesPerSecond(int fps);
  void setPictureDimensionToPhysical(bool checked);
  void setPictureDimensionToPixels(bool checked);
  void setPictureUnitsToInch(bool checked);
  void setPictureUnitsToCentimeters(bool checked);
  void savePicture();

  void saveMovie();
  void setNumberOfFramesPerSecond(int value);
  void setMovieFormat(int value);
  void setMovieType(int value);

  void setBackgroundColor();
  void setLinearGradientFromColor();
  void setLinearGradientToColor();
  void setLinearGradientAngle(double angle);
  void setRadialGradientFromColor();
  void setRadialGradientToColor();
  void setRadialGradientRoundness(double roundness);
  void selectBackgroundImage();

  void setBackgroundToColor(bool checked);
  void setBackgroundToLinearGradient(bool checked);
  void setBackgroundToRadialGradient(bool checked);
  void setBackgroundToImage(bool checked);
public slots:
  void reloadCameraData();
  void reloadCameraResetDirection();
  void reloadCameraProjection();
  void reloadCameraProperties();

  void reloadCameraEulerAngles();
  void reloadCameraModelViewMatrix();
  void reloadCameraMovement();

  void reloadSelectionProperties();

  void reloadAxesProperties();
  void reloadAxesBackgroundProperties();
  void reloadAxesTextProperties();

  void reloadLightsProperties();

  void reloadPictureProperties();

  void reloadBackgroundProperties();

  void renderWidgetResize(QSize size);
signals:
  void rendererReloadData();
  void rendererReloadSelectionData();
  void rendererReloadBackgroundImage();
  void updateRenderer();
  void rendererCreatePicture(QUrl filenName, int width, int height);
  void rendererCreateMovie(QUrl filenName, int width, int height, MovieWriter::Format, ProjectStructure::MovieType);
  void invalidateCachedAmbientOcclusionTexture(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};


