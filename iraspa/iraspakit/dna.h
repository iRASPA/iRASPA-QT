/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include "structure.h"
#include "dnaribbonmixin.h"

class DNA: public Structure,
           public AtomEditor, public BondEditor,
           public DNARibbonMixin
{
public:
  DNA();
  DNA(const DNA &dna);
  DNA(std::shared_ptr<SKStructure> frame);

  ~DNA() {}

  DNA(const std::shared_ptr<Object> object);
  ObjectType structureType() override final { return ObjectType::dna; }
  std::shared_ptr<Object> shallowClone() override final;
  SKBoundingBox boundingBox() const override;
  void reComputeBoundingBox() override;

  std::vector<RKInPerInstanceAttributesAtoms> renderAtoms() const override final;
  std::vector<RKInPerInstanceAttributesAtoms> renderSelectedAtoms() const override;
  std::vector<RKInPerInstanceAttributesText> atomTextData(RKFontAtlas *fontAtlas) const override final;

  std::vector<RKInPerInstanceAttributesBonds> renderInternalBonds() const override final;
  std::vector<RKInPerInstanceAttributesBonds> renderSelectedInternalBonds() const override final;

  std::shared_ptr<SKAtomTreeController> &atomsTreeController() override {return Structure::atomsTreeController();}
  void setAtomTreeController(std::shared_ptr<SKAtomTreeController> controller) override {Structure::setAtomTreeController(controller);}
  bool isFractional() override final {return true;}
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

  std::shared_ptr<SKBondSetController> bondSetController() override {return Structure::bondSetController();}
  void setBondSetController(std::shared_ptr<SKBondSetController> controller) override {_bondSetController = controller;}
  BondSelectionIndexSet filterCartesianBondPositions(std::function<bool(double3)> &) override final;
  void computeBonds() override final;
  double bondLength(std::shared_ptr<SKBond> bond) const override;
  void setBondSelection(int asymmetricBondId) override {Structure::setBondSelection(asymmetricBondId);}
  void addBondToSelection(int asymmetricBondId) override {Structure::addBondToSelection(asymmetricBondId);}
  void toggleBondSelection(int asymmetricAtomId) override {Structure::toggleBondSelection(asymmetricAtomId);}

  std::shared_ptr<Structure> flattenHierarchy() const override final;
  std::shared_ptr<Structure> appliedCellContentShift() const override final;

  std::vector<double3> atomPositions() const override final;

  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForFractionalPositions() override final;
  std::optional<std::pair<std::shared_ptr<SKCell>, double3>> cellForCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToFractionalPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToCartesianPositions() override final;
  std::vector<std::shared_ptr<SKAsymmetricAtom>> atomsCopiedAndTransformedToFractionalPositions() override final;

  double3 centerOfMassOfSelectionAsymmetricAtoms(std::vector<std::shared_ptr<SKAsymmetricAtom>> asymmetricAtoms) const override final;
  double3x3 matrixOfInertia(std::vector<std::shared_ptr<SKAsymmetricAtom> > atoms) const override final;

  double3 bondVector(std::shared_ptr<SKBond> bond) const override final;
  std::pair<double3, double3> computeChangedBondLength(std::shared_ptr<SKBond> bond, double bondlength) const override final;

protected:
  SKAtomTreeController &ribbonAtomTreeController() override { return *_atomsTreeController; }
  const SKAtomTreeController &ribbonAtomTreeController() const override { return *_atomsTreeController; }
  double3 ribbonContentShift() const override { return _cell->contentShift(); }

private:
  qint64 _versionNumber{4};
  friend QDataStream &operator<<(QDataStream &, const std::shared_ptr<DNA> &);
  friend QDataStream &operator>>(QDataStream &, std::shared_ptr<DNA> &);
};
