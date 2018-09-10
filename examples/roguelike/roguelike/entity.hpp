#pragma once
#include "vec2.hpp"
#include "../../../graphics.h"

class entity_t {
  vec2i grid_pos;
  surface_t bb;

public:
  entity_t(int x, int y, char c, int fg = WHITE, int bg = 0);
  ~entity_t();

  void move(int x, int y);
  const vec2i& position();
  void draw_to(surface_t* s, const vec2i& cam);
};


class player_t : public entity_t {
public:
  player_t(): entity_t(0, 0, '@') {}
};
