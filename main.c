#include "app.h"

#define WIDTH  640
#define HEIGHT 480

int rnd_int(int min, int max) {
  return min + rand() % (max+1 - min);
}
#define RNDRGB (rnd_int(0, 255))

void fill_rnd(surface_t* s) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      pset(s, x, y, RNDRGB, RNDRGB, RNDRGB);
}

int main(int argc, const char* argv[]) {
  surface_t* win = NULL;
  if (!(win = app_open("win", WIDTH, HEIGHT)))
    return 1;
  
  surface_t* test = create_surface(200, 200);
  rect_filled(test, 0, 0, 100, 100, 255, 0, 0);
  rect_filled(test, 100, 0, 100, 100, 0, 255, 0);
  rect_filled(test, 0, 100, 100, 100, 0, 0, 255);
  rect_filled(test, 100, 100, 100, 100, 255, 255, 255);
  
  surface_t* rnd = create_surface(50, 50);
  
  point_t tmpp  = { 0, 22 };
  rect_t  tmpr  = { 100, 100, 100, 100 };
  point_t tmpp2 = { 0, 200 };
  point_t tmpp3 = { 475, 175 };
  
  for (;;) {
    fill_surface(win, 0, 0, 0);
    
    blit_surface(win, &tmpp, test, &tmpr);
    blit_surface(win, &tmpp2, test, NULL);
    
    xline(win, 135, 110, 160, 255, 0, 0);
    yline(win, 135, 110, 160, 255, 0, 0);
    line(win, 0, 0, 300, 300, 0, 0, 255);
    circle(win, 300, 300, 30, 0, 255, 0);
    circle_filled(win, 350, 350, 30, 0, 255, 0);
    rect(win, 425, 125, 150, 150, 0, 0, 255);
    rect_filled(win, 450, 150, 100, 100, 0, 0, 255);
    
    fill_rnd(rnd);
    blit_surface(win, &tmpp3, rnd, NULL);
    
    if (!app_update())
      break;
  }
  
  free_surface(&test);
  free_surface(&rnd);
  app_close();
  return 0;
}

