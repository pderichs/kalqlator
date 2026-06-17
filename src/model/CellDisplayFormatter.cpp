// KalQlator - CellDisplayFormatter.cpp
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

#include "CellDisplayFormatter.h"

#include <gmpxx.h>
#include <regex>

#include "../lisp/gmp_tools.h"
#include "../lisp/object.h"
#include "../tools/tools.h"

ParsedFormatSpecifier
CellDisplayFormatter::parse_specifier(const std::string &specifier) {
  ParsedFormatSpecifier result;

  static const std::regex specifier_regex(R"(^([^#]*)(#+(?:\.#+)?)([^#]*)$)");
  std::smatch match;

  if (!std::regex_match(specifier, match, specifier_regex)) {
    return result;
  }

  result.prefix = match[1].str();
  const std::string mask = match[2].str();
  result.postfix = match[3].str();

  auto dot_pos = mask.find('.');
  if (dot_pos != std::string::npos) {
    result.decimal_places = static_cast<int>(mask.size() - dot_pos - 1);
  } else {
    result.decimal_places = 0;
  }

  result.valid = true;
  return result;
}

bool CellDisplayFormatter::is_valid_specifier(const std::string &specifier) {
  return parse_specifier(specifier).valid;
}

// Round value to decimal_places using mpq_class arithmetic.
// Rounds half away from zero (e.g. -12.345 → -12.35).
static mpq_class round_mpq_to_decimals(const mpq_class &value,
                                       int decimal_places, RoundingMode mode) {
  if (decimal_places < 0) {
    return value;
  }

  mpz_class scale = 1;
  for (int i = 0; i < decimal_places; ++i) {
    scale *= 10;
  }

  mpq_class scaled = value * scale;
  scaled.canonicalize();

  mpz_class num = scaled.get_num();
  mpz_class den = scaled.get_den();

  bool negative = num < 0;
  if (negative) {
    num = -num;
  }

  mpz_class int_part = num / den;
  mpz_class remainder = num % den;

  bool round_away = false;

  switch (mode) {
  case RoundingMode::Nearest: {
    // half-away-from-zero: remainder * 2 >= den means >= 0.5
    if (remainder * 2 >= den) {
      round_away = true;
    }
    break;
  }
  case RoundingMode::Floor: {
    if (remainder != 0 && negative) {
      round_away = true;
    }
    break;
  }
  case RoundingMode::Ceil: {
    if (remainder != 0 && !negative) {
      round_away = true;
    }
    break;
  }
  case RoundingMode::Truncate: {
    break;
  }
  }

  if (round_away) {
    int_part += 1;
  }

  if (negative) {
    int_part = -int_part;
  }

  mpq_class result(int_part, 1);
  result /= scale;
  result.canonicalize();
  return result;
}

static std::string format_rounded_value(const mpq_class &value,
                                        int decimal_places) {
  if (decimal_places == 0) {
    mpz_class int_part = value.get_num() / value.get_den();
    return int_part.get_str();
  }

  mpz_class num = value.get_num();
  mpz_class den = value.get_den();

  bool negative = num < 0;
  if (negative) {
    num = -num;
  }

  mpz_class int_part = num / den;
  mpz_class remainder = num % den;

  std::string result;
  if (negative) {
    result = "-";
  }
  result += int_part.get_str();
  result += ".";

  mpz_class scale = 1;
  for (int i = 0; i < decimal_places; ++i) {
    scale *= 10;
  }

  mpz_class frac = remainder * scale / den;
  std::string frac_str = frac.get_str();

  while (static_cast<int>(frac_str.size()) < decimal_places) {
    frac_str = "0" + frac_str;
  }

  if (static_cast<int>(frac_str.size()) > decimal_places) {
    frac_str.resize(static_cast<size_t>(decimal_places));
  }

  result += frac_str;
  return result;
}

// Shared core: format an already-rounded mpq value with prefix/postfix
static std::string format_mpq(const mpq_class &value,
                              const ParsedFormatSpecifier &parsed) {
  std::string formatted = format_rounded_value(value, parsed.decimal_places);
  return parsed.prefix + formatted + parsed.postfix;
}

std::string CellDisplayFormatter::format(const std::string &value,
                                         const CellFormat &format) {
  if (!format.has_specifier()) {
    return value;
  }

  auto parsed = parse_specifier(format.specifier);
  if (!parsed.valid) {
    return value;
  }

  if (!pdtools::is_number(value)) {
    return value;
  }

  mpq_class num = mpq_class_from_decimal_or_int(value);
  mpq_class rounded =
      round_mpq_to_decimals(num, parsed.decimal_places, format.rounding_mode);
  return format_mpq(rounded, parsed);
}

std::string
CellDisplayFormatter::format_number(const lisp::LispObjectPtr &evaluated_value,
                                    const CellFormat &format) {
  if (!format.has_specifier() || !evaluated_value ||
      !evaluated_value->is_number()) {
    return {};
  }

  auto parsed = parse_specifier(format.specifier);
  if (!parsed.valid) {
    return {};
  }

  mpq_class num = evaluated_value->as_number();
  mpq_class rounded =
      round_mpq_to_decimals(num, parsed.decimal_places, format.rounding_mode);
  return format_mpq(rounded, parsed);
}
