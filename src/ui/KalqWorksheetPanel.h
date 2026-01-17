// KalQlator - KalqWorksheetPanel.h
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

#include <QListView>

#include "../model/Document.h"

class QListWidget;
class QPushButton;

class KalqWorksheetPanel : public QWidget {
    Q_OBJECT

public:
    explicit KalqWorksheetPanel(QWidget *parent);

    [[nodiscard]] QListWidget *listWidget() const;

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onMoveUpClicked();
    void onMoveDownClicked();
    void onRenameClicked();

private:
    void setupUi();

    QListWidget *list_widget_;
    QPushButton *btn_add_;
    QPushButton *btn_remove_;
    QPushButton *btn_move_up_;
    QPushButton *btn_move_down_;
    QPushButton *btn_rename_;
};
