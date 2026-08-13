/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "appearancetreewidgetcontroller.h"
#include "appearanceribbonhelpers.h"
#include "dnaribbonmixin.h"
#include "proteinnucleicacidcartoon.h"

namespace
{
  DNARibbonMixin *dnaRibbonMixin(const std::shared_ptr<iRASPAObject> &structure)
  {
    if (!objectIsDNARibbon(structure)) { return nullptr; }
    return dynamic_cast<DNARibbonMixin *>(structure->object().get());
  }

  NucleicAcidBackboneStyle dnaBackboneStyleFromIndex(int index)
  {
    return static_cast<NucleicAcidBackboneStyle>(index);
  }

  NucleicAcidTraceMode dnaTraceModeFromIndex(int index)
  {
    return index == 0 ? NucleicAcidTraceMode::phosphateMode4 : NucleicAcidTraceMode::c3PrimeMode1;
  }

  int dnaTraceModeIndex(NucleicAcidTraceMode mode)
  {
    return mode == NucleicAcidTraceMode::c3PrimeMode1 ? 1 : 0;
  }
}

void AppearanceTreeWidgetController::reloadDNABackboneStyle()
{
  _appearanceRibbonsDNAForm->dnaBackboneStyleComboBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<NucleicAcidBackboneStyle, enum_hash>> values = dnaBackboneStyle(); values)
    {
      _appearanceRibbonsDNAForm->dnaBackboneStyleComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaBackboneStyleComboBox)->setCurrentIndex(static_cast<int>(*(values->begin())));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaBackboneStyleComboBox)->setCurrentIndex(-1);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNATraceMode()
{
  _appearanceRibbonsDNAForm->dnaTraceModeComboBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<NucleicAcidTraceMode, enum_hash>> values = dnaTraceMode(); values)
    {
      _appearanceRibbonsDNAForm->dnaTraceModeComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaTraceModeComboBox)->setCurrentIndex(dnaTraceModeIndex(*(values->begin())));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaTraceModeComboBox)->setCurrentIndex(-1);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNAShowRings()
{
  _appearanceRibbonsDNAForm->dnaShowRingsCheckBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = dnaShowRings(); values)
    {
      _appearanceRibbonsDNAForm->dnaShowRingsCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowRingsCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowRingsCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowRingsCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowRingsCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNAShowLadder()
{
  _appearanceRibbonsDNAForm->dnaShowLadderCheckBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = dnaShowLadder(); values)
    {
      _appearanceRibbonsDNAForm->dnaShowLadderCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowLadderCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowLadderCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowLadderCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsDNAForm->dnaShowLadderCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNAOvalLength()
{
  _appearanceRibbonsDNAForm->dnaOvalLengthDoubleSpinBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaOvalLength(); values && values->size() == 1)
    {
      _appearanceRibbonsDNAForm->dnaOvalLengthDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->dnaOvalLengthDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      whileBlocking(_appearanceRibbonsDNAForm->dnaOvalLengthDoubleSpinBox)->setValue(*(values->begin()));
    }
  }
}

void AppearanceTreeWidgetController::reloadDNAOvalWidth()
{
  _appearanceRibbonsDNAForm->dnaOvalWidthDoubleSpinBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaOvalWidth(); values && values->size() == 1)
    {
      _appearanceRibbonsDNAForm->dnaOvalWidthDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->dnaOvalWidthDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      whileBlocking(_appearanceRibbonsDNAForm->dnaOvalWidthDoubleSpinBox)->setValue(*(values->begin()));
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARingWidth()
{
  _appearanceRibbonsDNAForm->dnaRingWidthDoubleSpinBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRingWidth(); values && values->size() == 1)
    {
      _appearanceRibbonsDNAForm->dnaRingWidthDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->dnaRingWidthDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      whileBlocking(_appearanceRibbonsDNAForm->dnaRingWidthDoubleSpinBox)->setValue(*(values->begin()));
    }
  }
}

void AppearanceTreeWidgetController::reloadDNALadderRadius()
{
  _appearanceRibbonsDNAForm->dnaLadderRadiusDoubleSpinBox->setDisabled(true);
  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaLadderRadius(); values && values->size() == 1)
    {
      _appearanceRibbonsDNAForm->dnaLadderRadiusDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->dnaLadderRadiusDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      whileBlocking(_appearanceRibbonsDNAForm->dnaLadderRadiusDoubleSpinBox)->setValue(*(values->begin()));
    }
  }
}

void AppearanceTreeWidgetController::setDNABackboneStyle(int value)
{
  const NucleicAcidBackboneStyle style = dnaBackboneStyleFromIndex(value);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidBackboneStyle(style);
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNABackboneStyle();
  emit rendererReloadData();
  emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
  emit rendererReloadAmbientOcclusionData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setDNATraceMode(int value)
{
  const NucleicAcidTraceMode mode = dnaTraceModeFromIndex(value);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidTraceMode(mode);
      mixin->rebuildBackbone();
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNATraceMode();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setDNAShowRings(int state)
{
  const NucleicAcidRingMode mode = state ? NucleicAcidRingMode::filledPlanes : NucleicAcidRingMode::off;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidRingMode(mode);
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNAShowRings();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setDNAShowLadder(int state)
{
  const NucleicAcidLadderMode mode = state ? NucleicAcidLadderMode::rungs : NucleicAcidLadderMode::off;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidLadderMode(mode);
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNAShowLadder();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setDNAOvalLength(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidOvalLength(value);
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNAOvalLength();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setDNAOvalWidth(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidOvalWidth(value);
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNAOvalWidth();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setDNARingWidth(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidRingWidth(value);
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNARingWidth();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

void AppearanceTreeWidgetController::setDNALadderRadius(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      mixin->setNucleicAcidLadderRadius(value);
      mixin->rebuildRibbonMesh();
    }
  }
  reloadDNALadderRadius();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<NucleicAcidBackboneStyle, enum_hash>> AppearanceTreeWidgetController::dnaBackboneStyle()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<NucleicAcidBackboneStyle, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidBackboneStyle());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<NucleicAcidBackboneStyle, enum_hash>>(set);
}

std::optional<std::unordered_set<NucleicAcidTraceMode, enum_hash>> AppearanceTreeWidgetController::dnaTraceMode()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<NucleicAcidTraceMode, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidTraceMode());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<NucleicAcidTraceMode, enum_hash>>(set);
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::dnaShowRings()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidRingMode() == NucleicAcidRingMode::filledPlanes);
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::dnaShowLadder()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidLadderMode() == NucleicAcidLadderMode::rungs);
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaOvalLength()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidOvalLength());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaOvalWidth()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidOvalWidth());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRingWidth()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidRingWidth());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaLadderRadius()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (DNARibbonMixin *mixin = dnaRibbonMixin(iraspaStructure))
    {
      set.insert(mixin->nucleicAcidLadderRadius());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}
