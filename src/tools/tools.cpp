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
#include <random>
#include <sstream>
#include <iomanip>

namespace pdtools {
    std::string ltrim(const std::string& s) {
        std::string result(s);
        result.erase(0, s.find_first_not_of(" \t\n\r"));
        return result;
    }

    std::string rtrim(const std::string& s) {
        std::string result(s);
        result.erase(s.find_last_not_of(" \t\n\r") + 1);
        return result;
    }

    std::string trim(const std::string& s) {
        std::string result(s);
        ltrim(result);
        rtrim(result);
        return result;
    }

    StringVector split(const std::string &s, char delim) {
        StringVector result;
        std::stringstream ss(s);

        std::string part;
        while (std::getline(ss, part, delim)) {
            result.push_back(trim(part));
        }

        return result;
    }

    std::string stringVectorToString(const StringVector &v) {
        std::stringstream ss;

        std::for_each(v.begin(), v.end(),
                      [&](const auto &item) { ss << item << ","; });

        return ss.str();
    }

    std::string intVectorToString(const IntVector &v) {
        std::stringstream ss;

        std::for_each(v.begin(), v.end(),
                      [&](const auto &item) { ss << item << ","; });

        return ss.str();
    }

    std::string locationToString(const Location &l) {
        std::stringstream oss;

        oss << "(x: " << l.x() << ", y: " << l.y() << ")";

        return oss.str();
    }

    // Function to generate a random integer within a specified range [min, max]
    int generate_random_int_in_range(int min, int max) {
        std::random_device rd;
        std::mt19937 gen(rd()); // Mersenne Twister random number generator
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }

    bool double_nearly_eq(double a, double b, double epsilon) {
        return std::fabs(a - b) <= epsilon;
    }

    bool is_number(const std::string& s) {
        if (s.empty()) return false;

        double value{};
        auto [ptr, ec] = std::from_chars(s.data(),
                                         s.data() + s.size(),
                                         value);

        return ec == std::errc{} && ptr == s.data() + s.size();
    }

    std::string generate_uuid() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);

        std::ostringstream oss;
        oss << std::hex << std::setfill('0')
            << std::setw(8) << dis(gen) << "-"
            << std::setw(4) << (dis(gen) & 0xFFFF) << "-"
            << std::setw(4) << ((dis(gen) & 0x0FFF) | 0x4000) << "-"
            << std::setw(4) << ((dis(gen) & 0x3FFF) | 0x8000) << "-"
            << std::setw(8) << dis(gen)
            << std::setw(4) << (dis(gen) & 0xFFFF);
        return oss.str();
    }
}; // namespace pdtools
