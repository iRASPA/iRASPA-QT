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
#include "skpdbwriter.h"

SKPDBWriter::SKPDBWriter(QString displayName, SKSpaceGroup &spacegroup, std::shared_ptr<SKCell> cell, double3 origin, std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms):
   _displayName(displayName), _spacegroup(spacegroup), _cell(cell), _origin(origin), _atoms(atoms)
{
}

QString SKPDBWriter::string()
{
  QString outputString = "COMPND    " + _displayName + "\n" + "AUTHOR    GENERATED BY IRASPA\n";

  if(_cell)
  {
    QString spaceGroupRecordName = "CRYST1";
    QString a = QString("%1").arg(_cell->a(), 9, 'f', 3, ' ');
    QString b = QString("%1").arg(_cell->b(), 9, 'f', 3, ' ');
    QString c = QString("%1").arg(_cell->c(), 9, 'f', 3, ' ');
    QString alpha = QString("%1").arg(_cell->alpha() * 180.0 / M_PI, 7, 'f', 2, ' ');
    QString beta = QString("%1").arg(_cell->beta() * 180.0 / M_PI, 7, 'f', 2, ' ');
    QString gamma = QString("%1").arg(_cell->gamma() * 180.0 / M_PI, 7, 'f', 2, ' ');

    QString spaceGroupString = _spacegroup.spaceGroupSetting().HMString().leftJustified(11, ' ', true);
    QString zValue = QString("%1").arg(_cell->zValue()).rightJustified(4, ' ', true);

    QString line = spaceGroupRecordName + a + b + c + alpha + beta + gamma + " " + spaceGroupString + zValue + "\n";
    outputString += line;
  }

  int counter = 1;
  for(const std::shared_ptr<SKAsymmetricAtom> &atom : _atoms)
  {
    int atomicNumber = atom->elementIdentifier();
    SKElement element = PredefinedElements::predefinedElements[atomicNumber];
    QString elementSymbol =  element._chemicalSymbol.rightJustified(2, ' ', true);
    QString atomRecordName = "ATOM  ";
    QString atomSerialNumber = QString::number(counter).rightJustified(5, ' ', true);
    QString atomName = elementSymbol;
    QString remotenessIndicator = QString(atom->remotenessIndicator());
    QString branchIndicator = QString(atom->branchDesignator());
    QString alternateLocationIndicator = QString(" ");
    QString residueName = atom->residueName().rightJustified(3, ' ', true);
    QString chainIdentifier = QString(atom->chainIdentifier());
    QString residueSequenceNumber = QString::number(atom->residueSequenceNumber()).rightJustified(4, ' ', true);
    QString codeForInsertionOfResidues = QString(atom->codeForInsertionOfResidues());
    QString orthogonalCoordinatesForX =  QString("%1").arg(atom->position().x - _origin.x, 8, 'f', 3, ' ');
    QString orthogonalCoordinatesForY =  QString("%1").arg(atom->position().y - _origin.y, 8, 'f', 3, ' ');
    QString orthogonalCoordinatesForZ =  QString("%1").arg(atom->position().z - _origin.z, 8, 'f', 3, ' ');
    QString occupancy =  QString("%1").arg(atom->occupancy(), 6, 'f', 2, ' ');
    QString isotropicBFactor =  QString("%1").arg(atom->temperaturefactor(), 6, 'f', 2, ' ');
    QString segmentIdentifier = atom->segmentIdentifier().rightJustified(4, ' ', true);
    QString charge = QString("  ");
    QString line = atomRecordName + atomSerialNumber + " " + atomName + remotenessIndicator + branchIndicator +
                   alternateLocationIndicator + residueName + " " + chainIdentifier + residueSequenceNumber +
                   codeForInsertionOfResidues + "   " + orthogonalCoordinatesForX + orthogonalCoordinatesForY +
                   orthogonalCoordinatesForZ + occupancy + isotropicBFactor + "      " +
                   segmentIdentifier + elementSymbol + charge + "\n";
    outputString += line;
    counter += 1;
  }

  outputString += "END\n";

  return outputString;
}
