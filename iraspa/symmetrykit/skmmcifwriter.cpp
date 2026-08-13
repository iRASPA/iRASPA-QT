/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
 ********************************************************************************************************************/

#include <cmath>
#include <qmath.h>
#include "skelement.h"
#include "skmmcifwriter.h"

SKmmCIFWriter::SKmmCIFWriter(QString displayName,
                             SKSpaceGroup &spaceGroup,
                             std::shared_ptr<SKCell> cell,
                             double3 origin,
                             std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms,
                             bool withProteinInfo):
   _displayName(displayName),
   _spaceGroup(spaceGroup),
   _cell(cell),
   _origin(origin),
   _atoms(std::move(atoms)),
   _withProteinInfo(withProteinInfo)
{
}

QString SKmmCIFWriter::cifDataBlockName(const QString &displayName)
{
  const QString trimmed = displayName.trimmed();
  QString mapped;
  mapped.reserve(trimmed.size());
  for (const QChar &character : trimmed)
  {
    if (character.isLetterOrNumber() || character == QChar('_') || character == QChar('-')) mapped.append(character);
    else mapped.append(QChar('_'));
  }
  return mapped.isEmpty() ? QStringLiteral("structure") : mapped;
}

QString SKmmCIFWriter::characterValue(QChar character, const QString &valueWhenEmpty)
{
  if (character == QChar(' ') || character == QChar('\0')) return valueWhenEmpty;
  return QString(character);
}

QString SKmmCIFWriter::atomName(const std::shared_ptr<SKAsymmetricAtom> &atom, const QString &chemicalElement)
{
  const QString display = atom->displayName().trimmed();
  if (!display.isEmpty())
  {
    return display.contains(QChar(' ')) ? QString("'%1'").arg(display) : display;
  }

  QString name = chemicalElement;
  if (atom->remotenessIndicator() != QChar(' ') && atom->remotenessIndicator() != QChar('\0'))
  {
    name.append(atom->remotenessIndicator());
  }
  if (atom->branchDesignator() != QChar(' ') && atom->branchDesignator() != QChar('\0'))
  {
    name.append(atom->branchDesignator());
  }
  return name;
}

QString SKmmCIFWriter::string()
{
  QString outputString = QString("data_%1\n\n").arg(cifDataBlockName(_displayName));

  if (_cell)
  {
    outputString += QString("_cell.length_a     %1\n").arg(_cell->a(), 12, 'f', 6, ' ');
    outputString += QString("_cell.length_b     %1\n").arg(_cell->b(), 12, 'f', 6, ' ');
    outputString += QString("_cell.length_c     %1\n").arg(_cell->c(), 12, 'f', 6, ' ');
    outputString += QString("_cell.angle_alpha  %1\n").arg(_cell->alpha() * 180.0 / M_PI, 12, 'f', 6, ' ');
    outputString += QString("_cell.angle_beta   %1\n").arg(_cell->beta() * 180.0 / M_PI, 12, 'f', 6, ' ');
    outputString += QString("_cell.angle_gamma  %1\n").arg(_cell->gamma() * 180.0 / M_PI, 12, 'f', 6, ' ');
    if (_cell->zValue() > 0)
    {
      outputString += QString("_cell.Z_PDB        %1\n").arg(_cell->zValue());
    }
    outputString += QString("\n");

    outputString += QString("_symmetry.space_group_name_Hall '%1'\n").arg(_spaceGroup.spaceGroupSetting().HallString());
    outputString += QString("_symmetry.pdbx_full_space_group_name_H-M '%1'\n").arg(_spaceGroup.spaceGroupSetting().HMString());
    outputString += QString("_symmetry.Int_Tables_number %1\n\n").arg(_spaceGroup.spaceGroupSetting().number());
  }

  outputString += QString("loop_\n");
  outputString += QString("_atom_site.group_PDB\n");
  outputString += QString("_atom_site.id\n");
  outputString += QString("_atom_site.type_symbol\n");
  if (_withProteinInfo)
  {
    outputString += QString("_atom_site.label_atom_id\n");
    outputString += QString("_atom_site.label_alt_id\n");
    outputString += QString("_atom_site.label_comp_id\n");
    outputString += QString("_atom_site.label_asym_id\n");
    outputString += QString("_atom_site.label_entity_id\n");
    outputString += QString("_atom_site.label_seq_id\n");
    outputString += QString("_atom_site.pdbx_PDB_ins_code\n");
  }
  else
  {
    outputString += QString("_atom_site.label_atom_id\n");
  }
  outputString += QString("_atom_site.Cartn_x\n");
  outputString += QString("_atom_site.Cartn_y\n");
  outputString += QString("_atom_site.Cartn_z\n");
  outputString += QString("_atom_site.occupancy\n");
  if (_withProteinInfo)
  {
    outputString += QString("_atom_site.auth_seq_id\n");
    outputString += QString("_atom_site.auth_comp_id\n");
    outputString += QString("_atom_site.auth_asym_id\n");
    outputString += QString("_atom_site.auth_atom_id\n");
  }
  outputString += QString("_atom_site.charge\n");

  int serial = 1;
  for (const std::shared_ptr<SKAsymmetricAtom> &atom : _atoms)
  {
    const double3 position = atom->position() - _origin;
    const int atomicNumber = atom->elementIdentifier();
    const SKElement element = PredefinedElements::predefinedElements[atomicNumber];
    const QString chemicalElement = element._chemicalSymbol;
    const QString groupPDB = (_withProteinInfo && atom->solvent()) ? QStringLiteral("HETATM") : QStringLiteral("ATOM");
    const int atomId = atom->serialNumber() > 0 ? static_cast<int>(atom->serialNumber()) : serial;
    const QString name = atomName(atom, chemicalElement);
    const QString positionX = QString("%1").arg(position.x, 12, 'f', 6, ' ');
    const QString positionY = QString("%1").arg(position.y, 12, 'f', 6, ' ');
    const QString positionZ = QString("%1").arg(position.z, 12, 'f', 6, ' ');
    const QString occupancy = QString("%1").arg(atom->occupancy(), 0, 'f', 2);
    const QString charge = QString("%1").arg(atom->charge(), 12, 'f', 6, ' ');

    if (_withProteinInfo)
    {
      const QString residueName = atom->residueName().trimmed().isEmpty() ? QStringLiteral("UNK") : atom->residueName();
      const QString chain = characterValue(atom->chainIdentifier(), QStringLiteral("A"));
      const QString sequenceId = atom->residueSequenceNumber() == 0
                                   ? QStringLiteral("?")
                                   : QString::number(atom->residueSequenceNumber());
      const QString insertionCode = characterValue(atom->codeForInsertionOfResidues(), QStringLiteral("?"));
      const QString altId = characterValue(atom->alternateLocationIndicator(), QStringLiteral("."));

      outputString += QString("%1 %2 %3 %4 %5 %6 %7 ? %8 %9 %10 %11 %12 %13 %14 %15 %16 %17 %18\n")
                        .arg(groupPDB)
                        .arg(atomId)
                        .arg(chemicalElement)
                        .arg(name)
                        .arg(altId)
                        .arg(residueName)
                        .arg(chain)
                        .arg(sequenceId)
                        .arg(insertionCode)
                        .arg(positionX)
                        .arg(positionY)
                        .arg(positionZ)
                        .arg(occupancy)
                        .arg(sequenceId)
                        .arg(residueName)
                        .arg(chain)
                        .arg(name)
                        .arg(charge);
    }
    else
    {
      outputString += QString("%1 %2 %3 %4 %5 %6 %7 %8 %9\n")
                        .arg(groupPDB)
                        .arg(atomId)
                        .arg(chemicalElement)
                        .arg(name)
                        .arg(positionX)
                        .arg(positionY)
                        .arg(positionZ)
                        .arg(occupancy)
                        .arg(charge);
    }
    ++serial;
  }

  return outputString;
}
