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

#pragma  once

#include <QObject>
#include "renderkit.h"

class AnnotationViewer
{
public:
 virtual ~AnnotationViewer() = 0;

 virtual RKTextType renderTextType() const = 0;
 virtual QString renderTextFont() const = 0;
 virtual RKTextAlignment renderTextAlignment() const = 0;
 virtual RKTextStyle renderTextStyle() const = 0;
 virtual QColor renderTextColor() const = 0;
 virtual double renderTextScaling() const = 0;
 virtual double3 renderTextOffset() const = 0;
};

class AnnotationEditor: public AnnotationViewer
{
public:
 virtual ~AnnotationEditor() = 0;

 virtual void setRenderTextType(RKTextType type) = 0;
 virtual void setRenderTextFont(QString value) = 0;
 virtual void setRenderTextAlignment(RKTextAlignment alignment) = 0;
 virtual void setRenderTextStyle(RKTextStyle style) = 0;
 virtual void setRenderTextColor(QColor color) = 0;
 virtual void setRenderTextScaling(double scaling) = 0;
 virtual void setRenderTextOffsetX(double value) = 0;
 virtual void setRenderTextOffsetY(double value) = 0;
 virtual void setRenderTextOffsetZ(double value) = 0;
};

