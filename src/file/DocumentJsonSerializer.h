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
    DocumentJsonSerializer(DocumentPtr document, std::string filename,
                           const std::weak_ptr<SheetRegistry> &sheet_registry) : document_(std::move(document)),
        filename_(std::move(filename)),
        sheet_registry_(sheet_registry) {
    }

    virtual ~DocumentJsonSerializer() = default;

    static QJsonArray create_cells_array(const std::shared_ptr<Sheet> &sheet);

    static QJsonObject location_to_json(Location location);

    QJsonArray convert_selected_cells_to_json_array(const std::shared_ptr<Sheet> &sheet) const;

    QJsonArray get_row_heights(const SheetPtr& sheet) const;

    QJsonArray get_column_widths(const SheetPtr& sheet) const;

    QJsonObject create_macros_json(const MacroMap & map) const;

    [[nodiscard]] bool save() const;

    static void create_cell_by_task(const SheetPtr &sheet, const CellValueTask &task);

    void setSheetSizes(
        const QJsonArray& array,
        const QString& sizeKey,
        std::function<void(size_t, size_t)> setter) const;

    void setSheetRowHeights(const SheetPtr & sheet, const QJsonArray & jsons) const;

    void setSheetColumnWidths(const SheetPtr & sheet, const QJsonArray & jsons) const;

    void applySizes(const SheetPtr & sheet, const QJsonObject & json_values) const;

    void add_sheets(const QJsonObject &workbook) const;

    void add_macros(const QJsonObject & workbook) const;

    [[nodiscard]] bool open() const;

private:
    DocumentPtr document_;
    std::string filename_;
    std::weak_ptr<SheetRegistry> sheet_registry_;
};
