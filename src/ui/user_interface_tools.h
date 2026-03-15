// KalQlator - user_interface_tools.h
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

#include <QIcon>
#include <QPainter>

#include <regex>
#include <string>

#include "../lisp/NotImplementedError.h"
#include "../tools/location.h"
#include "../tools/tools.h"

inline std::string get_cell_name_by_coordinates(size_t col, size_t row);

inline std::string get_cell_name_by_coordinates(const Location &location) {
  return get_cell_name_by_coordinates(location.x(), location.y());
}

inline std::string get_cell_name_by_coordinates(size_t col, size_t row) {
  std::string colName;
  int column_index = static_cast<int>(col) + 1; // 1 based for calculation

  while (column_index > 0) {
    column_index--; // adjust since A=0, not A=1
    colName = char('A' + (column_index % 26)) + colName;
    column_index /= 26;
  }

  return colName + std::to_string(row + 1);
}

inline Location get_cell_location_by_name(const std::string &name) {
  if (name.size() < 2) {
    throw std::runtime_error("incompatible name");
  }

  std::regex pattern(R"(([A-Z]*)(\d+))");
  std::smatch match;

  if (!std::regex_match(name, match, pattern)) {
    throw std::runtime_error("invalid input");
  }

  const std::string col_id = match[1].str();
  const std::string row_id = match[2].str();

  // TODO Support larger tables
  if (col_id.size() > 1) {
    throw NotImplementedError(
        "Cell references with two or more char columns are not supported yet");
  }

  const long col = col_id[0] - 'A';
  const long row = std::stoll(row_id) - 1;

  return Location(col, row);
}

inline bool is_function(const std::string &str) {
  auto trimmed_string = pdtools::trim(str);
  return trimmed_string.starts_with('=');
}

inline QIcon iconFromFont(const QString &text, int size = 24,
                          const QColor &color = Qt::black) {
  QPixmap pixmap(size, size);
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  QFont font("Material Icons", size * 0.75); // Adjust size
  painter.setFont(font);
  painter.setPen(color);
  painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

  return {pixmap};
}

inline std::string make_function(const std::string &content) {
  if (pdtools::trim(content).starts_with("=")) {
    return content;
  }

  return "=" + content;
}

inline std::string make_non_function(const std::string &content) {
  if (!pdtools::trim(content).starts_with("=")) {
    return content;
  }

  return pdtools::trim(content).substr(1);
}