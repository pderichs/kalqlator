// KalQlator - Document.h
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
#include <vector>
#include <memory>

#include "MoveSheetDirection.h"
#include "Sheet.h"
#include "SheetRegistry.h"
#include "triggers.h"
#include "search/SearchResultItem.h"

using SheetVector = std::vector<std::unique_ptr<Sheet>>;
using MacroMap = std::map<std::string, std::string>;

class Document : public SheetRegistry {
public:
    Document() = default;

    void initialize(bool add_initial_sheet = true);

    void set_active_sheet(size_t index) { current_sheet_index_ = index; }
    [[nodiscard]] size_t get_active_sheet() const { return current_sheet_index_; }

    void set_file_name(const std::string &filename) { filename_ = filename; }

    [[nodiscard]] std::string get_file_name() const { return filename_; }

    void set_changed_flag(bool changed) { changed_ = changed; }
    [[nodiscard]] bool changed() const { return changed_; }

    size_t add_sheet(std::string identifier, std::string name);

    void remove_sheet(size_t index);

    template<typename Predicate>
    Sheet* find_sheet(Predicate pred) const;

    [[nodiscard]] Sheet* get_sheet_by_name(const std::string &name) const;

    [[nodiscard]] Sheet* get_sheet_by_id(const std::string &identifier) const;

    [[nodiscard]] size_t sheet_count() const {
        return sheets_.size();
    }

    [[nodiscard]] Sheet* current_sheet() const;

    void set_cell_content(int row, int column, const std::string &content);

    void update_all_cells() const;

    void refresh_cells(const std::string &name, const lisp::LispObjectPtr &value,
                       const pdtools::StringVector &dependencies) const;

    std::weak_ptr<SheetRegistry> get_sheet_registry_weak_ptr();

    int add_next_sheet();

    void remove_current_sheet();

    void move_current_sheet(MoveSheetDirection direction);

    void rename_current_sheet(const std::string &name);

    void set_selected_cells(const LocationSet &selected_cells);

    void set_cell_content(Sheet* sheet, int row, int column, const std::string &content);

    void set_current_cell(const Location &location);

    void clear(bool add_initial_sheet = true);

    [[nodiscard]] Location get_current_selected_cell() const;

    [[nodiscard]] LocationSet get_selected_cells() const;


    void set_row_height(size_t row_index, size_t height) const;

    void set_column_width(size_t column_index, size_t width) const;

    std::unordered_map<size_t, size_t> sheet_row_heights() const;
    std::unordered_map<size_t, size_t> sheet_column_widths() const;

    // Implementing SheetRegistry
    Sheet* sheet_by_index(size_t index) override {
        if (index >= sheets_.size()) {
            return nullptr;
        }

        return sheets_[index].get();
    }

    Sheet* sheet_by_name(const std::string &name) override {
        return get_sheet_by_name(name);
    }

    Sheet* sheet_by_id(const std::string &id) override {
        return get_sheet_by_id(id);
    }

    SearchResultItems search(const SearchOptions &options) const;

    int get_sheet_index(const Sheet *sheet) const;

    void select_sheet_and_cell(const std::string& table_name, const Location& cell_location);

    std::string get_cell_raw_content(int row, int col) const;

    MacroMap macro_map() const  { return macros_; }
    void set_macro_map(const MacroMap& map) {
        parsed_macro_cache_.clear();
        macros_ = map;

        run_macros_by_trigger(Trigger_OnLoad);
    }

    void run_macros_by_trigger(const std::string & trigger);

private:
    std::string filename_;
    SheetVector sheets_;
    size_t current_sheet_index_{0};
    bool changed_{false};

    MacroMap macros_;
    std::unordered_map<std::string, lisp::LispObjectPtrVector> parsed_macro_cache_;
};

using DocumentPtr = std::shared_ptr<Document>;
