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
#define GRAPHICS_EXTERNAL_WINDOW
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
  
#if !defined(HALDEF)
#if defined(GRAPHICS_STATIC)
#define HALDEF static
#else
#define HALDEF extern
#endif
#endif

#if defined(__cplusplus)
#define GRAPHICS_EXTERN extern "C"
#else
#define GRAPHICS_EXTERN extern
#endif

#if !defined(_MSC_VER)
#if defined(__cplusplus)
#define graphics_inline inline
#else
#define graphics_inline
#endif
#else
#define graphics_inline __forceinline
#endif

#if !defined(GRAPHICS_VERSION_MAJOR)
#define GRAPHICS_VERSION_MAJOR 0
#endif
#if !defined(GRAPHICS_VERSION_MINOR)
#define GRAPHICS_VERSION_MINOR 0
#endif
#if !defined(GRAPHICS_VERSION_REV)
#define GRAPHICS_VERSION_REV 0
#endif
#define GRAPHICS_VERSION_INT (GRAPHICS_VERSION_MAJOR << 16 | GRAPHICS_VERSION_MINOR << 8 | GRAPHICS_VERSION_REV)
#define GRAPHICS_VERSION_DOT_STR(a, b, c) a ##.## b ##.## c
#define GRAPHICS_VERSION_STR GRAPHICS_VERSION_DOT_STR(a, b, c)
#if !defined(GRAPHICS_VERSION_GIT)
#define GRAPHICS_VERSION_GIT "unknown"
#endif

// Taken from: https://stackoverflow.com/a/1911632
#if _MSC_VER
#define STRINGISE_IMPL(x) #x
#define STRINGISE(x) STRINGISE_IMPL(x)
#define FILE_LINE_LINK __FILE__ "(" STRINGISE(__LINE__) ") : "
#define WARN(exp) (FILE_LINE_LINK "WARNING: " exp)
#else
#define WARN(exp) ("WARNING: " exp)
#endif

#define COL_RGBA(r, g, b, a) ((((unsigned int)(a)) << 24) | (((unsigned int)(r)) << 16) | (((unsigned int)(g)) << 8) | (b))
#define COL_RGB(r, g, b) (COL_RGBA((r), (g), (b), 255))
#define COL_R(v) (((v) >> 16) & 0xFF)
#define COL_G(v) (((v) >>  8) & 0xFF)
#define COL_B(v) ( (v)        & 0xFF)
#define COL_A(v) (((v) >> 24) & 0xFF)
#define COL_RCHAN(a, b) (((a) & ~0x00FF0000) | ((b) << 16))
#define COL_GCHAN(a, b) (((a) & ~0x0000FF00) | ((b) << 8))
#define COL_BCHAN(a, b) (((a) & ~0x000000FF) |  (b))
#define COL_ACHAN(a, b) (((a) & ~0xFF000000) | ((b) << 24))
#define COL_RGB1(c) (COL_RGB((c), (c), (c)))
#define COL_RGBA1(c, a) (COL_RGBA((c), (c), (c), (a)))

#if !defined(GRAPHICS_NO_COLORS)
  /*!
   * @typedef COLOURS
   * @brief A list of colours with names
   */
  typedef enum {
    BLACK = COL_RGB(0, 0, 0),
    BLUE = COL_RGB(0, 0, 255),
    CYAN = COL_RGB(0, 255, 255),
    GRAY = COL_RGB(128, 128, 128),
    GREEN = COL_RGB(0, 128, 0),
    LIME = COL_RGB(0, 255, 0),
    MAGENTA = COL_RGB(255, 0, 255),
    MAROON = COL_RGB(128, 0, 0),
    NAVY = COL_RGB(0, 0, 128),
    PURPLE = COL_RGB(128, 0, 128),
    RED = COL_RGB(255, 0, 0),
    TEAL = COL_RGB(0, 128, 128),
    WHITE = COL_RGB(255, 255, 255),
    YELLOW = COL_RGB(255, 255, 0),

    ALICE_BLUE = COL_RGB(240, 248, 255),
    ANTIQUE_WHITE = COL_RGB(250, 235, 215),
    AQUA = COL_RGB(0, 255, 255),
    AQUA_MARINE = COL_RGB(127, 255, 212),
    AZURE = COL_RGB(240, 255, 255),
    BEIGE = COL_RGB(245, 245, 220),
    BISQUE = COL_RGB(255, 228, 196),
    BLANCHED_ALMOND = COL_RGB(255, 235, 205),
    BLUE_VIOLET = COL_RGB(138, 43, 226),
    BROWN = COL_RGB(165, 42, 42),
    BURLY_WOOD = COL_RGB(222, 184, 135),
    CADET_BLUE = COL_RGB(95, 158, 160),
    CHART_REUSE = COL_RGB(127, 255, 0),
    CHOCOLATE = COL_RGB(210, 105, 30),
    CORAL = COL_RGB(255, 127, 80),
    CORN_FLOWER_BLUE = COL_RGB(100, 149, 237),
    CORN_SILK = COL_RGB(255, 248, 220),
    CRIMSON = COL_RGB(220, 20, 60),
    DARK_BLUE = COL_RGB(0, 0, 139),
    DARK_CYAN = COL_RGB(0, 139, 139),
    DARK_GOLDEN_ROD = COL_RGB(184, 134, 11),
    DARK_GRAY = COL_RGB(169, 169, 169),
    DARK_GREEN = COL_RGB(0, 100, 0),
    DARK_KHAKI = COL_RGB(189, 183, 107),
    DARK_MAGENTA = COL_RGB(139, 0, 139),
    DARK_OLIVE_GREEN = COL_RGB(85, 107, 47),
    DARK_ORANGE = COL_RGB(255, 140, 0),
    DARK_ORCHID = COL_RGB(153, 50, 204),
    DARK_RED = COL_RGB(139, 0, 0),
    DARK_SALMON = COL_RGB(233, 150, 122),
    DARK_SEA_GREEN = COL_RGB(143, 188, 143),
    DARK_SLATE_BLUE = COL_RGB(72, 61, 139),
    DARK_SLATE_GRAY = COL_RGB(47, 79, 79),
    DARK_TURQUOISE = COL_RGB(0, 206, 209),
    DARK_VIOLET = COL_RGB(148, 0, 211),
    DEEP_PINK = COL_RGB(255, 20, 147),
    DEEP_SKY_BLUE = COL_RGB(0, 191, 255),
    DIM_GRAY = COL_RGB(105, 105, 105),
    DODGER_BLUE = COL_RGB(30, 144, 255),
    FIREBRICK = COL_RGB(178, 34, 34),
    FLORAL_WHITE = COL_RGB(255, 250, 240),
    FOREST_GREEN = COL_RGB(34, 139, 34),
    GAINSBORO = COL_RGB(220, 220, 220),
    GHOST_WHITE = COL_RGB(248, 248, 255),
    GOLD = COL_RGB(255, 215, 0),
    GOLDEN_ROD = COL_RGB(218, 165, 32),
    GREEN_YELLOW = COL_RGB(173, 255, 47),
    HONEYDEW = COL_RGB(240, 255, 240),
    HOT_PINK = COL_RGB(255, 105, 180),
    INDIAN_RED = COL_RGB(205, 92, 92),
    INDIGO = COL_RGB(75, 0, 130),
    IVORY = COL_RGB(255, 255, 240),
    KHAKI = COL_RGB(240, 230, 140),
    LAVENDER = COL_RGB(230, 230, 250),
    LAVENDER_BLUSH = COL_RGB(255, 240, 245),
    LAWN_GREEN = COL_RGB(124, 252, 0),
    LEMON_CHIFFON = COL_RGB(255, 250, 205),
    LIGHT_BLUE = COL_RGB(173, 216, 230),
    LIGHT_CORAL = COL_RGB(240, 128, 128),
    LIGHT_CYAN = COL_RGB(224, 255, 255),
    LIGHT_GOLDEN_ROD = COL_RGB(250, 250, 210),
    LIGHT_GRAY = COL_RGB(211, 211, 211),
    LIGHT_GREEN = COL_RGB(144, 238, 144),
    LIGHT_PINK = COL_RGB(255, 182, 193),
    LIGHT_SALMON = COL_RGB(255, 160, 122),
    LIGHT_SEA_GREEN = COL_RGB(32, 178, 170),
    LIGHT_SKY_BLUE = COL_RGB(135, 206, 250),
    LIGHT_SLATE_GRAY = COL_RGB(119, 136, 153),
    LIGHT_STEEL_BLUE = COL_RGB(176, 196, 222),
    LIGHT_YELLOW = COL_RGB(255, 255, 224),
    LIME_GREEN = COL_RGB(50, 205, 50),
    LINEN = COL_RGB(250, 240, 230),
    MEDIUM_AQUA_MARINE = COL_RGB(102, 205, 170),
    MEDIUM_BLUE = COL_RGB(0, 0, 205),
    MEDIUM_ORCHID = COL_RGB(186, 85, 211),
    MEDIUM_PURPLE = COL_RGB(147, 112, 219),
    MEDIUM_SEA_GREEN = COL_RGB(60, 179, 113),
    MEDIUM_SLATE_BLUE = COL_RGB(123, 104, 238),
    MEDIUM_SPRING_GREEN = COL_RGB(0, 250, 154),
    MEDIUM_TURQUOISE = COL_RGB(72, 209, 204),
    MEDIUM_VIOLET_RED = COL_RGB(199, 21, 133),
    MIDNIGHT_BLUE = COL_RGB(25, 25, 112),
    MINT_CREAM = COL_RGB(245, 255, 250),
    MISTY_ROSE = COL_RGB(255, 228, 225),
    MOCCASIN = COL_RGB(255, 228, 181),
    NAVAJO_WHITE = COL_RGB(255, 222, 173),
    OLD_LACE = COL_RGB(253, 245, 230),
    OLIVE_DRAB = COL_RGB(107, 142, 35),
    ORANGE = COL_RGB(255, 165, 0),
    ORANGE_RED = COL_RGB(255, 69, 0),
    ORCHID = COL_RGB(218, 112, 214),
    PALE_GOLDEN_ROD = COL_RGB(238, 232, 170),
    PALE_GREEN = COL_RGB(152, 251, 152),
    PALE_TURQUOISE = COL_RGB(175, 238, 238),
    PALE_VIOLET_RED = COL_RGB(219, 112, 147),
    PAPAYA_WHIP = COL_RGB(255, 239, 213),
    PEACH_PUFF = COL_RGB(255, 218, 185),
    PERU = COL_RGB(205, 133, 63),
    PINK = COL_RGB(255, 192, 203),
    PLUM = COL_RGB(221, 160, 221),
    POWDER_BLUE = COL_RGB(176, 224, 230),
    ROSY_BROWN = COL_RGB(188, 143, 143),
    ROYAL_BLUE = COL_RGB(65, 105, 225),
    SADDLE_BROWN = COL_RGB(139, 69, 19),
    SALMON = COL_RGB(250, 128, 114),
    SANDY_BROWN = COL_RGB(244, 164, 96),
    SEA_GREEN = COL_RGB(46, 139, 87),
    SEA_SHELL = COL_RGB(255, 245, 238),
    SIENNA = COL_RGB(160, 82, 45),
    SKY_BLUE = COL_RGB(135, 206, 235),
    SLATE_BLUE = COL_RGB(106, 90, 205),
    SLATE_GRAY = COL_RGB(112, 128, 144),
    SNOW = COL_RGB(255, 250, 250),
    SPRING_GREEN = COL_RGB(0, 255, 127),
    STEEL_BLUE = COL_RGB(70, 130, 180),
    TAN = COL_RGB(210, 180, 140),
    THISTLE = COL_RGB(216, 191, 216),
    TOMATO = COL_RGB(255, 99, 71),
    TURQUOISE = COL_RGB(64, 224, 208),
    VIOLET = COL_RGB(238, 130, 238),
    WHEAT = COL_RGB(245, 222, 179),
    WHITE_SMOKE = COL_RGB(245, 245, 245),
    YELLOW_GREEN = COL_RGB(154, 205, 50)
  } COLOURS;
#endif

#if defined(GRAPHICS_CHROMA_KEY) && !defined(BLIT_CHROMA_KEY)
#if !defined(GRAPHICS_NO_DEFAULT_COLORS)
#define BLIT_CHROMA_KEY LIME
#else
#define BLIT_CHROMA_KEY -16711936
#endif
#endif
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
   * @discussion Get size of given surface
   * @param s Surface object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  HALDEF void surface_size(struct surface_t* s, unsigned int* w, unsigned int* h);
  /*!
   * @discussion Get pointer to buffer of given surface
   * @param s Surface object
   * @return The pointer to surface buffer
   */
  HALDEF int* surface_raw(struct surface_t* s);
  /*!
   * @discussion Create a new surface
   * @param s Pointer to surface object to create
   * @param w Width of new surface
   * @param h Height of new surface
   * @return Boolean for success
   */
  HALDEF bool surface(struct surface_t* s, unsigned int w, unsigned int h);
  /*!
   * @discussion Destroy a surface
   * @param s Pointer to pointer to surface object
   */
  HALDEF void surface_destroy(struct surface_t* s);

  /*!
   * @discussion Fill a surface with a given colour
   * @param s Surface object
   * @param col Colour to set
   */
  HALDEF void fill(struct surface_t* s, int col);
  /*!
   * @discussion Flood portion of surface with given colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param col Colour to set
   */
  HALDEF void flood(struct surface_t* s, int x, int y, int col);
  /*!
   * @discussion Clear a surface, zero the buffer
   * @param s Surface object
   */
  HALDEF void cls(struct surface_t* s);
  /*!
   * @discussion Set surface pixel colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param col Colour to set
   */
  HALDEF void pset(struct surface_t* s, int x, int y, int col);
  /*!
   * @discussion Get surface pixel colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @return Pixel colour
   */
  HALDEF int  pget(struct surface_t* s, int x, int y);
  /*!
   * @discussion Blit one surface onto another at point
   * @param dst Surface to blit to
   * @param src Surface to blit
   * @param x X position
   * @param y Y position
   * @return Boolean of success
   */
  HALDEF bool paste(struct surface_t* dst, struct surface_t* src, int x, int y);
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
  HALDEF bool clip_paste(struct surface_t* dst, struct surface_t* src, int x, int y, int rx, int ry, int rw, int rh);
  /*!
   * @discussion Reallocate a surface
   * @param s Surface object
   * @param nw New width
   * @param nh New height
   * @return Boolean of success
   */
  HALDEF bool reset(struct surface_t* s, int nw, int nh);
  /*!
   * @discussion Create a copy of a surface
   * @param a Original surface object
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  HALDEF bool copy(struct surface_t* a, struct surface_t* b);
  /*!
   * @discussion Loop through each pixel of surface and run position and colour through a callback. Return value of the callback is the new colour at the position
   * @param s Surface object
   * @param fn Callback function
   */
  HALDEF void passthru(struct surface_t* s, int(*fn)(int x, int y, int col));
  /*!
   * @discussion Resize (and scale) surface to given size
   * @param a Original surface object
   * @param nw New width
   * @param nh New height
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  HALDEF bool resize(struct surface_t* a, int nw, int nh, struct surface_t* b);
  /*!
   * @discussion Rotate a surface by a given degree
   * @param a Original surface object
   * @param angle Angle to rotate by
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  HALDEF bool rotate(struct surface_t* a, float angle, struct surface_t* b);
  /*!
   * @discussion https://en.wikipedia.org/wiki/Color_quantization
   * @param a Original surface object
   * @param n_colors Maximum colours
   * @param b New surface object to be allocated
   */
  HALDEF void quantize(struct surface_t* a, int n_colors, struct surface_t* b);

  /*!
   * @discussion Simple Bresenham line
   * @param s Surface object
   * @param x0 Vector A X position
   * @param y0 Vector A Y position
   * @param x1 Vector B X position
   * @param y1 Vector B Y position
   * @param col Colour of line
   */
  HALDEF void line(struct surface_t* s, int x0, int y0, int x1, int y1, int col);
  /*!
   * @discussion Draw a circle
   * @param s Surface object
   * @param xc Centre X position
   * @param yc Centre Y position
   * @param r Circle radius
   * @param col Colour of cricle
   * @param fill Fill circle boolean
   */
  HALDEF void circle(struct surface_t* s, int xc, int yc, int r, int col, bool fill);
  /*!
   * @discussion Draw a rectangle
   * @param x X position
   * @param y Y position
   * @param w Rectangle width
   * @param h Rectangle height
   * @param col Colour of rectangle
   * @param fill Fill rectangle boolean
   */
  HALDEF void rect(struct surface_t* s, int x, int y, int w, int h, int col, bool fill);
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
  HALDEF void tri(struct surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int col, bool fill);

  /*!
   * @discussion Load BMP file from path
   * @param s Surface object to allocate
   * @param path Path to BMP file
   * @return Boolean of success
   */
  HALDEF bool bmp(struct surface_t* s, const char* path);

  /*!
   * @discussion Save surface to BMP file
   * @params s Surface object to save
   * @param path Path to save BMP file to
   * @return Boolean of success
   */
  HALDEF bool save_bmp(struct surface_t* s, const char* path);
#if !defined(GRAPHICS_NO_TEXT)
  /*!
   * @discussion Draw a character from ASCII value using default in-built font
   * @param s Surface object
   * @param ch ASCII character code
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   */
  HALDEF void ascii(struct surface_t* s, unsigned char ch, int x, int y, int fg, int bg);
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
  HALDEF int character(struct surface_t* s, const char* ch, int x, int y, int fg, int bg);
  /*!
   * @discussion Draw a string using default in-built font
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  HALDEF void writeln(struct surface_t* s, int x, int y, int fg, int bg, const char* str);
  /*!
   * @discussion Draw a string using default in-built font
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  HALDEF void writelnf(struct surface_t* s, int x, int y, int fg, int bg, const char* fmt, ...);
  /*!
   * @discussion Create a surface object for text using default in-built font
   * @param s Surface object to be allocated
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  HALDEF void string(struct surface_t* s, int fg, int bg, const char* str);
  /*!
   * @discussion Create a surface object for formatted text using default in-built font
   * @param s Surface object to be allocated
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  HALDEF void stringf(struct surface_t* s, int fg, int bg, const char* fmt, ...);
#endif

#if !defined(GRAPHICS_NO_BDF)
  /*!
   * @typedef bdf_t
   * @brief BDF font object
   */
  struct bdf_char_t {
    unsigned int width;
    char* bitmap;
    int bb_x, bb_y, bb_w, bb_h;
  };
  
  struct bdf_t {
    int fbb_x, fbb_y, fbb_w, fbb_h;
    unsigned int* encoding_table;
    struct bdf_char_t* chars;
    int n_chars;
  };

  /*!
   * @discussion Destroy a BDF font object
   * @param f Pointer to BDF font object
   */
  HALDEF void bdf_destroy(struct bdf_t* f);
  /*!
   * @discussion Load a BDF font from path
   * @param out BDF object to be allocated
   * @param path Path of BDF file
   * @return Boolean of success
   */
  HALDEF bool bdf(struct bdf_t* out, const char* path);
  /*!
   * @discussion Draw a string using BDF font
   * @param s Surface object
   * @param f BDF font object
   * @param ch Source string
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @return Returns length of character
   */
  HALDEF int bdf_character(struct surface_t* s, struct bdf_t* f, const char* ch, int x, int y, int fg, int bg);
  /*!
   * @discussion Draw a string using BDF font object
   * @param s Surface object
   * @param f BDF font object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  HALDEF void bdf_writeln(struct surface_t* s, struct bdf_t* f, int x, int y, int fg, int bg, const char* str);
  /*!
   * @discussion Draw a formatted string using BDF font object
   * @param s Surface object
   * @param f BDF font object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  HALDEF void bdf_writelnf(struct surface_t* s, struct bdf_t* f, int x, int y, int fg, int bg, const char* fmt, ...);
  /*!
   * @discussion Create a surface object for text using BDF font object
   * @param s Surface object to be allocated
   * @param f BDF font object
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  HALDEF void bdf_string(struct surface_t* s, struct bdf_t* f, int fg, int bg, const char* str);
  /*!
   * @discussion Create a surface object for formatted text using BDF font object
   * @param s Surface object to be allocated
   * @param f BDF font object
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  HALDEF void bdf_stringf(struct surface_t* s, struct bdf_t* f, int fg, int bg, const char* fmt, ...);
#endif

#if !defined(GRAPHICS_NO_ALERTS)
  /*!
   * @typedef ALERT_LVL
   * @brief A list of alert levels for dialog boxes
   */
  typedef enum {
    ALERT_INFO,
    ALERT_WARNING,
    ALERT_ERROR
  } ALERT_LVL;

  /*!
   * @typedef ALERT_BTNS
   * @brief A list of button options for dialog boxes
   */
  typedef enum {
    ALERT_OK,
    ALERT_OK_CANCEL,
    ALERT_YES_NO
  } ALERT_BTNS;

  /*!
   * @typedef DIALOG_ACTION
   * @brief A list of options for dialog boxes
   * @constant DIALOG_OPEN Open file dialog
   * @constant DIALOG_OPEN_DIR Open directory dialog
   * @constant DIALOG_SAVE Save file dialog
   */
  typedef enum {
    DIALOG_OPEN,
    DIALOG_OPEN_DIR,
    DIALOG_SAVE
  } DIALOG_ACTION;

  /*!
   * @discussion Open an alert dialog with message
   * @param lvl Dialog level
   * @param btns Dialog buttons
   * @param fmt Formatted message
   * @return User value from dialog action
   */
  HALDEF bool alert(ALERT_LVL lvl, ALERT_BTNS btns, const char* fmt, ...);
  /*!
   * @discussion Open file dialog
   * @param action Save, open directory, open file
   * @param path Initial path for dialog
   * @param fname Default filename in dialog path
   * @param allow_multiple Allow selection of multiple files
   * @param nfilters Number of extention filters
   * @param ... Extension filters
   * @return Selected paths in dialog or NULL is cancelled
   */
  HALDEF int dialog(DIALOG_ACTION action, char*** result, const char* path, const char* fname, bool allow_multiple, int nfilters, ...);
#endif

  /*!
   * @typedef MOUSE_BTN
   * @brief A list of mouse buttons
   */
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
  } MOUSE_BTN;

#define MOUSE_LAST   MOUSE_BTN_8
#define MOUSE_LEFT   MOUSE_BTN_0
#define MOUSE_RIGHT  MOUSE_BTN_1
#define MOUSE_MIDDLE MOUSE_BTN_2

  /*!
   * @typedef KEY_SYM
   * @brief A list of key symbols
   */
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
  } KEY_SYM;

#define KB_KEY_UNKNOWN -1
#define KB_KEY_LAST KB_KEY_MENU

  /*!
   * @typedef KEY_MOD
   * @brief A list of key modifiers
   */
  typedef enum {
    KB_MOD_SHIFT = 0x0001,
    KB_MOD_CONTROL = 0x0002,
    KB_MOD_ALT = 0x0004,
    KB_MOD_SUPER = 0x0008,
    KB_MOD_CAPS_LOCK = 0x0010,
    KB_MOD_NUM_LOCK = 0x0020
  } KEY_MOD;

#define XMAP_SCREEN_CB \
  X(keyboard, (void*, KEY_SYM, KEY_MOD, bool)) \
  X(mouse_button, (void*, MOUSE_BTN, KEY_MOD, bool)) \
  X(mouse_move, (void*, int, int, int, int)) \
  X(scroll, (void*, KEY_MOD, float, float)) \
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
  HALDEF void window_set_parent(struct window_t* s, void* p);
  /*!
   * @discussion Get parent point from window object
   * @param s Window object
   * @return Point to parent
   */
  HALDEF void* window_parent(struct window_t* s);

#define X(a, b) \
  void(*a##_cb)b,

  HALDEF void window_callbacks(XMAP_SCREEN_CB struct window_t* window);
#undef X
#define X(a, b) \
  HALDEF void a##_callback(struct window_t* window, void(*a##_cb)b);
  XMAP_SCREEN_CB
#undef X

    /*!
     * @typedef CURSOR_TYPE
     * @brief A list of default cursor icons
     */
    typedef enum {
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
    } CURSOR_TYPE;

  /*!
   * @typedef WINDOW_FLAGS
   * @brief A list of window flag options
   */
  typedef enum {
    NONE = 0,
    RESIZABLE = 0x01,
    FULLSCREEN = 0x02,
    FULLSCREEN_DESKTOP = 0x04,
    BORDERLESS = 0x08,
    ALWAYS_ON_TOP = 0x10,
  } WINDOW_FLAGS;

  /*!
   * @discussion Create a new window object
   * @param s Window object to be allocated
   * @param t Window title
   * @param w Window width
   * @param h Window height
   * @param flags Window flags
   * @return Boolean of success
   */
  HALDEF bool window(struct window_t* s, const char* t, int w, int h, short flags);
  /*!
   * @discussion Set window icon from surface object
   * @param s Window object
   * @param b Surface object
   */
  HALDEF void window_icon(struct window_t* s, struct surface_t* b);
  /*!
   * @discussion Set window title
   * @param s Window object
   * @param t New title
   */
  HALDEF void window_title(struct window_t* s, const char* t);
  /*!
   * @discussion Get the position of a window object
   * @param s Window object
   * @param x Pointer to int to set
   * @param y Pointer to int to set
   */
  HALDEF void window_position(struct window_t* s, int* x, int*  y);
  /*!
   * @discussion Get the size of the screen a window is on
   * @param s Window object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  HALDEF void screen_size(struct window_t* s, int* w, int* h);
  /*!
   * @discussion Destroy window object
   * @param s Window object
   */
  HALDEF void window_destroy(struct window_t* s);
  /*!
   * @discussion Unique window ID for window object
   * @param s Window object
   * @return Unique ID of window object
   */
  HALDEF int window_id(struct window_t* s);
  /*!
   * @discussion Get size of window
   * @param s Window object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  HALDEF void window_size(struct window_t* s, int* w, int* h);
  /*!
   * @discussion Check if a window is still open
   * @param s Window object
   * @return Boolean if window is open
   */
  HALDEF bool closed(struct window_t* s);
  /*!
   * @discussion Check if n windows are still open
   * @param n Numbers of arguments
   * @param ... Window objects
   * @return Boolean if any window are still open
   */
  HALDEF bool closed_va(int n, ...);

  /*!
   * @discussion Lock or unlock cursor movement to active window
   * @param locked Turn on or off
   */
  HALDEF void cursor_lock(struct window_t* s, bool locked);
  /*!
   * @discussion Hide or show system cursor
   * @param show Hide or show
   */
  HALDEF void cursor_visible(struct window_t* s, bool show);
  /*!
   * @discussion Change cursor icon to system icon
   * @param s Window object
   * @param t Type of cursor
   */
  HALDEF void cursor_icon(struct window_t* s, CURSOR_TYPE t);
  /*!
   * @discussion Change cursor icon to icon from surface object
   * @param s Window object
   * @param b Surface object
   */
  HALDEF void cursor_icon_custom(struct window_t* s, struct surface_t* b);
  /*!
   * @discussion Get cursor position
   * @param x Integer to set
   * @param y Integer to set
   */
  HALDEF void cursor_pos(int* x, int* y);
  /*!
   * @discussion Set cursor position
   * @param x X position
   * @param y Y position
   */
  HALDEF void cursor_set_pos(int x, int y);

  /*!
   * @discussion Poll for window events
   */
  HALDEF void events(void);
  /*!
   * @discussion Draw surface object to window
   * @param s Window object
   * @param b Surface object
   */
  HALDEF void flush(struct window_t* s, struct surface_t* b);
  /*!
   * @discussion Release anything allocated by this library
   */
  HALDEF void release(void);

#define GRAPHICS_ERROR(A, ...) graphics_error((A), __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
  
  /*!
   * @typedef ERROR_TYPE
   * @brief A list of different error types the library can generate
   */
  typedef enum {
    UNKNOWN_ERROR,
    OUT_OF_MEMEORY,
    FILE_OPEN_FAILED,
    INVALID_BMP,
    UNSUPPORTED_BMP,
    INVALID_PARAMETERS,
    BDF_NO_CHAR_SIZE,
    BDF_NO_CHAR_LENGTH,
    BDF_TOO_MANY_BITMAPS,
    BDF_UNKNOWN_CHAR,
    GL_SHADER_ERROR,
    GL_LOAD_DL_FAILED,
    GL_GET_PROC_ADDR_FAILED,
    CURSOR_MOD_FAILED,
    MTK_LIBRARY_ERROR,
    MTK_CREATE_PIPELINE_FAILED,
    OSX_WINDOW_CREATION_FAILED,
    OSX_APPDEL_CREATION_FAILED,
    OSX_FULLSCREEN_FAILED,
    WIN_GL_PF_ERROR,
    WIN_WINDOW_CREATION_FAILED,
    WIN_FULLSCREEN_FAILED,
    WIN_DX9_CREATION_FAILED,
    WIN_DX11_CREATION_FAILED,
    NIX_CURSOR_PIXMAP_ERROR,
    NIX_OPEN_DISPLAY_FAILED,
    NIX_GL_FB_ERROR,
    NIX_GL_CONTEXT_ERROR,
    NIX_WINDOW_CREATION_FAILED,
    WINDOW_ICON_FAILED,
    CUSTOM_CURSOR_NOT_CREATED
  } GRAPHICS_ERROR_TYPE;
  
  /*!
   * @discussion Callback for errors inside library
   * @param cb Function pointer to callback
   */
  HALDEF void graphics_error_callback(void(*cb)(GRAPHICS_ERROR_TYPE, const char*, const char*, const char*, int));
  /*!
   * @discussion Internal function to send an error to the error callback
   * @param type The GRAPHICS_ERROR produced
   * @param file The file the error occured in
   * @param func The Function error occured in
   * @param line The line number the error occured on
   * @param msg Formatted error description
   */
  void graphics_error(GRAPHICS_ERROR_TYPE type, const char* file, const char* func, int line, const char* msg, ...);
#if defined(__cplusplus)
}
#endif
#endif // graphics_h

