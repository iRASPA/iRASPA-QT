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

#include "informationpanelview.h"

#include <algorithm>
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

InformationPanelView::InformationPanelView(QWidget *parent) : QWidget(parent)
{
  setFixedWidth(350);
  setFixedHeight(32);
  setAttribute(Qt::WA_TranslucentBackground, false);

  _content = new QWidget(this);
  _content->setAttribute(Qt::WA_TranslucentBackground);
  _opacityEffect = new QGraphicsOpacityEffect(_content);
  _opacityEffect->setOpacity(1.0);
  _content->setGraphicsEffect(_opacityEffect);

  QHBoxLayout *layout = new QHBoxLayout(_content);
  layout->setContentsMargins(6, 0, 6, 0);
  layout->setSpacing(4);
  layout->setAlignment(Qt::AlignVCenter);

  _iconLabel = new QLabel(_content);
  _iconLabel->setFixedSize(20, 20);
  _iconLabel->setScaledContents(true);
  _iconLabel->setAttribute(Qt::WA_TranslucentBackground);

  _textLabel = new QLabel(_content);
  QFont font = _textLabel->font();
  font.setPointSize(18);
  _textLabel->setFont(font);
  _textLabel->setStyleSheet("color: gray; background: transparent;");
  _textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _textLabel->setWordWrap(false);
  _textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  layout->addWidget(_iconLabel);
  layout->addWidget(_textLabel, 1);

  _hideTimer = new QTimer(this);
  _hideTimer->setSingleShot(true);
  QObject::connect(_hideTimer, &QTimer::timeout, this, &InformationPanelView::startFadeOut);

  _fade = new QPropertyAnimation(_opacityEffect, "opacity", this);
  _fade->setDuration(1000);
  _fade->setStartValue(1.0);
  _fade->setEndValue(0.0);
  QObject::connect(_fade, &QPropertyAnimation::finished, this, &InformationPanelView::hideContent);

  _content->hide();
  _content->setGeometry(rect());
}

void InformationPanelView::showInfoItem(const QIcon &icon, const QString &message)
{
  _hideTimer->stop();
  _fade->stop();
  _opacityEffect->setOpacity(1.0);

  _fullMessage = message;
  _iconLabel->setPixmap(icon.pixmap(20, 20));
  _iconLabel->setVisible(!icon.isNull());
  updateElidedText();
  _content->show();
  _content->raise();
  _hideTimer->start(5000);
}

void InformationPanelView::updateElidedText()
{
  const int textWidth = std::max(0, _content->width() - (_iconLabel->isVisible() ? 30 : 12));
  const QFontMetrics metrics(_textLabel->font());
  _textLabel->setText(metrics.elidedText(_fullMessage, Qt::ElideRight, textWidth));
}

void InformationPanelView::startFadeOut()
{
  _fade->start();
}

void InformationPanelView::hideContent()
{
  _content->hide();
  _fullMessage.clear();
}

void InformationPanelView::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);
  _content->setGeometry(rect());
  updateElidedText();
}

void InformationPanelView::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QLinearGradient grad1(0, 0, 0, height());
  grad1.setColorAt(0.0, QColor(237, 241, 225));
  grad1.setColorAt(0.5, QColor(230, 235, 213));
  grad1.setColorAt(0.51, QColor(222, 228, 199));
  grad1.setColorAt(1.0, QColor(242, 245, 224));

  const QRectF rectangle(0.5, 0.5, width() - 1.0, height() - 1.0);
  QPainterPath path;
  path.addRoundedRect(rectangle, 3.5, 3.5);
  painter.fillPath(path, grad1);
  painter.setPen(QPen(QColor(145, 145, 145), 1));
  painter.drawPath(path);
}
