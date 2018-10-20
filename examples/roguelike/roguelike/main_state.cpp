#include "game_engine.hpp"
#include "main_state.hpp"

using namespace std::chrono_literals;

void main_state_t::add_panel(int w, int h, int x, int y, const char* title) {
  panels.emplace_back(std::make_unique<panel_t>(w, h, x, y, title));
}

void main_state_t::enter(game_engine_t& e) {
  map_handler.init(&player);
}

void main_state_t::update(game_engine_t& e, long t) {
  switch (state) {
    case LOADING:
      if (map_handler.is_current_map_loaded()) {
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
      dragging = (e.is_btn_down(MOUSE_BTN_1));
      if (dragging)
        map_handler.move_camera(e.last_mouse_pos().x - e.mouse_pos().x, e.last_mouse_pos().y - e.mouse_pos().y);
      
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
      break;
    case PLAYING: {
      for (auto&& p : panels)
        p->clear();
      
      map_handler.draw_current_map_to(*panels[MAIN]);
      
      sgl_writelnf(*panels[INV], 5, 5, WHITE, 0, "MOUSE: %d, %d", e.mouse_pos().x, e.mouse_pos().y);
      sgl_line(*panels[MAIN], 0, 0, e.mouse_pos().x, e.mouse_pos().y, RED);
      
      for (auto&& p : panels)
        p->draw_to(e);
      break;
    }
  }
}

void main_state_t::exit(game_engine_t& e) {
  maps.clear();
  panels.clear();
}
