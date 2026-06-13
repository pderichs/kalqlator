// KalQlator - TableLispEnvironment.cpp
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

#include "TableLispEnvironment.h"

#include <algorithm>
#include <ranges>
#include <regex>
#include <unordered_set>
#include <utility>

#include "../lisp/factories.h"
#include "../lisp/tools.h"
#include "../messagebus/event_dispatcher.h"
#include "../tools/FlagScope.h"
#include "../ui/user_interface_tools.h"
#include "CircularReferenceError.h"
#include "TableContext.h"
#include "events/TableEnvironmentUpdateEvent.h"
#include "functions/Cell.h"
#include "functions/TableCell.h"

void TableLispEnvironment::initialize() {
  FlagScope initializing_scope(&initializing_);

  // Define all cells as environment vars
  for (const char col : std::views::iota('A', 'Z' + 1)) {
    for (const int row : std::views::iota(1, 101)) {
      std::string cellId = std::string(1, col) + std::to_string(row);
      define(cellId, lisp::make_nil());
    }
  }

  // Cell function
  define("cell", lisp::make_native_fn(FnCell));

  // Cross table cell reference
  define("table_cell", lisp::make_native_fn(FnTableCell));
}

void TableLispEnvironment::dfs(const std::string &cell,
                               pdtools::StringSet &visited,
                               pdtools::StringVector &result) {
  if (visited.contains(cell)) {
    return;
  }

  visited.insert(cell);

  for (const auto &ref : referenced_by_[cell]) {
    dfs(ref, visited, result);
  }

  result.push_back(cell); // Post order
}

pdtools::StringVector
TableLispEnvironment::dependency_chain_in_topological_order(
    const std::string &cell) {
  pdtools::StringVector result;
  pdtools::StringSet visited;

  for (const auto &ref : referenced_by_[cell]) {
    dfs(ref, visited, result);
  }

  std::ranges::reverse(result);

  return result;
}

void TableLispEnvironment::signal_environment_update(
    const std::string &name, lisp::LispObjectPtr value) {
  if (initializing_) {
    return;
  }

  const pdtools::StringVector dependencies =
      dependency_chain_in_topological_order(name);

  EventDispatcher::dispatch(TableEnvironmentUpdateEvent{
      .name = name,
      .value = std::move(value),
      .dependencies_in_topological_order = dependencies});
}

bool TableLispEnvironment::is_cell_name(const std::string &name) const {
  if (name.length() >= 2) {
    static const std::regex pattern(R"(([A-Z]+)(\d+))");
    std::smatch match;

    if (std::regex_match(name, match, pattern)) {
      return true;
    }
  }

  return false;
}

void TableLispEnvironment::define(const std::string &name,
                                  lisp::LispObjectPtr value) {
  DefaultEnvironment::define(name, value);
  signal_environment_update(name, value);
}

void TableLispEnvironment::set(const std::string &name,
                               const lisp::LispObjectPtr &value) {
  DefaultEnvironment::set(name, value);
  signal_environment_update(name, value);
}

void TableLispEnvironment::remove_references(const std::string &name) {
  // The given cell is about to be updated - clear references
  references_.erase(name);

  for (auto &chunk : referenced_by_) {
    auto &cells = chunk.second;

    if (cells.contains(name)) {
      cells.erase(name);
    }
  }
}

bool TableLispEnvironment::is_reachable(const std::string &start,
                                        const std::string &target) {
  std::unordered_set<std::string> visited;
  std::stack<std::string> stack;
  stack.push(start);

  while (!stack.empty()) {
    auto current = stack.top();
    stack.pop();
    if (current == target) {
      return true;
    }
    if (visited.contains(current)) {
      continue;
    }
    visited.insert(current);
    for (const auto &neighbor : references_[current]) {
      stack.push(neighbor);
    }
  }

  return false;
}

void TableLispEnvironment::update_references(const std::string &from_cell,
                                             const std::string &to_cell) {
  if (is_reachable(to_cell, from_cell)) {
    throw CircularReferenceError("Circular reference detected", from_cell,
                                 to_cell);
  }

  references_[from_cell].insert(to_cell);
  referenced_by_[to_cell].insert(from_cell);
}

lisp::LispObjectPtr TableLispEnvironment::lookup(const std::string &name,
                                                 const std::any &context) {
  const auto *table_context = std::any_cast<TableContext>(&context);
  if (table_context != nullptr) {
    const std::string &from_cell = table_context->source_cell;

    if (!from_cell.empty() && from_cell != name && is_cell_name(name)) {
      update_references(from_cell, name);
    }
  }

  return DefaultEnvironment::lookup(name, context);
}
