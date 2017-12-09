#define GRAPHICS_EXTRA_COLORS
#include "graphics.h"

/* TODO:
 *  - Chroma key to blit
 *  - RLE8/4 loading + OS/2 BMP
 *  - ANSI colour escapes for print()
 *  - HSV, HSL, INT to RGB functions
 *  - Window event callbacks mouse, keyboard, etc
 *  - Optional API init, like SDL
 *  - Extended surface functions, rotate, filters, etc
 *  - stb_image/stb_truetype extras
 *  - Test on Linux/Windows
 *  - Add copyright for borrowed stuff
 *  - Native system timer
 *  - OpenGL alternative backend ???
 */

#define WIDTH  640
#define HEIGHT 480

#define RND_255 (rand() % 256)
#define RND_RBG RGB(RND_255, RND_255, RND_255)

static int mx = 0, my = 0;

void test_cb_move(int x, int y) {
  mx = x;
  my = y;
  printf("(%d, %d)\n", x, y);
}

void test_cb_enter(bool entered) {
  printf("%s!\n", (entered ? "entered" : "exited"));
}

void fill_rnd(surface_t* s) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      pset(s, x, y, RND_RBG);
}

int main(int argc, const char* argv[]) {
  surface_t* win = screen("test", WIDTH, HEIGHT);
  if (!win) {
    fprintf(stderr, "%s\n", get_last_error());
    return 1;
  }
  
  mouse_move_cb(test_cb_move);
  mouse_entered_cb(test_cb_enter);
  
  surface_t* test = surface(200, 200);
  rect(test, 0, 0, 100, 100, MAROON, true);
  rect(test, 100, 0, 100, 100, DARKRED, true);
  rect(test, 0, 100, 100, 100, FIREBRICK, true);
  rect(test, 100, 100, 100, 100, BROWN, true);
  
  surface_t* rnd = surface(50, 50);
  
  surface_t* c = load_bmp_from_file("/Users/roryb/Desktop/Uncompressed-24.bmp");
  surface_t* d = string(WHITE, "test string");
  
  rect_t  tmpr = { 0, 100, 100, 100 };
  point_t tmpp = { 0, 22 };
  point_t tmpp1 = { 101, 22 };
  point_t tmpp2 = { 5, 227 };
  point_t tmpp3 = { 475, 175 };
  
  for (;;) {
    fill(win, CORNFLOWERBLUE);
    
    blit(win, &tmpp1, test, NULL);
    blit(win, &tmpp, test, &tmpr);
    
    blit(win, &tmpp2, c, NULL);
    blit(win, &tmpp2, d, NULL);
    
    xline(win, 135, 110, 160, THISTLE);
    yline(win, 135, 110, 160, THISTLE);
    line(win, 0, 0, mx, my, THISTLE);
    circle(win, mx, my, 30, THISTLE, false);
    circle(win, 350, 350, 30, RND_RBG, true);
    rect(win, 425, 125, 150, 150, PALEGOLDENROD, false);
    rect(win, 450, 150, 100, 100, PALEGOLDENROD, true);
    
    fill_rnd(rnd);
    blit(win, &tmpp3, rnd, NULL);
    
    print_f(test, 10, 50, WHITE, "this is a test: %dx%d\nI hope this works.", test->w, test->h);
    
    if (!redraw())
      break;
  }
  
  destroy(&c);
  destroy(&d);
  destroy(&test);
  destroy(&rnd);
  release();
  return 0;
}

