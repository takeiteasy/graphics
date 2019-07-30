#include <stdio.h>
#include "hal.h"

static screen_t win;
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

int main(int argc, const char* argv[]) {
  hal_error_callback(on_error);

  hal_screen(&win, "test",  SW, SH, RESIZABLE);
  
  hal_cursor_lock(true);

  hal_surface(&buf, SW, SH);
  hal_fill(buf, RED);

  surface_t img;
  hal_bmp(&img, "/Users/roryb/git/hal/old/test/resources/Uncompressed-24.bmp");
  hal_paste(buf, img, 10, 10);

  while (!hal_closed(win) && running) {
    hal_poll();
    hal_flush(win, buf);
  }

  hal_destroy(&buf);
  hal_screen_destroy(&win);
  return 0;
}

