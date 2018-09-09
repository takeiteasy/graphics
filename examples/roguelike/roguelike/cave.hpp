#pragma once
#include "dungeon.hpp"

class cave_t : public dungeon_t {
  int w, h, fill_prob, iterations, survival, starve;
  std::vector<std::string>& info_ref;

  vector2d<int> cellular_automata();
  void get_rooms(const vector2d<int>& map);
  void update_main_room_connections(int r, vector2d<bool>& rci, vector1d<bool>& mci);
  void draw_connection(vector2d<int>& map, const vec2i* a, const vec2i* b);
  void connect_closest_rooms(vector2d<int>& map, const vector1d<room_t>& rooms_a, const vector1d<room_t>& rooms_b, vector2d<bool>& rci, vector1d<bool>& mci);
  void connect_rooms(vector2d<int>& map);
  void finalize_map(vector2d<int>& map);

public:
  cave_t(std::vector<std::string>& info,
         int _w = 0,
         int _h = 0,
         int _fill_prob = 40,
         int _iterations = 5,
         int _survival = 4,
         int _starve = 3);
  void generate() override;
};