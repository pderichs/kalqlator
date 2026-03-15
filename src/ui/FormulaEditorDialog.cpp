// KalQlator - FormulaEditorDialog.cpp
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

#include "FormulaEditorDialog.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

FormulaEditorDialog::FormulaEditorDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Formula Editor"));
  resize(500, 400);

  createWidgets();
  createLayout();
}

void FormulaEditorDialog::createWidgets() {
  m_textEdit = new QTextEdit(this);
  m_textEdit->setPlaceholderText(tr("(+ (cell A1) 42)"));
  m_textEdit->setAcceptRichText(false);

  // Monospace font for Lisp code
  QFont monoFont("Monospace");
  monoFont.setStyleHint(QFont::Monospace);
  m_textEdit->setFont(monoFont);

  m_okButton = new QPushButton(tr("OK"), this);
  m_okButton->setDefault(true);
  connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);

  m_cancelButton = new QPushButton(tr("Cancel"), this);
  connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void FormulaEditorDialog::createLayout() {
  auto *mainLayout = new QVBoxLayout(this);

  auto *label = new QLabel(tr("Enter your Lisp formula:"), this);
  mainLayout->addWidget(label);
  mainLayout->addWidget(m_textEdit, 1);

  auto *buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_okButton);
  buttonLayout->addWidget(m_cancelButton);

  mainLayout->addLayout(buttonLayout);
}

void FormulaEditorDialog::setFormula(const QString &formula) {
  m_textEdit->setPlainText(formula);
}

QString FormulaEditorDialog::formula() const {
  return m_textEdit->toPlainText();
}