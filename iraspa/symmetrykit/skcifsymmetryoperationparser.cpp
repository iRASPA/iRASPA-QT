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

#include "skcifsymmetryoperationparser.h"
#include <cmath>

std::optional<SKSeitzIntegerMatrix> SKCIFSymmetryOperationParser::parse(const QString &xyz)
{
  QString trimmed = xyz.trimmed();
  while (!trimmed.isEmpty() && (trimmed.front() == '\'' || trimmed.front() == '"' || trimmed.front().isSpace()))
  {
    trimmed.remove(0, 1);
  }
  while (!trimmed.isEmpty() && (trimmed.back() == '\'' || trimmed.back() == '"' || trimmed.back().isSpace()))
  {
    trimmed.chop(1);
  }

  #if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
  QStringList components = trimmed.split(',', QString::KeepEmptyParts);
  #else
  QStringList components = trimmed.split(',', Qt::KeepEmptyParts);
  #endif

  if (components.size() != 3)
  {
    return std::nullopt;
  }

  int3 column0(0, 0, 0);
  int3 column1(0, 0, 0);
  int3 column2(0, 0, 0);
  double3 translation(0.0, 0.0, 0.0);

  for (int outputIndex = 0; outputIndex < 3; ++outputIndex)
  {
    std::optional<std::pair<int3, double>> parsed = parseComponent(components[outputIndex]);
    if (!parsed)
    {
      return std::nullopt;
    }
    const int3 &coefficients = parsed->first;
    double componentTranslation = parsed->second;
    column0[outputIndex] = coefficients.x;
    column1[outputIndex] = coefficients.y;
    column2[outputIndex] = coefficients.z;
    translation[outputIndex] = componentTranslation;
  }

  SKRotationMatrix rotation(column0, column1, column2);
  return SKSeitzIntegerMatrix(rotation, double3::fract(translation));
}

std::optional<std::vector<SKSeitzIntegerMatrix>> SKCIFSymmetryOperationParser::parseOperations(const std::vector<QString> &xyzStrings)
{
  std::vector<SKSeitzIntegerMatrix> operations;
  operations.reserve(xyzStrings.size());
  for (const QString &xyz : xyzStrings)
  {
    std::optional<SKSeitzIntegerMatrix> op = parse(xyz);
    if (!op)
    {
      return std::nullopt;
    }
    operations.push_back(*op);
  }
  return operations;
}

std::optional<std::pair<int3, double>> SKCIFSymmetryOperationParser::parseComponent(const QString &rawInput)
{
  QString input;
  input.reserve(rawInput.size());
  for (QChar ch : rawInput)
  {
    if (!ch.isSpace())
    {
      input.append(ch);
    }
  }

  if (input.isEmpty())
  {
    return std::nullopt;
  }

  int3 coefficients(0, 0, 0);
  double translation = 0.0;

  int index = 0;
  int end = input.size();

  while (index < end)
  {
    double sign = 1.0;

    if (input[index] == '+')
    {
      ++index;
    }
    else if (input[index] == '-')
    {
      sign = -1.0;
      ++index;
    }

    bool hasNumber = false;
    double numerator = 0.0;
    double denominator = 1.0;

    while (index < end && input[index].isDigit())
    {
      hasNumber = true;
      numerator = numerator * 10.0 + double(input[index].digitValue());
      ++index;
    }

    if (index < end && input[index] == '.')
    {
      ++index;
      double fractionFactor = 0.1;
      bool hasFractionDigits = false;
      while (index < end && input[index].isDigit())
      {
        hasNumber = true;
        hasFractionDigits = true;
        numerator += double(input[index].digitValue()) * fractionFactor;
        fractionFactor *= 0.1;
        ++index;
      }
      if (!hasFractionDigits)
      {
        return std::nullopt;
      }
    }
    else if (index < end && input[index] == '/')
    {
      ++index;
      denominator = 0.0;
      while (index < end && input[index].isDigit())
      {
        denominator = denominator * 10.0 + double(input[index].digitValue());
        ++index;
      }
      if (denominator == 0.0)
      {
        return std::nullopt;
      }
    }

    double value = sign * (hasNumber ? numerator / denominator : 1.0);

    if (index < end)
    {
      QChar axis = input[index].toLower();
      if (axis == 'x' || axis == 'y' || axis == 'z')
      {
        if (std::fabs(value) != 1.0)
        {
          return std::nullopt;
        }

        if (axis == 'x')
        {
          coefficients.x += int32_t(value);
        }
        else if (axis == 'y')
        {
          coefficients.y += int32_t(value);
        }
        else if (axis == 'z')
        {
          coefficients.z += int32_t(value);
        }
        ++index;
      }
      else if (hasNumber)
      {
        translation += value;
      }
      else
      {
        return std::nullopt;
      }
    }
    else if (hasNumber)
    {
      translation += value;
    }
    else
    {
      return std::nullopt;
    }
  }

  return std::make_pair(coefficients, translation);
}
