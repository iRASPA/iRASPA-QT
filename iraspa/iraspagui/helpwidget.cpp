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

#include "helpwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>

HelpWidget::HelpWidget(QWidget *parent): QMainWindow(parent)
{
  setAttribute(Qt::WA_DeleteOnClose);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
  setAttribute(Qt::WA_GroupLeader);
#endif
  setWindowFlags(Qt::WindowStaysOnTopHint);
  setWindowFlags(Qt::Window);

  QWidget* mainWidget = new QWidget(this);

  textBrowser = new HelpBrowser();

  homeButton = new QPushButton(tr("&Home"));
  forwardButton = new QPushButton(tr("&Forward"));
  backButton = new QPushButton(tr("&Back"));
  closeButton = new QPushButton(tr("Close"));
  closeButton->setShortcut(tr("Esc"));

  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(homeButton);
  buttonLayout->addWidget(forwardButton);
  buttonLayout->addWidget(backButton);
  buttonLayout->addStretch();
  buttonLayout->addWidget(closeButton);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  //mainLayout->addLayout(buttonLayout);
  mainLayout->addWidget(textBrowser);
  mainLayout->setContentsMargins(0,0,0,0);

  this->setCentralWidget(mainWidget);
  mainWidget->setLayout(mainLayout);

  //QObject::connect(homeButton, &QPushButton::clicked, textBrowser, &HelpBrowser::home);
  //QObject::connect(forwardButton, &QPushButton::clicked, textBrowser, &HelpBrowser::forward);
  //QObject::connect(backButton, &QPushButton::clicked, textBrowser, &HelpBrowser::back);
  //QObject::connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

void HelpWidget::updateWindowTitle()
{
  //setWindowTitle(tr("Help: %1").arg(textBrowser->documentTitle()));
}
