#include "graphics.h"

/* TODO:
 *  - Replace font system
 *  - ANSI colour escapes for print()
 *  - Test on Linux/Windows
 *  - Add copyright for borrowed stuff
 *  - OpenGL alternative backend ???
 *
 * EXTRA:
 *  - Extended surface functions, resize, rotate, filters, etc
 *  - stb_image/stb_truetype
 */

#define TICK_INTERVAL 30
static long next_time;

long time_left(void) {
  long now = ticks();
  return (next_time <= now ? 0 : next_time - now);
}

static int mx = 0, my = 0, running = 1;

int invert(int x, int y, int c) {
  return RGB(255 - ((c >> 16) & 0xFF), 255 - ((c >> 8) & 0xFF), 255 - (c & 0xFF));
}

#define RND_255 (rand() % 256)

int rnd(int x, int y, int c) {
  return RGB(RND_255, RND_255, RND_255);
}

int main(int argc, const char* argv[]) {
  surface_t* win = screen(NULL, 640, 480);
  if (!win) {
    fprintf(stderr, "%s\n", get_last_error());
    return 1;
  }
  
  surface_t* a = surface(50, 50);
  surface_t* c = bmp("/Users/roryb/Documents/Uncompressed-24.bmp");
  surface_t* e = copy(c);
  iterate(e, invert);
  
  rect_t  tmpr  = { 150, 50, 50, 50 };
  point_t tmpp  = { 10, 150 };
  point_t tmpp2 = { 5, 227 };
  point_t tmpp3 = { 350, 125 };
  point_t tmpp4 = { tmpp2.x + c->w + 5, tmpp2.y };
  point_t tmpp5 = { 10, 110 };
  point_t tmpp6 = { 425, 110 };
  point_t tmpp7 = { 482, 170 };
  
  surface_t* d = string_f(RED, LIME, "cut from the\nimage below\nx: %d y: %d\nw: %d h: %d", tmpr.x, tmpr.y, tmpr.w, tmpr.h);
  surface_t* h = string(RED, LIME, "NO\nGREEN\nHERE");
  surface_t* k = string(LIME, BLACK, "WOW");
  surface_t* l = surface(50, 50);
  fill(l, BLACK);
  point_t tmmp8 = { 13, 20 };
  blit(l, &tmmp8, k, NULL, -1);
  destroy(&k);
  
  surface_t* f = surface(100, 100);
  rect(f, 0,  0,  50, 50, RGB(255, 0, 0), true);
  rect(f, 50, 50, 50, 50, RGB(0, 255, 0), true);
  rect(f, 50, 0,  50, 50, RGB(0, 0, 255), true);
  rect(f, 0,  50, 50, 50, RGB(255, 255, 0), true);
  
  int r, g, b, col;
  user_event_t ue;
  next_time = ticks() + TICK_INTERVAL;
  while (running) {
    fill(win, WHITE);
    
    for (int x = 32; x < win->w; x += 32)
      yline(win, x, 0, win->h, GRAY);
    for (int y = 32; y < win->h; y += 32)
      xline(win, y, 0, win->w, GRAY);
    
    blit(win, &tmpp5, d, NULL, LIME);
    blit(win, &tmpp, c, &tmpr, LIME);
    
    blit(win, &tmpp2, c, NULL, LIME);
    blit(win, &tmpp4, e, NULL, LIME);
    
    blit(win, &tmpp6, f, NULL, LIME);
    blit(win, &tmpp7, h, NULL, LIME);
    
    circle(win, 352, 32, 30, RGB(255, 0,   0),   true);
    circle(win, 382, 32, 30, RGB(255, 127, 0),   true);
    circle(win, 412, 32, 30, RGB(255, 255, 0),   true);
    circle(win, 442, 32, 30, RGB(0,   255, 0),   true);
    circle(win, 472, 32, 30, RGB(0,   0,   255), true);
    circle(win, 502, 32, 30, RGB(75,  0,   130), true);
    circle(win, 532, 32, 30, RGB(148, 0,   211), true);
    
    iterate(a, rnd);
    blit(a, NULL, l, NULL, LIME);
    blit(win, &tmpp3, a, NULL, LIME);
    
    print_f(win, 10, 8, BLACK, "mouse x,y: (%d, %d)\nA S T H E T I C", mx, my);
    
    get_mouse_pos(&mx, &my);
    col = pget(win, mx, my);
    rgb(col, &r, &g, &b);
    rect(win, 15, 50, 10, 10, RGB(r, 0, 0), true);
    rect(win, 35, 50, 10, 10, RGB(0, g, 0), true);
    rect(win, 55, 50, 10, 10, RGB(0, 0, b), true);
    print_f(win, 15, 40, RED, "rgb(%d, %d, %d)", r, g, b);
    
    rect(win, 150, 50,  100, 100, col, false);
    rect(win, 200, 100, 100, 100, col, false);
    line(win, 150, 50,  200, 100, col);
    line(win, 250, 50,  300, 100, col);
    line(win, 150, 150, 200, 200, col);
    line(win, 250, 150, 300, 200, col);
    
    line(win, 0, 0, mx, my, col);
    circle(win, mx, my, 30, col, false);
    
    while (poll_events(&ue)) {
      switch (ue.type) {
        case WINDOW_CLOSED:
          running = 0;
          break;
        case KEYBOARD_KEY_DOWN:
#if defined(__APPLE__)
          if (ue.sym == KB_KEY_Q && ue.mod & KB_MOD_SUPER)
            running = 0;
#endif
          break;
        default:
          break;
      }
    }
    render();
    delay(time_left());
    next_time += TICK_INTERVAL;
  }
  
  destroy(&c);
  destroy(&d);
  destroy(&e);
  destroy(&a);
  destroy(&h);
  destroy(&l);
  release();
  return 0;
}
