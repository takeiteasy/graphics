#include <stdio.h>
#include <stdlib.h>
#include "graphics/graphics.h"

static struct window_t win;
static struct surface_t buf;
static bool running = true;

#define SW 575
#define SH 500

#if 1
#define printf
#endif
#if 1
#define MULTI_WINDOW_TEST
#endif

#if defined(MULTI_WINDOW_TEST)
static struct window_t win2;
static struct surface_t buf2;
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
#if !defined(GRAPHICS_EMCC)
  if ((sym == KB_KEY_PRINT_SCREEN || sym == KB_KEY_F13) && !down) {
    char** paths = NULL;
    int n_paths = dialog(DIALOG_SAVE, &paths, NULL, NULL, false, 0);
    if (n_paths && paths) {
      if (!save_bmp(&buf, paths[0])) {
        fprintf(stderr, "ERROR: Failed to save BMP file to \"%s\"", paths[0]);
        abort();
      }
      free(paths[0]);
      free(paths);
    }
  }
#endif
  printf("keyboard: %d is %s\n", sym, (down ? "down" : "up"));
}

void on_mouse_btn(void* _, MOUSE_BTN btn, KEY_MOD mod, bool down) {
  printf("mouse btn: %d is %s\n", btn, (down ? "down" : "up"));
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
#elif !defined(MULTI_WINDOW_TEST)
  running = false;
#endif
}

void loop() {
  events();
  flush(&win, &buf);
#if defined(MULTI_WINDOW_TEST)
  flush(&win2, &buf2);
#endif
}

int main(int argc, const char* argv[]) {
  graphics_error_callback(on_error);
  
  window(&win, "test",  SW, SH, RESIZABLE);
  window_callbacks(on_keyboard, on_mouse_btn, on_mouse_move, on_scroll, on_focus, on_resize, on_closed, &win);
  surface(&buf, SW, SH);
  fill(&buf, RED);
  
#if defined(MULTI_WINDOW_TEST)
  window(&win2, "test2",  SW, SH, RESIZABLE);
  window_callbacks(on_keyboard, on_mouse_btn, on_mouse_move, on_scroll, on_focus, on_resize, on_closed, &win2);
  
  surface(&buf2, SW, SH);
  fill(&buf2, BLUE);
#endif

#if 0
  struct surface_t test_icon;
  #define TEST_SIZE 32
  surface(&test_icon, TEST_SIZE, TEST_SIZE);
  rect(&test_icon, 0, 0, TEST_SIZE / 4, TEST_SIZE / 4, RED, true);
  rect(&test_icon, TEST_SIZE / 4, TEST_SIZE / 4, TEST_SIZE / 4, TEST_SIZE / 4, BLUE, true);
  rect(&test_icon, 0, TEST_SIZE / 4, TEST_SIZE / 4, TEST_SIZE / 4, YELLOW, true);
  rect(&test_icon, TEST_SIZE / 4, 0, TEST_SIZE / 4, TEST_SIZE / 4, LIME, true);
  window_icon(&win, &test_icon);
  struct surface_t test_cursor;
  rotate(&test_icon, 90.f, &test_cursor);
  cursor_icon_custom(&win, &test_cursor);
  surface_destroy(&test_icon);
  surface_destroy(&test_cursor);
#endif

cursor_visible(&win, false);

  struct surface_t img;
#if defined(GRAPHICS_EMCC)
  bmp(&img, "tests/bmp/g/rgb32.bmp");
#elif defined(GRAPHICS_OSX)
  bmp(&img, "/Users/roryb/git/hal/tests/bmp/g/rgb32.bmp");
#elif defined(GRAPHICS_WINDOWS)
  bmp(&img, "C:\\Users\\Rory B. Bellows\\git\\graphics\\tests\\bmp\\g\\rgb32.bmp");
#elif defined(GRAPHICS_LINUX)
  bmp(&img, "tests/bmp/g/rgb32.bmp");
#endif
  paste(&buf, &img, 10, 30);
  
#if !defined(GRAPHICS_NO_BDF)
  struct bdf_t font;
#if defined(GRAPHICS_EMCC)
  bdf(&font, "tests/tewi.bdf");
#elif defined(GRAPHICS_OSX)
  bdf(&font, "/Users/roryb/git/hal/tests/tewi.bdf");
#elif defined(GRAPHICS_WINDOWS)
  bdf(&font, "C:\\Users\\Rory B. Bellows\\git\\graphics\\tests\\tewi.bdf");
#elif defined(GRAPHICS_LINUX)
  bdf(&font, "tests/tewi.bdf");
#endif
  bdf_writelnf(&buf, &font, 10, 10, WHITE, BLACK, "This is a test! %d", 42);
#endif


#if defined(GRAPHICS_EMCC)
  emscripten_set_main_loop(loop, 0, 1);
#else
#if defined(MULTI_WINDOW_TEST)
  while (!closed_va(2, &win, &win2) && running)
#else
  while (!closed(&win) && running)
#endif
    loop();
#endif

  surface_destroy(&img);
#if !defined(GRAPHICS_NO_BDF)
  bdf_destroy(&font);
#endif
#if defined(MULTI_WINDOW_TEST)
  surface_destroy(&buf2);
  window_destroy(&win2);
#endif
  surface_destroy(&buf);
  window_destroy(&win);
  return 0;
}

