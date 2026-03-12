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

    QJsonArray convert_selected_cells_to_json_array(Sheet *sheet) const;

    QJsonArray get_row_heights(Sheet* sheet) const;

    QJsonArray get_column_widths(Sheet* sheet) const;

    QJsonObject create_macros_json(const MacroMap & map) const;

    [[nodiscard]] bool save() const;

    static void create_cell_by_task(Sheet* sheet, const CellValueTask &task);

    void setSheetSizes(
        const QJsonArray& array,
        const QString& sizeKey,
        std::function<void(size_t, size_t)> setter) const;

    void setSheetRowHeights(Sheet* sheet, const QJsonArray & jsons) const;

    void setSheetColumnWidths(Sheet* sheet, const QJsonArray & jsons) const;

    void applySizes(Sheet* sheet, const QJsonObject & json_values) const;

    void add_sheets(const QJsonObject &workbook) const;

    void add_macros(const QJsonObject & workbook) const;

    [[nodiscard]] bool open() const;

private:
    DocumentPtr document_;
    std::string filename_;
};
