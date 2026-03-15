// KalQlator - CellChangeCommand.cpp
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

#include "CellChangeCommand.h"

CellChangeCommand::CellChangeCommand(DocumentPtr document, const int row,
                                     const int col, std::string oldValue,
                                     std::string newValue)
    : m_document(std::move(document)), m_row(row), m_col(col),
      m_oldValue(std::move(oldValue)), m_newValue(std::move(newValue)) {
  setText(QString("Zelle (%1,%2) ändern").arg(row).arg(col));
}

void CellChangeCommand::redo() {
  m_document->set_cell_content(m_row, m_col, m_newValue);
}

void CellChangeCommand::undo() {
  m_document->set_cell_content(m_row, m_col, m_oldValue);
}

bool CellChangeCommand::mergeWith(const QUndoCommand *other) {
  const auto *cmd = dynamic_cast<const CellChangeCommand *>(other);
  if (cmd == nullptr || cmd->m_row != m_row || cmd->m_col != m_col) {
    return false;
  }

  m_newValue = cmd->m_newValue;
  return true;
}
