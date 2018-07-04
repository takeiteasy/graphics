#include "graphics_bdf.h"
#include "graphics_image.h"
#include "graphics_colours.h"
#include "graphics.h"

int invert(int x, int y, int c) {
  return RGB(255 - ((c >> 16) & 0xFF), 255 - ((c >> 8) & 0xFF), 255 - (c & 0xFF));
}

int greyscale(int x, int y, int c) {
  int r, g, b;
  rgb(c, &r, &g, &b);
  int gc = 0.2126 * r + 0.7152 * g + 0.0722 * b;
  return RGB(gc, gc, gc);
}

#define RND_255 (rand() % 256)

int rnd(int x, int y, int c) {
  return RGB(RND_255, RND_255, RND_255);
}

static surface_t win;
static int win_w = 0, win_h = 0, mx = 0, my = 0, running = 1;

typedef struct {
  surface_t* s;
  point_t* p;
  rect_t* r;
} sprite_t;

void blit_sprite(surface_t* dst, sprite_t* src, float opacity, int chroma) {
  blit(dst, src->p, src->s, src->r, opacity, chroma);
}

#define DEBUG_NATIVE_RESIZE 1

void on_resize(int w, int h) {
  win_w = w;
  win_h = h;
#if !DEBUG_NATIVE_RESIZE
  reset(&win, w, h);
#else
  cls(&win);
#endif
  writelnf(&win, 4, 5, WHITE, -1, "%dx%d\n", w, h);
}

#define SW 640
#define SH 480

#if !defined(RES_PATH)
#if defined(__APPLE__)
#define RES_PATH "/Users/roryb/Documents/git/graphics.h/tests/"
#elif defined(_WIN32)
#define RES_PATH "C:\\Users\\DESKTOP\\Documents\\graphics.h\\tests\\"
#else
#define RES_PATH "/home/reimu/Desktop/graphics.h/tests/"
#endif
#endif
#define RES_JOIN(X,Y) (X Y)
#define RES(X) (RES_JOIN(RES_PATH, X))

int main(int argc, const char* argv[]) {
  if (!screen("test", SW, SH)) {
    fprintf(stderr, "%s\n", get_last_error());
    return 1;
  }
  win_w = SW;
  win_h = SH;
  resize_callback(on_resize);
  surface(&win, SW, SH);

  surface_t a, b, c, d, e, f, g, h, k, l;
  surface(&a, 50, 50);
  
  if (!image(&b, RES("test_alpha.png"), LIME)) {
    fprintf(stderr, "%s\n", get_last_stb_error());
    return 1;
  }
  
  if (!bmp(&g, RES("lena.bmp"))) {
    fprintf(stderr, "%s\n", get_last_error());
    return 1;
  }
  resize(&g, g.w / 2, g.h / 2, &c);
  destroy(&g);
  
  // BDF example from tewi-font: https://github.com/lucy/tewi-font
  bdf_t tewi;
  if (!bdf(&tewi, RES("tewi.bdf"))) {
    fprintf(stderr, "%s\n", get_last_bdf_error());
    return 1;
  }
  
  copy(&c, &e);
  filter(&e, greyscale);

  rect_t  tmpr  = { 125, 120, 50, 50 };
  point_t tmpp  = { 10, 150 };
  point_t tmpp2 = { 5, 227 };
  point_t tmpp3 = { 350, 125 };
  point_t tmpp4 = { tmpp2.x + c.w + 5, tmpp2.y };
  point_t tmpp5 = { 10, 110 };
  point_t tmpp6 = { 425, 110 };
  point_t tmpp7 = { 482, 170 };
  point_t tmpp8 = { 530, 370 };
  point_t tmpp9 = { 100, 20 };

  stringf(&d, RED, LIME, "cut from the\nimage below\nx: %d y: %d\nw: %d h: %d", tmpr.x, tmpr.y, tmpr.w, tmpr.h);
  string(&h, RED, LIME, "NO\nGREEN\nHERE");
  string(&k, LIME, BLACK, "WOW");
  surface(&l, 50, 50);
  fill(&l, BLACK);
  point_t tmmp8 = { 13, 20 };
  BLIT(&l, &tmmp8, &k, NULL);
  destroy(&k);

  surface(&f, 100, 100);
  rect(&f, 0,  0,  50, 50, RED, 1);
  rect(&f, 50, 50, 50, 50, LIME, 1);
  rect(&f, 50, 0,  50, 50, BLUE, 1);
  rect(&f, 0,  50, 50, 50, YELLOW, 1);

  int col = 0;
  long sine_i = 0;
  user_event_t ue;
  long prev_frame_tick;
  long curr_frame_tick = ticks();
  while (running) {
    prev_frame_tick = curr_frame_tick;
    curr_frame_tick = ticks();
    
    while (poll_events(&ue)) {
      switch (ue.type) {
        case WINDOW_CLOSED:
          running = 0;
          break;
        case KEYBOARD_KEY_DOWN:
          switch (ue.sym) {
#if defined(__APPLE__)
            case KB_KEY_Q:
              if (ue.mod & KB_MOD_SUPER)
                running = 0;
              break;
#else
            case KB_KEY_F4:
              if (ue.mod & KB_MOD_ALT)
                running = 0;
              break;
#endif
            case KB_KEY_SPACE:
              save_bmp(&win, "test.bmp");
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
    }
    
    long speed = curr_frame_tick - prev_frame_tick;

//    cls(&win);
    fill(&win, WHITE);
    
    for (int x = 32; x < win.w; x += 32)
      vline(&win, x, 0, win.h, GRAY);
    for (int y = 32; y < win.h; y += 32)
      hline(&win, y, 0, win.w, GRAY);
    
    blit(&win, &tmpp9, &b, NULL, -1, LIME);
    
    writeln(&win, 10, 10, RED, -1, "Hello World");
    writeln(&win, 10, 22, MAROON, -1, "こんにちはせかい");
    bdf_writeln(&win, &tewi, 10, 34, WHITE, BLACK, "ΔhelloΔ bdf!");

    int last_x = 0, last_y = 150;
    for (long i = sine_i; i < (sine_i + win.w); ++i) {
      float x = (float)(i - sine_i);
      float y = 150.f + (100.f * sinf(i * (3.141f / 180.f)));
      line(&win, last_x, last_y, x, y, col);
      last_x = x;
      last_y = y;
    }
    sine_i += (int)(speed * .3);

    blit(&win, &tmpp5, &d, NULL, -1, LIME);
    blit(&win, &tmpp, &c, &tmpr, -1, LIME);

    blit(&win, &tmpp2, &c, NULL, -1, LIME);
    blit(&win, &tmpp4, &e, NULL, -1, LIME);

    blit(&win, &tmpp6, &f, NULL, -1, LIME);
    blit(&win, &tmpp7, &h, NULL, -1, LIME);

    blit(&win, &tmpp8, &f, NULL, .5f, -1);

    filter(&a, rnd);
    blit(&a, NULL, &l, NULL, -1, LIME);
    blit(&win, &tmpp3, &a, NULL, -1, LIME);

    circle(&win, 352, 32, 30, RED,    1);
    circle(&win, 382, 32, 30, ORANGE, 1);
    circle(&win, 412, 32, 30, YELLOW, 1);
    circle(&win, 442, 32, 30, LIME,   1);
    circle(&win, 472, 32, 30, BLUE,   1);
    circle(&win, 502, 32, 30, INDIGO, 1);
    circle(&win, 532, 32, 30, VIOLET, 1);

    writelnf(&win, 400, 88, BLACK, -1, "mouse x,y: (%d, %d)", mx, my);
    get_mouse_pos(&mx, &my);
#if DEBUG_NATIVE_RESIZE
    mx = (int)(((float)mx / (float)win_w) * win.w);
    my = (int)(((float)my / (float)win_h) * win.h);
#endif
    col = pget(&win, mx, my);
//    rect(&win, 150, 50,  100, 100, col, 0);
//    rect(&win, 200, 100, 100, 100, col, 0);
//    line(&win, 150, 50,  200, 100, col);
//    line(&win, 250, 50,  300, 100, col);
//    line(&win, 150, 150, 200, 200, col);
//    line(&win, 250, 150, 300, 200, col);

    line(&win, 0, 0, mx, my, col);
    circle(&win, mx, my, 30, col, 0);
    
    render(&win);
  }

  bdf_destroy(&tewi);
  destroy(&win);
  destroy(&a);
  destroy(&b);
  destroy(&c);
  destroy(&d);
  destroy(&e);
  destroy(&h);
  destroy(&l);
  destroy(&k);
  destroy(&l);
  release();
  return 0;
}
