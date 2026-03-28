#pragma once

#include <string>

struct MacroErrorEvent {
  static constexpr std::string_view event_name = "model:macro-error";

  std::string macro;
  std::string def;
  std::string message;
};