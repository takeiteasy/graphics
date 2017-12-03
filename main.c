#include "app.h"

#define WIDTH  640
#define HEIGHT 480

int main(int argc, const char* argv[]) {
  surface_t* win = NULL;
  if (!(win = app_open("win", WIDTH, HEIGHT)))
    return 1;
  
  surface_t* test = create_surface(200, 200);
  for (int x = 0; x < 100; ++x) {
    for (int y = 0; y < 100; ++y) {
      pset(test, x, y, 255, 0, 0);
      pset(test, x, y + 100, 0, 255, 0);
      pset(test, x + 100, y, 0, 0, 255);
      pset(test, x + 100, y + 100, 255, 255, 0);
    }
  }
  
  for (;;) {
    fill_surface(win, 0, 0, 0);
    
    point_t tmpp = { 0, 22 };
    rect_t  tmpr = { 100, 100, 100, 100 };
    blit_surface(win, &tmpp, test, &tmpr);
    
    point_t tmpp2 = { 0, 200 };
    blit_surface(win, &tmpp2, test, NULL);
    
    if (!app_update())
      break;
  }
  free_surface(&test);
  app_close();
  return 0;
}

