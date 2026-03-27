// engine/systems/conway.h
#pragma once

#include "../../entities/conway.h"
#include "../components/conway.h"
#include "../ecs/ecs.h"
#include "../globals.h"
#include "raylib.h"

namespace motrix::engine::systems {

inline void SimulateConway(ECS& ecs) {
  static float accumulator = 0.f;

  if (paused) return;

  accumulator += GetFrameTime();

  const float step = 1.0f / conway_speed;

  if (accumulator < step) return;

  accumulator -= step;

  for (int y = 0; y < ACTIVE_H; ++y) {
    for (int x = 0; x < ACTIVE_W; ++x) {
      int liveNeighbors = 0;

      for (auto& offset : entities::neighbor_offsets) {
        int nx = x + static_cast<int>(offset.x);
        int ny = y + static_cast<int>(offset.y);

        if (nx >= 0 && nx < ACTIVE_W && ny >= 0 && ny < ACTIVE_H) {
          if (currentState[entities::index(nx, ny)]) liveNeighbors++;
        }
      }

      bool alive = currentState[entities::index(x, y)];

      bool nextAlive = alive ? (liveNeighbors == 2 || liveNeighbors == 3)
                             : (liveNeighbors == 3);

      nextState[entities::index(x, y)] = nextAlive;
    }
  }

  alive_cells = 0;
  for (int i = 0; i < (int)gridEntities.size(); ++i) {
    auto& cell = ecs.get<components::CellComponent>(gridEntities[i]);

    bool alive = nextState[i];

    cell.color = alive ? WHITE : BLANK;

    if (alive) alive_cells++;
  }

  currentState.swap(nextState);
}

inline void RenderCells(ECS& ecs) {
  ecs.group_view<components::CellComponent>(
    [&](Entity e, components::CellComponent& quad) {
      if (entities::is_alive(quad.color))
        DrawRectangleRec(quad.rect, quad.color);
    });
}

}  // namespace motrix::engine::systems
