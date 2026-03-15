// KalQlator - tools.h
// Copyright (C) 2026  pderichs
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "location.h"
#include <deque>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace pdtools {
using StringVector = std::vector<std::string>;
using StringSet = std::set<std::string>;
using StringDeque = std::deque<std::string>;

using IntVector = std::vector<int>;
using BoolVector = std::vector<bool>;
using UnsignedCharVector = std::vector<unsigned char>;

std::string ltrim(const std::string &string);
std::string rtrim(const std::string &string);
std::string trim(const std::string &string);

std::vector<std::string> split(const std::string &string, char delimiter);

std::string stringVectorToString(const StringVector &strings);

std::string intVectorToString(const IntVector &int_vector);

std::string locationToString(const Location &location);

int generate_random_int_in_range(int min, int max);

template <typename T>
std::optional<T> convert_string_to_number(const std::string &str) {
  std::istringstream iss(str);
  T value;
  char leftover;
  bool is_valid = (iss >> value) && !(iss >> leftover);
  if (!is_valid) {
    return std::nullopt;
  }

  return value;
}

bool double_nearly_eq(double value, double other, double epsilon);

bool is_number(const std::string &string_to_check);

std::string generate_uuid();
}; // namespace pdtools

// Thanks to
// https://github.com/osalbahr/adventOfCode/blob/main/problems/day17/day17_2.cpp
#define REPORT(X) std::cout << #X << " = " << (X) << std::endl

#define MARK(msg) std::cout << "*** MARK (" << msg << ")" << std::endl
