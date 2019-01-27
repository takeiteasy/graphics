#include "../graphics.h"

int invert(int x, int y, int c) {
  return RGB(255 - R(c), 255 - G(c), 255 - B(c));
}

int greyscale(int x, int y, int c) {
  int gc = 0.2126 * R(c) + 0.7152 * G(c) + 0.0722 * B(c);
  return RGB(gc, gc, gc);
}

#define RND_255 (rand() % 256)
#define RND_RGB (RGB(RND_255, RND_255, RND_255))

int rnd(int x, int y, int c) {
  return RND_RGB;
}

static surface_t win;
static int win_w = 575, win_h = 500, mx = 0, my = 0, running = 1;
static bool grey = false;

#define SKIP_RESIZE 0
#define SKIP_PRINTF 0
#define SKIP_RENDING 0

#if SKIP_PRINTF
#define printf(fmt, ...) (0)
#endif

void on_keyboard(void* data, KEYSYM sym, KEYMOD mod, bool down) {
  if (down) {
    printf("kb: key %d is down\n", sym);
    switch (sym) {
#if defined(__APPLE__)
      case KB_KEY_Q:
      case KB_KEY_W:
        if (mod & KB_MOD_SUPER)
          running = 0;
        break;
  #else
      case KB_KEY_F4:
        if (mod & KB_MOD_ALT)
          running = 0;
        break;
#endif
      case KB_KEY_SPACE:
        grey = true;
        break;
    }
  } else {
    printf("kb: key %d is up\n", sym);
    switch (sym) {
      case KB_KEY_SPACE:
        grey = false;
        break;
      case KB_KEY_F1:
        sgl_save_bmp(&win, "test.bmp");
        break;
#if defined(SGL_ENABLE_STB_IMAGE)
      case KB_KEY_F2:
        sgl_save_image(&win, "test.png", PNG);
        break;
#endif
    }
  }
}

void on_mouse_btn(void* data, MOUSEBTN btn, KEYMOD mod, bool down) {
  printf("mouse btn: %d is %s\n", (int)btn, (down ? "down" : "up"));
}

void on_mouse_move(void* data, int x, int y, int dx, int dy) {
  mx = x;
  my = y;
}

void on_scroll(void* data, KEYMOD mod, float dx, float dy) {
  printf("scroll: %f %f\n", dx, dy);
}

void on_focus(void* data, bool focused) {
  printf("%s\n", (focused ? "FOCUSED" : "UNFOCUSED"));
}

void on_resize(void* data, int w, int h) {
  win_w = w;
  win_h = h;
#if !SKIP_RESIZE
  sgl_reset(&win, w, h);
#else
  cls(&win);
#endif
  sgl_writelnf(&win, 4, 5, WHITE, -1, "%dx%d\n", w, h);
}

// Define RES_PATH or it will use my paths
#if !defined(RES_PATH)
#if defined(__APPLE__)
#define RES_PATH "/Users/roryb/git/graphics.h/tests/"
#elif defined(_WIN32)
#define RES_PATH "C:\\Users\\DESKTOP\\Documents\\graphics.h\\tests\\"
#else
#define RES_PATH "/home/reimu/Desktop/graphics.h/tests/"
#endif
#endif
#define RES_JOIN(X,Y) (X Y)
#define RES(X) (RES_JOIN(RES_PATH, X))

void on_error(void* data, ERRORLVL lvl, ERRORTYPE type, const char* msg, const char* file, const char* func, int line) {
  fprintf(stderr, "ERROR ENCOUNTERED: %s\nFrom %s, in %s() at %d\n", msg, file, func, line);
  if (lvl == HIGH_PRIORITY || lvl == NORMAL_PRIORITY)
    abort();
}

#if defined(SGL_ENABLE_JOYSTICKS)
void on_joystick_connect(void* data, joystick_t* d, int i) {
  printf("%s:%d connected\n", d->description, i);
}

void on_joystick_disconnect(void* data, joystick_t* d, int i) {
  printf("%s:%d disconnected\n", d->description, i);
}

void on_joystick_btn(void* data, joystick_t* d, int btn, bool down, long time) {
  printf("%s:%d bnt %d: %d\n", d->description, d->device_id, btn, down);
}

void on_joystick_axis(void* data, joystick_t* d, int axis, float v, float lv, long time) {
  printf("%s:%d axis: %d: %f %f\n", d->description, d->device_id, axis, v, lv);
}
#endif

int main(int argc, const char* argv[]) {
  sgl_error_callback(on_error);

  sgl_screen("test", &win, win_w, win_h, DEFAULT);
  sgl_screen_callbacks(on_keyboard, on_mouse_btn, on_mouse_move, on_scroll, on_focus, on_resize);
  sgl_cursor(SHOWN, LOCKED, CURSOR_HAND);

#if defined(SGL_ENABLE_JOYSTICKS)
  sgl_joystick_callbacks(on_joystick_connect, on_joystick_disconnect, on_joystick_btn, on_joystick_axis);
  sgl_joystick_init(true);
#endif
  
#define TOTAL_SURFACES 11
  surface_t s[TOTAL_SURFACES];
  for (int i = 0; i < TOTAL_SURFACES; ++i)
    s[i].buf = NULL;
  sgl_surface(&s[0], 50, 50);
  
#if defined(SGL_ENABLE_FREETYPE)
  sgl_ft_init();

  ftfont_t ftf;
  sgl_ftfont(&ftf, "/Library/Fonts/Times New Roman.ttf", 32);
  
  sgl_ftfont_string(&s[10], ftf, WHITE, BLACK, "Hello World!\nThis is a test ABCDEFGHIJKLMNOPQ\nHello Word... Again!");
#endif

#if defined(SGL_ENABLE_STB_IMAGE)
  sgl_image(&s[1], RES("test_alpha.png"));
#endif

  sgl_bmp(&s[6], RES("lena.bmp"));
  sgl_resize(&s[6], s[6].w / 2, s[6].h / 2, &s[2]);
  sgl_destroy(&s[6]);

#if defined(SGL_ENABLE_BDF)
  // BDF font from tewi-font: https://github.com/lucy/tewi-font
  bdf_t tewi;
  sgl_bdf(&tewi, RES("tewi.bdf"));
#endif

  sgl_copy(&s[2], &s[4]);
  sgl_filter(&s[4], invert);

  point_t points[11] = {
    { 10,  150 },
    { 5,   227 },
    { 350, 125 },
    { 0,   0   },
    { 10,  110 },
    { 425, 110 },
    { 482, 170 },
    { 530, 370 },
    { 100, 20  },
    { 15, 400 },
    { 200, 100 }
  };
  points[3].x = points[1].x + s[2].w + 5;
  points[3].y = points[1].y;

  rect_t cutr  = { 125, 120, 50, 50 };
  sgl_stringf(&s[3], RED, LIME, "cut from the\nimage below\nx: %d y: %d\nw: %d h: %d", cutr.x, cutr.y, cutr.w, cutr.h);

  sgl_surface(&s[9], 50, 50);
  sgl_fill(&s[9], BLACK);
  sgl_writeln(&s[9], 13, 20, LIME, BLACK, "WOW");

  sgl_surface(&s[5], 100, 100);
  sgl_rect(&s[5], 0,  0,  50, 50, RGBA(255, 0, 0, 128), 1);
  sgl_rect(&s[5], 50, 50, 50, 50, RGBA(0, 255, 0, 128), 1);
  sgl_rect(&s[5], 50, 0,  50, 50, RGBA(0, 0, 255, 128), 1);
  sgl_rect(&s[5], 0,  50, 50, 50, RGBA(255, 255, 0, 128), 1);
  
  clock_t current_ticks, delta_ticks;
  clock_t fps = 0;
  time_t rt;
  
#if defined(SGL_ENABLE_GIF)
  gif_t ok_hand;
  sgl_gif(&ok_hand, RES("ok.gif"));
#endif
  
  float theta = 1.f;
  int col = 0;
  long sine_i = 0;
  long prev_frame_tick;
  long curr_frame_tick = sgl_ticks();
  int test_alpha = 255;
  while (!sgl_closed() && running) {
    prev_frame_tick = curr_frame_tick;
    curr_frame_tick = sgl_ticks();
#if defined(SGL_ENABLE_JOYSTICKS)
    sgl_joystick_poll();
#endif
    sgl_poll();
    long speed = curr_frame_tick - prev_frame_tick;

#if SKIP_RENDING
    goto FLUSH;
#endif
    
    current_ticks = clock();

//    cls(&win);
    sgl_fill(&win, WHITE);

    for (int x = 32; x < win.w; x += 32)
      sgl_vline(&win, x, 0, win.h, GRAY);
    for (int y = 32; y < win.h; y += 32)
      sgl_hline(&win, y, 0, win.w, GRAY);

#if defined(SGL_ENABLE_STB_IMAGE)
    sgl_blit(&win, &points[8], &s[1], NULL);
#endif

    sgl_writeln(&win, 10, 10, RED, -1, "Hello World");
    sgl_writeln(&win, 10, 22, MAROON, -1, "こんにちは");
#if defined(SGL_ENABLE_BDF)
    sgl_bdf_writeln(&win, tewi, 10, 34, WHITE, BLACK, "ΔhelloΔ bdf!");
#endif
    sgl_writeln(&win, 10, 48, RED, BLACK, "\f(255,0,0)\b(0,0,0)test\f(0,255,0)\b(0,0,0)test");

    int last_x = 0, last_y = 200;
    for (long i = sine_i; i < (sine_i + win.w); ++i) {
      float x = (float)(i - sine_i);
      float y = 200.f + (75.f * sinf(i * (3.141f / 180.f)));
      sgl_line(&win, last_x, last_y, x, y, col);
      last_x = x;
      last_y = y;
    }
    sine_i += (int)(speed * .2);

    sgl_blit(&win, &points[4], &s[3], NULL);
    sgl_blit(&win, &points[0], &s[2], &cutr);

    sgl_blit(&win, &points[1], &s[2], NULL);
    sgl_blit(&win, &points[3], &s[4], NULL);

    sgl_rotate(&s[5], theta, &s[7]);
    point_t tmp = {
      points[5].x - s[7].w / 2 + s[5].w / 2,
      points[5].y - s[7].h / 2 + s[5].h / 2,
    };
    sgl_blit(&win, &tmp, &s[7], NULL);
    sgl_blit(&win, &points[5], &s[5], NULL);
    theta += (.05f * speed);
    if (theta >= 360.f)
      theta = 0.f;
    sgl_destroy(&s[7]);

    sgl_filter(&s[0], rnd);
    sgl_blit(&s[0], NULL, &s[9], NULL);
    sgl_blit(&win, &points[2], &s[0], NULL);
    
#if defined(SGL_ENABLE_GIF)
    sgl_blit(&win, &points[10], &ok_hand.surfaces[ok_hand.frame], NULL);
    ok_hand.frame++;
    if (ok_hand.frame >= ok_hand.frames)
      ok_hand.frame = 0;
#endif

    sgl_circle(&win, 352, 32, 30, RED,    1);
    sgl_circle(&win, 382, 32, 30, ORANGE, 1);
    sgl_circle(&win, 412, 32, 30, YELLOW, 1);
    sgl_circle(&win, 442, 32, 30, LIME,   1);
    sgl_circle(&win, 472, 32, 30, BLUE,   1);
    sgl_circle(&win, 502, 32, 30, INDIGO, 1);
    sgl_circle(&win, 532, 32, 30, VIOLET, 1);

    sgl_blit(&win, NULL, &s[8], NULL);
    
    delta_ticks = clock() - current_ticks;
    if (delta_ticks > 0)
      fps = CLOCKS_PER_SEC / delta_ticks;
    sgl_writelnf(&win, 2, 2, RED, 0, "FPS: %ju", fps);
    
    time(&rt);
    struct tm tm = *localtime(&rt);
    sgl_ftfont_writelnf(&win, ftf, 0, 90, RGBA(255, 0, 0, test_alpha), RGBA(0, 255, 0, test_alpha), "It is %d/%d/%d at %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    test_alpha -= 1;
    if (test_alpha <= 0)
      test_alpha = 255;
    
    sgl_writelnf(&win, 400, 88, BLACK, 0, "mouse pos: (%d, %d)\ntheta: %f", mx, my, theta);
    col = sgl_pget(&win, mx, my);
    
    sgl_line(&win, 0, 0, mx, my, col);
    sgl_circle(&win, mx, my, 30, col, 0);
    
    sgl_blit(&win, &points[9], &s[10], NULL);

    if (grey)
      sgl_filter(&win, greyscale);
    
FLUSH:
    sgl_flush(&win);
  }

#if defined(SGL_ENABLE_JOYSTICKS)
  sgl_joystick_release();
#endif
#if defined(SGL_ENABLE_BDF)
  sgl_bdf_destroy(&tewi);
#endif
#if defined(SGL_ENABLE_FREETYPE)
  sgl_ftfont_destroy(&ftf);
  sgl_ft_release();
#endif
  sgl_destroy(&win);
  for (int i = 0; i < (int)(sizeof(s) / sizeof(s[0])); ++i)
    sgl_destroy(&s[i]);
  sgl_release();
  return 0;
}
