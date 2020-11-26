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
#include "skposcarwriter.h"

SKPOSCARWriter::SKPOSCARWriter(QString displayName, SKSpaceGroup &spaceGroup, std::shared_ptr<SKCell> cell, double3 origin, std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms):
   _displayName(displayName), _spaceGroup(spaceGroup), _cell(cell), _origin(origin), _atoms(atoms)
{

}

QString SKPOSCARWriter::string()
{
  QString outputString  = "# " + _displayName + "\n";
  outputString += "1.00000000000000\n";

  double3x3 unitCell = _cell->unitCell();
  QString cellAX = QString("%1 ").arg(unitCell.ax, 12, 'f', 8, ' ');
  QString cellAY = QString("%1 ").arg(unitCell.ay, 12, 'f', 8, ' ');
  QString cellAZ = QString("%1\n").arg(unitCell.az, 12, 'f', 8, ' ');
  outputString += cellAX + cellAY + cellAZ;

  QString cellBX = QString("%1 ").arg(unitCell.bx, 12, 'f', 8, ' ');
  QString cellBY = QString("%1 ").arg(unitCell.by, 12, 'f', 8, ' ');
  QString cellBZ = QString("%1\n").arg(unitCell.bz, 12, 'f', 8, ' ');
  outputString += cellBX + cellBY + cellBZ;

  QString cellCX = QString("%1 ").arg(unitCell.cx, 12, 'f', 8, ' ');
  QString cellCY = QString("%1 ").arg(unitCell.cy, 12, 'f', 8, ' ');
  QString cellCZ = QString("%1\n").arg(unitCell.cz, 12, 'f', 8, ' ');
  outputString += cellCX + cellCY + cellCZ;

  std::vector<std::shared_ptr<SKAsymmetricAtom>> sortedAtoms{_atoms};
  std::sort(sortedAtoms.begin(), sortedAtoms.end(),
            [](const std::shared_ptr<SKAsymmetricAtom> & a, const std::shared_ptr<SKAsymmetricAtom> & b) -> bool
            {
                return a->elementIdentifier() < b->elementIdentifier();
            });

  std::map<int,int> histogram;

  for (const auto& e : sortedAtoms)
    histogram[e->elementIdentifier()] += 1;

  QString elementString{};
  QString countString{};
  for (const auto& x : histogram)
  {
    SKElement element = PredefinedElements::predefinedElements[x.first];
    elementString += " " + element._chemicalSymbol.leftJustified(4, ' ');
    countString += " " + QString::number(x.second).leftJustified(4, ' ');
  }
  outputString += elementString + "\n";
  outputString += countString + "\n";

  outputString += QString("Direct\n");
  for(std::shared_ptr<SKAsymmetricAtom> atom : sortedAtoms)
  {
    QString positionX = QString("%1").arg(atom->position().x - _origin.x, 12, 'f', 8, ' ');
    QString positionY = QString("%1").arg(atom->position().y - _origin.y, 12, 'f', 8, ' ');
    QString positionZ = QString("%1").arg(atom->position().z - _origin.z, 12, 'f', 8, ' ');
    QString charge = QString("%1").arg(atom->charge(), 12, 'f', 8, ' ');

    QString line = positionX + " " + positionY + " " + positionZ + "\n";
    outputString +=  line;
  }

  return outputString;
}
