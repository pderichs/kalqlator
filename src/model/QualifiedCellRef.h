// KalQlator - QualifiedCellRef.h
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

#include <set>
#include <string>
#include <tuple>
#include <vector>

struct QualifiedCellRef {
  std::string sheet_id;
  std::string cell_name;

  bool operator==(const QualifiedCellRef &other) const {
    return sheet_id == other.sheet_id && cell_name == other.cell_name;
  }

  bool operator<(const QualifiedCellRef &other) const {
    return std::tie(sheet_id, cell_name) <
           std::tie(other.sheet_id, other.cell_name);
  }
};

using QualifiedCellRefSet = std::set<QualifiedCellRef>;
using QualifiedCellRefVector = std::vector<QualifiedCellRef>;
