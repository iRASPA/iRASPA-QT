/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <memory>
#include <vector>
#include <iraspaobject.h>
#include <object.h>
#include "proteinribbonmixin.h"
#include "dnaribbonmixin.h"
#include "primitivestructureviewer.h"

inline bool objectTypeIsPrimitive(ObjectType type)
{
  switch (type)
  {
  case ObjectType::ellipsoidPrimitive:
  case ObjectType::cylinderPrimitive:
  case ObjectType::polygonalPrismPrimitive:
  case ObjectType::crystalEllipsoidPrimitive:
  case ObjectType::crystalCylinderPrimitive:
  case ObjectType::crystalPolygonalPrismPrimitive:
    return true;
  default:
    return false;
  }
}

inline bool objectTypeIsProteinRibbon(ObjectType type)
{
  return type == ObjectType::protein || type == ObjectType::proteinCrystal;
}

inline bool objectTypeIsDNARibbon(ObjectType type)
{
  return type == ObjectType::dna || type == ObjectType::dnaCrystal;
}

inline ObjectType iraspaObjectStructureType(const std::shared_ptr<iRASPAObject> &structure)
{
  if (!structure || !structure->object()) return ObjectType::none;
  return structure->object()->structureType();
}

inline bool objectIsDNARibbon(const std::shared_ptr<iRASPAObject> &structure)
{
  return structure && structure->object() &&
         std::dynamic_pointer_cast<DNARibbonMixin>(structure->object()) != nullptr;
}

inline bool objectIsProteinRibbon(const std::shared_ptr<iRASPAObject> &structure)
{
  if (!structure || !structure->object()) return false;
  const ObjectType type = iraspaObjectStructureType(structure);
  return objectTypeIsProteinRibbon(type) &&
         std::dynamic_pointer_cast<ProteinRibbonMixin>(structure->object()) != nullptr;
}

inline bool hasPrimitiveStructure(const std::vector<std::shared_ptr<iRASPAObject>> &structures)
{
  for (const std::shared_ptr<iRASPAObject> &structure : structures)
  {
    if (objectTypeIsPrimitive(structure->type()) &&
        std::dynamic_pointer_cast<PrimitiveViewer>(structure->object()))
    {
      return true;
    }
  }
  return false;
}

inline bool hasProteinRibbonStructure(const std::vector<std::shared_ptr<iRASPAObject>> &structures)
{
  for (const std::shared_ptr<iRASPAObject> &structure : structures)
  {
    if (objectIsProteinRibbon(structure))
    {
      return true;
    }
  }
  return false;
}

inline bool hasDNARibbonStructure(const std::vector<std::shared_ptr<iRASPAObject>> &structures)
{
  for (const std::shared_ptr<iRASPAObject> &structure : structures)
  {
    if (objectIsDNARibbon(structure))
    {
      return true;
    }
  }
  return false;
}
