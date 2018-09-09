#pragma once
#include "../../../graphics.h"

class panel_t {
  surface_t bb;
  point_t pos;
  const char* name = nullptr;
  rect_t aabb;

public:
  panel_t(int w, int h, int x, int y, const char* title = nullptr);
  ~panel_t();

  void clear();
  void draw_to(surface_t* s);

  operator surface_t*();
};