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

#pragma once

#include <QWidget>
#include <QListWidget>
#include <QObject>
#include <QListView>
#include <optional>
#include "elementsform.h"
#include "iraspamainwindowconsumerprotocol.h"
#include "ui_elementsform.h"

class ElementListWidgetController: public QListWidget, public MainWindowConsumer, public ProjectConsumer
{
  Q_OBJECT
public:
  ElementListWidgetController(QWidget* parent);
  void setProject(std::shared_ptr<ProjectTreeNode> projectTreeNode) override final;
  void setMainWindow(MainWindow *mainWindow) override final;
  void resetData();
  void reloadData();
  void reloadColorData();
private:
  ElementsForm* _elementsForm;
  MainWindow *_mainWindow;
  std::shared_ptr<ProjectStructure> _projectStructure;
  std::vector<std::vector<std::shared_ptr<iRASPAObject>>> _structures{};
  int _selectedColorSetIndex = 0;
  int _selectedForceFieldSetIndex = 0;

  void setColorComboBoxIndex(size_t type);
  void setForceFieldComboBoxIndex(int type);

  std::optional<size_t> rowForItem(QObject* item);
  void addNewForceFieldAtomType();
  void removeForceFieldAtomType();
  void selectColorButton();
  void setEpsilonParameter(double parameter);
  void setSigmaParameter(double parameter);
  void setMass(double mass);
signals:
  void rendererReloadData();
  void invalidateCachedAmbientOcclusionTextures(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
  void invalidateCachedIsoSurfaces(std::vector<std::vector<std::shared_ptr<iRASPAObject>>> structures);
};
