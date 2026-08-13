/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "appearancetreewidgetcontroller.h"

#include <QColorDialog>
#include <memory>
#include "appearanceribbonhelpers.h"
#include "dnaribbonmixin.h"
#include "dnaribbonstructureeditor.h"
#include "rkrenderkitprotocols.h"
#include "foundationkit.h"

namespace
{
  DNARibbonMixin *dnaRibbonMixin(const std::shared_ptr<iRASPAObject> &structure)
  {
    if (!objectIsDNARibbon(structure)) { return nullptr; }
    return dynamic_cast<DNARibbonMixin *>(structure->object().get());
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonProperties()
{
  reloadDNADrawRibbonsCheckBox();
  reloadDNARibbonScaleFactor();
  reloadDNABackboneStyle();
  reloadDNATraceMode();
  reloadDNAShowRings();
  reloadDNAShowLadder();
  reloadDNAOvalLength();
  reloadDNAOvalWidth();
  reloadDNARingWidth();
  reloadDNALadderRadius();
  reloadDNARibbonSelectionStyle();
  reloadDNARibbonSelectionFrequency();
  reloadDNARibbonSelectionDensity();
  reloadDNARibbonSelectionIntensity();
  reloadDNARibbonSelectionScaling();
  reloadDNARibbonHighDynamicRange();
  reloadDNARibbonHDRExposure();
  reloadDNARibbonHue();
  reloadDNARibbonSaturation();
  reloadDNARibbonValue();
  reloadDNARibbonAmbientOcclusion();
  reloadDNARibbonAmbientLight();
  reloadDNARibbonDiffuseLight();
  reloadDNARibbonSpecularLight();
  reloadDNARibbonShininess();
}

void AppearanceTreeWidgetController::reloadDNADrawRibbonsCheckBox()
{
  _appearanceRibbonsDNAForm->drawRibbonsCheckBox->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = dnaRibbonDrawRibbons(); values)
    {
      _appearanceRibbonsDNAForm->drawRibbonsCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->drawRibbonsCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsDNAForm->drawRibbonsCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->drawRibbonsCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsDNAForm->drawRibbonsCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonScaleFactor()
{
  _appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonScaleFactor(); values)
    {
      _appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonScaleFactorDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}





void AppearanceTreeWidgetController::reloadDNARibbonSelectionStyle()
{
  _appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonSelectionStyleNuDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonSelectionStyleRhoDoubleSpinBox->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<RKSelectionStyle, enum_hash>> values = dnaRibbonSelectionStyle(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        if (int index = _appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox->findText("Multiple values"); index >= 0)
        {
          whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox)->setCurrentIndex(static_cast<int>(*(values->begin())));
      }
      else
      {
        if (int index = _appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox->findText("Multiple values"); index < 0)
        {
          whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleComboBox)->setCurrentText("Multiple values");
      }
    }

    if (std::optional<std::unordered_set<double>> values = dnaRibbonSelectionStyleNu(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSelectionStyleNuDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleNuDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleNuDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<std::unordered_set<double>> values = dnaRibbonSelectionStyleRho(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSelectionStyleRhoDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleRhoDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionStyleRhoDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonSelectionFrequency()
{
  _appearanceRibbonsDNAForm->ribbonSelectionFrequencyDoubleSpinBox->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonSelectionFrequency(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSelectionFrequencyDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonSelectionFrequencyDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionFrequencyDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionFrequencyDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonSelectionDensity()
{
  _appearanceRibbonsDNAForm->ribbonSelectionDensityDoubleSpinBox->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonSelectionDensity(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSelectionDensityDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonSelectionDensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionDensityDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionDensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonSelectionIntensity()
{
  _appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonSelectionIntensity(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonSelectionScaling()
{
  _appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonSelectionScaling(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSelectionScalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonHighDynamicRange()
{
  _appearanceRibbonsDNAForm->ribbonHighDynamicRangeCheckBox->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = dnaRibbonHighDynamicRange(); values)
    {
      _appearanceRibbonsDNAForm->ribbonHighDynamicRangeCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHighDynamicRangeCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHighDynamicRangeCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHighDynamicRangeCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHighDynamicRangeCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonHDRExposure()
{
  _appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonHDRExposure(); values)
    {
      _appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHDRExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonHue()
{
  _appearanceRibbonsDNAForm->ribbonHueDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonHueDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonHue(); values)
    {
      _appearanceRibbonsDNAForm->ribbonHueDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonHueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonHueDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHueDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHueDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonHueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonSaturation()
{
  _appearanceRibbonsDNAForm->ribbonSaturationDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonSaturationDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonSaturation(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSaturationDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonSaturationDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonSaturationDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSaturationDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSaturationDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSaturationDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonValue()
{
  _appearanceRibbonsDNAForm->ribbonValueDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonValueDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonValue(); values)
    {
      _appearanceRibbonsDNAForm->ribbonValueDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonValueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonValueDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonValueDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonValueDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonValueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonAmbientOcclusion()
{
  _appearanceRibbonsDNAForm->ribbonAmbientOcclusionCheckBox->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = dnaRibbonAmbientOcclusion(); values)
    {
      _appearanceRibbonsDNAForm->ribbonAmbientOcclusionCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonAmbientOcclusionCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsDNAForm->ribbonAmbientOcclusionCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonAmbientOcclusionCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsDNAForm->ribbonAmbientOcclusionCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonAmbientLight()
{
  _appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSlider->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonAmbientColorPushButton->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonAmbientLightIntensity(); values)
    {
      _appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonAmbientIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<QColor> color = dnaRibbonAmbientLightColor(); color)
    {
      _appearanceRibbonsDNAForm->ribbonAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonAmbientColorPushButton->setColor(*color);
    }
    else if (hasDNARibbonStructure(_iraspa_structures))
    {
      _appearanceRibbonsDNAForm->ribbonAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonDiffuseLight()
{
  _appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSlider->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonDiffuseColorPushButton->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonDiffuseLightIntensity(); values)
    {
      _appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonDiffuseIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<QColor> color = dnaRibbonDiffuseLightColor(); color)
    {
      _appearanceRibbonsDNAForm->ribbonDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonDiffuseColorPushButton->setColor(*color);
    }
    else if (hasDNARibbonStructure(_iraspa_structures))
    {
      _appearanceRibbonsDNAForm->ribbonDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonSpecularLight()
{
  _appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSlider->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonSpecularColorPushButton->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonSpecularLightIntensity(); values)
    {
      _appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonSpecularIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<QColor> color = dnaRibbonSpecularLightColor(); color)
    {
      _appearanceRibbonsDNAForm->ribbonSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonSpecularColorPushButton->setColor(*color);
    }
    else if (hasDNARibbonStructure(_iraspa_structures))
    {
      _appearanceRibbonsDNAForm->ribbonSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void AppearanceTreeWidgetController::reloadDNARibbonShininess()
{
  _appearanceRibbonsDNAForm->ribbonShininessDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsDNAForm->ribbonShininessDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasDNARibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = dnaRibbonShininess(); values)
    {
      _appearanceRibbonsDNAForm->ribbonShininessDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsDNAForm->ribbonShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsDNAForm->ribbonShininessDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonShininessDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsDNAForm->ribbonShininessDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsDNAForm->ribbonShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::setDrawDNARibbons(int state)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setDrawRibbon(bool(state));
      if (bool(state))
      {
        editor->rebuildBackbone();
      }
    }
  }
  reloadDNARibbonProperties();
  emit rendererReloadData();
  emit rendererReloadStructureUniforms();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::dnaRibbonDrawRibbons()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->drawRibbon());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonScaleFactorSlider(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonScaleFactor(value);
    }
  }
  reloadDNARibbonScaleFactor();
  emit redrawWithQuality(RKRenderQuality::low);
}

void AppearanceTreeWidgetController::setDNARibbonScaleFactorSpinBox(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonScaleFactor(value);
    }
    if (std::shared_ptr<RKRenderRibbonSource> ribbonSource = std::dynamic_pointer_cast<RKRenderRibbonSource>(iraspaStructure->object()))
    {
      ribbonSource->rebuildRibbonMesh();
    }
  }
  reloadDNARibbonScaleFactor();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonScaleFactor()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonScaleFactor());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}





void AppearanceTreeWidgetController::setDNARibbonSelectionStyle(int value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionStyle(static_cast<RKSelectionStyle>(value));
      }
    }
  }
  reloadDNARibbonSelectionStyle();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<RKSelectionStyle, enum_hash>> AppearanceTreeWidgetController::dnaRibbonSelectionStyle()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<RKSelectionStyle, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionStyle());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<RKSelectionStyle, enum_hash>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSelectionStyleNu(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionWorleyNoise3DFrequency(value);
      }
    }
  }
  reloadDNARibbonSelectionStyle();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSelectionStyleNu()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionWorleyNoise3DFrequency());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSelectionStyleRho(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionWorleyNoise3DJitter(value);
      }
    }
  }
  reloadDNARibbonSelectionStyle();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSelectionStyleRho()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionWorleyNoise3DJitter());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSelectionFrequency(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionFrequency(value);
      }
    }
  }
  reloadDNARibbonSelectionFrequency();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSelectionFrequency()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionFrequency());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSelectionDensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionDensity(value);
      }
    }
  }
  reloadDNARibbonSelectionDensity();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSelectionDensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionDensity());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSelectionIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setSelectionIntensity(value);
      }
    }
  }
  reloadDNARibbonSelectionIntensity();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSelectionIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionIntensity());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSelectionScaling(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionScaling(value);
      }
    }
  }
  reloadDNARibbonSelectionScaling();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSelectionScaling()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionScaling());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonHighDynamicRange(int value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonHDR(bool(value));
    }
  }
  reloadDNARibbonProperties();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::dnaRibbonHighDynamicRange()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonHDR());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonHDRExposure(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonHDRExposure(value);
    }
  }
  reloadDNARibbonHDRExposure();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonHDRExposure()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonHDRExposure());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonHue(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonHue(value);
    }
  }
  reloadDNARibbonHue();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonHue()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonHue());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSaturation(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSaturation(value);
    }
  }
  reloadDNARibbonSaturation();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSaturation()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonSaturation());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonValue(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonValue(value);
    }
  }
  reloadDNARibbonValue();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonValue()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonValue());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonAmbientOcclusion(int value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonAmbientOcclusion(bool(value));
    }
  }
  reloadDNARibbonAmbientOcclusion();
  emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
  emit rendererReloadStructureUniforms();
  emit rendererReloadAmbientOcclusionData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::dnaRibbonAmbientOcclusion()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonAmbientOcclusion());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonAmbientLightIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonAmbientIntensity(value);
    }
  }
  reloadDNARibbonAmbientLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonAmbientLightIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonAmbientIntensity());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
  if (!color.isValid()) return;
  _appearanceRibbonsDNAForm->ribbonAmbientColorPushButton->setColor(color);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonAmbientColor(color);
    }
  }
  reloadDNARibbonAmbientLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<QColor> AppearanceTreeWidgetController::dnaRibbonAmbientLightColor()
{
  std::optional<QColor> color;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      const QColor value = editor->ribbonAmbientColor();
      if (!color)
      {
        color = value;
      }
      else if (*color != value)
      {
        return std::nullopt;
      }
    }
  }
  return color;
}

void AppearanceTreeWidgetController::setDNARibbonDiffuseLightIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonDiffuseIntensity(value);
    }
  }
  reloadDNARibbonDiffuseLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonDiffuseLightIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonDiffuseIntensity());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
  if (!color.isValid()) return;
  _appearanceRibbonsDNAForm->ribbonDiffuseColorPushButton->setColor(color);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonDiffuseColor(color);
    }
  }
  reloadDNARibbonDiffuseLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<QColor> AppearanceTreeWidgetController::dnaRibbonDiffuseLightColor()
{
  std::optional<QColor> color;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      const QColor value = editor->ribbonDiffuseColor();
      if (!color)
      {
        color = value;
      }
      else if (*color != value)
      {
        return std::nullopt;
      }
    }
  }
  return color;
}

void AppearanceTreeWidgetController::setDNARibbonSpecularLightIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSpecularIntensity(value);
    }
  }
  reloadDNARibbonSpecularLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonSpecularLightIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonSpecularIntensity());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setDNARibbonSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
  if (!color.isValid()) return;
  _appearanceRibbonsDNAForm->ribbonSpecularColorPushButton->setColor(color);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSpecularColor(color);
    }
  }
  reloadDNARibbonSpecularLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<QColor> AppearanceTreeWidgetController::dnaRibbonSpecularLightColor()
{
  std::optional<QColor> color;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      const QColor value = editor->ribbonSpecularColor();
      if (!color)
      {
        color = value;
      }
      else if (*color != value)
      {
        return std::nullopt;
      }
    }
  }
  return color;
}

void AppearanceTreeWidgetController::setDNARibbonShininess(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonShininess(value);
    }
  }
  reloadDNARibbonShininess();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::dnaRibbonShininess()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsDNARibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<DNARibbonStructureEditor> editor = std::dynamic_pointer_cast<DNARibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonShininess());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

