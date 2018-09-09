#pragma once
#include "game_state.hpp"
#include "cave.hpp"
#include "panel.hpp"
#include <future>
#include <memory>

class main_state_t : public game_state_t {
  std::vector<std::unique_ptr<dungeon_t>> maps;

  enum game_state {
    LOADING,
    PLAYING
  };
  game_state state;

  enum panel_ids : int {
    MAIN,
    INV
  };

  std::vector<std::string> loading_info;
  std::string loading_str = "Loading.....";
  std::future<bool> load_thrd;
  int loading_ticks = 5;
  long last_loading_tick = 0;
  int current_map = 0;

  std::vector<std::unique_ptr<panel_t>> panels;

  void add_panel(int w, int h, int x, int y, const char* title = nullptr);
  template<typename T, typename... Args> void add_map(Args... args);

public:
  void enter(game_engine_t& e) override;
  void update(game_engine_t& e, long t) override;
  void render(game_engine_t& e) override;
  void exit(game_engine_t& e) override;
};