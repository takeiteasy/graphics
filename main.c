#include "app.h"

#define WIDTH 640
#define HEIGHT 480

int main(int argc, const char* argv[]) {
  surface_t* test = NULL;
  if (!(test = app_open("test", WIDTH, HEIGHT)))
    return 1;
  atexit(app_close);
  
  for (;;) {
    fill_surface(test, 0, 0, 0);
    
    for (int i = 0; i < 100; ++i) {
      for (int j = 0; j < 100; ++j) {
        pset(test, i, j, 255, 0, 0);
      }
    }
    
    if (!app_update())
      break;
  }
  return 0;
}

