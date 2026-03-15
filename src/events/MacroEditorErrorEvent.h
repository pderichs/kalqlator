#pragma once

#include <string>

struct MacroEditorErrorEvent {
  std::string macro;
  std::string def;
  std::string message;
};