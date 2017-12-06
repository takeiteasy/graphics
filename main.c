#include "app.h"

#define WIDTH  640
#define HEIGHT 480

#define RNDRGB (rand() % 256)

void fill_rnd(surface_t* s) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      pset(s, x, y, RNDRGB, RNDRGB, RNDRGB);
}

int main(int argc, const char* argv[]) {
  surface_t* win = screen("test", WIDTH, HEIGHT);
  if (!win)
    return 1;
  
  surface_t* test = surface(200, 200);
  rect_filled(test, 0, 0, 100, 100, 255, 0, 0);
  rect_filled(test, 100, 0, 100, 100, 0, 255, 0);
  rect_filled(test, 0, 100, 100, 100, 0, 0, 255);
  rect_filled(test, 100, 100, 100, 100, 255, 255, 255);
  
  surface_t* rnd = surface(50, 50);
  
  surface_t* c = load_bmp_from_file("/Users/roryb/Desktop/charset.bmp");
  
  rect_t  tmpr  = { 0, 100, 100, 100 };
  point_t tmpp  = { 0, 22 };
  point_t tmpp1  = { 101, 22 };
  point_t tmpp2 = { 5, 227 };
  point_t tmpp3 = { 475, 175 };
  
  int noise, carry, seed = 0xBEEF, i;
  for (;;) {
    for (i = 0; i < WIDTH * HEIGHT; ++i) {
      noise = seed;
      noise >>= 3;
      noise ^= seed;
      carry = noise & 1;
      noise >>= 1;
      seed >>= 1;
      seed |= (carry << 30);
      noise &= 0xFF;
      win->buf[i] = RGB(noise, noise, noise);
    }
    // fill_surface(win, 0, 0, 0);
    
    blit(win, &tmpp1, test, NULL);
    blit(win, &tmpp, test, &tmpr);
    
    blit(win, &tmpp2, c, NULL);
    
    xline(win, 135, 110, 160, 255, 255, 255);
    yline(win, 135, 110, 160, 255, 255, 255);
    line(win, 0, 0, 300, 300, 255, 255, 0);
    circle(win, 300, 300, 30, 255, 255, 0);
    circle_filled(win, 350, 350, 30, 255, 255, 0);
    rect(win, 425, 125, 150, 150, 0, 255, 255);
    rect_filled(win, 450, 150, 100, 100, 0, 255, 255);
    
    fill_rnd(rnd);
    blit(win, &tmpp3, rnd, NULL);
    
    print_f(test, 0, 0, 255, 255, 255, "this is a test: %dx%d\nI hope this works.", test->w, test->h);
    
    if (!redraw())
      break;
  }
  
  destroy(&c);
  destroy(&test);
  destroy(&rnd);
  release();
  return 0;
}

