// KalQlator - DocumentJsonSerializer.h
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

#include <QJsonArray>
#include <string>
#include <utility>

#include "../model/Document.h"

struct CellValueTask {
    Location location;
    std::string name;
    std::string content;
    std::string formula;
};

class DocumentJsonSerializer {
public:
    DocumentJsonSerializer(DocumentPtr document, std::string filename) : document_(std::move(document)),
        filename_(std::move(filename)) {

    }

    virtual ~DocumentJsonSerializer() = default;

    static QJsonArray create_cells_array(Sheet *sheet);

    static QJsonObject location_to_json(Location location);

    static QJsonArray convert_selected_cells_to_json_array(const Sheet *sheet) ;

    static QJsonArray get_row_heights(const Sheet* sheet) ;

    static QJsonArray get_column_widths(const Sheet* sheet) ;

    static QJsonObject create_macros_json(const MacroMap & map) ;

    [[nodiscard]] bool save() const;

    static void create_cell_by_task(Sheet* sheet, const CellValueTask &task);

    static void setSheetSizes(
        const QJsonArray& array,
        const QString& sizeKey,
        std::function<void(size_t, size_t)> setter) ;

    static void setSheetRowHeights(Sheet* sheet, const QJsonArray & heights) ;

    static void setSheetColumnWidths(Sheet* sheet, const QJsonArray & widths) ;

    static void applySizes(Sheet* sheet, const QJsonObject & json_values);

    void add_sheets(const QJsonObject &workbook) const;

    void add_macros(const QJsonObject & workbook) const;

    [[nodiscard]] bool open() const;

private:
    [[nodiscard]] bool write_to_file(const QString& json) const;

    DocumentPtr document_;
    std::string filename_;
};
