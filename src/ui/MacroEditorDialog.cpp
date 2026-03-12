#include "MacroEditorDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFont>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "../events/MacroEditorErrorEvent.h"
#include "../events/MacroErrorEvent.h"
#include "../lisp/Evaluator.h"
#include "../lisp/tools.h"
#include "../messagebus/event_dispatcher.h"
#include "../model/triggers.h"


MacroEditorDialog::MacroEditorDialog(MacroMap *macros, QWidget *parent) : QDialog(parent), m_macros(macros) {
    setupUi();

    setWindowTitle(tr("Macro Editor"));

    // Allow the user to resize the dialog freely in both directions.
    setSizeGripEnabled(true);
    resize(640, 480); // sensible default; user can drag it larger

    connect(m_triggerCombo, &QComboBox::currentTextChanged,
            this, &MacroEditorDialog::onTriggerChanged);
}

void MacroEditorDialog::accept() {
    const auto &code = m_editor->toPlainText().toStdString();

    // Quick check
    try {
        const auto& parsed = lisp::parse_all_string(code);
        const auto& test_env = std::make_shared<TableLispEnvironment>();
        lisp::Evaluator evaluator(test_env, {});
        evaluator.evaluate(parsed);
    } catch (const std::runtime_error &e) {
        EventDispatcher::dispatch("ui:macro_editor_error", MacroEditorErrorEvent{m_previousTrigger, code, e.what()});

        // Do not accept dialog.
        return;
    }

    m_macros->insert_or_assign(m_previousTrigger, code);

    QDialog::accept();
}

void MacroEditorDialog::setupUi() {
    QFont editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    editorFont.setPointSize(11);

    auto *triggerLabel = new QLabel(tr("Trigger:"), this);

    m_triggerCombo = new QComboBox(this);
    m_triggerCombo->addItem(tr(Trigger_OnLoad));
    // (Additional triggers can be added here)
    m_triggerCombo->setCurrentIndex(0); // first item pre-selected

    m_previousTrigger = Trigger_OnLoad;

    // Align label + combo to the right
    auto *topBar = new QHBoxLayout;
    topBar->addStretch();
    topBar->addWidget(triggerLabel);
    topBar->addWidget(m_triggerCombo);

    m_editor = new QPlainTextEdit(this);
    m_editor->setFont(editorFont);
    m_editor->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_editor->setTabStopDistance(
        QFontMetricsF(editorFont).horizontalAdvance(' ') * 4);
    m_editor->setPlaceholderText(tr("; Enter macro code here"));
    if (m_macros->contains(Trigger_OnLoad)) {
        m_editor->setPlainText(QString::fromStdString(m_macros->at(Trigger_OnLoad)));
    }

    // The editor should grab all extra vertical space when the dialog grows.
    m_editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, this);

    //connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &MacroEditorDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topBar);
    mainLayout->addWidget(m_editor, /*stretch=*/1);
    mainLayout->addWidget(m_buttonBox);

    setLayout(mainLayout);
}

void MacroEditorDialog::onTriggerChanged(const QString &trigger) {
    // Save current state in map
    m_macros->insert_or_assign(m_previousTrigger, m_editor->toPlainText().toStdString());

    std::string s_trig = trigger.toStdString();
    if (m_macros->contains(s_trig)) {
        m_editor->setPlainText(QString::fromStdString(m_macros->at(s_trig)));
    } else {
        m_editor->setPlainText("");
    }

    m_previousTrigger = trigger.toStdString();
}
