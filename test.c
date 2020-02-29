#include <stdio.h>
#include "hal.h"

static struct window_t win;
static struct surface_t buf;
static bool running = true;

#define SW 575
#define SH 500

void on_error(HAL_ERROR_TYPE type, const char* msg, const char* file, const char* func, int line) {
  fprintf(stderr, "ERROR ENCOUNTERED: %s\nFrom %s, in %s() at %d\n", msg, file, func, line);
#if defined(HAL_DIALOGS)
  alert(ALERT_ERROR, ALERT_OK, "ERROR! See logs for info");
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
  window_position(win, &wx, &wy);
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
  events();
  flush(&win, &buf);
}

int main(int argc, const char* argv[]) {
  hal_error_callback(on_error);

  window(&win, "test",  SW, SH, DEFAULT);
  window_callbacks(on_keyboard, on_mouse_btn, on_mouse_move, on_scroll, on_focus, on_resize, on_closed, &win);
  cursor_icon(&win, CURSOR_HAND);

  surface(&buf, SW, SH);
  fill(&buf, RED);

  struct surface_t img;
#if defined(HAL_EMCC)
  bmp(&img, "old/test/resources/Uncompressed-24.bmp");
#elif defined(HAL_OSX)
  bmp(&img, "/Users/roryb/git/hal/old/test/resources/Uncompressed-24.bmp");
#elif defined(HAL_WINDOWS)
  bmp(&img, "Z:\\hal\\old\\test\\resources\\Uncompressed-24.bmp");
#endif
  paste(&buf, &img, 10, 10);
  
  struct bdf_t font;
#if defined(HAL_EMCC)
  bdf(&font, "old/test/resources/tewi.bdf");
#elif defined(HAL_OSX)
  bdf(&font, "/Users/roryb/git/hal/old/test/resources/tewi.bdf");
#elif defined(HAL_WINDOWS)
  bdf(&font, "Z:\\hal\\old\\test\\resources\\tewi.bdf");
#endif
  bdf_writelnf(&buf, &font, 10, 10, WHITE, BLACK, "This is a test! %d", 42);

#if defined(HAL_EMCC)
  emscripten_set_main_loop(loop, 0, 1);
#else
  while (!closed(&win) && running)
    loop();
#endif

  surface_destroy(&img);
  bdf_destroy(&font);
  surface_destroy(&buf);
  window_destroy(&win);
  return 0;
}

