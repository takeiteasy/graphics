#include "app.h"
#include <iostream>
#include <cstdlib>

#define MFB_RGB(r, g, b) (((unsigned int)r) << 16) | (((unsigned int)g) << 8) | b
#define WIDTH 640
#define HEIGHT 480
static unsigned int s_buffer[WIDTH * HEIGHT];

auto main(int argc, const char* argv[]) -> int {
  app_open("test", WIDTH, HEIGHT);
  std::atexit(app_close);
  while (app_update(s_buffer)) {
  }
}
