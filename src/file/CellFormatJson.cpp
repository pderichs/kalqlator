// KalQlator - CellFormatJson.cpp
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

#include "CellFormatJson.h"
#include "../model/CellDisplayFormatter.h"

namespace {
QString mode_to_string(RoundingMode mode) {
  switch (mode) {
  case RoundingMode::Nearest:
    return QStringLiteral("nearest");
  case RoundingMode::Floor:
    return QStringLiteral("floor");
  case RoundingMode::Ceil:
    return QStringLiteral("ceil");
  case RoundingMode::Truncate:
    return QStringLiteral("truncate");
  }

  return QStringLiteral("nearest");
}

std::optional<RoundingMode> mode_from_string(const QString &str) {
  if (str == QStringLiteral("nearest")) {
    return RoundingMode::Nearest;
  }
  if (str == QStringLiteral("floor")) {
    return RoundingMode::Floor;
  }
  if (str == QStringLiteral("ceil")) {
    return RoundingMode::Ceil;
  }
  if (str == QStringLiteral("truncate")) {
    return RoundingMode::Truncate;
  }
  return std::nullopt;
}
} // namespace

QJsonObject cell_format_json::to_json(const CellFormat &format) {
  QJsonObject obj;
  obj[QStringLiteral("specifier")] = QString::fromStdString(format.specifier);
  obj[QStringLiteral("rounding_mode")] = mode_to_string(format.rounding_mode);
  obj[QStringLiteral("word_wrap")] = format.word_wrap;
  return obj;
}

std::optional<CellFormat> cell_format_json::from_json(const QJsonObject &json) {
  CellFormat format;

  format.specifier =
      json.value(QStringLiteral("specifier")).toString().toStdString();
  if (!format.specifier.empty() &&
      !CellDisplayFormatter::is_valid_specifier(format.specifier)) {
    return std::nullopt;
  }

  // Existent but wrong value gets rejected
  const QJsonValue rounding_mode = json.value(QStringLiteral("rounding_mode"));
  if (!rounding_mode.isUndefined() && !rounding_mode.isNull()) {
    if (!rounding_mode.isString()) {
      return std::nullopt;
    }
    const auto parsed = mode_from_string(rounding_mode.toString());
    if (!parsed) {
      return std::nullopt;
    }
    format.rounding_mode = *parsed;
  }

  format.word_wrap = json.value(QStringLiteral("word_wrap")).toBool(false);
  return format;
}
