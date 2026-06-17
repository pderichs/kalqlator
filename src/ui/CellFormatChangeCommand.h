// KalQlator - CellFormatChangeCommand.h
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

#include <QAbstractItemModel>
#include <QPointer>
#include <QUndoCommand>
#include <vector>

#include "../model/CellFormat.h"
#include "../model/Document.h"

struct CellFormatEntry {
  int row;
  int col;
  CellFormat old_format;
  CellFormat new_format;
};

enum class CellFormatField { Old, New };

class CellFormatChangeCommand : public QUndoCommand {
public:
  CellFormatChangeCommand(DocumentPtr document,
                          std::vector<CellFormatEntry> entries,
                          QAbstractItemModel *model);

  void redo() override;
  void undo() override;

  [[nodiscard]] int id() const override { return -1; }

private:
  void apply(const std::vector<CellFormatEntry> &entries,
             CellFormatField format_field) const;
  void emitDataChanged() const;

  DocumentPtr m_document;
  std::vector<CellFormatEntry> m_entries;
  QPointer<QAbstractItemModel> m_model;
};
