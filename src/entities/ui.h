// entities/ui.h
#pragma once

#include <random>
#include <string>

#include "../engine/components/conway.h"
#include "../engine/components/ui.h"
#include "../engine/ecs/ecs.h"
#include "../engine/globals.h"
#include "engine/components/ui_traits.h"

namespace motrix::entities {

inline void ResetConwayGrid(engine::ECS& ecs) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distProb(0.0, 1.0);

  alive_cells = 0;
  conway_speed = 20.f;
  alive_probability_control = 0.35f;
  for (int x = 0; x < ACTIVE_W; ++x) {
    for (int y = 0; y < ACTIVE_H; ++y) {
      int i = x + y * ACTIVE_W;

      bool alive = distProb(gen) < alive_probability_control;
      currentState[i] = alive;
      if (alive) alive_cells++;

      auto& cell = ecs.get<engine::components::CellComponent>(gridEntities[i]);

      cell.color = alive ? WHITE : BLANK;
    }
  }
}

inline void SetCellAlive(engine::ECS& ecs, int x, int y) {
  if (x < 0 || x >= ACTIVE_W || y < 0 || y >= ACTIVE_H) return;

  int i = x + y * ACTIVE_W;

  currentState[i] = true;

  auto& cell = ecs.get<engine::components::CellComponent>(gridEntities[i]);
  cell.color = WHITE;
}

inline void ClearConwayGrid(engine::ECS& ecs) {
  alive_cells = 0;

  for (int i = 0; i < (int)gridEntities.size(); ++i) {
    currentState[i] = false;

    auto& cell = ecs.get<engine::components::CellComponent>(gridEntities[i]);

    cell.color = BLACK;
  }
}

inline void ApplyPattern(engine::ECS& ecs, const std::string& selection) {
  ClearConwayGrid(ecs);

  if (selection == "Random") {
    ResetConwayGrid(ecs);
    return;
  }

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<> distX(2, ACTIVE_W - 3);
  std::uniform_int_distribution<> distY(2, ACTIVE_H - 3);

  const int pattern_count = (ACTIVE_W * ACTIVE_H) / 250;

  for (int n = 0; n < pattern_count; ++n) {
    int x = distX(gen);
    int y = distY(gen);

    if (selection == "Glider") {
      SetCellAlive(ecs, x, y);
      SetCellAlive(ecs, x + 1, y);
      SetCellAlive(ecs, x + 2, y);

      SetCellAlive(ecs, x + 2, y - 1);
      SetCellAlive(ecs, x + 1, y - 2);
    }

    else if (selection == "Blinker") {
      SetCellAlive(ecs, x - 1, y);
      SetCellAlive(ecs, x, y);
      SetCellAlive(ecs, x + 1, y);
    }

    else if (selection == "Block") {
      SetCellAlive(ecs, x, y);
      SetCellAlive(ecs, x + 1, y);
      SetCellAlive(ecs, x, y + 1);
      SetCellAlive(ecs, x + 1, y + 1);
    }
  }

  alive_cells = 0;

  for (bool alive : currentState)
    if (alive) alive_cells++;
}

inline void UpdateConwayUIText(engine::ECS& ecs) {
  ecs.group_view<engine::components::UITextComponent>(
    [&](engine::Entity e, engine::components::UITextComponent& text) {
      if (text.text.find("Cells:") == 0) {
        text.text = "Cells: " + std::to_string(alive_cells) + "/" +
                    std::to_string(total_cells);
        text.text_width = engine::components::MeasureUITextWidth(text.text);
      }
    });
}

inline void CreateUI(engine::ECS& ecs) {
  //
  // Window
  //
  constexpr float window_width = 240.f;
  constexpr float margin = 1.f;
  engine::Entity window = ecs.create_entity();

  ecs.add<engine::components::UIWindowComponent>(
    window, engine::components::UIWindowComponent{
              {GAME_W - window_width - margin, margin},
              window_width,
              0.f,
              "Conway Controls"});

  //
  // Simulation GROUP
  //
  engine::Entity sim_group = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    sim_group, engine::components::UILayoutChildComponent{window, -1.f, 0.f});

  ecs.add<engine::components::UIResolvedRectComponent>(sim_group);

  ecs.add<engine::components::UIGroupComponent>(
    sim_group, engine::components::UIGroupComponent{"Simulation", true});

  //
  // Speed slider
  //
  engine::Entity speed_slider = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    speed_slider,
    engine::components::UILayoutChildComponent{window, -1.f, 30.f});

  ecs.add<engine::components::UIResolvedRectComponent>(speed_slider);

  ecs.add<engine::components::UISliderComponent>(
    speed_slider, engine::components::UISliderComponent{
                    "Speed", &conway_speed, 1.f, 20.f, 1.f,
                    [](float value) { conway_speed = value; }});

  ecs.add<engine::components::UITooltipComponent>(
    speed_slider, "Adjust generations per second.");

  ecs.add<engine::components::UIGroupChildComponent>(speed_slider, sim_group);

  //
  // Alive probability slider
  //
  engine::Entity density_slider = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    density_slider,
    engine::components::UILayoutChildComponent{window, -1.f, 30.f});

  ecs.add<engine::components::UIResolvedRectComponent>(density_slider);

  ecs.add<engine::components::UISliderComponent>(
    density_slider, engine::components::UISliderComponent{
                      "Density", &alive_probability_control, 0.05f, 1.f, 0.05f,
                      [](float value) { alive_probability_control = value; }});

  ecs.add<engine::components::UITooltipComponent>(
    density_slider, "Probability of a cell starting alive.");

  ecs.add<engine::components::UIGroupChildComponent>(density_slider, sim_group);

  //
  // Pause checkbox
  //
  engine::Entity pause_checkbox = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    pause_checkbox,
    engine::components::UILayoutChildComponent{window, 0.f, 20.f});

  ecs.add<engine::components::UIResolvedRectComponent>(pause_checkbox);

  ecs.add<engine::components::UICheckboxComponent>(
    pause_checkbox, engine::components::UICheckboxComponent{
                      "Pause", &paused, [](bool value) { paused = value; }});

  ecs.add<engine::components::UITooltipComponent>(pause_checkbox,
                                                  "Pause simulation updates.");

  ecs.add<engine::components::UIGroupChildComponent>(pause_checkbox, sim_group);

  //
  // Current alive cells
  //
  engine::Entity alive_cells_text = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    alive_cells_text,
    engine::components::UILayoutChildComponent{window, 0.f, 20.f});

  ecs.add<engine::components::UIResolvedRectComponent>(alive_cells_text);

  ecs.add<engine::components::UITextComponent>(
    alive_cells_text, engine::components::UITextComponent("Cells: N/N"));
  ecs.add<engine::components::UITooltipComponent>(
    alive_cells_text, "The current amount of alive cells per total.");
  ecs.add<engine::components::UIGroupChildComponent>(alive_cells_text,
                                                     sim_group);

  //
  // Pattern GROUP
  //
  engine::Entity pattern_group = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    pattern_group,
    engine::components::UILayoutChildComponent{window, -1.f, 0.f});

  ecs.add<engine::components::UIResolvedRectComponent>(pattern_group);

  ecs.add<engine::components::UIGroupComponent>(
    pattern_group, engine::components::UIGroupComponent{"Patterns", true});

  //
  // Pattern dropdown
  //
  engine::Entity dropdown = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    dropdown, engine::components::UILayoutChildComponent{window, 0.f, 24.f});

  ecs.add<engine::components::UIResolvedRectComponent>(dropdown);

  ecs.add<engine::components::UIDropdownComponent>(
    dropdown,
    engine::components::UIDropdownComponent{
      "Preset",
      {"Random", "Glider", "Blinker", "Block"},
      &pattern_index,
      [&](const std::string& selection) { ApplyPattern(ecs, selection); }});

  ecs.add<engine::components::UITooltipComponent>(
    dropdown, "Choose initial cell preset.");

  ecs.add<engine::components::UIGroupChildComponent>(dropdown, pattern_group);

  //
  // Actions GROUP
  //
  engine::Entity action_group = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    action_group,
    engine::components::UILayoutChildComponent{window, -1.f, 0.f});

  ecs.add<engine::components::UIResolvedRectComponent>(action_group);

  ecs.add<engine::components::UIGroupComponent>(
    action_group, engine::components::UIGroupComponent{"Actions", true});

  //
  // Reset button
  //
  engine::Entity reset_button = ecs.create_entity();

  ecs.add<engine::components::UILayoutChildComponent>(
    reset_button,
    engine::components::UILayoutChildComponent{window, -1.f, 40.f});

  ecs.add<engine::components::UIResolvedRectComponent>(reset_button);

  ecs.add<engine::components::UIButtonComponent>(
    reset_button, engine::components::UIButtonComponent{
                    "Reset Grid", false, [&ecs]() {
                      pattern_index = 0;
                      ResetConwayGrid(ecs);
                      ClearBackground(Color{1, 127, 127, 255});
                    }});

  ecs.add<engine::components::UITooltipComponent>(
    reset_button, "Generate a new random grid.");

  ecs.add<engine::components::UIGroupChildComponent>(reset_button,
                                                     action_group);

  logger::info("[GUI] Created Conway UI window '{}' (entity:{}:{})",
               ecs.get<engine::components::UIWindowComponent>(window).title,
               window.index, window.version);
}

}  // namespace motrix::entities
