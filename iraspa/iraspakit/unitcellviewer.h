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

#include <QString>
#include <QDate>
#include <skcell.h>
#include "object.h"
#include "volumetricdataviewer.h"

class UnitCellViewer
{
public:
  virtual ~UnitCellViewer() = 0;

  virtual bool drawUnitCell() const = 0;
  virtual void setDrawUnitCell(bool value) = 0;

  virtual double unitCellScaleFactor() const = 0;
  virtual QColor unitCellDiffuseColor() const = 0;
  virtual double unitCellDiffuseIntensity() const = 0;
};

class UnitCellEditor: public UnitCellViewer
{
public:
  virtual ~UnitCellEditor() = 0;

  virtual void setUnitCellScaleFactor(double value) = 0;
  virtual void setUnitCellDiffuseColor(QColor value) = 0;
  virtual void setUnitCellDiffuseIntensity(double value) = 0;
};


