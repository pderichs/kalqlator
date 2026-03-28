#include "SpreadsheetModel.h"

#include "../ui/TableCellTypes.h"
#include <utility>

#include "../events/CellChangedEvent.h"
#include "../messagebus/event_dispatcher.h"

SpreadsheetModel::SpreadsheetModel(DocumentPtr doc, QObject *parent)
    : QAbstractTableModel(parent), m_doc(std::move(doc)) {}

int SpreadsheetModel::rowCount(const QModelIndex & /*parent*/) const {
  return static_cast<int>(m_doc->row_count());
}

int SpreadsheetModel::columnCount(const QModelIndex & /*parent*/) const {
  return static_cast<int>(m_doc->column_count());
}

QVariant SpreadsheetModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return {};
  }

  const auto *cell = m_doc->get_cell(index.row(), index.column());

  if (cell == nullptr) {
    return {};
  }

  if (role == Qt::DisplayRole) {
    return QString::fromStdString(cell->visible_content_);
  }

  if (role == Qt::EditRole) {
    return QString::fromStdString(cell->raw_formula_);
  }

  if (role == ErrorRole) {
    const auto opt_last_error = cell->get_last_error();
    if (opt_last_error) {
      return QString::fromStdString((*opt_last_error).message);
    }
  }

  if (role == FormulaRole) {
    return QString::fromStdString(cell->raw_formula_);
  }

  return {};
}

bool SpreadsheetModel::setData(const QModelIndex &index, const QVariant &value,
                               int /*role*/) {
  if (!index.isValid()) {
    return false;
  }

  const auto new_content = value.toString().toStdString();

  // Throw ui:cell_changed event for undo / redo handling
  EventDispatcher::dispatch(CellChangedEvent{
      CellEvent{.row = index.row(), .col = index.column()}, new_content});

  // Update domain model
  auto dirty =
      m_doc->set_cell_content(index.row(), index.column(), new_content);

  // Signal changed cells to view
  emit dataChanged(index, index, {Qt::DisplayRole});
  for (const auto &dep : dirty) {
    auto depIndex = createIndex(dep.row(), dep.column());
    emit dataChanged(depIndex, depIndex, {Qt::DisplayRole});
  }

  return true;
}

Qt::ItemFlags SpreadsheetModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QString SpreadsheetModel::columnLabel(int col) {
  QString label;
  col++;
  while (col > 0) {
    col--;
    label.prepend(QChar('A' + (col % 26)));
    col /= 26;
  }
  return label;
}

QVariant SpreadsheetModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const {
  if (role != Qt::DisplayRole) {
    return {};
  }

  if (orientation == Qt::Horizontal) {
    // Columns
    return columnLabel(section);
  }

  return section + 1;
}

void SpreadsheetModel::resetFromDocument() {
  beginResetModel();
  endResetModel();
}
