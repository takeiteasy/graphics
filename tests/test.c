#include "../graphics.h"

int invert(int x, int y, int c) {
  return RGB(255 - ((c >> 16) & 0xFF), 255 - ((c >> 8) & 0xFF), 255 - (c & 0xFF));
}

#define RND_255 (rand() % 256)

int rnd(int x, int y, int c) {
  return RGB(RND_255, RND_255, RND_255);
}

#define PRINT_CHAR_MAP(fn, min_i, max_i, c) \
for (i = min_i; i < max_i; ++i, x += 8) { \
  if (x + 8 > win.w) { \
    x  = 5; \
    y += 8; \
  } \
  fn(&win, i, x, y, c); \
} \
x  = 5;\
y += 8;

static surface_t win;
static int win_w = 0, win_h = 0, mx = 0, my = 0, running = 1;

#define DEBUG_NATIVE_RESIZE 1
#define DEBUG_TGA 0

void on_resize(int w, int h) {
  win_w = w;
  win_h = h;
#if DEBUG_NATIVE_RESIZE == 0
  reset(&win, w, h);
  print_f(&win, 4, 5, WHITE, "%dx%d\n", w, h);
#endif
}

#define INITIAL_WIN_W 640
#define INITIAL_WIN_H 480

int main(int argc, const char* argv[]) {
  if (!screen("test", INITIAL_WIN_W, INITIAL_WIN_H)) {
    fprintf(stderr, "%s\n", get_last_error());
    return 1;
  }
  win_w = INITIAL_WIN_W;
  win_h = INITIAL_WIN_H;
  resize_callback(on_resize);
  surface(&win, INITIAL_WIN_W, INITIAL_WIN_H);

  surface_t a, b, c, d, e, f, g, h, k, l;
  surface(&a, 50, 50);
  
#if DEBUG_TGA
#define IMG_EXT ".tga"
#else
#define IMG_EXT ".bmp"
#endif
  
#if defined(__APPLE__)
#define IMG_PATH "/Users/roryb/Documents/git/graphics.h/tests/lena"
#elif defined(_WIN32)
#define IMG_PATH "C:\\Users\\DESKTOP\\Documents\\graphics.h\\tests\\lena"
#else
#define IMG_PATH "/home/reimu/Desktop/graphics.h/tests/lena"
#endif
  
#define PATH_JOIN(a, b) a b
  
#if DEBUG_TGA
  if (!tga(&c, PATH_JOIN(IMG_PATH, IMG_EXT))) {
#else
  if (!bmp(&c, PATH_JOIN(IMG_PATH, IMG_EXT))) {
#endif
    fprintf(stderr, "%s\n", get_last_error());
    return 1;
  }
  
  copy(&c, &e);
  iterate(&e, invert);

  rect_t  tmpr  = { 280, 320, 50, 50 };
  point_t tmpp  = { 10, 150 };
  point_t tmpp2 = { 5, 227 };
  point_t tmpp3 = { 350, 125 };
  point_t tmpp4 = { tmpp2.x + c.w + 5, tmpp2.y };
  point_t tmpp5 = { 10, 110 };
  point_t tmpp6 = { 425, 110 };
  point_t tmpp7 = { 482, 170 };
  point_t tmpp8 = { 525, 370 };

  string_f(&d, RED, LIME, "cut from the\nimage below\nx: %d y: %d\nw: %d h: %d", tmpr.x, tmpr.y, tmpr.w, tmpr.h);
  string(&h, RED, LIME, "NO\nGREEN\nHERE");
  string(&k, LIME, BLACK, "WOW");
  surface(&l, 50, 50);
  fill(&l, BLACK);
  point_t tmmp8 = { 13, 20 };
  blit(&l, &tmmp8, &k, NULL, -1, -1);
  destroy(&k);

  surface(&f, 100, 100);
  rect(&f, 0,  0,  50, 50, RED, 1);
  rect(&f, 50, 50, 50, 50, LIME, 1);
  rect(&f, 50, 0,  50, 50, BLUE, 1);
  rect(&f, 0,  50, 50, 50, YELLOW, 1);

  int col = 0, i, x, y;
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

    fill(&win, WHITE);
    
    for (int x = 32; x < win.w; x += 32)
      yline(&win, x, 0, win.h, GRAY);
    for (int y = 32; y < win.h; y += 32)
      xline(&win, y, 0, win.w, GRAY);

    int last_x = 0, last_y = 240;
    for (long i = sine_i; i < (sine_i + win.w); ++i) {
      float x = (float)(i - sine_i);
      float y = 240.f + (100.f * sinf(i * (3.141f / 180.f)));
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

    iterate(&a, rnd);
    blit(&a, NULL, &l, NULL, -1, LIME);
    blit(&win, &tmpp3, &a, NULL, -1, LIME);

    circle(&win, 352, 32, 30, RED,    1);
    circle(&win, 382, 32, 30, ORANGE, 1);
    circle(&win, 412, 32, 30, YELLOW, 1);
    circle(&win, 442, 32, 30, LIME,   1);
    circle(&win, 472, 32, 30, BLUE,   1);
    circle(&win, 502, 32, 30, INDIGO, 1);
    circle(&win, 532, 32, 30, VIOLET, 1);

    print_f(&win, 400, 88, BLACK, "mouse x,y: (%d, %d)", mx, my);

    x = 5;
    y = 5;
    PRINT_CHAR_MAP(letter, 33, 128, MAROON);
    PRINT_CHAR_MAP(letter_block, 0, 32, DARK_RED);
    PRINT_CHAR_MAP(letter_box, 0, 128, BROWN);
    PRINT_CHAR_MAP(letter_extra, 0, 132, FIREBRICK);
    PRINT_CHAR_MAP(letter_greek, 0, 58, CRIMSON);
    PRINT_CHAR_MAP(letter_hiragana, 0, 96, RED);

    get_mouse_pos(&mx, &my);
#if DEBUG_NATIVE_RESIZE
    mx = (int)(((float)mx / (float)win_w) * win.w);
    my = (int)(((float)my / (float)win_h) * win.h);
#endif
    col = pget(&win, mx, my);
    rect(&win, 150, 50,  100, 100, col, 0);
    rect(&win, 200, 100, 100, 100, col, 0);
    line(&win, 150, 50,  200, 100, col);
    line(&win, 250, 50,  300, 100, col);
    line(&win, 150, 150, 200, 200, col);
    line(&win, 250, 150, 300, 200, col);

    line(&win, 0, 0, mx, my, col);
    circle(&win, mx, my, 30, col, 0);
    
    render(&win);
  }

  destroy(&win);
  destroy(&a);
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
