/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "appearancetreewidgetcontroller.h"

#include <QColorDialog>
#include <memory>
#include "appearanceribbonhelpers.h"
#include "atomviewer.h"
#include "ribbonstructureeditor.h"
#include "structure.h"
#include "foundationkit.h"
#include "rkrenderkitprotocols.h"

namespace
{
  bool hasRibbonStructure(const std::vector<std::shared_ptr<iRASPAObject>> &structures)
  {
    return hasProteinRibbonStructure(structures);
  }

  ProteinRibbonSecondaryStructureMethod ribbonSecondaryStructureMethodFromIndex(int index)
  {
    return static_cast<ProteinRibbonSecondaryStructureMethod>(index);
  }

  ProteinRibbonSplineType ribbonSplineTypeFromIndex(int index)
  {
    return static_cast<ProteinRibbonSplineType>(index);
  }

  ProteinRibbonColorSet ribbonColorSetFromIndex(int index)
  {
    return static_cast<ProteinRibbonColorSet>(index);
  }

  ProteinRibbonRepresentationStyle ribbonRepresentationStyleFromIndex(int index)
  {
    const std::vector<ProteinRibbonRepresentationStyle> cases = proteinRibbonRepresentationSelectableCases();
    if (index >= 0 && index < static_cast<int>(cases.size()))
    {
      return cases[static_cast<std::size_t>(index)];
    }
    return ProteinRibbonRepresentationStyle::defaultStyle;
  }

  int ribbonRepresentationStyleIndex(ProteinRibbonRepresentationStyle style)
  {
    const std::vector<ProteinRibbonRepresentationStyle> cases = proteinRibbonRepresentationSelectableCases();
    for (int index = 0; index < static_cast<int>(cases.size()); ++index)
    {
      if (cases[static_cast<std::size_t>(index)] == style)
      {
        return index;
      }
    }
    return -1;
  }
}

void AppearanceTreeWidgetController::reloadRibbonProperties()
{
  reloadDrawRibbonsCheckBox();
  reloadRibbonScaleFactor();
  reloadRibbonSecondaryStructureMethod();
  reloadRibbonSplineType();
  reloadRibbonRepresentationStyle();
  reloadRibbonColorSet();
  reloadRibbonSelectionStyle();
  reloadRibbonSelectionFrequency();
  reloadRibbonSelectionDensity();
  reloadRibbonSelectionIntensity();
  reloadRibbonSelectionScaling();
  reloadRibbonHighDynamicRange();
  reloadRibbonHDRExposure();
  reloadRibbonHue();
  reloadRibbonSaturation();
  reloadRibbonValue();
  reloadRibbonAmbientOcclusion();
  reloadRibbonAmbientLight();
  reloadRibbonDiffuseLight();
  reloadRibbonSpecularLight();
  reloadRibbonShininess();
}

void AppearanceTreeWidgetController::reloadDrawRibbonsCheckBox()
{
  _appearanceRibbonsForm->drawRibbonsCheckBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = ribbonDrawRibbons(); values)
    {
      _appearanceRibbonsForm->drawRibbonsCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->drawRibbonsCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsForm->drawRibbonsCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->drawRibbonsCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsForm->drawRibbonsCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonScaleFactor()
{
  _appearanceRibbonsForm->ribbonScaleFactorDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonScaleFactorDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonScaleFactor(); values)
    {
      _appearanceRibbonsForm->ribbonScaleFactorDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonScaleFactorDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonScaleFactorDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonScaleFactorDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonScaleFactorDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonScaleFactorDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSecondaryStructureMethod()
{
  _appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<ProteinRibbonSecondaryStructureMethod, enum_hash>> values = ribbonSecondaryStructureMethod(); values)
    {
      _appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        if (int index = _appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox->findText("Multiple values"); index >= 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox)->setCurrentIndex(static_cast<int>(*(values->begin())));
      }
      else
      {
        if (int index = _appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox->findText("Multiple values"); index < 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceRibbonsForm->ribbonSecondaryStructureMethodComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSplineType()
{
  _appearanceRibbonsForm->ribbonSplineTypeComboBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<ProteinRibbonSplineType, enum_hash>> values = ribbonSplineType(); values)
    {
      _appearanceRibbonsForm->ribbonSplineTypeComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        if (int index = _appearanceRibbonsForm->ribbonSplineTypeComboBox->findText("Multiple values"); index >= 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonSplineTypeComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceRibbonsForm->ribbonSplineTypeComboBox)->setCurrentIndex(static_cast<int>(*(values->begin())));
      }
      else
      {
        if (int index = _appearanceRibbonsForm->ribbonSplineTypeComboBox->findText("Multiple values"); index < 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonSplineTypeComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceRibbonsForm->ribbonSplineTypeComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonRepresentationStyle()
{
  _appearanceRibbonsForm->ribbonRepresentationStyleComboBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<ProteinRibbonRepresentationStyle, enum_hash>> values = ribbonRepresentationStyle(); values)
    {
      _appearanceRibbonsForm->ribbonRepresentationStyleComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        const ProteinRibbonRepresentationStyle style = *(values->begin());
        if (int index = ribbonRepresentationStyleIndex(style); index >= 0)
        {
          if (int multipleIndex = _appearanceRibbonsForm->ribbonRepresentationStyleComboBox->findText("Multiple values"); multipleIndex >= 0)
          {
            whileBlocking(_appearanceRibbonsForm->ribbonRepresentationStyleComboBox)->removeItem(multipleIndex);
          }
          whileBlocking(_appearanceRibbonsForm->ribbonRepresentationStyleComboBox)->setCurrentIndex(index);
        }
        else
        {
          if (int multipleIndex = _appearanceRibbonsForm->ribbonRepresentationStyleComboBox->findText("Custom"); multipleIndex < 0)
          {
            whileBlocking(_appearanceRibbonsForm->ribbonRepresentationStyleComboBox)->addItem("Custom");
          }
          whileBlocking(_appearanceRibbonsForm->ribbonRepresentationStyleComboBox)->setCurrentText("Custom");
        }
      }
      else
      {
        if (int index = _appearanceRibbonsForm->ribbonRepresentationStyleComboBox->findText("Multiple values"); index < 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonRepresentationStyleComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceRibbonsForm->ribbonRepresentationStyleComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonColorSet()
{
  _appearanceRibbonsForm->ribbonColorSetComboBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<ProteinRibbonColorSet, enum_hash>> values = ribbonColorSet(); values)
    {
      _appearanceRibbonsForm->ribbonColorSetComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        if (int index = _appearanceRibbonsForm->ribbonColorSetComboBox->findText("Multiple values"); index >= 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonColorSetComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceRibbonsForm->ribbonColorSetComboBox)->setCurrentIndex(static_cast<int>(*(values->begin())));
      }
      else
      {
        if (int index = _appearanceRibbonsForm->ribbonColorSetComboBox->findText("Multiple values"); index < 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonColorSetComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceRibbonsForm->ribbonColorSetComboBox)->setCurrentText("Multiple values");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSelectionStyle()
{
  _appearanceRibbonsForm->ribbonSelectionStyleComboBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonSelectionStyleNuDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonSelectionStyleRhoDoubleSpinBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<RKSelectionStyle, enum_hash>> values = ribbonSelectionStyle(); values)
    {
      _appearanceRibbonsForm->ribbonSelectionStyleComboBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        if (int index = _appearanceRibbonsForm->ribbonSelectionStyleComboBox->findText("Multiple values"); index >= 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleComboBox)->removeItem(index);
        }
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleComboBox)->setCurrentIndex(static_cast<int>(*(values->begin())));
      }
      else
      {
        if (int index = _appearanceRibbonsForm->ribbonSelectionStyleComboBox->findText("Multiple values"); index < 0)
        {
          whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleComboBox)->addItem("Multiple values");
        }
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleComboBox)->setCurrentText("Multiple values");
      }
    }

    if (std::optional<std::unordered_set<double>> values = ribbonSelectionStyleNu(); values)
    {
      _appearanceRibbonsForm->ribbonSelectionStyleNuDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleNuDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleNuDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<std::unordered_set<double>> values = ribbonSelectionStyleRho(); values)
    {
      _appearanceRibbonsForm->ribbonSelectionStyleRhoDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleRhoDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionStyleRhoDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSelectionFrequency()
{
  _appearanceRibbonsForm->ribbonSelectionFrequencyDoubleSpinBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonSelectionFrequency(); values)
    {
      _appearanceRibbonsForm->ribbonSelectionFrequencyDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonSelectionFrequencyDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionFrequencyDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionFrequencyDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSelectionDensity()
{
  _appearanceRibbonsForm->ribbonSelectionDensityDoubleSpinBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonSelectionDensity(); values)
    {
      _appearanceRibbonsForm->ribbonSelectionDensityDoubleSpinBox->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonSelectionDensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionDensityDoubleSpinBox)->setValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionDensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSelectionIntensity()
{
  _appearanceRibbonsForm->ribbonSelectionIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonSelectionIntensityDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonSelectionIntensity(); values)
    {
      _appearanceRibbonsForm->ribbonSelectionIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonSelectionIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonSelectionIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSelectionScaling()
{
  _appearanceRibbonsForm->ribbonSelectionScalingDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonSelectionScalingDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonSelectionScaling(); values)
    {
      _appearanceRibbonsForm->ribbonSelectionScalingDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonSelectionScalingDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonSelectionScalingDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionScalingDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionScalingDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSelectionScalingDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonHighDynamicRange()
{
  _appearanceRibbonsForm->ribbonHighDynamicRangeCheckBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = ribbonHighDynamicRange(); values)
    {
      _appearanceRibbonsForm->ribbonHighDynamicRangeCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonHighDynamicRangeCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsForm->ribbonHighDynamicRangeCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonHighDynamicRangeCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsForm->ribbonHighDynamicRangeCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonHDRExposure()
{
  _appearanceRibbonsForm->ribbonHDRExposureDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonHDRExposureDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonHDRExposure(); values)
    {
      _appearanceRibbonsForm->ribbonHDRExposureDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonHDRExposureDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonHDRExposureDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonHDRExposureDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonHDRExposureDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonHDRExposureDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonHue()
{
  _appearanceRibbonsForm->ribbonHueDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonHueDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonHue(); values)
    {
      _appearanceRibbonsForm->ribbonHueDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonHueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonHueDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonHueDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonHueDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonHueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSaturation()
{
  _appearanceRibbonsForm->ribbonSaturationDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonSaturationDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonSaturation(); values)
    {
      _appearanceRibbonsForm->ribbonSaturationDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonSaturationDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonSaturationDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSaturationDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonSaturationDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSaturationDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonValue()
{
  _appearanceRibbonsForm->ribbonValueDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonValueDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonValue(); values)
    {
      _appearanceRibbonsForm->ribbonValueDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonValueDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonValueDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonValueDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonValueDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonValueDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonAmbientOcclusion()
{
  _appearanceRibbonsForm->ribbonAmbientOcclusionCheckBox->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<bool>> values = ribbonAmbientOcclusion(); values)
    {
      _appearanceRibbonsForm->ribbonAmbientOcclusionCheckBox->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonAmbientOcclusionCheckBox)->setTristate(false);
        whileBlocking(_appearanceRibbonsForm->ribbonAmbientOcclusionCheckBox)->setCheckState(*(values->begin()) ? Qt::Checked : Qt::Unchecked);
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonAmbientOcclusionCheckBox)->setTristate(true);
        whileBlocking(_appearanceRibbonsForm->ribbonAmbientOcclusionCheckBox)->setCheckState(Qt::PartiallyChecked);
      }
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonAmbientLight()
{
  _appearanceRibbonsForm->ribbonAmbientIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonAmbientIntensityDoubleSlider->setDisabled(true);
  _appearanceRibbonsForm->ribbonAmbientColorPushButton->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonAmbientLightIntensity(); values)
    {
      _appearanceRibbonsForm->ribbonAmbientIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonAmbientIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonAmbientIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonAmbientIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonAmbientIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonAmbientIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<QColor> color = ribbonAmbientLightColor(); color)
    {
      _appearanceRibbonsForm->ribbonAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonAmbientColorPushButton->setColor(*color);
    }
    else if (hasRibbonStructure(_iraspa_structures))
    {
      _appearanceRibbonsForm->ribbonAmbientColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonDiffuseLight()
{
  _appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSlider->setDisabled(true);
  _appearanceRibbonsForm->ribbonDiffuseColorPushButton->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonDiffuseLightIntensity(); values)
    {
      _appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonDiffuseIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<QColor> color = ribbonDiffuseLightColor(); color)
    {
      _appearanceRibbonsForm->ribbonDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonDiffuseColorPushButton->setColor(*color);
    }
    else if (hasRibbonStructure(_iraspa_structures))
    {
      _appearanceRibbonsForm->ribbonDiffuseColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonSpecularLight()
{
  _appearanceRibbonsForm->ribbonSpecularIntensityDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonSpecularIntensityDoubleSlider->setDisabled(true);
  _appearanceRibbonsForm->ribbonSpecularColorPushButton->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonSpecularLightIntensity(); values)
    {
      _appearanceRibbonsForm->ribbonSpecularIntensityDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonSpecularIntensityDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonSpecularIntensityDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSpecularIntensityDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonSpecularIntensityDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonSpecularIntensityDoubleSpinBox)->setText("Mult. Val.");
      }
    }

    if (std::optional<QColor> color = ribbonSpecularLightColor(); color)
    {
      _appearanceRibbonsForm->ribbonSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonSpecularColorPushButton->setColor(*color);
    }
    else if (hasRibbonStructure(_iraspa_structures))
    {
      _appearanceRibbonsForm->ribbonSpecularColorPushButton->setEnabled(_projectTreeNode->isEditable());
    }
  }
}

void AppearanceTreeWidgetController::reloadRibbonShininess()
{
  _appearanceRibbonsForm->ribbonShininessDoubleSpinBox->setDisabled(true);
  _appearanceRibbonsForm->ribbonShininessDoubleSlider->setDisabled(true);

  if (_projectTreeNode && hasRibbonStructure(_iraspa_structures))
  {
    if (std::optional<std::unordered_set<double>> values = ribbonShininess(); values)
    {
      _appearanceRibbonsForm->ribbonShininessDoubleSpinBox->setEnabled(true);
      _appearanceRibbonsForm->ribbonShininessDoubleSpinBox->setReadOnly(!_projectTreeNode->isEditable());
      _appearanceRibbonsForm->ribbonShininessDoubleSlider->setEnabled(_projectTreeNode->isEditable());
      if (values->size() == 1)
      {
        whileBlocking(_appearanceRibbonsForm->ribbonShininessDoubleSpinBox)->setValue(*(values->begin()));
        whileBlocking(_appearanceRibbonsForm->ribbonShininessDoubleSlider)->setDoubleValue(*(values->begin()));
      }
      else
      {
        whileBlocking(_appearanceRibbonsForm->ribbonShininessDoubleSpinBox)->setText("Mult. Val.");
      }
    }
  }
}

void AppearanceTreeWidgetController::setDrawRibbons(int state)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setDrawRibbon(bool(state));
      if (bool(state))
      {
        editor->rebuildBackbone();
      }
    }
  }
  reloadRibbonProperties();
  emit rendererReloadData();
  emit rendererReloadStructureUniforms();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::ribbonDrawRibbons()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->drawRibbon());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

void AppearanceTreeWidgetController::setRibbonScaleFactorSlider(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonScaleFactor(value);
    }
  }
  reloadRibbonScaleFactor();
  emit redrawWithQuality(RKRenderQuality::low);
}

void AppearanceTreeWidgetController::setRibbonScaleFactorSpinBox(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonScaleFactor(value);
    }
    if (std::shared_ptr<RKRenderRibbonSource> ribbonSource = std::dynamic_pointer_cast<RKRenderRibbonSource>(iraspaStructure->object()))
    {
      ribbonSource->rebuildRibbonMesh();
    }
  }
  reloadRibbonScaleFactor();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonScaleFactor()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonScaleFactor());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSecondaryStructureMethod(int value)
{
  const ProteinRibbonSecondaryStructureMethod method = ribbonSecondaryStructureMethodFromIndex(value);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSecondaryStructureMethod(method);
    }
    if (std::shared_ptr<AtomViewer> atomViewer = std::dynamic_pointer_cast<AtomViewer>(iraspaStructure->object()))
    {
      if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
      {
        rebuildRibbonSecondaryStructureHierarchy(*editor, *atomViewer);
      }
    }
  }
  reloadRibbonProperties();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<ProteinRibbonSecondaryStructureMethod, enum_hash>> AppearanceTreeWidgetController::ribbonSecondaryStructureMethod()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<ProteinRibbonSecondaryStructureMethod, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonSecondaryStructureMethod());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<ProteinRibbonSecondaryStructureMethod, enum_hash>>(set);
}

void AppearanceTreeWidgetController::setRibbonSplineType(int value)
{
  const ProteinRibbonSplineType splineType = ribbonSplineTypeFromIndex(value);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSplineType(splineType);
    }
    if (std::shared_ptr<RKRenderRibbonSource> ribbonSource = std::dynamic_pointer_cast<RKRenderRibbonSource>(iraspaStructure->object()))
    {
      ribbonSource->rebuildRibbonMesh();
    }
  }
  reloadRibbonProperties();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<ProteinRibbonSplineType, enum_hash>> AppearanceTreeWidgetController::ribbonSplineType()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<ProteinRibbonSplineType, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonSplineType());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<ProteinRibbonSplineType, enum_hash>>(set);
}

void AppearanceTreeWidgetController::setRibbonRepresentationStyle(int value)
{
  const ProteinRibbonRepresentationStyle style = ribbonRepresentationStyleFromIndex(value);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      applyRibbonRepresentationStyle(*editor, style);
    }
  }
  reloadRibbonProperties();
  emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
  emit rendererReloadStructureUniforms();
  emit rendererReloadAmbientOcclusionData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<ProteinRibbonRepresentationStyle, enum_hash>> AppearanceTreeWidgetController::ribbonRepresentationStyle()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<ProteinRibbonRepresentationStyle, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonRepresentationStyle());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<ProteinRibbonRepresentationStyle, enum_hash>>(set);
}

void AppearanceTreeWidgetController::setRibbonColorSet(int value)
{
  const ProteinRibbonColorSet colorSet = ribbonColorSetFromIndex(value);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonColorSet(colorSet);
    }
  }
  reloadRibbonColorSet();
  emit rendererReloadStructureUniforms();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<ProteinRibbonColorSet, enum_hash>> AppearanceTreeWidgetController::ribbonColorSet()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<ProteinRibbonColorSet, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonColorSet());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<ProteinRibbonColorSet, enum_hash>>(set);
}

void AppearanceTreeWidgetController::setRibbonSelectionStyle(int value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionStyle(static_cast<RKSelectionStyle>(value));
      }
    }
  }
  reloadRibbonSelectionStyle();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<RKSelectionStyle, enum_hash>> AppearanceTreeWidgetController::ribbonSelectionStyle()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<RKSelectionStyle, enum_hash> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionStyle());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<RKSelectionStyle, enum_hash>>(set);
}

void AppearanceTreeWidgetController::setRibbonSelectionStyleNu(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionWorleyNoise3DFrequency(value);
      }
    }
  }
  reloadRibbonSelectionStyle();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSelectionStyleNu()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionWorleyNoise3DFrequency());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSelectionStyleRho(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionWorleyNoise3DJitter(value);
      }
    }
  }
  reloadRibbonSelectionStyle();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSelectionStyleRho()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionWorleyNoise3DJitter());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSelectionFrequency(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionFrequency(value);
      }
    }
  }
  reloadRibbonSelectionFrequency();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSelectionFrequency()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionFrequency());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSelectionDensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionDensity(value);
      }
    }
  }
  reloadRibbonSelectionDensity();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSelectionDensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionDensity());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSelectionIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setSelectionIntensity(value);
      }
    }
  }
  reloadRibbonSelectionIntensity();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSelectionIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionIntensity());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSelectionScaling(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        structure->setAtomSelectionScaling(value);
      }
    }
  }
  reloadRibbonSelectionScaling();
  emit rendererReloadData();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSelectionScaling()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      if (std::shared_ptr<Structure> structure = std::dynamic_pointer_cast<Structure>(iraspaStructure->object()))
      {
        set.insert(structure->atomSelectionScaling());
      }
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonHighDynamicRange(int value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonHDR(bool(value));
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonProperties();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::ribbonHighDynamicRange()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonHDR());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

void AppearanceTreeWidgetController::setRibbonHDRExposure(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonHDRExposure(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonHDRExposure();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonHDRExposure()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonHDRExposure());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonHue(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonHue(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonHue();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonHue()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonHue());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSaturation(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSaturation(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonSaturation();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSaturation()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonSaturation());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonValue(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonValue(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonValue();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonValue()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonValue());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonAmbientOcclusion(int value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonAmbientOcclusion(bool(value));
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonAmbientOcclusion();
  emit invalidateCachedAmbientOcclusionTextures({_iraspa_structures});
  emit rendererReloadStructureUniforms();
  emit rendererReloadAmbientOcclusionData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<bool>> AppearanceTreeWidgetController::ribbonAmbientOcclusion()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<bool> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonAmbientOcclusion());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<bool>>(set);
}

void AppearanceTreeWidgetController::setRibbonAmbientLightIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonAmbientIntensity(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonAmbientLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonAmbientLightIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonAmbientIntensity());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonAmbientLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
  if (!color.isValid()) return;
  _appearanceRibbonsForm->ribbonAmbientColorPushButton->setColor(color);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonAmbientColor(color);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonAmbientLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<QColor> AppearanceTreeWidgetController::ribbonAmbientLightColor()
{
  std::optional<QColor> color;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
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

void AppearanceTreeWidgetController::setRibbonDiffuseLightIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonDiffuseIntensity(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonDiffuseLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonDiffuseLightIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonDiffuseIntensity());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonDiffuseLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
  if (!color.isValid()) return;
  _appearanceRibbonsForm->ribbonDiffuseColorPushButton->setColor(color);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonDiffuseColor(color);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonDiffuseLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<QColor> AppearanceTreeWidgetController::ribbonDiffuseLightColor()
{
  std::optional<QColor> color;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
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

void AppearanceTreeWidgetController::setRibbonSpecularLightIntensity(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSpecularIntensity(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonSpecularLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonSpecularLightIntensity()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonSpecularIntensity());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

void AppearanceTreeWidgetController::setRibbonSpecularLightColor()
{
  QColor color = QColorDialog::getColor(Qt::white, this, "Choose Color");
  if (!color.isValid()) return;
  _appearanceRibbonsForm->ribbonSpecularColorPushButton->setColor(color);
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonSpecularColor(color);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonSpecularLight();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<QColor> AppearanceTreeWidgetController::ribbonSpecularLightColor()
{
  std::optional<QColor> color;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
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

void AppearanceTreeWidgetController::setRibbonShininess(double value)
{
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      editor->setRibbonShininess(value);
      editor->recheckRibbonRepresentationStyle();
    }
  }
  reloadRibbonShininess();
  emit rendererReloadStructureUniforms();
  emit rendererReloadData();
  emit redrawRenderer();
  _mainWindow->documentWasModified();
}

std::optional<std::unordered_set<double>> AppearanceTreeWidgetController::ribbonShininess()
{
  if (_iraspa_structures.empty()) return std::nullopt;
  std::unordered_set<double> set;
  for (const std::shared_ptr<iRASPAObject> &iraspaStructure : _iraspa_structures)
  {
    if (!objectIsProteinRibbon(iraspaStructure)) { continue; }
    if (std::shared_ptr<ProteinRibbonStructureEditor> editor = std::dynamic_pointer_cast<ProteinRibbonStructureEditor>(iraspaStructure->object()))
    {
      set.insert(editor->ribbonShininess());
    }
  }
  return set.empty() ? std::nullopt : std::optional<std::unordered_set<double>>(set);
}

