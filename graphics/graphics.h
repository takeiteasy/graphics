/* graphics.h
 *
 * Created by Rory B. Bellows on 26/11/2017.
 * Copyright © 2017-2019 George Watson. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * *   Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * *   Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * *   Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL GEORGE WATSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef graphics_h
#define graphics_h
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__EMSCRIPTEN__) || defined(EMSCRIPTEN)
#define GRAPHICS_EMCC
#include <emscripten/emscripten.h>
#endif
  
#if defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
#define GRAPHICS_LINUX
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
#define GRAPHICS_OSX
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
#define GRAPHICS_WINDOWS
#else
#define GRAPHICS_NO_WINDOW
#endif

#if defined(GRAPHICS_MALLOC) && defined(GRAPHICS_FREE) && (defined(GRAPHICS_REALLOC) || defined(GRAPHICS_REALLOC_SIZED))
#elif !defined(GRAPHICS_MALLOC) && !defined(GRAPHICS_FREE) && !defined(GRAPHICS_REALLOC) && !defined(GRAPHICS_REALLOC_SIZED)
#else
#error "Must define all or none of GRAPHICS_MALLOC, GRAPHICS_FREE, and GRAPHICS_REALLOC (or GRAPHICS_REALLOC_SIZED)."
#endif
  
#if defined(DEBUG) && !defined(GRAPHICS_DEBUG)
#define GRAPHICS_DEBUG
#endif

#if !defined(GRAPHICS_MALLOC)
#define GRAPHICS_MALLOC(sz)       malloc(sz)
#define GRAPHICS_REALLOC(p,newsz) realloc(p,newsz)
#define GRAPHICS_FREE(p)          free(p)
#endif
#define GRAPHICS_SAFE_FREE(x) \
if ((x)) { \
  free((void*)(x)); \
  (x) = NULL; \
}

#if defined(_MSC_VER)
#define bool int
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#include <stdarg.h>

// Taken from: https://stackoverflow.com/a/1911632
#if _MSC_VER
#define STRINGISE_IMPL(x) #x
#define STRINGISE(x) STRINGISE_IMPL(x)
#define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else
#define WARN(exp) ("WARNING: " exp)
#endif
  
  /*!
   * @discussion Convert RGBA to packed integer
   * @param r R channel
   * @param g G channel
   * @param b B channel
   * @param a A channel
   * @return Packed RGBA colour
   */
  int rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
  /*!
   * @discussion Convert RGB to packed integer
   * @param r R channel
   * @param g G channel
   * @param b B channel
   * @return Packed RGB colour
   */
  int rgb(unsigned char r, unsigned char g, unsigned char b);
  /*!
   * @discussion Convert channel to packed RGBA integer
   * @param c R channel
   * @return Packed RGBA colour
   */
  int rgba1(unsigned char c);
  /*!
   * @discussion Convert channel to packed RGB integer
   * @param c R channel
   * @return Packed RGBA colour
   */
  int rgb1(unsigned char c);
  /*!
   * @discussion Retrieve R channel from packed RGBA integer
   * @param c Packed RGBA integer
   * @return R channel value
   */
  unsigned char r_channel(int c);
  /*!
   * @discussion Retrieve G channel from packed RGBA integer
   * @param c Packed RGBA integer
   * @return G channel value
   */
  unsigned char g_channel(int c);
  /*!
   * @discussion Retrieve B channel from packed RGBA integer
   * @param c Packed RGBA integer
   * @return B channel value
   */
  unsigned char b_channel(int c);
  /*!
   * @discussion Retrieve A channel from packed RGBA integer
   * @param c Packed RGBA integer
   * @return A channel value
   */
  unsigned char a_channel(int c);
  /*!
   * @discussion Modify R channel of packed RGBA integer
   * @param c Packed RGBA integer
   * @param r New R channel
   * @return Packed RGBA integer
   */
  int rgba_r(int c, unsigned char r);
  /*!
   * @discussion Modify G channel of packed RGBA integer
   * @param c Packed RGBA integer
   * @param g New G channel
   * @return Packed RGBA integer
   */
  int rgba_g(int c, unsigned char g);
  /*!
   * @discussion Modify B channel of packed RGBA integer
   * @param c Packed RGBA integer
   * @param b New B channel
   * @return Packed RGBA integer
   */
  int rgba_b(int c, unsigned char b);
  /*!
   * @discussion Modify A channel of packed RGBA integer
   * @param c Packed RGBA integer
   * @param a New A channel
   * @return Packed RGBA integer
   */
  int rgba_a(int c, unsigned char a);

  /*!
   * @typedef colours
   * @brief A list of colours with names
   */
  enum colour {
    BLACK = -16777216,
    BLUE = -16776961,
    CYAN = -16711681,
    GRAY = -8355712,
    GREEN = -16744448,
    LIME = -16711936,
    MAGENTA = -65281,
    MAROON = -8388608,
    NAVY = -16777088,
    PURPLE = -8388480,
    RED = -65536,
    TEAL = -16744320,
    WHITE = -1,
    YELLOW = -256,
    
    ALICE_BLUE = -984833,
    ANTIQUE_WHITE = -332841,
    AQUA = -16711681,
    AQUA_MARINE = -8388652,
    AZURE = -983041,
    BEIGE = -657956,
    BISQUE = -6972,
    BLANCHED_ALMOND = -5171,
    BLUE_VIOLET = -7722014,
    BROWN = -5952982,
    BURLY_WOOD = -2180985,
    CADET_BLUE = -10510688,
    CHART_REUSE = -8388864,
    CHOCOLATE = -2987746,
    CORAL = -32944,
    CORN_FLOWER_BLUE = -10185235,
    CORN_SILK = -1828,
    CRIMSON = -2354116,
    DARK_BLUE = -16777077,
    DARK_CYAN = -16741493,
    DARK_GOLDEN_ROD = -4684277,
    DARK_GRAY = -5658199,
    DARK_GREEN = -16751616,
    DARK_KHAKI = -4343957,
    DARK_MAGENTA = -7667573,
    DARK_OLIVE_GREEN = -11179217,
    DARK_ORANGE = -29696,
    DARK_ORCHID = -6737204,
    DARK_RED = -7667712,
    DARK_SALMON = -1468806,
    DARK_SEA_GREEN = -7357297,
    DARK_SLATE_BLUE = -12042869,
    DARK_SLATE_GRAY = -13676721,
    DARK_TURQUOISE = -16724271,
    DARK_VIOLET = -7077677,
    DEEP_PINK = -60269,
    DEEP_SKY_BLUE = -16728065,
    DIM_GRAY = -9868951,
    DODGER_BLUE = -14774017,
    FIREBRICK = -5103070,
    FLORAL_WHITE = -1296,
    FOREST_GREEN = -14513374,
    GAINSBORO = -2302756,
    GHOST_WHITE = -460545,
    GOLD = -10496,
    GOLDEN_ROD = -2448096,
    GREEN_YELLOW = -5374161,
    HONEYDEW = -983056,
    HOT_PINK = -38476,
    INDIAN_RED = -3318692,
    INDIGO = -11861886,
    IVORY = -16,
    KHAKI = -989556,
    LAVENDER = -1644806,
    LAVENDER_BLUSH = -3851,
    LAWN_GREEN = -8586240,
    LEMON_CHIFFON = -1331,
    LIGHT_BLUE = -5383962,
    LIGHT_CORAL = -1015680,
    LIGHT_CYAN = -2031617,
    LIGHT_GOLDEN_ROD = -329006,
    LIGHT_GRAY = -2894893,
    LIGHT_GREEN = -7278960,
    LIGHT_PINK = -18751,
    LIGHT_SALMON = -24454,
    LIGHT_SEA_GREEN = -14634326,
    LIGHT_SKY_BLUE = -7876870,
    LIGHT_SLATE_GRAY = -8943463,
    LIGHT_STEEL_BLUE = -5192482,
    LIGHT_YELLOW = -32,
    LIME_GREEN = -13447886,
    LINEN = -331546,
    MEDIUM_AQUA_MARINE = -10039894,
    MEDIUM_BLUE = -16777011,
    MEDIUM_ORCHID = -4565549,
    MEDIUM_PURPLE = -7114533,
    MEDIUM_SEA_GREEN = -12799119,
    MEDIUM_SLATE_BLUE = -8689426,
    MEDIUM_SPRING_GREEN = -16713062,
    MEDIUM_TURQUOISE = -12004916,
    MEDIUM_VIOLET_RED = -3730043,
    MIDNIGHT_BLUE = -15132304,
    MINT_CREAM = -655366,
    MISTY_ROSE = -6943,
    MOCCASIN = -6987,
    NAVAJO_WHITE = -8531,
    OLD_LACE = -133658,
    OLIVE_DRAB = -9728477,
    ORANGE = -23296,
    ORANGE_RED = -47872,
    ORCHID = -2461482,
    PALE_GOLDEN_ROD = -1120086,
    PALE_GREEN = -6751336,
    PALE_TURQUOISE = -5247250,
    PALE_VIOLET_RED = -2396013,
    PAPAYA_WHIP = -4139,
    PEACH_PUFF = -9543,
    PERU = -3308225,
    PINK = -16181,
    PLUM = -2252579,
    POWDER_BLUE = -5185306,
    ROSY_BROWN = -4419697,
    ROYAL_BLUE = -12490271,
    SADDLE_BROWN = -7650029,
    SALMON = -360334,
    SANDY_BROWN = -744352,
    SEA_GREEN = -13726889,
    SEA_SHELL = -2578,
    SIENNA = -6270419,
    SKY_BLUE = -7876885,
    SLATE_BLUE = -9807155,
    SLATE_GRAY = -9404272,
    SNOW = -1286,
    SPRING_GREEN = -16711809,
    STEEL_BLUE = -12156236,
    TAN = -2968436,
    THISTLE = -2572328,
    TOMATO = -40121,
    TURQUOISE = -12525360,
    VIOLET = -1146130,
    WHEAT = -663885,
    WHITE_SMOKE = -657931,
    YELLOW_GREEN = -6632142
  };

  /*!
   * @typedef surface_t
   * @brief An object to hold image data
   * @constant buf Buffer holding pixel data
   * @constant w Width of image
   * @constant h Height of image
   */
  struct surface_t {
    int *buf, w, h;
  };
  
  /*!
   * @discussion Create a new surface
   * @param s Pointer to surface object to create
   * @param w Width of new surface
   * @param h Height of new surface
   * @return Boolean for success
   */
  bool surface(struct surface_t* s, unsigned int w, unsigned int h);
  /*!
   * @discussion Destroy a surface
   * @param s Pointer to pointer to surface object
   */
  void surface_destroy(struct surface_t* s);
  
  /*!
   * @typedef draw_mode
   * @brief Draw modes, normal = no alpha, mask = <255 transparent, alpha = transparency
   */
  enum draw_mode {
    NORMAL,
    MASK,
    ALPHA
  };
  
  /*!
   * @discussion Set draw mode
   * @param m Which mode to use
   */
  void graphics_draw_mode(enum draw_mode m);
  
  /*!
   * @discussion Fill a surface with a given colour
   * @param s Surface object
   * @param col Colour to set
   */
  void fill(struct surface_t* s, int col);
  /*!
   * @discussion Flood portion of surface with given colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param col Colour to set
   */
  void flood(struct surface_t* s, int x, int y, int col);
  /*!
   * @discussion Clear a surface, zero the buffer
   * @param s Surface object
   */
  void cls(struct surface_t* s);
  /*!
   * @discussion Set surface pixel colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param col Colour to set
   */
  void pset(struct surface_t* s, int x, int y, int col);
  /*!
   * @discussion Get surface pixel colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @return Pixel colour
   */
  int pget(struct surface_t* s, int x, int y);
  /*!
   * @discussion Blit one surface onto another at point
   * @param dst Surface to blit to
   * @param src Surface to blit
   * @param x X position
   * @param y Y position
   * @return Boolean of success
   */
  bool paste(struct surface_t* dst, struct surface_t* src, int x, int y);
  /*!
   * @discussion Blit one surface onto another at point with clipping rect
   * @param dst Surface to blit to
   * @param src Surface to blit
   * @param x X position
   * @param y Y position
   * @param rx Clip rect X
   * @param ry Clip rect Y
   * @param rw Clip rect width
   * @param rh Clip rect height
   * @return Boolean of success
   */
  bool clip_paste(struct surface_t* dst, struct surface_t* src, int x, int y, int rx, int ry, int rw, int rh);
  /*!
   * @discussion Reallocate a surface
   * @param s Surface object
   * @param nw New width
   * @param nh New height
   * @return Boolean of success
   */
  bool reset(struct surface_t* s, int nw, int nh);
  /*!
   * @discussion Create a copy of a surface
   * @param a Original surface object
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  bool copy(struct surface_t* a, struct surface_t* b);
  /*!
   * @discussion Loop through each pixel of surface and run position and colour through a callback. Return value of the callback is the new colour at the position
   * @param s Surface object
   * @param fn Callback function
   */
  void passthru(struct surface_t* s, int(*fn)(int x, int y, int col));
  /*!
   * @discussion Resize (and scale) surface to given size
   * @param a Original surface object
   * @param nw New width
   * @param nh New height
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  bool resize(struct surface_t* a, int nw, int nh, struct surface_t* b);
  /*!
   * @discussion Rotate a surface by a given degree
   * @param a Original surface object
   * @param angle Angle to rotate by
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  bool rotate(struct surface_t* a, float angle, struct surface_t* b);

  /*!
   * @discussion Simple Bresenham line
   * @param s Surface object
   * @param x0 Vector A X position
   * @param y0 Vector A Y position
   * @param x1 Vector B X position
   * @param y1 Vector B Y position
   * @param col Colour of line
   */
  void line(struct surface_t* s, int x0, int y0, int x1, int y1, int col);
  /*!
   * @discussion Draw a circle
   * @param s Surface object
   * @param xc Centre X position
   * @param yc Centre Y position
   * @param r Circle radius
   * @param col Colour of cricle
   * @param fill Fill circle boolean
   */
  void circle(struct surface_t* s, int xc, int yc, int r, int col, bool fill);
  /*!
   * @discussion Draw a rectangle
   * @param x X position
   * @param y Y position
   * @param w Rectangle width
   * @param h Rectangle height
   * @param col Colour of rectangle
   * @param fill Fill rectangle boolean
   */
  void rect(struct surface_t* s, int x, int y, int w, int h, int col, bool fill);
  /*!
   * @discussion Draw a triangle
   * @param s Surface object
   * @param x0 Vector A X position
   * @param y0 Vector A Y position
   * @param x1 Vector B X position
   * @param y1 Vector B Y position
   * @param x2 Vector C X position
   * @param y2 Vector C Y position
   * @param col Colour of line
   * @param fill Fill triangle boolean
   */
  void tri(struct surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int col, bool fill);

  /*!
   * @discussion Load BMP file from path
   * @param s Surface object to allocate
   * @param path Path to BMP file
   * @return Boolean of success
   */
  bool bmp(struct surface_t* s, const char* path);

  /*!
   * @discussion Save surface to BMP file
   * @params s Surface object to save
   * @param path Path to save BMP file to
   * @return Boolean of success
   */
  bool save_bmp(struct surface_t* s, const char* path);

  /*!
   * @discussion Draw a character from ASCII value using default in-built font
   * @param s Surface object
   * @param ch ASCII character code
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   */
  void ascii(struct surface_t* s, unsigned char ch, int x, int y, int fg, int bg);
  /*!
   * @discussion Draw first character (ASCII or Unicode) from string using default in-built font
   * @param s Surface object
   * @param ch Source string
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @return Returns length of character
   */
  int character(struct surface_t* s, const char* ch, int x, int y, int fg, int bg);
  /*!
   * @discussion Draw a string using default in-built font
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  void writeln(struct surface_t* s, int x, int y, int fg, int bg, const char* str);
  /*!
   * @discussion Draw a string using default in-built font
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  void writelnf(struct surface_t* s, int x, int y, int fg, int bg, const char* fmt, ...);
  /*!
   * @discussion Create a surface object for text using default in-built font
   * @param s Surface object to be allocated
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  void string(struct surface_t* s, int fg, int bg, const char* str);
  /*!
   * @discussion Create a surface object for formatted text using default in-built font
   * @param s Surface object to be allocated
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  void stringf(struct surface_t* s, int fg, int bg, const char* fmt, ...);
  
  /*!
   * @discussion High precision timer
   * @return Number of CPU ticks
   */
  unsigned long long ticks(void);

  /*!
   * @typedef button
   * @brief A list of mouse buttons
   */
  enum button {
    MOUSE_BTN_0 = 0,
    MOUSE_BTN_1,
    MOUSE_BTN_2,
    MOUSE_BTN_3,
    MOUSE_BTN_4,
    MOUSE_BTN_5,
    MOUSE_BTN_6,
    MOUSE_BTN_7,
    MOUSE_BTN_8
  };

  static const int MOUSE_LAST   = MOUSE_BTN_8;
  static const int MOUSE_LEFT   = MOUSE_BTN_0;
  static const int MOUSE_RIGHT  = MOUSE_BTN_1;
  static const int MOUSE_MIDDLE = MOUSE_BTN_2;

  /*!
   * @typedef key_sym
   * @brief A list of key symbols
   */
  enum key_sym {
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
    KB_KEY_EQUALS = 61,
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
    KB_KEY_KP_EQUALS = 336,
    KB_KEY_LEFT_SHIFT = 340,
    KB_KEY_LEFT_CONTROL = 341,
    KB_KEY_LEFT_ALT = 342,
    KB_KEY_LEFT_SUPER = 343,
    KB_KEY_RIGHT_SHIFT = 344,
    KB_KEY_RIGHT_CONTROL = 345,
    KB_KEY_RIGHT_ALT = 346,
    KB_KEY_RIGHT_SUPER = 347,
    KB_KEY_MENU = 348
  };
  
  static const int KB_KEY_UNKNOWN = -1;
  static const int KB_KEY_LAST = KB_KEY_MENU;
  
  /*!
   * @typedef key_mod
   * @brief A list of key modifiers
   */
  enum key_mod {
    KB_MOD_SHIFT = 0x0001,
    KB_MOD_CONTROL = 0x0002,
    KB_MOD_ALT = 0x0004,
    KB_MOD_SUPER = 0x0008,
    KB_MOD_CAPS_LOCK = 0x0010,
    KB_MOD_NUM_LOCK = 0x0020
  };

#define XMAP_SCREEN_CB \
  X(keyboard, (void*, enum key_sym, enum key_mod, bool)) \
  X(mouse_button, (void*, enum button, enum key_mod, bool)) \
  X(mouse_move, (void*, int, int, int, int)) \
  X(scroll, (void*, enum key_mod, float, float)) \
  X(focus, (void*, bool)) \
  X(resize, (void*, int, int)) \
  X(closed, (void*))
  
  /*!
   * @typedef window_t
   * @brief An object to hold window data
   * @constant w Width of window
   * @constant h Height of window
   * @constant window Pointer to internet platform specific window data
   * @parent parent Pointer to userdata for callbacks
   */
  struct window_t {
    int id, w, h;

#define X(a, b) void(*a##_callback)b;
    XMAP_SCREEN_CB
#undef X
	
    void *window, *parent;
  };

  /*!
   * @discussion Set "parent" for a window object. The parent pointer will be passed to window callbacks.
   * @param s Window object
   * @param p Pointer to parent
   */
  void window_set_parent(struct window_t* s, void* p);
  /*!
   * @discussion Get parent point from window object
   * @param s Window object
   * @return Point to parent
   */
  void* window_parent(struct window_t* s);

#define X(a, b) \
  void(*a##_cb)b,

  void window_callbacks(XMAP_SCREEN_CB struct window_t* window);
#undef X
#define X(a, b) \
  void a##_callback(struct window_t* window, void(*a##_cb)b);
  XMAP_SCREEN_CB
#undef X

    /*!
     * @typedef cursor_type
     * @brief A list of default cursor icons
     */
  enum cursor_type {
      CURSOR_ARROW,     // Arrow
      CURSOR_IBEAM,     // I-beam
      CURSOR_WAIT,      // Wait
      CURSOR_CROSSHAIR, // Crosshair
      CURSOR_WAITARROW, // Small wait cursor (or Wait if not available)
      CURSOR_SIZENWSE,  // Double arrow pointing northwest and southeast
      CURSOR_SIZENESW,  // Double arrow pointing northeast and southwest
      CURSOR_SIZEWE,    // Double arrow pointing west and east
      CURSOR_SIZENS,    // Double arrow pointing north and south
      CURSOR_SIZEALL,   // Four pointed arrow pointing north, south, east, and west
      CURSOR_NO,        // Slashed circle or crossbones
      CURSOR_HAND       // Hand
    };

  /*!
   * @typedef WINDOW_FLAGS
   * @brief A list of window flag options
   */
  enum window_flags {
    NONE = 0,
    RESIZABLE = 0x01,
    FULLSCREEN = 0x02,
    FULLSCREEN_DESKTOP = 0x04,
    BORDERLESS = 0x08,
    ALWAYS_ON_TOP = 0x10,
  };

  /*!
   * @discussion Create a new window object
   * @param s Window object to be allocated
   * @param t Window title
   * @param w Window width
   * @param h Window height
   * @param flags Window flags
   * @return Boolean of success
   */
  bool window(struct window_t* s, const char* t, int w, int h, short flags);
  /*!
   * @discussion Set window icon from surface object
   * @param s Window object
   * @param b Surface object
   */
  void window_icon(struct window_t* s, struct surface_t* b);
  /*!
   * @discussion Set window title
   * @param s Window object
   * @param t New title
   */
  void window_title(struct window_t* s, const char* t);
  /*!
   * @discussion Get the position of a window object
   * @param s Window object
   * @param x Pointer to int to set
   * @param y Pointer to int to set
   */
  void window_position(struct window_t* s, int* x, int*  y);
  /*!
   * @discussion Get the size of the screen a window is on
   * @param s Window object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  void screen_size(struct window_t* s, int* w, int* h);
  /*!
   * @discussion Destroy window object
   * @param s Window object
   */
  void window_destroy(struct window_t* s);
  /*!
   * @discussion Unique window ID for window object
   * @param s Window object
   * @return Unique ID of window object
   */
  int window_id(struct window_t* s);
  /*!
   * @discussion Get size of window
   * @param s Window object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  void window_size(struct window_t* s, int* w, int* h);
  /*!
   * @discussion Check if a window is still open
   * @param s Window object
   * @return Boolean if window is open
   */
  bool closed(struct window_t* s);
  /*!
   * @discussion Check if n windows are still open
   * @param n Numbers of arguments
   * @param ... Window objects
   * @return Boolean if any window are still open
   */
  bool closed_va(int n, ...);
  /*!
   * @discussion Checks if any windows are still open
   * @return Boolean if any windows are still open
   */
  bool closed_all(void);

  /*!
   * @discussion Lock or unlock cursor movement to active window
   * @param locked Turn on or off
   */
  void cursor_lock(struct window_t* s, bool locked);
  /*!
   * @discussion Hide or show system cursor
   * @param show Hide or show
   */
  void cursor_visible(struct window_t* s, bool show);
  /*!
   * @discussion Change cursor icon to system icon
   * @param s Window object
   * @param t Type of cursor
   */
  void cursor_icon(struct window_t* s, enum cursor_type t);
  /*!
   * @discussion Change cursor icon to icon from surface object
   * @param s Window object
   * @param b Surface object
   */
  void cursor_icon_custom(struct window_t* s, struct surface_t* b);
  /*!
   * @discussion Get cursor position
   * @param x Integer to set
   * @param y Integer to set
   */
  void cursor_pos(int* x, int* y);
  /*!
   * @discussion Set cursor position
   * @param x X position
   * @param y Y position
   */
  void cursor_set_pos(int x, int y);

  /*!
   * @discussion Poll for window events
   */
  void events(void);
  /*!
   * @discussion Draw surface object to window
   * @param s Window object
   * @param b Surface object
   */
  void flush(struct window_t* s, struct surface_t* b);
  /*!
   * @discussion Release anything allocated by this library
   */
  void release(void);

#define GRAPHICS_ERROR(A, ...) graphics_error((A), __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
  
  /*!
   * @typedef graphics_error
   * @brief A list of different error types the library can generate
   */
  enum graphics_error {
    UNKNOWN_ERROR,
    OUT_OF_MEMEORY,
    FILE_OPEN_FAILED,
    INVALID_BMP,
    UNSUPPORTED_BMP,
    INVALID_PARAMETERS,
    CURSOR_MOD_FAILED,
    OSX_WINDOW_CREATION_FAILED,
    OSX_APPDEL_CREATION_FAILED,
    OSX_FULLSCREEN_FAILED,
    WIN_WINDOW_CREATION_FAILED,
    WIN_FULLSCREEN_FAILED,
    NIX_CURSOR_PIXMAP_ERROR,
    NIX_OPEN_DISPLAY_FAILED,
    NIX_WINDOW_CREATION_FAILED,
    WINDOW_ICON_FAILED,
    CUSTOM_CURSOR_NOT_CREATED
  };
  
  /*!
   * @discussion Callback for errors inside library
   * @param cb Function pointer to callback
   */
  void graphics_error_callback(void(*cb)(enum graphics_error, const char*, const char*, const char*, int));
  /*!
   * @discussion Internal function to send an error to the error callback
   * @param type The GRAPHICS_ERROR produced
   * @param file The file the error occured in
   * @param func The Function error occured in
   * @param line The line number the error occured on
   * @param msg Formatted error description
   */
  void graphics_error(enum graphics_error type, const char* file, const char* func, int line, const char* msg, ...);
  
#if defined(__cplusplus)
}
#endif
#endif // graphics_h

