// KalQlator - DocumentJsonSerializer.cpp
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

#include "DocumentJsonSerializer.h"

#include <fstream>
#include <QJsonDocument>
#include <QJsonObject>

#define KALQLATOR_VERSION "0.0.1"

QJsonArray DocumentJsonSerializer::create_cells_array(const std::shared_ptr<Sheet> &sheet) {
    QJsonArray cellsArray;

    sheet->for_each_cell([&cellsArray](const CellPtr &cell) {
        if (!cell->empty()) {
            QJsonObject cell1;
            cell1["name"] = QString::fromStdString(cell->name_);
            cell1["content"] = QString::fromStdString(cell->raw_content_);

            cellsArray.append(cell1);
        }
    });

    return cellsArray;
}

QJsonObject DocumentJsonSerializer::location_to_json(Location location) {
    QJsonObject result;

    result["x"] = static_cast<int>(location.x());
    result["y"] = static_cast<int>(location.y());

    return result;
}

QJsonArray DocumentJsonSerializer::convert_selected_cells_to_json_array(const std::shared_ptr<Sheet> &sheet) const {
    QJsonArray selected_cells;
    for (const auto &location: sheet->get_selected_cells()) {
        QJsonObject selected_cell = location_to_json(location);
        selected_cells.append(selected_cell);
    }
    return selected_cells;
}

QJsonArray DocumentJsonSerializer::get_row_heights(const SheetPtr &sheet) const {
    QJsonArray result;
    const auto &row_heights = sheet->get_row_heights();

    for (const auto &item: row_heights) {
        QJsonObject rowHeightObj;
        rowHeightObj["index"] = static_cast<int>(item.first);
        rowHeightObj["height"] = static_cast<int>(item.second);
        result.append(rowHeightObj);
    }

    return result;
}

QJsonArray DocumentJsonSerializer::get_column_widths(const SheetPtr &sheet) const {
    QJsonArray result;
    const auto &column_widths = sheet->get_column_widths();

    for (const auto &item: column_widths) {
        QJsonObject colWidthObj;
        colWidthObj["index"] = static_cast<int>(item.first);
        colWidthObj["width"] = static_cast<int>(item.second);
        result.append(colWidthObj);
    }

    return result;
}

bool DocumentJsonSerializer::save() const {
    QJsonObject metaObj;
    metaObj["version"] = KALQLATOR_VERSION;

    QJsonObject workbookObj;

    QJsonArray sheetsArray;
    int index = 0;
    for (const auto &sheet: document_->sheets()) {
        QJsonObject sheetObj;
        sheetObj["index"] = index;
        sheetObj["id"] = QString::fromStdString(sheet->id());
        sheetObj["name"] = QString::fromStdString(sheet->name());

        sheetObj["current_cell"] = location_to_json(sheet->get_current_selected_cell());

        QJsonArray selected_cells = convert_selected_cells_to_json_array(sheet);
        sheetObj["selected_cells"] = selected_cells;

        auto cellsArray = create_cells_array(sheet);
        sheetObj["cells"] = cellsArray;

        sheetObj["row_heights"] = get_row_heights(sheet);
        sheetObj["column_widths"] = get_column_widths(sheet);

        sheetsArray.append(sheetObj);

        index++;
    }

    workbookObj["sheets"] = sheetsArray;

    QJsonObject rootObj;
    rootObj["meta"] = metaObj;
    rootObj["workbook"] = workbookObj;

    QJsonDocument doc(rootObj);
    QString json = doc.toJson(QJsonDocument::Indented);

    std::ofstream f(filename_, std::ofstream::out);
    if (!f) {
        return false;
    }

    f << json.toStdString();
    f.close();

    return true;
}

void DocumentJsonSerializer::create_cell_by_task(const SheetPtr &sheet, const CellValueTask &task) {
    const auto cell = sheet->create_cell_model(task.location);
    cell->name_ = task.name;
    cell->raw_content_ = task.content;
    cell->raw_formula_ = task.formula;
}

void DocumentJsonSerializer::setSheetSizes(
    const QJsonArray &array,
    const QString &sizeKey,
    std::function<void(size_t, size_t)> setter) const {
    for (const auto &item: array) {
        setter(item["index"].toInt(), item[sizeKey].toInt());
    }
}

void DocumentJsonSerializer::setSheetRowHeights(const SheetPtr &sheet, const QJsonArray &heights) const {
    setSheetSizes(heights, "height", [&](size_t i, size_t v) { sheet->set_row_height(i, v); });
}

void DocumentJsonSerializer::setSheetColumnWidths(const SheetPtr &sheet, const QJsonArray &widths) const {
    setSheetSizes(widths, "width", [&](size_t i, size_t v) { sheet->set_column_width(i, v); });
}

void DocumentJsonSerializer::applySizes(const SheetPtr &sheet, const QJsonObject &json_values) const {
    const auto &row_array = json_values["row_heights"].toArray();
    const auto &col_array = json_values["column_widths"].toArray();

    setSheetRowHeights(sheet, row_array);
    setSheetColumnWidths(sheet, col_array);
}

void DocumentJsonSerializer::add_sheets(const QJsonObject &workbook) const {
    // Apply values in order - first we set value cells then formulas.
    std::vector<CellValueTask> values;
    std::vector<CellValueTask> formulas;

    int index = 0;
    QJsonArray sheets = workbook["sheets"].toArray();
    for (const auto &item: sheets) {
        QJsonObject jsonSheet = item.toObject();
        SheetPtr sheet = std::make_shared<Sheet>(jsonSheet["id"].toString().toStdString(),
                                                 jsonSheet["name"].toString().toStdString(),
                                                 sheet_registry_);

        QJsonArray cellsArray = jsonSheet["cells"].toArray();
        for (const auto &cellItem: cellsArray) {
            QJsonObject jsonCell = cellItem.toObject();
            auto name = jsonCell["name"].toString().toStdString();

            std::string content = jsonCell["content"].toString().toStdString();

            std::string formula;
            if (is_function(content)) {
                formula = content;
            }

            const Location cell_location = get_cell_location_by_name(name);

            auto task = CellValueTask{
                cell_location,
                name,
                content,
                formula
            };

            if (formula.empty()) {
                values.push_back(task);
            } else {
                formulas.push_back(task);
            }
        }

        QJsonArray selected_cells = jsonSheet["selected_cells"].toArray();
        LocationSet selected_cells_set;
        for (const auto &selected_cell: selected_cells) {
            auto selected = selected_cell.toObject();
            Location location(selected["x"].toInt(), selected["y"].toInt());
            selected_cells_set.insert(location);
        }
        sheet->set_selected_cells(selected_cells_set);
        QJsonObject current = jsonSheet["current_cell"].toObject();
        sheet->set_current_cell(Location(current["x"].toInt(), current["y"].toInt()));

        // TODO Sizes
        applySizes(sheet, jsonSheet);

        document_->add_sheet(sheet);
        document_->set_active_sheet(index);

        for (const auto &task: values) {
            create_cell_by_task(sheet, task);
        }
        for (const auto &task: formulas) {
            qDebug() << "Updating formula: Setting current cell selection to " << task.location.y() << ", " << task.
                    location.x();
            create_cell_by_task(sheet, task);
        }

        index++;
    }
}

bool DocumentJsonSerializer::open() const {
    document_->clear(false); // Do not initialize with sheet

    std::ifstream file(filename_, std::ifstream::in);

    if (!file) {
        qWarning() << "Unable to open file";
        return false;
    }

    std::stringstream stringstream;
    std::string line;
    while (std::getline(file, line)) {
        stringstream << line;
    }

    QByteArray jsonData = QByteArray::fromStdString(stringstream.str());

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON";
        return false;
    }

    QJsonObject rootObj = doc.object();

    QJsonObject meta = rootObj["meta"].toObject();
    if (meta["version"].toString() != KALQLATOR_VERSION) {
        qWarning() << "Invalid version";
        return false;
    }

    document_->set_file_name(filename_);

    QJsonObject workbook = rootObj["workbook"].toObject();

    add_sheets(workbook);

    int active_sheet_index = workbook["active_sheet_index"].toInt();
    document_->set_active_sheet(active_sheet_index);

    return true;
}
