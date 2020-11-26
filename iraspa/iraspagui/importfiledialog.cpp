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

#include "importfiledialog.h"

ImportFileDialog::ImportFileDialog(QWidget *parent) : QFileDialog(parent)
{
  setWindowTitle("Import structures");
  setOption(QFileDialog::DontUseNativeDialog);
  setFileMode(QFileDialog::ExistingFiles);

  setNameFilters(QStringList({"all supported files (*.cif *.pdb POSCAR CONTCAR)","CIF files (*.cif)","PDB files (*.pdb)", "VASP Files (POSCAR CONTCAR)"}));
  selectNameFilter(QString("all supported files (*.cif *.pdb POSCAR CONTCAR)"));

  checkboxSeperateProjects = new QCheckBox(this);
  checkboxSeperateProjects->setText("As seperate projects");
  layout()->addWidget(checkboxSeperateProjects);


  QWidget* frame = new QWidget(this);
  QHBoxLayout* horizontalLayout = new QHBoxLayout();
  frame->setLayout(horizontalLayout);
  checkboxOnlyAsymmetricUnitCell = new QCheckBox(frame);
  checkboxOnlyAsymmetricUnitCell->setText("Only assymmetric unit");
  horizontalLayout->addWidget(checkboxOnlyAsymmetricUnitCell);

  checkboxImportAsMolecule = new QCheckBox(frame);
  checkboxImportAsMolecule->setText("As molecule");
  horizontalLayout->addWidget(checkboxImportAsMolecule);

  layout()->addWidget(frame);
}
