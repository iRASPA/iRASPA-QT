#pragma once

#include <array>
#include "rkrenderuniforms.h"

namespace VolumeTransferFunctions
{
constexpr int kTableCount = 23;
constexpr int kTableSize = 256;
extern const std::array<std::array<float4, kTableSize>, kTableCount> kTables;
}
