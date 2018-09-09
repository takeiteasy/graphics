#pragma once
#include "game_state.hpp"

class menu_state_t : public game_state_t {
public:
  void enter(game_engine_t& e) override;
  void update(game_engine_t& e, long t) override;
  void render(game_engine_t& e) override;
  void exit(game_engine_t& e) override;
};