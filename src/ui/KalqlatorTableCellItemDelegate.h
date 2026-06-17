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

#include <algorithm>

#include <QLineEdit>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTableView>

#include "TableCellTypes.h"

class KalqlatorTableCellItemDelegate : public QStyledItemDelegate {
public:
  using QStyledItemDelegate::QStyledItemDelegate;

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override {
    bool hasError = index.data(ErrorRole).toBool();
    bool wrap = index.data(WordWrapRole).toBool();

    QStyleOptionViewItem opt(option);
    if (wrap) {
      opt.features |= QStyleOptionViewItem::WrapText;
      opt.textElideMode = Qt::ElideNone;
    }

    QStyledItemDelegate::paint(painter, opt, index);

    if (hasError) {
      painter->save();
      painter->setPen(QPen(Qt::red, 2));
      painter->drawRect(option.rect.adjusted(1, 1, -1, -1));
      painter->restore();
    }
  }

  void setEditorData(QWidget *editor, const QModelIndex &index) const override {
    if (auto *lineEdit = qobject_cast<QLineEdit *>(editor)) {
      QString text = index.data(FormulaRole).toString();
      if (text.isEmpty()) {
        text = index.data(Qt::EditRole).toString();
      }
      lineEdit->setText(text);
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

  [[nodiscard]] QSize sizeHint(const QStyleOptionViewItem &option,
                               const QModelIndex &index) const override {
    QSize base = QStyledItemDelegate::sizeHint(option, index);
    if (!index.data(WordWrapRole).toBool()) {
      return base;
    }

    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    int text_width = option.rect.width() - 4;
    if (text_width <= 0) {
      const auto *table_view = qobject_cast<const QTableView *>(option.widget);
      if (table_view != nullptr) {
        text_width = table_view->columnWidth(index.column()) - 4;
      }
    }

    if (text_width <= 0) {
      return base;
    }

    const QRect wrapped_bounds = opt.fontMetrics.boundingRect(
        QRect(0, 0, text_width, 1000000), Qt::TextWordWrap, opt.text);

    return {base.width(), std::max(base.height(), wrapped_bounds.height() + 4)};
  }
};
