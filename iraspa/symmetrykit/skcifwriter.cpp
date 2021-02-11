/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.
    D.Dubbeldam@uva.nl            https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html
    S.Calero@tue.nl               https://www.tue.nl/en/research/researchers/sofia-calero/
    t.j.h.vlugt@tudelft.nl        http://homepage.tudelft.nl/v9k6y

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************************************************************/

#include <cmath>
#include <qmath.h>
#include "skelement.h"
#include "skcifwriter.h"

SKCIFWriter::SKCIFWriter(QString displayName, SKSpaceGroup &spaceGroup, std::shared_ptr<SKCell> cell, double3 origin, std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms):
   _displayName(displayName), _spaceGroup(spaceGroup), _cell(cell), _origin(origin), _atoms(atoms)
{
}

QString SKCIFWriter::string()
{
  // write local header
  QString outputString = QString("data_") + _displayName + "\n\n";

  // write spacegroup data
  outputString += QString("_cell_length_a %1\n").arg(_cell->a(), 12, 'f', 8, ' ');
  outputString += QString("_cell_length_b %1\n").arg(_cell->b(), 12, 'f', 8, ' ');
  outputString += QString("_cell_length_c %1\n").arg(_cell->c(), 12, 'f', 8, ' ');
  outputString += QString("_cell_angle_alpha %1\n").arg(_cell->alpha() * 180.0 / M_PI, 12, 'f', 8, ' ');
  outputString += QString("_cell_angle_beta %1\n").arg(_cell->beta() * 180.0 / M_PI, 12, 'f', 8, ' ');
  outputString += QString("_cell_angle_gamma %1\n\n").arg(_cell->gamma() * 180.0 / M_PI, 12, 'f', 8, ' ');

  outputString += QString("_symmetry_space_group_name_Hall %1\n").arg(_spaceGroup.spaceGroupSetting().HallString());
  outputString += QString("_symmetry_space_group_name_H-M %1\n").arg(_spaceGroup.spaceGroupSetting().HMString());
  outputString += QString("_symmetry_Int_Tables_number %1\n\n").arg(QString::number(_spaceGroup.spaceGroupSetting().number()));

  // write structure atom data
  outputString += QString("loop_\n");
  outputString += QString("_atom_site_label\n");
  outputString += QString("_atom_site_type_symbol\n");
  outputString += QString("_atom_site_forcefield_label\n");

  outputString += QString("_atom_site_fract_x\n");
  outputString += QString("_atom_site_fract_y\n");
  outputString += QString("_atom_site_fract_z\n");
  outputString += QString("_atom_site_charge\n");

  int counter = 1;
  for(std::shared_ptr<SKAsymmetricAtom> &atom : _atoms)
  {
    int atomicNumber = atom->elementIdentifier();
    SKElement element = PredefinedElements::predefinedElements[atomicNumber];

    QString name = atom->displayName().leftJustified(8, ' ');
    QString chemicalElement =  element._chemicalSymbol.leftJustified(4, ' ', true);

    QString forcefieldName = chemicalElement.leftJustified(5, ' ');
    if(!atom->uniqueForceFieldName().isEmpty())
    {
      forcefieldName = atom->uniqueForceFieldName().leftJustified(5, ' ');
    }

    QString positionX = QString("%1").arg(atom->position().x - _origin.x, 12, 'f', 8, ' ');
    QString positionY = QString("%1").arg(atom->position().y - _origin.y, 12, 'f', 8, ' ');
    QString positionZ = QString("%1").arg(atom->position().z - _origin.z, 12, 'f', 8, ' ');
    QString charge = QString("%1").arg(atom->charge(), 12, 'f', 8, ' ');

    QString line = name + " " + chemicalElement + " " + forcefieldName + " " + positionX + " " + positionY + " " + positionZ + " " + charge + "\n";
    outputString += line;
    counter += 1;
  }

  return outputString;
}
