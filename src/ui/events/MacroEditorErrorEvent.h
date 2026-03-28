#pragma once

#include <string>

struct MacroEditorErrorEvent {
  static constexpr std::string_view event_name = "ui:macro_editor_error";

  std::string macro;
  std::string def;
  std::string message;
};