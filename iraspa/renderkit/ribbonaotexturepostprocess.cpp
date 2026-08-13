/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#include "ribbonaotexturepostprocess.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
  float floatFromBits(uint32_t bits)
  {
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
  }

  uint32_t bitsFromFloat(float value)
  {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
  }
}

float RKHalfFloat::floatFromHalfBits(uint16_t half)
{
  const uint32_t sign = static_cast<uint32_t>(half & 0x8000u) << 16;
  const uint32_t exponent = static_cast<uint32_t>((half >> 10) & 0x1Fu);
  const uint32_t mantissa = static_cast<uint32_t>(half & 0x03FFu);

  if (exponent == 0)
  {
    if (mantissa == 0) { return floatFromBits(sign); }
    uint32_t normalizedMantissa = mantissa;
    uint32_t shift = 0;
    while ((normalizedMantissa & 0x0400u) == 0)
    {
      normalizedMantissa <<= 1;
      shift += 1;
    }
    normalizedMantissa &= 0x03FFu;
    const uint32_t floatExponent = 127u - 15u - shift + 1u;
    return floatFromBits(sign | (floatExponent << 23) | (normalizedMantissa << 13));
  }

  if (exponent == 31)
  {
    if (mantissa == 0) { return floatFromBits(sign | 0x7F800000u); }
    return floatFromBits(sign | 0x7F800000u | (mantissa << 13));
  }

  const uint32_t floatExponent = exponent + 112u;
  return floatFromBits(sign | (floatExponent << 23) | (mantissa << 13));
}

uint16_t RKHalfFloat::halfBitsFromFloat(float value)
{
  const uint32_t bits = bitsFromFloat(value);
  uint16_t half = static_cast<uint16_t>((bits >> 16) & 0x8000u);
  const int exponent = static_cast<int>((bits >> 23) & 0xFFu);
  uint32_t mantissa = bits & 0x007FFFFFu;

  if (exponent == 255)
  {
    if (mantissa != 0) { return static_cast<uint16_t>(half | 0x7E00u); }
    return static_cast<uint16_t>(half | 0x7C00u);
  }

  int halfExponent = exponent - 127 + 15;
  if (halfExponent >= 31) { return static_cast<uint16_t>(half | 0x7C00u); }
  if (halfExponent <= 0)
  {
    if (halfExponent < -10) { return half; }
    mantissa |= 0x00800000u;
    const int shift = 1 - halfExponent;
    mantissa >>= static_cast<uint32_t>(shift);
    return static_cast<uint16_t>(half | static_cast<uint16_t>(mantissa >> 13));
  }

  return static_cast<uint16_t>(half | static_cast<uint16_t>(halfExponent << 10) | static_cast<uint16_t>(mantissa >> 13));
}

namespace
{
  float sampleClamped(const std::vector<float> &source, int width, int height, float x, float y)
  {
    const int clampedX = std::clamp(static_cast<int>(std::lround(x)), 0, width - 1);
    const int clampedY = std::clamp(static_cast<int>(std::lround(y)), 0, height - 1);
    return source[static_cast<size_t>(clampedY * width + clampedX)];
  }
}

void RibbonAOTexturePostProcess::gaussianBlur(std::vector<float> &data, int width, int height)
{
  if (width <= 0 || height <= 0 || static_cast<int>(data.size()) < width * height) { return; }

  static const float weights[8] = {
    0.159576912161f, 0.147308056121f, 0.115876621105f, 0.0776744219933f,
    0.0443683338718f, 0.0215963866053f, 0.00895781211794f, 0.0044299121055113265f
  };
  static const float horizontalSteps[7] = {8.0f, 16.0f, 24.0f, 32.0f, 40.0f, 48.0f, 56.0f};
  static const float verticalSteps[7] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f};

  std::vector<float> scratch(static_cast<size_t>(width * height), 0.0f);

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      float sum = sampleClamped(data, width, height, static_cast<float>(x), static_cast<float>(y)) * weights[0];
      for (int i = 0; i < 7; ++i)
      {
        const float offset = horizontalSteps[i];
        sum += sampleClamped(data, width, height, static_cast<float>(x) - offset, static_cast<float>(y)) * weights[i + 1];
        sum += sampleClamped(data, width, height, static_cast<float>(x) + offset, static_cast<float>(y)) * weights[i + 1];
      }
      scratch[static_cast<size_t>(y * width + x)] = sum;
    }
  }

  data = std::move(scratch);
  scratch.assign(static_cast<size_t>(width * height), 0.0f);

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      float sum = sampleClamped(data, width, height, static_cast<float>(x), static_cast<float>(y)) * weights[0];
      for (int i = 0; i < 7; ++i)
      {
        const float offset = verticalSteps[i];
        sum += sampleClamped(data, width, height, static_cast<float>(x), static_cast<float>(y) - offset) * weights[i + 1];
        sum += sampleClamped(data, width, height, static_cast<float>(x), static_cast<float>(y) + offset) * weights[i + 1];
      }
      scratch[static_cast<size_t>(y * width + x)] = sum;
    }
  }

  data = std::move(scratch);
}

void RibbonAOTexturePostProcess::dilateAndSmooth(std::vector<float> &data, int width, int height, int smoothPasses)
{
  if (width <= 0 || height <= 0 || static_cast<int>(data.size()) < width * height) { return; }

  std::vector<float> dilated = data;
  for (int pass = 0; pass < 2; ++pass)
  {
    dilate(dilated, width, height);
  }

  std::vector<float> current = dilated;
  std::vector<float> scratch(static_cast<size_t>(width * height), 0.0f);
  for (int pass = 0; pass < std::max(smoothPasses, 1); ++pass)
  {
    smooth(current, scratch, width, height);
    std::swap(current, scratch);
  }
  smoothAlongStrand(current, scratch, width, height, 4);
  std::swap(current, scratch);
  data = std::move(current);
}

void RibbonAOTexturePostProcess::dilate(std::vector<float> &data, int width, int height)
{
  const std::vector<float> source = data;
  const int offsets[4][2] = {{-1, 0}, {0, 1}, {1, 0}, {0, -1}};

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const int index = y * width + x;
      if (source[static_cast<size_t>(index)] > 0.0f) { continue; }

      float sum = 0.0f;
      int count = 0;
      for (const auto &offset : offsets)
      {
        const int neighborX = x + offset[0];
        const int neighborY = y + offset[1];
        if (neighborX < 0 || neighborX >= width || neighborY < 0 || neighborY >= height) { continue; }
        const float neighborValue = source[static_cast<size_t>(neighborY * width + neighborX)];
        if (neighborValue > 0.0f)
        {
          sum += neighborValue;
          count += 1;
        }
      }
      if (count > 0) { data[static_cast<size_t>(index)] = sum / static_cast<float>(count); }
    }
  }
}

void RibbonAOTexturePostProcess::smooth(const std::vector<float> &source, std::vector<float> &data, int width, int height)
{
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const int centerIndex = y * width + x;
      const float centerValue = source[static_cast<size_t>(centerIndex)];
      if (centerValue <= 0.0f)
      {
        data[static_cast<size_t>(centerIndex)] = 0.0f;
        continue;
      }

      float sum = centerValue * 2.0f;
      float weight = 2.0f;
      for (int dy = -1; dy <= 1; ++dy)
      {
        for (int dx = -1; dx <= 1; ++dx)
        {
          if (dx == 0 && dy == 0) { continue; }
          const int neighborX = x + dx;
          const int neighborY = y + dy;
          if (neighborX < 0 || neighborX >= width || neighborY < 0 || neighborY >= height) { continue; }
          const float neighborValue = source[static_cast<size_t>(neighborY * width + neighborX)];
          if (neighborValue > 0.0f)
          {
            sum += neighborValue;
            weight += 1.0f;
          }
        }
      }
      data[static_cast<size_t>(centerIndex)] = sum / weight;
    }
  }
}

void RibbonAOTexturePostProcess::smoothAlongStrand(const std::vector<float> &source, std::vector<float> &data, int width, int height, int radius)
{
  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      const int centerIndex = y * width + x;
      const float centerValue = source[static_cast<size_t>(centerIndex)];
      if (centerValue <= 0.0f)
      {
        data[static_cast<size_t>(centerIndex)] = 0.0f;
        continue;
      }

      float sum = centerValue * 2.0f;
      float weight = 2.0f;
      for (int dx = -radius; dx <= radius; ++dx)
      {
        if (dx == 0) { continue; }
        const int neighborX = x + dx;
        if (neighborX < 0 || neighborX >= width) { continue; }
        const float neighborValue = source[static_cast<size_t>(y * width + neighborX)];
        if (neighborValue > 0.0f)
        {
          sum += neighborValue;
          weight += 1.0f;
        }
      }
      data[static_cast<size_t>(centerIndex)] = sum / weight;
    }
  }
}
