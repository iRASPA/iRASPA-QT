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
#include <cfloat>
#include <QSize>
#include <array>
#include <algorithm>
#include <QPainter>
#include <QFileInfo>
#include <QBuffer>

ProjectStructure::ProjectStructure(): _camera(std::make_shared<RKCamera>())
{
  _backgroundImage = QImage(QSize(1024,1024), QImage::Format_ARGB32);
  _backgroundImage.fill(QColor(255,255,255,255));
}

ProjectStructure::ProjectStructure(QString filename, SKColorSets& colorSets, ForceFieldSets& forcefieldSets,
                                   bool asSeparateProject, bool onlyAsymmetricUnit, bool asMolecule): _camera(std::make_shared<RKCamera>())
{
  QUrl url = QUrl::fromLocalFile(filename);
  if (url.isValid())
  {
    std::shared_ptr<Scene> scene = std::make_shared<Scene>(url, colorSets, forcefieldSets, asSeparateProject, onlyAsymmetricUnit, asMolecule);
    for(std::shared_ptr<Movie> movie : scene->movies())
    {
      movie->setParent(scene);
    }
    _sceneList->appendScene(scene);
    _camera->resetForNewBoundingBox(this->renderBoundingBox());
  }

  _backgroundImage = QImage(QSize(1024,1024), QImage::Format_ARGB32);
  _backgroundImage.fill(QColor(255,255,255,255));

}

ProjectStructure::ProjectStructure(QList<QUrl>  fileURLs, SKColorSets& colorSets, ForceFieldSets& forcefieldSets,
                                   bool asSeparateProject, bool onlyAsymmetricUnit, bool asMolecule): _camera(std::make_shared<RKCamera>())
{
  foreach (const QUrl &url, fileURLs)
  {
    if (url.isValid())
    {
      std::shared_ptr<Scene> scene = std::make_shared<Scene>(url, colorSets, forcefieldSets, asSeparateProject, onlyAsymmetricUnit, asMolecule);
      for(std::shared_ptr<Movie> movie : scene->movies())
      {
        movie->setParent(scene);
      }
      _sceneList->appendScene(scene);
    }
  }
  _camera->resetForNewBoundingBox(this->renderBoundingBox());

  _backgroundImage = QImage(QSize(1024,1024), QImage::Format_ARGB32);
  _backgroundImage.fill(QColor(255,255,255,255));
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

std::vector<RKInPerInstanceAttributesAtoms> ProjectStructure::renderMeasurementPoints() const
{
  return std::vector<RKInPerInstanceAttributesAtoms>();
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

