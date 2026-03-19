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
#include <QHeaderView>
#include <QKeyEvent>

#include "KalqlatorTableWidget.h"

#include "../events/CellChangedEvent.h"
#include "../events/CellEvent.h"
#include "../events/SelectionChangedEvent.h"
#include "../events/TablePropertyResizedEvent.h"
#include "../messagebus/event_dispatcher.h"
#include "../tools/FlagScope.h"
#include "../tools/location.h"
#include "TableErrorDelegate.h"

KalqlatorTableWidget::KalqlatorTableWidget(QWidget *parent)
    : QTableView(parent) {
    internal_cell_update_flag_ = false;

    setItemDelegate(new KalqlatorTableCellItemDelegate(this));

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(horizontalHeader(), &QHeaderView::sectionResized, this,
            &KalqlatorTableWidget::onColumnResized);

    connect(verticalHeader(), &QHeaderView::sectionResized, this,
            &KalqlatorTableWidget::onRowResized);
}

void KalqlatorTableWidget::setModel(QAbstractItemModel *model) {
    QTableView::setModel(model);

    connect(selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &KalqlatorTableWidget::onSelectionChanged);

    connect(selectionModel(), &QItemSelectionModel::currentChanged,
        this, &KalqlatorTableWidget::onCurrentChanged);
}

void KalqlatorTableWidget::clearCell(const QModelIndex &index) {
    model()->setData(index, QString(), Qt::EditRole);
    model()->setData(index, QString(), FormulaRole);
    model()->setData(index, QString(), ErrorRole);
}

void KalqlatorTableWidget::setSelectedCells(
    const LocationSet &selected_cells, const Location &current_selected_cell) {
    QItemSelection selection;

    for (const auto &loc: selected_cells) {
        QModelIndex index = model()->index(loc.row(), loc.column());
        if (!index.isValid()) {
            qWarning() << "Invalid index:" << loc.row() << loc.column();
            continue;
        }

        selection.select(index, index);
    }

    selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);

    QModelIndex currentIndex =
            model()->index(current_selected_cell.y(), current_selected_cell.x());
    selectionModel()->setCurrentIndex(currentIndex,
                                      QItemSelectionModel::NoUpdate);
}

void KalqlatorTableWidget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Delete) {
        const auto selectedItems = selectedIndexes();
        for (const QModelIndex &index: selectedItems) {
            clearCell(index);
        }

        viewport()->update();
    } else {
        QTableView::keyPressEvent(event);
    }
}

void KalqlatorTableWidget::onSelectionChanged() const {
    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        return;
    }

    LocationSet selected_cells;
    for (const QModelIndex &index: selectionModel()->selectedIndexes()) {
        selected_cells.insert(Location{index.column(), index.row()});
    }

    const Location current_cell(current.column(), current.row());

    EventDispatcher::dispatch(
        "ui:cell_selection_changed",
        SelectionChangedEvent{
            .selection = selected_cells,
            .current_cell = current_cell
        });
}

void KalqlatorTableWidget::onCurrentChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
    if (!current.isValid()) {
        return;
    }

    const Location current_cell(current.column(), current.row());

    EventDispatcher::dispatch(
        "ui:cell_selection_changed",
        SelectionChangedEvent{
            .selection = {},
            .current_cell = current_cell
        });
}

void KalqlatorTableWidget::onColumnResized(int logicalIndex, int oldSize,
                                           int newSize) {
    EventDispatcher::dispatch(
        "ui:column_resized",
        TablePropertyResizedEvent{
            .logical_index = logicalIndex,
            .old_size = oldSize,
            .new_size = newSize
        });
}

void KalqlatorTableWidget::onRowResized(int logicalIndex, int oldSize,
                                        int newSize) {
    EventDispatcher::dispatch(
        "ui:row_resized", TablePropertyResizedEvent{
            .logical_index = logicalIndex,
            .old_size = oldSize,
            .new_size = newSize
        });
}
