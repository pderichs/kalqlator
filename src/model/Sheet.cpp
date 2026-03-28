// KalQlator - Sheet.cpp
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

#include "Sheet.h"

#include <QDebug>
#include <algorithm>
#include <iomanip>

#include "../events/CellUpdateErrorEvent.h"
#include "../events/SelectedCellChangedEvent.h"
#include "../lisp/Evaluator.h"
#include "../lisp/LispObjectStringConverter.h"
#include "../lisp/parser/parser.h"
#include "../lisp/tokenizer/tokenizer.h"
#include "../messagebus/event_dispatcher.h"
#include "../tools/tools.h"
#include "CircularReferenceError.h"
#include "TableContext.h"
#include "search/SearchOptions.h"

Sheet::Sheet(std::string identifier, std::string name,
             SheetRegistry *sheet_registry)
    : id_(std::move(identifier)), name_(std::move(name)),
      table_lisp_environment_(std::make_shared<TableLispEnvironment>()),
      sheet_registry_(sheet_registry) {
  table_lisp_environment_->initialize();

  set_current_cell(Location(0, 0));
  selected_cells_.insert(current_selected_cell_);
}

Cell *Sheet::get_cell(const Location &location) const {
  const auto iterator = cells_.find(location);
  if (iterator != cells_.end()) {
    return iterator->second.get();
  }

  return nullptr;
}

Cell *Sheet::get_cell(int row, int column) const {
  return get_cell(Location(column, row));
}

Cell *Sheet::get_cell_by_name(const std::string &name) const {
  const auto &location = get_cell_location_by_name(name);
  return get_cell(location);
}

Cell *Sheet::create_cell_model(const Location &location) {
  cells_[location] =
      std::make_unique<Cell>(location.row(), location.column(),
                             get_cell_name_by_coordinates(location));
  return cells_[location].get();
}

Cell *Sheet::get_or_create_cell_by_pos(int row, int column) {
  auto *cell = get_cell(row, column);
  if (cell == nullptr) {
    auto location = Location(column, row);
    return create_cell_model(location);
  }

  return cell;
}

// Called when a new document is loaded
void Sheet::update_all_cells() {
  for_each_cell([this](const auto &cell) {
    // Trigger cell reference update
    set_cell_content(cell, cell->raw_content_);
  });
}

void Sheet::refresh_cells(const std::string &name,
                          const lisp::LispObjectPtr & /*unused*/,
                          const pdtools::StringVector &dependencies) {
  for (const auto &dependency_cell_name : dependencies) {
    if (dependency_cell_name == name) {
      continue;
    }

    auto location = get_cell_location_by_name(dependency_cell_name);
    auto *cell = get_cell(location.y(), location.x());
    if (cell != nullptr) {
      refresh_cell(cell);
    }
  }
}

void Sheet::set_name(const std::string &name) { name_ = name; }

Location Sheet::get_max_cell_locations() const {
  int max_x = 0;
  int max_y = 0;

  for (const auto &item : cells_) {
    max_x = std::max(item.second->column_, max_x);

    max_y = std::max(item.second->row_, max_y);
  }

  return Location(max_x, max_y);
}

void Sheet::run_macros_by_trigger(const std::string & /*unused*/,
                                  const lisp::LispObjectPtrVector &lisp) {
  lisp::Evaluator evaluator(table_lisp_environment_, {});
  evaluator.evaluate(lisp);
}

size_t Sheet::row_count() const { return row_count_; }

size_t Sheet::column_count() const { return column_count_; }

bool Sheet::field_matches_search(const SearchOptions &options,
                                 const std::string &field_content,
                                 std::string *out_complete_match) {
  if (options.useRegularExpression) {
    // Might throw exception
    const std::regex pattern(options.searchString);

    if (std::regex_match(field_content, pattern)) {
      *out_complete_match = field_content;
      return true;
    }

    return false;
  }

  // Normal string search
  std::string content(field_content);
  std::string search_text(options.searchString);

  if (!options.caseSensitive) {
    std::ranges::transform(
        content, content.begin(),
        [](unsigned char character) { return std::toupper(character); });

    std::ranges::transform(
        search_text, search_text.begin(),
        [](unsigned char character) { return std::toupper(character); });
  }

  if (content.find(search_text) != std::string::npos) {
    *out_complete_match = field_content;
    return true;
  }

  return false;
}

bool Sheet::matches_search(const SearchOptions &options, const Cell *cell,
                           std::string *out_complete_match) {
  bool match = false;

  // TODO Refactor to own class.

  if (options.scope == SearchScope::ContentAndFormula ||
      options.scope == SearchScope::OnlyContent) {
    match = field_matches_search(options, cell->visible_content_,
                                 out_complete_match);
  }

  if (!match) {
    if (options.scope == SearchScope::ContentAndFormula ||
        options.scope == SearchScope::OnlyFormula) {
      match =
          field_matches_search(options, cell->raw_formula_, out_complete_match);
    }
  }

  return match;
}

SearchResultItems Sheet::search(const SearchOptions &options) const {
  SearchResultItems result;

  for (const auto &cell_item : cells_) {
    const auto &location = cell_item.first;
    auto *const cell = cell_item.second.get();

    std::string complete_match;
    if (matches_search(options, cell, &complete_match)) {
      auto result_item =
          std::make_shared<SearchResultItem>(-1, // TODO
                                             name_, location, complete_match);
      result.push_back(result_item);
    }
  }

  return result;
}

FormulaResult Sheet::evaluate_formula(const std::string &formula_text,
                                      const lisp::EnvironmentPtr &env,
                                      const std::string &cell_name) const {
  lisp::Tokenizer tokenizer(formula_text);
  auto tokens = tokenizer.scan();
  lisp::Parser parser(tokens);
  auto formula = parser.parse_all();
  lisp::Evaluator evaluator(env,
                            TableContext{.source_cell = cell_name,
                                         .sheet_registry = sheet_registry_});
  auto result = evaluator.evaluate(formula);

  return {.tokens = std::move(tokens),
          .formula = std::move(formula),
          .result = std::move(result)};
}

void Sheet::refresh_cell(Cell *cell) const {
  const std::string content = cell->raw_content_;
  const bool is_func = is_function(content);
  const std::string formula_text = is_func ? content.substr(1) : content;

  table_lisp_environment_->remove_references(cell->name_);

  const auto evaluation =
      evaluate_formula(formula_text, table_lisp_environment_, cell->name_);
  update_cell_contents(cell->raw_content_, cell->contains_formula(), evaluation,
                       cell);

  table_lisp_environment_->define(
      cell->name_, evaluation.result); // TODO Fires event in model - required?
}

void Sheet::update_cell_contents(const std::string &content, const bool is_func,
                                 FormulaResult evaluation, Cell *cell_p) {
  cell_p->raw_content_ = content;
  cell_p->tokens_ = std::move(evaluation.tokens);
  cell_p->formula_ = std::move(evaluation.formula);
  cell_p->visible_content_ =
      is_func ? lisp::LispObjectStringConverter(evaluation.result).to_str()
              : content;
  cell_p->raw_formula_ = is_func ? content : "";
}

std::string modify_user_entry(const std::string &content, bool is_func,
                              bool is_number) {
  std::string text = is_func ? content.substr(1) : content;

  if (!is_func && !is_number) {
    std::ostringstream oss;
    oss << std::quoted(text);
    return oss.str();
  }

  return text;
}

void Sheet::update_cell(int row, int column, const std::string &cell_name,
                        const std::string &content) {
  const bool is_func = is_function(content);
  const bool is_number = pdtools::is_number(content);

  const std::string formula_text =
      modify_user_entry(content, is_func, is_number);

  Cell *cell_p = get_or_create_cell_by_pos(row, column);

  try {
    cell_p->clear_errors();
    cell_p->raw_content_ = content;

    table_lisp_environment_->remove_references(cell_name);

    const auto evaluation =
        evaluate_formula(formula_text, table_lisp_environment_, cell_name);
    update_cell_contents(content, is_func, evaluation, cell_p);
    table_lisp_environment_->define(cell_name, evaluation.result);
  } catch (const CircularReferenceError &circular_reference_error) {
    cell_p->add_error(CellError{.error_type = ERROR_CIRCREF,
                                .message = circular_reference_error.what()});
    cell_p->raw_formula_ = "";
    cell_p->visible_content_ = content;

    EventDispatcher::dispatch(
        CellUpdateErrorEvent{.cell = cell_p,
                             .content = content,
                             .error_message = circular_reference_error.what(),
                             .error_type = ERROR_CIRCREF});
  } catch (const std::runtime_error &e) {
    qDebug() << e.what();

    cell_p->add_error(
        CellError{.error_type = ERROR_GENERAL, .message = e.what()});
    cell_p->raw_formula_ = "";
    cell_p->visible_content_ = content;

    EventDispatcher::dispatch(
        CellUpdateErrorEvent{.cell = cell_p,
                             .content = content,
                             .error_message = e.what(),
                             .error_type = ERROR_GENERAL});
  }

  // TODO: Check CellUpdateDoneEvent
  // EventDispatcher::dispatch(
  //     CellUpdateDoneEvent{{.row = row, .col = column}, cell_p});
}

void Sheet::set_cell_content(const Cell *cell, const std::string &content) {
  set_cell_content(cell->row_, cell->column_, content);
}

LocationSet Sheet::collect_reference_cells(const std::string &cell_name) {
  pdtools::StringSet cells;

  auto references = table_lisp_environment_->get_references(cell_name);
  auto referenced_by = table_lisp_environment_->get_referenced_by(cell_name);

  cells.insert(references.begin(), references.end());
  cells.insert(referenced_by.begin(), referenced_by.end());

  LocationSet locations;

  for (const auto &cell : cells) {
    locations.insert(get_cell_location_by_name(cell));
  }

  return locations;
}

LocationSet Sheet::set_cell_content(int row, int column,
                                    const std::string &content) {
  const std::string cell_name = get_cell_name_by_coordinates(column, row);

  update_cell(row, column, cell_name, content);

  return collect_reference_cells(cell_name);
}

void Sheet::set_current_cell(const Location &location) {
  current_selected_cell_ = location;
  auto *const cell_p = get_or_create_cell_by_pos(location.y(), location.x());
  EventDispatcher::dispatch(SelectedCellChangedEvent{cell_p});
}
