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
  this->setWindowTitle("Import structures");
  this->setOption(QFileDialog::DontUseNativeDialog);
  this->setFileMode(QFileDialog::ExistingFiles);

  this->setDirectory(QDir::home());

  this->setNameFilters(QStringList({"all supported files (*.cif *.pdb *.xyz *.vtk *.cube POSCAR CONTCAR CHGCAR ELFCAR LOCPOT)",
                                    "CIF files (*.cif)",
                                    "PDB files (*.pdb)",
                                    "XYZ files (*.xyz)",
                                    "VTK files (*.vtk)",
                                    "CUBE files (*.cube)"
                                    "VASP Files (POSCAR CONTCAR CHGCAR ELFCAR LOCPOT)"}));
  this->selectNameFilter(QString("all supported files (*.cif *.pdb *.xyz *.vtk POSCAR CONTCAR CHGCAR ELFCAR LOCPOT)"));

  QWidget* frame = new QWidget(this);
  QHBoxLayout* horizontalLayout = new QHBoxLayout();
  frame->setLayout(horizontalLayout);

  QWidget* verticalframe = new QWidget(this);
  QVBoxLayout* verticalLayout = new QVBoxLayout();
  verticalframe->setLayout(verticalLayout);

  radioButtonAsProjects = new QRadioButton(this);
  radioButtonAsProjects->setText("As Seperate Projects");
  verticalLayout->addWidget(radioButtonAsProjects);
  radioButtonAsMovies = new QRadioButton(this);
  radioButtonAsMovies->setChecked(true);
  radioButtonAsMovies->setText("As Single Project");
  verticalLayout->addWidget(radioButtonAsMovies);
  radioButtonAsFrames = new QRadioButton(this);
  radioButtonAsFrames->setText("As Movie Frames");
  verticalLayout->addWidget(radioButtonAsFrames);

  horizontalLayout->addWidget(verticalframe);

  horizontalLayout->addSpacing(20);

  QWidget* verticalframe2 = new QWidget(this);
  QVBoxLayout* verticalLayout2 = new QVBoxLayout();
  verticalframe2->setLayout(verticalLayout2);

  checkboxProteinsOnlyAsymmetricUnitCell = new QCheckBox(frame);
  checkboxProteinsOnlyAsymmetricUnitCell->setCheckState(Qt::CheckState::Checked);
  checkboxProteinsOnlyAsymmetricUnitCell->setText("For Proteins Only Asymmetric-Unit");
  verticalLayout2->addWidget(checkboxProteinsOnlyAsymmetricUnitCell);

  checkboxImportAsMolecule = new QCheckBox(frame);
  checkboxImportAsMolecule->setText("As Molecule");
  verticalLayout2->addWidget(checkboxImportAsMolecule);

  horizontalLayout->addWidget(verticalframe2);

  layout()->addWidget(frame);
}
