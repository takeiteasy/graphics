#include "graphics.h"

/* TODO:
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

static bool running = true, chroma_on = false;
static point_t p = { 0, 0 };

void test_cb_move(int x, int y) {
  p.x = x;
  p.y = y;
}

void test_cb_kdown(KEY_e k, MOD_e mod) {
  if (k == KEY_Q && mod == MOD_SUPER)
    running = false;
  else if (k == KEY_SPACE) {
    if (chroma_on) {
      set_chroma_key(-1);
      chroma_on = false;
    } else {
      set_chroma_key(WHITE);
      chroma_on = true;
    }
  }
}

int main(int argc, const char* argv[]) {
  surface_t* win = screen(NULL, 640, 480);
  if (!win) {
    fprintf(stderr, "%s\n", get_last_error());
    return 1;
  }
  
  key_down_cb(test_cb_kdown);
  mouse_move_cb(test_cb_move);
  
  surface_t* c = bmp("/Users/roryb/Documents/Uncompressed-24.bmp");
  
  while (running) {
    fill(win, RGB(100, 149, 237));
    
    yline(win, p.x, -10, 490, RED);
    xline(win, p.y, -10, 650, LIME);
    line(win, -10, -10, 650, 490, YELLOW);
    line(win, -10, 490, 650, -10, YELLOW);
    
    circle(win, p.x, p.y, 30, BLUE, false);
    
    print_f(win, 10, 8, WHITE, "mouse: (x:%d, y:%d)\nticks: %ld \002", p.x, p.y, ticks());
    
    if (!redraw())
      break;
  }
  
  destroy(&c);
  release();
  return 0;
}

