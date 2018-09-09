#include "game_engine.hpp"
#include "menu_state.hpp"
#include "main_state.hpp"

void menu_state_t::enter(game_engine_t& e) {

}

void menu_state_t::update(game_engine_t& e, long t) {
  if (e.key_pressed(KB_KEY_SPACE))
    e.push<main_state_t>();
}

void menu_state_t::render(game_engine_t& e) {
  sgl_writeln(e, 5, 5, WHITE, 0, "This is the menu\nPRESS SPACE");
}

void menu_state_t::exit(game_engine_t& e) {

}