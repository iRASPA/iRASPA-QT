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

#include "aboutdialog.h"

#include <QCoreApplication>

// The credits are kept in sync with 'Credits.rtf' and
// 'CreditsOtherContributors.rtf' of the macOS version of iRASPA.
namespace
{
  QString paragraph(const QString &content, const QString &style = QString())
  {
    return QStringLiteral("<p style=\"margin:0px; %1\">%2</p>").arg(style, content);
  }

  QString emptyLine()
  {
    return paragraph(QStringLiteral("<br/>"), QStringLiteral("font-size:6pt;"));
  }

  QString heading(const QString &text)
  {
    return paragraph(text, QStringLiteral("text-decoration:underline;"));
  }

  QString hyperlink(const QString &text, const QString &url)
  {
    return QStringLiteral("<a href=\"%1\">%2</a>").arg(url, text);
  }

  QString credit(const QString &name, const QString &url, const QString &remainder)
  {
    return paragraph(hyperlink(name, url) + remainder);
  }
}

AboutDialog::AboutDialog( QWidget * parent): QDialog(parent)
{
  setupUi(this);
  this->resize(720,490);

  this->setStyleSheet("background-color:rgb(255, 255, 255);");

  textBrowser->setHtml(creditsHtml());
}

QString AboutDialog::creditsHtml() const
{
  QString html = QStringLiteral("<html><body style=\"font-size:11pt; color:#666666;\">");

  html += paragraph(QStringLiteral("iRASPA"),
                    QStringLiteral("font-size:24pt; font-style:italic; color:#000000;"));
  html += paragraph(tr("Version %1").arg(QCoreApplication::applicationVersion()));
  html += emptyLine();

  html += heading(tr("Creators:"));
  html += credit(QStringLiteral("David Dubbeldam"),
                 QStringLiteral("https://www.uva.nl/en/profile/d/u/d.dubbeldam/d.dubbeldam.html"),
                 QStringLiteral(" (University of Amsterdam)"));
  html += credit(QStringLiteral("Jocelyne Vreede"),
                 QStringLiteral("https://www.uva.nl/en/profile/v/r/j.vreede/j.vreede.html"),
                 QStringLiteral(" (University of Amsterdam)"));
  html += credit(QStringLiteral("Sofia Calero"),
                 QStringLiteral("https://www.tue.nl/en/research/researchers/sofia-calero/"),
                 QStringLiteral(" (Eindhoven University of Technology)"));
  html += credit(QStringLiteral("Thijs J.H. Vlugt"),
                 QStringLiteral("http://homepage.tudelft.nl/v9k6y/"),
                 QStringLiteral(" (Delft University of Technology)"));
  html += emptyLine();

  html += paragraph(hyperlink(tr("Link to the article in 'Molecular Simulation Journal' (open access)"),
                              QStringLiteral("http://dx.doi.org/10.1080/08927022.2018.1426855")));
  html += emptyLine();

  html += heading(tr("Acknowledgements:"));
  html += credit(QStringLiteral("Randall Q. Snurr"),
                 QStringLiteral("http://www.iec.northwestern.edu"),
                 QStringLiteral(" (Northwestern University): CoRE MOF database"));
  html += credit(QStringLiteral("Yongchul G. Chung"),
                 QStringLiteral("https://cmcp-group.github.io"),
                 QStringLiteral(" (Pusan National University, South Korea): CoRE MOF database"));
  html += credit(QStringLiteral("Stefan Gustavson"),
                 QStringLiteral("https://github.com/stegu"),
                 QStringLiteral(": GLSL 2D and 3D cellular noise"));
  html += credit(QStringLiteral("Erik Smistad"),
                 QStringLiteral("https://www.eriksmistad.no/marching-cubes-implementation-using-opencl-and-opengl/"),
                 QStringLiteral(": OpenCL/GL implementation Marching Cubes"));
  html += credit(QStringLiteral("Mario Holubar"),
                 QStringLiteral("https://github.com/mario-holubar"),
                 QStringLiteral(": Protein Ribbons"));
  html += emptyLine();

  html += heading(tr("Other contributors:"));
  html += paragraph(QStringLiteral("Tim Becker, Yongchul G. Chung, Noura Dawass, Weina Du, "
                                   "Masafumi Miyaji, Sebastian Caro Ortiz, Anna Pavlova, "
                                   "Ahmadreza Rahbari, Shrinjay Sharma, and "
                                   "Ana Celia Araujo Vila Verde."));

  html += QStringLiteral("</body></html>");

  return html;
}

