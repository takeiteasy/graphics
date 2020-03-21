#include <stdio.h>
#include "graphics/graphics.h"

static struct window_t win;
static struct surface_t buf;
static bool running = true;

#define SW 575
#define SH 500

#if 0
#define printf
#endif

void on_error(GRAPHICS_ERROR_TYPE type, const char* msg, const char* file, const char* func, int line) {
#if defined(GRAPHICS_DIALOGS)
  alert(ALERT_ERROR, ALERT_OK, "ERROR! See logs for info");
#else
  fprintf(stderr, "ERROR ENCOUNTERED: %s\nFrom %s, in %s() at %d\n", msg, file, func, line);
#endif
  abort();
}

void on_keyboard(void* _, KEY_SYM sym, KEY_MOD mod, bool down) {
  if (sym == KB_KEY_ESCAPE)
    running = false;
  if (sym == KB_KEY_SPACE) {
    char** paths = NULL;
    int n_paths = dialog(DIALOG_OPEN, &paths, "C:\\", NULL, false, 1, "bmp");
    for (int i = 0; i < n_paths; ++i) {
      printf("%s\n", paths[i]);
      free(paths[i]);
    }
    if (paths)
      free(paths);
  }
  printf("keyboard: %d is %s\n", sym, (down ? "down" : "up"));
}

void on_mouse_btn(void* _, MOUSE_BTN btn, KEY_MOD mod, bool down) {
  printf("mouse btn: %d is %s\n", (int)btn, (down ? "down" : "up"));
}

void on_mouse_move(void* _, int x, int y, int dx, int dy) {
#if defined(GRAPHICS_EMCC)
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
#if defined(GRAPHICS_EMCC)
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
  graphics_error_callback(on_error);

  window(&win, "test",  SW, SH, RESIZABLE);
  window_callbacks(on_keyboard, on_mouse_btn, on_mouse_move, on_scroll, on_focus, on_resize, on_closed, &win);

  //struct surface_t test_icon;
  //#define TEST_SIZE 32
  //surface(&test_icon, TEST_SIZE, TEST_SIZE);
  //rect(&test_icon, 0, 0, TEST_SIZE / 4, TEST_SIZE / 4, RED, true);
  //rect(&test_icon, TEST_SIZE / 4, TEST_SIZE / 4, TEST_SIZE / 4, TEST_SIZE / 4, BLUE, true);
  //rect(&test_icon, 0, TEST_SIZE / 4, TEST_SIZE / 4, TEST_SIZE / 4, YELLOW, true);
  //rect(&test_icon, TEST_SIZE / 4, 0, TEST_SIZE / 4, TEST_SIZE / 4, LIME, true);
  //window_icon(&win, &test_icon);
  //cursor_icon_custom(&win, &test_icon);

  surface(&buf, SW, SH);
  fill(&buf, RED);

  struct surface_t img;
#if defined(GRAPHICS_EMCC)
  bmp(&img, "old/test/resources/Uncompressed-24.bmp");
#elif defined(GRAPHICS_OSX)
  bmp(&img, "/Users/roryb/git/hal/old/test/resources/Uncompressed-24.bmp");
#elif defined(GRAPHICS_WINDOWS)
  bmp(&img, "C:\\Users\\Rory B. Bellows\\git\\graphics\\tests\\bmp\\g\\rgb32.bmp");
#endif
  paste(&buf, &img, 10, 30);
  
#if !defined(GRAPHICS_NO_BDF)
  struct bdf_t font;
#if defined(GRAPHICS_EMCC)
  bdf(&font, "old/test/resources/tewi.bdf");
#elif defined(GRAPHICS_OSX)
  bdf(&font, "/Users/roryb/git/hal/old/test/resources/tewi.bdf");
#elif defined(GRAPHICS_WINDOWS)
  bdf(&font, "C:\\Users\\Rory B. Bellows\\git\\graphics\\tests\\tewi.bdf");
#endif
  bdf_writelnf(&buf, &font, 10, 10, WHITE, BLACK, "This is a test! %d", 42);
#endif


#if defined(GRAPHICS_EMCC)
  emscripten_set_main_loop(loop, 0, 1);
#else
  while (!closed(&win) && running)
    loop();
#endif

  surface_destroy(&img);
#if !defined(GRAPHICS_NO_BDF)
  bdf_destroy(&font);
#endif
  surface_destroy(&buf);
  window_destroy(&win);
  return 0;
}

