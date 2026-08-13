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

#include "skaminoacididealgeometry.h"
#include <algorithm>

namespace {

const std::vector<QString> kReplaceableResidueCodes = {
    QStringLiteral("ALA"), QStringLiteral("ARG"), QStringLiteral("ASN"), QStringLiteral("ASP"), QStringLiteral("CYS"),
    QStringLiteral("GLN"), QStringLiteral("GLU"), QStringLiteral("GLY"), QStringLiteral("HIS"), QStringLiteral("ILE"),
    QStringLiteral("LEU"), QStringLiteral("LYS"), QStringLiteral("MET"), QStringLiteral("PHE"), QStringLiteral("PRO"),
    QStringLiteral("SER"), QStringLiteral("THR"), QStringLiteral("TRP"), QStringLiteral("TYR"), QStringLiteral("VAL"),
};

std::map<QString, std::map<QString, double3>> buildCoordinatesByResidue()
{
  return {
      {QStringLiteral("ALA"), {{QStringLiteral("N"), double3(-0.525, 1.363, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.526, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.529, -0.774, -1.205)}, {QStringLiteral("O"), double3(0.627, 1.062, 0.000)}}},
      {QStringLiteral("ARG"), {{QStringLiteral("N"), double3(-0.524, 1.362, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.525, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.524, -0.778, -1.209)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.616, 1.390, 0.000)}, {QStringLiteral("CD"), double3(0.564, 1.414, 0.000)}, {QStringLiteral("NE"), double3(0.539, 1.357, 0.000)}, {QStringLiteral("NH1"), double3(0.206, 2.301, 0.000)}, {QStringLiteral("NH2"), double3(2.078, 0.978, 0.000)}, {QStringLiteral("CZ"), double3(0.758, 1.093, 0.000)}}},
      {QStringLiteral("ASN"), {{QStringLiteral("N"), double3(-0.536, 1.357, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.526, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.531, -0.787, -1.200)}, {QStringLiteral("O"), double3(0.625, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.584, 1.399, 0.000)}, {QStringLiteral("ND2"), double3(0.593, -1.188, 0.001)}, {QStringLiteral("OD1"), double3(0.633, 1.059, 0.000)}}},
      {QStringLiteral("ASP"), {{QStringLiteral("N"), double3(-0.525, 1.362, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.527, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.526, -0.778, -1.208)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.593, 1.398, 0.000)}, {QStringLiteral("OD1"), double3(0.610, 1.091, 0.000)}, {QStringLiteral("OD2"), double3(0.592, -1.101, -0.003)}}},
      {QStringLiteral("CYS"), {{QStringLiteral("N"), double3(-0.522, 1.362, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.524, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.519, -0.773, -1.212)}, {QStringLiteral("O"), double3(0.625, 1.062, 0.000)}, {QStringLiteral("SG"), double3(0.728, 1.653, 0.000)}}},
      {QStringLiteral("GLN"), {{QStringLiteral("N"), double3(-0.526, 1.361, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.526, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.525, -0.779, -1.207)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.615, 1.393, 0.000)}, {QStringLiteral("CD"), double3(0.587, 1.399, 0.000)}, {QStringLiteral("NE2"), double3(0.593, -1.189, -0.001)}, {QStringLiteral("OE1"), double3(0.634, 1.060, 0.000)}}},
      {QStringLiteral("GLU"), {{QStringLiteral("N"), double3(-0.528, 1.361, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.526, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.526, -0.781, -1.207)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.615, 1.392, 0.000)}, {QStringLiteral("CD"), double3(0.600, 1.397, 0.000)}, {QStringLiteral("OE1"), double3(0.607, 1.095, 0.000)}, {QStringLiteral("OE2"), double3(0.589, -1.104, -0.001)}}},
      {QStringLiteral("GLY"), {{QStringLiteral("N"), double3(-0.572, 1.337, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.517, 0.000, 0.000)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}}},
      {QStringLiteral("HIS"), {{QStringLiteral("N"), double3(-0.527, 1.360, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.525, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.525, -0.778, -1.208)}, {QStringLiteral("O"), double3(0.625, 1.063, 0.000)}, {QStringLiteral("CG"), double3(0.600, 1.370, 0.000)}, {QStringLiteral("CD2"), double3(0.889, -1.021, 0.003)}, {QStringLiteral("ND1"), double3(0.744, 1.160, 0.000)}, {QStringLiteral("CE1"), double3(2.030, 0.851, 0.002)}, {QStringLiteral("NE2"), double3(2.145, -0.466, 0.004)}}},
      {QStringLiteral("ILE"), {{QStringLiteral("N"), double3(-0.493, 1.373, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.527, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.536, -0.793, -1.213)}, {QStringLiteral("O"), double3(0.627, 1.062, 0.000)}, {QStringLiteral("CG1"), double3(0.534, 1.437, 0.000)}, {QStringLiteral("CG2"), double3(0.540, -0.785, -1.199)}, {QStringLiteral("CD1"), double3(0.619, 1.391, 0.000)}}},
      {QStringLiteral("LEU"), {{QStringLiteral("N"), double3(-0.520, 1.363, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.525, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.522, -0.773, -1.214)}, {QStringLiteral("O"), double3(0.625, 1.063, 0.000)}, {QStringLiteral("CG"), double3(0.678, 1.371, 0.000)}, {QStringLiteral("CD1"), double3(0.530, 1.430, 0.000)}, {QStringLiteral("CD2"), double3(0.535, -0.774, 1.200)}}},
      {QStringLiteral("LYS"), {{QStringLiteral("N"), double3(-0.526, 1.362, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.526, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.524, -0.778, -1.208)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.619, 1.390, 0.000)}, {QStringLiteral("CD"), double3(0.559, 1.417, 0.000)}, {QStringLiteral("CE"), double3(0.560, 1.416, 0.000)}, {QStringLiteral("NZ"), double3(0.554, 1.387, 0.000)}}},
      {QStringLiteral("MET"), {{QStringLiteral("N"), double3(-0.521, 1.364, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.525, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.523, -0.776, -1.210)}, {QStringLiteral("O"), double3(0.625, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.613, 1.391, 0.000)}, {QStringLiteral("SD"), double3(0.703, 1.695, 0.000)}, {QStringLiteral("CE"), double3(0.320, 1.786, 0.000)}}},
      {QStringLiteral("PHE"), {{QStringLiteral("N"), double3(-0.518, 1.363, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.524, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.525, -0.776, -1.212)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.607, 1.377, 0.000)}, {QStringLiteral("CD1"), double3(0.709, 1.195, 0.000)}, {QStringLiteral("CD2"), double3(0.706, -1.196, 0.000)}, {QStringLiteral("CE1"), double3(2.102, 1.198, 0.000)}, {QStringLiteral("CE2"), double3(2.098, -1.201, 0.000)}, {QStringLiteral("CZ"), double3(2.794, -0.003, -0.001)}}},
      {QStringLiteral("PRO"), {{QStringLiteral("N"), double3(-0.566, 1.351, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.527, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.546, -0.611, -1.293)}, {QStringLiteral("O"), double3(0.621, 1.066, 0.000)}, {QStringLiteral("CG"), double3(0.382, 1.445, 0.000)}, {QStringLiteral("CD"), double3(0.477, 1.424, 0.000)}}},
      {QStringLiteral("SER"), {{QStringLiteral("N"), double3(-0.529, 1.360, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.525, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.518, -0.777, -1.211)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("OG"), double3(0.503, 1.325, 0.000)}}},
      {QStringLiteral("THR"), {{QStringLiteral("N"), double3(-0.517, 1.364, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.526, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.516, -0.793, -1.215)}, {QStringLiteral("O"), double3(0.626, 1.062, 0.000)}, {QStringLiteral("CG2"), double3(0.550, -0.718, -1.228)}, {QStringLiteral("OG1"), double3(0.472, 1.353, 0.000)}}},
      {QStringLiteral("TRP"), {{QStringLiteral("N"), double3(-0.521, 1.363, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.525, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.523, -0.776, -1.212)}, {QStringLiteral("O"), double3(0.627, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.609, 1.370, 0.000)}, {QStringLiteral("CD1"), double3(0.824, 1.091, 0.000)}, {QStringLiteral("CD2"), double3(0.854, -1.148, -0.005)}, {QStringLiteral("CE2"), double3(2.186, -0.678, -0.007)}, {QStringLiteral("CE3"), double3(0.622, -2.530, -0.007)}, {QStringLiteral("NE1"), double3(2.140, 0.690, -0.004)}, {QStringLiteral("CH2"), double3(3.028, -2.890, -0.013)}, {QStringLiteral("CZ2"), double3(3.283, -1.543, -0.011)}, {QStringLiteral("CZ3"), double3(1.715, -3.389, -0.011)}}},
      {QStringLiteral("TYR"), {{QStringLiteral("N"), double3(-0.522, 1.362, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.524, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.522, -0.776, -1.213)}, {QStringLiteral("O"), double3(0.627, 1.062, 0.000)}, {QStringLiteral("CG"), double3(0.607, 1.382, 0.000)}, {QStringLiteral("CD1"), double3(0.716, 1.195, 0.000)}, {QStringLiteral("CD2"), double3(0.713, -1.194, -0.001)}, {QStringLiteral("CE1"), double3(2.107, 1.200, -0.002)}, {QStringLiteral("CE2"), double3(2.104, -1.201, -0.003)}, {QStringLiteral("OH"), double3(4.168, -0.002, -0.005)}, {QStringLiteral("CZ"), double3(2.791, -0.001, -0.003)}}},
      {QStringLiteral("VAL"), {{QStringLiteral("N"), double3(-0.494, 1.373, 0.000)}, {QStringLiteral("CA"), double3(0.0, 0.0, 0.0)}, {QStringLiteral("C"), double3(1.527, 0.000, 0.000)}, {QStringLiteral("CB"), double3(-0.533, -0.795, -1.213)}, {QStringLiteral("O"), double3(0.627, 1.062, 0.000)}, {QStringLiteral("CG1"), double3(0.540, 1.429, 0.000)}, {QStringLiteral("CG2"), double3(0.533, -0.776, 1.203)}}},
  };
}

double3x3 localFrame(const double3 &alphaCarbon, const double3 &nitrogen, const double3 &carbonyl)
{
  double3 xAxis = nitrogen - alphaCarbon;
  const double xLength = xAxis.length();
  if (xLength > 1.0e-8)
  {
    xAxis = xAxis / xLength;
  }
  else
  {
    xAxis = double3(1.0, 0.0, 0.0);
  }

  double3 zAxis = double3::cross(xAxis, carbonyl - alphaCarbon);
  const double zLength = zAxis.length();
  if (zLength > 1.0e-8)
  {
    zAxis = zAxis / zLength;
  }
  else
  {
    zAxis = double3(0.0, 0.0, 1.0);
  }

  const double3 yAxis = double3::cross(zAxis, xAxis);
  return double3x3(xAxis, yAxis, zAxis);
}

std::pair<double3x3, double3> backboneAlignmentTransform(const double3 &idealN,
                                                         const double3 &idealCA,
                                                         const double3 &idealC,
                                                         const double3 &actualN,
                                                         const double3 &actualCA,
                                                         const double3 &actualC)
{
  const double3x3 idealFrame = localFrame(idealCA, idealN, idealC);
  const double3x3 actualFrame = localFrame(actualCA, actualN, actualC);
  const double3x3 rotation = actualFrame * double3x3::transpose(idealFrame);
  const double3 translation = actualCA - rotation * idealCA;
  return {rotation, translation};
}

} // namespace

const std::vector<QString> &SKAminoAcidIdealGeometry::replaceableResidueCodes()
{
  return kReplaceableResidueCodes;
}

std::optional<std::map<QString, double3>> SKAminoAcidIdealGeometry::idealCoordinates(const QString &residueCode)
{
  static const std::map<QString, std::map<QString, double3>> coordinatesByResidue = buildCoordinatesByResidue();
  const QString key = residueCode.trimmed().toUpper();
  const auto iterator = coordinatesByResidue.find(key);
  if (iterator == coordinatesByResidue.end())
  {
    return std::nullopt;
  }
  return iterator->second;
}

std::vector<QString> SKAminoAcidIdealGeometry::atomNames(const QString &residueCode)
{
  const std::optional<std::map<QString, double3>> coordinates = idealCoordinates(residueCode);
  if (!coordinates.has_value())
  {
    return {};
  }
  std::vector<QString> names;
  names.reserve(coordinates->size());
  for (const auto &entry : coordinates.value())
  {
    names.push_back(entry.first);
  }
  std::sort(names.begin(), names.end());
  return names;
}

std::optional<std::map<QString, double3>> SKAminoAcidIdealGeometry::alignedCoordinates(const QString &residueCode,
                                                                                        const double3 &actualN,
                                                                                        const double3 &actualCA,
                                                                                        const double3 &actualC)
{
  const std::optional<std::map<QString, double3>> ideal = idealCoordinates(residueCode);
  if (!ideal.has_value())
  {
    return std::nullopt;
  }
  const auto nitrogenIterator = ideal->find(QStringLiteral("N"));
  const auto alphaCarbonIterator = ideal->find(QStringLiteral("CA"));
  const auto carbonylIterator = ideal->find(QStringLiteral("C"));
  if (nitrogenIterator == ideal->end() || alphaCarbonIterator == ideal->end() || carbonylIterator == ideal->end())
  {
    return std::nullopt;
  }

  const auto transform = backboneAlignmentTransform(nitrogenIterator->second, alphaCarbonIterator->second, carbonylIterator->second,
                                                    actualN, actualCA, actualC);
  const double3x3 &rotation = transform.first;
  const double3 &translation = transform.second;

  std::map<QString, double3> aligned;
  for (const auto &entry : ideal.value())
  {
    aligned[entry.first] = rotation * entry.second + translation;
  }
  return aligned;
}
