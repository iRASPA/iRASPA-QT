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

#include "scene.h"
#include "iraspaobject.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <iostream>
#include "skposcarparser.h"
#include "skpdbparser.h"
#include "skcifparser.h"
#include "skxyzparser.h"
#include "skvtkparser.h"

Scene::Scene()
{

}

Scene::Scene(QString displayName): _displayName(displayName)
{
}

// private constructor, parent of movie needs to be set
Scene::Scene(std::shared_ptr<Movie> movie): _movies{movie}
{
}

std::shared_ptr<Scene> Scene::create(std::shared_ptr<Movie> movie)
{
  // std::make_shared can not call private constructor
  std::shared_ptr<Scene> scene =  std::shared_ptr<Scene>( new Scene( std::forward<Scene>(movie)));
  movie->setParent(scene);
  return scene;
}

std::shared_ptr<Movie> Scene::selectedMovie()
{
  if(_selectedMovie)
    return _selectedMovie;
  return nullptr;
}

/*
void Scene::setSelectedFrameIndices(int frameIndex)
{
  for(std::shared_ptr<Movie> movie : _movies)
  {
    movie->setSelectedFrameIndex(frameIndex);
  }
}*/

void Scene::setSelectedMovie(std::shared_ptr<Movie> movie)
{
  _selectedMovie = movie;
  if(_selectedMovie)
  {
    _selectedMovies.insert(_selectedMovie);
  }
}

void Scene::setSelectedMovies(std::set<std::shared_ptr<Movie>> movies)
{
  _selectedMovies = movies;
}

Scene::Scene(QUrl url, const SKColorSets& colorSets, ForceFieldSets& forcefieldSets, [[maybe_unused]] bool asSeparateProject, bool onlyAsymmetricUnit, bool asMolecule)
{
  QFile file(url.toLocalFile());
  QFileInfo info(file);

  std::shared_ptr<SKParser> parser;

   LogReporting *log;

  if((info.fileName().toUpper() == "POSCAR" && info.suffix().isEmpty()) ||
     (info.fileName().toUpper() == "CONTCAR" && info.suffix().isEmpty()) ||
      info.suffix().toLower() == "poscar")
  {
    parser = std::make_shared<SKPOSCARParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }
  else if(info.fileName().toUpper() == "CHGCAR" && info.suffix().isEmpty())
  {
    parser = std::make_shared<SKCHGCARParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }
  else if(info.fileName().toUpper() == "ELFCAR" && info.suffix().isEmpty())
  {
    parser = std::make_shared<SKELFCARParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }
  else if(info.fileName().toUpper() == "LOCPOT" && info.suffix().isEmpty())
  {
    parser = std::make_shared<SKLOCPOTParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }
  else if (info.suffix().toLower() == "cif")
  {
    parser = std::make_shared<SKCIFParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }
  else if (info.suffix().toLower() == "pdb")
  {
    parser = std::make_shared<SKPDBParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }
  else if (info.suffix().toLower() == "xyz")
  {
    parser = std::make_shared<SKXYZParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }
  else if (info.suffix().toLower() == "vtk")
  {
    parser = std::make_shared<SKVTKParser>(url, QDataStream::BigEndian);
  }
  else if (info.suffix().toLower() == "cube")
  {
    qDebug() << "Reading cube";
    parser = std::make_shared<SKGaussianCubeParser>(url, onlyAsymmetricUnit, asMolecule, CharacterSet::whitespaceAndNewlineCharacterSet());
  }

  parser->startParsing();

  std::vector<std::vector<std::shared_ptr<SKStructure>>> movies = parser->movies();

  for (const std::vector<std::shared_ptr<SKStructure>> &movieFrames : movies)
  {
    std::vector<std::shared_ptr<iRASPAObject>> iraspastructures{};
    for (std::shared_ptr<SKStructure> frame : movieFrames)
    {
      std::shared_ptr<iRASPAObject> iraspastructure;

      switch(frame->kind)
      {
      case SKStructure::Kind::crystal:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<Crystal>(frame));
        break;
      case SKStructure::Kind::molecularCrystal:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<MolecularCrystal>(frame));
        break;
      case SKStructure::Kind::molecule:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<Molecule>(frame));
        break;
      case SKStructure::Kind::protein:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<Protein>(frame));
        break;
      case SKStructure::Kind::proteinCrystal:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<ProteinCrystal>(frame));
        break;
      case SKStructure::Kind::RASPADensityVolume:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<RASPADensityVolume>(frame));
        break;
      case SKStructure::Kind::VTKDensityVolume:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<VTKDensityVolume>(frame));
        break;
      case SKStructure::Kind::VASPDensityVolume:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<VASPDensityVolume>(frame));
        break;
      case SKStructure::Kind::GaussianCubeVolume:
        iraspastructure = std::make_shared<iRASPAObject>(std::make_shared<GaussianCubeVolume>(frame));
        break;
      default:
        throw "Unknown structure format";
      }

      if(std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspastructure->object()))
      {
        structure->setRepresentationStyle(Structure::RepresentationStyle::defaultStyle, colorSets);
        structure->setAtomForceFieldIdentifier("Default", forcefieldSets);
        structure->updateForceField(forcefieldSets);

        structure->computeBonds();

        structure->reComputeBoundingBox();
        structure->recomputeDensityProperties();

        //if (log)
        //{
        //  size_t numberOfAtoms = structure->atomsTreeController()->flattenedLeafNodes().size();
        //  log->logMessage(LogReporting::ErrorLevel::info, "Read " + QString::number(numberOfAtoms) + " atoms");
        //}
      }

      iraspastructure->object()->reComputeBoundingBox();

      iraspastructures.push_back(iraspastructure);
    }
    std::shared_ptr<Movie> movie = Movie::create(info.baseName(), iraspastructures);
    _movies.push_back(movie);
  }
}


std::optional<int> Scene::selectMovieIndex()
{
  std::vector<std::shared_ptr<Movie>>::const_iterator itr = std::find(_movies.begin(), _movies.end(), selectedMovie());
  if (itr != _movies.end())
  {
    int row = itr-_movies.begin();
    return row;
  }

  return std::nullopt;
}


std::optional<int> Scene::findChildIndex(std::shared_ptr<Movie> movie)
{
  std::vector<std::shared_ptr<Movie>>::const_iterator itr = std::find(_movies.begin(), _movies.end(), movie);
  if (itr != _movies.end())
  {
    int row = itr-_movies.begin();
    return row;
  }

  return std::nullopt;
}

bool Scene::removeChild(size_t row)
{
  if (row < 0 || row >= _movies.size())
     return false;

  _movies.erase(_movies.begin() + row);
  return true;
}

bool Scene::removeChildren(size_t position, size_t count)
{
  if (position < 0 || position + count > _movies.size())
    return false;
  std::vector<std::shared_ptr<Movie>>::iterator start = _movies.begin() + position;
  std::vector<std::shared_ptr<Movie>>::iterator end = _movies.begin() + position + count;
  _movies.erase(start, end);

  return true;
}

bool Scene::insertChild(size_t row, std::shared_ptr<Movie> child)
{
  if (row < 0 || row > _movies.size())
    return false;

  _movies.insert(_movies.begin() + row, child);
  child->setParent(this->shared_from_this());
  return true;
}

QDataStream &operator<<(QDataStream& stream, const std::vector<std::shared_ptr<Movie>>& val)
{
  stream << static_cast<int32_t>(val.size());
  for(const std::shared_ptr<Movie>& singleVal : val)
    stream << singleVal;
  return stream;
}

QDataStream &operator>>(QDataStream& stream, std::vector<std::shared_ptr<Movie>>& val)
{
  int32_t vecSize;
  val.clear();
  stream >> vecSize;
  val.reserve(vecSize);

  while(vecSize--)
  {
    std::shared_ptr<Movie> tempVal = std::make_shared<Movie>();
    stream >> tempVal;
    val.push_back(tempVal);
  }
  return stream;
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Scene> &scene)
{
  stream << scene->_versionNumber;
  stream << scene->_displayName;
  stream << scene->_movies;
  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Scene> &scene)
{
  qint64 versionNumber;
  stream >> versionNumber;

  if(versionNumber > scene->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Scene");
  }
  stream >> scene->_displayName;
  stream >> scene->_movies;

  for(std::shared_ptr<Movie> movie : scene->_movies)
  {
    movie->setParent(scene);
  }

  return stream;
}
