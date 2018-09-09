#include "entity.hpp"

entity_t::entity_t(int x, int y, char c, int fg, int bg) {
  grid_pos = { x, y };
  sgl_ascii(&bb, c, 0, 0, fg, bg);
}

entity_t::~entity_t() {
  sgl_destroy(&bb);
}

void entity_t::move(int x, int y) {
  grid_pos.x += x;
  grid_pos.y += y;
}

const vec2i& entity_t::position() {
  return grid_pos;
}

void entity_t::draw_to(surface_t* s, const vec2i& cam) {

}