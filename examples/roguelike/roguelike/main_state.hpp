#pragma once
#include "game_state.hpp"
#include "cave.hpp"
#include "panel.hpp"
#include <thread>
#include <memory>

class player_t {
public:
};

class dungeon_handler_t {
  dungeon_t *main_map, *current_map;
  player_t* p = nullptr;
  vec2i camera = { 0, 0 };
  
public:
  void init(player_t* _p) {
    p = _p;
    main_map = new cave_t();
    current_map = main_map;
    std::thread([&]() {
      main_map->generate();
    }).detach();
  }
  
  ~dungeon_handler_t() {
    delete main_map;
  }
  
  void move_camera(int x, int y) {
    camera.x += x;
    camera.y += y;
  }
  
  const vec2i& view() {
    return camera;
  }
  
  bool is_current_map_loaded() {
    return current_map->is_loaded();
  }
  
  void draw_current_map_to(surface_t* s) {
    current_map->draw_to(s, camera);
  }
};

class main_state_t : public game_state_t {
  std::vector<std::unique_ptr<dungeon_t>> maps;
  dungeon_handler_t map_handler;
  player_t player;
  
  typedef enum {
    LOADING,
    PLAYING
  } GAME_STATE;
  GAME_STATE state = LOADING;

  enum PANEL_IDS : int {
    MAIN,
    INV
  };
  
  std::string loading_str = "Loading.....";
  int loading_ticks = 5;
  long last_loading_tick = 0;

  std::vector<std::unique_ptr<panel_t>> panels;
  void add_panel(int w, int h, int x, int y, const char* title = nullptr);
  
  bool dragging = false;

public:
  void enter(game_engine_t& e) override;
  void update(game_engine_t& e, long t) override;
  void render(game_engine_t& e) override;
  void exit(game_engine_t& e) override;
};
