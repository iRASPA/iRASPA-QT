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

#include <QStyledItemDelegate>
#include <QWidget>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QComboBox>
#include <QIcon>



class BondListViewComboBoxStyledItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
    BondListViewComboBoxStyledItemDelegate(QWidget *parent);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override final;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override final;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
private slots:
  void cellEntered(const QModelIndex &index);
private:
  QIcon *singleBondIcon;
  QIcon *doubleBondIcon;
  QIcon *partialDoubleBondIcon;
  QIcon *tripleBondIcon;
  QComboBox *comboBox;
  QTreeView *treeView;
  bool isOneCellInEditMode = false;
  QPersistentModelIndex currentEditedCellIndex;
};
