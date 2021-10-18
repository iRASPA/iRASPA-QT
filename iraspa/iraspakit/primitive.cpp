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

#include "primitive.h"
#include "crystal.h"
#include "crystalcylinderprimitive.h"
#include "crystalellipsoidprimitive.h"
#include "crystalpolygonalprismprimitive.h"
#include "gridvolume.h"

Primitive::Primitive(): _atomsTreeController(std::make_shared<SKAtomTreeController>())
{

}

Primitive::Primitive(const Primitive &primitive): Object(primitive), _atomsTreeController(std::make_shared<SKAtomTreeController>())
{

}

std::shared_ptr<Object> Primitive::shallowClone()
{
  return std::make_shared<Primitive>(static_cast<const Primitive&>(*this));
}


Primitive::Primitive(const std::shared_ptr<const Structure> structure): Object(structure), _atomsTreeController(std::make_shared<SKAtomTreeController>())
{
  _cell = std::make_shared<SKCell>(*structure->cell());

  structure->atomsTreeController()->setTags();

  QByteArray byteArray = QByteArray();
  QDataStream stream(&byteArray, QIODevice::WriteOnly);
  stream << structure->atomsTreeController();

  QDataStream streamRead(&byteArray, QIODevice::ReadOnly);
  streamRead >> _atomsTreeController;
}

Primitive::Primitive(const std::shared_ptr<const Primitive> primitive): Object(primitive), _atomsTreeController(std::make_shared<SKAtomTreeController>())
{

}

Primitive::Primitive(const std::shared_ptr<const GridVolume> volume): Object(volume), _atomsTreeController(std::make_shared<SKAtomTreeController>())
{

}


/*
Primitive::Primitive(std::shared_ptr<Object> s): Object(s), _atomsTreeController(std::make_shared<SKAtomTreeController>())
{
       qDebug() << "CALLED!";
  _cell = std::make_shared<SKCell>(*s->cell());



  if(dynamic_cast<Crystal*>(s.get()) ||
     dynamic_cast<CrystalEllipsoidPrimitive*>(s.get()) ||
     dynamic_cast<CrystalCylinderPrimitive*>(s.get()) ||
     dynamic_cast<CrystalPolygonalPrismPrimitive*>(s.get()))
  {
   // convertAsymmetricAtomsToFractional();
  }

  expandSymmetry();
  _atomsTreeController->setTags();
  reComputeBoundingBox();
 // computeBonds();
}*/

// MARK: Symmetry
// =====================================================================

void Primitive::expandSymmetry()
{

}

double3x3 Primitive::matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
{
  Q_UNUSED(atoms);
  return double3x3();
}

double3 Primitive::centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms) const
{
  Q_UNUSED(atoms);
  return double3();
}

void Primitive::recomputeSelectionBodyFixedBasis()
{
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = _atomsTreeController->selectedObjects();
  _selectionCOMTranslation = centerOfMassOfSelectionAsymmetricAtoms(atoms);
  double3x3 inertia = matrixOfInertia(atoms);

  qDebug() << "inertia";
  qDebug() << inertia.ax << ", " << inertia.bx << ", " << inertia.cx;
  qDebug() << inertia.ay << ", " << inertia.by << ", " << inertia.cy;
  qDebug() << inertia.az << ", " << inertia.bz << ", " << inertia.cz;

  double3x3 eigenvectors = inertia;
  double3 eigenvalues{};
  inertia.EigenSystemSymmetric(eigenvalues, eigenvectors);
  _selectionBodyFixedBasis = eigenvectors;

  qDebug() << "_selectionBodyFixedBasis";
  qDebug() << _selectionBodyFixedBasis.ax << ", " << _selectionBodyFixedBasis.bx << ", " << _selectionBodyFixedBasis.cx;
  qDebug() << _selectionBodyFixedBasis.ay << ", " << _selectionBodyFixedBasis.by << ", " << _selectionBodyFixedBasis.cy;
  qDebug() << _selectionBodyFixedBasis.az << ", " << _selectionBodyFixedBasis.bz << ", " << _selectionBodyFixedBasis.cz;
  qDebug() << "eigenvalues:";
  qDebug() << eigenvalues.x << ", " << eigenvalues.y << ", " << eigenvalues.z;

}


double Primitive::primitiveSelectionFrequency() const
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _primitiveSelectionWorleyNoise3DFrequency;
  case RKSelectionStyle::striped:
      return _primitiveSelectionStripesFrequency;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
     return 0.0;
  }
  return 0.0;
}

void Primitive::setPrimitiveSelectionFrequency(double value)
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _primitiveSelectionWorleyNoise3DFrequency = value;
    break;
  case RKSelectionStyle::striped:
    _primitiveSelectionStripesFrequency = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}

double Primitive::primitiveSelectionDensity() const
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    return _primitiveSelectionWorleyNoise3DJitter;
  case RKSelectionStyle::striped:
    return _primitiveSelectionStripesDensity;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    return 0.0;
  }
  return 0.0;
}
void Primitive::setPrimitiveSelectionDensity(double value)
{
  switch(_primitiveSelectionStyle)
  {
  case RKSelectionStyle::WorleyNoise3D:
    _primitiveSelectionWorleyNoise3DJitter = value;
    break;
  case RKSelectionStyle::striped:
    _primitiveSelectionStripesDensity = value;
    break;
  case RKSelectionStyle::None:
  case RKSelectionStyle::glow:
  case RKSelectionStyle::multiple_values:
    break;
  }
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> Primitive::cellForFractionalPositions()
{
  return std::nullopt;
}

std::optional<std::pair<std::shared_ptr<SKCell>, double3>> Primitive::cellForCartesianPositions()
{
  return std::nullopt;
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Primitive::asymmetricAtomsCopiedAndTransformedToCartesianPositions()
{
  return {};
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Primitive::asymmetricAtomsCopiedAndTransformedToFractionalPositions()
{
  return {};
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Primitive::atomsCopiedAndTransformedToCartesianPositions()
{
  return {};
}

std::vector<std::shared_ptr<SKAsymmetricAtom>> Primitive::atomsCopiedAndTransformedToFractionalPositions()
{
  return {};
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Primitive::translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(translation);
  return {};
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Primitive::translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(translation);
  return {};
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Primitive::rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(rotation);
  return {};
}

std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> Primitive::rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const
{
  Q_UNUSED(atoms);
  Q_UNUSED(rotation);
  return {};
}

std::set<int> Primitive::filterCartesianAtomPositions(std::function<bool(double3)> &)
{
  return std::set<int>();
};


void  Primitive::clearSelection()
{
  _atomsTreeController->clearSelection();
}


void Primitive::setAtomSelection(int asymmetricAtomId)
{
  _atomsTreeController->clearSelection();
  addAtomToSelection(asymmetricAtomId);

  recomputeSelectionBodyFixedBasis();
}

void Primitive::addAtomToSelection(int atomId)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes = _atomsTreeController->flattenedLeafNodes();

  std::shared_ptr<SKAtomTreeNode> selectedAtom = atomNodes[atomId];

  _atomsTreeController->insertSelectionIndexPath(selectedAtom->indexPath());

  recomputeSelectionBodyFixedBasis();
}


void Primitive::toggleAtomSelection(int asymmetricAtomId)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes = _atomsTreeController->flattenedLeafNodes();

  std::shared_ptr<SKAtomTreeNode> selectedAtom = atomNodes[asymmetricAtomId];
  IndexPath indexPathSelectedAtom = selectedAtom->indexPath();

  AtomSelectionIndexPaths atomSelection = _atomsTreeController->selectionIndexPaths();

  std::set<IndexPath>::const_iterator search = atomSelection.second.find(indexPathSelectedAtom);
  if (search != atomSelection.second.end())
  {
    _atomsTreeController->removeSelectionIndexPath(*search);
  }
  else
  {
    _atomsTreeController->insertSelectionIndexPath(indexPathSelectedAtom);
  }

  recomputeSelectionBodyFixedBasis();
}


void Primitive::setAtomSelection(std::set<int>& atomIds)
{
  _atomsTreeController->clearSelection();
  addToAtomSelection(atomIds);
  recomputeSelectionBodyFixedBasis();
}

void Primitive::addToAtomSelection(std::set<int>& atomIds)
{
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomNodes = _atomsTreeController->flattenedLeafNodes();

  std::vector<std::shared_ptr<SKAtomCopy>> atomCopies = _atomsTreeController->atomCopies();

  for(int atomId: atomIds)
  {
    std::shared_ptr<SKAtomTreeNode> selectedAtom = atomNodes[atomId];

    _atomsTreeController->insertSelectionIndexPath(selectedAtom->indexPath());
  }

  recomputeSelectionBodyFixedBasis();
}

void Primitive::convertAsymmetricAtomsToCartesian()
{
  double3x3 unitCell = _cell->unitCell();
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomTreeNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &atomTreeNode: atomTreeNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = atomTreeNode->representedObject())
    {
      double3 pos = atom->position();
      atom->setPosition(unitCell * pos);
    }
  }
}

void Primitive::convertAsymmetricAtomsToFractional()
{
  double3x3 inverseUnitCell = _cell->inverseUnitCell();
  std::vector<std::shared_ptr<SKAtomTreeNode>> atomTreeNodes = _atomsTreeController->flattenedLeafNodes();
  for(const std::shared_ptr<SKAtomTreeNode> &atomTreeNode: atomTreeNodes)
  {
    if(std::shared_ptr<SKAsymmetricAtom> atom = atomTreeNode->representedObject())
    {
      double3 pos = atom->position();
      atom->setPosition(inverseUnitCell * pos);
    }
  }
}

QDataStream &operator<<(QDataStream &stream, const std::shared_ptr<Primitive> &primitive)
{
  stream << primitive->_versionNumber;

  stream << primitive->_drawAtoms;
  stream << primitive->_atomsTreeController;

  stream << primitive->_primitiveTransformationMatrix;
  stream << primitive->_primitiveOrientation;
  stream << primitive->_primitiveRotationDelta;
  stream << primitive->_primitiveOpacity;
  stream << primitive->_primitiveIsCapped;
  stream << primitive->_primitiveIsFractional;
  stream << primitive->_primitiveNumberOfSides;
  stream << primitive->_primitiveThickness;

  stream << primitive->_primitiveHue;
  stream << primitive->_primitiveSaturation;
  stream << primitive->_primitiveValue;

  stream << static_cast<typename std::underlying_type<RKSelectionStyle>::type>(primitive->_primitiveSelectionStyle);
  stream << primitive->_primitiveSelectionStripesDensity;
  stream << primitive->_primitiveSelectionStripesFrequency;
  stream << primitive->_primitiveSelectionWorleyNoise3DFrequency;
  stream << primitive->_primitiveSelectionWorleyNoise3DJitter;
  stream << primitive->_primitiveSelectionScaling;
  stream << primitive->_primitiveSelectionIntensity;

  stream << primitive->_primitiveFrontSideHDR;
  stream << primitive->_primitiveFrontSideHDRExposure;
  stream << primitive->_primitiveFrontSideAmbientColor;
  stream << primitive->_primitiveFrontSideDiffuseColor;
  stream << primitive->_primitiveFrontSideSpecularColor;
  stream << primitive->_primitiveFrontSideDiffuseIntensity;
  stream << primitive->_primitiveFrontSideAmbientIntensity;
  stream << primitive->_primitiveFrontSideSpecularIntensity;
  stream << primitive->_primitiveFrontSideShininess;

  stream << primitive->_primitiveBackSideHDR;
  stream << primitive->_primitiveBackSideHDRExposure;
  stream << primitive->_primitiveBackSideAmbientColor;
  stream << primitive->_primitiveBackSideDiffuseColor;
  stream << primitive->_primitiveBackSideSpecularColor;
  stream << primitive->_primitiveBackSideDiffuseIntensity;
  stream << primitive->_primitiveBackSideAmbientIntensity;
  stream << primitive->_primitiveBackSideSpecularIntensity;
  stream << primitive->_primitiveBackSideShininess;

  qDebug() << "CHECK writing primitive";

  // handle super class
  stream << std::static_pointer_cast<Object>(primitive);

  return stream;
}

QDataStream &operator>>(QDataStream &stream, std::shared_ptr<Primitive> &primitive)
{
  qint64 versionNumber;
  stream >> versionNumber;
  if(versionNumber > primitive->_versionNumber)
  {
    throw InvalidArchiveVersionException(__FILE__, __LINE__, "Primitive");
  }

  stream >> primitive->_drawAtoms;
  stream >> primitive->_atomsTreeController;

  stream >> primitive->_primitiveTransformationMatrix;
  stream >> primitive->_primitiveOrientation;
  stream >> primitive->_primitiveRotationDelta;

  stream >> primitive->_primitiveOpacity;
  stream >> primitive->_primitiveIsCapped;
  stream >> primitive->_primitiveIsFractional;
  stream >> primitive->_primitiveNumberOfSides;
  stream >> primitive->_primitiveThickness;

  stream >> primitive->_primitiveHue;
  stream >> primitive->_primitiveSaturation;
  stream >> primitive->_primitiveValue;

  qint64 primitiveSelectionStyle;
  stream >> primitiveSelectionStyle;
  primitive->_primitiveSelectionStyle = RKSelectionStyle(primitiveSelectionStyle);
  stream >> primitive->_primitiveSelectionStripesDensity;
  stream >> primitive->_primitiveSelectionStripesFrequency;
  stream >> primitive->_primitiveSelectionWorleyNoise3DFrequency;
  stream >> primitive->_primitiveSelectionWorleyNoise3DJitter;
  stream >> primitive->_primitiveSelectionScaling;
  stream >> primitive->_primitiveSelectionIntensity;

  stream >> primitive->_primitiveFrontSideHDR;
  stream >> primitive->_primitiveFrontSideHDRExposure;
  stream >> primitive->_primitiveFrontSideAmbientColor;
  stream >> primitive->_primitiveFrontSideDiffuseColor;
  stream >> primitive->_primitiveFrontSideSpecularColor;
  stream >> primitive->_primitiveFrontSideDiffuseIntensity;
  stream >> primitive->_primitiveFrontSideAmbientIntensity;
  stream >> primitive->_primitiveFrontSideSpecularIntensity;
  stream >> primitive->_primitiveFrontSideShininess;

  stream >> primitive->_primitiveBackSideHDR;
  stream >> primitive->_primitiveBackSideHDRExposure;
  stream >> primitive->_primitiveBackSideAmbientColor;
  stream >> primitive->_primitiveBackSideDiffuseColor;
  stream >> primitive->_primitiveBackSideSpecularColor;
  stream >> primitive->_primitiveBackSideDiffuseIntensity;
  stream >> primitive->_primitiveBackSideAmbientIntensity;
  stream >> primitive->_primitiveBackSideSpecularIntensity;
  stream >> primitive->_primitiveBackSideShininess;

  qDebug() << "Reading primitive LInk to super";
  std::shared_ptr<Object> structure = std::static_pointer_cast<Object>(primitive);
  stream >> structure;

  return stream;
}

