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
#include <skatomtreecontroller.h>

class AtomViewer
{
public:
  virtual ~AtomViewer() = 0;

  virtual std::shared_ptr<SKAtomTreeController> &atomsTreeController() = 0;
  virtual void expandSymmetry() = 0;
  virtual bool isFractional() = 0;

  virtual void clearSelection() = 0;
  virtual void setAtomSelection(int asymmetricAtomId) = 0;
  virtual void addAtomToSelection(int asymmetricAtomId) = 0;
  virtual void toggleAtomSelection(int asymmetricAtomId) = 0;
  virtual void setAtomSelection(std::set<int>& atomIds) = 0;
  virtual void addToAtomSelection(std::set<int>& atomIds) = 0;

  virtual std::set<int> filterCartesianAtomPositions(std::function<bool(double3)> &) = 0;

  virtual void recomputeSelectionBodyFixedBasis() = 0;

  virtual void convertAsymmetricAtomsToFractional() = 0;
  virtual void convertAsymmetricAtomsToCartesian() = 0;

  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const = 0;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> translatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, double3 translation) const = 0;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionCartesian(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const = 0;
  virtual std::vector<std::pair<std::shared_ptr<SKAsymmetricAtom>, double3>> rotatedPositionsSelectionBodyFrame(std::vector<std::shared_ptr<SKAsymmetricAtom>> atoms, simd_quatd rotation) const = 0;
};

class AtomEditor
{
public:
  virtual ~AtomEditor() = 0;

  virtual void setAtomsTreeController(std::shared_ptr<SKAtomTreeController> controller) = 0;
};
