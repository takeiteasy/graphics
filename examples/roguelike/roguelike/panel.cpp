#include "panel.hpp"

panel_t::panel_t(int w, int h, int x, int y, const char* title) {
  sgl_surface(&bb, w, h);
  pos.x = x;
  pos.y = y;
  if (title)
    name = title;
  aabb = { x, y, x + w, y + h };
}

panel_t::~panel_t() {
  sgl_destroy(&bb);
}

void panel_t::clear() {
  sgl_cls(&bb);
}

void panel_t::draw_to(surface_t* s) {
  sgl_rect(s, pos.x - 1, pos.y - 1, bb.w + 1, bb.h + 1, WHITE, false);
  sgl_blit(s, &pos, &bb, NULL);
  if (name)
    sgl_writeln(s, pos.x + 5, pos.y - 4, WHITE, BLACK, name);
}

panel_t::operator surface_t*() {
  return &bb;
}