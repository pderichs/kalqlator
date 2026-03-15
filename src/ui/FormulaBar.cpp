// KalQlator - FormulaBar.cpp
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

#include "FormulaBar.h"
#include "FormulaEditorDialog.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "user_interface_tools.h"

FormulaBar::FormulaBar(QWidget *parent) : QWidget(parent) {
  createWidgets();
  createLayout();
}

void FormulaBar::createWidgets() {
  // Cell reference display (read-only, shows current cell like "A1")
  m_cellRefLabel = new QLineEdit(this);
  m_cellRefLabel->setReadOnly(true);
  m_cellRefLabel->setFixedWidth(60);
  m_cellRefLabel->setAlignment(Qt::AlignCenter);
  m_cellRefLabel->setStyleSheet("background-color: #f0f0f0;");

  // Formula input field
  m_formulaEdit = new QLineEdit(this);
  m_formulaEdit->setPlaceholderText(tr("Enter formula, e.g. (+ A1 B1)"));

  connect(m_formulaEdit, &QLineEdit::textChanged, this,
          &FormulaBar::formulaChanged);
  connect(m_formulaEdit, &QLineEdit::editingFinished, this,
          &FormulaBar::formulaEditingFinished);

  // Expand button - opens multi-line editor dialog
  m_expandButton = new QPushButton(tr("..."), this);
  m_expandButton->setToolTip(tr("Open formula editor"));
  m_expandButton->setFixedWidth(30);
  connect(m_expandButton, &QPushButton::clicked, this,
          &FormulaBar::onExpandClicked);

  // Confirm button (checkmark)
  m_confirmButton = new QPushButton(tr("✓"), this);
  m_confirmButton->setToolTip(tr("Confirm formula"));
  m_confirmButton->setFixedWidth(30);
  connect(m_confirmButton, &QPushButton::clicked, this,
          &FormulaBar::onConfirmClicked);

  // Cancel button (X)
  m_cancelButton = new QPushButton(tr("✗"), this);
  m_cancelButton->setToolTip(tr("Cancel editing"));
  m_cancelButton->setFixedWidth(30);
  connect(m_cancelButton, &QPushButton::clicked, this,
          &FormulaBar::onCancelClicked);
}

void FormulaBar::createLayout() {
  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  layout->addWidget(m_cellRefLabel);
  layout->addWidget(m_formulaEdit, 1); // stretch factor 1
  layout->addWidget(m_expandButton);
  layout->addWidget(m_confirmButton);
  layout->addWidget(m_cancelButton);
}

void FormulaBar::setFormula(const QString &formula) {
  m_originalFormula = formula;
  m_formulaEdit->setText(formula);
}

QString FormulaBar::formula() const { return m_formulaEdit->text(); }

void FormulaBar::setCellReference(const QString &ref) {
  m_cellRefLabel->setText(ref);
}

void FormulaBar::clear() {
  m_cellRefLabel->clear();
  m_formulaEdit->clear();
  m_originalFormula.clear();
}

void FormulaBar::onExpandClicked() {
  FormulaEditorDialog dialog(this);

  const std::string formula_content = m_formulaEdit->text().toStdString();
  const std::string editor_formula = make_non_function(formula_content);
  dialog.setFormula(QString::fromStdString(editor_formula));

  if (dialog.exec() == QDialog::Accepted) {
    const std::string edited_formula = dialog.formula().toStdString();
    m_formulaEdit->setText(
        QString::fromStdString(make_function(edited_formula)));
    emit formulaEditingFinished();
  }
}

void FormulaBar::onConfirmClicked() {
  emit confirmClicked();
  emit formulaEditingFinished();
}

void FormulaBar::onCancelClicked() {
  m_formulaEdit->setText(m_originalFormula);
  emit cancelClicked();
}