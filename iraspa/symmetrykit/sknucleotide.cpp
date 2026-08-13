/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "sknucleotide.h"
#include <set>

namespace
{
  const std::set<QString> &nucleotideResidueNames()
  {
    static const std::set<QString> names = {
      QStringLiteral("A"), QStringLiteral("C"), QStringLiteral("G"), QStringLiteral("T"), QStringLiteral("U"),
      QStringLiteral("DA"), QStringLiteral("DC"), QStringLiteral("DG"), QStringLiteral("DT"),
      QStringLiteral("ADE"), QStringLiteral("CYT"), QStringLiteral("GUA"), QStringLiteral("THY"), QStringLiteral("URI"),
      QStringLiteral("RA"), QStringLiteral("RC"), QStringLiteral("RG"), QStringLiteral("RU"),
      QStringLiteral("A5"), QStringLiteral("C5"), QStringLiteral("G5"), QStringLiteral("U5"),
      QStringLiteral("RA5"), QStringLiteral("RC5"), QStringLiteral("RG5"), QStringLiteral("RU5")
    };
    return names;
  }
}

namespace SKNucleotide
{
  bool isNucleotideResidueName(const QString &residueName)
  {
    return nucleotideResidueNames().count(residueName.trimmed().toUpper()) > 0;
  }

  QString normalizedAtomName(const QString &atomName)
  {
    QString normalized = atomName.trimmed().toUpper();
    normalized.replace(QStringLiteral("*"), QStringLiteral("'"));
    return normalized;
  }
}
