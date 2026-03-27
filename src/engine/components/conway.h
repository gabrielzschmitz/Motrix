// engine/components/conway.h
#pragma once

#include <string_view>

#include "raylib.h"

namespace motrix::engine::components {

struct CellComponent {
  static constexpr std::string_view Name = "CellComponent";

  Rectangle rect;
  Color color;

  CellComponent(const Rectangle& r = {0, 0, 0, 0}, Color c = WHITE)
    : rect(r), color(c) {}
};
}  // namespace motrix::engine::components
