// app/app_state.h
#pragma once

#include "engine/ecs/ecs.h"
#include "raylib.h"

namespace motrix::app {

struct AppState {
  Camera2D camera{};
  engine::ECS ecs;
};

}  // namespace motrix::app
