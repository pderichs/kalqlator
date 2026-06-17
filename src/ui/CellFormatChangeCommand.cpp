// KalQlator - CellFormatChangeCommand.cpp
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

#include "CellFormatChangeCommand.h"

CellFormatChangeCommand::CellFormatChangeCommand(
    DocumentPtr document, std::vector<CellFormatEntry> entries,
    QAbstractItemModel *model)
    : m_document(std::move(document)), m_entries(std::move(entries)),
      m_model(model) {
  setText(QObject::tr("Change cell format", nullptr,
                      static_cast<int>(m_entries.size())));
}

void CellFormatChangeCommand::emitDataChanged() const {
  if (m_model == nullptr) {
    return;
  }

  for (const auto &entry : m_entries) {
    QModelIndex idx = m_model->index(entry.row, entry.col);
    if (idx.isValid()) {
      emit m_model->dataChanged(idx, idx, {Qt::DisplayRole});
    }
  }
}

void CellFormatChangeCommand::apply(const std::vector<CellFormatEntry> &entries,
                                    CellFormatField field) const {
  for (const auto &entry : entries) {
    const auto &fmt =
        field == CellFormatField::New ? entry.new_format : entry.old_format;
    auto *cell = m_document->get_cell(entry.row, entry.col);
    if (cell != nullptr) {
      cell->format_ = fmt;
    } else {
      m_document->set_cell_format(entry.row, entry.col, fmt);
    }
  }
}

void CellFormatChangeCommand::redo() {
  apply(m_entries, CellFormatField::New);
  m_document->set_changed_flag(true);
  emitDataChanged();
}

void CellFormatChangeCommand::undo() {
  apply(m_entries, CellFormatField::Old);
  m_document->set_changed_flag(true);
  emitDataChanged();
}
