/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonStructureEditor.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "ribbonstructureeditor.h"
#include "proteinatomtreebuilder.h"
#include "atomviewer.h"
#include <cmath>

namespace
{
  bool ribbonFloatEqual(double left, double right)
  {
    return std::abs(left - right) < 1e-3;
  }

  bool ribbonColorIsWhite(const QColor &color)
  {
    return ribbonFloatEqual(color.redF(), 1.0)
        && ribbonFloatEqual(color.greenF(), 1.0)
        && ribbonFloatEqual(color.blueF(), 1.0)
        && ribbonFloatEqual(color.alphaF(), 1.0);
  }
}

ProteinRibbonMeshParameters ribbonMeshParameters(const ProteinRibbonStructureEditor &editor)
{
  return ProteinRibbonMeshParameters(editor.ribbonSplineType(),
                                     editor.ribbonSubdivisionsPerSegment(),
                                     editor.ribbonCrossSectionRingResolution(),
                                     editor.ribbonCoilRadiusScale(),
                                     editor.ribbonWidthClamp(),
                                     editor.ribbonSheetArrowLengthExtent(),
                                     editor.ribbonSheetArrowWingPosition(),
                                     editor.ribbonSheetArrowPeakWidthFactor(),
                                     editor.ribbonNormalSmoothingRadius());
}

void setRibbonMeshParameters(ProteinRibbonStructureEditor &editor, const ProteinRibbonMeshParameters &parameters)
{
  editor.setRibbonSplineType(parameters.splineType);
  editor.setRibbonSubdivisionsPerSegment(parameters.subdivisionsPerSegment);
  editor.setRibbonCrossSectionRingResolution(parameters.crossSectionRingResolution);
  editor.setRibbonCoilRadiusScale(parameters.coilRadiusScale);
  editor.setRibbonWidthClamp(parameters.ribbonWidthClamp);
  editor.setRibbonSheetArrowLengthExtent(parameters.sheetArrowLengthExtent);
  editor.setRibbonSheetArrowWingPosition(parameters.sheetArrowWingPosition);
  editor.setRibbonSheetArrowPeakWidthFactor(parameters.sheetArrowPeakWidthFactor);
  editor.setRibbonNormalSmoothingRadius(parameters.normalSmoothingRadius);
}

void migrateLegacySheetArrowDefaultsIfNeeded(ProteinRibbonStructureEditor &editor)
{
  const double length = editor.ribbonSheetArrowLengthExtent();
  const double wing = editor.ribbonSheetArrowWingPosition();
  const double peak = editor.ribbonSheetArrowPeakWidthFactor();
  const bool legacyPaddle = std::abs(length - 2.5) < 1.0e-9 && std::abs(wing - 1.0) < 1.0e-9 && std::abs(peak - 4.0) < 1.0e-9;
  const bool tooSubtle = std::abs(length - 1.5) < 1.0e-9 && std::abs(wing - 0.5) < 1.0e-9 && std::abs(peak - 1.5) < 1.0e-9;
  const bool longDiamond = std::abs(length - 2.0) < 1.0e-9 && std::abs(wing - 1.0) < 1.0e-9 && std::abs(peak - 2.5) < 1.0e-9;
  if (!(legacyPaddle || tooSubtle || longDiamond)) return;
  editor.setRibbonSheetArrowLengthExtent(1.5);
  editor.setRibbonSheetArrowWingPosition(1.0);
  editor.setRibbonSheetArrowPeakWidthFactor(2.5);
}

void applyDefaultRibbonAppearance(ProteinRibbonStructureEditor &editor)
{
  editor.setRibbonHDR(true);
  editor.setRibbonHDRExposure(1.5);
  editor.setRibbonHue(1.0);
  editor.setRibbonSaturation(1.0);
  editor.setRibbonValue(1.0);
  editor.setRibbonAmbientOcclusion(false);
  editor.setRibbonAmbientColor(QColor(255, 255, 255));
  editor.setRibbonDiffuseColor(QColor(255, 255, 255));
  editor.setRibbonSpecularColor(QColor(255, 255, 255));
  editor.setRibbonAmbientIntensity(0.2);
  editor.setRibbonDiffuseIntensity(1.0);
  editor.setRibbonSpecularIntensity(1.0);
  editor.setRibbonShininess(6.0);
  editor.setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle::defaultStyle);
}

void applyFancyRibbonAppearanceDefault(ProteinRibbonStructureEditor &editor)
{
  editor.setRibbonHDR(true);
  editor.setRibbonHDRExposure(2.5);
  editor.setRibbonHue(1.0);
  editor.setRibbonSaturation(1.0);
  editor.setRibbonValue(1.0);
  editor.setRibbonAmbientOcclusion(true);
  editor.setRibbonAmbientColor(QColor(255, 255, 255));
  editor.setRibbonDiffuseColor(QColor(255, 255, 255));
  editor.setRibbonSpecularColor(QColor(255, 255, 255));
  editor.setRibbonAmbientIntensity(0.2);
  editor.setRibbonDiffuseIntensity(1.0);
  editor.setRibbonSpecularIntensity(1.0);
  editor.setRibbonShininess(4.0);
  editor.setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle::fancy);
}

void applyRibbonRepresentationStyle(ProteinRibbonStructureEditor &editor, ProteinRibbonRepresentationStyle style)
{
  editor.setRibbonRepresentationStyle(style);
  switch (style)
  {
  case ProteinRibbonRepresentationStyle::defaultStyle:
    applyDefaultRibbonAppearance(editor);
    break;
  case ProteinRibbonRepresentationStyle::fancy:
    applyFancyRibbonAppearanceDefault(editor);
    break;
  case ProteinRibbonRepresentationStyle::custom:
    break;
  }
}

bool matchesDefaultRibbonAppearance(const ProteinRibbonStructureEditor &editor)
{
  return editor.ribbonHDR()
      && ribbonFloatEqual(editor.ribbonHDRExposure(), 1.5)
      && ribbonFloatEqual(editor.ribbonHue(), 1.0)
      && ribbonFloatEqual(editor.ribbonSaturation(), 1.0)
      && ribbonFloatEqual(editor.ribbonValue(), 1.0)
      && !editor.ribbonAmbientOcclusion()
      && ribbonColorIsWhite(editor.ribbonAmbientColor())
      && ribbonColorIsWhite(editor.ribbonDiffuseColor())
      && ribbonColorIsWhite(editor.ribbonSpecularColor())
      && ribbonFloatEqual(editor.ribbonAmbientIntensity(), 0.2)
      && ribbonFloatEqual(editor.ribbonDiffuseIntensity(), 1.0)
      && ribbonFloatEqual(editor.ribbonSpecularIntensity(), 1.0)
      && ribbonFloatEqual(editor.ribbonShininess(), 6.0);
}

bool matchesFancyRibbonAppearance(const ProteinRibbonStructureEditor &editor)
{
  return editor.ribbonHDR()
      && ribbonFloatEqual(editor.ribbonHDRExposure(), 2.5)
      && ribbonFloatEqual(editor.ribbonHue(), 1.0)
      && ribbonFloatEqual(editor.ribbonSaturation(), 1.0)
      && ribbonFloatEqual(editor.ribbonValue(), 1.0)
      && editor.ribbonAmbientOcclusion()
      && ribbonColorIsWhite(editor.ribbonAmbientColor())
      && ribbonColorIsWhite(editor.ribbonDiffuseColor())
      && ribbonColorIsWhite(editor.ribbonSpecularColor())
      && ribbonFloatEqual(editor.ribbonAmbientIntensity(), 0.2)
      && ribbonFloatEqual(editor.ribbonDiffuseIntensity(), 1.0)
      && ribbonFloatEqual(editor.ribbonSpecularIntensity(), 1.0)
      && ribbonFloatEqual(editor.ribbonShininess(), 4.0);
}

void recheckRibbonRepresentationStyle(ProteinRibbonStructureEditor &editor)
{
  if (matchesDefaultRibbonAppearance(editor))
  {
    editor.setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle::defaultStyle);
  }
  else if (matchesFancyRibbonAppearance(editor))
  {
    editor.setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle::fancy);
  }
  else
  {
    editor.setRibbonRepresentationStyle(ProteinRibbonRepresentationStyle::custom);
  }
}

void rebuildRibbonSecondaryStructureHierarchy(ProteinRibbonStructureEditor &editor, AtomViewer &atomViewer)
{
  const std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms = atomViewer.atomsTreeController()->flattenedObjects();
  const std::vector<std::shared_ptr<SKAtomTreeNode>> newRoots =
    ProteinAtomTreeBuilder::build(atoms, editor.ribbonSecondaryStructureMethod());
  std::vector<std::shared_ptr<SKAtomTreeNode>> oldRoots = atomViewer.atomsTreeController()->rootNodes();
  for (const std::shared_ptr<SKAtomTreeNode> &root : oldRoots)
  {
    atomViewer.atomsTreeController()->removeNode(root);
  }
  for (const std::shared_ptr<SKAtomTreeNode> &node : newRoots)
  {
    atomViewer.atomsTreeController()->appendToRootnodes(node);
  }
  atomViewer.atomsTreeController()->setTags();
  editor.rebuildBackbone();
}
