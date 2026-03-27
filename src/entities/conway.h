// entities/conway.h
#pragma once

#include <random>
#include <vector>

#include "../engine/components/conway.h"
#include "../engine/ecs/ecs.h"
#include "../engine/globals.h"
#include "raylib.h"

namespace motrix::entities {

/**
 * ============================================================================
 * Conway Entities
 * ============================================================================
 *
 * Creates multiple simulated particles used by fluid systems.
 *
 * Characteristics:
 *   • multiple ECS entities
 *   • physics + render components
 * ============================================================================
 */

// Helper to get neighbors coordinates offsets
inline const std::vector<Vector2> neighbor_offsets = {
  {-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

// Helper: Check if a color is "alive"
inline bool is_alive(const Color& c) { return c.a != 0; }

// Helper to convert 2D coords to index
inline int index(int x, int y) { return x + y * ACTIVE_W; }

inline void CreateConway(engine::ECS& ecs) {
  conway_speed = 20.f;
  alive_probability_control = 0.35f;
  total_cells = ACTIVE_W * ACTIVE_H;
  alive_cells = 0;

  gridEntities.resize(ACTIVE_W * ACTIVE_H);
  currentState.resize(ACTIVE_W * ACTIVE_H, false);
  nextState.resize(ACTIVE_W * ACTIVE_H, false);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distProb(0.0, 1.0);
  const float aliveProbability = 0.40f;

  for (int x = 0; x < ACTIVE_W; ++x) {
    for (int y = 0; y < ACTIVE_H; ++y) {
      int i = x + y * ACTIVE_W;
      engine::Entity e = ecs.create_entity();
      bool alive = distProb(gen) < aliveProbability;
      Color cellColor = alive ? WHITE : BLANK;
      ecs.add<engine::components::CellComponent>(
        e, Rectangle{(float)(x), (float)(y), 1.f, 1.f}, cellColor);
      gridEntities[i] = e;
      currentState[i] = alive;
      if (alive) alive_cells++;
    }
  }

  logger::info("[CONWAY] Created {} cells", ACTIVE_W * ACTIVE_H);
}

}  // namespace motrix::entities
