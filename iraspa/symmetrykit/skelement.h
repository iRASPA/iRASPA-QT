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

#include <QString>
#include <array>
#include <vector>
#include <map>
#include <set>

struct SKElement
{
  QString _chemicalSymbol = QString("Undefined");
  qint64 _atomicNumber = 0;
  qint64 _group = 0;
  qint64 _period = 0;
  QString _name = QString("Undefined");
  double _mass = 1.0;
  double _atomRadius = 0.0;
  double _covalentRadius = 0.0;
  double _singleBondCovalentRadius = 0.0;
  double _doubleBondCovalentRadius = 0.0;
  double _tripleBondCovalentRadius = 0.0;
  double _VDWRadius = 1.0;
  std::vector<int> _possibleOxidationStates;
  qint64 _oxidationState = 0;
  double _atomicPolarizability = 0.0;
  SKElement();
  SKElement(QString string, qint64 atomicNumber, qint64 group, qint64 period, QString name, double mass, double atomRadius, double covalentRadius, double singleBondCovalentRadius,
            double doubleBondCovalentRadius, double tripleBondCovalentRadius, double vDWRadius, std::vector<int> possibleOxidationStates);
};


struct PredefinedElements
{
  static std::vector<SKElement> predefinedElements;
  static std::map<QString, int> atomicNumberData;

  static std::set<QString> residueDefinitions;
  static std::map<QString, QString> residueDefinitionsElement;
  static std::map<QString, QString> residueDefinitionsType;
  static std::map<QString, std::vector<QString>> residueDefinitionsBondedAtoms;
};
