// KalQlator - MainWindow.cpp
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

#include "MainWindow.h"

#include <fstream>

#include "FormulaBar.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDockWidget>
#include <QFileDialog>
#include <QFontDatabase>
#include <QHeaderView>
#include <QInputDialog>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QToolBar>
#include <QVBoxLayout>

#include "../events/CellChangedEvent.h"
#include "../events/CellUpdateErrorEvent.h"
#include "../events/DocumentLoadedEvent.h"
#include "../events/MacroEditorErrorEvent.h"
#include "../events/MacroErrorEvent.h"
#include "../events/ModelSheetSelectionChangedEvent.h"
#include "../events/SearchEvent.h"
#include "../events/SelectSheetAndCellEvent.h"
#include "../events/SelectionChangedEvent.h"
#include "../events/SheetListUpdateRequestEvent.h"
#include "../events/SheetSelectionChangedEvent.h"
#include "../events/TableColumnResizedEvent.h"
#include "../events/TableEnvironmentUpdateEvent.h"
#include "../events/TableRowResizedEvent.h"
#include "../events/UISheetAddEvent.h"
#include "../events/UISheetMoveDownEvent.h"
#include "../events/UISheetMoveUpEvent.h"
#include "../events/UISheetRemoveEvent.h"
#include "../events/UISheetRenameEvent.h"
#include "../file/DocumentJsonSerializer.h"
#include "../messagebus/event_dispatcher.h"
#include "../model/triggers.h"
#include "CellChangeCommand.h"
#include "MacroEditorDialog.h"

void MainWindow::registerEventHandlers() {
  on<CellChangedEvent>([this](const auto &event) {
    // User changed cell in UI
    auto old_content = document_->get_cell_raw_content(event.row, event.col);

    if (old_content == event.content) {
      return;
    }

    m_undoStack->push(new CellChangeCommand(document_, event.row, event.col,
                                            old_content, event.content));
  });

  on<SelectionChangedEvent>([this](const auto &event) {
    if (model_cell_selection_update_) {
      return;
    }
    document_->set_current_cell(event.current_cell);
    document_->set_selected_cells(event.selection);
  });

  on<SelectedCellChangedEvent>(
      [this](const auto &event) { this->onCellSelectionChanged(event); });

  on<CellUpdateErrorEvent>([this](const auto &event) {
    if (event.error_type == ERROR_CIRCREF) {
      QMessageBox::warning(
          this, "Error",
          "Circular reference detected.\n\nPlease fix your formula.");
    } else {
      QMessageBox::warning(
          this, "Error",
          "Your input contains one or more errors.\n\nPlease fix them.");
    }
  });

  on<DocumentLoadedEvent>([this](const auto &) {
    updateSheetsList();
    updateCellSelectionByDocument();
    updateSheetSizesByDocument();
    document_->run_macros_by_trigger(Trigger_OnLoad);
    document_->update_all_cells();
  });

  on<TableEnvironmentUpdateEvent>([this](const auto &event) {
    document_->refresh_cells(event.name, event.value,
                             event.dependencies_in_topological_order);
  });

  on<UISheetAddEvent>([this](const auto &) {
    int index = document_->add_next_sheet();
    document_->set_active_sheet(index);
    EventDispatcher::dispatch(SheetListUpdateRequestEvent{});
  });

  on<UISheetRemoveEvent>([this](const auto &) {
    document_->remove_current_sheet();
    EventDispatcher::dispatch(SheetListUpdateRequestEvent{});
  });

  on<UISheetMoveUpEvent>([this](const auto &) {
    document_->move_current_sheet(UP);
    EventDispatcher::dispatch(SheetListUpdateRequestEvent{});
  });

  on<UISheetMoveDownEvent>([this](const auto &) {
    document_->move_current_sheet(DOWN);
    EventDispatcher::dispatch(SheetListUpdateRequestEvent{});
  });

  on<UISheetRenameEvent>([this](const auto &) {
    const std::string new_sheet_name(getNewSheetName());
    if (new_sheet_name.empty()) {
      return;
    }
    document_->rename_current_sheet(new_sheet_name);
    EventDispatcher::dispatch(SheetListUpdateRequestEvent{});
  });

  on<SheetListUpdateRequestEvent>([this](const auto &) { updateSheetsList(); });

  on<SheetSelectionChangedEvent>([this](const auto &event) {
    auto [item_index] = event;
    document_->set_active_sheet(item_index);
    m_sheetModel->resetFromDocument();
    updateUIAfterSheetChange();
  });

  on<ModelSheetSelectionChangedEvent>(
      [this](const auto &) { this->updateUIAfterSheetChange(); });

  on<TableRowResizedEvent>([this](const auto &event) {
    document_->set_row_height(event.logical_index, event.new_size);
    document_->set_changed_flag(true);
  });

  on<TableColumnResizedEvent>([this](const auto &event) {
    document_->set_column_width(event.logical_index, event.new_size);
    document_->set_changed_flag(true);
  });

  on<SearchEvent>([this](const auto &event) {
    auto result = document_->search(event.search_options);

    m_sidePanel->clearSearchResults();

    for (const auto &item : result) {
      m_sidePanel->addSearchResult(
          QString::fromStdString(item->complete_match),
          QString::fromStdString(item->table_name),
          QString::fromStdString(item->cell.to_string()), item);
    }
  });

  on<SelectSheetAndCellEvent>([this](const auto &event) {
    document_->select_sheet_and_cell(event.table_name, event.cell_location);
    updateSheetsList();
  });

  on<MacroErrorEvent>([this](const auto &event) {
    std::stringstream message;
    message << "Error in macro: \"" << event.macro << "\".\n"
            << "Message: " << event.message << ".\n\n"
            << "Please fix it.";

    QMessageBox::warning(this, "Macro Error",
                         QString::fromStdString(message.str()));
  });

  on<MacroEditorErrorEvent>([this](const auto &event) {
    std::stringstream message;
    message << "Error in macro: \"" << event.macro << "\".\n"
            << "Message: " << event.message << ".\n\n"
            << "Please fix it.";

    QMessageBox::warning(this, "Macro Error",
                         QString::fromStdString(message.str()));
  });
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  QFontDatabase::addApplicationFont(":/fonts/MaterialIcons-Regular.ttf");
  QFontDatabase::addApplicationFont(":/fonts/MaterialSymbolsSharp.ttf");

  model_cell_selection_update_ = false;

  m_undoStack = new QUndoStack(this);
  connect(m_undoStack, &QUndoStack::indexChanged, this,
          [this](int) { m_tableWidget->viewport()->update(); });

  setWindowTitle(tr("KalQlator"));
  resize(1200, 800);

  createNewDocument();

  createActions();
  createMenus();
  createToolBar();
  createFormulaBar();
  createCentralWidget();
  createDockWidget();

  m_tableWidget->setDisabled(false);
  m_formulaBar->setCellReference("A1");

  registerEventHandlers();

  statusBar()->showMessage(tr("Ready"));

  EventDispatcher::registerSink(this);

  updateSheetsList();
  updateCellSelectionByDocument();
}

void MainWindow::createActions() {
  const QString newDocument_icon("\ue66d");
  const QString openDocument_icon("\ueaf3");
  const QString save_icon("\ue161");
  const QString copy_icon("\ue173");
  const QString cut_icon("\ue14e");
  const QString exit_icon("\ue879");
  const QString paste_icon("\ue14f");
  const QString code_icon("\uf84d");

  // File actions
  m_newAction = new QAction(tr("&New"), this);
  m_newAction->setShortcut(QKeySequence::New);
  m_newAction->setIcon(
      iconFromFont(newDocument_icon, 24, Qt::black, "Material Symbols Sharp"));
  m_newAction->setStatusTip(tr("Create a new table"));
  connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);

  m_openAction = new QAction(tr("&Open..."), this);
  m_openAction->setShortcut(QKeySequence::Open);
  m_openAction->setIcon(iconFromFont(openDocument_icon));
  m_openAction->setStatusTip(tr("Open file"));
  connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);

  m_saveAction = new QAction(tr("&Save"), this);
  m_saveAction->setShortcut(QKeySequence::Save);
  m_saveAction->setIcon(iconFromFont(save_icon, 24, Qt::black));
  m_saveAction->setStatusTip(tr("Save file"));
  connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);

  m_editMacros = new QAction(tr("Open Macro Editor..."), this);
  m_editMacros->setIcon(iconFromFont(code_icon));
  m_editMacros->setStatusTip(tr("Open the macro editor."));
  connect(m_editMacros, &QAction::triggered, this,
          &MainWindow::openMacroEditor);

  m_exitAction = new QAction(tr("&Quit"), this);
  m_exitAction->setShortcut(QKeySequence::Quit);
  m_exitAction->setIcon(iconFromFont(exit_icon));
  m_exitAction->setStatusTip(tr("Exit"));
  connect(m_exitAction, &QAction::triggered, this, &QMainWindow::close);

  // Edit actions
  m_undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
  m_undoAction->setShortcut(QKeySequence::Undo);

  m_redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
  m_redoAction->setShortcut(QKeySequence::Redo);

  m_cutAction = new QAction(tr("Cut"), this);
  m_cutAction->setShortcut(QKeySequence::Cut);
  m_cutAction->setIcon(iconFromFont(cut_icon));
  m_cutAction->setStatusTip(tr("Cut selection"));
  connect(m_cutAction, &QAction::triggered, this, &MainWindow::cut);

  m_copyAction = new QAction(tr("&Copy"), this);
  m_copyAction->setShortcut(QKeySequence::Copy);
  m_copyAction->setIcon(iconFromFont(copy_icon));
  m_copyAction->setStatusTip(tr("Copy selection"));
  connect(m_copyAction, &QAction::triggered, this, &MainWindow::copy);

  m_pasteAction = new QAction(tr("&Paste"), this);
  m_pasteAction->setShortcut(QKeySequence::Paste);
  m_pasteAction->setIcon(iconFromFont(paste_icon));
  m_pasteAction->setStatusTip(tr("Paste"));
  connect(m_pasteAction, &QAction::triggered, this, &MainWindow::paste);

  // View actions
  m_toggleDockAction = new QAction(tr("&Toggle side bar"), this);
  m_toggleDockAction->setCheckable(true);
  m_toggleDockAction->setChecked(true);
  m_toggleDockAction->setStatusTip(tr("Toggle side bar"));

  // Help actions
  m_aboutAction = new QAction(tr("About..."), this);
  m_aboutAction->setStatusTip(tr("Information about this application"));
  connect(m_aboutAction, &QAction::triggered, this, &MainWindow::about);
}

void MainWindow::createMenus() {
  // File
  m_fileMenu = menuBar()->addMenu(tr("&File"));
  m_fileMenu->addAction(m_newAction);
  m_fileMenu->addAction(m_openAction);
  m_fileMenu->addAction(m_saveAction);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_exitAction);

  // Edit
  m_editMenu = menuBar()->addMenu(tr("&Edit"));
  m_editMenu->addAction(m_undoAction);
  m_editMenu->addAction(m_redoAction);
  m_editMenu->addSeparator();
  m_editMenu->addAction(m_cutAction);
  m_editMenu->addAction(m_copyAction);
  m_editMenu->addAction(m_pasteAction);
  m_editMenu->addSeparator();
  m_editMenu->addAction(m_editMacros);

  // View
  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_viewMenu->addAction(m_toggleDockAction);

  // Help
  m_helpMenu = menuBar()->addMenu(tr("&Help"));
  m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolBar() {
  m_toolBar = addToolBar(tr("Main Toolbar"));
  m_toolBar->setMovable(true);

  m_toolBar->addAction(m_newAction);
  m_toolBar->addAction(m_openAction);
  m_toolBar->addAction(m_saveAction);
  m_toolBar->addSeparator();
  m_toolBar->addAction(m_cutAction);
  m_toolBar->addAction(m_copyAction);
  m_toolBar->addAction(m_pasteAction);
}

void MainWindow::createFormulaBar() {
  m_formulaBar = new FormulaBar(this);

  connect(m_formulaBar, &FormulaBar::formulaEditingFinished, this,
          &MainWindow::onFormulaEditingFinished);
  connect(m_formulaBar, &FormulaBar::cancelClicked, this,
          &MainWindow::onFormulaCancelled);
}

void MainWindow::createCentralWidget() {
  m_tableWidget = new KalqlatorTableWidget(this);
  m_sheetModel = std::make_shared<SpreadsheetModel>(document_, this);

  m_tableWidget->setModel(m_sheetModel.get());

  // Adjust column width
  m_tableWidget->horizontalHeader()->setDefaultSectionSize(80);
  m_tableWidget->verticalHeader()->setDefaultSectionSize(25);

  // Add selection
  m_tableWidget->setSelectionMode(QAbstractItemView::ContiguousSelection);

  // Disable until document is created
  m_tableWidget->setDisabled(true);

  // Create container widget with formula bar and table
  m_centralContainer = new QWidget(this);
  auto *layout = new QVBoxLayout(m_centralContainer);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  layout->addWidget(m_formulaBar);
  layout->addWidget(m_tableWidget, 1);

  setCentralWidget(m_centralContainer);
}

void MainWindow::createDockWidget() {
  m_dockWidget = new QDockWidget(tr("Worksheets"), this);
  m_dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
                                Qt::RightDockWidgetArea);

  // m_listWidget = new QListWidget(m_dockWidget);
  // m_dockWidget->setWidget(m_listWidget);

  m_sidePanel = new KalqSidePanel(m_dockWidget);
  m_dockWidget->setWidget(m_sidePanel);

  // Zugriff auf das WorksheetPanel wie bisher:
  m_worksheetPanel =
      m_sidePanel
          ->worksheetPanel(); // TODO: Add api to worksheet panel for this
  m_listWidget = m_worksheetPanel->listWidget();

  addDockWidget(Qt::LeftDockWidgetArea, m_dockWidget);

  // Toggle-Action mit Dock-Widget verbinden
  connect(m_toggleDockAction, &QAction::toggled, m_dockWidget,
          &QDockWidget::setVisible);
  connect(m_dockWidget, &QDockWidget::visibilityChanged, m_toggleDockAction,
          &QAction::setChecked);
}

void MainWindow::updateSheetsList() const {
  m_listWidget->clear();

  for (size_t i = 0; i < document_->sheet_count(); i++) {
    auto *const sheet = document_->sheet_by_index(i);
    m_listWidget->addItem(tr(sheet->name().c_str()));
  }

  int index = document_->get_active_sheet();
  m_listWidget->setCurrentRow(index);
}

void MainWindow::createNewDocument() {
  document_ = std::make_shared<Document>();
  document_->initialize();
  document_->update_all_cells();
}

void MainWindow::onCellSelectionChanged(
    const SelectedCellChangedEvent &cell_selection_changed_event) const {
  const auto *const cell = cell_selection_changed_event.cell;

  if (cell != nullptr) {
    m_formulaBar->setCellReference(QString::fromStdString(cell->name_));

    if (cell->contains_formula()) {
      m_formulaBar->setFormula(QString::fromStdString(cell->raw_formula_));
    } else {
      m_formulaBar->setFormula(QString::fromStdString(cell->visible_content_));
    }
  } else {
    m_formulaBar->setFormula(QString());
  }
}

void MainWindow::onFormulaEditingFinished() {
  QModelIndex current = m_tableWidget->currentIndex();
  int row = current.row();
  int col = current.column();

  if (row < 0 || col < 0) {
    return;
  }

  const QString formula = m_formulaBar->formula();
  m_sheetModel->setData(current, formula, Qt::EditRole);

  statusBar()->showMessage(
      tr("Formula applied to %1")
          .arg(QString::fromStdString(get_cell_name_by_coordinates(col, row)),
               2000));
}

void MainWindow::onFormulaCancelled() {
  statusBar()->showMessage(tr("Formula editing cancelled"), 2000);
}

void MainWindow::updateSheetSizesByDocument() {
  for (const auto &item : document_->sheet_row_heights()) {
    m_tableWidget->setRowHeight(item.first, item.second);
  }

  for (const auto &item : document_->sheet_column_widths()) {
    m_tableWidget->setColumnWidth(item.first, item.second);
  }
}

void MainWindow::newFile() {
  // m_tableWidget->clearAndResetSizes();
  m_formulaBar->clear();
  document_->clear();
  updateSheetsList();
  updateCellSelectionByDocument();
  statusBar()->showMessage(tr("Created a new table"), 2000);
}

void MainWindow::openFile() {
  if (hasUnsavedChanges()) {
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Open file"),
        tr("The current workbook contains unsaved changes.\n"
           "These changes will be lost if a new file is opened.\n"
           "Do you want to cancel the open operation?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
      return;
    }
  }

  // m_tableWidget->clearAndResetSizes();

  QString fileName = QFileDialog::getOpenFileName(
      this, tr("Open file"), QDir::homePath(),
      tr("KalQlator files (*.kalq);; All files (*)"));

  if (fileName.isEmpty()) {
    return;
  }

  DocumentJsonSerializer serializer(document_, fileName.toStdString());

  if (!serializer.open()) {
    QMessageBox::critical(this, tr("Error opening the file"),
                          tr("Load file operation failed."));
    return;
  }

  m_sheetModel->resetFromDocument();
  EventDispatcher::dispatch(DocumentLoadedEvent{});
}

void MainWindow::saveFile() {
  const QString fileName = QFileDialog::getSaveFileName(
      this, tr("Save table"), QDir::homePath() + "/Unnamed.kalq",
      tr("KalQlator files (*.kalq);; All files (*)"));

  if (fileName.isEmpty()) {
    return;
  }

  DocumentJsonSerializer serializer(document_, fileName.toStdString());
  if (!serializer.save()) {
    QMessageBox::critical(this, tr("Error saving file"),
                          tr("Save operation failed."));
    return;
  }

  document_->set_file_name(fileName.toStdString());
  document_->set_changed_flag(false);
}

void MainWindow::cut() {
  copy();

  QItemSelectionModel *selModel = m_tableWidget->selectionModel();
  QModelIndexList selected = selModel->selectedIndexes();

  for (auto item : selected) {
    document_->set_cell_content(item.row(), item.column(), "");
  }

  statusBar()->showMessage(tr("Successfully cut content."), 2000);
}

void MainWindow::copy() const {
  QItemSelectionModel *selModel = m_tableWidget->selectionModel();
  QModelIndexList selected = selModel->selectedIndexes();

  if (selected.isEmpty()) {
    return;
  }

  // Find bounding box of selection
  int minRow = INT_MAX;
  int maxRow = 0;
  int minCol = INT_MAX;
  int maxCol = 0;

  for (const auto item : selected) {
    minRow = std::min(minRow, item.row());
    maxRow = std::max(maxRow, item.row());
    minCol = std::min(minCol, item.column());
    maxCol = std::max(maxCol, item.column());
  }

  QString result;
  for (int row = minRow; row <= maxRow; ++row) {
    for (int col = minCol; col <= maxCol; ++col) {
      const auto index = m_sheetModel->index(row, col);
      QVariant value = m_sheetModel->data(index, Qt::DisplayRole);
      result += value.toString();

      if (col < maxCol) {
        result += "\t";
      }
    }

    if (row < maxRow) {
      result += "\n";
    }
  }

  QApplication::clipboard()->setText(result);

  statusBar()->showMessage(tr("Successfully copied cells."), 2000);
}

void MainWindow::paste() const {
  QString text = QApplication::clipboard()->text();
  if (text.isEmpty()) {
    return;
  }

  QModelIndex current = m_tableWidget->currentIndex();
  if (!current.isValid()) {
    return;
  }

  int startRow = current.row();
  int startCol = current.column();

  QStringList rows = text.split("\n");
  for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
    QStringList cols = rows[rowIndex].split("\t");
    for (int colIndex = 0; colIndex < cols.size(); ++colIndex) {
      const int targetRow = startRow + rowIndex;
      const int targetCol = startCol + colIndex;

      QModelIndex index = m_sheetModel->index(targetRow, targetCol);
      if (!index.isValid()) {
        continue;
      }

      m_sheetModel->setData(index, cols[colIndex], Qt::EditRole);
    }
  }

  statusBar()->showMessage(tr("Successfully pasted content."), 2000);
}

void MainWindow::openMacroEditor() {
  MacroMap map = document_->macro_map(); // copy intentional
  MacroEditorDialog dlg(&map, this);

  if (dlg.exec() == QDialog::Accepted) {
    document_->set_macro_map(map);
    document_->set_changed_flag(true);
  }
}

void MainWindow::about() {
  QMessageBox::about(this, tr("About KalQlator"),
                     tr("<h2>KalQlator 0.1</h2>"
                        "<p>A simple spreadsheet application with a lisp like "
                        "language for formulas.</p>"));
}

std::string MainWindow::getNewSheetName() {
  QListWidgetItem *item = m_listWidget->currentItem();
  if (item == nullptr) {
    return "";
  }

  bool dialog_result;
  QString newName = QInputDialog::getText(this,
                                          tr("Rename Sheet"), // title
                                          tr("New name:"),    // label
                                          QLineEdit::Normal,
                                          item->text(), // old name
                                          &dialog_result);

  return newName.toStdString();
}

void MainWindow::updateCellSelectionByDocument() {
  Location current_selected_cell = document_->get_current_selected_cell();
  LocationSet selected_cells = document_->get_selected_cells();

  // FlagScope scope(&model_cell_selection_update_);
  // m_tableWidget->blockSignals(true);
  m_tableWidget->setSelectedCells(selected_cells, current_selected_cell);
  // m_tableWidget->blockSignals(false);
}

void MainWindow::onEvent(const std::string &name, const std::any &param) {
  if (const auto iterator = m_handlers.find(name);
      iterator != m_handlers.end()) {
    iterator->second(param);
  } else {
    // log unknown flag
    qDebug() << "Attention. No handler for: " << QString::fromStdString(name);
  }
}

bool MainWindow::hasUnsavedChanges() const { return document_->changed(); }

void MainWindow::closeEvent(QCloseEvent *event) {
  if (hasUnsavedChanges()) {
    const auto result = QMessageBox::question(
        this, tr("Quit Application"),
        tr("The current document has unsaved changes.\n\nDo you want to "
           "save the changes before closing the application?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (result == QMessageBox::Cancel) {
      event->ignore();
    } else {
      if (result == QMessageBox::Yes) {
        saveFile();
      }

      event->accept();
    }
  } else {
    event->accept();
  }
}

void MainWindow::updateUIAfterSheetChange() {
  // m_tableWidget->blockSignals(true);
  // m_tableWidget->clearAndResetSizes();
  // m_tableWidget->blockSignals(false);

  document_->update_all_cells();
  updateCellSelectionByDocument();
  updateSheetSizesByDocument();

  m_tableWidget->setFocus();
}
