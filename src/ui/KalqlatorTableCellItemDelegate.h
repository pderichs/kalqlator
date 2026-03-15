// KalQlator - KalqlatorTableCellItemDelegate.h
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

#include <QLineEdit>
#include <QPainter>
#include <QStyledItemDelegate>

#include "TableCellTypes.h"

class KalqlatorTableCellItemDelegate : public QStyledItemDelegate {
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    QStyledItemDelegate::paint(painter, option, index);

    bool hasError = index.data(ErrorRole).toBool();
    if (hasError) {
      painter->save();
      painter->setPen(QPen(Qt::red, 2));
      painter->drawRect(option.rect.adjusted(1, 1, -1, -1));
      painter->restore();
    }
  }

  void setEditorData(QWidget *editor, const QModelIndex &index) const override {
    if (auto *lineEdit = qobject_cast<QLineEdit *>(editor)) {
      lineEdit->setText(index.data(FormulaRole).toString());
    } else {
      QStyledItemDelegate::setEditorData(editor, index);
    }
  }

  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const override {
    if (auto *lineEdit = qobject_cast<QLineEdit *>(editor)) {
      model->setData(index, lineEdit->text(), FormulaRole);
    } else {
      QStyledItemDelegate::setModelData(editor, model, index);
    }
  }
};
