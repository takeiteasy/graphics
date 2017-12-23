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

static bool running = true;
static int mx = 0, my = 0;

void test_cb_move(int x, int y) {
  mx = x;
  my = y;
}

void test_cb_kdown(KEY_e k, MOD_e mod) {
  if (k == KEY_Q && mod == MOD_SUPER)
    running = false;
  printf("%d key down\n", k);
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
  point_t p = { 0, 0 };
  
  while (running) {
    fill(win, RGB(100, 149, 237));
    
    p.x = mx;
    p.y = my;
    blit(win, &p, c, NULL);
    
    print_f(win, 10, 8, BLACK, "mouse x,y: (%d, %d)", mx, my);
    
    if (!redraw())
      break;
  }
  
  destroy(&c);
  release();
  return 0;
}

