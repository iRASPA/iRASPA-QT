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

#include <cmath>
#include <QString>
#include <QUrl>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <cwctype>
#include <optional>
#include <foundationkit.h>
#include "skparser.h"
#include "skasymmetricatom.h"
#include "skatomtreenode.h"
#include "skatomtreecontroller.h"
#include "skspacegroup.h"
#include "skstructure.h"

class SKLOCPOTParser: public SKParser
{
public:
  SKLOCPOTParser(QUrl url, bool onlyAsymmetricUnitCell, bool asMolecule, CharacterSet charactersToBeSkipped);
  void startParsing() noexcept(false) override final;
private:
  Scanner _scanner;
  [[maybe_unused]] bool _proteinOnlyAsymmetricUnitCell;
  [[maybe_unused]] bool _asMolecule;
  [[maybe_unused]] QString::const_iterator _previousScanLocation;

  [[maybe_unused]] int _numberOfAtoms = 0;
  [[maybe_unused]] int _numberOfAminoAcidAtoms = 0;
  std::shared_ptr<SKStructure> _frame;
  std::optional<SKCell> _cell;
  [[maybe_unused]] int _spaceGroupHallNumber;
};
