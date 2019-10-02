#include <stdio.h>
#include "hal.h"

static window_t win;
static surface_t buf;
static bool running = true;

#define SW 575
#define SH 500

void on_error(ERROR_TYPE type, const char* msg, const char* file, const char* func, i32 line) {
  fprintf(stderr, "ERROR ENCOUNTERED: %s\nFrom %s, in %s() at %d\n", msg, file, func, line);
#if defined(HAL_DIALOGS)
  hal_alert(ALERT_ERROR, ALERT_OK, "ERROR! See logs for info");
  abort();
#endif
}

void on_keyboard(void* _, KEY_SYM sym, KEY_MOD mod, bool down) {
  printf("keyboard: %d is %s\n", sym, (down ? "down" : "up"));
}

void on_mouse_btn(void* _, MOUSE_BTN btn, KEY_MOD mod, bool down) {
  printf("mouse btn: %d is %s\n", (int)btn, (down ? "down" : "up"));
}

void on_mouse_move(void* _, int x, int y, int dx, int dy) {
#if defined(HAL_EMCC)
  static int wx, wy;
  hal_window_position(win, &wx, &wy);
  printf("mouse move: %d,%d - %d,%d\n", x - wx, y - wy, dx, dy);
#else
  printf("mouse move: %d,%d - %d,%d\n", x, y, dx, dy);
#endif
}

void on_scroll(void* _, KEY_MOD mod, float dx, float dy) {
  printf("scroll: %f %f\n", dx, dy);
}

void on_focus(void* _, bool focused) {
  printf("%s\n", (focused ? "FOCUSED" : "UNFOCUSED"));
}

void on_resize(void* _, int w, int h) {
  printf("resize: %d %d\n", w, h);
}

void on_closed(void* _) {
#if defined(HAL_EMCC)
  emscripten_cancel_main_loop();
#else
  running = false;
#endif
}

void loop() {
  hal_poll();
  hal_flush(win, buf);
}

int main(int argc, const char* argv[]) {
  hal_error_callback(on_error);

  hal_window(&win, "test",  SW, SH, DEFAULT);
  //hal_window_callbacks(on_keyboard, on_mouse_btn, on_mouse_move, on_scroll, on_focus, on_resize, on_closed, win);
  hal_cursor_icon(win, CURSOR_HAND);

  hal_surface(&buf, SW, SH);
  hal_fill(buf, RED);

  surface_t img;
#if defined(HAL_EMCC)
  hal_bmp(&img, "old/test/resources/Uncompressed-24.bmp");
#elif defined(HAL_OSX)
  hal_bmp(&img, "/Users/roryb/git/hal/old/test/resources/Uncompressed-24.bmp");
#elif defined(HAL_WINDOWS)
  hal_bmp(&img, "Z:\\hal\\old\\test\\resources\\Uncompressed-24.bmp");
#endif
  hal_paste(buf, img, 10, 10);

#if defined(HAL_EMCC)
  emscripten_set_main_loop(loop, 0, 1);
#else
  while (!hal_closed(win) && running)
    loop();
#endif

  hal_destroy(&buf);
  hal_window_destroy(&win);
  return 0;
}

