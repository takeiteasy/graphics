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
static int win_w = 640, win_h = 480, mx = 0, my = 0, running = 1;

#define DEBUG_NATIVE_RESIZE 0

void update_mxy() {
  mouse_xy(&mx, &my);
#if DEBUG_NATIVE_RESIZE
  mx = (int)(((float)mx / (float)win_w) * win.w);
  my = (int)(((float)my / (float)win_h) * win.h);
#endif
}

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
  if (!surface(&win, 640, 480)) {
    fprintf(stderr, "%s\n", last_error());
    return 1;
  }
  if (!screen("test", &win_w, &win_h, RESIZABLE)) {
    fprintf(stderr, "%s\n", last_error());
    return 1;
  }
#if !DEBUG_NATIVE_RESIZE
  if (win.w != win_w || win.h != win_h)
    reset(&win, win_w, win_h);
#endif
  resize_callback(on_resize);

  surface_t s[10];
  surface(&s[0], 50, 50);
  
  if (!image(&s[1], RES("test_alpha.png"), LIME)) {
    fprintf(stderr, "%s\n", get_last_stb_error());
    return 1;
  }
  
  if (!bmp(&s[6], RES("lena.bmp"))) {
    fprintf(stderr, "%s\n", last_error());
    return 1;
  }
  resize(&s[6], s[6].w / 2, s[6].h / 2, &s[2]);
  destroy(&s[6]);
  
  // BDF example from tewi-font: https://github.com/lucy/tewi-font
  bdf_t tewi;
  if (!bdf(&tewi, RES("tewi.bdf"))) {
    fprintf(stderr, "%s\n", get_last_bdf_error());
    return 1;
  }
  
  copy(&s[2], &s[4]);
  filter(&s[4], invert);
  
  point_t points[9] = {
    { 10,  150 },
    { 5,   227 },
    { 350, 125 },
    { 0,   0   },
    { 10,  110 },
    { 425, 110 },
    { 482, 170 },
    { 530, 370 },
    { 100, 20  }
  };
  points[3].x = points[1].x + s[2].w + 5;
  points[3].y = points[1].y;

  rect_t cutr  = { 125, 120, 50, 50 };
  stringf(&s[3], RED, LIME, "cut from the\nimage below\nx: %d y: %d\nw: %d h: %d", cutr.x, cutr.y, cutr.w, cutr.h);
  string(&s[7], RED, LIME, "NO\nGREEN\nHERE");
  string(&s[8], LIME, BLACK, "WOW");
  surface(&s[9], 50, 50);
  fill(&s[9], BLACK);
  point_t tmmp8 = { 13, 20 };
  BLIT(&s[9], &tmmp8, &s[8], NULL);
  destroy(&s[8]);

  surface(&s[5], 100, 100);
  rect(&s[5], 0,  0,  50, 50, RED, 1);
  rect(&s[5], 50, 50, 50, 50, LIME, 1);
  rect(&s[5], 50, 0,  50, 50, BLUE, 1);
  rect(&s[5], 0,  50, 50, 50, YELLOW, 1);

  int col = 0, grey = 0;
  long sine_i = 0;
  event_t e;
  long prev_frame_tick;
  long curr_frame_tick = ticks();
  while (!closed() && running) {
    prev_frame_tick = curr_frame_tick;
    curr_frame_tick = ticks();

    while (poll(&e)) {
      switch (e.type) {
        case WINDOW_CLOSED:
          running = 0;
          break;
        case KEYBOARD_KEY_DOWN:
          switch (e.sym) {
#if defined(__APPLE__)
            case KB_KEY_Q:
              if (e.mod & KB_MOD_SUPER)
                running = 0;
              break;
#else
            case KB_KEY_F4:
              if (ue.mod & KB_MOD_ALT)
                running = 0;
              break;
#endif
            case KB_KEY_SPACE:
              grey = 1;
              break;
            default:
              break;
          }
          break;
        case KEYBOARD_KEY_UP:
          switch (e.sym) {
            case KB_KEY_SPACE:
              grey = 0;
              break;
            case KB_KEY_F1:
              save_bmp(&win, "test.bmp");
              break;
            default:
              break;
          }
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
    
    blit(&win, &points[8], &s[1], NULL, -1, LIME);
    
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

    blit(&win, &points[4], &s[3], NULL, -1, LIME);
    blit(&win, &points[0], &s[2], &cutr, -1, LIME);

    blit(&win, &points[1], &s[2], NULL, -1, LIME);
    blit(&win, &points[3], &s[4], NULL, -1, LIME);

    blit(&win, &points[5], &s[5], NULL, -1, LIME);
    blit(&win, &points[6], &s[7], NULL, -1, LIME);

    blit(&win, &points[7], &s[5], NULL, .5f, -1);

    filter(&s[0], rnd);
    blit(&s[0], NULL, &s[9], NULL, -1, LIME);
    blit(&win, &points[2], &s[0], NULL, -1, LIME);

    circle(&win, 352, 32, 30, RED,    1);
    circle(&win, 382, 32, 30, ORANGE, 1);
    circle(&win, 412, 32, 30, YELLOW, 1);
    circle(&win, 442, 32, 30, LIME,   1);
    circle(&win, 472, 32, 30, BLUE,   1);
    circle(&win, 502, 32, 30, INDIGO, 1);
    circle(&win, 532, 32, 30, VIOLET, 1);

    update_mxy();
    writelnf(&win, 400, 88, BLACK, -1, "mouse x,y: (%d, %d)", mx, my);
    col = pget(&win, mx, my);
//    rect(&win, 150, 50,  100, 100, col, 0);
//    rect(&win, 200, 100, 100, 100, col, 0);
//    line(&win, 150, 50,  200, 100, col);
//    line(&win, 250, 50,  300, 100, col);
//    line(&win, 150, 150, 200, 200, col);
//    line(&win, 250, 150, 300, 200, col);

    line(&win, 0, 0, mx, my, col);
    circle(&win, mx, my, 30, col, 0);
    
    if (grey)
      filter(&win, greyscale);
    
    flush(&win);
  }

  bdf_destroy(&tewi);
  destroy(&win);
  for (int i = 0; i < (int)(sizeof(s) / sizeof(s[0])); ++i)
    destroy(&s[i]);
  release();
  return 0;
}
