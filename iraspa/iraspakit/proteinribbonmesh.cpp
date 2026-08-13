/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    Ported from iRASPAKit ProteinRibbonMesh.swift (MIT License, 2014-2022).
 ********************************************************************************************************************/

#include "proteinribbonmesh.h"
#include "proteinnucleicacidmesh.h"
#include "dnanucleotidegeometry.h"
#include "dnabackbone.h"
#include "sknucleotidebase.h"
#include "mathkit.h"
#include "proteinribbonsegmentsupport.h"
#include "proteinbspline.h"
#include "proteincatmullromspline.h"
#include "proteinribbonsecondarystructure.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>

namespace
{
  inline double length_squared(const double3 &vector) { return vector.length_squared(); }
  inline double3 normalize(const double3 &vector) { return double3::normalize(vector); }
  inline double3 cross(const double3 &a, const double3 &b) { return double3::cross(a, b); }
  inline double dot(const double3 &a, const double3 &b) { return double3::dot(a, b); }
  inline double length(const double3 &vector) { return vector.length(); }

  struct RibbonPoint3
  {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    double3 asDouble3() const { return double3(x, y, z); }

    static RibbonPoint3 from(const double3 &vector) { return {vector.x, vector.y, vector.z}; }

    static RibbonPoint3 fromSum(const double3 &vector, double3 shift)
    {
      return {vector.x + shift.x, vector.y + shift.y, vector.z + shift.z};
    }

    RibbonPoint3 &operator+=(const double3 &vector)
    {
      x += vector.x;
      y += vector.y;
      z += vector.z;
      return *this;
    }

    RibbonPoint3 &operator+=(const RibbonPoint3 &vector)
    {
      x += vector.x;
      y += vector.y;
      z += vector.z;
      return *this;
    }

    RibbonPoint3 &operator/=(double divisor)
    {
      x /= divisor;
      y /= divisor;
      z /= divisor;
      return *this;
    }
  };

  RibbonPoint3 add(const RibbonPoint3 &point, const double3 &vector)
  {
    return {point.x + vector.x, point.y + vector.y, point.z + vector.z};
  }

  double lengthBetween(const RibbonPoint3 &from, const RibbonPoint3 &to)
  {
    const double dx = to.x - from.x;
    const double dy = to.y - from.y;
    const double dz = to.z - from.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
  }

  struct RibbonStation
  {
    RibbonPoint3 center;
    RibbonPoint3 tangent;
    RibbonPoint3 faceNormal;
    RibbonPoint3 widthDirection;
    ProteinRibbonSecondaryStructure secondaryStructure;
    double sheetArrowFactor;
    double sheetArrowWidthFactor;
    double residuePosition;
  };

  struct CrossSectionProfile
  {
    double widthClamp;
    double radiusScale;
    double normalScale;
    /// PyMOL ExtrudeRectangle for sheets/arrows; oval clamp for helices.
    bool rectangular = false;
  };

  struct NucleicAcidCrossSectionOffset
  {
    double alongWidthDirection;
    double alongFaceNormal;
  };

  void rectangleCrossSectionPoint(double perimeterFraction,
                                  double halfLength,
                                  double halfWidth,
                                  double &alongWidthDirection,
                                  double &alongFaceNormal)
  {
    const double edgeLength = 2.0 * halfLength;
    const double edgeWidth = 2.0 * halfWidth;
    const double perimeter = 2.0 * (edgeLength + edgeWidth);
    double distance = perimeterFraction * perimeter;
    if (distance < 0.0) distance += perimeter * std::ceil(-distance / perimeter);
    distance = std::fmod(distance, perimeter);

    if (distance <= edgeWidth)
    {
      alongFaceNormal = -halfWidth + distance;
      alongWidthDirection = -halfLength;
      return;
    }
    distance -= edgeWidth;
    if (distance <= edgeLength)
    {
      alongFaceNormal = halfWidth;
      alongWidthDirection = -halfLength + distance;
      return;
    }
    distance -= edgeLength;
    if (distance <= edgeWidth)
    {
      alongFaceNormal = halfWidth - distance;
      alongWidthDirection = halfLength;
      return;
    }
    distance -= edgeWidth;
    alongFaceNormal = -halfWidth;
    alongWidthDirection = halfLength - distance;
  }

  void dumbbellCrossSectionPoint(double perimeterFraction,
                                 double halfLength,
                                 double halfWidth,
                                 double bulbRadius,
                                 double &alongWidthDirection,
                                 double &alongFaceNormal)
  {
    const double centerOffset = std::max(halfLength - bulbRadius, bulbRadius * 0.25);
    const double arcLength = M_PI * bulbRadius;
    const double connectorLength = 2.0 * centerOffset;
    const double bridgeWidth = halfWidth;
    const double perimeter = 2.0 * arcLength + 2.0 * connectorLength + 2.0 * bridgeWidth;
    double distance = perimeterFraction * perimeter;
    if (distance < 0.0) distance += perimeter * std::ceil(-distance / perimeter);
    distance = std::fmod(distance, perimeter);

    const auto bottomCenterY = -centerOffset;
    const auto topCenterY = centerOffset;

    if (distance <= arcLength)
    {
      const double theta = M_PI + distance / bulbRadius;
      alongWidthDirection = bottomCenterY + bulbRadius * std::sin(theta);
      alongFaceNormal = bulbRadius * std::cos(theta);
      return;
    }
    distance -= arcLength;

    if (distance <= connectorLength)
    {
      alongWidthDirection = bottomCenterY + distance;
      alongFaceNormal = bridgeWidth;
      return;
    }
    distance -= connectorLength;

    if (distance <= arcLength)
    {
      const double theta = distance / bulbRadius;
      alongWidthDirection = topCenterY + bulbRadius * std::sin(theta);
      alongFaceNormal = bulbRadius * std::cos(theta);
      return;
    }
    distance -= arcLength;

    alongWidthDirection = topCenterY - distance;
    alongFaceNormal = -bridgeWidth;
  }

  NucleicAcidCrossSectionOffset nucleicAcidCrossSectionOffset(double angle,
                                                              double radius,
                                                              const ProteinRibbonMeshParameters &parameters)
  {
    const double perimeterFraction = angle / (2.0 * M_PI);
    double alongWidthDirection = 0.0;
    double alongFaceNormal = 0.0;

    switch (parameters.nucleicAcidBackboneStyle)
    {
    case NucleicAcidBackboneStyle::oval:
    {
      const double d = std::sin(angle);
      double n = std::cos(angle);
      n = std::max(-parameters.nucleicAcidOvalWidth, std::min(parameters.nucleicAcidOvalWidth, n));
      alongWidthDirection = parameters.nucleicAcidOvalLength * radius * d;
      alongFaceNormal = radius * n;
      break;
    }
    case NucleicAcidBackboneStyle::tube:
      alongWidthDirection = radius * std::sin(angle);
      alongFaceNormal = radius * std::cos(angle);
      break;
    case NucleicAcidBackboneStyle::rect:
      rectangleCrossSectionPoint(perimeterFraction,
                                 parameters.nucleicAcidOvalLength * radius * 0.5,
                                 parameters.nucleicAcidOvalWidth * radius * 0.5,
                                 alongWidthDirection,
                                 alongFaceNormal);
      break;
    case NucleicAcidBackboneStyle::dumbbell:
      dumbbellCrossSectionPoint(perimeterFraction,
                                parameters.nucleicAcidDumbbellLength * radius * 0.5,
                                parameters.nucleicAcidDumbbellWidth * radius * 0.5,
                                parameters.nucleicAcidDumbbellRadius * radius,
                                alongWidthDirection,
                                alongFaceNormal);
      break;
    }
    return {alongWidthDirection, alongFaceNormal};
  }

  double3 safeNormalize(const double3 &vector, const double3 &fallback)
  {
    if (length_squared(vector) < 1.0e-12) return fallback;
    return normalize(vector);
  }

  double3 perpendicularVector(const double3 &tangent)
  {
    if (std::abs(tangent.x) > std::abs(tangent.z))
    {
      return normalize(cross(tangent, double3(0.0, 0.0, 1.0)));
    }
    return normalize(cross(tangent, double3(0.0, 1.0, 0.0)));
  }

  double3 projectToPlane(const double3 &vector, const double3 &planeNormal)
  {
    return vector - planeNormal * dot(vector, planeNormal);
  }

  double3 parallelTransport(const double3 &vector, const double3 &from, const double3 &to)
  {
    const double3 rotationAxis = cross(from, to);
    const double axisLengthSquared = length_squared(rotationAxis);
    if (axisLengthSquared < 1.0e-16) return vector;
    const double3 axis = rotationAxis / std::sqrt(axisLengthSquared);
    const double cosAngle = std::max(-1.0, std::min(1.0, dot(from, to)));
    const double sinAngle = std::sin(std::acos(cosAngle));
    return vector * cosAngle + cross(axis, vector) * sinAngle + axis * dot(axis, vector) * (1.0 - cosAngle);
  }

  float structureTypeCode(ProteinRibbonSecondaryStructure structure)
  {
    switch (structure)
    {
    case ProteinRibbonSecondaryStructure::coil: return 0.0f;
    case ProteinRibbonSecondaryStructure::helix: return 1.0f;
    case ProteinRibbonSecondaryStructure::sheet: return 2.0f;
    }
    return 0.0f;
  }

  float insetLightmapUV(float coordinate)
  {
    return 0.999f * coordinate + 0.0005f;
  }

  float2 parametricLightmapUV(int ringIndex, int splineSamples, float arcLengthFraction, int chainIndex, int totalChains)
  {
    const float u = splineSamples > 1 ? static_cast<float>(ringIndex) / static_cast<float>(splineSamples - 1) : 0.0f;
    const float vLocal = arcLengthFraction;
    const float v = (static_cast<float>(chainIndex) + vLocal) / static_cast<float>(std::max(totalChains, 1));
    return float2(insetLightmapUV(u), insetLightmapUV(v));
  }

  std::pair<double3, double3> ribbonFrame(double t,
                                          const double3 &tangent,
                                          const ProteinRibbonCenterlinePath &path,
                                          const double3 *previousTangent,
                                          const double3 *previousFaceNormal)
  {
    (void)previousTangent;
    (void)previousFaceNormal;
    // Match PyMOL get_system2f3f: x=tangent, y=orientation, z=cross(tangent, orientation).
    const double3 x = safeNormalize(tangent, double3(0.0, 0.0, 1.0));
    const double3 orientation = path.evaluateOrientation(t);
    const double3 z = safeNormalize(cross(x, orientation), perpendicularVector(x));
    const double3 y = safeNormalize(cross(z, x), orientation);
    return {y, z};
  }

  ProteinRibbonSecondaryStructure interpolatedSecondaryStructure(const std::vector<ProteinRibbonSecondaryStructure> &assignment,
                                                                 double residuePosition)
  {
    if (assignment.empty()) return ProteinRibbonSecondaryStructure::coil;
    const double clampedPosition = std::min(std::max(residuePosition, 0.0), static_cast<double>(assignment.size() - 1));
    const int lowerIndex = std::min(static_cast<int>(std::floor(clampedPosition)), static_cast<int>(assignment.size()) - 1);
    const int upperIndex = std::min(lowerIndex + 1, static_cast<int>(assignment.size()) - 1);
    const double localT = clampedPosition - static_cast<double>(lowerIndex);
    return ProteinRibbonSecondaryStructureAssigner::interpolate(assignment[lowerIndex], assignment[upperIndex], localT);
  }

  double sheetArrowFactor(double residuePosition,
                          const std::vector<ProteinRibbonSecondaryStructure> &assignment,
                          const ProteinRibbonMeshParameters &parameters)
  {
    const int index = std::min(std::max(static_cast<int>(std::round(residuePosition)), 0), static_cast<int>(assignment.size()) - 1);
    if (assignment[index] != ProteinRibbonSecondaryStructure::sheet) return 1.0;

    int runStart = index;
    while (runStart > 0 && assignment[runStart - 1] == ProteinRibbonSecondaryStructure::sheet) --runStart;
    int runEnd = index;
    while (runEnd + 1 < static_cast<int>(assignment.size()) && assignment[runEnd + 1] == ProteinRibbonSecondaryStructure::sheet) ++runEnd;

    const int runLength = runEnd - runStart + 1;
    if (runLength < 3) return 1.0;

    const double sheetArrowLengthExtent = parameters.sheetArrowLengthExtent;
    const double distanceFromTrailingEdge = static_cast<double>(runEnd) - residuePosition;
    if (distanceFromTrailingEdge > sheetArrowLengthExtent) return 1.0;
    if (distanceFromTrailingEdge <= 0.0) return 0.25;
    if (distanceFromTrailingEdge <= 1.0) return 0.25 + 0.25 * distanceFromTrailingEdge;
    return 0.5 + 0.5 * (distanceFromTrailingEdge - 1.0);
  }

  double sheetArrowWidthFactor(double residuePosition,
                               const std::vector<ProteinRibbonSecondaryStructure> &assignment,
                               const ProteinRibbonMeshParameters &parameters)
  {
    const int index = std::min(std::max(static_cast<int>(std::round(residuePosition)), 0), static_cast<int>(assignment.size()) - 1);
    if (assignment[index] != ProteinRibbonSecondaryStructure::sheet) return 1.0;

    int runStart = index;
    while (runStart > 0 && assignment[runStart - 1] == ProteinRibbonSecondaryStructure::sheet) --runStart;
    int runEnd = index;
    while (runEnd + 1 < static_cast<int>(assignment.size()) && assignment[runEnd + 1] == ProteinRibbonSecondaryStructure::sheet) ++runEnd;

    const int runLength = runEnd - runStart + 1;
    if (runLength < 3) return 1.0;

    const double sheetArrowLengthExtent = parameters.sheetArrowLengthExtent;
    const double sheetArrowPeakWidthFactor = parameters.sheetArrowPeakWidthFactor;
    const double distanceFromTrailingEdge = static_cast<double>(runEnd) - residuePosition;
    if (distanceFromTrailingEdge > sheetArrowLengthExtent) return 1.0;
    if (distanceFromTrailingEdge <= 0.0) return parameters.coilRadiusScale;
    const double tapered = sheetArrowPeakWidthFactor * (distanceFromTrailingEdge / sheetArrowLengthExtent);
    return std::max(tapered, parameters.coilRadiusScale);
  }

  CrossSectionProfile crossSectionProfile(const RibbonStation &station, const ProteinRibbonMeshParameters &parameters)
  {
    if (parameters.nucleicAcidRendering)
    {
      switch (parameters.nucleicAcidBackboneStyle)
      {
      case NucleicAcidBackboneStyle::oval:
        return {1.0, parameters.nucleicAcidOvalLength, parameters.nucleicAcidOvalWidth, false};
      case NucleicAcidBackboneStyle::tube:
        return {1.0, 1.0, 1.0, false};
      case NucleicAcidBackboneStyle::rect:
        return {1.0, parameters.nucleicAcidOvalLength, parameters.nucleicAcidOvalWidth, true};
      case NucleicAcidBackboneStyle::dumbbell:
        return {1.0, parameters.nucleicAcidDumbbellLength, parameters.nucleicAcidDumbbellWidth, false};
      }
    }
    switch (station.secondaryStructure)
    {
    case ProteinRibbonSecondaryStructure::coil:
      return {1.0, parameters.coilRadiusScale, 1.0, false};
    case ProteinRibbonSecondaryStructure::helix:
      // PyMOL ExtrudeOval: narrow cartoon_oval_width along orientation (cos / faceNormal),
      // full cartoon_oval_length along cross(tangent, orientation) (sin / widthDirection).
      return {1.0, 1.0, parameters.ribbonWidthClamp, false};
    case ProteinRibbonSecondaryStructure::sheet:
      // PyMOL ExtrudeRectangle + ExtrudeCGOSurfaceStrand: flat rectangle in the sheet plane
      // (wide along widthDirection). Arrow scales only that in-plane width; thickness stays constant.
      return {station.sheetArrowWidthFactor, 1.0, parameters.ribbonWidthClamp, true};
    }
    return {1.0, 1.0, 1.0, false};
  }

  struct RibbonBackboneSample
  {
    RibbonPoint3 center;
    RibbonPoint3 nitrogen;
    RibbonPoint3 carbonylCarbon;
    RibbonPoint3 carbonylOxygen;
    bool hasNitrogen = false;
    bool hasCarbonylCarbon = false;
    bool hasCarbonylOxygen = false;
  };

  double3 vectorFromPoints(const RibbonPoint3 &from, const RibbonPoint3 &to)
  {
    return double3(to.x - from.x, to.y - from.y, to.z - from.z);
  }

  std::vector<RibbonBackboneSample> ribbonBackboneSamples(const ProteinBackboneChain &chain, double3 contentShift)
  {
    std::vector<RibbonBackboneSample> samples;
    samples.reserve(chain.residues.size());
    for (const ProteinBackboneResidue &residue : chain.residues)
    {
      if (!residue.alphaCarbon) continue;

      RibbonBackboneSample sample;
      sample.center = RibbonPoint3::fromSum(residue.alphaCarbon->position(), contentShift);
      if (residue.nitrogen)
      {
        sample.hasNitrogen = true;
        sample.nitrogen = RibbonPoint3::fromSum(residue.nitrogen->position(), contentShift);
      }
      if (residue.carbonylCarbon)
      {
        sample.hasCarbonylCarbon = true;
        sample.carbonylCarbon = RibbonPoint3::fromSum(residue.carbonylCarbon->position(), contentShift);
      }
      if (residue.carbonylOxygen)
      {
        sample.hasCarbonylOxygen = true;
        sample.carbonylOxygen = RibbonPoint3::fromSum(residue.carbonylOxygen->position(), contentShift);
      }
      samples.push_back(sample);
    }
    return samples;
  }

  double3 orientationOrthogonalToTangent(const double3 &orientation, const double3 &tangent)
  {
    return safeNormalize(projectToPlane(orientation, tangent), orientation);
  }

  // PyMOL RepCartoon PASS1: cross(normalize(N-C), normalize(N-O)).
  std::vector<double3> computeCartoonOrientationVectors(const std::vector<RibbonBackboneSample> &samples)
  {
    const size_t count = samples.size();
    if (count == 0) return {};

    std::vector<double3> orientations;
    orientations.reserve(count);
    for (size_t index = 0; index < count; ++index)
    {
      const RibbonBackboneSample &sample = samples[index];
      double3 orientation;
      if (sample.hasNitrogen && sample.hasCarbonylCarbon && sample.hasCarbonylOxygen)
      {
        const double3 nToC = safeNormalize(vectorFromPoints(sample.carbonylCarbon, sample.nitrogen), double3(0.0, 0.0, 1.0));
        const double3 nToO = safeNormalize(vectorFromPoints(sample.carbonylOxygen, sample.nitrogen), double3(0.0, 0.0, 1.0));
        orientation = cross(nToC, nToO);
      }
      else if (index > 0 && index + 1 < count)
      {
        const double3 t0 = vectorFromPoints(samples[index - 1].center, sample.center);
        const double3 t1 = vectorFromPoints(samples[index + 1].center, sample.center);
        orientation = safeNormalize(t0 + t1, double3(0.0, 0.0, 1.0));
      }
      else
      {
        orientation = double3(0.0, 0.0, 0.0);
      }
      if (length_squared(orientation) < 1.0e-12) orientation = double3(0.0, 0.0, 0.0);
      else orientation = normalize(orientation);
      orientations.push_back(orientation);
    }
    return orientations;
  }

  // PyMOL RepCartoonComputeDifferencesAndNormals (chain direction normals).
  std::vector<double3> computeChainDirectionNormals(const std::vector<double3> &centers)
  {
    const int count = static_cast<int>(centers.size());
    if (count == 0) return {};
    if (count == 1) return {double3(0.0, 0.0, 0.0)};

    std::vector<double3> normals(count, double3(0.0, 0.0, 0.0));
    for (int index = 0; index < count - 1; ++index)
    {
      const double3 difference = centers[static_cast<size_t>(index + 1)] - centers[static_cast<size_t>(index)];
      const double segmentLength = length(difference);
      if (segmentLength > 1.0e-6)
      {
        normals[static_cast<size_t>(index)] = difference / segmentLength;
      }
      else if (index > 0)
      {
        normals[static_cast<size_t>(index)] = normals[static_cast<size_t>(index - 1)];
      }
    }
    normals.back() = normals[static_cast<size_t>(count - 2)];
    return normals;
  }

  // PyMOL RepCartoonComputeTangents.
  std::vector<double3> computeChainTangents(const std::vector<double3> &directionNormals)
  {
    const int count = static_cast<int>(directionNormals.size());
    if (count == 0) return {};
    if (count == 1) return directionNormals;

    std::vector<double3> tangents(count);
    tangents.front() = directionNormals.front();
    for (int index = 1; index < count - 1; ++index)
    {
      tangents[static_cast<size_t>(index)] =
        safeNormalize(directionNormals[static_cast<size_t>(index)] + directionNormals[static_cast<size_t>(index - 1)],
                      directionNormals[static_cast<size_t>(index)]);
    }
    tangents.back() = directionNormals[static_cast<size_t>(count - 2)];
    return tangents;
  }

  void setOrientationFromAxisCrossTangent(std::vector<double3> &orientations,
                                          const std::vector<double3> &tangents,
                                          int orientationIndex,
                                          const double3 &axis)
  {
    if (orientationIndex < 0 || orientationIndex >= static_cast<int>(orientations.size())) return;
    orientations[static_cast<size_t>(orientationIndex)] =
      orientationOrthogonalToTangent(cross(axis, tangents[static_cast<size_t>(orientationIndex)]),
                                   tangents[static_cast<size_t>(orientationIndex)]);
  }

  // PyMOL RepCartoonComputeRoundHelices (cartoon_round_helices).
  void computeRoundHelices(std::vector<double3> &orientations,
                           const std::vector<double3> &centers,
                           const std::vector<double3> &tangents,
                           const std::vector<ProteinRibbonSecondaryStructure> &secondaryStructure)
  {
    const int count = static_cast<int>(centers.size());
    if (count <= 1 || static_cast<int>(orientations.size()) != count || static_cast<int>(tangents.size()) != count)
    {
      return;
    }
    if (static_cast<int>(secondaryStructure.size()) != count) return;

    const double3 *helixCA1 = nullptr;
    const double3 *helixCA2 = nullptr;
    const double3 *helixCA3 = nullptr;
    const double3 *helixCA4 = nullptr;
    const double3 *helixCA5 = nullptr;
    int helixRoundPassCount = 0;
    std::optional<double3> previousHelixAxisPoint;

    for (int index = 0; index < count; ++index)
    {
      helixCA5 = helixCA4;
      helixCA4 = helixCA3;
      helixCA3 = helixCA2;
      helixCA2 = helixCA1;

      if (secondaryStructure[static_cast<size_t>(index)] == ProteinRibbonSecondaryStructure::helix)
      {
        helixCA1 = &centers[static_cast<size_t>(index)];
      }
      else
      {
        if (helixRoundPassCount < 2 && helixCA2 && helixCA3)
        {
          double3 axis = safeNormalize(*helixCA2 - centers[static_cast<size_t>(index)], double3(0.0, 0.0, 0.0));
          double3 segment = safeNormalize(*helixCA3 - *helixCA2, axis);
          axis = safeNormalize(axis + segment, axis);
          if (helixCA4)
          {
            segment = safeNormalize(*helixCA4 - *helixCA3, segment);
            axis = safeNormalize(axis + segment, axis);
          }
          if (helixCA5)
          {
            segment = safeNormalize(*helixCA5 - *helixCA4, segment);
            axis = safeNormalize(axis + segment, axis);
          }
          if (length_squared(axis) > 1.0e-12)
          {
            setOrientationFromAxisCrossTangent(orientations, tangents, index - 1, axis);
            setOrientationFromAxisCrossTangent(orientations, tangents, index - 2, axis);
            if (helixCA4)
            {
              setOrientationFromAxisCrossTangent(orientations, tangents, index - 3, axis);
            }
            if (helixCA5)
            {
              setOrientationFromAxisCrossTangent(orientations, tangents, index - 4, axis);
              if (index >= 4 &&
                  dot(orientations[static_cast<size_t>(index - 3)], orientations[static_cast<size_t>(index - 4)]) < -0.8)
              {
                orientations[static_cast<size_t>(index - 4)] = -orientations[static_cast<size_t>(index - 4)];
              }
            }
          }
        }
        helixCA1 = nullptr;
        helixCA2 = nullptr;
        helixCA3 = nullptr;
        helixCA4 = nullptr;
        helixCA5 = nullptr;
        helixRoundPassCount = 0;
        previousHelixAxisPoint.reset();
      }

      if (helixCA1 && helixCA2 && helixCA3 && helixCA4)
      {
        const double3 axisPoint =
          (*helixCA1 + *helixCA4) * 0.2130 + (*helixCA2 + *helixCA3) * 0.2870;
        if (helixRoundPassCount > 0 && previousHelixAxisPoint.has_value())
        {
          const double3 axisDirection =
            safeNormalize(*previousHelixAxisPoint - axisPoint, tangents[static_cast<size_t>(index)]);
          setOrientationFromAxisCrossTangent(orientations, tangents, index, axisDirection);
          setOrientationFromAxisCrossTangent(orientations, tangents, index - 1, axisDirection);
          setOrientationFromAxisCrossTangent(orientations, tangents, index - 2, axisDirection);
          if (helixRoundPassCount == 1)
          {
            setOrientationFromAxisCrossTangent(orientations, tangents, index - 3, axisDirection);
            setOrientationFromAxisCrossTangent(orientations, tangents, index - 4, axisDirection);
          }
        }
        helixRoundPassCount++;
        previousHelixAxisPoint = axisPoint;
      }
    }
  }

  // PyMOL RepCartoonRefineNormals (cartoon_refine_normals).
  void refineCartoonOrientationNormals(std::vector<double3> &orientations,
                                       const std::vector<double3> &tangents,
                                       const std::vector<ProteinRibbonSecondaryStructure> &secondaryStructure)
  {
    const int count = static_cast<int>(orientations.size());
    if (count < 2 || static_cast<int>(tangents.size()) != count) return;
    if (static_cast<int>(secondaryStructure.size()) != count) return;

    for (int index = 1; index < count - 1; ++index)
    {
      orientations[static_cast<size_t>(index)] =
        orientationOrthogonalToTangent(orientations[static_cast<size_t>(index)], tangents[static_cast<size_t>(index)]);
    }

    std::vector<double3> alternatives(static_cast<size_t>(count) * 2);
    for (int index = 0; index < count; ++index)
    {
      alternatives[static_cast<size_t>(index) * 2] = orientations[static_cast<size_t>(index)];
      alternatives[static_cast<size_t>(index) * 2 + 1] = orientations[static_cast<size_t>(index)];
      if (secondaryStructure[static_cast<size_t>(index)] != ProteinRibbonSecondaryStructure::helix)
      {
        alternatives[static_cast<size_t>(index) * 2 + 1] = -alternatives[static_cast<size_t>(index) * 2 + 1];
      }
    }

    for (int index = 1; index < count - 1; ++index)
    {
      const double3 previousOrientation =
        orientationOrthogonalToTangent(orientations[static_cast<size_t>(index - 1)], tangents[static_cast<size_t>(index)]);
      const double3 candidateA =
        orientationOrthogonalToTangent(alternatives[static_cast<size_t>(index) * 2], tangents[static_cast<size_t>(index)]);
      const double3 candidateB =
        orientationOrthogonalToTangent(alternatives[static_cast<size_t>(index) * 2 + 1], tangents[static_cast<size_t>(index)]);
      double bestDot = dot(previousOrientation, candidateA);
      orientations[static_cast<size_t>(index)] = alternatives[static_cast<size_t>(index) * 2];
      const double alternateDot = dot(previousOrientation, candidateB);
      if (alternateDot > bestDot)
      {
        orientations[static_cast<size_t>(index)] = alternatives[static_cast<size_t>(index) * 2 + 1];
      }
    }

    std::vector<double3> softenedAlternatives(static_cast<size_t>(count) * 2, double3(0.0, 0.0, 0.0));
    for (int index = 1; index < count - 1; ++index)
    {
      const double kinkMetric = dot(orientations[static_cast<size_t>(index)], orientations[static_cast<size_t>(index + 1)]) *
                                dot(orientations[static_cast<size_t>(index)], orientations[static_cast<size_t>(index - 1)]);
      if (kinkMetric < -0.10)
      {
        double3 blended = orientations[static_cast<size_t>(index + 1)] + orientations[static_cast<size_t>(index - 1)];
        blended += orientations[static_cast<size_t>(index)] * 0.001;
        blended = orientationOrthogonalToTangent(blended, tangents[static_cast<size_t>(index)]);
        double3 adjusted = dot(orientations[static_cast<size_t>(index)], blended) < 0.0
                             ? orientations[static_cast<size_t>(index)] - blended
                             : orientations[static_cast<size_t>(index)] + blended;
        adjusted = normalize(adjusted);
        double mixAmount = 2.0 * (-0.10 - kinkMetric);
        mixAmount = std::min(mixAmount, 1.0);
        softenedAlternatives[static_cast<size_t>(index) * 2] =
          normalize(orientations[static_cast<size_t>(index)] * (1.0 - mixAmount) + adjusted * mixAmount);
        softenedAlternatives[static_cast<size_t>(index) * 2 + 1] = -softenedAlternatives[static_cast<size_t>(index) * 2];
      }
      else
      {
        softenedAlternatives[static_cast<size_t>(index) * 2] = orientations[static_cast<size_t>(index)];
        softenedAlternatives[static_cast<size_t>(index) * 2 + 1] = -softenedAlternatives[static_cast<size_t>(index) * 2];
      }
    }

    for (int index = 1; index < count - 1; ++index)
    {
      orientations[static_cast<size_t>(index)] = softenedAlternatives[static_cast<size_t>(index) * 2];
    }
  }

  // PyMOL RepCartoonFlattenSheets (cartoon_flat_sheets / cartoon_flat_cycles).
  void flattenCartoonSheets(std::vector<double3> &centers,
                            std::vector<double3> &orientations,
                            const std::vector<ProteinRibbonSecondaryStructure> &secondaryStructure,
                            int cycles)
  {
    const int count = static_cast<int>(centers.size());
    if (cycles <= 0 || count < 3) return;
    if (static_cast<int>(orientations.size()) != count) return;
    if (static_cast<int>(secondaryStructure.size()) != count) return;

    std::vector<double3> tmpCenters = centers;
    std::vector<double3> tmpOrientations = orientations;
    const int window = 1;

    int index = 0;
    while (index < count)
    {
      if (secondaryStructure[static_cast<size_t>(index)] != ProteinRibbonSecondaryStructure::sheet)
      {
        ++index;
        continue;
      }

      const int runStart = index;
      while (index < count && secondaryStructure[static_cast<size_t>(index)] == ProteinRibbonSecondaryStructure::sheet)
      {
        ++index;
      }
      const int runEnd = index - 1;
      if (runEnd - runStart < 2 * window) continue;

      for (int cycle = 0; cycle < cycles; ++cycle)
      {
        for (int b = runStart + window; b <= runEnd - window; ++b)
        {
          double3 sum(0.0, 0.0, 0.0);
          for (int e = -window; e <= window; ++e)
          {
            sum += centers[static_cast<size_t>(b + e)];
          }
          tmpCenters[static_cast<size_t>(b)] = sum / static_cast<double>(window * 2 + 1);
        }
        for (int b = runStart + window; b <= runEnd - window; ++b)
        {
          centers[static_cast<size_t>(b)] = tmpCenters[static_cast<size_t>(b)];
        }

        for (int b = runStart + window; b <= runEnd - window; ++b)
        {
          double3 sum(0.0, 0.0, 0.0);
          for (int e = -window; e <= window; ++e)
          {
            sum += orientations[static_cast<size_t>(b + e)];
          }
          tmpOrientations[static_cast<size_t>(b)] = sum / static_cast<double>(window * 2 + 1);
        }
        for (int b = runStart + window; b <= runEnd - window; ++b)
        {
          const double3 tangent = safeNormalize(centers[static_cast<size_t>(b + 1)] - centers[static_cast<size_t>(b - 1)],
                                                perpendicularVector(orientations[static_cast<size_t>(b)]));
          orientations[static_cast<size_t>(b)] =
            orientationOrthogonalToTangent(tmpOrientations[static_cast<size_t>(b)], tangent);
        }
      }
    }
  }

  enum class NucleicAcidTraceRole
  {
    o5PrimeCap,
    phosphate,
    o3PrimeCap
  };

  struct NucleicAcidBackboneSample
  {
    RibbonPoint3 center;
    RibbonPoint3 c2;
    RibbonPoint3 c3;
    bool hasC2 = false;
    bool hasC3 = false;
    NucleicAcidTraceRole role = NucleicAcidTraceRole::phosphate;
    int residueIndex = 0;
    double nucleotidePosition = 0.0;
  };

  void assignNucleicAcidOrientationAtoms(const DNABackboneResidue &residue,
                                         double3 contentShift,
                                         NucleicAcidBackboneSample &sample)
  {
    if (residue.c2Prime)
    {
      sample.hasC2 = true;
      sample.c2 = RibbonPoint3::fromSum(residue.c2Prime->position(), contentShift);
    }
    if (residue.c3Prime)
    {
      sample.hasC3 = true;
      sample.c3 = RibbonPoint3::fromSum(residue.c3Prime->position(), contentShift);
    }
  }

  NucleicAcidBackboneSample makeNucleicAcidTraceSample(const DNABackboneResidue &residue,
                                                       const std::shared_ptr<SKAsymmetricAtom> &centerAtom,
                                                       double3 contentShift,
                                                       NucleicAcidTraceRole role,
                                                       int residueIndex,
                                                       double nucleotidePosition)
  {
    NucleicAcidBackboneSample sample;
    sample.center = RibbonPoint3::fromSum(centerAtom->position(), contentShift);
    assignNucleicAcidOrientationAtoms(residue, contentShift, sample);
    sample.role = role;
    sample.residueIndex = residueIndex;
    sample.nucleotidePosition = nucleotidePosition;
    return sample;
  }

  // PyMOL cartoon_nucleic_acid_mode 4 (P + O5'/O3' caps) or mode 1 (C3' trace).
  std::vector<NucleicAcidBackboneSample> nucleicAcidBackboneSamples(const DNABackboneChain &chain,
                                                                  double3 contentShift,
                                                                  NucleicAcidTraceMode traceMode)
  {
    const int residueCount = static_cast<int>(chain.residues.size());
    if (residueCount == 0) return {};

    std::vector<NucleicAcidBackboneSample> samples;
    samples.reserve(static_cast<size_t>(residueCount) + 2U);

    if (traceMode == NucleicAcidTraceMode::c3PrimeMode1)
    {
      for (int residueIndex = 0; residueIndex < residueCount; ++residueIndex)
      {
        const DNABackboneResidue &residue = chain.residues[static_cast<size_t>(residueIndex)];
        if (!residue.c3Prime) continue;
        samples.push_back(makeNucleicAcidTraceSample(residue,
                                                     residue.c3Prime,
                                                     contentShift,
                                                     NucleicAcidTraceRole::phosphate,
                                                     residueIndex,
                                                     static_cast<double>(residueIndex)));
      }
      return samples;
    }

    const DNABackboneResidue &firstResidue = chain.residues.front();
    if (firstResidue.o5Prime)
    {
      samples.push_back(makeNucleicAcidTraceSample(firstResidue,
                                                   firstResidue.o5Prime,
                                                   contentShift,
                                                   NucleicAcidTraceRole::o5PrimeCap,
                                                   0,
                                                   0.0));
    }

    for (int residueIndex = 0; residueIndex < residueCount; ++residueIndex)
    {
      const DNABackboneResidue &residue = chain.residues[static_cast<size_t>(residueIndex)];
      if (!residue.phosphate) continue;
      samples.push_back(makeNucleicAcidTraceSample(residue,
                                                   residue.phosphate,
                                                   contentShift,
                                                   NucleicAcidTraceRole::phosphate,
                                                   residueIndex,
                                                   static_cast<double>(residueIndex)));
    }

    const DNABackboneResidue &lastResidue = chain.residues.back();
    if (lastResidue.o3Prime)
    {
      samples.push_back(makeNucleicAcidTraceSample(lastResidue,
                                                   lastResidue.o3Prime,
                                                   contentShift,
                                                   NucleicAcidTraceRole::o3PrimeCap,
                                                   residueCount - 1,
                                                   static_cast<double>(residueCount - 1)));
    }

    return samples;
  }

  std::vector<double3> nucleicAcidNucleotidePositions(const std::vector<NucleicAcidBackboneSample> &samples)
  {
    std::vector<double3> positions;
    positions.reserve(samples.size());
    for (const NucleicAcidBackboneSample &sample : samples)
    {
      positions.emplace_back(sample.nucleotidePosition, 0.0, 0.0);
    }
    return positions;
  }

  std::vector<double3> computeNucleicAcidOrientationVectors(const std::vector<NucleicAcidBackboneSample> &samples)
  {
    const size_t count = samples.size();
    if (count == 0) return {};

    std::vector<double3> orientations;
    orientations.reserve(count);
    std::optional<RibbonPoint3> previousC2;

    for (size_t index = 0; index < count; ++index)
    {
      const NucleicAcidBackboneSample &sample = samples[index];
      double3 orientation = double3(0.0, 0.0, 0.0);
      if (sample.hasC2 && sample.hasC3)
      {
        const double3 c3 = sample.c3.asDouble3();
        const double3 c2 = sample.c2.asDouble3();
        if (previousC2.has_value())
        {
          const double3 midpoint = (c2 + previousC2->asDouble3() * 2.0) * (1.0 / 3.0);
          orientation = safeNormalize(c3 - midpoint, double3(0.0, 0.0, 0.0));
        }
        else
        {
          orientation = safeNormalize(c3 - c2, double3(0.0, 0.0, 0.0));
        }
        previousC2 = sample.c2;
      }
      else if (index > 0 && index + 1 < count)
      {
        const double3 t0 = sample.center.asDouble3() - samples[index - 1].center.asDouble3();
        const double3 t1 = samples[index + 1].center.asDouble3() - sample.center.asDouble3();
        orientation = safeNormalize(t0 + t1, double3(0.0, 0.0, 1.0));
        previousC2 = std::nullopt;
      }
      else
      {
        previousC2 = std::nullopt;
      }
      orientations.push_back(orientation);
    }
    return orientations;
  }

  double3 tangentAtNucleicAcidSamples(const std::vector<NucleicAcidBackboneSample> &samples, int index)
  {
    const int count = static_cast<int>(samples.size());
    const int clampedIndex = std::max(0, std::min(index, count - 1));
    const auto point = [](const RibbonPoint3 &p) { return double3(p.x, p.y, p.z); };
    if (count < 2) return double3(0.0, 0.0, 1.0);
    if (clampedIndex == 0) return normalize(point(samples[1].center) - point(samples[0].center));
    if (clampedIndex == count - 1)
    {
      return normalize(point(samples[clampedIndex].center) - point(samples[clampedIndex - 1].center));
    }
    return normalize(point(samples[clampedIndex + 1].center) - point(samples[clampedIndex - 1].center));
  }

  std::vector<RibbonStation> ribbonStationsFromNucleicAcid(const std::vector<NucleicAcidBackboneSample> &samples,
                                                           const std::vector<ProteinRibbonSecondaryStructure> &secondaryStructure,
                                                           const ProteinRibbonMeshParameters &parameters)
  {
    if (samples.size() < 2) return {};

    const int centerCount = static_cast<int>(samples.size());
    std::vector<double3> centers;
    centers.reserve(samples.size());
    for (const NucleicAcidBackboneSample &sample : samples)
    {
      centers.emplace_back(sample.center.x, sample.center.y, sample.center.z);
    }

    std::vector<double3> orientationVectors = computeNucleicAcidOrientationVectors(samples);
    const std::vector<double3> directionNormals = computeChainDirectionNormals(centers);
    const std::vector<double3> chainTangents = computeChainTangents(directionNormals);
    // PyMOL RepCartoonComputeRoundHelices only runs for ss_t::HELIX, not NUCLEIC — keep C2'/C3' frame.
    refineCartoonOrientationNormals(orientationVectors, chainTangents, secondaryStructure);

    const std::vector<double3> nucleotidePositions = nucleicAcidNucleotidePositions(samples);
    std::vector<double3> nucleotidePositionVectors = nucleotidePositions;
    std::vector<double3> nucleotideOrientationAxes(nucleotidePositions.size(), double3(0.0, 0.0, 1.0));

    // PyMOL interpolates through trace atoms (phosphate / terminal O5'/O3'), not an approximating B-spline.
    std::unique_ptr<ProteinRibbonCenterlinePath> path =
      std::make_unique<ProteinCatmullRomSpline>(std::move(centers), std::move(orientationVectors));
    std::unique_ptr<ProteinRibbonCenterlinePath> nucleotidePath =
      std::make_unique<ProteinCatmullRomSpline>(std::move(nucleotidePositionVectors), std::move(nucleotideOrientationAxes));

    const double totalLength = path->arcLength(1.0);
    const int subdivisionsPerSegment = parameters.subdivisionsPerSegment;
    const int sampleCount = (centerCount - 1) * subdivisionsPerSegment + 1;

    std::vector<RibbonStation> stations;
    stations.reserve(sampleCount);
    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
      const double targetLength = static_cast<double>(sampleIndex) / static_cast<double>(sampleCount - 1) * totalLength;
      const double t = path->parameterFromArcLength(targetLength);
      const double residuePosition = nucleotidePath->evaluate(t).x;
      const int nearestSampleIndex =
        std::min(std::max(static_cast<int>(std::round(t * static_cast<double>(centerCount - 1))), 0), centerCount - 1);

      const double3 center = path->evaluate(t);
      const double3 tangent = safeNormalize(path->derivative(t),
                                            tangentAtNucleicAcidSamples(samples, nearestSampleIndex));
      const double3 previousTangentValue = stations.empty() ? double3(0.0, 0.0, 0.0) : stations.back().tangent.asDouble3();
      const double3 previousFaceNormalValue = stations.empty() ? double3(0.0, 0.0, 0.0) : stations.back().faceNormal.asDouble3();
      const double3 *previousTangent = stations.empty() ? nullptr : &previousTangentValue;
      const double3 *previousFaceNormal = stations.empty() ? nullptr : &previousFaceNormalValue;
      const auto frame = ribbonFrame(t, tangent, *path, previousTangent, previousFaceNormal);

      stations.push_back({RibbonPoint3::from(center),
                          RibbonPoint3::from(tangent),
                          RibbonPoint3::from(frame.first),
                          RibbonPoint3::from(frame.second),
                          interpolatedSecondaryStructure(secondaryStructure, residuePosition),
                          sheetArrowFactor(residuePosition, secondaryStructure, parameters),
                          sheetArrowWidthFactor(residuePosition, secondaryStructure, parameters),
                          residuePosition});
    }
    return stations;
  }

  std::vector<double3> ribbonCenterline(const std::vector<RibbonBackboneSample> &samples)
  {
    std::vector<double3> centers;
    centers.reserve(samples.size());
    for (const RibbonBackboneSample &sample : samples)
    {
      centers.emplace_back(sample.center.x, sample.center.y, sample.center.z);
    }
    return centers;
  }

  double3 tangentAtSamples(const std::vector<RibbonBackboneSample> &samples, int index)
  {
    const int count = static_cast<int>(samples.size());
    const int clampedIndex = std::max(0, std::min(index, count - 1));
    const auto point = [](const RibbonPoint3 &p) { return double3(p.x, p.y, p.z); };
    if (count < 2) return double3(0.0, 0.0, 1.0);
    if (clampedIndex == 0) return normalize(point(samples[1].center) - point(samples[0].center));
    if (clampedIndex == count - 1) return normalize(point(samples[clampedIndex].center) - point(samples[clampedIndex - 1].center));
    return normalize(point(samples[clampedIndex + 1].center) - point(samples[clampedIndex - 1].center));
  }

  std::vector<RibbonStation> ribbonStations(const ProteinBackboneChain &chain,
                                            double3 contentShift,
                                            const std::vector<ProteinRibbonSecondaryStructure> &secondaryStructure,
                                            const ProteinRibbonMeshParameters &parameters)
  {
    std::vector<RibbonBackboneSample> samples = ribbonBackboneSamples(chain, contentShift);
    if (samples.size() < 2) return {};

    const int centerCount = static_cast<int>(samples.size());
    std::vector<double3> centers = ribbonCenterline(samples);
    std::vector<double3> orientationVectors = computeCartoonOrientationVectors(samples);
    const std::vector<double3> directionNormals = computeChainDirectionNormals(centers);
    const std::vector<double3> chainTangents = computeChainTangents(directionNormals);
    computeRoundHelices(orientationVectors, centers, chainTangents, secondaryStructure);
    refineCartoonOrientationNormals(orientationVectors, chainTangents, secondaryStructure);
    // PyMOL cartoon_flat_sheets: tauten β-strands by averaging Cα and orientations.
    flattenCartoonSheets(centers, orientationVectors, secondaryStructure, parameters.normalSmoothingRadius);

    std::unique_ptr<ProteinRibbonCenterlinePath> path;
    if (parameters.splineType == ProteinRibbonSplineType::bSpline)
    {
      path = std::make_unique<ProteinBSpline>(std::move(centers), std::move(orientationVectors), 3);
    }
    else
    {
      path = std::make_unique<ProteinCatmullRomSpline>(std::move(centers), std::move(orientationVectors));
    }

    const double totalLength = path->arcLength(1.0);
    const int subdivisionsPerSegment = parameters.subdivisionsPerSegment;
    const int sampleCount = (centerCount - 1) * subdivisionsPerSegment + 1;

    std::vector<RibbonStation> stations;
    stations.reserve(sampleCount);
    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
      const double targetLength = static_cast<double>(sampleIndex) / static_cast<double>(sampleCount - 1) * totalLength;
      const double t = path->parameterFromArcLength(targetLength);
      const double residuePosition = t * static_cast<double>(centerCount - 1);

      const double3 center = path->evaluate(t);
      const double3 tangent = safeNormalize(path->derivative(t),
                                            tangentAtSamples(samples,
                                                             std::min(static_cast<int>(std::round(residuePosition)), centerCount - 1)));
      const double3 previousTangentValue = stations.empty() ? double3(0.0, 0.0, 0.0) : stations.back().tangent.asDouble3();
      const double3 previousFaceNormalValue = stations.empty() ? double3(0.0, 0.0, 0.0) : stations.back().faceNormal.asDouble3();
      const double3 *previousTangent = stations.empty() ? nullptr : &previousTangentValue;
      const double3 *previousFaceNormal = stations.empty() ? nullptr : &previousFaceNormalValue;
      const auto frame = ribbonFrame(t, tangent, *path, previousTangent, previousFaceNormal);

      stations.push_back({RibbonPoint3::from(center),
                          RibbonPoint3::from(tangent),
                          RibbonPoint3::from(frame.first),
                          RibbonPoint3::from(frame.second),
                          interpolatedSecondaryStructure(secondaryStructure, residuePosition),
                          sheetArrowFactor(residuePosition, secondaryStructure, parameters),
                          sheetArrowWidthFactor(residuePosition, secondaryStructure, parameters),
                          residuePosition});
    }
    return stations;
  }

  void applyRibbonStripeCoordinates(RKRibbonMesh &mesh)
  {
    for (const RKRibbonChainDrawRange &drawRange : mesh.residueDrawRanges)
    {
      if (drawRange.indexCount <= 0) continue;
      const int end = drawRange.indexStart + drawRange.indexCount;
      float minU = std::numeric_limits<float>::max();
      float maxU = -std::numeric_limits<float>::max();
      for (int indexOffset = drawRange.indexStart; indexOffset < end; ++indexOffset)
      {
        const float alongChain = mesh.vertices[mesh.indices[static_cast<size_t>(indexOffset)]].st.x;
        minU = std::min(minU, alongChain);
        maxU = std::max(maxU, alongChain);
      }
      const float span = std::max(maxU - minU, 1.0e-6f);
      for (int indexOffset = drawRange.indexStart; indexOffset < end; ++indexOffset)
      {
        RKVertex &vertex = mesh.vertices[mesh.indices[static_cast<size_t>(indexOffset)]];
        vertex.stripeST.x = (vertex.st.x - minU) / span;
      }
    }
  }

  void applyPickingIndices(RKRibbonMesh &mesh)
  {
    for (size_t segmentIndex = 0; segmentIndex < mesh.segmentDrawRanges.size(); ++segmentIndex)
    {
      const RKRibbonChainDrawRange &drawRange = mesh.segmentDrawRanges[segmentIndex];
      if (drawRange.indexCount <= 0) continue;
      const int end = drawRange.indexStart + drawRange.indexCount;
      for (int indexOffset = drawRange.indexStart; indexOffset < end; ++indexOffset)
      {
        mesh.vertices[mesh.indices[static_cast<size_t>(indexOffset)]].normal.w = static_cast<float>(segmentIndex);
      }
    }
    for (size_t residueIndex = 0; residueIndex < mesh.residueDrawRanges.size(); ++residueIndex)
    {
      const RKRibbonChainDrawRange &drawRange = mesh.residueDrawRanges[residueIndex];
      if (drawRange.indexCount <= 0) continue;
      const int end = drawRange.indexStart + drawRange.indexCount;
      for (int indexOffset = drawRange.indexStart; indexOffset < end; ++indexOffset)
      {
        mesh.vertices[mesh.indices[static_cast<size_t>(indexOffset)]].pad.y = static_cast<float>(residueIndex);
      }
    }
  }

  void appendResidueDrawRanges(const ProteinBackboneChain &chain,
                               const std::vector<RibbonStation> &stations,
                               int chainIndexStart,
                               int ringResolution,
                               RKRibbonMesh &mesh)
  {
    if (stations.size() < 2) return;
    std::vector<std::shared_ptr<SKAsymmetricAtom>> alphaCarbons;
    for (const ProteinBackboneResidue &residue : chain.residues)
    {
      if (residue.alphaCarbon) alphaCarbons.push_back(residue.alphaCarbon);
    }
    const int residueCount = static_cast<int>(alphaCarbons.size());
    if (residueCount <= 0) return;

    const int trianglesPerRingPair = ringResolution * 6;
    for (int residueIndex = 0; residueIndex < residueCount; ++residueIndex)
    {
      std::optional<int> ringStart;
      std::optional<int> ringEnd;
      for (size_t stationIndex = 0; stationIndex < stations.size(); ++stationIndex)
      {
        const int stationResidueIndex = static_cast<int>(std::round(stations[stationIndex].residuePosition));
        if (stationResidueIndex == residueIndex)
        {
          ringStart = ringStart.has_value() ? std::min(*ringStart, static_cast<int>(stationIndex)) : static_cast<int>(stationIndex);
          ringEnd = ringEnd.has_value() ? std::max(*ringEnd, static_cast<int>(stationIndex)) : static_cast<int>(stationIndex);
        }
      }
      if (!ringStart.has_value() || !ringEnd.has_value() || *ringEnd <= *ringStart) continue;
      int drawEndRing = *ringEnd;
      if (residueIndex + 1 < residueCount)
      {
        drawEndRing = std::min(static_cast<int>(stations.size()) - 1, *ringEnd + 1);
      }
      mesh.residueDrawRanges.emplace_back(chainIndexStart + *ringStart * trianglesPerRingPair,
                                          (drawEndRing - *ringStart) * trianglesPerRingPair);
      const int tag = (residueIndex >= 0 && residueIndex < residueCount) ? alphaCarbons[static_cast<size_t>(residueIndex)]->tag() : -1;
      mesh.residueAlphaCarbonTags.push_back(tag);
    }
  }

  void appendSegmentDrawRanges(const ProteinBackboneChain &chain,
                               const std::vector<RibbonStation> &stations,
                               int chainIndexStart,
                               const std::vector<ProteinRibbonResidueSegment> &residueSegments,
                               int ringResolution,
                               RKRibbonMesh &mesh)
  {
    if (stations.size() < 2) return;
    std::vector<std::shared_ptr<SKAsymmetricAtom>> alphaCarbons;
    for (const ProteinBackboneResidue &residue : chain.residues)
    {
      if (residue.alphaCarbon) alphaCarbons.push_back(residue.alphaCarbon);
    }
    const int trianglesPerRingPair = ringResolution * 6;
    for (size_t segmentIndex = 0; segmentIndex < residueSegments.size(); ++segmentIndex)
    {
      const ProteinRibbonResidueSegment &segment = residueSegments[segmentIndex];
      std::optional<int> ringStart;
      std::optional<int> ringEnd;
      for (size_t stationIndex = 0; stationIndex < stations.size(); ++stationIndex)
      {
        const int residueIndex = static_cast<int>(std::round(stations[stationIndex].residuePosition));
        if (residueIndex >= segment.firstResidueIndex && residueIndex <= segment.lastResidueIndex)
        {
          ringStart = ringStart.has_value() ? std::min(*ringStart, static_cast<int>(stationIndex)) : static_cast<int>(stationIndex);
          ringEnd = ringEnd.has_value() ? std::max(*ringEnd, static_cast<int>(stationIndex)) : static_cast<int>(stationIndex);
        }
      }
      if (!ringStart.has_value() || !ringEnd.has_value() || *ringEnd <= *ringStart) continue;
      int drawEndRing = *ringEnd;
      if (segmentIndex + 1 < residueSegments.size())
      {
        drawEndRing = std::min(static_cast<int>(stations.size()) - 1, *ringEnd + 1);
      }
      mesh.segmentDrawRanges.emplace_back(chainIndexStart + *ringStart * trianglesPerRingPair,
                                          (drawEndRing - *ringStart) * trianglesPerRingPair);
      const int firstIndex = segment.firstResidueIndex;
      const int tag = (firstIndex >= 0 && firstIndex < static_cast<int>(alphaCarbons.size()))
                        ? alphaCarbons[static_cast<size_t>(firstIndex)]->tag()
                        : -1;
      mesh.segmentAlphaCarbonTags.push_back(tag);
    }
  }

  struct IndexedRibbonGeometry
  {
    std::vector<RKVertex> vertices;
    std::vector<uint32_t> indices;
  };

  void appendIndexedGeometry(RKRibbonMesh &mesh, const IndexedRibbonGeometry &geometry)
  {
    const uint32_t vertexBase = static_cast<uint32_t>(mesh.vertices.size());
    mesh.vertices.insert(mesh.vertices.end(), geometry.vertices.begin(), geometry.vertices.end());
    mesh.indices.reserve(mesh.indices.size() + geometry.indices.size());
    for (uint32_t index : geometry.indices)
    {
      mesh.indices.push_back(vertexBase + index);
    }
  }

  IndexedRibbonGeometry ringMesh(const std::vector<RibbonStation> &stations,
                                 double radius,
                                 int chainIndex,
                                 int totalChains,
                                 int ringResolution,
                                 const ProteinRibbonMeshParameters &parameters,
                                 int ringOffsetWithinChain = 0,
                                 int chainRingTotal = 0)
  {
    const int splineSamples = static_cast<int>(stations.size());
    if (splineSamples < 2) return {};
    const int lightmapRingTotal = chainRingTotal > 0 ? chainRingTotal : splineSamples;

    const int loopResolution = ringResolution;
    const int verticesPerRing = loopResolution + 1;
    const int totalVertices = splineSamples * verticesPerRing;

    std::vector<std::vector<RibbonPoint3>> ringPositions;
    ringPositions.reserve(splineSamples);
    std::vector<float> ringStructureTypes;
    ringStructureTypes.reserve(splineSamples);

    for (const RibbonStation &station : stations)
    {
      const CrossSectionProfile profile = crossSectionProfile(station, parameters);
      const float structureType = parameters.nucleicAcidRendering
                                    ? SKNucleotideBase::vertexStructureTypeCode(SKNucleotideBaseKind::unknown, true)
                                    : structureTypeCode(station.secondaryStructure);
      std::vector<RibbonPoint3> ring;
      ring.reserve(loopResolution);
      // Lightmap UV seam is at segment 0 / loopResolution. Offset by π/2 so the seam sits on a thin lateral edge.
      const double seamAngleOffset = 0.5 * M_PI;
      for (int segment = 0; segment < loopResolution; ++segment)
      {
        const double angle = 2.0 * M_PI * static_cast<double>(segment) / static_cast<double>(loopResolution) + seamAngleOffset;
        double3 offset;
        if (parameters.nucleicAcidRendering)
        {
          const NucleicAcidCrossSectionOffset crossSection = nucleicAcidCrossSectionOffset(angle, radius, parameters);
          offset = crossSection.alongWidthDirection * station.widthDirection.asDouble3() +
                   crossSection.alongFaceNormal * station.faceNormal.asDouble3();
        }
        else
        {
          const double angleCos = std::cos(angle);
          const double angleSin = std::sin(angle);
          double d = 0.0;
          double n = 0.0;
          if (profile.rectangular)
          {
            // Unit square via max-norm: flat faces in the sheet plane (PyMOL ExtrudeRectangle).
            const double m = std::max(std::abs(angleCos), std::abs(angleSin));
            const double scale = m > 1.0e-12 ? 1.0 / m : 1.0;
            d = angleSin * scale * profile.widthClamp;
            n = angleCos * scale * profile.normalScale;
          }
          else if (station.secondaryStructure == ProteinRibbonSecondaryStructure::helix)
          {
            double helixN = angleCos;
            if (profile.normalScale < 1.0)
            {
              helixN = std::max(-profile.normalScale, std::min(profile.normalScale, helixN));
            }
            d = angleSin * profile.widthClamp;
            n = helixN;
          }
          else
          {
            double width = angleSin;
            double normal = angleCos;
            if (profile.widthClamp < 1.0) width = std::max(-profile.widthClamp, std::min(profile.widthClamp, width));
            normal *= profile.normalScale;
            d = width;
            n = normal;
          }
          offset = profile.radiusScale * radius * (d * station.widthDirection.asDouble3() + n * station.faceNormal.asDouble3());
        }
        ring.push_back(add(station.center, offset));
      }
      ringPositions.push_back(std::move(ring));
      ringStructureTypes.push_back(structureType);
    }

    std::vector<RKVertex> meshVertices(totalVertices);
    std::vector<RibbonPoint3> vertexNormals(totalVertices);
    std::vector<int> normalCounts(totalVertices, 0);

    auto accumulateNormal = [&](int vertexIndex, const double3 &normal)
    {
      vertexNormals[vertexIndex] += normal;
      normalCounts[vertexIndex] += 1;
    };

    for (int ringIndex = 0; ringIndex < splineSamples; ++ringIndex)
    {
      std::vector<double> distAroundRing(verticesPerRing, 0.0);
      RibbonPoint3 previousPosition = ringPositions[ringIndex][0];
      for (int segment = 1; segment <= loopResolution; ++segment)
      {
        const int wrappedSegment = segment % loopResolution;
        const RibbonPoint3 position = ringPositions[ringIndex][static_cast<size_t>(wrappedSegment)];
        distAroundRing[static_cast<size_t>(segment)] =
          distAroundRing[static_cast<size_t>(segment - 1)] + lengthBetween(position, previousPosition);
        previousPosition = position;
      }
      const double totalRingDistance = distAroundRing[static_cast<size_t>(loopResolution)];

      for (int segment = 0; segment <= loopResolution; ++segment)
      {
        const int vertexIndex = ringIndex * verticesPerRing + segment;
        const int wrappedSegment = segment % loopResolution;
        const RibbonPoint3 position = ringPositions[ringIndex][static_cast<size_t>(wrappedSegment)];
        const float arcLengthFraction = totalRingDistance > 1.0e-12f
            ? static_cast<float>(distAroundRing[static_cast<size_t>(segment)] / totalRingDistance)
            : 0.0f;

        meshVertices[vertexIndex].position = float4(static_cast<float>(position.x),
                                                    static_cast<float>(position.y),
                                                    static_cast<float>(position.z),
                                                    1.0f);
        meshVertices[vertexIndex].st = parametricLightmapUV(ringOffsetWithinChain + ringIndex,
                                                            lightmapRingTotal,
                                                            arcLengthFraction,
                                                            chainIndex,
                                                            totalChains);
        meshVertices[vertexIndex].pad = float2(ringStructureTypes[static_cast<size_t>(ringIndex)], 0.0f);
        meshVertices[vertexIndex].stripeST = float2(0.0f, arcLengthFraction);
      }
    }

    for (int ringIndex = 0; ringIndex < splineSamples - 1; ++ringIndex)
    {
      for (int segment = 0; segment < loopResolution; ++segment)
      {
        const int v0 = ringIndex * verticesPerRing + segment;
        const int v1Base = ringIndex * verticesPerRing + (segment + 1) % loopResolution;
        const int v2 = (ringIndex + 1) * verticesPerRing + segment;
        const int v3Base = (ringIndex + 1) * verticesPerRing + (segment + 1) % loopResolution;

        const double3 pos0 = ringPositions[static_cast<size_t>(ringIndex)][static_cast<size_t>(segment)].asDouble3();
        const double3 pos1 = ringPositions[static_cast<size_t>(ringIndex)][static_cast<size_t>((segment + 1) % loopResolution)].asDouble3();
        const double3 pos2 = ringPositions[static_cast<size_t>(ringIndex + 1)][static_cast<size_t>(segment)].asDouble3();
        const double3 pos3 = ringPositions[static_cast<size_t>(ringIndex + 1)][static_cast<size_t>((segment + 1) % loopResolution)].asDouble3();

        const double3 normal1 = safeNormalize(cross(pos1 - pos0, pos2 - pos0), double3(0.0, 0.0, 1.0));
        accumulateNormal(v0, normal1);
        accumulateNormal(v1Base, normal1);
        accumulateNormal(v2, normal1);

        const double3 normal2 = safeNormalize(cross(pos3 - pos1, pos2 - pos1), double3(0.0, 0.0, 1.0));
        accumulateNormal(v1Base, normal2);
        accumulateNormal(v3Base, normal2);
        accumulateNormal(v2, normal2);
      }
    }

    for (int vertexIndex = 0; vertexIndex < totalVertices; ++vertexIndex)
    {
      int baseIndex = vertexIndex;
      if (vertexIndex % verticesPerRing == loopResolution) baseIndex -= loopResolution;
      if (normalCounts[baseIndex] > 0)
      {
        RibbonPoint3 averagedNormal = vertexNormals[baseIndex];
        averagedNormal /= static_cast<double>(normalCounts[baseIndex]);
        const double3 normalized = safeNormalize(averagedNormal.asDouble3(), double3(0.0, 0.0, 1.0));
        meshVertices[vertexIndex].normal = float4(static_cast<float>(normalized.x),
                                                  static_cast<float>(normalized.y),
                                                  static_cast<float>(normalized.z),
                                                  0.0f);
      }
    }

    IndexedRibbonGeometry geometry;
    geometry.vertices = std::move(meshVertices);
    geometry.indices.reserve(static_cast<size_t>((splineSamples - 1) * loopResolution * 6));
    for (int ringIndex = 0; ringIndex < splineSamples - 1; ++ringIndex)
    {
      for (int segment = 0; segment < loopResolution; ++segment)
      {
        const uint32_t v0 = static_cast<uint32_t>(ringIndex * verticesPerRing + segment);
        const uint32_t v1 = static_cast<uint32_t>(ringIndex * verticesPerRing + segment + 1);
        const uint32_t v2 = static_cast<uint32_t>((ringIndex + 1) * verticesPerRing + segment);
        const uint32_t v3 = static_cast<uint32_t>((ringIndex + 1) * verticesPerRing + segment + 1);
        geometry.indices.push_back(v0);
        geometry.indices.push_back(v1);
        geometry.indices.push_back(v2);
        geometry.indices.push_back(v1);
        geometry.indices.push_back(v3);
        geometry.indices.push_back(v2);
      }
    }
    return geometry;
  }
}

RKRibbonMesh ProteinRibbonMeshBuilder::build(const ProteinBackbone &backbone,
                                             double radius,
                                             double3 contentShift,
                                             ProteinRibbonMeshParameters parameters,
                                             ProteinRibbonSecondaryStructureMethod secondaryStructureMethod)
{
  const ProteinRibbonMeshParameters meshParameters = parameters.clamped();
  const int ringResolution = meshParameters.crossSectionRingResolution;
  constexpr double chainBreakAlphaCarbonDistance = 4.5;

  auto tracedResidues = [](const ProteinBackboneChain &chain) -> ProteinBackboneChain
  {
    std::vector<ProteinBackboneResidue> residues;
    residues.reserve(chain.residues.size());
    for (const ProteinBackboneResidue &residue : chain.residues)
    {
      if (residue.alphaCarbon) residues.push_back(residue);
    }
    return ProteinBackboneChain(chain.chainIdentifier, std::move(residues));
  };

  auto continuousResidueRanges = [&](const ProteinBackboneChain &traced) -> std::vector<std::pair<int, int>>
  {
    std::vector<std::pair<int, int>> ranges;
    if (traced.residues.empty()) return ranges;
    int start = 0;
    for (size_t index = 1; index < traced.residues.size(); ++index)
    {
      const auto &previous = traced.residues[index - 1].alphaCarbon;
      const auto &current = traced.residues[index].alphaCarbon;
      if (!previous || !current) continue;
      if ((current->position() - previous->position()).length() > chainBreakAlphaCarbonDistance)
      {
        ranges.emplace_back(start, static_cast<int>(index));
        start = static_cast<int>(index);
      }
    }
    ranges.emplace_back(start, static_cast<int>(traced.residues.size()));
    return ranges;
  };

  struct FragmentStationData
  {
    ProteinBackboneChain chain;
    std::vector<RibbonStation> stations;
    std::vector<ProteinRibbonResidueSegment> residueSegments;
    int chainSlot = 0;
  };

  std::vector<FragmentStationData> fragments;
  std::vector<int> chainRingTotals;

  for (const ProteinBackboneChain &chain : backbone.chains)
  {
    const ProteinBackboneChain traced = tracedResidues(chain);
    const std::vector<ProteinRibbonSecondaryStructure> secondaryStructure =
      ProteinRibbonSecondaryStructureAssigner::assign(traced, contentShift, secondaryStructureMethod);

    std::vector<FragmentStationData> chainFragments;
    int chainRings = 0;
    for (const std::pair<int, int> &range : continuousResidueRanges(traced))
    {
      const int first = range.first;
      const int last = std::min(range.second, static_cast<int>(secondaryStructure.size()));
      if (last <= first) continue;

      std::vector<ProteinBackboneResidue> fragmentResidues(traced.residues.begin() + first, traced.residues.begin() + last);
      const ProteinBackboneChain fragmentChain(traced.chainIdentifier, std::move(fragmentResidues));
      const std::vector<ProteinRibbonSecondaryStructure> fragmentStructure(secondaryStructure.begin() + first,
                                                                            secondaryStructure.begin() + last);
      const std::vector<ProteinRibbonResidueSegment> residueSegments =
        ProteinRibbonSegmentSupport::residueSegments(fragmentStructure, fragmentChain.chainIdentifier);
      const std::vector<RibbonStation> stations =
        ribbonStations(fragmentChain, contentShift, fragmentStructure, meshParameters);
      if (stations.size() < 2) continue;

      chainRings += static_cast<int>(stations.size());
      chainFragments.push_back({fragmentChain, stations, residueSegments, static_cast<int>(chainRingTotals.size())});
    }

    if (chainFragments.empty()) continue;
    chainRingTotals.push_back(chainRings);
    fragments.insert(fragments.end(),
                     std::make_move_iterator(chainFragments.begin()),
                     std::make_move_iterator(chainFragments.end()));
  }

  RKRibbonMesh mesh;
  const int totalChains = std::max(static_cast<int>(chainRingTotals.size()), 1);
  int currentSlot = 0;
  bool haveSlot = false;
  int slotIndexStart = 0;
  int ringOffset = 0;

  for (const FragmentStationData &fragment : fragments)
  {
    if (!haveSlot || fragment.chainSlot != currentSlot)
    {
      if (haveSlot)
      {
        mesh.chainDrawRanges.emplace_back(slotIndexStart, static_cast<int>(mesh.indices.size()) - slotIndexStart);
        mesh.chainSplineSampleCounts.push_back(chainRingTotals[static_cast<size_t>(currentSlot)]);
      }
      currentSlot = fragment.chainSlot;
      haveSlot = true;
      slotIndexStart = static_cast<int>(mesh.indices.size());
      ringOffset = 0;
    }

    const int indexStart = static_cast<int>(mesh.indices.size());
    appendIndexedGeometry(mesh, ringMesh(fragment.stations,
                                         radius,
                                         fragment.chainSlot,
                                         totalChains,
                                         ringResolution,
                                         meshParameters,
                                         ringOffset,
                                         chainRingTotals[static_cast<size_t>(fragment.chainSlot)]));
    appendSegmentDrawRanges(fragment.chain, fragment.stations, indexStart, fragment.residueSegments, ringResolution, mesh);
    appendResidueDrawRanges(fragment.chain, fragment.stations, indexStart, ringResolution, mesh);
    ringOffset += static_cast<int>(fragment.stations.size());
  }
  if (haveSlot)
  {
    mesh.chainDrawRanges.emplace_back(slotIndexStart, static_cast<int>(mesh.indices.size()) - slotIndexStart);
    mesh.chainSplineSampleCounts.push_back(chainRingTotals[static_cast<size_t>(currentSlot)]);
  }

  applyPickingIndices(mesh);
  applyRibbonStripeCoordinates(mesh);
  return mesh;
}

RKRibbonMesh ProteinRibbonMeshBuilder::buildNucleicAcidRibbon(const DNABackbone &backbone,
                                                              const std::vector<std::shared_ptr<SKAsymmetricAtom>> &atoms,
                                                              double radius,
                                                              double3 contentShift,
                                                              ProteinRibbonMeshParameters parameters)
{
  ProteinRibbonMeshParameters meshParameters = parameters.clamped();
  meshParameters.nucleicAcidRendering = true;
  const int ringResolution = meshParameters.crossSectionRingResolution;
  RKRibbonMesh mesh;

  struct ChainStationData
  {
    ProteinBackboneChain chain;
    std::vector<RibbonStation> stations;
    std::vector<ProteinRibbonResidueSegment> residueSegments;
  };

  const ProteinBackbone proteinBackbone = backbone.toProteinBackbone();
  std::vector<ChainStationData> chainStations;
  chainStations.reserve(backbone.chains.size());

  for (size_t chainIndex = 0; chainIndex < backbone.chains.size(); ++chainIndex)
  {
    const DNABackboneChain &dnaChain = backbone.chains[chainIndex];
    const std::vector<NucleicAcidBackboneSample> samples =
      nucleicAcidBackboneSamples(dnaChain, contentShift, meshParameters.nucleicAcidTraceMode);
    if (samples.size() < 2) continue;

    ProteinBackboneChain proteinChain;
    if (chainIndex < proteinBackbone.chains.size())
    {
      proteinChain = proteinBackbone.chains[chainIndex];
    }
    else
    {
      proteinChain.chainIdentifier = dnaChain.chainIdentifier;
    }

    // PyMOL tags nucleic acids as ss_t::NUCLEIC (not HELIX) so refine_normals may flip orientations.
    std::vector<ProteinRibbonSecondaryStructure> secondaryStructure(samples.size(), ProteinRibbonSecondaryStructure::coil);
    const std::vector<ProteinRibbonResidueSegment> residueSegments =
      ProteinRibbonSegmentSupport::residueSegments(secondaryStructure, dnaChain.chainIdentifier);
    const std::vector<RibbonStation> stations =
      ribbonStationsFromNucleicAcid(samples, secondaryStructure, meshParameters);
    if (stations.size() >= 2)
    {
      chainStations.push_back({proteinChain, stations, residueSegments});
    }
  }

  const int totalChains = std::max(static_cast<int>(chainStations.size()), 1);
  for (size_t chainIndex = 0; chainIndex < chainStations.size(); ++chainIndex)
  {
    const ChainStationData &chainData = chainStations[chainIndex];
    const int indexStart = static_cast<int>(mesh.indices.size());
    appendIndexedGeometry(mesh, ringMesh(chainData.stations,
                                         radius,
                                         static_cast<int>(chainIndex),
                                         totalChains,
                                         ringResolution,
                                         meshParameters));
    mesh.chainDrawRanges.emplace_back(indexStart, static_cast<int>(mesh.indices.size()) - indexStart);
    mesh.chainSplineSampleCounts.push_back(static_cast<int>(chainData.stations.size()));
    appendSegmentDrawRanges(chainData.chain, chainData.stations, indexStart, chainData.residueSegments, ringResolution, mesh);
    appendResidueDrawRanges(chainData.chain, chainData.stations, indexStart, ringResolution, mesh);
  }

  DNANucleotideGeometry nucleotideGeometry = DNANucleotideGeometry::build(atoms);
  DNANucleotideGeometry::assignGlobalResidueIndicesFromBackbone(nucleotideGeometry, backbone);
  const std::vector<DNANucleotideBasePair> basePairs = DNANucleotideGeometry::detectWatsonCrickPairs(nucleotideGeometry);
  ProteinNucleicAcidMeshBuilder::appendRingAndLadderMeshes(mesh,
                                                         nucleotideGeometry,
                                                         basePairs,
                                                         contentShift,
                                                         radius,
                                                         meshParameters);

  applyPickingIndices(mesh);
  applyRibbonStripeCoordinates(mesh);
  return mesh;
}

RKRibbonMesh ProteinRibbonMeshBuilder::buildHelixRibbon(const ProteinBackbone &backbone,
                                                        double radius,
                                                        double3 contentShift,
                                                        ProteinRibbonMeshParameters parameters)
{
  const ProteinRibbonMeshParameters meshParameters = parameters.clamped();
  const int ringResolution = meshParameters.crossSectionRingResolution;
  RKRibbonMesh mesh;

  struct ChainStationData
  {
    ProteinBackboneChain chain;
    std::vector<RibbonStation> stations;
    std::vector<ProteinRibbonResidueSegment> residueSegments;
  };

  std::vector<ChainStationData> chainStations;
  chainStations.reserve(backbone.chains.size());
  for (const ProteinBackboneChain &chain : backbone.chains)
  {
    std::vector<ProteinBackboneResidue> residuesWithCenter;
    for (const ProteinBackboneResidue &residue : chain.residues)
    {
      if (residue.alphaCarbon) residuesWithCenter.push_back(residue);
    }
    std::vector<ProteinRibbonSecondaryStructure> secondaryStructure(residuesWithCenter.size(),
                                                                    ProteinRibbonSecondaryStructure::helix);
    const std::vector<ProteinRibbonResidueSegment> residueSegments =
      ProteinRibbonSegmentSupport::residueSegments(secondaryStructure, chain.chainIdentifier);
    const std::vector<RibbonStation> stations = ribbonStations(chain, contentShift, secondaryStructure, meshParameters);
    if (stations.size() >= 2)
    {
      chainStations.push_back({chain, stations, residueSegments});
    }
  }

  const int totalChains = std::max(static_cast<int>(chainStations.size()), 1);
  for (size_t chainIndex = 0; chainIndex < chainStations.size(); ++chainIndex)
  {
    const ChainStationData &chainData = chainStations[chainIndex];
    const int indexStart = static_cast<int>(mesh.indices.size());
    appendIndexedGeometry(mesh, ringMesh(chainData.stations,
                                         radius,
                                         static_cast<int>(chainIndex),
                                         totalChains,
                                         ringResolution,
                                         meshParameters));
    mesh.chainDrawRanges.emplace_back(indexStart, static_cast<int>(mesh.indices.size()) - indexStart);
    mesh.chainSplineSampleCounts.push_back(static_cast<int>(chainData.stations.size()));
    appendSegmentDrawRanges(chainData.chain, chainData.stations, indexStart, chainData.residueSegments, ringResolution, mesh);
    appendResidueDrawRanges(chainData.chain, chainData.stations, indexStart, ringResolution, mesh);
  }

  applyPickingIndices(mesh);
  applyRibbonStripeCoordinates(mesh);
  return mesh;
}
