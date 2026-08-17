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

#include "structureicons.h"

#include <algorithm>
#include <limits>

#include "atomviewer.h"
#include "iraspaobject.h"
#include "iraspaproject.h"
#include "movie.h"
#include "projectstructure.h"
#include "projecttreenode.h"
#include "skatomcopy.h"
#include "skatomtreenode.h"

namespace
{
QIcon iconFromResource(const char *resourcePath)
{
  return QIcon(QString::fromUtf8(resourcePath));
}
} // namespace

QIcon structureInfoPanelIcon(ObjectType type)
{
  switch (type)
  {
  case ObjectType::crystal:
    return iconFromResource(":/iraspa/crystalicon.png");
  case ObjectType::molecularCrystal:
    return iconFromResource(":/iraspa/molecularcrystalicon.png");
  case ObjectType::molecule:
    return iconFromResource(":/iraspa/molecularicon.png");
  case ObjectType::protein:
    return iconFromResource(":/iraspa/proteinicon.png");
  case ObjectType::proteinCrystal:
    return iconFromResource(":/iraspa/proteincrystalicon.png");
  case ObjectType::dna:
    return iconFromResource(":/iraspa/dnaicon.png");
  case ObjectType::dnaCrystal:
    return iconFromResource(":/iraspa/dnacrystalicon.png");
  case ObjectType::crystalEllipsoidPrimitive:
    return iconFromResource(":/iraspa/ellipsoidcrystalicon.png");
  case ObjectType::crystalCylinderPrimitive:
    return iconFromResource(":/iraspa/cylindercrystalicon.png");
  case ObjectType::crystalPolygonalPrismPrimitive:
    return iconFromResource(":/iraspa/prismcrystalicon.png");
  case ObjectType::ellipsoidPrimitive:
    return iconFromResource(":/iraspa/ellipsoidicon.png");
  case ObjectType::cylinderPrimitive:
    return iconFromResource(":/iraspa/cylindericon.png");
  case ObjectType::polygonalPrismPrimitive:
    return iconFromResource(":/iraspa/prismicon.png");
  default:
    return iconFromResource(":/iraspa/genericfoldericon.png");
  }
}

int infoPanelAtomCount(const std::shared_ptr<iRASPAObject> &object)
{
  if (!object || !object->object())
  {
    return 0;
  }
  AtomViewer *viewer = dynamic_cast<AtomViewer *>(object->object().get());
  if (!viewer)
  {
    return 0;
  }
  int count = 0;
  for (const std::shared_ptr<SKAtomTreeNode> &node : viewer->atomsTreeController()->flattenedLeafNodes())
  {
    if (!node || !node->representedObject())
    {
      continue;
    }
    for (const std::shared_ptr<SKAtomCopy> &copy : node->representedObject()->copies())
    {
      if (copy && copy->type() == SKAtomCopy::AtomCopyType::copy)
      {
        ++count;
      }
    }
  }
  return count;
}

QIcon infoPanelIcon(const std::shared_ptr<iRASPAObject> &object)
{
  if (!object)
  {
    return iconFromResource(":/iraspa/genericfoldericon.png");
  }
  return structureInfoPanelIcon(object->type());
}

QIcon infoPanelIcon(const std::shared_ptr<Movie> &movie)
{
  if (!movie || movie->frames().empty())
  {
    return iconFromResource(":/iraspa/genericfoldericon.png");
  }
  return infoPanelIcon(movie->frames().front());
}

QIcon infoPanelIcon(const std::shared_ptr<ProjectTreeNode> &node)
{
  if (!node)
  {
    return iconFromResource(":/iraspa/genericfoldericon.png");
  }
  if (std::shared_ptr<iRASPAProject> project = node->representedObject())
  {
    if (project->isGroup())
    {
      return iconFromResource(":/iraspa/genericfoldericon.png");
    }
    if (std::shared_ptr<ProjectStructure> structure = std::dynamic_pointer_cast<ProjectStructure>(project->project()))
    {
      const std::vector<std::shared_ptr<iRASPAObject>> frames = structure->sceneList()->flattenedAllIRASPAStructures();
      if (!frames.empty())
      {
        return infoPanelIcon(frames.front());
      }
    }
  }
  return iconFromResource(":/iraspa/genericfoldericon.png");
}

QString infoPanelString(const std::shared_ptr<iRASPAObject> &object)
{
  if (!object || !object->object())
  {
    return QString();
  }
  return object->object()->displayName() + QString(" (%1 atoms)").arg(infoPanelAtomCount(object));
}

QString infoPanelString(const std::shared_ptr<Movie> &movie)
{
  if (!movie)
  {
    return QString();
  }
  int minimumNumberOfAtoms = std::numeric_limits<int>::max();
  int maximumNumberOfAtoms = 0;
  for (const std::shared_ptr<iRASPAObject> &frame : movie->frames())
  {
    const int count = infoPanelAtomCount(frame);
    minimumNumberOfAtoms = std::min(minimumNumberOfAtoms, count);
    maximumNumberOfAtoms = std::max(maximumNumberOfAtoms, count);
  }
  if (movie->frames().empty())
  {
    minimumNumberOfAtoms = 0;
  }
  if (minimumNumberOfAtoms == maximumNumberOfAtoms)
  {
    return movie->displayName() + QString(" (%1 atoms)").arg(minimumNumberOfAtoms);
  }
  return movie->displayName() + QString(" (min %1 atoms, max %2 atoms)").arg(minimumNumberOfAtoms).arg(maximumNumberOfAtoms);
}

QString infoPanelString(const std::shared_ptr<ProjectTreeNode> &node)
{
  if (!node)
  {
    return QString();
  }
  if (std::shared_ptr<iRASPAProject> project = node->representedObject())
  {
    if (std::shared_ptr<ProjectStructure> structure = std::dynamic_pointer_cast<ProjectStructure>(project->project()))
    {
      int total = 0;
      for (const std::shared_ptr<iRASPAObject> &frame : structure->sceneList()->flattenedAllIRASPAStructures())
      {
        total += infoPanelAtomCount(frame);
      }
      const QString name = structure->displayName().isEmpty() ? node->displayName() : structure->displayName();
      return name + QString(" (%1 atoms)").arg(total);
    }
    if (std::shared_ptr<Project> generic = project->project())
    {
      if (!generic->displayName().isEmpty())
      {
        return generic->displayName();
      }
    }
  }
  return node->displayName();
}
