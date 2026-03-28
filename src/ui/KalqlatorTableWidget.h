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

#include "../tools/location.h"
#include "KalqlatorTableCellItemDelegate.h"

class KalqlatorTableWidget : public QTableView {
  Q_OBJECT

public:
  explicit KalqlatorTableWidget(QWidget *parent = nullptr);

  void setModel(QAbstractItemModel *model) override;

  void clearCell(const QModelIndex &index);

  void setSelectedCells(const LocationSet &selected_cells,
                        const Location &current_selected_cell);

protected:
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  // void onCellChanged(int row, int col) const;

  void onSelectionChanged() const;

  static void onCurrentChanged(const QModelIndex &current,
                               const QModelIndex & /*previous*/);

  static void onColumnResized(int logicalIndex, int oldSize, int newSize);

  static void onRowResized(int logicalIndex, int oldSize, int newSize);

private:
  bool internal_cell_update_flag_;
};
