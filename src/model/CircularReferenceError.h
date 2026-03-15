// KalQlator - CircularReferenceError.h
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
#include <stdexcept>
#include <utility>

class CircularReferenceError : public std::runtime_error {
public:
  CircularReferenceError(const std::string &message, std::string from_cell,
                         std::string to_cell)
      : std::runtime_error(message), from_cell(std::move(from_cell)),
        to_cell(std::move(to_cell)) {}

  std::string from_cell;
  std::string to_cell;
};
