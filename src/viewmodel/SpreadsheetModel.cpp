// KalQlator - SpreadSheetModel.cpp
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

#include "SpreadsheetModel.h"

#include "../model/CellDisplayFormatter.h"
#include "../ui/TableCellTypes.h"
#include <utility>

#include "../messagebus/event_dispatcher.h"
#include "events/CellChangedEvent.h"

SpreadsheetModel::SpreadsheetModel(DocumentPtr doc, QObject *parent)
    : QAbstractTableModel(parent), m_doc(std::move(doc)) {}

int SpreadsheetModel::rowCount(const QModelIndex & /*parent*/) const {
  return static_cast<int>(m_doc->row_count());
}

int SpreadsheetModel::columnCount(const QModelIndex & /*parent*/) const {
  return static_cast<int>(m_doc->column_count());
}

QVariant SpreadsheetModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return {};
  }

  const auto *cell = m_doc->get_cell(index.row(), index.column());

  if (cell == nullptr) {
    return {};
  }

  if (role == Qt::DisplayRole) {
    if (cell->format_.has_specifier() && cell->evaluated_value_ &&
        cell->evaluated_value_->is_number()) {
      QString formatted =
          QString::fromStdString(CellDisplayFormatter::format_number(
              cell->evaluated_value_, cell->format_));
      if (!formatted.isEmpty()) {
        return formatted;
      }
    }
    return QString::fromStdString(cell->visible_content_);
  }

  if (role == Qt::EditRole) {
    return QString::fromStdString(cell->raw_content_);
  }

  if (role == UnformattedValueRole) {
    return QString::fromStdString(cell->visible_content_);
  }

  if (role == WordWrapRole) {
    return cell->format_.word_wrap;
  }

  if (role == CellFormatRole) {
    return QString::fromStdString(cell->format_.specifier);
  }

  if (role == ErrorRole) {
    const auto opt_last_error = cell->get_last_error();
    if (opt_last_error) {
      return QString::fromStdString((*opt_last_error).message);
    }
  }

  if (role == FormulaRole) {
    return QString::fromStdString(cell->raw_formula_);
  }

  return {};
}

bool SpreadsheetModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
  if (!index.isValid()) {
    return false;
  }

  if (role == Qt::EditRole || role == FormulaRole) {
    const auto new_content = value.toString().toStdString();

    EventDispatcher::dispatch(CellChangedEvent{
        CellEvent{.row = index.row(), .col = index.column()}, new_content});

    auto dirty =
        m_doc->set_cell_content(index.row(), index.column(), new_content);

    emit dataChanged(index, index, {Qt::DisplayRole});
    for (const auto &dep : dirty) {
      auto depIndex = createIndex(dep.row(), dep.column());
      emit dataChanged(depIndex, depIndex, {Qt::DisplayRole});
    }

    return true;
  }

  return false;
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QString SpreadsheetModel::columnLabel(int col) {
  QString label;
  col++;
  while (col > 0) {
    col--;
    label.prepend(QChar('A' + (col % 26)));
    col /= 26;
  }
  return label;
}

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const {
  if (role != Qt::DisplayRole) {
    return {};
  }

  if (orientation == Qt::Horizontal) {
    return columnLabel(section);
  }

  return section + 1;
}

void SpreadsheetModel::resetFromDocument() {
  beginResetModel();
  endResetModel();
}
