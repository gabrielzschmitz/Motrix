// app/app_helpers.h
#pragma once

#include <cmath>

#include "engine/globals.h"
#include "raylib.h"

namespace motrix::app {

inline void UpdateCamera2D(Camera2D* camera) {
  const int window_width = GetScreenWidth();
  const int window_height = GetScreenHeight();

  const float scale_x =
    static_cast<float>(window_width) / static_cast<float>(GAME_W);

  const float scale_y =
    static_cast<float>(window_height) / static_cast<float>(GAME_H);

  float scale = std::fmin(scale_x, scale_y);

  if (scale < 1.0f) scale = 1.0f;

  camera->zoom = scale;
  uiScale = scale;

  camera->offset = Vector2{(window_width - GAME_W * scale) * 0.5f,
                           (window_height - GAME_H * scale) * 0.5f};

  camera->target = Vector2{0.f, 0.f};
}

}  // namespace motrix::app
