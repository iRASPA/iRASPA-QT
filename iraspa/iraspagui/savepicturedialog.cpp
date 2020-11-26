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

#include <QStandardPaths>
#include "savepicturedialog.h"

SavePictureDialog::SavePictureDialog(QWidget *parent) : QFileDialog(parent)
{
  setWindowTitle("Save picture");
  setOption(QFileDialog::DontUseNativeDialog);
  setFileMode(QFileDialog::AnyFile);
  setAcceptMode(QFileDialog::AcceptSave);

  setNameFilters(QStringList({"PNG files (*.png)","TIFF files (*.tiff)", "Images (*.png *.xpm *.jpg *.tiff)"}));
  selectNameFilter(QString("PNG files (*.png)"));

  QDir desktopDir(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
  setDirectory(desktopDir);
  selectFile("picture.png");

  QGridLayout* mainLayout = dynamic_cast <QGridLayout*>(this->layout());

  _textBrowser = new QTextBrowser(this);
  _textBrowser->setOpenExternalLinks(true);
  _textBrowser->setHtml("For use in scientific publications, please cite:<br> "
                        "D.Dubbeldam, S. Calero, and T.j.H. Vlugt,<br> "
                        "<a href=\"http://dx.doi.org/10.1080/08927022.2018.1426855\">\"iRASPA, GPU-Accelerated Visualization Software for Materials Scientists\"</a><br> "
                        "Mol. Simulat., DOI: 10.1080/08927022.2018.1426855, 2018.");

  int num_rows = mainLayout->rowCount();
  mainLayout->addWidget(_textBrowser,num_rows,0,1,-1);
}
