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

#include "structure.h"

class Molecule: public Structure,
                public AtomEditor, public BondEditor
{
public:
  Molecule();
  Molecule(const Molecule &molecule);
  Molecule(std::shared_ptr<SKStructure> frame);

  ~Molecule() {}

  // Object
  // ===============================================================================================
  Molecule(const std::shared_ptr<Object> object);
  ObjectType structureType() override final { return ObjectType::molecule; }
  std::shared_ptr<Object> shallowClone() override final;
  SKBoundingBox boundingBox() const override;
  void reComputeBoundingBox() override;

  // RKRenderAtomSource: overwritten from Structure
  // ===============================================================================================
  std::vector<RKInPerInstanceAttributesAtoms> renderAtoms() const override final;
  std::vector<RKInPerInstanceAttributesAtoms> renderSelectedAtoms() const override final;
  std::vector<RKInPerInstanceAttributesText> atomTextData(RKFontAtlas *fontAtlas) const override final;

  // RKRenderBondSource: overwritten from Structure
  // ===============================================================================================
  std::vector<RKInPerInstanceAttributesBonds> renderInternalBonds() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderSelectedInternalBonds() const override final;

  // AtomEditor
  // ===============================================================================================
  std::shared_ptr<SKAtomTreeController> &atomsTreeController() override {return Structure::atomsTreeController();}
  void setAtomTreeController(std::shared_ptr<SKAtomTreeController> controller) override {Structure::setAtomTreeController(controller);}
  bool isFractional() override final {return false;}
  void expandSymmetry() override final;
  void expandSymmetry(std::shared_ptr<SKAsymmetricAtom> asymmetricAtom);
  std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) override final;
  void clearSelection() override {Structure::clearSelection();}
  void setAtomSelection(int asymmetricAtomId) override {Structure::setAtomSelection(asymmetricAtomId);}
  void addAtomToSelection(int asymmetricAtomId) override {Structure::addAtomToSelection(asymmetricAtomId);}
  void toggleAtomSelection(int asymmetricAtomId) override {Structure::toggleAtomSelection(asymmetricAtomId);}
  void setAtomSelection(std::set<int>& atomIds) override {Structure::setAtomSelection(atomIds);}
  void addToAtomSelection(std::set<int>& atomIds) override {Structure::addToAtomSelection(atomIds);}
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;
  std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const override final;
  void recomputeSelectionBodyFixedBasis() override {Structure::recomputeSelectionBodyFixedBasis();}
  void convertAsymmetricAtomsToFractional() override {Structure::convertAsymmetricAtomsToFractional();}
  void convertAsymmetricAtomsToCartesian() override {Structure::convertAsymmetricAtomsToCartesian();}

  // BondEditor
  // ===============================================================================================
  std::shared_ptr<SKBondSetController> bondSetController() override {return Structure::bondSetController();}
  void setBondSetController(std::shared_ptr<SKBondSetController> controller) override {_bondSetController = controller;}
  BondSelectionIndexSet filterCartesianBondPositions(std::function<bool(double3)> &) override final;
  void computeBonds() override final;
  double bondLength(std::shared_ptr<SKBond> bond) const override;
  void setBondSelection(int asymmetricBondId) override {Structure::setBondSelection(asymmetricBondId);}
  void addBondToSelection(int asymmetricBondId) override {Structure::addBondToSelection(asymmetricBondId);}
  void toggleBondSelection(int asymmetricAtomId) override {Structure::toggleBondSelection(asymmetricAtomId);}


  // Structure: reimplemented
  // ===============================================================================================
  std::shared_ptr<Structure> flattenHierarchy() const override final;
  std::shared_ptr<Structure> appliedCellContentShift() const override final;

  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions() override final;
  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions() override final;

  std::vector<double3> atomPositions() const override final;

  double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms) const override final;
  double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom> > atoms) const override final;

  double3 bondVector(std::shared_ptr<SKBond> bond) const override final;
  std::pair<double3, double3> computeChangedBondLength(std::shared_ptr<SKBond> bond, double bondlength) const override final;

private:
  qint64 _versionNumber{2};
  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<Molecule> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<Molecule> &);

  double3x3 unitCell();
};

