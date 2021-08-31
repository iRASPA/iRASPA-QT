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

#include <mathkit.h>
#include <optional>

class SKSymmetryCell
{
public:
    SKSymmetryCell();
    double3x3 cell();
    double3x3 metricTensor();
    double volume();
    static double3x3 findSmallestPrimitiveCell(std::vector<std::tuple<double3, int, double> > reducedAtoms, std::vector<std::tuple<double3, int, double> > atoms, double3x3 unitCell, bool allowPartialOccupancies, double symmetryPrecision);
    static bool testTranslationalSymmetry(double3 translationVector, std::vector<std::tuple<double3, int, double>> atoms, double3x3 unitCell, bool allowPartialOccupancies, double precision);
    static std::optional<double3x3> computeDelaunayReducedCell(double3x3 unitCell, double symmetryPrecision);
private:
    double _a;
    double _b;
    double _c;
    double _alpha;
    double _beta;
    double _gamma;
};

