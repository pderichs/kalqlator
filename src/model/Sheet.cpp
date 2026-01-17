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

#include <iomanip>
#include <QDebug>

#include "CircularReferenceError.h"
#include "../lisp/Evaluator.h"
#include "../lisp/parser/parser.h"
#include "../lisp/tokenizer/lisp_tokenizer.h"
#include "../events/CellUpdateDoneEvent.h"
#include "../events/SelectedCellChangedEvent.h"
#include "../events/CellUpdateErrorEvent.h"
#include "LispObjectStringConverter.h"
#include "SheetRegistry.h"
#include "TableContext.h"
#include "../messagebus/event_dispatcher.h"
#include "../tools/tools.h"
#include "search/SearchOptions.h"

Sheet::Sheet(std::string id, std::string name, const std::weak_ptr<SheetRegistry> &sheet_registry) : id_(std::move(id)),
    name_(std::move(name)), sheet_registry_(sheet_registry) {
    tableLispEnvironment_ = std::make_shared<TableLispEnvironment>();
    tableLispEnvironment_->initialize();

    current_selected_cell_ = Location(0, 0);
    selected_cells_.insert(current_selected_cell_);
}

std::optional<CellPtr> Sheet::get_cell(const Location &location) {
    const auto it = cells_.find(location);
    if (it != cells_.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::optional<CellPtr> Sheet::get_cell(int row, int column) {
    return get_cell(Location(column, row));
}

std::optional<CellPtr> Sheet::get_cell_by_name(const std::string &name) {
    const auto &location = get_cell_location_by_name(name);
    return get_cell(location);
}


Location Sheet::get_max_cell_locations() const {
    int max_x = 0;
    int max_y = 0;

    for (const auto &item: cells_) {
        if (item.second->column_ > max_x) {
            max_x = item.second->column_;
        }

        if (item.second->row_ > max_y) {
            max_y = item.second->row_;
        }
    }

    return Location(max_x, max_y);
}

bool Sheet::field_matches_search(const SearchOptions &options, const std::string &field_content,
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
        std::ranges::transform(content, content.begin(),
                               [](unsigned char c) { return std::toupper(c); });

        std::ranges::transform(search_text, search_text.begin(),
                               [](unsigned char c) { return std::toupper(c); });
    }

    if (content.find(search_text) != std::string::npos) {
        *out_complete_match = field_content;
        return true;
    }

    return false;
}

bool Sheet::matches_search(const SearchOptions &options, const CellPtr &cell, std::string *out_complete_match) {
    bool match = false;

    // TODO Refactor to own class.

    if (options.scope == SearchScope::ContentAndFormula || options.scope == SearchScope::OnlyContent) {
        match = field_matches_search(options, cell->visible_content_, out_complete_match);
    }

    if (!match) {
        if (options.scope == SearchScope::ContentAndFormula || options.scope == SearchScope::OnlyFormula) {
            match = field_matches_search(options, cell->raw_formula_, out_complete_match);
        }
    }

    return match;
}

SearchResultItems Sheet::search(const SearchOptions &options) const {
    SearchResultItems result;

    for (const auto &cell_item: cells_) {
        const auto &location = cell_item.first;
        const auto &cell = cell_item.second;

        std::string complete_match;
        if (matches_search(options, cell, &complete_match)) {
            auto result_item = std::make_shared<SearchResultItem>(
                -1, // TODO
                name_,
                location,
                complete_match);
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
    lisp::Evaluator evaluator(env, TableContext{cell_name, sheet_registry_});
    auto result = evaluator.evaluate(formula);

    return {std::move(tokens), std::move(formula), std::move(result)};
}

CellPtr Sheet::create_cell_model(const Location &location) {
    auto cell = std::make_shared<Cell>(location.y(), location.x(), get_cell_name_by_coordinates(location));
    cells_[location] = cell;
    return cell;
}

CellPtr Sheet::get_or_create_cell_by_pos(int row, int column) {
    CellPtr cell_p;
    auto cell = get_cell(row, column);
    if (!cell) {
        cell_p = create_cell_model(Location(column, row));
        cells_[Location(column, row)] = cell_p;
    } else {
        cell_p = *cell;
    }
    return cell_p;
}


void Sheet::refresh_cell(const CellPtr &cell) const {
    const std::string content = cell->raw_content_;
    const bool is_func = is_function(content);
    const std::string formula_text = is_func ? content.substr(1) : content;

    tableLispEnvironment_->remove_references(cell->name_);

    const auto evaluation = evaluate_formula(formula_text, tableLispEnvironment_, cell->name_);
    update_cell_contents(cell->raw_content_, cell->contains_formula(), evaluation, cell);

    tableLispEnvironment_->define(cell->name_, evaluation.result); // TODO Fires event in model - required?
}

void Sheet::update_cell_contents(
    const std::string &content,
    const bool is_func,
    FormulaResult evaluation,
    const CellPtr &cell_p) {
    cell_p->raw_content_ = content;
    cell_p->tokens_ = std::move(evaluation.tokens);
    cell_p->formula_ = std::move(evaluation.formula);
    cell_p->visible_content_ = is_func ? LispObjectStringConverter(evaluation.result).to_str() : content;
    cell_p->raw_formula_ = is_func ? content : "";
}

// Called when a new document is loaded
void Sheet::update_all_cells() {
    for_each_cell([this](const auto &cell) {
        // Trigger cell reference update
        set_cell_content(cell, cell->raw_content_);
    });
}

void Sheet::refresh_cells(const std::string &name, const lisp::LispObjectPtr &,
                          const pdtools::StringVector &dependencies) {
    std::vector<CellUpdateDoneEvent> events;

    for (const auto &dependency_cell_name: dependencies) {
        if (dependency_cell_name == name) {
            continue;
        }

        auto location = get_cell_location_by_name(dependency_cell_name);
        auto opt_cell = get_cell(location.y(), location.x());
        if (opt_cell) {
            auto cell = *opt_cell;
            refresh_cell(cell);

            events.push_back(CellUpdateDoneEvent{{cell->row_, cell->column_}, cell});
        }
    }

    for (const auto &e: events) {
        EventDispatcher::dispatch("model:cell_update_done", e);
    }
}

void Sheet::set_name(const std::string &name) {
    name_ = name;
}

std::string modify_user_entry(const std::string &content, bool is_func, bool is_number) {
    std::string text = is_func ? content.substr(1) : content;

    if (!is_func && !is_number) {
        std::ostringstream oss;
        oss << std::quoted(text);
        return oss.str();
    }

    return text;
}

void Sheet::update_cell(int row, int column, const std::string &cell_name, const std::string &content) {
    const bool is_func = is_function(content);
    const bool is_number = pdtools::is_number(content);

    const std::string formula_text = modify_user_entry(content, is_func, is_number);

    const CellPtr cell_p = get_or_create_cell_by_pos(row, column);

    try {
        cell_p->clear_errors();
        cell_p->raw_content_ = content;

        tableLispEnvironment_->remove_references(cell_name);

        const auto evaluation = evaluate_formula(formula_text, tableLispEnvironment_, cell_name);
        update_cell_contents(content, is_func, evaluation, cell_p);
        tableLispEnvironment_->define(cell_name, evaluation.result);
    } catch (const CircularReferenceError &circular_reference_error) {
        cell_p->add_error(CellError{ERROR_CIRCREF, circular_reference_error.what()});
        cell_p->raw_formula_ = "";
        cell_p->visible_content_ = content;

        EventDispatcher::dispatch("model:cell_update_error", CellUpdateErrorEvent{
                                      cell_p, content, circular_reference_error.what(), ERROR_CIRCREF
                                  });
    } catch (const std::runtime_error &e) {
        qDebug() << e.what();

        cell_p->add_error(CellError{ERROR_GENERAL, e.what()});
        cell_p->raw_formula_ = "";
        cell_p->visible_content_ = content;

        EventDispatcher::dispatch("model:cell_update_error",
                                  CellUpdateErrorEvent{cell_p, content, e.what(), ERROR_GENERAL});
    }

    EventDispatcher::dispatch("model:cell_update_done", CellUpdateDoneEvent{{row, column}, cell_p});
}

void Sheet::set_cell_content(const CellPtr &cell, const std::string &content) {
    set_cell_content(cell->row_, cell->column_, content);
}

void Sheet::set_cell_content(int row, int column, const std::string &content) {
    const std::string cell_name = get_cell_name_by_coordinates(column, row);

    update_cell(row, column, cell_name, content);
}

void Sheet::set_current_cell(const Location &location) {
    current_selected_cell_ = location;
    const auto cell_p = get_or_create_cell_by_pos(location.y(), location.x());
    EventDispatcher::dispatch("model:selected_cell_changed", SelectedCellChangedEvent{cell_p});
}
