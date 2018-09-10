#include "dungeon.hpp"

void dungeon_t::fill_map(vector2d<int>& _map) {
  map.resize(_map.size());
  int i, j, x;
  for (i = 0; i < map.size(); ++i) {
    map[i].resize(_map[i].size());
    for (j = 0; j < _map[i].size(); ++j) {
      x = _map[i][j];
      if (x == 0)
        map[i][j] = { false, '.' };
      else if (x >= 1 and x <= 9)
        map[i][j] = { true, static_cast<char>(34 + x) };
      else
        map[i][j] = { true, ' ' };
    }
  }
}

void dungeon_t::fill_entities() {
  
}

dungeon_t::dungeon_t() {
  rnd_eng = std::mt19937(rnd_dev());
}

bool dungeon_t::is_loaded() {
  return loaded;
}

#define TILE_MAP_MAX_H 8
#define RGB_INC (160 / TILE_MAP_MAX_H)

void dungeon_t::draw_to(surface_t* s) {
  int gmw = s->w / 8, gmh = s->h / 8;
  int cx = camera.x - (camera.x % 8), cy = camera.y - (camera.y % 8);
  int off_cx = cx - (gmw / 2);
  int off_cy = cy - (gmh / 2);
  int off_cgx = cx / 8;
  int off_cgy = cy / 8;
  int rgb = 95, i, j, k;
  char c;

  for (k = 0; k < TILE_MAP_MAX_H; ++k) {
    for (i = 0; i < gmw; ++i) {
      for (j = 0; j < gmh; ++j) {
        int gx = i + off_cgx;
        int gy = j + off_cgy;
        if (gx < 0 or gy < 0 or gx >= map.size() or gy >= map[0].size())
          continue;
        
        c = map[gx][gy].map;
        if (k >= tile_map[c].size() or tile_map[c][k] == ' ')
          continue;

        sgl_ascii(s, tile_map[c][k],
                 (i * 8) + (k * ((i + off_cx) - cx)) / draw_distance,
                 (j * 8) + (k * ((j + off_cy) - cy)) / draw_distance,
                 RGB1(rgb), 0);
      }
    }
    rgb += RGB_INC;
  }
}
