/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include "rkribbonmesh.h"
#include "dnanucleotidegeometry.h"
#include "proteinribbonmeshparameters.h"

struct ProteinNucleicAcidMeshBuilder
{
  /// PyMOL cartoon_ring_mode 1 filled planes + cartoon_ladder_mode 1 rungs.
  static void appendRingAndLadderMeshes(RKRibbonMesh &mesh,
                                        const DNANucleotideGeometry &geometry,
                                        const std::vector<DNANucleotideBasePair> &basePairs,
                                        double3 contentShift,
                                        double radius,
                                        const ProteinRibbonMeshParameters &parameters);
};
