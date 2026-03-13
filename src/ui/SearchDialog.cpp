// KalQlator - SearchDialog.cpp
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

#include "SearchDialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

SearchDialog::SearchDialog(QWidget *parent)
    : QDialog(parent) {
    setupUi();
    setupConnections();
}

SearchDialog::SearchDialog(const SearchOptions &initialOptions, QWidget *parent)
    : QDialog(parent) {
    setupUi();
    setupConnections();
    applyOptions(initialOptions);
}

void SearchDialog::setupUi() {
    setWindowTitle(tr("Search"));
    setMinimumWidth(350);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // --- Search input ---
    auto *searchGroup = new QGroupBox(tr("Search"), this);
    auto *searchLayout = new QFormLayout(searchGroup);

    m_searchInput = new QLineEdit(this);
    m_searchInput->setPlaceholderText(tr("Enter search string..."));
    searchLayout->addRow(tr("Search for:"), m_searchInput);

    m_scopeCombo = new QComboBox(this);
    m_scopeCombo->addItem(tr("Content and Formula"), static_cast<int>(SearchScope::ContentAndFormula));
    m_scopeCombo->addItem(tr("Only Content"), static_cast<int>(SearchScope::OnlyContent));
    m_scopeCombo->addItem(tr("Only Formula"), static_cast<int>(SearchScope::OnlyFormula));
    searchLayout->addRow(tr("Search in:"), m_scopeCombo);

    mainLayout->addWidget(searchGroup);

    // --- Options ---
    auto *optionsGroup = new QGroupBox(tr("Options"), this);
    auto *optionsLayout = new QVBoxLayout(optionsGroup);

    m_caseSensitiveCheck = new QCheckBox(tr("Case sensitive"), this);
    m_useRegexCheck = new QCheckBox(tr("Use Regular expression"), this);

    optionsLayout->addWidget(m_caseSensitiveCheck);
    optionsLayout->addWidget(m_useRegexCheck);

    mainLayout->addWidget(optionsGroup);

    // --- Buttons ---
    auto *buttonBox = new QDialogButtonBox(this);
    m_clearButton = buttonBox->addButton(tr("Clear"), QDialogButtonBox::ResetRole);
    m_okButton = buttonBox->addButton(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->addButton(QDialogButtonBox::Cancel);
    m_okButton->setDefault(true);

    mainLayout->addWidget(buttonBox);
}

void SearchDialog::setupConnections() {
    connect(m_useRegexCheck, &QCheckBox::toggled,
            this, &SearchDialog::onUseRegexToggled);

    connect(m_clearButton, &QPushButton::clicked, this, &SearchDialog::onClearClicked);
    connect(m_okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SearchDialog::onUseRegexToggled(bool checked) {
    m_caseSensitiveCheck->setEnabled(!checked);
    if (checked) {
        m_caseSensitiveCheck->setChecked(false);
}
}

void SearchDialog::applyOptions(const SearchOptions &options) {
    m_searchInput->setText(QString::fromStdString(options.searchString));

    const int index = m_scopeCombo->findData(static_cast<int>(options.scope));
    if (index != -1) {
        m_scopeCombo->setCurrentIndex(index);
    }

    // Apply regex first so the toggled signal correctly enables/disables caseSensitive
    m_useRegexCheck->setChecked(options.useRegularExpression);
    m_caseSensitiveCheck->setChecked(!options.useRegularExpression && options.caseSensitive);
}

void SearchDialog::onClearClicked() {
    applyOptions(SearchOptions{
        .searchString = {},
        .scope = SearchScope::ContentAndFormula,
        .caseSensitive = false,
        .useRegularExpression = false
    });
}

SearchOptions SearchDialog::searchOptions() const {
    return SearchOptions{
        .searchString = m_searchInput->text().toStdString(),
        .scope = static_cast<SearchScope>(m_scopeCombo->currentData().toInt()),
        .caseSensitive = m_caseSensitiveCheck->isChecked(),
        .useRegularExpression = m_useRegexCheck->isChecked()
    };
}
