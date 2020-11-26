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

/// https://gist.github.com/pavel-perina/1324ff064aedede0e01311aab315f83d

#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>


/// \brief Layout that keeps single item and maintains given aspect ratio
class AspectRatioSingleItemLayout : public QLayout {
public:

    /// \brief Constructor
    AspectRatioSingleItemLayout(QWidget *parent = nullptr, double aspectRatio = 16.0/9.0)
        : QLayout(parent)
        , m_aspectRatio(aspectRatio)
        , m_item(nullptr)
    {
    }

    /// \brief Destructor
    /// \warning Let's hope we're responsible for deleting item
    ~AspectRatioSingleItemLayout()
    {
        delete m_item;
    }

    /// \brief Set aspect ratio to keep
    /// \todo Invalidate geometry
    void setAspectRatio(double aspectRatio)
    {
        m_aspectRatio = aspectRatio;
    }

    /// \brief Return number of items
    int count() const override
    {
        return m_item != nullptr ? 1 : 0;
    }

    /// \brief Return item at given index, nullptr if index is out of range
    QLayoutItem *itemAt(int i) const override
    {
        return i == 0 ? m_item : nullptr;
    }

    /// \brief Take item at index. Now caller is responsible for deletion, we no longer own it
    QLayoutItem *takeAt(int) override
    {
        QLayoutItem *retval = m_item;
        m_item = nullptr;
        return retval;
    }

    /// \brief Returns directions where we want to expand beyond sizeHint()
    Qt::Orientations expandingDirections() const override
    {
        // we'd like grow beyond sizeHint()
        return  Qt::Horizontal | Qt::Vertical;
    }

    /// \brief We want to limit height based on width
    bool hasHeightForWidth() const override
    {
        return false;
    }

    /// \brief We want that much height for given \a width
    int heightForWidth(int width) const override
    {
        int height = (width - 2 * margin()) / m_aspectRatio + 2 * margin();
        return height;
    }

    /// \brief Set geometry of our only widget
    void setGeometry(const QRect &rect) override
    {
        QLayout::setGeometry(rect);
        if (m_item) {
            QWidget *wdg = m_item->widget();
            int availW = rect.width() - 2 * margin();
            int availH = rect.height() - 2 * margin();
            int w, h;
            h = availH;
            w = h * m_aspectRatio;
            if (w > availW) {
                // fill width
                w = availW;
                h = w / m_aspectRatio;
                int y;
                if (m_item->alignment() & Qt::AlignTop)
                    y = margin();
                else if (m_item->alignment() & Qt::AlignBottom)
                    y = rect.height() - margin() - h;
                else
                    y = margin() + (availH - h) / 2;
                wdg->setGeometry(rect.x() + margin(), rect.y() + y, w, h);
            }
            else {
                int x;
                if (m_item->alignment() & Qt::AlignLeft)
                    x = margin();
                else if (m_item->alignment() & Qt::AlignRight)
                    x = rect.width() - margin() - w;
                else
                    x = margin() + (availW - w) / 2;
                wdg->setGeometry(rect.x() + x, rect.y() + margin(), w, h);
            }
        }
    }

    QSize sizeHint() const override
    {
        const int margins = 2 * margin();
        return m_item ? m_item->sizeHint() + QSize(margins, margins) : QSize(margins, margins);
    }

    QSize minimumSize() const override
    {
        const int margins = 2 * margin();
        return m_item ? m_item->minimumSize() + QSize(margins, margins) : QSize(margins,margins);
    }

    /// \brief Add item, this removes existing from container
    void addItem(QLayoutItem *item) override
    {
        delete m_item;
        m_item = item;
        item->setAlignment(0);
    }

private:

    QLayoutItem *m_item; // at most one :-)

    double m_aspectRatio;

};
