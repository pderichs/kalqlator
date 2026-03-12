// KalQlator - Document.cpp
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

#include "Document.h"

#include "triggers.h"
#include "../events/MacroErrorEvent.h"
#include "../ui/user_interface_tools.h"
#include "../messagebus/event_dispatcher.h"
#include "../lisp/tokenizer/lisp_tokenizer.h"
#include "../lisp/parser/parser.h"
#include "../lisp/Evaluator.h"
#include "../lisp/tools.h"

Document::Document() : current_sheet_index_(0), changed_(false) {
}

void Document::initialize(const bool add_initial_sheet) {
    if (add_initial_sheet) {
        add_next_sheet();
    }

    set_changed_flag(false);
    set_active_sheet(0);
    filename_.clear();
}

void Document::add_sheet(const SheetPtr &sheet) {
    sheets_.push_back(sheet);
    set_changed_flag(true);

    if (macros_.contains(Trigger_OnLoad)) {
        if (!parsed_macro_cache_.contains(Trigger_OnLoad)) {
            parsed_macro_cache_[Trigger_OnLoad] = lisp::parse_all_string(macros_.at(Trigger_OnLoad));
        }
        const auto &lisp = parsed_macro_cache_.at(Trigger_OnLoad);

        sheet->run_macros_by_trigger(Trigger_OnLoad, lisp);
    }
}

void Document::remove_sheet(size_t index) {
    sheets_.erase(sheets_.begin() + index);
    set_changed_flag(true);
}

template<typename Predicate>
std::optional<SheetPtr> Document::find_sheet(Predicate pred) const {
    auto it = std::ranges::find_if(sheets_, pred);
    return it != sheets_.end() ? std::optional{*it} : std::nullopt;
}

std::optional<SheetPtr> Document::get_sheet_by_name(const std::string &name) const {
    return find_sheet([&](const SheetPtr &s) { return s->name() == name; });
}

std::optional<SheetPtr> Document::get_sheet_by_id(const std::string &id) const {
    return find_sheet([&](const SheetPtr &s) { return s->id() == id; });
}

SheetPtr Document::current_sheet() const {
    return sheets_.at(current_sheet_index_);
}

void Document::set_cell_content(int row, int column, const std::string &content) {
    const SheetPtr sheet = current_sheet();
    set_cell_content(sheet, row, column, content);

    set_changed_flag(true);
}

void Document::update_all_cells() {
    const SheetPtr sheet = current_sheet();
    sheet->update_all_cells();
}

void Document::refresh_cells(const std::string &name, const lisp::LispObjectPtr &value,
                             const pdtools::StringVector &dependencies) {
    const SheetPtr sheet = current_sheet();
    sheet->refresh_cells(name, value, dependencies);
}

std::weak_ptr<SheetRegistry> Document::get_sheet_registry_weak_ptr() {
    return std::weak_ptr<SheetRegistry>(
        std::static_pointer_cast<SheetRegistry>(shared_from_this())
    );
}

int Document::add_next_sheet() {
    const int next_index = sheets_.size();

    std::stringstream ss;
    ss << "Table ";
    ss << (next_index + 1);

    auto weak_self = get_sheet_registry_weak_ptr();
    const auto sheet = std::make_shared<Sheet>(pdtools::generate_uuid(), ss.str(), weak_self);
    add_sheet(sheet);

    set_changed_flag(true);

    return next_index;
}

void Document::remove_current_sheet() {
    if (sheets_.size() > 1) {
        remove_sheet(current_sheet_index_);
        set_active_sheet(0);

        set_changed_flag(true);
    }
}


void Document::move_current_sheet(MoveSheetDirection direction) {
    if (direction == UP) {
        if (current_sheet_index_ == 0) {
            return;
        }
        std::swap(sheets_[current_sheet_index_], sheets_[current_sheet_index_ - 1]);
        current_sheet_index_--;
    } else {
        // DOWN
        if (current_sheet_index_ == (sheets_.size() - 1)) {
            return;
        }
        std::swap(sheets_[current_sheet_index_], sheets_[current_sheet_index_ + 1]);
        current_sheet_index_++;
    }

    set_changed_flag(true);
}

void Document::rename_current_sheet(const std::string &name) {
    if (name.empty()) {
        return;
    }

    const SheetPtr sheet = current_sheet();
    sheet->set_name(name);

    set_changed_flag(true);
}

void Document::set_selected_cells(const LocationSet &selected_cells) {
    const SheetPtr sheet = current_sheet();
    sheet->set_selected_cells(selected_cells);

    set_changed_flag(true);
}

void Document::set_cell_content(const SheetPtr &sheet, int row, int column, const std::string &content) {
    sheet->set_cell_content(row, column, content);

    set_changed_flag(true);
}

void Document::set_current_cell(const Location &location) {
    const SheetPtr sheet = current_sheet();
    sheet->set_current_cell(location);

    set_changed_flag(true);
}

void Document::clear(bool add_initial_sheet) {
    sheets_.clear();
    macros_.clear();
    initialize(add_initial_sheet);
}

Location Document::get_current_selected_cell() const {
    const SheetPtr sheet = current_sheet();
    return sheet->get_current_selected_cell();
}

LocationSet Document::get_selected_cells() const {
    const SheetPtr sheet = current_sheet();
    return sheet->get_selected_cells();
}

void Document::set_row_height(size_t row_index, size_t height) {
    const SheetPtr sheet = current_sheet();
    sheet->set_row_height(row_index, height);
}

void Document::set_column_width(size_t column_index, size_t width) {
    const SheetPtr sheet = current_sheet();
    sheet->set_column_width(column_index, width);
}

std::unordered_map<size_t, size_t> Document::sheet_row_heights() const {
    const SheetPtr sheet = current_sheet();
    return sheet->get_row_heights();
}

std::unordered_map<size_t, size_t> Document::sheet_column_widths() const {
    const SheetPtr sheet = current_sheet();
    return sheet->get_column_widths();
}

SearchResultItems Document::search(const SearchOptions &options) const {
    SearchResultItems result;

    for (const auto &sheet: sheets_) {
        const auto sheet_result = sheet->search(options);
        if (!sheet_result.empty()) {
            result.reserve(result.size() + sheet_result.size());
            result.insert(result.end(), sheet_result.begin(), sheet_result.end());
        }
    }

    return result;
}

int Document::get_sheet_index(const std::shared_ptr<Sheet> &sheet) {
    int i = 0;

    for (const auto &s: sheets_) {
        if (s == sheet) {
            return i;
        }

        i++;
    }

    return -1;
}

void Document::select_sheet_and_cell(const std::string &table_name, const Location &cell_location) {
    const auto &opt_sheet = get_sheet_by_name(table_name);
    if (!opt_sheet) {
        // Something is off.
        return;
    }

    const auto &sheet = *opt_sheet;

    int index = get_sheet_index(sheet);
    if (index < 0) {
        // Something is off.
        return;
    }

    set_active_sheet(index);

    EventDispatcher::dispatch("model:sheet_selection_changed");
    sheet->set_current_cell(cell_location);
}

std::string Document::get_cell_raw_content(int row, int col) const {
    const auto &sheet = current_sheet();
    const auto &opt_cell = sheet->get_cell(row, col);
    if (!opt_cell) return "";
    return (*opt_cell)->raw_content_;
}

void Document::run_macros_by_trigger(const std::string &trigger) {
    if (!macros_.contains(trigger)) {
        return;
    }

    const auto &def = macros_.at(trigger);
    if (def.empty()) {
        return;
    }

    try {
        lisp::LispObjectPtrVector lisp = lisp::parse_all_string(def);

        for (const auto &sheet: sheets_) {
            sheet->run_macros_by_trigger(trigger, lisp);
        }
    } catch (const std::runtime_error &e) {
        EventDispatcher::dispatch("model:macro-error", MacroErrorEvent{trigger, def, e.what()});
    }
}
