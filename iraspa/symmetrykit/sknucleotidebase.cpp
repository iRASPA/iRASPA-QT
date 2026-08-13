/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include "sknucleotidebase.h"
#include "sknucleotide.h"

namespace
{
  SKNucleotideBaseKind baseKindFromChar(QChar base)
  {
    switch (base.toLatin1())
    {
    case 'A': return SKNucleotideBaseKind::adenine;
    case 'C': return SKNucleotideBaseKind::cytosine;
    case 'G': return SKNucleotideBaseKind::guanine;
    case 'T': return SKNucleotideBaseKind::thymine;
    case 'U': return SKNucleotideBaseKind::uracil;
    default: return SKNucleotideBaseKind::unknown;
    }
  }
}

namespace SKNucleotideBase
{
  float vertexStructureTypeCode(SKNucleotideBaseKind baseKind, bool backbone)
  {
    if (backbone) return kNucleicBackboneStructureType;
    switch (baseKind)
    {
    case SKNucleotideBaseKind::adenine: return kNucleicAdenineStructureType;
    case SKNucleotideBaseKind::cytosine: return kNucleicCytosineStructureType;
    case SKNucleotideBaseKind::guanine: return kNucleicGuanineStructureType;
    case SKNucleotideBaseKind::thymine:
    case SKNucleotideBaseKind::uracil: return kNucleicThymineStructureType;
    case SKNucleotideBaseKind::unknown:
      break;
    }
    return kNucleicBackboneStructureType;
  }

  SKNucleotideBaseKind baseKindFromResidueName(const QString &residueName)
  {
    const QString name = residueName.trimmed().toUpper();
    if (name == QStringLiteral("A") || name == QStringLiteral("DA") || name == QStringLiteral("ADE") ||
        name == QStringLiteral("RA") || name == QStringLiteral("A5") || name == QStringLiteral("RA5"))
    {
      return SKNucleotideBaseKind::adenine;
    }
    if (name == QStringLiteral("C") || name == QStringLiteral("DC") || name == QStringLiteral("CYT") ||
        name == QStringLiteral("RC") || name == QStringLiteral("C5") || name == QStringLiteral("RC5"))
    {
      return SKNucleotideBaseKind::cytosine;
    }
    if (name == QStringLiteral("G") || name == QStringLiteral("DG") || name == QStringLiteral("GUA") ||
        name == QStringLiteral("RG") || name == QStringLiteral("G5") || name == QStringLiteral("RG5"))
    {
      return SKNucleotideBaseKind::guanine;
    }
    if (name == QStringLiteral("T") || name == QStringLiteral("DT") || name == QStringLiteral("THY"))
    {
      return SKNucleotideBaseKind::thymine;
    }
    if (name == QStringLiteral("U") || name == QStringLiteral("RU") || name == QStringLiteral("URI") ||
        name == QStringLiteral("U5") || name == QStringLiteral("RU5"))
    {
      return SKNucleotideBaseKind::uracil;
    }
    if (name.length() >= 2)
    {
      return baseKindFromChar(name.back());
    }
    return SKNucleotideBaseKind::unknown;
  }

  bool areWatsonCrickComplementary(SKNucleotideBaseKind a, SKNucleotideBaseKind b)
  {
    if (a == SKNucleotideBaseKind::unknown || b == SKNucleotideBaseKind::unknown) return false;
    if (a == b) return false;
    const auto complements = [](SKNucleotideBaseKind x, SKNucleotideBaseKind y)
    {
      return (x == SKNucleotideBaseKind::adenine &&
              (y == SKNucleotideBaseKind::thymine || y == SKNucleotideBaseKind::uracil)) ||
             (y == SKNucleotideBaseKind::adenine &&
              (x == SKNucleotideBaseKind::thymine || x == SKNucleotideBaseKind::uracil)) ||
             (x == SKNucleotideBaseKind::guanine && y == SKNucleotideBaseKind::cytosine) ||
             (y == SKNucleotideBaseKind::guanine && x == SKNucleotideBaseKind::cytosine);
    };
    return complements(a, b);
  }

  bool areWatsonCrickComplementary(const QString &residueNameA, const QString &residueNameB)
  {
    return areWatsonCrickComplementary(baseKindFromResidueName(residueNameA), baseKindFromResidueName(residueNameB));
  }

  std::vector<QString> riboseRingAtomNames()
  {
    return {QStringLiteral("C1'"), QStringLiteral("C2'"), QStringLiteral("C3'"), QStringLiteral("C4'"),
            QStringLiteral("O4'")};
  }

  std::vector<QString> baseRingAtomNames(SKNucleotideBaseKind baseKind)
  {
    switch (baseKind)
    {
    case SKNucleotideBaseKind::cytosine:
    case SKNucleotideBaseKind::thymine:
    case SKNucleotideBaseKind::uracil:
      return {QStringLiteral("N1"), QStringLiteral("C2"), QStringLiteral("N3"),
              QStringLiteral("C4"), QStringLiteral("C5"), QStringLiteral("C6")};
    case SKNucleotideBaseKind::adenine:
    case SKNucleotideBaseKind::guanine:
      return {QStringLiteral("N9"), QStringLiteral("C4"), QStringLiteral("N3"), QStringLiteral("C2"),
              QStringLiteral("N1"), QStringLiteral("C6"), QStringLiteral("C5"), QStringLiteral("N7"),
              QStringLiteral("C8")};
    case SKNucleotideBaseKind::unknown:
      break;
    }
    return {};
  }

  QString baseAnchorAtomName(SKNucleotideBaseKind baseKind)
  {
    switch (baseKind)
    {
    case SKNucleotideBaseKind::cytosine:
    case SKNucleotideBaseKind::thymine:
    case SKNucleotideBaseKind::uracil:
      return QStringLiteral("N1");
    case SKNucleotideBaseKind::adenine:
    case SKNucleotideBaseKind::guanine:
      return QStringLiteral("N9");
    case SKNucleotideBaseKind::unknown:
      break;
    }
    return QString();
  }
}
