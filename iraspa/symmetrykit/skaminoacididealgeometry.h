/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from SymmetryKit SKAminoAcidIdealGeometry.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#pragma once

#include <mathkit.h>
#include <map>
#include <optional>
#include <QString>
#include <vector>

struct SKAminoAcidIdealGeometry
{
  static const std::vector<QString> &replaceableResidueCodes();
  static std::optional<std::map<QString, double3>> idealCoordinates(const QString &residueCode);
  static std::vector<QString> atomNames(const QString &residueCode);
  static std::optional<std::map<QString, double3>> alignedCoordinates(const QString &residueCode,
                                                                      const double3 &actualN,
                                                                      const double3 &actualCA,
                                                                      const double3 &actualC);
};
