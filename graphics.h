//
//  graphics.h
//  graphics
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#ifndef graphics_h
#define graphics_h
#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define RGB(r, g, b) (((unsigned int)r) << 16) | (((unsigned int)g) << 8) | b
  
#define RED RGB(255, 0, 0)
#define BLUE RGB(0, 0, 255)
#define LIME RGB(0, 255, 0)
#define BLACK RGB(0, 0, 0)
#define WHITE RGB(255, 255, 255)
#define YELLOW RGB(255, 255, 0)
#define CYAN RGB(0, 255, 255)
#define MAGENTA RGB(255, 0, 255)
#define SILVER RGB(192, 192, 192)
#define GRAY RGB(128, 128, 128)
#define MAROON RGB(128, 0, 0)
#define OLIVE RGB(128, 128, 0)
#define GREEN RGB(0, 128, 0)
#define PURPLE RGB(128, 0, 128)
#define TEAL RGB(0, 128, 128)
#define NAVY RGB(0, 0, 128)

typedef struct {
  int* buf, w, h;
} surface_t;

typedef struct {
  int x, y, w, h;
} rect_t;

typedef struct {
  int x, y;
} point_t;

surface_t* surface(unsigned int, unsigned int);
void destroy(surface_t**);
void fill(surface_t* s, int col);
bool pset(surface_t* s, int x, int y, int col);
int pget(surface_t* s, int x, int y);
bool blit(surface_t* dst, point_t* p, surface_t* src, rect_t* rect);
bool yline(surface_t* s, int x, int y1, int y2, int col);
bool xline(surface_t* s, int y, int x1, int x2, int col);
bool line(surface_t* s, int x1, int y1, int x2, int y2, int col);
bool circle(surface_t* s, int xc, int yc, int r, int col, bool fill);
bool rect(surface_t* s, int x, int y, int w, int h, int col, bool fill);
unsigned char* load_file_to_mem(const char* path);
surface_t* bmp_mem(unsigned char* data);
surface_t* bmp_fp(FILE* fp);
surface_t* bmp(const char* path);
void letter(surface_t* s, unsigned char c, unsigned int x, unsigned int y, int col);
void print(surface_t* s, unsigned int x, unsigned int y, int col, const char* str);
void print_f(surface_t* s, unsigned int x, unsigned int y, int col, const char* fmt, ...);
surface_t* string(int col, int bg, const char* str);
surface_t* string_f(int col, int bg, const char* fmt, ...);
void rgb(int c, int* r, int* g, int* b);
surface_t* copy(surface_t* s);
void iterate(surface_t*, int (*fn)(int, int, int));
void set_chroma_key(unsigned int c);
long ticks(void);

typedef enum {
  MOUSE_0,
  MOUSE_1,
  MOUSE_2,
  MOUSE_3,
  MOUSE_4,
  MOUSE_5,
  MOUSE_6,
  MOUSE_7,
  MOUSE_8
} MOUSE_e;

#define MOUSE_LAST   MOUSE_8
#define MOUSE_LEFT   MOUSE_0
#define MOUSE_RIGHT  MOUSE_1
#define MOUSE_MIDDLE MOUSE_2
  
typedef enum {
  KEY_SPACE = 32,
  KEY_APOSTROPHE = 39,
  KEY_COMMA = 44,
  KEY_MINUS = 45,
  KEY_PERIOD = 46,
  KEY_SLASH = 47,
  KEY_0 = 48,
  KEY_1 = 49,
  KEY_2 = 50,
  KEY_3 = 51,
  KEY_4 = 52,
  KEY_5 = 53,
  KEY_6 = 54,
  KEY_7 = 55,
  KEY_8 = 56,
  KEY_9 = 57,
  KEY_SEMICOLON = 59,
  KEY_EQUAL = 61,
  KEY_A = 65,
  KEY_B = 66,
  KEY_C = 67,
  KEY_D = 68,
  KEY_E = 69,
  KEY_F = 70,
  KEY_G = 71,
  KEY_H = 72,
  KEY_I = 73,
  KEY_J = 74,
  KEY_K = 75,
  KEY_L = 76,
  KEY_M = 77,
  KEY_N = 78,
  KEY_O = 79,
  KEY_P = 80,
  KEY_Q = 81,
  KEY_R = 82,
  KEY_S = 83,
  KEY_T = 84,
  KEY_U = 85,
  KEY_V = 86,
  KEY_W = 87,
  KEY_X = 88,
  KEY_Y = 89,
  KEY_Z = 90,
  KEY_LEFT_BRACKET = 91,
  KEY_BACKSLASH = 92,
  KEY_RIGHT_BRACKET = 93,
  KEY_GRAVE_ACCENT = 96,
  KEY_WORLD_1 = 161,
  KEY_WORLD_2 = 162,
  KEY_ESCAPE = 256,
  KEY_ENTER = 257,
  KEY_TAB = 258,
  KEY_BACKSPACE = 259,
  KEY_INSERT = 260,
  KEY_DELETE = 261,
  KEY_RIGHT = 262,
  KEY_LEFT = 263,
  KEY_DOWN = 264,
  KEY_UP = 265,
  KEY_PAGE_UP = 266,
  KEY_PAGE_DOWN = 267,
  KEY_HOME = 268,
  KEY_END = 269,
  KEY_CAPS_LOCK = 280,
  KEY_SCROLL_LOCK = 281,
  KEY_NUM_LOCK = 282,
  KEY_PRINT_SCREEN = 283,
  KEY_PAUSE = 284,
  KEY_F1 = 290,
  KEY_F2 = 291,
  KEY_F3 = 292,
  KEY_F4 = 293,
  KEY_F5 = 294,
  KEY_F6 = 295,
  KEY_F7 = 296,
  KEY_F8 = 297,
  KEY_F9 = 298,
  KEY_F10 = 299,
  KEY_F11 = 300,
  KEY_F12 = 301,
  KEY_F13 = 302,
  KEY_F14 = 303,
  KEY_F15 = 304,
  KEY_F16 = 305,
  KEY_F17 = 306,
  KEY_F18 = 307,
  KEY_F19 = 308,
  KEY_F20 = 309,
  KEY_F21 = 310,
  KEY_F22 = 311,
  KEY_F23 = 312,
  KEY_F24 = 313,
  KEY_F25 = 314,
  KEY_KP_0 = 320,
  KEY_KP_1 = 321,
  KEY_KP_2 = 322,
  KEY_KP_3 = 323,
  KEY_KP_4 = 324,
  KEY_KP_5 = 325,
  KEY_KP_6 = 326,
  KEY_KP_7 = 327,
  KEY_KP_8 = 328,
  KEY_KP_9 = 329,
  KEY_KP_DECIMAL = 330,
  KEY_KP_DIVIDE = 331,
  KEY_KP_MULTIPLY = 332,
  KEY_KP_SUBTRACT = 333,
  KEY_KP_ADD = 334,
  KEY_KP_ENTER = 335,
  KEY_KP_EQUAL = 336,
  KEY_LEFT_SHIFT = 340,
  KEY_LEFT_CONTROL = 341,
  KEY_LEFT_ALT = 342,
  KEY_LEFT_SUPER = 343,
  KEY_RIGHT_SHIFT = 344,
  KEY_RIGHT_CONTROL = 345,
  KEY_RIGHT_ALT = 346,
  KEY_RIGHT_SUPER = 347,
  KEY_MENU = 348
} KEY_e;

#define KEY_UNKNOWN -1
#define KEY_LAST KEY_MENU

typedef enum {
  MOD_SHIFT = 0x0001,
  MOD_CONTROL = 0x0002,
  MOD_ALT = 0x0004,
  MOD_SUPER = 0x0008,
  MOD_CAPS_LOCK = 0x0010,
  MOD_NUM_LOCK = 0x0020
} MOD_e;

surface_t* screen(const char* title, int w, int h);
bool redraw(void);
void release(void);
void init_default_font(void);
void init_sys_keymap(void);
const char* get_last_error(void);
  
void mouse_move_cb(void (*fn)(int, int));
void mouse_entered_cb(void (*fn)(bool));
void mouse_down_cb(void (*fn)(MOUSE_e, MOD_e));
void mouse_up_cb(void (*fn)(MOUSE_e, MOD_e));
void mouse_pos(int* x, int* y);
void key_down_cb(void (*fn)(KEY_e, MOD_e));
void key_up_cb(void (*fn)(KEY_e, MOD_e));

#ifdef __cplusplus
}
#endif
#endif /* graphics_h */
