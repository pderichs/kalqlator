// KalQlator - KalqSidePanel.h
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

#include <QWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include <QPushButton>
#include "KalqWorksheetPanel.h"
#include "../model/search/SearchOptions.h"

class KalqSidePanel : public QWidget {
    Q_OBJECT

public:
    explicit KalqSidePanel(QWidget *parent = nullptr);

    KalqWorksheetPanel *worksheetPanel() const;

    QTreeWidget *searchResultWidget() const;

    void clearSearchResults();

    void addSearchResult(const QString &foundString,
                         const QString &sheet,
                         const QString &cell,
                         const SearchResultItemPtr& result_item);

private slots:
    void onSearchClicked();

    void onItemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void setupUi();

    QTabWidget *tab_widget_ = nullptr;
    KalqWorksheetPanel *worksheet_panel_ = nullptr;

    // Search tab
    QWidget *search_tab_ = nullptr;
    QPushButton *btn_search_ = nullptr;
    QTreeWidget *search_result_ = nullptr;

    std::optional<SearchOptions> search_options_;
};
