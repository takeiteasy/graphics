#pragma once
#include <random>
#include <queue>
#include <thread>
#include <map>
#include "common.hpp"
#include "panel.hpp"

static std::map<char, vector1d<char>> tile_map {
  { ' ', { ' ' } },
  { '.', { '.' } },
  { '#', { '#' } },
  { '$', { '#', '#' } },
  { '%', { '#', '#', '#' } },
  { '&', { '#', '#', '#', '#' } },
  { '\'', { '#', '#', '#', '#', '#' } },
  { '(', { '#', '#', '#', '#', '#', '#' } },
  { ')', { '#', '#', '#', '#', '#', '#', '#' } },
  { '*', { '#', '#', '#', '#', '#', '#', '#', '#' } },
  { '+', { ' ', '#', ' ', '#', ' ', '#', ' ', '#' } },
  { '?', { '?', '?', '?', '?', '?', '?', '?', '?' } }
};

typedef enum {
  FLOOR,
  WALL,
  ABYSS,
  EXIT
} TILE_TYPE;

typedef struct {
  bool solid;
  char map;
  TILE_TYPE type;
} tile_t;

typedef struct {
  int id;
  vector1d<vec2i> tiles, edges;
} room_t;

class dungeon_t {
  std::random_device rnd_dev;
  int uid;
  
protected:
  std::mt19937 rnd_eng;
  volatile bool loaded = false;
  vector1d<room_t> rooms;
  int main_room;
  vector2d<tile_t> map;
  int draw_distance = 5;
  int depth = 0;
  vec2i size = { 0, 0 };
  dungeon_t* parent = nullptr;

  std::vector<std::unique_ptr<dungeon_t>> sub_maps;
  template<typename T, typename... Args> void add_map(Args... args);
  void fill_map(vector2d<int>& _map);

public:
  dungeon_t();
  virtual void generate() = 0;
  bool is_loaded();
  int id();
  const vec2i& wh();
  void draw_to(surface_t* s, const vec2i& camera);
};
