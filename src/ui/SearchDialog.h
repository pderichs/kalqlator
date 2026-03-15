// KalQlator - SearchDialog.h
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

#include "../model/search/SearchOptions.h"
#include <QDialog>

class QLineEdit;
class QComboBox;
class QCheckBox;
class QPushButton;

class SearchDialog : public QDialog {
  Q_OBJECT

public:
  explicit SearchDialog(QWidget *parent = nullptr);

  explicit SearchDialog(const SearchOptions &initialOptions,
                        QWidget *parent = nullptr);

  [[nodiscard]] SearchOptions searchOptions() const;

private slots:
  void onUseRegexToggled(bool checked);

  void onClearClicked();

private:
  void setupUi();

  void setupConnections();

  void applyOptions(const SearchOptions &options);

  QLineEdit *m_searchInput;
  QComboBox *m_scopeCombo;
  QCheckBox *m_caseSensitiveCheck;
  QCheckBox *m_useRegexCheck;
  QPushButton *m_okButton;
  QPushButton *m_cancelButton;
  QPushButton *m_clearButton;
};
