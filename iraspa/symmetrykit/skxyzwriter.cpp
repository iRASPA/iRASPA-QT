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
#include "skelement.h"
#include "skxyzwriter.h"

SKXYZWriter::SKXYZWriter(QString displayName, SKSpaceGroup &spacegroup, std::shared_ptr<SKCell> cell, double3 origin, std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms):
   _displayName(displayName), _spacegroup(spacegroup), _cell(cell), _origin(origin), _atoms(atoms)
{
}

QString SKXYZWriter::string()
{
  QString outputString = QString::number(_atoms.size()) + "\n";

  QString commentString = "# " + _displayName + "\n";
  if(_cell)
  {
    double3x3 unitCell = _cell->unitCell();
    commentString = QString("Lattice=\"%1 %2 %3 %4 %5 %6 %7 %8 %9\"\n").
        arg(unitCell.ax).arg(unitCell.ay).arg(unitCell.az).
        arg(unitCell.bx).arg(unitCell.by).arg(unitCell.bz).
        arg(unitCell.cx).arg(unitCell.cy).arg(unitCell.cz);
  }
  outputString += commentString;

  for(const std::shared_ptr<SKAsymmetricAtom> &atom : _atoms)
  {
    int atomicNumber = atom->elementIdentifier();
    SKElement element = PredefinedElements::predefinedElements[atomicNumber];
    QString elementSymbol =  element._chemicalSymbol.rightJustified(4, ' ', true);

    QString orthogonalCoordinatesForX = QString("%1 ").arg(atom->position().x - _origin.x, 12, 'f', 8, ' ');
    QString orthogonalCoordinatesForY = QString("%1 ").arg(atom->position().y - _origin.y, 12, 'f', 8, ' ');
    QString orthogonalCoordinatesForZ = QString("%1\n").arg(atom->position().z - _origin.z, 12, 'f', 8, ' ');

    QString line = elementSymbol + orthogonalCoordinatesForX + orthogonalCoordinatesForY + orthogonalCoordinatesForZ;

    outputString += line;
  }

  return outputString;
}
