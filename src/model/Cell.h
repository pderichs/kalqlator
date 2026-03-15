// KalQlator - Cell.h
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

#include <utility>
#include <vector>

#include "../lisp/object.h"
#include "../lisp/tokenizer/tokens.h"
#include "CellError.h"

struct Cell;

using CellVector = std::vector<Cell *>;

struct Cell {
  explicit Cell(const int row, const int column, std::string name)
      : row_(row), column_(column), name_(std::move(name)) {}

  int row_;

  int column_;

  /**
   * name serves as key to table env / current value
   */
  std::string name_;

  /**
   * what is actually shown
   */
  std::string visible_content_;

  /**
   * Raw lisp formula
   */
  std::string raw_formula_;

  /**
   * Raw entry by user
   */
  std::string raw_content_;

  /**
   * The main "program" behind the cell.
   * A list of lisp expressions.
   */
  lisp::LispObjectPtrVector formula_;

  /**
   * Tokens of the formula.
   */
  lisp::LispTokens tokens_;

  /**
   * If the cell has any issues, this vector contains detailed
   * information about each one of them.
   */
  CellErrorVector errors_;

  [[nodiscard]] bool contains_formula() const { return !raw_formula_.empty(); }

  [[nodiscard]] bool empty() const {
    return raw_content_.empty() && !contains_formula();
  }

  void add_error(const CellError &error) { errors_.push_back(error); }

  void clear_errors() { errors_.clear(); }

  [[nodiscard]] bool has_errors() const { return !errors_.empty(); }

  [[nodiscard]] std::optional<CellError> get_last_error() const {
    if (errors_.empty()) {
      return std::nullopt;
    }

    return errors_.back();
  }
};
