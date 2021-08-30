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

#include <QUrl>
#include <QList>
#include <QString>
#include <QDataStream>
#include <QStringList>
#include <renderkit.h>
#include <symmetrykit.h>
#include "project.h"
#include "scenelist.h"
#include "iraspastructure.h"

class ProjectStructure: public Project, public RKRenderDataSource
{
public:
  ProjectStructure();
  ~ProjectStructure() override;
  ProjectStructure(QString filename, SKColorSets& colorSets, ForceFieldSets& forcefieldSets, bool asSeparateProject, bool onlyAsymmetricUnit, bool asMolecule, LogReporting *log);
  ProjectStructure(QList<QUrl> fileURLs, SKColorSets& colorSets, ForceFieldSets& forcefieldSets, bool asSeparateProject, bool onlyAsymmetricUnit, bool asMolecule, LogReporting *log);

  std::vector<size_t> numberOfScenes() const override final;
  int numberOfMovies(int sceneIndex) const override final;
  std::vector<std::shared_ptr<RKRenderStructure>> renderStructuresForScene(size_t i) const;

  std::vector<std::shared_ptr<RKLight>>& renderLights() override final {return _renderLights;}

  std::vector<RKInPerInstanceAttributesAtoms> renderMeasurementPoints() const override final;
  std::vector<RKRenderStructure> renderMeasurementStructure() const override final;

  bool hasSelectedObjects() const override final;

  RKBackgroundType renderBackgroundType() const override final;
  void setBackgroundType(RKBackgroundType type);
  QColor renderBackgroundColor() const override final;
  void setBackgroundColor(QColor color);

  QColor linearGradientFromColor() const {return _backgroundLinearGradientFromColor;}
  void setLinearGradientFromColor(QColor color) {_backgroundLinearGradientFromColor = color;}
  QColor linearGradientToColor() const {return _backgroundLinearGradientToColor;}
  void setLinearGradientToColor(QColor color) {_backgroundLinearGradientToColor = color;}
  double linearGradientAngle() const {return _backgroundLinearGradientAngle;}
  void setLinearGradientAngle(double angle) {_backgroundLinearGradientAngle = angle;}

  QColor radialGradientFromColor() const {return _backgroundRadialGradientFromColor;}
  void setRadialGradientFromColor(QColor color) {_backgroundRadialGradientFromColor = color;}
  QColor radialGradientToColor() const {return _backgroundRadialGradientToColor;}
  void setRadialGradientToColor(QColor color) {_backgroundRadialGradientToColor = color;}
  double radialGradientRoundness() const {return _backgroundRadialGradientRoundness;}
  void setRadialGradientRoundness(double value) {_backgroundRadialGradientRoundness = value;}
  void loadBackgroundImage(QString filename);
  QString backgroundImageFilename() {return _backgroundImageFilename;}

  const QImage renderBackgroundCachedImage() override final;

  int renderImageNumberOfPixels() const override final {return _renderImageNumberOfPixels;}
  void setImageNumberOfPixels(int width) {_renderImageNumberOfPixels = width;}
  void setImageAspectRatio(double ratio) {_aspectRatio = ratio;}
  double imageAspectRatio() {return _aspectRatio;}
  double renderImagePhysicalSizeInInches() const override final {return _renderImagePhysicalSizeInInches;}
  void setImagePhysicalSizeInInches(double width) {_renderImagePhysicalSizeInInches = width;}
  RKImageDPI imageDPI() {return _imageDPI;}
  void setImageDPI(RKImageDPI dpi) {_imageDPI = dpi;}
  RKImageUnits imageUnits() {return _imageUnits;}
  void setImageUnits(RKImageUnits units) {_imageUnits = units;}
  RKImageDimensions imageDimensions() {return _imageDimensions;}
  void setImageDimensions(RKImageDimensions dimensions) {_imageDimensions = dimensions;}
  RKImageQuality renderImageQuality() {return _renderImageQuality;}
  void setImageQuality(RKImageQuality quality) {_renderImageQuality = quality;}
  int movieFramesPerSecond() {return _movieFramesPerSecond;}
  void setMovieFramesPerSecond(int fps) {_movieFramesPerSecond = fps;}
  double imageDotsPerInchValue();

  size_t maxNumberOfMoviesFrames();
  //void setMovieFrameIndex(int index);

  void setShowBoundingBox(bool show) {_showBoundingBox = show;}
  SKBoundingBox renderBoundingBox() const override final;
  bool showBoundingBox() const override final;
  std::vector<RKInPerInstanceAttributesAtoms> renderBoundingBoxSpheres() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderBoundingBoxCylinders() const override final;
  std::shared_ptr<RKCamera> camera() const override final {return _camera;}

  std::shared_ptr<SceneList> sceneList() {return _sceneList;}

  void setInitialSelectionIfNeeded() override final;
  std::shared_ptr<RKGlobalAxes> axes() const override final {return _renderAxes;}
private:
  qint64 _versionNumber{3};

  SKBoundingBox _boundingBox = SKBoundingBox();
  bool _showBoundingBox{false};

  RKBackgroundType _backgroundType = RKBackgroundType::color;
  QImage _backgroundImage;
  QString _backgroundImageFilename;
  QColor _backgroundColor = QColor(255,255,255,255);
  QColor _backgroundLinearGradientFromColor = QColor(0,0,0,255);
  QColor _backgroundLinearGradientToColor = QColor(255,255,255,255);
  QColor _backgroundRadialGradientFromColor = QColor(0,0,0,255);
  QColor _backgroundRadialGradientToColor = QColor(255,255,255,255);
  double _backgroundLinearGradientAngle = 45.0;
  double _backgroundRadialGradientRoundness = 0.4;

  double _renderImagePhysicalSizeInInches = 6.5;
  qint64 _renderImageNumberOfPixels = 1200;
  double _aspectRatio = 1.0;
  RKImageDPI _imageDPI = RKImageDPI::dpi_300;
  RKImageUnits _imageUnits = RKImageUnits::cm;
  RKImageDimensions _imageDimensions = RKImageDimensions::pixels;
  RKImageQuality _renderImageQuality = RKImageQuality::rgb_8_bits;
  qint64 _movieFramesPerSecond = 10;

  std::vector<std::shared_ptr<RKLight>> _renderLights{std::make_shared<RKLight>(),std::make_shared<RKLight>(),std::make_shared<RKLight>(),std::make_shared<RKLight>()};
  std::shared_ptr<RKCamera> _camera;
  std::shared_ptr<RKGlobalAxes> _renderAxes = std::make_shared<RKGlobalAxes>();

  std::shared_ptr<SceneList> _sceneList = std::make_shared<SceneList>();

  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<ProjectStructure> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<ProjectStructure> &);
};
