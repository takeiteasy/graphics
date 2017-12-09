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
  
  surface_t* rnd = surface(50, 50);
  
  surface_t* c = load_bmp_from_file("/Users/roryb/Desktop/Uncompressed-24.bmp");
  surface_t* d = string(WHITE, "test string");
  
  rect_t  tmpr = { 150, 50, 50, 50 };
  point_t tmpp = { 10, 150 };
  point_t tmpp2 = { 5, 227 };
  point_t tmpp3 = { 475, 175 };
  
  for (;;) {
    fill(win, WHITE);
    
    for (int x = 32; x < win->w; x += 32)
      yline(win, x, 0, win->h, GRAY);
    for (int y = 32; y < win->h; y += 32)
      xline(win, y, 0, win->w, GRAY);
    
    blit(win, &tmpp2, c, NULL);
    blit(win, &tmpp, c, &tmpr);
    blit(c, NULL, d, NULL);
    
    rect(win, 150, 50, 100, 100, BLUE, false);
    rect(win, 200, 100, 100, 100, BLUE, false);
    line(win, 150, 50, 200, 100, BLUE);
    line(win, 250, 50, 300, 100, BLUE);
    line(win, 150, 150, 200, 200, BLUE);
    line(win, 250, 150, 300, 200, BLUE);
    
    circle(win, 352, 400, 30, RGB(255, 0, 0), true);
    circle(win, 382, 400, 30, RGB(255, 127, 0), true);
    circle(win, 412, 400, 30, RGB(255, 255, 0), true);
    circle(win, 442, 400, 30, RGB(0, 255, 0), true);
    circle(win, 472, 400, 30, RGB(0, 0, 255), true);
    circle(win, 502, 400, 30, RGB(75, 0, 130), true);
    circle(win, 532, 400, 30, RGB(148, 0, 211), true);
    
    
    rect(win, 425, 125, 150, 150, RED, false);
    rect(win, 450, 150, 100, 100, RED, true);
    
    fill_rnd(rnd);
    blit(win, &tmpp3, rnd, NULL);
    
    print_f(win, 10, 8, BLACK, "mouse x,y: (%d, %d)\nA S T H E T I C", mx, my);
    
    line(win, 0, 0, mx, my, MAGENTA);
    circle(win, mx, my, 30, MAGENTA, false);
    
    if (!redraw())
      break;
  }
  
  destroy(&c);
  destroy(&d);
  destroy(&rnd);
  release();
  return 0;
}

