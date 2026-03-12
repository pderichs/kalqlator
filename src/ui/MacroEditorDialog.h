#pragma once

#include <QDialog>

#include "../model/Document.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QDialogButtonBox;
class QPlainTextEdit;
class QLabel;
QT_END_NAMESPACE

class MacroEditorDialog : public QDialog {
    Q_OBJECT

public:
    explicit MacroEditorDialog(MacroMap *macros, QWidget *parent = nullptr);

    MacroMap macros();

private slots:
    void accept();

private:
    void setupUi();

    void onTriggerChanged(const QString &trigger);

    QComboBox *m_triggerCombo{nullptr};
    QPlainTextEdit *m_editor{nullptr};
    QDialogButtonBox *m_buttonBox{nullptr};

    MacroMap *m_macros;
    std::string m_previousTrigger;
};
