// KalQlator - Sheet.h
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

#include <memory>
#include <optional>
#include <map>
#include <ranges>

#include "Cell.h"
#include "TableLispEnvironment.h"
#include "../ui/user_interface_tools.h"
#include "../tools/location.h"
#include "search/SearchResultItem.h"

struct SearchOptions;
class SheetRegistry;
typedef std::map<Location, CellPtr> CellMap;

struct FormulaResult {
    lisp::LispTokens tokens;
    lisp::LispObjectPtrVector formula;
    lisp::LispObjectPtr result;
};

class Sheet {
public:
    explicit Sheet(std::string id, std::string name, const std::weak_ptr<SheetRegistry> &sheet_registry);

    [[nodiscard]] std::string name() { return name_; }

    [[nodiscard]] std::string id() const { return id_; }

    std::optional<CellPtr> get_cell(const Location &location);

    std::optional<CellPtr> get_cell(int row, int column);

    std::optional<CellPtr> get_cell_by_name(const std::string &name);

    CellPtr create_cell_model(const Location &location);

    CellPtr get_or_create_cell_by_pos(int row, int column);

    /**
     * Update cells from UI event handler.
     * @param row Row index
     * @param column Column index
     * @param cell_name Name of the cell to be updated
     * @param content New content of cell
     */
    void update_cell(int row, int column, const std::string &cell_name, const std::string &content);

    void set_cell_content(const CellPtr &cell, const std::string &content);

    void set_cell_content(int row, int column, const std::string &content);

    void set_current_cell(const Location &location);

    template<typename Func>
    void for_each_cell(Func &&func) {
        for (auto &val: cells_ | std::views::values) {
            func(val);
        }
    }

    void update_all_cells();

    void refresh_cells(const std::string &name, const lisp::LispObjectPtr &value,
                       const pdtools::StringVector &dependencies);

    void set_name(const std::string &name);

    // TODO: Might need to add current cell
    [[nodiscard]] LocationSet get_selected_cells() const { return selected_cells_; }

    void set_selected_cells(const LocationSet &current_selection) { selected_cells_ = current_selection; }

    [[nodiscard]] Location get_current_selected_cell() const { return current_selected_cell_; }

    [[nodiscard]] lisp::EnvironmentPtr environment() const {
        // TODO: Read only env wrapper?
        return tableLispEnvironment_;
    }

    Location get_max_cell_locations() const;


    void set_row_height(size_t row_index, size_t height) {
        row_heights_[row_index] = height;
    }

    void set_column_width(size_t column_index, size_t width) {
        column_widths_[column_index] = width;
    }

    std::unordered_map<size_t, size_t> get_column_widths() const { return column_widths_; }
    std::unordered_map<size_t, size_t> get_row_heights() const { return row_heights_; }


    static bool field_matches_search(const SearchOptions & options, const std::string & field_content, std::string * out_complete_match);

    static bool matches_search(const SearchOptions &options, const CellPtr &cell, std::string *out_complete_match);

    SearchResultItems search(const SearchOptions &options) const;

private:
    FormulaResult evaluate_formula(const std::string &formula_text,
                                   const lisp::EnvironmentPtr &env,
                                   const std::string &cell_name) const;

    /**
     * Update content by cell data itself.
     *
     * @param cell Cell to be updated
     */
    void refresh_cell(const CellPtr &cell) const;

    /**
     * Update cell content with explicit arguments. Used for updates
     * coming from the UI.
     *
     * @param cell_name
     * @param content
     * @param is_func
     * @param evaluation
     * @param cell_p
     */
    static void update_cell_contents(const std::string &content, bool is_func,
                                     FormulaResult evaluation, const CellPtr &cell_p);

    std::string id_;
    std::string name_;
    CellMap cells_;
    TableLispEnvironmentPtr tableLispEnvironment_;
    LocationSet selected_cells_;
    Location current_selected_cell_;
    std::weak_ptr<SheetRegistry> sheet_registry_;

    std::unordered_map<size_t, size_t> column_widths_;
    std::unordered_map<size_t, size_t> row_heights_;
};

typedef std::shared_ptr<Sheet> SheetPtr;
