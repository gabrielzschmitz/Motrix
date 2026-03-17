// main.cpp
#include "engine/ecs.h"
#include "engine/systems.h"
#include "entities/conway.h"
#include "globals.h"
#include "raylib.h"
#include "resource_dir.h"

static void UpdateCamera2D(Camera2D* camera) {
  int winW = GetScreenWidth();
  int winH = GetScreenHeight();

  // Compute integer scale that fits the simulation
  float scaleX = (float)winW / (float)GAME_W;
  float scaleY = (float)winH / (float)GAME_H;
  float scale = floorf(fminf(scaleX, scaleY));

  if (scale < 1.0f) scale = 1.0f;

  camera->zoom = scale;
  camera->offset = (Vector2){winW * 0.5f, winH * 0.5f};
}

int main() {
  InitWindow(GAME_W * SCALE, GAME_H * SCALE, "GAME");

  SearchAndSetResourceDir("resources");

  Camera2D camera = {0};
  camera.target = (Vector2){GAME_W * 0.5f, GAME_H * 0.5f};
  camera.rotation = 0.0f;

  defaultFont = LoadFont("fonts/simple-font.png");

  // ECS
  ECS ecs;
  CreateConway(ecs);

  while (!WindowShouldClose()) {
    float dt = GetFrameTime();

    // UPDATE
    SimulateConway(ecs);
    UpdateCamera2D(&camera);

    // DRAW
    BeginDrawing();
    ClearBackground((Color){20, 22, 34, 255});
    BeginMode2D(camera);
    RenderCells(ecs);
    EndMode2D();

    DrawTextEx(defaultFont, TextFormat("FPS: %d", GetFPS()), (Vector2){10, 10},
               defaultFont.baseSize * 2, 1, (Color){255, 80, 150, 255});
    EndDrawing();
  }

  // Cleanup
  UnloadFont(defaultFont);
  CloseWindow();
  return 0;
}
