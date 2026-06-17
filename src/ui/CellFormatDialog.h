// KalQlator - CellFormatDialog.h
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

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>

#include "../model/CellFormat.h"

class CellFormatDialog : public QDialog {
  Q_OBJECT

public:
  explicit CellFormatDialog(const CellFormat &initial_format,
                            QWidget *parent = nullptr);

  [[nodiscard]] CellFormat selectedFormat() const;

private slots:
  void onSpecifierChanged(const QString &text);
  void onClearClicked();

private:
  void setupUi();
  void updatePreview();

  QLineEdit *m_specifierEdit;
  QComboBox *m_roundingCombo;
  QCheckBox *m_wordWrapCheck;
  QLabel *m_previewLabel;
};
