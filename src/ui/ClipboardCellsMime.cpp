// KalQlator - ClipboardCellsMime.cpp
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

#include "ClipboardCellsMime.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
QJsonObject cell_format_to_json(const CellFormat &format) {
  QJsonObject obj;
  obj[QStringLiteral("specifier")] = QString::fromStdString(format.specifier);
  obj[QStringLiteral("rounding_mode")] = static_cast<int>(format.rounding_mode);
  obj[QStringLiteral("word_wrap")] = format.word_wrap;
  return obj;
}

std::optional<CellFormat> cell_format_from_json(const QJsonObject &json) {
  CellFormat format;
  format.specifier = json[QStringLiteral("specifier")].toString().toStdString();
  int rounding_mode = json[QStringLiteral("rounding_mode")].toInt(
      static_cast<int>(RoundingMode::Nearest));
  if (rounding_mode < static_cast<int>(RoundingMode::Nearest) ||
      rounding_mode > static_cast<int>(RoundingMode::Truncate)) {
    return std::nullopt;
  }

  format.rounding_mode = static_cast<RoundingMode>(rounding_mode);
  format.word_wrap = json[QStringLiteral("word_wrap")].toBool(false);
  return format;
}
} // namespace

QByteArray
serialize_clipboard_cells_payload(const ClipboardCellsPayload &payload) {
  QJsonArray cells;
  for (const auto &cell : payload.cells) {
    QJsonObject cell_json;
    cell_json[QStringLiteral("content")] = QString::fromStdString(cell.content);
    if (cell.format.has_value()) {
      cell_json[QStringLiteral("format")] =
          cell_format_to_json(cell.format.value());
    }
    cells.append(cell_json);
  }

  QJsonObject root;
  root[QStringLiteral("version")] = KALQLATOR_CLIPBOARD_FORMAT_VERSION;
  root[QStringLiteral("rows")] = payload.rows;
  root[QStringLiteral("cols")] = payload.cols;
  root[QStringLiteral("cells")] = cells;

  return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

std::optional<ClipboardCellsPayload>
parse_clipboard_cells_payload(const QMimeData *mime_data) {
  if (mime_data == nullptr ||
      !mime_data->hasFormat(KALQLATOR_CLIPBOARD_MIME_TYPE)) {
    return std::nullopt;
  }

  const QByteArray payload_data =
      mime_data->data(KALQLATOR_CLIPBOARD_MIME_TYPE);
  QJsonParseError parse_error;
  const QJsonDocument parsed =
      QJsonDocument::fromJson(payload_data, &parse_error);
  if (parse_error.error != QJsonParseError::NoError || !parsed.isObject()) {
    return std::nullopt;
  }

  const QJsonObject root = parsed.object();

  const int version = root.value(QStringLiteral("version")).toInt(0);
  if (version != KALQLATOR_CLIPBOARD_FORMAT_VERSION) {
    return std::nullopt; // missing or unknown -> fall back to plain text
  }

  ClipboardCellsPayload payload;
  payload.rows = root[QStringLiteral("rows")].toInt();
  payload.cols = root[QStringLiteral("cols")].toInt();

  const QJsonArray cells = root[QStringLiteral("cells")].toArray();
  if (payload.rows <= 0 || payload.cols <= 0 ||
      cells.size() != payload.rows * payload.cols) {
    return std::nullopt;
  }

  payload.cells.reserve(static_cast<size_t>(cells.size()));
  for (const auto &item : cells) {
    const QJsonObject cell_json = item.toObject();
    ClipboardCellData cell;
    cell.content =
        cell_json[QStringLiteral("content")].toString().toStdString();

    if (cell_json.contains(QStringLiteral("format"))) {
      const auto format =
          cell_format_from_json(cell_json[QStringLiteral("format")].toObject());
      if (!format.has_value()) {
        return std::nullopt;
      }
      cell.format = format.value();
    }

    payload.cells.push_back(cell);
  }

  return payload;
}
