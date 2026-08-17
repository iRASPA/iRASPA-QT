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

#include "projectstructure.h"
#include "structure.h"
#include <cfloat>
#include <cmath>
#include <optional>
#include <utility>
#include <QSize>
#include <array>
#include <algorithm>
#include <QPainter>
#include <QFileInfo>
#include <QBuffer>

namespace
{
constexpr double kPi = 3.141592653589793;

bool isPeriodicObject(const std::shared_ptr<RKRenderObject> &object)
{
  auto *typed = dynamic_cast<Object *>(object.get());
  if (!typed || !object->cell())
  {
    return false;
  }
  switch (typed->structureType())
  {
  case ObjectType::crystal:
  case ObjectType::molecularCrystal:
  case ObjectType::proteinCrystal:
  case ObjectType::proteinCrystalSolvent:
  case ObjectType::crystalSolvent:
  case ObjectType::molecularCrystalSolvent:
  case ObjectType::dnaCrystal:
    return true;
  default:
    return false;
  }
}

std::optional<double3> measurementModelPosition(const ProjectStructure::MeasurementAtom &atom)
{
  auto *source = dynamic_cast<RKRenderAtomSource *>(atom.structure.get());
  if (!source)
  {
    return std::nullopt;
  }
  for (const RKInPerInstanceAttributesAtoms &instance : source->renderAtoms())
  {
    if (instance.tag == atom.instanceTag)
    {
      return double3(instance.position.x, instance.position.y, instance.position.z);
    }
  }
  return std::nullopt;
}

double3 measurementScenePosition(const ProjectStructure::MeasurementAtom &atom, const double3 &modelPosition)
{
  return modelPosition + atom.structure->origin();
}

int measurementAtomTag(const ProjectStructure::MeasurementAtom &atom)
{
  if (atom.copy)
  {
    return static_cast<int>(atom.copy->tag());
  }
  return atom.asymmetricAtomId;
}

double clampUnit(double value)
{
  return std::max(-1.0, std::min(1.0, value));
}

std::pair<double, std::optional<double>> measurementDistance(const ProjectStructure::MeasurementAtom &a,
                                                             const ProjectStructure::MeasurementAtom &b)
{
  const std::optional<double3> modelA = measurementModelPosition(a);
  const std::optional<double3> modelB = measurementModelPosition(b);
  if (!modelA || !modelB)
  {
    return {0.0, std::nullopt};
  }

  std::optional<double> periodicLength;
  if (a.structure.get() == b.structure.get() && isPeriodicObject(a.structure))
  {
    periodicLength = a.structure->cell()->applyFullCellBoundaryCondition(*modelB - *modelA).length();
  }

  const double sceneLength = (measurementScenePosition(b, *modelB) - measurementScenePosition(a, *modelA)).length();
  return {sceneLength, periodicLength};
}

double bendFromPositions(const double3 &a, const double3 &b, const double3 &c)
{
  const double3 vectorAB = double3::normalize(a - b);
  const double3 vectorBC = double3::normalize(c - b);
  return std::acos(clampUnit(double3::dot(vectorAB, vectorBC)));
}

std::pair<double, std::optional<double>> measurementBend(const ProjectStructure::MeasurementAtom &a,
                                                         const ProjectStructure::MeasurementAtom &b,
                                                         const ProjectStructure::MeasurementAtom &c)
{
  const std::optional<double3> modelA = measurementModelPosition(a);
  const std::optional<double3> modelB = measurementModelPosition(b);
  const std::optional<double3> modelC = measurementModelPosition(c);
  if (!modelA || !modelB || !modelC)
  {
    return {0.0, std::nullopt};
  }

  std::optional<double> periodicAngle;
  if (a.structure.get() == b.structure.get() && b.structure.get() == c.structure.get() && isPeriodicObject(a.structure))
  {
    const double3 dr1 = a.structure->cell()->applyFullCellBoundaryCondition(*modelA - *modelB);
    const double3 dr2 = a.structure->cell()->applyFullCellBoundaryCondition(*modelC - *modelB);
    periodicAngle = bendFromPositions(dr1 + *modelB, *modelB, dr2 + *modelB);
  }

  const double sceneAngle = bendFromPositions(measurementScenePosition(a, *modelA),
                                              measurementScenePosition(b, *modelB),
                                              measurementScenePosition(c, *modelC));
  return {sceneAngle, periodicAngle};
}

double dihedralFromPositions(const double3 &a, const double3 &b, const double3 &c, const double3 &d)
{
  const double3 Dab = a - b;
  const double3 Dbc = c - b;
  const double3 Dcd = d - c;
  const double3 dr = double3::normalize(Dab - double3::dot(Dab, Dbc) * Dbc);
  const double3 ds = double3::normalize(Dcd - double3::dot(Dcd, Dbc) * Dbc);
  const double cosPhi = clampUnit(double3::dot(dr, ds));
  const double3 Pb = double3::cross(Dbc, Dab);
  const double3 Pc = double3::cross(Dbc, Dcd);
  const double sign = double3::dot(Dbc, double3::cross(Pb, Pc));
  const double absPhi = std::fabs(std::acos(cosPhi));
  return sign > 0.0 ? absPhi : -absPhi;
}

std::pair<double, std::optional<double>> measurementDihedral(const ProjectStructure::MeasurementAtom &a,
                                                             const ProjectStructure::MeasurementAtom &b,
                                                             const ProjectStructure::MeasurementAtom &c,
                                                             const ProjectStructure::MeasurementAtom &d)
{
  const std::optional<double3> modelA = measurementModelPosition(a);
  const std::optional<double3> modelB = measurementModelPosition(b);
  const std::optional<double3> modelC = measurementModelPosition(c);
  const std::optional<double3> modelD = measurementModelPosition(d);
  if (!modelA || !modelB || !modelC || !modelD)
  {
    return {0.0, std::nullopt};
  }

  std::optional<double> periodicAngle;
  if (a.structure.get() == b.structure.get() && b.structure.get() == c.structure.get() && isPeriodicObject(a.structure))
  {
    const double3 Dab = a.structure->cell()->applyFullCellBoundaryCondition(*modelA - *modelB);
    const double3 Dbc = a.structure->cell()->applyFullCellBoundaryCondition(*modelC - *modelB);
    const double3 Dcd = a.structure->cell()->applyFullCellBoundaryCondition(*modelD - *modelC);
    periodicAngle = dihedralFromPositions(Dab + *modelB, *modelB, Dbc + *modelB, Dcd + *modelC);
  }

  const double sceneAngle = dihedralFromPositions(measurementScenePosition(a, *modelA),
                                                  measurementScenePosition(b, *modelB),
                                                  measurementScenePosition(c, *modelC),
                                                  measurementScenePosition(d, *modelD));
  return {sceneAngle, periodicAngle};
}

QString formatLength(const std::pair<double, std::optional<double>> &value)
{
  QString text = QString::number(value.first);
  if (value.second)
  {
    text += QStringLiteral(" (periodic: %1)").arg(*value.second);
  }
  return text;
}

QString formatAngle(const std::pair<double, std::optional<double>> &value)
{
  QString text = QString::number(value.first * 180.0 / kPi);
  if (value.second)
  {
    text += QStringLiteral(" (periodic: %1)").arg(*value.second * 180.0 / kPi);
  }
  return text;
}
}  // namespace

ProjectStructure::ProjectStructure(): _camera(std::make_shared<RKCamera>())
{
  _backgroundImage = QImage(QSize(1024,1024), QImage::Format_ARGB32);
  _backgroundImage.fill(QColor(255,255,255,255));
}

ProjectStructure::ProjectStructure(QString filename, SKColorSets& colorSets, ForceFieldSets& forcefieldSets,
                                   bool proteinOnlyAsymmetricUnit, bool asMolecule, bool separatePolymerChains) noexcept(false): _camera(std::make_shared<RKCamera>())
{
  QUrl url = QUrl::fromLocalFile(filename);
  if (url.isValid())
  {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>(url, colorSets, forcefieldSets, proteinOnlyAsymmetricUnit, asMolecule, separatePolymerChains);
    for(std::shared_ptr<Movie> movie : scene->movies())
    {
      movie->setParent(scene);
    }
    _sceneList->appendScene(scene);
    _camera->resetForNewBoundingBox(this->renderBoundingBox());
  }

  _backgroundImage = QImage(QSize(1024,1024), QImage::Format_ARGB32);
  _backgroundImage.fill(QColor(255,255,255,255));

  setInitialSelectionIfNeeded();
}

ProjectStructure::ProjectStructure(QList<QUrl>  fileURLs, SKColorSets& colorSets, ForceFieldSets& forcefieldSets,
                                   SKParser::ImportType importType, bool onlyAsymmetricUnit, bool asMolecule, bool separatePolymerChains) noexcept(false): _camera(std::make_shared<RKCamera>())
{
  foreach (const QUrl &url, fileURLs)
  {
    if (url.isValid())
    {
      std::shared_ptr<Scene> scene = std::make_shared<Scene>(url, colorSets, forcefieldSets, onlyAsymmetricUnit, asMolecule, separatePolymerChains);
      for(std::shared_ptr<Movie> movie : scene->movies())
      {
        movie->setParent(scene);
      }
      _sceneList->appendScene(scene);
    }
  }

  if(importType == SKParser::ImportType::asMovieFrames)
  {
    std::vector<std::shared_ptr<iRASPAObject>> iraspaStructures = _sceneList->flattenedAllIRASPAStructures();
    _sceneList = std::make_shared<SceneList>("NewMovie", iraspaStructures);
  }

  _camera->resetForNewBoundingBox(this->renderBoundingBox());

  _backgroundImage = QImage(QSize(1024,1024), QImage::Format_ARGB32);
  _backgroundImage.fill(QColor(255,255,255,255));

  setInitialSelectionIfNeeded();
}

ProjectStructure::~ProjectStructure()
{

}

void ProjectStructure::setInitialSelectionIfNeeded()
{
  if(!_sceneList->selectedScene())
  {
    if(!_sceneList->scenes().empty())
    {
      _sceneList->setSelectedScene(_sceneList->scenes().front());
      _sceneList->setSelectedFrameIndex(0);
    }
  }

  for(std::shared_ptr<Scene> scene : _sceneList->selectedScenes())
  {
    if(!scene->selectedMovie())
    {
      if(!scene->movies().empty())
      {
        scene->setSelectedMovie(scene->movies().front());
      }
      else
      {
        scene->setSelectedMovie(nullptr);
      }
    }
  }
}


std::vector<size_t> ProjectStructure::numberOfScenes() const
{
  std::vector<size_t> v = std::vector<size_t>(_sceneList->scenes().size());

  for(size_t i=0;i<_sceneList->scenes().size();i++)
  {
    v[i] = renderStructuresForScene(i).size();
  }

  return v;
}


int ProjectStructure::numberOfMovies([[maybe_unused]] int sceneIndex) const
{
  return 0;
}

std::vector<std::shared_ptr<RKRenderObject>> ProjectStructure::renderStructuresForScene(size_t i) const
{
  std::vector<std::shared_ptr<RKRenderObject>> structures = std::vector<std::shared_ptr<RKRenderObject>>();

  std::optional<size_t> selectedFrameIndex = _sceneList->selectedFrameIndex();
  if(selectedFrameIndex)
  {
    std::shared_ptr<Scene> scene = _sceneList->scenes()[i];
    for(std::shared_ptr<Movie> movie: scene->movies())
    {
      std::shared_ptr<iRASPAObject> selectedFrame = movie->frameAtIndex(*selectedFrameIndex);
      if(selectedFrame)
      {
        if(std::shared_ptr<Object> object = selectedFrame->object())
        {
          if(std::shared_ptr<RKRenderObject> structure = std::dynamic_pointer_cast<RKRenderObject>(object))
          {
            structures.push_back(structure);
          }
        }
      }
    }
  }
  return structures;
}

bool ProjectStructure::addAtomToMeasurement(std::shared_ptr<RKRenderObject> structure, int instanceTag)
{
  const std::optional<AtomInstancePick> pick = decodeAtomInstancePick(structure.get(), instanceTag);
  if (!structure || !pick || _measurementAtoms.size() >= 4)
  {
    return false;
  }
  _measurementAtoms.push_back({std::move(structure), instanceTag, pick->asymmetricAtomIndex, pick->copy, pick->replicaPosition});
  return true;
}

void ProjectStructure::clearMeasurement()
{
  _measurementAtoms.clear();
}

QString ProjectStructure::measurementLogMessage() const
{
  if (_measurementAtoms.size() == 2)
  {
    const std::pair<double, std::optional<double>> distance = measurementDistance(_measurementAtoms[0], _measurementAtoms[1]);
    return QStringLiteral("Distance between atoms [%1,   %2] is %3")
        .arg(measurementAtomTag(_measurementAtoms[0]))
        .arg(measurementAtomTag(_measurementAtoms[1]))
        .arg(formatLength(distance));
  }
  if (_measurementAtoms.size() == 3)
  {
    const std::pair<double, std::optional<double>> distance1 = measurementDistance(_measurementAtoms[0], _measurementAtoms[1]);
    const std::pair<double, std::optional<double>> distance2 = measurementDistance(_measurementAtoms[1], _measurementAtoms[2]);
    const std::pair<double, std::optional<double>> bend = measurementBend(_measurementAtoms[0], _measurementAtoms[1], _measurementAtoms[2]);
    return QStringLiteral("Distances between atoms [%1,   %2, %3] are [%4, %5]; Bend angle between the   atoms is %6")
        .arg(measurementAtomTag(_measurementAtoms[0]))
        .arg(measurementAtomTag(_measurementAtoms[1]))
        .arg(measurementAtomTag(_measurementAtoms[2]))
        .arg(formatLength(distance1))
        .arg(formatLength(distance2))
        .arg(formatAngle(bend));
  }
  if (_measurementAtoms.size() == 4)
  {
    const std::pair<double, std::optional<double>> distance1 = measurementDistance(_measurementAtoms[0], _measurementAtoms[1]);
    const std::pair<double, std::optional<double>> distance2 = measurementDistance(_measurementAtoms[1], _measurementAtoms[2]);
    const std::pair<double, std::optional<double>> distance3 = measurementDistance(_measurementAtoms[2], _measurementAtoms[3]);
    const std::pair<double, std::optional<double>> bend1 = measurementBend(_measurementAtoms[0], _measurementAtoms[1], _measurementAtoms[2]);
    const std::pair<double, std::optional<double>> bend2 = measurementBend(_measurementAtoms[1], _measurementAtoms[2], _measurementAtoms[3]);
    const std::pair<double, std::optional<double>> dihedral = measurementDihedral(_measurementAtoms[0], _measurementAtoms[1],
                                                                                 _measurementAtoms[2], _measurementAtoms[3]);
    return QStringLiteral("Distances between atoms [%1,   %2, %3, %4] are   [%5, %6 %7]; Bend angles between the atoms are [%8, %9]; Dihedral angle   between the atoms is %10")
        .arg(measurementAtomTag(_measurementAtoms[0]))
        .arg(measurementAtomTag(_measurementAtoms[1]))
        .arg(measurementAtomTag(_measurementAtoms[2]))
        .arg(measurementAtomTag(_measurementAtoms[3]))
        .arg(formatLength(distance1))
        .arg(formatLength(distance2))
        .arg(formatLength(distance3))
        .arg(formatAngle(bend1))
        .arg(formatAngle(bend2))
        .arg(formatAngle(dihedral));
  }
  return {};
}

std::vector<RKInPerInstanceAttributesAtoms> ProjectStructure::renderMeasurementPoints() const
{
  std::vector<RKInPerInstanceAttributesAtoms> points;
  const std::vector<std::vector<std::shared_ptr<iRASPAObject>>> selected = _sceneList->selectediRASPAStructures();
  uint32_t flatIndex = 0;
  for (const std::vector<std::shared_ptr<iRASPAObject>> &scene : selected)
  {
    for (const std::shared_ptr<iRASPAObject> &iraspaObject : scene)
    {
      std::shared_ptr<Object> object = iraspaObject ? iraspaObject->object() : nullptr;
      auto *atomSource = dynamic_cast<RKRenderAtomSource *>(object.get());
      if (atomSource && atomSource->drawAtoms())
      {
        for (const MeasurementAtom &measurement : _measurementAtoms)
        {
          if (measurement.structure.get() != object.get())
          {
            continue;
          }
          for (RKInPerInstanceAttributesAtoms instance : atomSource->renderAtoms())
          {
            if (instance.tag != measurement.instanceTag)
            {
              continue;
            }
            instance.ambient = float4(0.0f, 0.0f, 1.0f, 1.0f);
            instance.diffuse = float4(0.0f, 0.0f, 1.0f, 1.0f);
            instance.specular = float4(1.0f, 1.0f, 1.0f, 1.0f);
            instance.tag = static_cast<int32_t>(flatIndex);
            points.push_back(instance);
          }
        }
      }
      ++flatIndex;
    }
  }
  return points;
}

std::vector<RKRenderObject> ProjectStructure::renderMeasurementStructure() const
{
  return std::vector<RKRenderObject>();
}

SKBoundingBox ProjectStructure::renderBoundingBox() const
{
  std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures = _sceneList->selectediRASPAStructures();

  std::vector<std::shared_ptr<iRASPAObject>> flattenedRenderStructures{};
  for(const std::vector<std::shared_ptr<iRASPAObject>> &v : structures)
  {
    std::copy(v.begin(), v.end(), std::back_inserter(flattenedRenderStructures));
  }

   if(flattenedRenderStructures.empty())
   {
     return SKBoundingBox();
   }

   double3 minimum = double3(DBL_MAX, DBL_MAX, DBL_MAX);
   double3 maximum = double3(-DBL_MAX, -DBL_MAX, -DBL_MAX);

   for(const std::shared_ptr<iRASPAObject> &frame: flattenedRenderStructures)
   {
     // for rendering the bounding-box is in the global coordinate space (adding the frame origin)
     SKBoundingBox currentBoundingBox  = frame->object()->transformedBoundingBox() + frame->object()->origin();

     SKBoundingBox transformedBoundingBox = currentBoundingBox;

     minimum.x = std::min(minimum.x, transformedBoundingBox.minimum().x);
     minimum.y = std::min(minimum.y, transformedBoundingBox.minimum().y);
     minimum.z = std::min(minimum.z, transformedBoundingBox.minimum().z);
     maximum.x = std::max(maximum.x, transformedBoundingBox.maximum().x);
     maximum.y = std::max(maximum.y, transformedBoundingBox.maximum().y);
     maximum.z = std::max(maximum.z, transformedBoundingBox.maximum().z);
   }

   return SKBoundingBox(minimum, maximum);
}

bool ProjectStructure::hasSelectedObjects() const
{
  for (const std::vector<std::shared_ptr<iRASPAObject>> &iraspa_structures: _sceneList->selectediRASPAStructures())
  {
    for(const std::shared_ptr<iRASPAObject> &iraspa_structure: iraspa_structures)
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspa_structure->object()))
      {
        if(structure->hasSelectedAtoms())
        {
         return true;
        }
      }
    }
  }
  return false;
}

RKBackgroundType ProjectStructure::renderBackgroundType() const
{
  return _backgroundType;
}

void ProjectStructure::setBackgroundType(RKBackgroundType type)
{
  _backgroundType = type;
}

QColor ProjectStructure::renderBackgroundColor() const
{
   return _backgroundColor;
}



void ProjectStructure::setBackgroundColor(QColor color)
{
 _backgroundColor = color;
}



const QImage ProjectStructure::renderBackgroundCachedImage()
{
  switch(_backgroundType)
  {
    case RKBackgroundType::color:
    default:
    {
      QImage image = QImage(QSize(1024,1024), QImage::Format_ARGB32);
      image.fill(_backgroundColor);
      return image;
    }
    case RKBackgroundType::linearGradient:
    {
      double angle = _backgroundLinearGradientAngle;
      QRectF rect = QRectF(QPointF(0,0),QPointF(1024,1024));
      QPointF startPoint;
      QPointF endPoint;
      double radAngleToEndFromCenter;
      double width = rect.size().width();
      double height = rect.size().height();

      angle = fmod(angle, 360);
      if (angle < 90)
      {
        startPoint = QPointF(0, 0);
        radAngleToEndFromCenter = (angle * 2 - 45) / 180 * M_PI;
      }
      else if (angle < 180)
      {
        startPoint = QPointF(width, 0);
        radAngleToEndFromCenter = ((angle - 90) * 2 + 45) / 180 * M_PI;
      }
      else if (angle < 270)
      {
        startPoint = QPointF(width, height);
        radAngleToEndFromCenter = ((angle - 180) * 2 + 135) / 180 * M_PI;
      }
      else
      {
        startPoint = QPointF(0, height);
        radAngleToEndFromCenter = ((angle - 270) * 2 + 225) / 180 * M_PI;
      }

      endPoint = QPointF( (0.5 + cos(radAngleToEndFromCenter) / sqrt(2.0)) * width,
                          (0.5 + sin(radAngleToEndFromCenter) / sqrt(2.0)) * height);


      QImage image = QImage(QSize(1024,1024), QImage::Format_ARGB32);
      QPainter p(&image);
      QLinearGradient gradient(startPoint,endPoint);
      p.setCompositionMode(QPainter::CompositionMode_Source);

      gradient.setColorAt(0, _backgroundLinearGradientFromColor);
      gradient.setColorAt(1, _backgroundLinearGradientToColor);
      p.fillRect(0, 0, 1024, 1024, gradient);
      return image;
    }
    case RKBackgroundType::radialGradient:
    {
      QImage image = QImage(QSize(1024,1024), QImage::Format_ARGB32);

      double w = image.width()/2;
      double y1 = image.height()/_backgroundRadialGradientRoundness;
      double y2 = -w;

      QPainter p(&image);
      QPointF endCenter = QPointF(image.width()/2.0, y1);
      QPointF startCenter = QPointF(image.width()/2.0, y2);
      double radius = image.width()/2.0;

      QRadialGradient gradient(startCenter, radius, endCenter, radius);
      p.setCompositionMode(QPainter::CompositionMode_Source);
      gradient.setColorAt(0.0, _backgroundRadialGradientFromColor);
      gradient.setColorAt(1.0, _backgroundRadialGradientToColor);
      p.fillRect(0, 0, 1024, 1024, gradient);
      return image;
    }
    case RKBackgroundType::image:
      return _backgroundImage;
  }
}

void ProjectStructure::loadBackgroundImage(QString filename)
{
  QString baseFileName = QFileInfo(filename).fileName();
  _backgroundImageFilename = baseFileName;
  _backgroundImage.load(filename);
}


bool ProjectStructure::showBoundingBox() const
{
  return _showBoundingBox;
}

std::vector<RKInPerInstanceAttributesAtoms> ProjectStructure::renderBoundingBoxSpheres() const
{
  std::vector<RKInPerInstanceAttributesAtoms> data;

  double3 boundingBoxWidths = renderBoundingBox().widths();
  std::array<double3,8> corners = renderBoundingBox().corners();

  double scale = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});
  for(double3 corner: corners)
  {
    RKInPerInstanceAttributesAtoms sphere = RKInPerInstanceAttributesAtoms(
                float4(corner.x,corner.y,corner.z,1.0),
                float4(1.0,1.0,1.0,1.0),
                float4(1.0,1.0,1.0,1.0),
                float4(1.0,1.0,1.0,1.0),
                float4(scale,scale,scale,1.0),
                0);

    data.push_back(sphere);
  }
  return data;
}

std::vector<RKInPerInstanceAttributesBonds> ProjectStructure::renderBoundingBoxCylinders() const
{
  std::vector<RKInPerInstanceAttributesBonds> data;

  double3 boundingBoxWidths = renderBoundingBox().widths();
  std::array<std::pair<double3,double3>,12> sides = renderBoundingBox().sides();

  double scale = 0.0025 * std::max({boundingBoxWidths.x,boundingBoxWidths.y,boundingBoxWidths.z});
  for(std::pair<double3,double3> side: sides)
  {
    RKInPerInstanceAttributesBonds bondData = RKInPerInstanceAttributesBonds(
                float4(side.first,1.0),
                float4(side.second,1.0),
                float4(1.0,1.0,1.0,1.0),
                float4(1.0,1.0,1.0,1.0),
                float4(scale,1.0,scale,1.0),
                0,
                0);
    data.push_back(bondData);
  }

  return data;
}

double ProjectStructure::imageDotsPerInchValue()
{
  switch(imageDPI())
  {
  case RKImageDPI::dpi_72:
    return 72.0;
  case RKImageDPI::dpi_75:
    return 75.0;
  case RKImageDPI::dpi_150:
    return 150.0;
  case RKImageDPI::dpi_300:
  default:
    return 300.0;
  case RKImageDPI::dpi_600:
    return 600.0;
  case RKImageDPI::dpi_1200:
    return 1200.0;
  }
}

size_t ProjectStructure::maxNumberOfMoviesFrames()
{
  size_t maxNumberOfFrames=0;
  for(const std::shared_ptr<Scene> &scene : _sceneList->scenes())
  {
    for(const std::shared_ptr<Movie> &movie : scene->movies())
    {
      maxNumberOfFrames = std::max(movie->frames().size(), maxNumberOfFrames);
    }
  }
  return maxNumberOfFrames;
}


QDataStream &operator<<(QDataStream& stream, const std::shared_ptr<ProjectStructure>& node)
{
  stream << node->_versionNumber;

  stream << node->_showBoundingBox;

  stream << static_cast<typename std::underlying_type<RKBackgroundType>::type>(node->_backgroundType);

  // save picture in PNG format
  QByteArray imageByteArray;
  QBuffer buffer(&imageByteArray);
  buffer.open(QIODevice::WriteOnly);
  node->_backgroundImage.save(&buffer,"PNG");
  stream << imageByteArray;

  stream << node->_backgroundImageFilename;
  stream << node->_backgroundColor;
  stream << node->_backgroundLinearGradientFromColor;
  stream << node->_backgroundLinearGradientToColor;
  stream << node->_backgroundRadialGradientFromColor;
  stream << node->_backgroundRadialGradientToColor;
  stream << node->_backgroundLinearGradientAngle;
  stream << node->_backgroundRadialGradientRoundness;

  stream << node->_renderImagePhysicalSizeInInches;
  stream << node->_renderImageNumberOfPixels;
  stream << node->_aspectRatio;
  stream << static_cast<typename std::underlying_type<RKImageDPI>::type>(node->_imageDPI);
  stream << static_cast<typename std::underlying_type<RKImageUnits>::type>(node->_imageUnits);
  stream << static_cast<typename std::underlying_type<RKImageDimensions>::type>(node->_imageDimensions);
  stream << static_cast<typename std::underlying_type<RKImageQuality>::type>(node->_renderImageQuality);

  stream << node->_movieFramesPerSecond;
  stream << static_cast<typename std::underlying_type<ProjectStructure::MovieType>::type>(node->_movieType);

  stream << node->_camera;
  stream << node->_renderAxes;

  stream << node->_sceneList;

  stream << qint64(0x6f6b6180);

  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::shared_ptr<ProjectStructure>& node)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > node->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "ProjectStructure");
  }

  stream >> node->_showBoundingBox;
  qint64 backgroundType;
  stream >> backgroundType;
  node->_backgroundType = RKBackgroundType(backgroundType);

  // read picture in PNG-format
  QByteArray imageByteArray;
  stream >> imageByteArray;
  QBuffer buffer(&imageByteArray);
  buffer.open(QIODevice::ReadOnly);
  node->_backgroundImage.load(&buffer, "PNG");

  stream >> node->_backgroundImageFilename;
  stream >> node->_backgroundColor;
  stream >> node->_backgroundLinearGradientFromColor;
  stream >> node->_backgroundLinearGradientToColor;
  stream >> node->_backgroundRadialGradientFromColor;
  stream >> node->_backgroundRadialGradientToColor;
  stream >> node->_backgroundLinearGradientAngle;
  stream >> node->_backgroundRadialGradientRoundness;

  stream >> node->_renderImagePhysicalSizeInInches;
  stream >> node->_renderImageNumberOfPixels;
  stream >> node->_aspectRatio;
  qint64 imageDPI;
  stream >> imageDPI;
  node->_imageDPI = RKImageDPI(imageDPI);
  qint64 imageUnits;
  stream >> imageUnits;
  node->_imageUnits = RKImageUnits(imageUnits);
  qint64 imageDimensions;
  stream >> imageDimensions;
  node->_imageDimensions = RKImageDimensions(imageDimensions);
  qint64 renderImageQuality;
  stream >> renderImageQuality;
  node->_renderImageQuality = RKImageQuality(renderImageQuality);

  stream >> node->_movieFramesPerSecond;
  if(versionNumber >= 5) // introduced in version 5
  {
    qint64 movieType;
    stream >> movieType;
    node->_movieType = ProjectStructure::MovieType(movieType);
  }

  stream >> node->_camera;

  if(versionNumber >= 3) // introduced in version 3
  {
    stream >> node->_renderAxes;
  }

  stream >> node->_sceneList;

  if(versionNumber >= 4) // introduced in version 4
  {
    qint64 magicNumber;
    stream >> magicNumber;
    if(magicNumber != qint64(0x6f6b6180))
    {
      throw InvalidArchiveVersionException(__FILE__, __LINE__, "ProjectStructure invalid magic-number");
    }
  }

  return stream;
}

