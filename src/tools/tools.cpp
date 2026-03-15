// KalQlator - tools.cpp
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

#include "tools.h"
#include <algorithm>
#include <charconv>
#include <iomanip>
#include <random>
#include <sstream>

namespace pdtools {
static constexpr uint32_t UUID_MASK_32BIT = 0xFFFFFFFF;
static constexpr uint32_t UUID_MASK_16BIT = 0x0000FFFF;
static constexpr uint32_t UUID_MASK_12BIT = 0x00000FFF;
static constexpr uint32_t UUID_MASK_14BIT = 0x00003FFF;
static constexpr uint32_t UUID_VERSION4_BITS = 0x00004000;
static constexpr uint32_t UUID_VARIANT_BITS = 0x00008000;

static constexpr int UUID_WIDTH_WORD = 8;  // 32-Bit-Segment
static constexpr int UUID_WIDTH_SHORT = 4; // 16-Bit-Segment

std::string ltrim(const std::string &string) {
  std::string result(string);
  result.erase(0, string.find_first_not_of(" \t\n\r"));
  return result;
}

std::string rtrim(const std::string &string) {
  std::string result(string);
  result.erase(string.find_last_not_of(" \t\n\r") + 1);
  return result;
}

std::string trim(const std::string &string) {
  std::string result(string);
  ltrim(result);
  rtrim(result);
  return result;
}

StringVector split(const std::string &string, char delimiter) {
  StringVector result;
  std::stringstream split_string(string);

  std::string part;
  while (std::getline(split_string, part, delimiter)) {
    result.push_back(trim(part));
  }

  return result;
}

std::string stringVectorToString(const StringVector &strings) {
  std::stringstream result;

  std::ranges::for_each(strings,
                        [&](const auto &item) { result << item << ","; });

  return result.str();
}

std::string intVectorToString(const IntVector &int_vector) {
  std::stringstream result;

  std::ranges::for_each(int_vector,
                        [&](const auto &item) { result << item << ","; });

  return result.str();
}

std::string locationToString(const Location &location) {
  std::stringstream oss;

  oss << "(x: " << location.x() << ", y: " << location.y() << ")";

  return oss.str();
}

// Function to generate a random integer within a specified range [min, max]
int generate_random_int_in_range(const int min, const int max) {
  std::random_device random_device;
  std::mt19937 gen(random_device()); // Mersenne Twister random number generator
  std::uniform_int_distribution<int> dist(min, max);
  return dist(gen);
}

bool double_nearly_eq(double value, double other, double epsilon) {
  return std::fabs(value - other) <= epsilon;
}

bool is_number(const std::string &string_to_check) {
  if (string_to_check.empty()) {
    return false;
  }

  double value{};
  auto [ptr, ec] =
      std::from_chars(string_to_check.data(),
                      string_to_check.data() + string_to_check.size(), value);

  return ec == std::errc{} &&
         ptr == string_to_check.data() + string_to_check.size();
}

std::string generate_uuid() {
  std::random_device random_device;
  std::mt19937 gen(random_device());

  std::uniform_int_distribution<uint32_t> dis(0, UUID_MASK_32BIT);
  std::ostringstream result;

  result << std::hex << std::setfill('0') << std::setw(UUID_WIDTH_WORD)
         << dis(gen) << "-" << std::setw(UUID_WIDTH_SHORT)
         << (dis(gen) & UUID_MASK_16BIT) << "-" << std::setw(UUID_WIDTH_SHORT)
         << ((dis(gen) & UUID_MASK_12BIT) | UUID_VERSION4_BITS) << "-"
         << std::setw(UUID_WIDTH_SHORT)
         << ((dis(gen) & UUID_MASK_14BIT) | UUID_VARIANT_BITS) << "-"
         << std::setw(UUID_WIDTH_WORD) << dis(gen)
         << std::setw(UUID_WIDTH_SHORT) << (dis(gen) & UUID_MASK_16BIT);

  return result.str();
}
}; // namespace pdtools
