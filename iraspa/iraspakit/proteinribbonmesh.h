/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from RenderKit RKRibbonMesh.swift and iRASPAKit ProteinRibbonMesh.swift.
 ********************************************************************************************************************/

#pragma once

#include "rkribbonmesh.h"
#include "proteinbackbone.h"
#include "dnabackbone.h"
#include "proteinribbonmeshparameters.h"
#include "proteinribbonsecondarystructuremethod.h"

struct ProteinRibbonMeshBuilder
{
  static RKRibbonMesh build(const ProteinBackbone &backbone,
                            double radius,
                            double3 contentShift,
                            ProteinRibbonMeshParameters parameters = ProteinRibbonMeshParameters::defaultParameters(),
                            ProteinRibbonSecondaryStructureMethod secondaryStructureMethod = ProteinRibbonSecondaryStructureMethod::stride);

  /// DNA / nucleic-acid cartoon (PyMOL-style phosphate trace, C2'/C3' frame, oval cross-section).
  static RKRibbonMesh buildNucleicAcidRibbon(const DNABackbone &backbone,
                                             const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms,
                                             double radius,
                                             double3 contentShift,
                                             ProteinRibbonMeshParameters parameters = ProteinRibbonMeshParameters::defaultParameters());

  /// @deprecated Use buildNucleicAcidRibbon.
  static RKRibbonMesh buildHelixRibbon(const ProteinBackbone &backbone,
                                       double radius,
                                       double3 contentShift,
                                       ProteinRibbonMeshParameters parameters = ProteinRibbonMeshParameters::defaultParameters());
};
