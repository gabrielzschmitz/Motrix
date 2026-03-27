// engine/globals.h
#pragma once

#include "ecs/ecs.h"
#include "raylib.h"

/*
 * Common scaled resolutions:
 *
 * Base Resolution | SCALE | Window Size
 * --------------------------------------
 *   320 x 180     |   6   | 1920 x 1080
 *   320 x 180     |   3   |  960 x 540
 *   320 x 180     |   4   | 1280 x 720
 *   640 x 360     |   2   | 1280 x 720
 *  1280 x 720     |   1   | 1280 x 720
 */
inline constexpr int SCALE = 2;
inline constexpr int GAME_W = 640;
inline constexpr int GAME_H = 360;
inline constexpr int ACTIVE_W = GAME_W - 1;
inline constexpr int ACTIVE_H = GAME_H - 1;

inline float uiScale = 1.f;

inline Font defaultFont{};

inline std::vector<motrix::engine::Entity> gridEntities;
inline std::vector<bool> currentState;
inline std::vector<bool> nextState;

inline float conway_speed = 8.f;
inline float alive_probability_control = 0.65f;
inline bool paused = false;
inline int pattern_index = 0;
inline int alive_cells = 0;
inline int total_cells = 0;
