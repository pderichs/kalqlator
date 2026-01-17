// KalQlator - KalqlatorTableWidget.h
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

#include <QTableWidget>

#include "KalqlatorTableCellItemDelegate.h"
#include "../events/CellUpdateDoneEvent.h"
#include "../tools/location.h"

class KalqlatorTableWidget : public QTableWidget {
    Q_OBJECT

public:
    explicit KalqlatorTableWidget(int rows, int cols, QWidget *parent = nullptr);

    QTableWidgetItem* query_item_or_create(int row, int col);

    void update_cell(const CellPtr& cell);

    void clearCell(const QModelIndex &index);

    void setSelectedCells(const LocationSet& selected_cells, const Location& current_selected_cell);

    void clearAndResetSizes();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onCellChanged(int row, int col) const;

    void onSelectionChanged() const;

    void onColumnResized(int logicalIndex, int oldSize, int newSize) const;

    void onRowResized(int logicalIndex, int oldSize, int newSize) const;

private:
    bool internal_cell_update_flag_;
};
