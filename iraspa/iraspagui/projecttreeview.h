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

#include <QObject>
#include <QMainWindow>
#include <QTreeView>
#include <iraspatreeview.h>
#include <QModelIndex>
#include "projecttreeviewmodel.h"
#include <optional>
#include "logreporting.h"
#include <iraspakit.h>
#include "iraspamainwindowconsumerprotocol.h"

class ProjectTreeView: public iRASPATreeView, public MainWindowConsumer, public Reloadable, public LogReportingConsumer, public TabControlled
{
  Q_OBJECT
public:
  ProjectTreeView(QWidget* parent = nullptr);
  void setDocumentData(std::shared_ptr<DocumentData> documentData);
  void insertGalleryData(std::shared_ptr<DocumentData> database);
  void insertDatabaseCoReMOFData(std::shared_ptr<DocumentData> database);
  void insertDatabaseCoReMOFDDECData(std::shared_ptr<DocumentData> database);
  void insertDatabaseIZAFData(std::shared_ptr<DocumentData> database);
  void TabItemWasSelected() override final;
  QSize sizeHint() const override;

  void setLogReportingWidget(LogReporting *logReporting) override final;

  bool hasSelection() const override final;
  void paintEvent(QPaintEvent *event) override final;
  void setMainWindow(MainWindow* mainWindow) final override;
  void reloadSelection() override final;
  void reloadData() override final;

	bool insertRows(int position, int rows, const QModelIndex &parent, std::shared_ptr<ProjectTreeNode> item);

  void addStructureProject(QModelIndex index);
  void addGroupProject(QModelIndex index);

  void startDrag(Qt::DropActions supportedActions) override final;
  void dragMoveEvent(QDragMoveEvent *event) override final;
  QUndoStack& undoManager() {return _undoStack;}
  void focusInEvent( QFocusEvent* ) override final;
  void focusOutEvent( QFocusEvent* ) override final;
  std::shared_ptr<ProjectTreeController> projectTreeController() {return _projectTreeController;}
private:
  QRect _dropIndicatorRect;
  MainWindow* _mainWindow;
  LogReporting *_logReporting;
  std::shared_ptr<ProjectTreeController> _projectTreeController;
  std::shared_ptr<ProjectTreeViewModel> _model;
  QUndoStack _undoStack;
  QPixmap selectionToPixmap();
public slots:
  void deleteSelection();
  void copy();
  void paste();
  void cut();
  void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override final;
  void ShowContextMenu(const QPoint &pos);
  void keyPressEvent(QKeyEvent * event) override final;
  void addProjectStructure();
  void ShowToolButtonContextMenu(const QPoint &pos);
signals:
  void setSelectedRenderFrames(std::vector<std::vector<std::shared_ptr<iRASPAStructure>>> structures);
};

