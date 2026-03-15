#pragma once

#include <string>

struct MacroErrorEvent {
  std::string macro;
  std::string def;
  std::string message;
};