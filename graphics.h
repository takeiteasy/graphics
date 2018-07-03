//
//  graphics.h
//  graphics
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#ifndef graphics_h
#define graphics_h
#if defined(__cplusplus)
extern "C" {
#endif
  
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#if defined(_WIN32)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif // defined(_WIN32)
#if defined(__linux__) || defined(__unix__)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#endif
  
#define RGB(r, g, b) (((unsigned int)(r)) << 16) | (((unsigned int)(g)) << 8) | (b)

#define BLACK 0
#define BLUE 255
#define CYAN 65535
#define GRAY 8421504
#define GREEN 32768
#define LIME 65280
#define MAGENTA 16711935
#define MAROON 8388608
#define NAVY 128
#define OLIVE 8421376
#define PURPLE 8388736
#define RED 16711680
#define SILVER 12632256
#define TEAL 32896
#define WHITE 16777215
#define YELLOW 16776960
  
#define ALICE_BLUE 15792383
#define ANTIQUE_WHITE 16444375
#define AQUA 65535
#define AQUA_MARINE 8388564
#define AZURE 15794175
#define BEIGE 16119260
#define BISQUE 16770244
#define BLANCHED_ALMOND 16772045
#define BLUE_VIOLET 9055202
#define BROWN 10824234
#define BURLY_WOOD 14596231
#define CADET_BLUE 6266528
#define CHART_REUSE 8388352
#define CHOCOLATE 13789470
#define CORAL 16744272
#define CORN_FLOWER_BLUE 6591981
#define CORN_SILK 16775388
#define CRIMSON 14423100
#define DARK_BLUE 139
#define DARK_CYAN 35723
#define DARK_GOLDEN_ROD 12092939
#define DARK_GRAY 11119017
#define DARK_GREEN 25600
#define DARK_KHAKI 12433259
#define DARK_MAGENTA 9109643
#define DARK_OLIVE_GREEN 5597999
#define DARK_ORANGE 16747520
#define DARK_ORCHID 10040012
#define DARK_RED 9109504
#define DARK_SALMON 15308410
#define DARK_SEA_GREEN 9419919
#define DARK_SLATE_BLUE 4734347
#define DARK_SLATE_GRAY 3100495
#define DARK_TURQUOISE 52945
#define DARK_VIOLET 9699539
#define DEEP_PINK 16716947
#define DEEP_SKY_BLUE 49151
#define DIM_GRAY 6908265
#define DODGER_BLUE 2003199
#define FIREBRICK 11674146
#define FLORAL_WHITE 16775920
#define FOREST_GREEN 2263842
#define GAINSBORO 14474460
#define GHOST_WHITE 16316671
#define GOLD 16766720
#define GOLDEN_ROD 14329120
#define GREEN_YELLOW 11403055
#define HONEYDEW 15794160
#define HOT_PINK 16738740
#define INDIAN_RED 13458524
#define INDIGO 4915330
#define IVORY 16777200
#define KHAKI 15787660
#define LAVENDER 15132410
#define LAVENDER_BLUSH 16773365
#define LAWN_GREEN 8190976
#define LEMON_CHIFFON 16775885
#define LIGHT_BLUE 11393254
#define LIGHT_CORAL 15761536
#define LIGHT_CYAN 14745599
#define LIGHT_GOLDEN 16448210
#define LIGHT_GRAY 13882323
#define LIGHT_GREEN 9498256
#define LIGHT_PINK 16758465
#define LIGHT_SALMON 16752762
#define LIGHT_SEA_GREEN 2142890
#define LIGHT_SKY_BLUE 8900346
#define LIGHT_SLATE_GRAY 7833753
#define LIGHT_STEEL_BLUE 11584734
#define LIGHT_YELLOW 16777184
#define LIME_GREEN 3329330
#define LINEN 16445670
#define MEDIUM_AQUA_MARINE 6737322
#define MEDIUM_BLUE 205
#define MEDIUM_ORCHID 12211667
#define MEDIUM_PURPLE 9662683
#define MEDIUM_SEA_GREEN 3978097
#define MEDIUM_SLATE_BLUE 8087790
#define MEDIUM_SPRING_GREEN 64154
#define MEDIUM_TURQUOISE 4772300
#define MEDIUM_VIOLET_RED 13047173
#define MIDNIGHT_BLUE 1644912
#define MINT_CREAM 16121850
#define MISTY_ROSE 16770273
#define MOCCASIN 16770229
#define NAVAJO_WHITE 16768685
#define OLD_LACE 16643558
#define OLIVE_DRAB 7048739
#define ORANGE 16753920
#define ORANGE_RED 16729344
#define ORCHID 14315734
#define PALE_GOLDEN_ROD 15657130
#define PALE_GREEN 10025880
#define PALE_TURQUOISE 11529966
#define PALE_VIOLET_RED 14381203
#define PAPAYA_WHIP 16773077
#define PEACH_PUFF 16767673
#define PERU 13468991
#define PINK 16761035
#define PLUM 14524637
#define POWDER_BLUE 11591910
#define ROSY_BROWN 12357519
#define ROYAL_BLUE 4286945
#define SADDLE_BROWN 9127187
#define SALMON 16416882
#define SANDY_BROWN 16032864
#define SEA_GREEN 3050327
#define SEA_SHELL 16774638
#define SIENNA 10506797
#define SKY_BLUE 8900331
#define SLATE_BLUE 6970061
#define SLATE_GRAY 7372944
#define SNOW 16775930
#define SPRING_GREEN 65407
#define STEEL_BLUE 4620980
#define TAN 13808780
#define THISTLE 14204888
#define TOMATO 16737095
#define TURQUOISE 4251856
#define VIOLET 15631086
#define WHEAT 16113331
#define WHITE_SMOKE 16119285
#define YELLOW_GREEN 10145074
  
  typedef struct {
    int* buf, w, h;
  } surface_t;
  
  typedef struct {
    int x, y, w, h;
  } rect_t;
  
  typedef struct {
    int x, y;
  } point_t;
  
  int surface(surface_t* s, unsigned int w, unsigned int h);
  void destroy(surface_t*);
  void fill(surface_t* s, int col);
  void cls(surface_t* s);
  int pset(surface_t* s, int x, int y, int col);
  int pget(surface_t* s, int x, int y);
  int blit(surface_t* dst, point_t* p, surface_t* src, rect_t* rect, float opacity, int chroma);
#define BLIT(dst, p, src, rect) (blit((dst), (p), (src), (rect), -1, -1))
  void yline(surface_t* s, int x, int y0, int y1, int col);
  void xline(surface_t* s, int y, int x0, int x1, int col);
  void line(surface_t* s, int x0, int y0, int x1, int y1, int col);
  int bmp(surface_t* s, const char* path);
  void ascii(surface_t* s, char ch, int x, int y, int fg, int bg);
  int character(surface_t* s, const char* ch, int x, int y, int fg, int bg);
  void writeln(surface_t* s, int x, int y, int fg, int bg, const char* str);
  void writelnf(surface_t* s, int x, int y, int fg, int bg, const char* fmt, ...);
  void string(surface_t* out, int fg, int bg, const char* str);
  void stringf(surface_t* out, int fg, int bg, const char* fmt, ...);
  void rgb(int c, int* r, int* g, int* b);
  int alpha(int c1, int c2, float i);
  long ticks(void);
  void delay(long ms);
  int reset(surface_t* s, int nw, int nh);
  void resize_callback(void(*cb)(int, int));
  void circle(surface_t* s, int xc, int yc, int r, int col, int fill);
  void ellipse(surface_t* s, int xc, int yc, int rx, int ry, int col, int fill);
  void ellipse_rect(surface_t* s, int x0, int y0, int x1, int y1, int col, int fill);
  void bezier(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int col);
  void bezier_rational(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, float w, int col);
  void ellipse_rect_rotated(surface_t* s, int x0, int y0, int x1, int y1, long zd, int col);
  void ellipse_rotated(surface_t* s, int x, int y, int a, int b, float angle, int col);
  void bezier_cubic(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, int col);
  void rect(surface_t* s, int x, int y, int w, int h, int col, int fill);
  int save_bmp(surface_t* s, const char* path);
  int copy(surface_t* in, surface_t* out);
  void iterate(surface_t* s, int(*fn)(int x, int y, int col));
  int resize(surface_t* in, int nw, int nh, surface_t* out);
  
  typedef enum {
    MOUSE_BTN_0, // No mouse button
    MOUSE_BTN_1,
    MOUSE_BTN_2,
    MOUSE_BTN_3,
    MOUSE_BTN_4,
    MOUSE_BTN_5,
    MOUSE_BTN_6,
    MOUSE_BTN_7,
    MOUSE_BTN_8
  } mousebtn_t;
  
#define MOUSE_LAST   MOUSE_BTN_8
#define MOUSE_LEFT   MOUSE_BTN_0
#define MOUSE_RIGHT  MOUSE_BTN_1
#define MOUSE_MIDDLE MOUSE_BTN_2
  
  typedef enum {
    KB_KEY_SPACE = 32,
    KB_KEY_APOSTROPHE = 39,
    KB_KEY_COMMA = 44,
    KB_KEY_MINUS = 45,
    KB_KEY_PERIOD = 46,
    KB_KEY_SLASH = 47,
    KB_KEY_0 = 48,
    KB_KEY_1 = 49,
    KB_KEY_2 = 50,
    KB_KEY_3 = 51,
    KB_KEY_4 = 52,
    KB_KEY_5 = 53,
    KB_KEY_6 = 54,
    KB_KEY_7 = 55,
    KB_KEY_8 = 56,
    KB_KEY_9 = 57,
    KB_KEY_SEMICOLON = 59,
    KB_KEY_EQUAL = 61,
    KB_KEY_A = 65,
    KB_KEY_B = 66,
    KB_KEY_C = 67,
    KB_KEY_D = 68,
    KB_KEY_E = 69,
    KB_KEY_F = 70,
    KB_KEY_G = 71,
    KB_KEY_H = 72,
    KB_KEY_I = 73,
    KB_KEY_J = 74,
    KB_KEY_K = 75,
    KB_KEY_L = 76,
    KB_KEY_M = 77,
    KB_KEY_N = 78,
    KB_KEY_O = 79,
    KB_KEY_P = 80,
    KB_KEY_Q = 81,
    KB_KEY_R = 82,
    KB_KEY_S = 83,
    KB_KEY_T = 84,
    KB_KEY_U = 85,
    KB_KEY_V = 86,
    KB_KEY_W = 87,
    KB_KEY_X = 88,
    KB_KEY_Y = 89,
    KB_KEY_Z = 90,
    KB_KEY_LEFT_BRACKET = 91,
    KB_KEY_BACKSLASH = 92,
    KB_KEY_RIGHT_BRACKET = 93,
    KB_KEY_GRAVE_ACCENT = 96,
    KB_KEY_WORLD_1 = 161,
    KB_KEY_WORLD_2 = 162,
    KB_KEY_ESCAPE = 256,
    KB_KEY_ENTER = 257,
    KB_KEY_TAB = 258,
    KB_KEY_BACKSPACE = 259,
    KB_KEY_INSERT = 260,
    KB_KEY_DELETE = 261,
    KB_KEY_RIGHT = 262,
    KB_KEY_LEFT = 263,
    KB_KEY_DOWN = 264,
    KB_KEY_UP = 265,
    KB_KEY_PAGE_UP = 266,
    KB_KEY_PAGE_DOWN = 267,
    KB_KEY_HOME = 268,
    KB_KEY_END = 269,
    KB_KEY_CAPS_LOCK = 280,
    KB_KEY_SCROLL_LOCK = 281,
    KB_KEY_NUM_LOCK = 282,
    KB_KEY_PRINT_SCREEN = 283,
    KB_KEY_PAUSE = 284,
    KB_KEY_F1 = 290,
    KB_KEY_F2 = 291,
    KB_KEY_F3 = 292,
    KB_KEY_F4 = 293,
    KB_KEY_F5 = 294,
    KB_KEY_F6 = 295,
    KB_KEY_F7 = 296,
    KB_KEY_F8 = 297,
    KB_KEY_F9 = 298,
    KB_KEY_F10 = 299,
    KB_KEY_F11 = 300,
    KB_KEY_F12 = 301,
    KB_KEY_F13 = 302,
    KB_KEY_F14 = 303,
    KB_KEY_F15 = 304,
    KB_KEY_F16 = 305,
    KB_KEY_F17 = 306,
    KB_KEY_F18 = 307,
    KB_KEY_F19 = 308,
    KB_KEY_F20 = 309,
    KB_KEY_F21 = 310,
    KB_KEY_F22 = 311,
    KB_KEY_F23 = 312,
    KB_KEY_F24 = 313,
    KB_KEY_F25 = 314,
    KB_KEY_KP_0 = 320,
    KB_KEY_KP_1 = 321,
    KB_KEY_KP_2 = 322,
    KB_KEY_KP_3 = 323,
    KB_KEY_KP_4 = 324,
    KB_KEY_KP_5 = 325,
    KB_KEY_KP_6 = 326,
    KB_KEY_KP_7 = 327,
    KB_KEY_KP_8 = 328,
    KB_KEY_KP_9 = 329,
    KB_KEY_KP_DECIMAL = 330,
    KB_KEY_KP_DIVIDE = 331,
    KB_KEY_KP_MULTIPLY = 332,
    KB_KEY_KP_SUBTRACT = 333,
    KB_KEY_KP_ADD = 334,
    KB_KEY_KP_ENTER = 335,
    KB_KEY_KP_EQUAL = 336,
    KB_KEY_LEFT_SHIFT = 340,
    KB_KEY_LEFT_CONTROL = 341,
    KB_KEY_LEFT_ALT = 342,
    KB_KEY_LEFT_SUPER = 343,
    KB_KEY_RIGHT_SHIFT = 344,
    KB_KEY_RIGHT_CONTROL = 345,
    KB_KEY_RIGHT_ALT = 346,
    KB_KEY_RIGHT_SUPER = 347,
    KB_KEY_MENU = 348
  } keysym_t;
  
#define KB_KEY_UNKNOWN -1
#define KB_KEY_LAST KB_KEY_MENU
  
  typedef enum {
    KB_MOD_SHIFT = 0x0001,
    KB_MOD_CONTROL = 0x0002,
    KB_MOD_ALT = 0x0004,
    KB_MOD_SUPER = 0x0008,
    KB_MOD_CAPS_LOCK = 0x0010,
    KB_MOD_NUM_LOCK = 0x0020
  } keymod_t;
  
  typedef enum {
    MOUSE_BTN_DOWN,
    MOUSE_BTN_UP,
    KEYBOARD_KEY_DOWN,
    KEYBOARD_KEY_UP,
    SCROLL_WHEEL,
    WINDOW_CLOSED
  } user_event_type_t;
  
  typedef struct {
    user_event_type_t type;
    keysym_t sym;
    keymod_t mod;
    mousebtn_t btn;
    int data1, data2;
  } user_event_t;
  
  int screen(const char* title, int w, int h);
  int should_close(void);
  int poll_events(user_event_t* e);
  void render(surface_t* s);
  void release(void);
  const char* get_last_error(void);
  void get_mouse_pos(int* x, int* y);
  
#if defined(__cplusplus)
}
#endif
#endif /* graphics_h */
