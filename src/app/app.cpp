// app/app.cpp
#include "app/app.h"

#include "app/app_helpers.h"
#include "app/app_state.h"
#include "engine/globals.h"
#include "engine/logger.h"
#include "engine/systems/conway.h"
#include "engine/systems/ui.h"
#include "entities/conway.h"
#include "entities/ui.h"
#include "raylib.h"
#include "resource_dir.h"

namespace m_app = motrix::app;
namespace m_eng = motrix::engine;
namespace m_ett = motrix::entities;

static void InitApp(m_app::AppState& state) {
  InitWindow(GAME_W * SCALE, GAME_H * SCALE, "Motrix");

  SearchAndSetResourceDir("resources");

  state.camera.target = Vector2{0.f, 0.f};
  state.camera.offset = Vector2{0.f, 0.f};
  state.camera.rotation = 0.0f;
  state.camera.zoom = 1.0f;

  defaultFont = LoadFont("fonts/simple-font.png");

  m_ett::CreateConway(state.ecs);
  m_ett::CreateUI(state.ecs);

  // state.ecs.print_entities(logger::Level::Debug);
}

static void UpdateApp(m_app::AppState& state, float dt) {
  m_eng::systems::SimulateConway(state.ecs);
  m_ett::UpdateConwayUIText(state.ecs);
  m_app::UpdateCamera2D(&state.camera);
}

static void RenderApp(m_app::AppState& state) {
  BeginDrawing();

  BeginMode2D(state.camera);

  ClearBackground(Color{1, 127, 127, 255});
  m_eng::systems::RenderCells(state.ecs);

  EndMode2D();

  m_eng::systems::RenderUI(state.ecs);

  DrawTextEx(defaultFont, TextFormat("FPS: %d", GetFPS()), Vector2{10, 10},
             defaultFont.baseSize * SCALE, 1, Color{246, 120, 232, 255});

  EndDrawing();
}

int RunApp(int argc, char** argv) {
  (void)argc;
  (void)argv;

  m_app::AppState state;

  InitApp(state);

  while (!WindowShouldClose()) {
    const float dt = GetFrameTime();

    UpdateApp(state, dt);
    RenderApp(state);
  }

  UnloadFont(defaultFont);
  CloseWindow();

  return 0;
}
