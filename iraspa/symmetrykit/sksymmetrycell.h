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
#include "skpointsymmetryset.h"
#include "sktransformationmatrix.h"

class SKSymmetryCell
{
public:
    SKSymmetryCell();
    SKSymmetryCell(double a, double b, double c, double alpha, double beta, double gamma);
    static SKSymmetryCell createFromMetricTensor(double3x3 metricTensor);
    static SKSymmetryCell createFromUnitCell(double3x3 unitCell);
    double3x3 cell();
    double3x3 metricTensor();
    double volume();
    static double3x3 findSmallestPrimitiveCell(std::vector<std::tuple<double3, int, double> > reducedAtoms, std::vector<std::tuple<double3, int, double> > atoms, double3x3 unitCell, bool allowPartialOccupancies, double symmetryPrecision);
    static bool testTranslationalSymmetry(double3 translationVector, std::vector<std::tuple<double3, int, double>> atoms, double3x3 unitCell, bool allowPartialOccupancies, double precision);
    static bool testSymmetry(double3 translationVector, SKRotationMatrix rotationMatrix, std::vector<std::tuple<double3, int, double>> atoms, double3x3 unitCell, bool allowPartialOccupancies, double precision);
    static std::vector<double3> primitiveTranslationVectors(double3x3 unitCell, std::vector<std::tuple<double3, int, double>> reducedAtoms, std::vector<std::tuple<double3, int, double>> atoms, SKRotationMatrix rotationMatrix, bool allowPartialOccupancies, double symmetryPrecision);
    static std::optional<double3x3> computeDelaunayReducedCell(double3x3 unitCell, double symmetryPrecision);
    static std::optional<double3x3> computeDelaunayReducedCell2D(double3x3 unitCell, double symmetryPrecision);
    static SKPointSymmetrySet findLatticeSymmetry(double3x3 reducedLattice, double symmetryPrecision);
    static bool checkMetricSimilarity(double3x3 transformedMetricTensor, double3x3 metricTensor, double symmetryPrecision);
    static std::vector<std::tuple<double3, int, double>> trim(std::vector<std::tuple<double3, int, double>> atoms, double3x3 from, double3x3 to, bool allowPartialOccupancies, double symmetryPrecision);
    std::optional<std::pair<SKSymmetryCell, SKTransformationMatrix >> computeReducedNiggliCellAndChangeOfBasisMatrix();
private:
    double _a;
    double _b;
    double _c;
    double _alpha;
    double _beta;
    double _gamma;

    static bool isSmallerThen(double x, double y);
    static bool isLargerThen(double x, double y);
    static bool isSmallerEqualThen(double x, double y);
    static bool isLargerEqualThen(double x, double y);
    static bool isEqualTo(double x, double y);
    static bool isLargerThenZeroXiEtaZeta(double xi, double eta, double zeta);
};

