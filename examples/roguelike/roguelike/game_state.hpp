#pragma once
class game_engine_t;

class game_state_t {
public:
  virtual void enter(game_engine_t&) = 0;
  virtual void update(game_engine_t&, long) = 0;
  virtual void render(game_engine_t&) = 0;
  virtual void exit(game_engine_t&) = 0;
};