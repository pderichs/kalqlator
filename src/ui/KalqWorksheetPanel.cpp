// KalQlator - KalqWorksheetPanel.cpp
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

#include "KalqWorksheetPanel.h"

#include <QDebug>
#include <QFontDatabase>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "../events/SheetSelectionChangedEvent.h"
#include "../events/UISheetAddEvent.h"
#include "../events/UISheetMoveDownEvent.h"
#include "../events/UISheetMoveUpEvent.h"
#include "../events/UISheetRemoveEvent.h"
#include "../events/UISheetRenameEvent.h"
#include "../messagebus/event_dispatcher.h"

KalqWorksheetPanel::KalqWorksheetPanel(QWidget *parent) : QWidget(parent) {
  setupUi();

  connect(btn_add_, &QPushButton::clicked, this,
          &KalqWorksheetPanel::onAddClicked);
  connect(btn_remove_, &QPushButton::clicked, this,
          &KalqWorksheetPanel::onRemoveClicked);
  connect(btn_move_up_, &QPushButton::clicked, this,
          &KalqWorksheetPanel::onMoveUpClicked);
  connect(btn_move_down_, &QPushButton::clicked, this,
          &KalqWorksheetPanel::onMoveDownClicked);
  connect(btn_rename_, &QPushButton::clicked, this,
          &KalqWorksheetPanel::onRenameClicked);

  connect(list_widget_, &QListWidget::currentItemChanged, this,
          [this](QListWidgetItem *current, QListWidgetItem *) {
            if (current != nullptr) {
              EventDispatcher::dispatch(
                  SheetSelectionChangedEvent{list_widget_->row(current)});
            }
          });
}

QListWidget *KalqWorksheetPanel::listWidget() const { return list_widget_; }

void KalqWorksheetPanel::onAddClicked() {
  // std::stringstream ss;
  //
  // ss << "Table";
  // ss << document_->sheets().size() + 1;
  // document_->add_sheet(ss.str());
  EventDispatcher::dispatch(UISheetAddEvent{});
}

void KalqWorksheetPanel::onRemoveClicked() {
  EventDispatcher::dispatch(UISheetRemoveEvent{});
}

void KalqWorksheetPanel::onMoveUpClicked() {
  EventDispatcher::dispatch(UISheetMoveUpEvent{});
}

void KalqWorksheetPanel::onMoveDownClicked() {
  EventDispatcher::dispatch(UISheetMoveDownEvent{});
}

void KalqWorksheetPanel::onRenameClicked() {
  EventDispatcher::dispatch(UISheetRenameEvent{});
}

void KalqWorksheetPanel::setupUi() {
  QFont iconFont("Material Icons", 12);
  QString arrowUpward("\ue5d8");
  QString arrowDownward("\ue5db");
  QString rename("\ue3c9");
  QString add("\ue145");
  QString remove("\ue15b");

  // Buttons
  btn_add_ = new QPushButton(this);
  btn_remove_ = new QPushButton(this);
  btn_move_up_ = new QPushButton(this);
  btn_move_down_ = new QPushButton(this);
  btn_rename_ = new QPushButton(this);

  btn_add_->setFont(iconFont);
  btn_remove_->setFont(iconFont);
  btn_move_up_->setFont(iconFont);
  btn_move_down_->setFont(iconFont);
  btn_rename_->setFont(iconFont);

  btn_add_->setText(add);
  btn_remove_->setText(remove);
  btn_move_up_->setText(arrowUpward);
  btn_move_down_->setText(arrowDownward);
  btn_rename_->setText(rename);

  // TODO: Tooltips / Statusbar hints

  constexpr int button_size = 24;
  btn_add_->setFixedSize(button_size, button_size);
  btn_remove_->setFixedSize(button_size, button_size);
  btn_move_up_->setFixedSize(button_size, button_size);
  btn_move_down_->setFixedSize(button_size, button_size);
  btn_rename_->setFixedSize(button_size, button_size);

  // Toolbar
  auto *buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->setSpacing(2);
  buttonLayout->addStretch();
  buttonLayout->addWidget(btn_add_);
  buttonLayout->addWidget(btn_remove_);
  buttonLayout->addWidget(btn_move_up_);
  buttonLayout->addWidget(btn_move_down_);
  buttonLayout->addWidget(btn_rename_);

  // List
  list_widget_ = new QListWidget(this);

  // Main Layout
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(2);
  mainLayout->addLayout(buttonLayout);
  mainLayout->addWidget(list_widget_);
}
