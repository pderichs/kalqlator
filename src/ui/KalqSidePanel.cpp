// KalQlator - KalqSidePanel.cpp
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

#include "KalqSidePanel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

#include "SearchDialog.h"
#include "../messagebus/event_dispatcher.h"
#include "../events/SearchEvent.h"
#include "../events/SelectSheetAndCellEvent.h"

#include "SearchResultItemMetaType.h"

KalqSidePanel::KalqSidePanel(QWidget *parent)
    : QWidget(parent) {
    setupUi();
    connect(btn_search_, &QPushButton::clicked,
            this, &KalqSidePanel::onSearchClicked);

    connect(search_result_, &QTreeWidget::itemDoubleClicked,
            this, &KalqSidePanel::onItemDoubleClicked);
}

KalqWorksheetPanel *KalqSidePanel::worksheetPanel() const {
    return worksheet_panel_;
}

QTreeWidget *KalqSidePanel::searchResultWidget() const {
    return search_result_;
}

void KalqSidePanel::clearSearchResults() {
    search_result_->clear();
}

void KalqSidePanel::addSearchResult(const QString &foundString,
                                    const QString &sheet,
                                    const QString &cell,
                                    const SearchResultItemPtr &result_item) {
    auto *item = new QTreeWidgetItem(search_result_);
    item->setText(0, foundString);
    item->setText(1, sheet);
    item->setText(2, cell);
    item->setData(0, Qt::UserRole, QVariant::fromValue(result_item));
}

void KalqSidePanel::onSearchClicked() {
    std::unique_ptr<SearchDialog> dlg;

    if (search_options_) {
        dlg = std::make_unique<SearchDialog>(*search_options_, this);
    } else {
        dlg = std::make_unique<SearchDialog>(this);
    }

    if (dlg->exec() == QDialog::Accepted) {
        auto opts = dlg->searchOptions();

        EventDispatcher::dispatch("ui:search", SearchEvent{opts});

        // Copy for next call
        search_options_ = opts;
    }
}

void KalqSidePanel::onItemDoubleClicked(QTreeWidgetItem *item, int) {
    const auto search_result = item->data(0, Qt::UserRole).value<SearchResultItemPtr>();
    if (search_result) {
        EventDispatcher::dispatch("ui:select_sheet_and_cell",
                                  SelectSheetAndCellEvent{search_result->table_name, search_result->cell});
    }
}

void KalqSidePanel::setupUi() {
    worksheet_panel_ = new KalqWorksheetPanel(this);

    search_tab_ = new QWidget(this);

    QFont iconFont("Material Icons", 12);
    btn_search_ = new QPushButton(this);
    btn_search_->setFont(iconFont);
    btn_search_->setText("\ue8b6"); // search glyph
    constexpr int button_size = 24;
    btn_search_->setFixedSize(button_size, button_size);
    btn_search_->setToolTip(tr("Search"));

    auto *searchToolbar = new QHBoxLayout;
    searchToolbar->setContentsMargins(0, 0, 0, 0);
    searchToolbar->setSpacing(2);
    searchToolbar->addStretch();
    searchToolbar->addWidget(btn_search_);

    search_result_ = new QTreeWidget(this);
    search_result_->setColumnCount(3);
    search_result_->setHeaderLabels({tr("Found String"), tr("Sheet"), tr("Cell")});
    search_result_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    search_result_->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    search_result_->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    search_result_->setRootIsDecorated(false);
    search_result_->setAlternatingRowColors(true);

    auto *searchLayout = new QVBoxLayout(search_tab_);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchLayout->setSpacing(2);
    searchLayout->addLayout(searchToolbar);
    searchLayout->addWidget(search_result_);

    tab_widget_ = new QTabWidget(this);
    tab_widget_->addTab(worksheet_panel_, tr("Sheets"));
    tab_widget_->addTab(search_tab_, tr("Search"));

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(tab_widget_);
}
