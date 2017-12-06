#include "app.h"

#define WIDTH  640
#define HEIGHT 480

#define RND_255 (rand() % 256)
#define RND_RBG RGB(RND_255, RND_255, RND_255)

void fill_rnd(surface_t* s) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      pset(s, x, y, RND_RBG);
}

int main(int argc, const char* argv[]) {
  surface_t* win = screen("test", WIDTH, HEIGHT);
  if (!win)
    return 1;
  
  surface_t* test = surface(200, 200);
  rect_filled(test, 0, 0, 100, 100, SIENNA);
  rect_filled(test, 100, 0, 100, 100, CORNFLOWERBLUE);
  rect_filled(test, 0, 100, 100, 100, LAWNGREEN);
  rect_filled(test, 100, 100, 100, 100, WHEAT);
  
  surface_t* rnd = surface(50, 50);
  
  surface_t* c = load_bmp_from_file("/Users/roryb/Desktop/Uncompressed-24.bmp");
  
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
    
    xline(win, 135, 110, 160, INDIGO);
    yline(win, 135, 110, 160, INDIGO);
    line(win, 0, 0, 300, 300, THISTLE);
    circle(win, 300, 300, 30, THISTLE);
    circle_filled(win, 350, 350, 30, THISTLE);
    rect(win, 425, 125, 150, 150, PALEGOLDENROD);
    rect_filled(win, 450, 150, 100, 100, PALEGOLDENROD);
    
    fill_rnd(rnd);
    blit(win, &tmpp3, rnd, NULL);
    
    print_f(test, 10, 50, WHITE, "this is a test: %dx%d\nI hope this works.", test->w, test->h);
    
    if (!redraw())
      break;
  }
  
  destroy(&c);
  destroy(&test);
  destroy(&rnd);
  release();
  return 0;
}

