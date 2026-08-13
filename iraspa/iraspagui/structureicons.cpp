/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "structureicons.h"

namespace
{
  QIcon iconFromResource(const char *resourcePath)
  {
    return QIcon(QString::fromUtf8(resourcePath));
  }
}

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
