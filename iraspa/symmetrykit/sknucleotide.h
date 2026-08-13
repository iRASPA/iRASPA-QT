/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#pragma once

#include <QString>

namespace SKNucleotide
{
  bool isNucleotideResidueName(const QString &residueName);
  QString normalizedAtomName(const QString &atomName);
}
