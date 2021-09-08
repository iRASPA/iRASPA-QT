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

#include <functional>
#include <limits>
#include <cstdint>

// https://stackoverflow.com/questions/35985960/c-why-is-boosthash-combine-the-best-way-to-combine-hash-values/50978188

inline void hash_combine([[maybe_unused]] std::size_t&  seed) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
}

/*
template<typename T>
T xorshift(const T& n,int i){
  return n^(n>>i);
}

static uint32_t distribute(const uint32_t& n){
  uint32_t p = 0x55555555ul; // pattern of alternating 0 and 1
  uint32_t c = 3423571495ul; // random uneven integer constant;
  return c*xorshift(p*xorshift(n,16),16);
}

static uint64_t hash(const uint64_t& n){
  uint64_t p = 0x5555555555555555;     // pattern of alternating 0 and 1
  uint64_t c = 17316035218449499591ull;// random uneven integer constant;
  return c*xorshift(p*xorshift(n,32),32);
}

// if c++20 rotl is not available:
template <typename T,typename S>
typename std::enable_if<std::is_unsigned<T>::value,T>::type
constexpr rotl(const T n, const S i){
  const T m = (std::numeric_limits<T>::digits-1);
  const T c = i&m;
  return (n<<c)|(n>>((T(0)-c)&m)); // this is usually recognized by the compiler to mean rotation, also c++20 now gives us rotl directly
}

template <class T>
inline size_t hash_combine(std::size_t& seed, const T& v)
{
  return rotl(seed,std::numeric_limits<size_t>::digits/3) ^ distribute(std::hash<T>{}(v));
}
*/



