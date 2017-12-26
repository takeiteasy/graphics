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
#include <unistd.h>
#include <math.h>
#include <time.h>

#define RGB(r, g, b) (((unsigned int)r) << 16) | (((unsigned int)g) << 8) | b
  
#define MAROON RGB(128, 0, 0)
#define DARKRED RGB(139, 0, 0)
#define BROWN RGB(165, 42, 42)
#define FIREBRICK RGB(178, 34, 34)
#define CRIMSON RGB(220, 20, 20)
#define RED RGB(255, 0, 0)
#define TOMATO RGB(255, 99, 99)
#define CORAL RGB(255, 127, 127)
#define INDIANRED RGB(205, 92, 92)
#define LIGHTCORAL RGB(240, 128, 128)
#define DARKSALMON RGB(233, 150, 150)
#define SALMON RGB(250, 128, 128)
#define LIGHTSALMON RGB(255, 160, 160)
#define ORANGERED RGB(255, 69, 69)
#define DARKORANGE RGB(255, 140, 140)
#define ORANGE RGB(255, 165, 165)
#define GOLD RGB(255, 215, 215)
#define DARKGOLDENROD RGB(184, 134, 134)
#define GOLDENROD RGB(218, 165, 165)
#define PALEGOLDENROD RGB(238, 232, 232)
#define DARKKHAKI RGB(189, 183, 183)
#define KHAKI RGB(240, 230, 230)
#define OLIVE RGB(128, 128, 128)
#define YELLOW RGB(255, 255, 255)
#define YELLOWGREEN RGB(154, 205, 205)
#define DARKOLIVEGREEN RGB(85, 107, 107)
#define OLIVEDRAB RGB(107, 142, 142)
#define LAWNGREEN RGB(124, 252, 252)
#define CHARTREUSE RGB(127, 255, 255)
#define GREENYELLOW RGB(173, 255, 255)
#define DARKGREEN RGB(0, 100, 100)
#define GREEN RGB(0, 128, 128)
#define FORESTGREEN RGB(34, 139, 139)
#define LIME RGB(0, 255, 255)
#define LIMEGREEN RGB(50, 205, 205)
#define LIGHTGREEN RGB(144, 238, 238)
#define PALEGREEN RGB(152, 251, 251)
#define DARKSEAGREEN RGB(143, 188, 188)
#define MEDIUMSPRINGGREEN RGB(0, 250, 250)
#define SPRINGGREEN RGB(0, 255, 255)
#define SEAGREEN RGB(46, 139, 139)
#define MEDIUMAQUAMARINE RGB(102, 205, 205)
#define MEDIUMSEAGREEN RGB(60, 179, 179)
#define LIGHTSEAGREEN RGB(32, 178, 178)
#define DARKSLATEGRAY RGB(47, 79, 79)
#define TEAL RGB(0, 128, 128)
#define DARKCYAN RGB(0, 139, 139)
#define AQUA RGB(0, 255, 255)
#define CYAN RGB(0, 255, 255)
#define LIGHTCYAN RGB(224, 255, 255)
#define DARKTURQUOISE RGB(0, 206, 206)
#define TURQUOISE RGB(64, 224, 224)
#define MEDIUMTURQUOISE RGB(72, 209, 209)
#define PALETURQUOISE RGB(175, 238, 238)
#define AQUAMARINE RGB(127, 255, 255)
#define POWDERBLUE RGB(176, 224, 224)
#define CADETBLUE RGB(95, 158, 158)
#define STEELBLUE RGB(70, 130, 130)
#define CORNFLOWERBLUE RGB(100, 149, 149)
#define DEEPSKYBLUE RGB(0, 191, 191)
#define DODGERBLUE RGB(30, 144, 144)
#define LIGHTBLUE RGB(173, 216, 216)
#define SKYBLUE RGB(135, 206, 206)
#define LIGHTSKYBLUE RGB(135, 206, 206)
#define MIDNIGHTBLUE RGB(25, 25, 25)
#define NAVY RGB(0, 0, 0)
#define DARKBLUE RGB(0, 0, 0)
#define MEDIUMBLUE RGB(0, 0, 0)
#define BLUE RGB(0, 0, 0)
#define ROYALBLUE RGB(65, 105, 105)
#define BLUEVIOLET RGB(138, 43, 43)
#define INDIGO RGB(75, 0, 0)
#define DARKSLATEBLUE RGB(72, 61, 61)
#define SLATEBLUE RGB(106, 90, 90)
#define MEDIUMSLATEBLUE RGB(123, 104, 104)
#define MEDIUMPURPLE RGB(147, 112, 112)
#define DARKMAGENTA RGB(139, 0, 0)
#define DARKVIOLET RGB(148, 0, 0)
#define DARKORCHID RGB(153, 50, 50)
#define MEDIUMORCHID RGB(186, 85, 85)
#define PURPLE RGB(128, 0, 0)
#define THISTLE RGB(216, 191, 191)
#define PLUM RGB(221, 160, 160)
#define VIOLET RGB(238, 130, 130)
#define MAGENTA RGB(255, 0, 0)
#define ORCHID RGB(218, 112, 112)
#define MEDIUMVIOLETRED RGB(199, 21, 21)
#define PALEVIOLETRED RGB(219, 112, 112)
#define DEEPPINK RGB(255, 20, 20)
#define HOTPINK RGB(255, 105, 105)
#define LIGHTPINK RGB(255, 182, 182)
#define PINK RGB(255, 192, 192)
#define ANTIQUEWHITE RGB(250, 235, 235)
#define BEIGE RGB(245, 245, 245)
#define BISQUE RGB(255, 228, 228)
#define BLANCHEDALMOND RGB(255, 235, 235)
#define WHEAT RGB(245, 222, 222)
#define CORNSILK RGB(255, 248, 248)
#define LEMONCHIFFON RGB(255, 250, 250)
#define LIGHTGOLDENRODYELLOW RGB(250, 250, 250)
#define LIGHTYELLOW RGB(255, 255, 255)
#define SADDLEBROWN RGB(139, 69, 69)
#define SIENNA RGB(160, 82, 82)
#define CHOCOLATE RGB(210, 105, 105)
#define PERU RGB(205, 133, 133)
#define SANDYBROWN RGB(244, 164, 164)
#define BURLYWOOD RGB(222, 184, 184)
#define TAN RGB(210, 180, 180)
#define ROSYBROWN RGB(188, 143, 143)
#define MOCCASIN RGB(255, 228, 228)
#define NAVAJOWHITE RGB(255, 222, 222)
#define PEACHPUFF RGB(255, 218, 218)
#define MISTYROSE RGB(255, 228, 228)
#define LAVENDERBLUSH RGB(255, 240, 240)
#define LINEN RGB(250, 240, 240)
#define OLDLACE RGB(253, 245, 245)
#define PAPAYAWHIP RGB(255, 239, 239)
#define SEASHELL RGB(255, 245, 245)
#define MINTCREAM RGB(245, 255, 255)
#define SLATEGRAY RGB(112, 128, 128)
#define LIGHTSLATEGRAY RGB(119, 136, 136)
#define LIGHTSTEELBLUE RGB(176, 196, 196)
#define LAVENDER RGB(230, 230, 230)
#define FLORALWHITE RGB(255, 250, 250)
#define ALICEBLUE RGB(240, 248, 248)
#define GHOSTWHITE RGB(248, 248, 248)
#define HONEYDEW RGB(240, 255, 255)
#define IVORY RGB(255, 255, 255)
#define AZURE RGB(240, 255, 255)
#define SNOW RGB(255, 250, 250)
#define BLACK RGB(0, 0, 0)
#define DIMGRAY RGB(105, 105, 105)
#define GRAY RGB(128, 128, 128)
#define DARKGRAY RGB(169, 169, 169)
#define SILVER RGB(192, 192, 192)
#define LIGHTGRAY RGB(211, 211, 211)
#define GAINSBORO RGB(220, 220, 220)
#define WHITESMOKE RGB(245, 245, 245)
#define WHITE RGB(255, 255, 255)

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
bool blit(surface_t* dst, point_t* p, surface_t* src, rect_t* rect, int chroma);
bool yline(surface_t* s, int x, int y1, int y2, int col);
bool xline(surface_t* s, int y, int x1, int x2, int col);
bool line(surface_t* s, int x1, int y1, int x2, int y2, int col);
bool circle(surface_t* s, int xc, int yc, int r, int col, bool fill);
bool rect(surface_t* s, int x, int y, int w, int h, int col, bool fill);
unsigned char* load_file_to_mem(const char* path);
surface_t* bmp_mem(unsigned char* data);
surface_t* bmp_fp(FILE* fp);
surface_t* bmp(const char* path);
void letter(surface_t* s, unsigned char ch, unsigned int x, unsigned int y, int col);
void print(surface_t* s, unsigned int x, unsigned int y, int col, const char* str);
void print_f(surface_t* s, unsigned int x, unsigned int y, int col, const char* fmt, ...);
surface_t* string(int col, int bg, const char* str);
surface_t* string_f(int col, int bg, const char* fmt, ...);
void rgb(int c, int* r, int* g, int* b);
surface_t* copy(surface_t* s);
void iterate(surface_t*, int (*fn)(int, int, int));
long ticks(void);
void delay(long ms);

typedef enum {
  MOUSE_BTN_0,
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

surface_t* screen(const char* title, int w, int h);
bool should_close(void);
bool poll_events(user_event_t* e);
void render(void);
void release(void);
const char* get_last_error(void);
void get_mouse_pos(int* x, int* y);

#ifdef __cplusplus
}
#endif
#endif /* graphics_h */
