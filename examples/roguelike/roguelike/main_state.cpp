#include "game_engine.hpp"
#include "main_state.hpp"

using namespace std::chrono_literals;

void main_state_t::add_panel(int w, int h, int x, int y, const char* title) {
  panels.emplace_back(std::make_unique<panel_t>(w, h, x, y, title));
}

template<typename T, typename... Args> void main_state_t::add_map(Args... args) {
  maps.emplace_back(std::make_unique<T>(args...));
}

void main_state_t::enter(game_engine_t& e) {
  map_handler.init(&player);
  add_map<cave_t>(&loading_info, 200, 200);

  load_thrd = std::async(std::launch::async, [&]() {
    maps[0]->generate();
    return true;
  });

  std::future_status load_thrd_status = load_thrd.wait_for(1ns);
  state = (load_thrd_status == std::future_status::ready ? PLAYING : LOADING);
}

void main_state_t::update(game_engine_t& e, long t) {
  switch (state) {
    case LOADING:
      if (load_thrd.wait_for(1ns) == std::future_status::ready) {
        loading_info.clear();
        state = PLAYING;

        add_panel(456, e.buffer().h - 20, 10,  10, " camera ");
        add_panel(152, e.buffer().h - 20, 476, 10, " inventory ");
      }

      if (sgl_ticks() - last_loading_tick > 100) {
        loading_ticks--;
        last_loading_tick = sgl_ticks();
        if (loading_ticks < 0)
          loading_ticks = 5;
      }
      break;
    case PLAYING: {
      if (e.key_pressed(KB_KEY_SPACE))
        e.pop();
      break;
    }
  }
}

void main_state_t::render(game_engine_t& e) {
  switch (state) {
    case LOADING:
      sgl_writeln(e, 5, e.buffer().h - 15, WHITE, 0, loading_str.substr(0, 12 - loading_ticks).c_str());
      for (int i = loading_info.size() - 1, j = e.buffer().h - 25; i > -1; --i, j -= 10)
        sgl_writeln(e, 5, j, RGB1(255 - ((loading_info.size() - i) * 30)), 0, loading_info[i].c_str());
      break;
    case PLAYING: {

      for (auto&& p : panels)
        p->draw_to(e);
      break;
    }
  }
}

void main_state_t::exit(game_engine_t& e) {

}
