// KalQlator - KalqlatorTableWidget.cpp
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

#include <QDebug>
#include <QKeyEvent>
#include <QHeaderView>

#include "KalqlatorTableWidget.h"

#include "TableErrorDelegate.h"
#include "../events/CellChangedEvent.h"
#include "../events/CellEvent.h"
#include "../events/SelectionChangedEvent.h"
#include "../events/TablePropertyResizedEvent.h"
#include "../messagebus/event_dispatcher.h"
#include "../tools/FlagScope.h"
#include "../tools/location.h"

KalqlatorTableWidget::KalqlatorTableWidget(int rows, int cols, QWidget *parent) : QTableWidget(rows, cols, parent) {
    internal_cell_update_flag_ = false;

    setItemDelegate(new KalqlatorTableCellItemDelegate(this));

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(this, &QTableWidget::cellChanged,
            this, &KalqlatorTableWidget::onCellChanged);

    connect(this, &QTableWidget::itemSelectionChanged,
            this, &KalqlatorTableWidget::onSelectionChanged);

    connect(horizontalHeader(), &QHeaderView::sectionResized,
        this, &KalqlatorTableWidget::onColumnResized);

    connect(verticalHeader(), &QHeaderView::sectionResized,
            this, &KalqlatorTableWidget::onRowResized);
}

QTableWidgetItem *KalqlatorTableWidget::query_item_or_create(int row, int col) {
    QTableWidgetItem *item = this->item(row, col);
    if (!item) {
        item = new QTableWidgetItem();
        setItem(row, col, item);
    }

    return item;
}

// Handling cell update from model
void KalqlatorTableWidget::update_cell(const CellPtr &cell) {
    FlagScope fs(&internal_cell_update_flag_);
    QTableWidgetItem *item = this->query_item_or_create(cell->row_, cell->column_);
    if (cell->has_errors()) {
        item->setData(Qt::DisplayRole, QString::fromStdString(cell->visible_content_));

        item->setData(ErrorRole, true);
        item->setToolTip(QString::fromStdString(cell->get_last_error()->message));
    } else if (cell->contains_formula()) {
        item->setData(Qt::DisplayRole, QString::fromStdString(cell->visible_content_));
        item->setData(FormulaRole, QString::fromStdString(cell->raw_formula_));

        item->setData(ErrorRole, false);
        item->setToolTip("");
    } else {
        item->setData(Qt::DisplayRole, QString::fromStdString(cell->visible_content_));
        item->setData(FormulaRole, QString::fromStdString(cell->visible_content_));

        item->setData(ErrorRole, false);
        item->setToolTip("");
    }
}

void KalqlatorTableWidget::clearCell(const QModelIndex &index) {
    model()->setData(index, QString(), Qt::EditRole);
    model()->setData(index, QString(), FormulaRole);
    model()->setData(index, QString(), ErrorRole);
}

void KalqlatorTableWidget::setSelectedCells(const LocationSet &selected_cells, const Location &current_selected_cell) {
    QItemSelection selection;

    for (const auto &loc: selected_cells) {
        QModelIndex index = model()->index(loc.row(), loc.column());
        if (!index.isValid()) {
            qWarning() << "Invalid index:" << loc.row() << loc.column();
            continue;
        }

        if (!item(loc.row(), loc.column())) {
            setItem(loc.row(), loc.column(), new QTableWidgetItem());
        }

        selection.select(index, index);
    }

    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);

    QModelIndex currentIndex = model()->index(
        current_selected_cell.y(), current_selected_cell.x());
    selectionModel()->setCurrentIndex(
        currentIndex, QItemSelectionModel::NoUpdate);
}

void KalqlatorTableWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete) {
        const auto selectedItems = selectedIndexes();
        for (const QModelIndex &index: selectedItems) {
            clearCell(index);
        }

        viewport()->update();
    } else {
        QTableWidget::keyPressEvent(event);
    }
}

void KalqlatorTableWidget::onCellChanged(int row, int col) const {
    // Exit if we are performing an internal update
    if (internal_cell_update_flag_) {
        return;
    }

    const auto *cell = item(row, col);
    if (!cell) return;

    const QString text = cell->data(FormulaRole).toString();

    EventDispatcher::dispatch("ui:cell_changed",
                              CellChangedEvent{{row, col}, text.toStdString()});
}

void KalqlatorTableWidget::onSelectionChanged() const {
    const int row = currentRow();
    const int col = currentColumn();

    if (row < 0 || col < 0) {
        return;
    }

    LocationSet selected_cells;
    for (const QModelIndex &index: selectedIndexes()) {
        selected_cells.insert(Location{index.column(), index.row()});
    }

    const Location current_cell(col, row);

    EventDispatcher::dispatch("ui:cell_selection_changed", SelectionChangedEvent{selected_cells, current_cell});
}

void KalqlatorTableWidget::onColumnResized(int logicalIndex, int oldSize, int newSize) const {
    EventDispatcher::dispatch("ui:column_resized", TablePropertyResizedEvent{logicalIndex, oldSize, newSize});
}

void KalqlatorTableWidget::onRowResized(int logicalIndex, int oldSize, int newSize) const {
    EventDispatcher::dispatch("ui:row_resized", TablePropertyResizedEvent{logicalIndex, oldSize, newSize});
}

void KalqlatorTableWidget::clearAndResetSizes() {
    const int defaultColWidth = horizontalHeader()->defaultSectionSize();
    const int defaultRowHeight = verticalHeader()->defaultSectionSize();

    for (int col = 0; col < columnCount(); ++col) {
        setColumnWidth(col, defaultColWidth);
    }

    for (int row = 0; row < rowCount(); ++row) {
        setRowHeight(row, defaultRowHeight);
    }

    clearContents();
}