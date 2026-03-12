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

#include <QFontDatabase>
#include <QTableWidget>
#include <QListWidget>
#include <QDockWidget>
#include <QToolBar>
#include <QMenuBar>
#include <QAction>
#include <QApplication>
#include <QStatusBar>
#include <QMessageBox>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDir>
#include <QInputDialog>
#include <QCloseEvent>
#include <QClipboard>

#include "CellChangeCommand.h"
#include "MacroEditorDialog.h"
#include "../events/CellChangedEvent.h"
#include "../file/DocumentJsonSerializer.h"
#include "../events/DocumentLoadedEvent.h"
#include "../events/SearchEvent.h"
#include "../events/CellUpdateErrorEvent.h"
#include "../events/SearchResultEvent.h"
#include "../events/SelectionChangedEvent.h"
#include "../events/SelectSheetAndCellEvent.h"
#include "../events/SheetSelectionChangedEvent.h"
#include "../events/TableEnvironmentUpdateEvent.h"
#include "../events/TablePropertyResizedEvent.h"
#include "../events/MacroErrorEvent.h"
#include "../events/MacroEditorErrorEvent.h"
#include "../messagebus/event_dispatcher.h"
#include "../tools/FlagScope.h"
#include "../model/triggers.h"

void MainWindow::registerEventHandlers() {
    on<CellChangedEvent>("ui:cell_changed", [this](const auto &event) {
        // User changed cell in UI
        auto old_content = document_->get_cell_raw_content(event.row, event.col);

        if (old_content == event.content) return;

        m_undoStack->push(
            new CellChangeCommand(document_, event.row, event.col, old_content, event.content)
        );
    });

    on<SelectionChangedEvent>("ui:cell_selection_changed", [this](const auto &event) {
        if (model_cell_selection_update_) {
            return;
        }
        document_->set_current_cell(event.current_cell);
        document_->set_selected_cells(event.selection);
    });

    on<SelectedCellChangedEvent>("model:selected_cell_changed", [this](const auto &event) {
        this->onCellSelectionChanged(event);
    });

    on<CellUpdateErrorEvent>("model:cell_update_error", [this](const auto &event) {
        if (event.error_type == ERROR_CIRCREF) {
            QMessageBox::warning(this, "Error", "Circular reference detected.\n\nPlease fix your formula.");
        } else {
            QMessageBox::warning(this, "Error", "Your input contains one or more errors.\n\nPlease fix them.");
        }
    });

    on<CellUpdateDoneEvent>("model:cell_update_done", [this](const auto &event) {
        const auto &cell = event.cell;
        m_tableWidget->update_cell(cell);
    });

    on<DocumentLoadedEvent>("document_loaded", [this](const auto &) {
        updateSheetsList();
        updateCellSelectionByDocument();
        updateSheetSizesByDocument();
        document_->run_macros_by_trigger(Trigger_OnLoad);
        document_->update_all_cells();
    });

    on<TableEnvironmentUpdateEvent>("model:table_environment_update", [this](const auto &event) {
        document_->refresh_cells(event.name, event.value, event.dependencies_in_topological_order);
    });

    on("ui:sheet_add", [this]() {
        int index = document_->add_next_sheet();
        document_->set_active_sheet(index);
        EventDispatcher::dispatch("ui:sheet_list_update");
    });

    on("ui:sheet_remove", [this]() {
        document_->remove_current_sheet();
        EventDispatcher::dispatch("ui:sheet_list_update");
    });

    on("ui:sheet_move_up", [this]() {
        document_->move_current_sheet(UP);
        EventDispatcher::dispatch("ui:sheet_list_update");
    });

    on("ui:sheet_move_down", [this]() {
        document_->move_current_sheet(DOWN);
        EventDispatcher::dispatch("ui:sheet_list_update");
    });

    on("ui:sheet_rename", [this]() {
        const std::string s(getNewSheetName());
        if (s.empty()) {
            return;
        }
        document_->rename_current_sheet(s);
        EventDispatcher::dispatch("ui:sheet_list_update");
    });

    on("ui:sheet_list_update", [this]() {
        updateSheetsList();
    });

    on<SheetSelectionChangedEvent>("ui:sheet_selection_changed", [this](const auto &event) {
        auto [item_index] = event;
        document_->set_active_sheet(item_index);

        updateUIAfterSheetChange();
    });

    on("model:sheet_selection_changed", [this]() {
        this->updateUIAfterSheetChange();
    });


    on<TablePropertyResizedEvent>("ui:column_resized", [this](const auto &event) {
        document_->set_row_height(event.logical_index, event.new_size);
        document_->set_changed_flag(true);
    });

    on<TablePropertyResizedEvent>("ui:row_resized", [this](const auto &event) {
        document_->set_column_width(event.logical_index, event.new_size);
        document_->set_changed_flag(true);
    });

    on<SearchEvent>("ui:search", [this](const auto &event) {
        auto result = document_->search(event.search_options);

        m_sidePanel->clearSearchResults();


        for (auto item: result) {
            std::stringstream ss;
            ss << item->cell;

            m_sidePanel->addSearchResult(
                QString::fromStdString(item->complete_match),
                QString::fromStdString(item->table_name),
                QString::fromStdString(ss.str()),
                item);
        }
    });

    on<SelectSheetAndCellEvent>("ui:select_sheet_and_cell", [this](const auto &event) {
        document_->select_sheet_and_cell(event.table_name, event.cell_location);
        updateSheetsList();
    });

    on<MacroErrorEvent>("model:macro-error", [this](const auto &event) {
        std::stringstream ss;
        ss << "Error in macro: \"" << event.macro << "\".\n"
        << "Message: " << event.message << ".\n\n"
        << "Please fix it.";

        QMessageBox::warning(this, "Macro Error", QString::fromStdString(ss.str()));
    });

    on<MacroEditorErrorEvent>("ui:macro_editor_error", [this](const auto &event) {
        std::stringstream ss;
        ss << "Error in macro: \"" << event.macro << "\".\n"
        << "Message: " << event.message << ".\n\n"
        << "Please fix it.";

        QMessageBox::warning(this, "Macro Error", QString::fromStdString(ss.str()));
    });
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    QFontDatabase::addApplicationFont(":/fonts/MaterialIcons-Regular.ttf");

    model_cell_selection_update_ = false;

    m_undoStack = new QUndoStack(this);

    setWindowTitle(tr("KalQlator"));
    resize(1200, 800);

    createActions();
    createMenus();
    createToolBar();
    createFormulaBar();
    createCentralWidget();
    createDockWidget();
    createNewDocument();

    registerEventHandlers();

    statusBar()->showMessage(tr("Ready"));

    EventDispatcher::registerSink(this);

    updateSheetsList();
    updateCellSelectionByDocument();
}

void MainWindow::createActions() {
    const QString newDocument("\ue89c");
    const QString openDocument("\ueaf3");
    const QString copy("\ue173");
    const QString cut("\ue14e");
    const QString exit("\ue879");
    const QString paste("\ue14f");
    const QString code("\uf84d");

    // File actions
    m_newAction = new QAction(tr("&New"), this);
    m_newAction->setShortcut(QKeySequence::New);
    m_newAction->setIcon(iconFromFont(newDocument));
    m_newAction->setStatusTip(tr("Create a new table"));
    connect(m_newAction, &QAction::triggered, this, &MainWindow::newFile);

    m_openAction = new QAction(tr("&Open..."), this);
    m_openAction->setShortcut(QKeySequence::Open);
    m_openAction->setIcon(iconFromFont(openDocument));
    m_openAction->setStatusTip(tr("Open file"));
    connect(m_openAction, &QAction::triggered, this, &MainWindow::openFile);

    m_saveAction = new QAction(tr("&Save"), this);
    m_saveAction->setShortcut(QKeySequence::Save);
    m_saveAction->setIcon(iconFromFont(newDocument, 24, Qt::black));
    m_saveAction->setStatusTip(tr("Save file"));
    connect(m_saveAction, &QAction::triggered, this, &MainWindow::saveFile);

    m_editMacros = new QAction(tr("Open Macro Editor..."), this);
    m_editMacros->setIcon(iconFromFont(code));
    m_editMacros->setStatusTip(tr("Open the macro editor."));
    connect(m_editMacros, &QAction::triggered, this, &MainWindow::openMacroEditor);

    m_exitAction = new QAction(tr("&Quit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setIcon(iconFromFont(exit));
    m_exitAction->setStatusTip(tr("Exit"));
    connect(m_exitAction, &QAction::triggered, this, &QMainWindow::close);

    // Edit actions
    m_undoAction = m_undoStack->createUndoAction(this, tr("Undo"));
    m_undoAction->setShortcut(QKeySequence::Undo);

    m_redoAction = m_undoStack->createRedoAction(this, tr("Redo"));
    m_redoAction->setShortcut(QKeySequence::Redo);

    m_cutAction = new QAction(tr("Cut"), this);
    m_cutAction->setShortcut(QKeySequence::Cut);
    m_cutAction->setIcon(iconFromFont(cut));
    m_cutAction->setStatusTip(tr("Cut selection"));
    connect(m_cutAction, &QAction::triggered, this, &MainWindow::cut);

    m_copyAction = new QAction(tr("&Copy"), this);
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_copyAction->setIcon(iconFromFont(copy));
    m_copyAction->setStatusTip(tr("Copy selection"));
    connect(m_copyAction, &QAction::triggered, this, &MainWindow::copy);

    m_pasteAction = new QAction(tr("&Paste"), this);
    m_pasteAction->setShortcut(QKeySequence::Paste);
    m_pasteAction->setIcon(iconFromFont(paste));
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

    connect(m_formulaBar, &FormulaBar::formulaEditingFinished,
            this, &MainWindow::onFormulaEditingFinished);
    connect(m_formulaBar, &FormulaBar::cancelClicked,
            this, &MainWindow::onFormulaCancelled);
}

void MainWindow::createCentralWidget() {
    m_tableWidget = new KalqlatorTableWidget(100, 26, this);

    // Column headers
    QStringList headers;
    for (char c = 'A'; c <= 'Z'; ++c) {
        headers << QString(c);
    }
    m_tableWidget->setHorizontalHeaderLabels(headers);

    // Row headers
    QStringList rowHeaders;
    for (int i = 1; i <= 100; ++i) {
        rowHeaders << QString::number(i);
    }
    m_tableWidget->setVerticalHeaderLabels(rowHeaders);

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
    m_dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    // m_listWidget = new QListWidget(m_dockWidget);
    // m_dockWidget->setWidget(m_listWidget);

    m_sidePanel = new KalqSidePanel(m_dockWidget);
    m_dockWidget->setWidget(m_sidePanel);

    // Zugriff auf das WorksheetPanel wie bisher:
    m_worksheetPanel = m_sidePanel->worksheetPanel(); // TODO: Add api to worksheet panel for this
    m_listWidget = m_worksheetPanel->listWidget();

    addDockWidget(Qt::LeftDockWidgetArea, m_dockWidget);

    // Toggle-Action mit Dock-Widget verbinden
    connect(m_toggleDockAction, &QAction::toggled,
            m_dockWidget, &QDockWidget::setVisible);
    connect(m_dockWidget, &QDockWidget::visibilityChanged,
            m_toggleDockAction, &QAction::setChecked);
}

void MainWindow::updateSheetsList() const {
    m_listWidget->clear();

    for (const auto &sheet: document_->sheets()) {
        m_listWidget->addItem(tr(sheet->name().c_str()));
    }

    int index = document_->get_active_sheet();
    m_listWidget->setCurrentRow(index);
}

void MainWindow::createNewDocument() {
    document_ = std::make_shared<Document>();
    document_->initialize();
    m_tableWidget->setDisabled(false);
    document_->update_all_cells(); // hack?
}

void MainWindow::onCellSelectionChanged(const SelectedCellChangedEvent &cell_selection_changed_event) const {
    const auto &cell = cell_selection_changed_event.cell;

    if (cell) {
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
    const int row = m_tableWidget->currentRow();
    const int col = m_tableWidget->currentColumn();

    if (row < 0 || col < 0) {
        return;
    }

    m_tableWidget->query_item_or_create(row, col); // make sure item exists
    const QString formula = m_formulaBar->formula();

    document_->set_cell_content(row, col, formula.toStdString());

    statusBar()->showMessage(
        tr("Formula applied to %1").arg(QString::fromStdString(get_cell_name_by_coordinates(col, row)), 2000));
}

void MainWindow::onFormulaCancelled() {
    statusBar()->showMessage(tr("Formula editing cancelled"), 2000);
}

void MainWindow::updateSheetSizesByDocument() {
    for (const auto &item: document_->sheet_row_heights()) {
        m_tableWidget->setRowHeight(item.first, item.second);
    }

    for (const auto &item: document_->sheet_column_widths()) {
        m_tableWidget->setColumnWidth(item.first, item.second);
    }
}

void MainWindow::newFile() {
    m_tableWidget->clearAndResetSizes();
    m_formulaBar->clear();
    document_->clear();
    updateSheetsList();
    updateCellSelectionByDocument();
    statusBar()->showMessage(tr("Created a new table"), 2000);
}

void MainWindow::openFile() {
    if (document_->changed()) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Open file"),
            tr("The current workbook contains unsaved changes.\n"
                "These changes will be lost if a new file is opened.\n"
                "Do you want to cancel the open operation?"),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            return;
        }
    }

    m_tableWidget->clearAndResetSizes();

    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open file"),
        QDir::homePath(),
        tr("KalQlator files (*.kalq);; All files (*)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    DocumentJsonSerializer serializer(document_, fileName.toStdString(), document_->get_sheet_registry_weak_ptr());
    if (!serializer.open()) {
        QMessageBox::critical(this, tr("Error opening the file"), tr("Load file operation failed."));
        return;
    }

    EventDispatcher::dispatch("document_loaded", DocumentLoadedEvent{});
}

void MainWindow::saveFile() {
    const QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save table"),
        QDir::homePath() + "/Unnamed.kalq",
        tr("KalQlator files (*.kalq);; All files (*)")
    );

    if (fileName.isEmpty()) {
        return;
    }

    // TODO Remove - this is already handled in Qt's Filedialog
    if (QFile::exists(fileName)) {
        const QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Overwrite File confirmation"),
            tr("File '%1' already exists.\n"
                "Overwrite existing file?").arg(fileName),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::No) {
            return;
        }
    }

    DocumentJsonSerializer serializer(document_, fileName.toStdString(), document_->get_sheet_registry_weak_ptr());
    if (!serializer.save()) {
        QMessageBox::critical(this, tr("Error saving file"), tr("Save operation failed."));
        return;
    }

    document_->set_file_name(fileName.toStdString());
    document_->set_changed_flag(false);
}

void MainWindow::cut() {
    copy();
    for (auto *item: m_tableWidget->selectedItems()) {
        document_->set_cell_content(item->row(), item->column(), "");
    }

    statusBar()->showMessage(tr("Successfully cut content."), 2000);
}

void MainWindow::copy() {
    auto selected = m_tableWidget->selectedItems();
    if (selected.isEmpty()) return;

    // Find bounding box of selection
    int minRow = INT_MAX, maxRow = 0;
    int minCol = INT_MAX, maxCol = 0;
    for (const auto *item: selected) {
        minRow = std::min(minRow, item->row());
        maxRow = std::max(maxRow, item->row());
        minCol = std::min(minCol, item->column());
        maxCol = std::max(maxCol, item->column());
    }

    QString result;
    for (int row = minRow; row <= maxRow; ++row) {
        for (int col = minCol; col <= maxCol; ++col) {
            const auto *item = m_tableWidget->item(row, col);
            result += item ? item->text() : "";
            if (col < maxCol) result += "\t";
        }
        if (row < maxRow) result += "\n";
    }

    QApplication::clipboard()->setText(result);

    statusBar()->showMessage(tr("Successfully copied cells."), 2000);
}

void MainWindow::paste() {
    QString text = QApplication::clipboard()->text();
    if (text.isEmpty()) return;

    int startRow = m_tableWidget->currentRow();
    int startCol = m_tableWidget->currentColumn();

    QStringList rows = text.split("\n");
    for (int r = 0; r < rows.size(); ++r) {
        QStringList cols = rows[r].split("\t");
        for (int c = 0; c < cols.size(); ++c) {
            const int targetRow = startRow + r;
            const int targetCol = startCol + c;

            // Grow table if required
            if (targetRow >= m_tableWidget->rowCount()) {
                m_tableWidget->setRowCount(targetRow + 1);
            }

            if (targetCol >= m_tableWidget->columnCount()) {
                m_tableWidget->setColumnCount(targetCol + 1);
            }

            if (!m_tableWidget->item(targetRow, targetCol)) {
                m_tableWidget->setItem(targetRow, targetCol,
                                       new QTableWidgetItem());
            }

            document_->set_cell_content(targetRow, targetCol, cols[c].toStdString());
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
                           "<p>A simple spreadsheet application with a lisp like language for formulas.</p>"));
}

std::string MainWindow::getNewSheetName() {
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item)
        return "";

    bool ok;
    QString newName = QInputDialog::getText(
        this,
        tr("Rename Sheet"), // Titel
        tr("New name:"), // Label
        QLineEdit::Normal,
        item->text(), // old name
        &ok
    );

    return newName.toStdString();
}

void MainWindow::updateCellSelectionByDocument() {
    Location current_selected_cell = document_->get_current_selected_cell();
    LocationSet selected_cells = document_->get_selected_cells();

    FlagScope scope(&model_cell_selection_update_);
    m_tableWidget->blockSignals(true);
    m_tableWidget->setSelectedCells(selected_cells, current_selected_cell);
    m_tableWidget->blockSignals(false);
}

void MainWindow::onEvent(const std::string &name, const std::any &param) {
    if (const auto it = m_handlers.find(name); it != m_handlers.end()) {
        it->second(param);
    } else {
        // log unknown flag
        qDebug() << "Attention. No handler for: " << QString::fromStdString(name);
    }
}

bool MainWindow::hasUnsavedChanges() const {
    return document_->changed();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if (hasUnsavedChanges()) {
        const auto result = QMessageBox::question(
            this,
            tr("Quit Application?"),
            tr("The current document has unsaved changes. Do you really want to close the app and lose those changes?"),
            QMessageBox::Yes | QMessageBox::No
        );

        if (result == QMessageBox::Yes) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::updateUIAfterSheetChange() {
    m_tableWidget->blockSignals(true);
    m_tableWidget->clearAndResetSizes();
    m_tableWidget->blockSignals(false);

    document_->update_all_cells();
    updateCellSelectionByDocument();
    updateSheetSizesByDocument();

    m_tableWidget->setFocus();
}
