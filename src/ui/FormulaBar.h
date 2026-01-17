// KalQlator - FormulaBar.h
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

class QLineEdit;
class QPushButton;

class FormulaBar : public QWidget {
    Q_OBJECT

public:
    explicit FormulaBar(QWidget *parent = nullptr);

    void setFormula(const QString &formula);
    QString formula() const;

    void setCellReference(const QString &ref);

    signals:
        void formulaChanged(const QString &formula);
    void formulaEditingFinished();
    void confirmClicked();
    void cancelClicked();

public slots:
    void clear();

private slots:
    void onExpandClicked();
    void onConfirmClicked();
    void onCancelClicked();

private:
    void createWidgets();
    void createLayout();

    QLineEdit *m_cellRefLabel;
    QLineEdit *m_formulaEdit;
    QPushButton *m_expandButton;
    QPushButton *m_confirmButton;
    QPushButton *m_cancelButton;

    QString m_originalFormula;
};
