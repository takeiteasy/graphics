#pragma once
#include <random>
#include <queue>
#include <thread>
#include <map>
#include "common.hpp"
#include "entity.hpp"
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
  { '+', { ' ', '#', ' ', '#', ' ', '#', ' ', '#' } }
};

typedef struct {
  bool solid;
  char map;
} tile_t;

typedef struct {
  int id;
  vector1d<vec2i> tiles, edges;
} room_t;

class dungeon_t {
  std::random_device rnd_dev;
  
protected:
  std::mt19937 rnd_eng;
  bool loaded = false;
  vector1d<room_t> rooms;
  int main_room;
  vector2d<tile_t> map;
  vec2i camera = { 0, 0 };
  int draw_distance = 5;

  void fill_map(vector2d<int>& _map);

  vector1d<std::unique_ptr<entity_t>> entities;

public:
  dungeon_t();
  virtual void generate() = 0;
  bool is_loaded();
  void draw_to(surface_t* p);
};