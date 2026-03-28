#pragma once

#include <gmpxx.h>

inline mpq_class mpq_class_from_decimal_or_int(const std::string &number) {
  auto dot = number.find('.');
  if (dot == std::string::npos) {
    return mpq_class(number); // int
  }

  std::string digits = number.substr(0, dot) + number.substr(dot + 1);
  size_t decimals = number.size() - dot - 1;

  mpz_class denom = 1;
  for (size_t i = 0; i < decimals; ++i) {
    denom *= 10;
  }

  mpq_class result(mpz_class(digits), denom);
  result.canonicalize();

  return result;
}

inline std::string to_decimal_string(const mpq_class &value_source,
                                     int precision = 10) {
  mpf_class float_value(value_source, 256);
  mp_exp_t exp;
  std::string float_string = float_value.get_str(exp, 10, precision);

  if (float_string.empty()) {
    return "0";
  }

  // Vorzeichen separat behandeln
  std::string sign;
  if (float_string[0] == '-') {
    sign = "-";
    float_string = float_string.substr(1);
  }

  if (exp <= 0) {
    float_string = "0." + std::string(-exp, '0') + float_string;
  } else if (exp >= static_cast<int>(float_string.size())) {
    float_string += std::string(exp - float_string.size(), '0');
  } else {
    float_string.insert(exp, ".");
  }

  return sign + float_string;
}
