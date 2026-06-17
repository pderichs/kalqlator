// KalQlator - CellFormatDialog.cpp
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

#include "CellFormatDialog.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "../model/CellDisplayFormatter.h"

CellFormatDialog::CellFormatDialog(const CellFormat &initial_format,
                                   QWidget *parent)
    : QDialog(parent) {
  setupUi();

  m_specifierEdit->setText(QString::fromStdString(initial_format.specifier));
  m_roundingCombo->setCurrentIndex(
      static_cast<int>(initial_format.rounding_mode));
  m_wordWrapCheck->setChecked(initial_format.word_wrap);

  updatePreview();
}

void CellFormatDialog::setupUi() {
  setWindowTitle(tr("Cell Format"));
  setMinimumWidth(400);

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(12);
  mainLayout->setContentsMargins(12, 12, 12, 12);

  auto *formatGroup = new QGroupBox(tr("Number Format"), this);
  auto *formatLayout = new QFormLayout(formatGroup);

  m_specifierEdit = new QLineEdit(this);
  m_specifierEdit->setPlaceholderText(tr("e.g. ###.## or ####.## EUR"));
  formatLayout->addRow(tr("Format specifier:"), m_specifierEdit);

  m_roundingCombo = new QComboBox(this);
  m_roundingCombo->addItem(tr("Nearest"),
                           static_cast<int>(RoundingMode::Nearest));
  m_roundingCombo->addItem(tr("Floor"), static_cast<int>(RoundingMode::Floor));
  m_roundingCombo->addItem(tr("Ceil"), static_cast<int>(RoundingMode::Ceil));
  m_roundingCombo->addItem(tr("Truncate"),
                           static_cast<int>(RoundingMode::Truncate));
  formatLayout->addRow(tr("Rounding mode:"), m_roundingCombo);

  mainLayout->addWidget(formatGroup);

  auto *wrapGroup = new QGroupBox(tr("Text Wrapping"), this);
  auto *wrapLayout = new QVBoxLayout(wrapGroup);

  m_wordWrapCheck = new QCheckBox(tr("Wrap text"), this);
  m_wordWrapCheck->setToolTip(tr("Wrap text at cell width boundary"));
  wrapLayout->addWidget(m_wordWrapCheck);

  mainLayout->addWidget(wrapGroup);

  auto *previewGroup = new QGroupBox(tr("Preview"), this);
  auto *previewLayout = new QVBoxLayout(previewGroup);

  m_previewLabel = new QLabel(tr("(no value)"), this);
  m_previewLabel->setWordWrap(true);
  previewLayout->addWidget(m_previewLabel);

  mainLayout->addWidget(previewGroup);

  auto *buttonBox = new QDialogButtonBox(this);
  auto *clearButton =
      buttonBox->addButton(tr("Clear Format"), QDialogButtonBox::ResetRole);
  buttonBox->addButton(QDialogButtonBox::Ok);
  buttonBox->addButton(QDialogButtonBox::Cancel);

  mainLayout->addWidget(buttonBox);

  connect(m_specifierEdit, &QLineEdit::textChanged, this,
          &CellFormatDialog::onSpecifierChanged);
  connect(m_roundingCombo, qOverload<int>(&QComboBox::currentIndexChanged),
          this, [this](int /*index*/) { updatePreview(); });
  connect(m_wordWrapCheck, &QCheckBox::checkStateChanged, this,
          [this](Qt::CheckState /*state*/) { updatePreview(); });
  connect(clearButton, &QPushButton::clicked, this,
          &CellFormatDialog::onClearClicked);
  connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
    if (!m_specifierEdit->text().isEmpty() &&
        !CellDisplayFormatter::is_valid_specifier(
            m_specifierEdit->text().toStdString())) {
      QMessageBox::warning(this, tr("Invalid Format"),
                           tr("The format specifier is invalid.\n"
                              "Use a mask like ###.## or ####.## EUR."));
      return;
    }
    accept();
  });
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void CellFormatDialog::onSpecifierChanged(const QString &text) {
  Q_UNUSED(text);
  updatePreview();
}

void CellFormatDialog::onClearClicked() {
  m_specifierEdit->clear();
  m_roundingCombo->setCurrentIndex(0);
  m_wordWrapCheck->setChecked(false);
  updatePreview();
}

void CellFormatDialog::updatePreview() {
  const CellFormat fmt = selectedFormat();

  if (!fmt.has_specifier()) {
    m_previewLabel->setText(tr("(no format applied)"));
    m_previewLabel->setStyleSheet(QString());
    return;
  }

  if (!CellDisplayFormatter::is_valid_specifier(fmt.specifier)) {
    m_previewLabel->setText(tr("(invalid specifier)"));
    m_previewLabel->setStyleSheet(QString());
    return;
  }

  m_previewLabel->setStyleSheet(QString());

  const std::string formatted = CellDisplayFormatter::format("1234.5678", fmt);
  m_previewLabel->setText(QString::fromStdString("1234.5678 -> " + formatted));
}

CellFormat CellFormatDialog::selectedFormat() const {
  CellFormat fmt;
  fmt.specifier = m_specifierEdit->text().toStdString();
  fmt.rounding_mode =
      static_cast<RoundingMode>(m_roundingCombo->currentData().toInt());
  fmt.word_wrap = m_wordWrapCheck->isChecked();
  return fmt;
}
