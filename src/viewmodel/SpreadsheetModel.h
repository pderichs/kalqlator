#pragma once

#include <QAbstractTableModel>
#include "../model/Document.h"

class SpreadsheetModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit SpreadsheetModel(DocumentPtr doc, QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] int columnCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    void resetFromDocument();

private:
    static QString columnLabel(int col);

private:
    DocumentPtr m_doc;
};