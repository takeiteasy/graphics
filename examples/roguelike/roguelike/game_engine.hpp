#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <array>
#include <algorithm>
#include "vec2.hpp"
#include "../../../graphics.h"
#include "menu_state.hpp"

class game_engine_t {
  bool running = true,
       active = true,
       mouse_enabled = true;
  int screen_w = 640,
    screen_h = 480;
  surface_t bb;
  std::vector<std::unique_ptr<game_state_t>> states;

  struct mouse_t {
    std::array<bool, MOUSE_LAST - 1> btns;
    vec2i pos;
    vec2f wheel;
    KEYMOD mod;
  } mouse_handler, last_mouse_handler;

  struct keyboard_t {
    std::array<bool, KB_KEY_LAST> keys;
    KEYMOD mod;
  } keyboard_handler, last_keyboard_handler;

  void on_keyboard(KEYSYM sym, KEYMOD mod, bool down) {
    keyboard_handler.keys[sym] = down;
    keyboard_handler.mod = mod;
  }

  void on_mouse_button(MOUSEBTN btn, KEYMOD mod, bool down) {
    mouse_handler.btns[btn - 1] = down;
    mouse_handler.mod = mod;
  }

  void on_mouse_move(int x, int y, int dx, int dy) {
    mouse_handler.pos.set((int)(((float)x / (float)screen_w) * bb.w),
      (int)(((float)y / (float)screen_h) * bb.h));
  }

  void on_scroll(KEYMOD mod, float dx, float dy) {
    mouse_handler.wheel.set(dx, dy);
    mouse_handler.mod = mod;
  }

  void on_active(bool focused) {
    active = focused;
  }

  void on_resize(int w, int h) {
    screen_w = w;
    screen_h = h;
    sgl_fill(&bb, BLACK);
    sgl_writelnf(&bb, 4, 5, WHITE, -1, "%dx%d\n", w, h);
  }

  void push_impl(std::unique_ptr<game_state_t> state) {
    state->enter(*this);
    states.push_back(std::move(state));
  }

public:
  game_engine_t(int argc, const char* argv[]) {
    sgl_set_userdata(this);

#define XMAP \
      X(keyboard) \
      X(mouse_button) \
      X(mouse_move) \
      X(scroll) \
      X(active) \
      X(resize)

#define X(a) \
      sgl_##a##_callback([](void* data, auto... args) { \
        static_cast<game_engine_t*>(data)->on_##a(args...); \
      });
    XMAP
#undef X
#undef XMAP

      if (not sgl_screen("the caverns below", &bb, screen_w, screen_h, RESIZABLE))
        std::cerr << "Couldn't start game. See you, buddy..." << std::endl;

    keyboard_handler.keys.fill(false);
    mouse_handler.btns.fill(false);

    push<menu_state_t>();
  }

  ~game_engine_t() {
    sgl_destroy(&bb);
    sgl_release();
  }

  int run() {
    long prev_frame_tick;
    long curr_frame_tick = sgl_ticks();
    while (not sgl_closed() and running) {
      prev_frame_tick = curr_frame_tick;
      curr_frame_tick = sgl_ticks();
      last_keyboard_handler = keyboard_handler;
      last_mouse_handler = mouse_handler;
      sgl_poll();

#if defined(SGL_OSX)
      if ((keyboard_handler.keys[KB_KEY_Q] or keyboard_handler.keys[KB_KEY_W]) and keyboard_handler.mod == KB_MOD_SUPER)
#else
      if (keyboard_handler.keys[KB_KEY_F4] and keyboard_handler.mod == KB_MOD_ALT)
#endif
        running = false;

      if (not states.empty())
        states.back()->update(*this, prev_frame_tick - curr_frame_tick);

      sgl_fill(&bb, BLACK);
      if (active and not states.empty())
        states.back()->render(*this);
      sgl_flush(&bb);

      mouse_handler.wheel.set(0.f, 0.f);
      mouse_handler.mod = static_cast<KEYMOD>(0);
    }
    return EXIT_SUCCESS;
  }

  template<typename T, typename... Args> void push(Args... args) {
    push_impl(std::make_unique<T>(args...));
  }

  void pop() {
    if (not states.empty()) {
      states.back()->exit(*this);
      states.pop_back();
    }
  }

  bool is_key_down(KEYSYM key) {
    return keyboard_handler.keys[key];
  }

  bool last_key_down(KEYSYM key) {
    return last_keyboard_handler.keys[key];
  }

  KEYMOD key_mod() {
    return keyboard_handler.mod;
  }

  KEYMOD last_key_mod() {
    return last_keyboard_handler.mod;
  }

  bool key_pressed(KEYSYM key) {
    return (last_keyboard_handler.keys[key] && !keyboard_handler.keys[key]);
  }

  bool is_btn_down(MOUSEBTN btn) {
    return mouse_handler.btns[btn];
  }

  bool last_btn_down(MOUSEBTN btn) {
    return last_mouse_handler.btns[btn];
  }

  bool btn_pressed(MOUSEBTN btn) {
    return (last_mouse_handler.btns[btn] && !mouse_handler.btns[btn]);
  }

  KEYMOD btn_mod() {
    return mouse_handler.mod;
  }

  KEYMOD last_btn_mod() {
    return last_mouse_handler.mod;
  }

  const vec2i& mouse_pos() {
    return mouse_handler.pos;
  }

  const vec2i& last_mouse_pos() {
    return last_mouse_handler.pos;
  }

  const vec2f& mouse_wheel() {
    return mouse_handler.wheel;
  }

  const vec2f& last_mouse_wheel() {
    return last_mouse_handler.wheel;
  }

  surface_t& buffer() {
    return bb;
  }

  operator surface_t*() {
    return &bb;
  }
};