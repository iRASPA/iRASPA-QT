/********************************************************************************************************************
    iRASPA: GPU-accelated visualisation software for materials scientists
    Copyright (c) 2016-2021 David Dubbeldam, Sofia Calero, Thijs J.H. Vlugt.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
 ********************************************************************************************************************/

#pragma once

#include <vector>
#include <cstdint>

struct RKHalfFloat
{
  static float floatFromHalfBits(uint16_t half);
  static uint16_t halfBitsFromFloat(float value);
};

struct RibbonAOTexturePostProcess
{
  static void dilateAndSmooth(std::vector<float> &data, int width, int height, int smoothPasses = 2);
  static void gaussianBlur(std::vector<float> &data, int width, int height);

private:
  static void dilate(std::vector<float> &data, int width, int height);
  static void smooth(const std::vector<float> &source, std::vector<float> &data, int width, int height);
  static void smoothAlongStrand(const std::vector<float> &source, std::vector<float> &data, int width, int height, int radius);
};
