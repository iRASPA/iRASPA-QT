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

#include "atomtreeviewribbonvisibilitystyleditemdelegate.h"
#include "atomtreeviewmodel.h"
#include "qtsegmentcontrol.h"

#include <QApplication>
#include <QIcon>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QToolTip>
#include <QWidget>

namespace
{
  constexpr int atomsSegment = 0;
  constexpr int ribbonSegment = 1;
  constexpr int segmentControlMargin = 2;
  constexpr int minimumSegmentControlWidth = 48;

  QStyle *styleForOption(const QStyleOptionViewItem &option)
  {
    return option.widget ? option.widget->style() : QApplication::style();
  }
}

AtomTreeViewRibbonVisibilityStyledItemDelegate::AtomTreeViewRibbonVisibilityStyledItemDelegate(QWidget* parent):
  QStyledItemDelegate(parent),
  _segmentControl(new QtSegmentControl(parent))
{
  _segmentControl->hide();
  _segmentControl->setAttribute(Qt::WA_DontShowOnScreen);
  _segmentControl->setCount(2);
  _segmentControl->setSelectionBehavior(QtSegmentControl::SelectAll);
  _segmentControl->setSegmentText(atomsSegment, QStringLiteral("A"));
  _segmentControl->setSegmentText(ribbonSegment, QStringLiteral("R"));
  _segmentControl->setSegmentToolTip(atomsSegment, QStringLiteral("Show the atoms of this group"));
  _segmentControl->setSegmentToolTip(ribbonSegment, QStringLiteral("Show the ribbon of this group"));

  QFont font = _segmentControl->font();
  font.setPointSize(qMax(9, font.pointSize() - 2));
  _segmentControl->setFont(font);
}

bool AtomTreeViewRibbonVisibilityStyledItemDelegate::showsDualVisibility(const QModelIndex &index)
{
  return index.column() == 0 && index.data(AtomTreeViewModel::IsRibbonHierarchyGroupRole).toBool();
}

int AtomTreeViewRibbonVisibilityStyledItemDelegate::visibilityRole(int segment)
{
  return segment == atomsSegment ? AtomTreeViewModel::GroupAtomsVisibilityRole
                                 : AtomTreeViewModel::GroupRibbonVisibilityRole;
}

QRect AtomTreeViewRibbonVisibilityStyledItemDelegate::segmentControlRect(const QStyleOptionViewItem &option) const
{
  const QSize hint = _segmentControl->sizeHint();
  const int height = qBound(18, option.rect.height() - 2, qMax(hint.height(), option.rect.height() - 4));
  const int width = qMax(hint.width(), minimumSegmentControlWidth);
  const int top = option.rect.top() + (option.rect.height() - height) / 2;
  return QRect(option.rect.left() + segmentControlMargin, top, width, height);
}

void AtomTreeViewRibbonVisibilityStyledItemDelegate::syncSegmentControl(const QModelIndex &index,
                                                                        const QStyleOptionViewItem &option) const
{
  const QRect rect = segmentControlRect(option);
  _segmentControl->resize(rect.size());
  // Cocoa only lights a segment when that side is fully visible (partial → unselected).
  _segmentControl->setSegmentSelected(atomsSegment,
                                      static_cast<Qt::CheckState>(index.data(visibilityRole(atomsSegment)).toInt()) == Qt::Checked);
  _segmentControl->setSegmentSelected(ribbonSegment,
                                      static_cast<Qt::CheckState>(index.data(visibilityRole(ribbonSegment)).toInt()) == Qt::Checked);
  _segmentControl->setEnabled(option.state & QStyle::State_Enabled);
}

void AtomTreeViewRibbonVisibilityStyledItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                                           const QModelIndex &index) const
{
  if(!showsDualVisibility(index))
  {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }

  QStyleOptionViewItem itemOption = option;
  initStyleOption(&itemOption, index);
  itemOption.features &= ~QStyleOptionViewItem::HasCheckIndicator;
  QStyle *style = styleForOption(itemOption);

  QStyleOptionViewItem backgroundOption = itemOption;
  backgroundOption.text.clear();
  backgroundOption.icon = QIcon();
  backgroundOption.features &= ~QStyleOptionViewItem::HasDecoration;
  style->drawControl(QStyle::CE_ItemViewItem, &backgroundOption, painter, itemOption.widget);

  const QRect controlRect = segmentControlRect(itemOption);
  syncSegmentControl(index, itemOption);

  const qreal devicePixelRatio = painter->device()->devicePixelRatioF();
  QPixmap pixmap(controlRect.size() * devicePixelRatio);
  pixmap.setDevicePixelRatio(devicePixelRatio);
  pixmap.fill(Qt::transparent);
  _segmentControl->render(&pixmap, QPoint(), QRegion(), QWidget::DrawChildren);
  painter->drawPixmap(controlRect.topLeft(), pixmap);

  QStyleOptionViewItem textOption = itemOption;
  textOption.rect.adjust(controlRect.width() + 2 * segmentControlMargin, 0, 0, 0);
  style->drawControl(QStyle::CE_ItemViewItem, &textOption, painter, itemOption.widget);
}

QSize AtomTreeViewRibbonVisibilityStyledItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  if(showsDualVisibility(index))
  {
    QStyleOptionViewItem itemOption = option;
    initStyleOption(&itemOption, index);
    size.setWidth(size.width() + segmentControlRect(itemOption).width() + 2 * segmentControlMargin);
  }
  return size;
}

bool AtomTreeViewRibbonVisibilityStyledItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                                                 const QStyleOptionViewItem &option, const QModelIndex &index)
{
  if(showsDualVisibility(index) &&
     (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease))
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    if(mouseEvent->button() == Qt::LeftButton)
    {
      #if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const QPoint position = mouseEvent->pos();
      #else
        const QPoint position = mouseEvent->position().toPoint();
      #endif

      const QRect controlRect = segmentControlRect(option);
      if(!controlRect.contains(position))
      {
        return QStyledItemDelegate::editorEvent(event, model, option, index);
      }

      syncSegmentControl(index, option);
      const int segment = _segmentControl->segmentAt(position - controlRect.topLeft());
      if(segment < 0) return true;
      if(event->type() == QEvent::MouseButtonPress) return true;

      const int role = visibilityRole(segment);
      const bool isVisible = static_cast<Qt::CheckState>(index.data(role).toInt()) != Qt::Checked;
      return model->setData(index, isVisible, role);
    }
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

bool AtomTreeViewRibbonVisibilityStyledItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view,
                                                               const QStyleOptionViewItem &option, const QModelIndex &index)
{
  if(showsDualVisibility(index) && event->type() == QEvent::ToolTip)
  {
    const QRect controlRect = segmentControlRect(option);
    if(controlRect.contains(event->pos()))
    {
      syncSegmentControl(index, option);
      const int segment = _segmentControl->segmentAt(event->pos() - controlRect.topLeft());
      if(segment >= 0)
      {
        QToolTip::showText(event->globalPos(), _segmentControl->segmentToolTip(segment), view);
        return true;
      }
    }
  }

  return QStyledItemDelegate::helpEvent(event, view, option, index);
}
