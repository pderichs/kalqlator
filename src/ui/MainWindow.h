// KalQlator - MainWindow.h
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

#include <QMainWindow>
#include <QUndoStack>
#include <QVariant>
#include <any>
#include <memory>

#include "../model/Document.h"
#include "KalqlatorTableWidget.h"
#include "KalqSidePanel.h"
#include "KalqWorksheetPanel.h"
#include "../events/SelectedCellChangedEvent.h"
#include "../messagebus/event_sink.h"

class QTableWidget;
class QListWidget;
class QDockWidget;
class QToolBar;
class QMenu;
class QAction;
class FormulaBar;


class MainWindow : public QMainWindow, public EventSink {
    Q_OBJECT

public:
    void registerEventHandlers();

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    std::string getNewSheetName();

    void updateCellSelectionByDocument();

    void onEvent(const std::string &name, const std::any& payload) override;

protected:
    bool hasUnsavedChanges() const;

    void closeEvent(QCloseEvent *event) override;


private slots:
    void newFile();
    void openFile();
    void saveFile();
    void cut();
    void copy();
    void paste();

    void openMacroEditor();

    void about();

    // Formula bar slots
    void onCellSelectionChanged(const SelectedCellChangedEvent& cell_selection_changed_event) const;
    void onFormulaEditingFinished();
    void onFormulaCancelled();

private:
    void updateSheetSizesByDocument();
    void updateUIAfterSheetChange();

    template<typename TEvent>
    void on(const std::string& name, std::function<void(const TEvent&)> handler) {
        m_handlers[name] = [handler](const std::any& param) {
            handler(std::any_cast<TEvent>(param));
        };
    }

    void on(const std::string& name, std::function<void()> handler) {
        m_handlers[name] = [handler](const std::any&) {
            handler();
        };
    }

    void createActions();
    void createMenus();
    void createToolBar();
    void createFormulaBar();
    void createCentralWidget();
    void createDockWidget();
    void createNewDocument();
    void updateSheetsList() const;

    // Menus
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_helpMenu;

    // Toolbar
    QToolBar *m_toolBar;

    // Central widgets
    QWidget *m_centralContainer;
    FormulaBar *m_formulaBar;
    KalqlatorTableWidget *m_tableWidget;

    // Dock
    QDockWidget *m_dockWidget;
    KalqWorksheetPanel *m_worksheetPanel;
    QListWidget *m_listWidget;
    KalqSidePanel *m_sidePanel;

    bool model_cell_selection_update_;

    // Actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_exitAction;
    QAction *m_undoAction;
    QAction *m_redoAction;
    QAction *m_cutAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_toggleDockAction;
    QAction *m_aboutAction;
    QAction *m_editMacros;

    // Event handling
    using EventHandler = std::function<void(const std::any&)>;
    std::unordered_map<std::string, EventHandler> m_handlers;

    QUndoStack* m_undoStack;

    // Current document
    DocumentPtr document_;
};
