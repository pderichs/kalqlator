// KalQlator - CellDisplayFormatter.h
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

#include <string>

#include "../lisp/types.h"
#include "CellFormat.h"

struct ParsedFormatSpecifier {
  std::string prefix;
  int decimal_places = 0;
  std::string postfix;
  bool valid = false;
};

class CellDisplayFormatter {
public:
  static ParsedFormatSpecifier parse_specifier(const std::string &specifier);
  static bool is_valid_specifier(const std::string &specifier);
  static std::string format(const std::string &value, const CellFormat &format);
  static std::string format_number(const lisp::LispObjectPtr &evaluated_value,
                                   const CellFormat &format);
};
