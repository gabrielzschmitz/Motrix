// engine/systems/ui.h
#pragma once

#include <cmath>
#include <string>
#include <vector>

#include "../components/ui.h"
#include "../ecs/ecs.h"
#include "raymath.h"
#include "ui_helpers.h"

namespace motrix::engine::systems {

using namespace motrix::engine::components;

//
// Layout Helpers
//
struct RowItem {
  UILayoutChildComponent* layout;
  UIResolvedRectComponent* rect;
  float width;
};

inline void FlushRow(std::vector<RowItem>& row, float content_width,
                     float content_left, float& cursor_y, float gap_spacing,
                     bool add_gap = true) {
  if (row.empty()) return;

  float row_height = 0.f;
  float total_width = 0.f;

  for (auto& item : row) {
    row_height = std::max(row_height, item.layout->preferred_height);
    total_width += item.width;
  }

  float gap = (row.size() > 1)
                ? (content_width - total_width) / float(row.size() - 1)
                : 0.f;

  float used_width = total_width + gap * (row.size() - 1);
  float x = content_left + (content_width - used_width) * 0.5f;

  for (auto& item : row) {
    item.rect->rect = {x, cursor_y, item.width, item.layout->preferred_height};
    x += item.width + gap;
  }

  cursor_y += row_height + (add_gap ? gap_spacing : 0.f);
  row.clear();
}

//
// Layout
//
inline void LayoutGroup(ECS& ecs, Entity group, UIGroupComponent& g,
                        UIResolvedRectComponent& group_rect, float content_left,
                        float content_width, float& cursor_y) {
  float group_start_y = cursor_y + g.padding_top;
  float group_cursor_y = group_start_y;

  float inner_content_left = content_left + g.padding_left;
  float inner_content_width = content_width - g.padding_left - g.padding_right;

  std::vector<RowItem> group_row;

  ecs.group_view<UIGroupChildComponent, UIResolvedRectComponent,
                 UILayoutChildComponent>(
    [&](Entity child, UIGroupChildComponent& gc,
        UIResolvedRectComponent& child_rect, UILayoutChildComponent& layout) {
      if (gc.parent_group != group) return;

      float width = layout.preferred_width;

      if (ecs.has<UISliderComponent>(child) || width == -1.f) {
        width = inner_content_width;
      } else {
        if (width <= 0.f) width = ResolveIntrinsicWidth(ecs, child);
        if (width <= 0.f) width = inner_content_width * 0.5f;
      }

      float row_width = width;
      for (auto& item : group_row) row_width += item.width;
      if (!group_row.empty()) row_width += group_row.size() * g.spacing;

      if ((ecs.has<UISliderComponent>(child) ||
           layout.preferred_width == -1.f) ||
          row_width > inner_content_width) {
        FlushRow(group_row, inner_content_width, inner_content_left,
                 group_cursor_y, g.spacing, true);
      }

      group_row.push_back(RowItem{&layout, &child_rect, width});
    });

  FlushRow(group_row, inner_content_width, inner_content_left, group_cursor_y,
           g.spacing, false);

  float group_height = group_cursor_y - group_start_y;

  group_rect.rect = {content_left - 4.f, group_start_y - g.padding_top,
                     content_width + 8.f,
                     group_height + g.padding_top + g.padding_bottom};

  cursor_y = group_rect.rect.y + group_rect.rect.height;
}

inline void LayoutStandaloneChildren(ECS& ecs, Entity window_entity,
                                     float content_left, float content_width,
                                     float& cursor_y,
                                     UIWindowComponent& window) {
  std::vector<RowItem> row;

  ecs.group_view<UILayoutChildComponent, UIResolvedRectComponent>(
    [&](Entity child, UILayoutChildComponent& layout,
        UIResolvedRectComponent& child_rect) {
      if (layout.parent != window_entity) return;
      if (ecs.has<UIGroupChildComponent>(child)) return;
      if (ecs.has<UIGroupComponent>(child)) return;

      float width = layout.preferred_width;
      bool full_row = ecs.has<UISliderComponent>(child) || width == -1.f;

      if (full_row) {
        width = content_width;
      } else {
        if (width <= 0.f) width = ResolveIntrinsicWidth(ecs, child);
        if (width <= 0.f) width = content_width * 0.5f;
      }

      float row_width = width;
      for (auto& item : row) row_width += item.width;
      if (!row.empty()) row_width += row.size() * window.gap;

      if (full_row || row_width > content_width) {
        FlushRow(row, content_width, content_left, cursor_y, window.gap);
      }

      row.push_back(RowItem{&layout, &child_rect, width});
    });

  FlushRow(row, content_width, content_left, cursor_y, window.gap, false);
}

inline void LayoutUI(ECS& ecs) {
  ecs.group_view<UIWindowComponent>([&](Entity window_entity,
                                        UIWindowComponent& window) {
    if (!window.layout_dirty) return;

    constexpr float title_h = 24.f;

    float cursor_y = window.position.y + window.padding / 2 + title_h;
    float content_left = window.position.x + window.padding;
    float content_width = window.width - 2.f * window.padding;

    size_t group_count = 0;

    ecs.group_view<UIGroupComponent>(
      [&](Entity, UIGroupComponent&) { group_count++; });

    bool has_standalone_children = false;

    ecs.group_view<UILayoutChildComponent>(
      [&](Entity child, UILayoutChildComponent& layout) {
        if (layout.parent == window_entity &&
            !ecs.has<UIGroupChildComponent>(child) &&
            !ecs.has<UIGroupComponent>(child)) {
          has_standalone_children = true;
        }
      });

    size_t current_group_idx = 0;

    ecs.group_view<UIGroupComponent, UIResolvedRectComponent>(
      [&](Entity group, UIGroupComponent& g, UIResolvedRectComponent& rect) {
        LayoutGroup(ecs, group, g, rect, content_left, content_width, cursor_y);

        current_group_idx++;

        if (current_group_idx < group_count || has_standalone_children) {
          cursor_y += window.gap;
        }
      });

    LayoutStandaloneChildren(ecs, window_entity, content_left, content_width,
                             cursor_y, window);

    if (window.auto_height) {
      window.height = cursor_y - window.position.y + window.padding;
    }

    window.layout_dirty = false;
  });
}

//
// Render
//
inline void RenderWindow(ECS& ecs) {
  Vector2 mouse = GetMousePosition();

  ecs.group_view<UIWindowComponent>([&](Entity e, UIWindowComponent& window) {
    Rectangle rect{window.position.x, window.position.y, window.width,
                   window.height};

    rect = ScaleRect(rect);

    const float title_h = 18.f * uiScale;

    Rectangle close_button{rect.x + rect.width - 2.f - title_h, rect.y + 2.f,
                           title_h, title_h};

    Rectangle title_bar{rect.x + 2.f, rect.y + 2.f, rect.width - 4.f - title_h,
                        title_h};

    // Drag start
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(mouse, title_bar)) {
      window.dragging = true;
      window.layout_dirty = true;

      window.drag_offset = {(mouse.x / uiScale) - window.position.x,
                            (mouse.y / uiScale) - window.position.y};
    }

    // Drag stop
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) window.dragging = false;

    // Drag move
    if (window.dragging) {
      window.position.x = (mouse.x / uiScale) - window.drag_offset.x;
      window.position.y = (mouse.y / uiScale) - window.drag_offset.y;
      window.layout_dirty = true;
    }

    // Close request
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(mouse, close_button)) {
      window.close_requested = true;
    }

    // Draw frame
    DrawWin95Box(rect, LIGHTGRAY);

    DrawRectangleRec(title_bar, {0, 0, 128, 255});

    DrawText(window.title.c_str(), rect.x + 6 * uiScale, rect.y + 4 * uiScale,
             10 * uiScale, WHITE);

    DrawWin95Box(close_button, LIGHTGRAY);

    DrawText("X", close_button.x + 5.5f * uiScale,
             close_button.y + 4.5f * uiScale, 10 * uiScale, BLACK);

    // Destroy window + children
    if (window.close_requested) {
      ecs.group_view<UILayoutChildComponent>(
        [&](Entity child, UILayoutChildComponent& layout) {
          if (layout.parent.index == e.index &&
              layout.parent.version == e.version) {
            ecs.destroy_entity(child);
          }
        });

      logger::info("[GUI] Closed window '{}' (entity:{}:{})", window.title,
                   e.index, e.version);

      ecs.destroy_entity(e);
      // ecs.print_entities();
    }
  });
}

inline void RenderGroups(ECS& ecs) {
  ecs.group_view<UIGroupComponent, UIResolvedRectComponent>(
    [&](Entity, UIGroupComponent& g, UIResolvedRectComponent& rect) {
      Rectangle r = ScaleRectCached(rect);

      DrawRectangleLinesEx(r, 1.f, DARKGRAY);

      if (g.separator)
        DrawLine(r.x, r.y + 18 * uiScale, r.x + r.width, r.y + 18 * uiScale,
                 GRAY);

      if (!g.title.empty())
        DrawText(g.title.c_str(), r.x + 4 * uiScale, r.y + 2 * uiScale,
                 12 * uiScale, BLACK);
    });
}

inline void RenderSliders(ECS& ecs, bool& input_consumed) {
  ecs.group_view<UISliderComponent, UIResolvedRectComponent>(
    [&](Entity, UISliderComponent& slider, UIResolvedRectComponent& resolved) {
      if (!slider.value) return;

      float current_val = *slider.value;
      Rectangle rect = ScaleRectCached(resolved);
      Rectangle bar{rect.x, rect.y + 14 * uiScale, rect.width, 16 * uiScale};

      DrawText(TextFormat("%s: %.2f", slider.label.c_str(), current_val),
               rect.x, rect.y, 10 * uiScale, BLACK);

      DrawRectangleRec(bar, WHITE);
      DrawRectangleLinesEx(bar, 1 * uiScale, BLACK);

      float t = (current_val - slider.min) / (slider.max - slider.min);
      float knob_x = bar.x + t * bar.width;

      Rectangle knob{knob_x - 4 * uiScale, bar.y - 2 * uiScale, 8 * uiScale,
                     bar.height + 4 * uiScale};

      DrawWin95Box(knob, LIGHTGRAY);

      bool hit = CheckCollisionPointRec(GetMousePosition(), bar) ||
                 CheckCollisionPointRec(GetMousePosition(), knob);

      if (!input_consumed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hit) {
        slider.dragging = true;
      }

      if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) slider.dragging = false;

      if (slider.dragging) {
        float mouse_t = (GetMouseX() - bar.x) / bar.width;
        mouse_t = Clamp(mouse_t, 0.f, 1.f);

        float raw = slider.min + mouse_t * (slider.max - slider.min);
        float snapped = std::round(raw / slider.step) * slider.step;
        float new_value = Clamp(snapped, slider.min, slider.max);

        if (*slider.value != new_value) {
          *slider.value = new_value;
          if (slider.on_change) slider.on_change(new_value);
        }
      }
    });
}

inline void RenderCheckboxes(ECS& ecs, bool input_consumed) {
  ecs.group_view<UICheckboxComponent, UIResolvedRectComponent>(
    [&](Entity, UICheckboxComponent& checkbox,
        UIResolvedRectComponent& resolved) {
      Rectangle rect = ScaleRectCached(resolved);

      float fontSize = 10 * uiScale;
      float box_size = 16.f * uiScale;
      float box_y = rect.y + (rect.height - box_size) * 0.5f;

      Rectangle box{rect.x, box_y, box_size, box_size};

      DrawWin95Box(box, LIGHTGRAY);

      if (*checkbox.value) {
        DrawLine(box.x + 3 * uiScale, box.y + 8 * uiScale, box.x + 7 * uiScale,
                 box.y + 12 * uiScale, BLACK);
        DrawLine(box.x + 7 * uiScale, box.y + 12 * uiScale,
                 box.x + 13 * uiScale, box.y + 3 * uiScale, BLACK);
      }

      DrawText(checkbox.label.c_str(), box.x + 24 * uiScale,
               rect.y + (rect.height - fontSize) * 0.5f, fontSize, BLACK);

      if (!input_consumed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
          CheckCollisionPointRec(GetMousePosition(), box)) {
        *checkbox.value = !(*checkbox.value);
        if (checkbox.on_change) checkbox.on_change(*checkbox.value);
      }
    });
}

inline void RenderButtons(ECS& ecs, bool input_consumed) {
  ecs.group_view<UIButtonComponent, UIResolvedRectComponent>(
    [&](Entity, UIButtonComponent& button, UIResolvedRectComponent& resolved) {
      Rectangle rect = ScaleRectCached(resolved);

      DrawWin95Box(rect, LIGHTGRAY);

      float fontSize = 10 * uiScale;
      float text_w = button.text_width * uiScale;

      DrawText(button.label.c_str(), rect.x + (rect.width - text_w) * 0.5f,
               rect.y + (rect.height - fontSize) * 0.5f, fontSize, BLACK);

      if (!input_consumed && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
          CheckCollisionPointRec(GetMousePosition(), rect)) {
        button.clicked = true;
        if (button.on_click) button.on_click();
        button.clicked = false;
      }
    });
}

inline void RenderText(ECS& ecs) {
  ecs.group_view<UITextComponent, UIResolvedRectComponent>(
    [&](Entity, UITextComponent& text, UIResolvedRectComponent& resolved) {
      Rectangle rect = ScaleRectCached(resolved);

      DrawText(text.text.c_str(), rect.x,
               rect.y + (rect.height - 10.f * uiScale) * 0.5f, 10 * uiScale,
               BLACK);
    });
}

inline void RenderDropdowns(ECS& ecs, bool& input_consumed) {
  Vector2 mouse = GetMousePosition();

  ecs.group_view<UIDropdownComponent, UIResolvedRectComponent>(
    [&](Entity e, UIDropdownComponent& dropdown,
        UIResolvedRectComponent& resolved) {
      Rectangle rect = ScaleRectCached(resolved);

      DrawWin95Box(rect, LIGHTGRAY);

      std::string display =
        dropdown.label + ": " + dropdown.options[*dropdown.selected_index];

      float text_w = dropdown.SelectedWidth() * uiScale;

      DrawText(display.c_str(), rect.x + (rect.width - text_w) * 0.5f,
               rect.y + (rect.height - 10.f * uiScale) * 0.5f, 10 * uiScale,
               BLACK);

      bool clicked_main_box = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                              CheckCollisionPointRec(mouse, rect);

      if (dropdown.expanded && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
          !clicked_main_box && !input_consumed) {
        dropdown.expanded = false;
      }

      if (clicked_main_box) dropdown.expanded = !dropdown.expanded;

      if (dropdown.expanded) {
        float option_h = 20.f * uiScale;

        for (size_t i = 0; i < dropdown.options.size(); ++i) {
          Rectangle option_rect{rect.x, rect.y + rect.height + i * option_h,
                                rect.width, option_h};

          bool hover = CheckCollisionPointRec(mouse, option_rect);

          DrawWin95Box(option_rect,
                       hover ? Color{225, 225, 225, 255} : LIGHTGRAY);

          DrawText(dropdown.options[i].c_str(), option_rect.x + 4 * uiScale,
                   option_rect.y + (option_rect.height - 10.f * uiScale) * 0.5f,
                   10 * uiScale, BLACK);

          if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && hover) {
            *dropdown.selected_index = static_cast<int>(i);
            dropdown.expanded = false;

            if (dropdown.on_select) dropdown.on_select(dropdown.options[i]);

            input_consumed = true;
          }
        }
      }
    });
}

inline void PrepassDropdownInput(ECS& ecs, bool& input_consumed) {
  Vector2 mouse = GetMousePosition();

  ecs.group_view<UIDropdownComponent, UIResolvedRectComponent>(
    [&](Entity, UIDropdownComponent& dropdown,
        UIResolvedRectComponent& resolved) {
      if (!dropdown.expanded) return;

      Rectangle rect = ScaleRectCached(resolved);
      float option_h = 20.f * uiScale;

      Rectangle total_list_rect{rect.x, rect.y + rect.height, rect.width,
                                option_h * dropdown.options.size()};

      if (CheckCollisionPointRec(mouse, total_list_rect)) input_consumed = true;
    });
}

inline void RenderTooltips(ECS& ecs, bool input_consumed) {
  Vector2 mouse = GetMousePosition();
  float dt = GetFrameTime();
  std::string tooltip_to_draw = "";

  ecs.view<components::UITooltipComponent, components::UIResolvedRectComponent>(
    [&](Entity e, components::UITooltipComponent& tooltip,
        components::UIResolvedRectComponent& resolved) {
      bool is_hovering = !input_consumed && CheckCollisionPointRec(
                                              mouse, ScaleRectCached(resolved));
      if (is_hovering) {
        tooltip.hover_timer += dt;
        if (tooltip.hover_timer >= tooltip.delay) {
          tooltip_to_draw = tooltip.text;
          if (!tooltip.logged_visible) {
            logger::debug("[GUI] Tooltip appeared: '{}' (entity:{}:{})",
                          tooltip.text, e.index, e.version);
            tooltip.logged_visible = true;
          }
        }
      } else if (tooltip.hover_timer > 0.0f) {
        tooltip.hover_timer = 0.0f;
        tooltip.logged_visible = false;
      }
    });

  if (!tooltip_to_draw.empty()) {
    float fontSize = 10 * uiScale;
    int textWidth = MeasureText(tooltip_to_draw.c_str(), fontSize);
    float tipWidth = (float)textWidth + 10 * uiScale;
    float tipHeight = fontSize + 8 * uiScale;

    float tipX = mouse.x + 12;
    if (tipX + tipWidth > GAME_W * SCALE) tipX = mouse.x - tipWidth - 12;

    float tipY = mouse.y + 12;
    if (tipY + tipHeight > GAME_W * SCALE) tipY = mouse.y - tipHeight - 12;

    Rectangle tipRect = {tipX, tipY, tipWidth, tipHeight};

    DrawRectangleRec(tipRect, {255, 255, 225, 255});
    DrawRectangleLinesEx(tipRect, 1, BLACK);
    DrawText(tooltip_to_draw.c_str(), tipRect.x + 5 * uiScale,
             tipRect.y + 4 * uiScale, fontSize, BLACK);
  }
}

inline void RenderUI(ECS& ecs) {
  LayoutUI(ecs);

  bool input_consumed = false;

  PrepassDropdownInput(ecs, input_consumed);

  RenderWindow(ecs);
  RenderGroups(ecs);
  RenderSliders(ecs, input_consumed);
  RenderCheckboxes(ecs, input_consumed);
  RenderButtons(ecs, input_consumed);
  RenderText(ecs);
  RenderDropdowns(ecs, input_consumed);

  RenderTooltips(ecs, input_consumed);
}

}  // namespace motrix::engine::systems
