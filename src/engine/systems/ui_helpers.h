// engine/systems/ui_helpers.h
#pragma once

#include "../components/ui.h"
#include "../components/ui_traits.h"

namespace motrix::engine::systems {

inline Rectangle ScaleRect(Rectangle rect) {
  return {rect.x * uiScale, rect.y * uiScale, rect.width * uiScale,
          rect.height * uiScale};
}

inline Rectangle ScaleRectCached(
  components::UIResolvedRectComponent& resolved) {
  if (resolved.scaled_rect.width != resolved.rect.width ||
      resolved.scaled_rect.height != resolved.rect.height ||
      resolved.scaled_rect.x != resolved.rect.x ||
      resolved.scaled_rect.y != resolved.rect.y) {
    resolved.scaled_rect = {
      resolved.rect.x * uiScale, resolved.rect.y * uiScale,
      resolved.rect.width * uiScale, resolved.rect.height * uiScale};
  }
  return resolved.scaled_rect;
}

inline void DrawWin95Box(Rectangle rect, Color fill) {
  DrawRectangleRec(rect, fill);
  DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y, WHITE);
  DrawLine(rect.x, rect.y, rect.x, rect.y + rect.height, WHITE);
  DrawLine(rect.x, rect.y + rect.height, rect.x + rect.width,
           rect.y + rect.height, DARKGRAY);
  DrawLine(rect.x + rect.width, rect.y, rect.x + rect.width,
           rect.y + rect.height, DARKGRAY);
}

inline float ResolveIntrinsicWidth(ECS& ecs, Entity entity) {
  using namespace motrix::engine::components;
  if (ecs.has<UIButtonComponent>(entity))
    return UIComponentTraits<UIButtonComponent>::Width(
      ecs.get<UIButtonComponent>(entity));
  if (ecs.has<UICheckboxComponent>(entity))
    return UIComponentTraits<UICheckboxComponent>::Width(
      ecs.get<UICheckboxComponent>(entity));
  if (ecs.has<UITextComponent>(entity))
    return UIComponentTraits<UITextComponent>::Width(
      ecs.get<UITextComponent>(entity));
  if (ecs.has<UISliderComponent>(entity))
    return UIComponentTraits<UISliderComponent>::Width(
      ecs.get<UISliderComponent>(entity));
  return 0.f;
}

}  // namespace motrix::engine::systems
