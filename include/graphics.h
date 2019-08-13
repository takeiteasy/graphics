/* graphics.h
 *
 *           _____                     _____                     _____
 *          /\    \                   /\    \                   /\    \
 *         /::\____\                 /::\    \                 /::\____\
 *        /:::/    /                /::::\    \               /:::/    /
 *       /:::/    /                /::::::\    \             /:::/    /
 *      /:::/    /                /:::/\:::\    \           /:::/    /
 *     /:::/____/                /:::/__\:::\    \         /:::/    /
 *    /::::\    \               /::::\   \:::\    \       /:::/    /
 *   /::::::\    \   _____     /::::::\   \:::\    \     /:::/    /
 *  /:::/\:::\    \ /\    \   /:::/\:::\   \:::\    \   /:::/    /
 * /:::/  \:::\    /::\____\ /:::/  \:::\   \:::\____\ /:::/____/
 * \::/    \:::\  /:::/    / \::/    \:::\  /:::/    / \:::\    \
 *  \/____/ \:::\/:::/    /   \/____/ \:::\/:::/    /   \:::\    \
 *           \::::::/    /             \::::::/    /     \:::\    \
 *            \::::/    /               \::::/    /       \:::\    \
 *            /:::/    /                /:::/    /         \:::\    \
 *           /:::/    /                /:::/    /           \:::\    \
 *          /:::/    /                /:::/    /             \:::\    \
 *         /:::/    /                /:::/    /               \:::\____\
 *         \::/    /                 \::/    /                 \::/    /
 *          \/____/                   \/____/                   \/____/
 *
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
 *
 */

#ifndef graphics_h
#define graphics_h
#if defined(__cplusplus)
extern "C" {
#endif

#include "hal.h"

#if defined(_MSC_VER) && _MSC_VER <= 1600
#define bool int
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif

#define RGBA(r, g, b, a) ((((u32)(a)) << 24) | (((u32)(r)) << 16) | (((u32)(g)) << 8) | (b))
#define RGB(r, g, b) (RGBA((r), (g), (b), 255))
#define R(v) (((v) >> 16) & 0xFF)
#define G(v) (((v) >>  8) & 0xFF)
#define B(v) ( (v)        & 0xFF)
#define A(v) (((v) >> 24) & 0xFF)
#define RCHAN(a, b) (((a) & ~0x00FF0000) | ((b) << 16))
#define GCHAN(a, b) (((a) & ~0x0000FF00) | ((b) << 8))
#define BCHAN(a, b) (((a) & ~0x000000FF) |  (b))
#define ACHAN(a, b) (((a) & ~0xFF000000) | ((b) << 24))
#define RGB1(c) (RGB((c), (c), (c)))
#define RGBA1(c, a) (RGBA((c), (c), (c), (a)))

#if !defined(HAL_NO_COLORS)
  /*!
   * @typedef COLOURS
   * @brief A list of colours with names
   */
  typedef enum {
    BLACK = RGB(0, 0, 0),
    BLUE = RGB(0, 0, 255),
    CYAN = RGB(0, 255, 255),
    GRAY = RGB(128, 128, 128),
    GREEN = RGB(0, 128, 0),
    LIME = RGB(0, 255, 0),
    MAGENTA = RGB(255, 0, 255),
    MAROON = RGB(128, 0, 0),
    NAVY = RGB(0, 0, 128),
    PURPLE = RGB(128, 0, 128),
    RED = RGB(255, 0, 0),
    TEAL = RGB(0, 128, 128),
    WHITE = RGB(255, 255, 255),
    YELLOW = RGB(255, 255, 0),

    ALICE_BLUE = RGB(240, 248, 255),
    ANTIQUE_WHITE = RGB(250, 235, 215),
    AQUA = RGB(0, 255, 255),
    AQUA_MARINE = RGB(127, 255, 212),
    AZURE = RGB(240, 255, 255),
    BEIGE = RGB(245, 245, 220),
    BISQUE = RGB(255, 228, 196),
    BLANCHED_ALMOND = RGB(255, 235, 205),
    BLUE_VIOLET = RGB(138, 43, 226),
    BROWN = RGB(165, 42, 42),
    BURLY_WOOD = RGB(222, 184, 135),
    CADET_BLUE = RGB(95, 158, 160),
    CHART_REUSE = RGB(127, 255, 0),
    CHOCOLATE = RGB(210, 105, 30),
    CORAL = RGB(255, 127, 80),
    CORN_FLOWER_BLUE = RGB(100, 149, 237),
    CORN_SILK = RGB(255, 248, 220),
    CRIMSON = RGB(220, 20, 60),
    DARK_BLUE = RGB(0, 0, 139),
    DARK_CYAN = RGB(0, 139, 139),
    DARK_GOLDEN_ROD = RGB(184, 134, 11),
    DARK_GRAY = RGB(169, 169, 169),
    DARK_GREEN = RGB(0, 100, 0),
    DARK_KHAKI = RGB(189, 183, 107),
    DARK_MAGENTA = RGB(139, 0, 139),
    DARK_OLIVE_GREEN = RGB(85, 107, 47),
    DARK_ORANGE = RGB(255, 140, 0),
    DARK_ORCHID = RGB(153, 50, 204),
    DARK_RED = RGB(139, 0, 0),
    DARK_SALMON = RGB(233, 150, 122),
    DARK_SEA_GREEN = RGB(143, 188, 143),
    DARK_SLATE_BLUE = RGB(72, 61, 139),
    DARK_SLATE_GRAY = RGB(47, 79, 79),
    DARK_TURQUOISE = RGB(0, 206, 209),
    DARK_VIOLET = RGB(148, 0, 211),
    DEEP_PINK = RGB(255, 20, 147),
    DEEP_SKY_BLUE = RGB(0, 191, 255),
    DIM_GRAY = RGB(105, 105, 105),
    DODGER_BLUE = RGB(30, 144, 255),
    FIREBRICK = RGB(178, 34, 34),
    FLORAL_WHITE = RGB(255, 250, 240),
    FOREST_GREEN = RGB(34, 139, 34),
    GAINSBORO = RGB(220, 220, 220),
    GHOST_WHITE = RGB(248, 248, 255),
    GOLD = RGB(255, 215, 0),
    GOLDEN_ROD = RGB(218, 165, 32),
    GREEN_YELLOW = RGB(173, 255, 47),
    HONEYDEW = RGB(240, 255, 240),
    HOT_PINK = RGB(255, 105, 180),
    INDIAN_RED = RGB(205, 92, 92),
    INDIGO = RGB(75, 0, 130),
    IVORY = RGB(255, 255, 240),
    KHAKI = RGB(240, 230, 140),
    LAVENDER = RGB(230, 230, 250),
    LAVENDER_BLUSH = RGB(255, 240, 245),
    LAWN_GREEN = RGB(124, 252, 0),
    LEMON_CHIFFON = RGB(255, 250, 205),
    LIGHT_BLUE = RGB(173, 216, 230),
    LIGHT_CORAL = RGB(240, 128, 128),
    LIGHT_CYAN = RGB(224, 255, 255),
    LIGHT_GOLDEN_ROD = RGB(250, 250, 210),
    LIGHT_GRAY = RGB(211, 211, 211),
    LIGHT_GREEN = RGB(144, 238, 144),
    LIGHT_PINK = RGB(255, 182, 193),
    LIGHT_SALMON = RGB(255, 160, 122),
    LIGHT_SEA_GREEN = RGB(32, 178, 170),
    LIGHT_SKY_BLUE = RGB(135, 206, 250),
    LIGHT_SLATE_GRAY = RGB(119, 136, 153),
    LIGHT_STEEL_BLUE = RGB(176, 196, 222),
    LIGHT_YELLOW = RGB(255, 255, 224),
    LIME_GREEN = RGB(50, 205, 50),
    LINEN = RGB(250, 240, 230),
    MEDIUM_AQUA_MARINE = RGB(102, 205, 170),
    MEDIUM_BLUE = RGB(0, 0, 205),
    MEDIUM_ORCHID = RGB(186, 85, 211),
    MEDIUM_PURPLE = RGB(147, 112, 219),
    MEDIUM_SEA_GREEN = RGB(60, 179, 113),
    MEDIUM_SLATE_BLUE = RGB(123, 104, 238),
    MEDIUM_SPRING_GREEN = RGB(0, 250, 154),
    MEDIUM_TURQUOISE = RGB(72, 209, 204),
    MEDIUM_VIOLET_RED = RGB(199, 21, 133),
    MIDNIGHT_BLUE = RGB(25, 25, 112),
    MINT_CREAM = RGB(245, 255, 250),
    MISTY_ROSE = RGB(255, 228, 225),
    MOCCASIN = RGB(255, 228, 181),
    NAVAJO_WHITE = RGB(255, 222, 173),
    OLD_LACE = RGB(253, 245, 230),
    OLIVE_DRAB = RGB(107, 142, 35),
    ORANGE = RGB(255, 165, 0),
    ORANGE_RED = RGB(255, 69, 0),
    ORCHID = RGB(218, 112, 214),
    PALE_GOLDEN_ROD = RGB(238, 232, 170),
    PALE_GREEN = RGB(152, 251, 152),
    PALE_TURQUOISE = RGB(175, 238, 238),
    PALE_VIOLET_RED = RGB(219, 112, 147),
    PAPAYA_WHIP = RGB(255, 239, 213),
    PEACH_PUFF = RGB(255, 218, 185),
    PERU = RGB(205, 133, 63),
    PINK = RGB(255, 192, 203),
    PLUM = RGB(221, 160, 221),
    POWDER_BLUE = RGB(176, 224, 230),
    ROSY_BROWN = RGB(188, 143, 143),
    ROYAL_BLUE = RGB(65, 105, 225),
    SADDLE_BROWN = RGB(139, 69, 19),
    SALMON = RGB(250, 128, 114),
    SANDY_BROWN = RGB(244, 164, 96),
    SEA_GREEN = RGB(46, 139, 87),
    SEA_SHELL = RGB(255, 245, 238),
    SIENNA = RGB(160, 82, 45),
    SKY_BLUE = RGB(135, 206, 235),
    SLATE_BLUE = RGB(106, 90, 205),
    SLATE_GRAY = RGB(112, 128, 144),
    SNOW = RGB(255, 250, 250),
    SPRING_GREEN = RGB(0, 255, 127),
    STEEL_BLUE = RGB(70, 130, 180),
    TAN = RGB(210, 180, 140),
    THISTLE = RGB(216, 191, 216),
    TOMATO = RGB(255, 99, 71),
    TURQUOISE = RGB(64, 224, 208),
    VIOLET = RGB(238, 130, 238),
    WHEAT = RGB(245, 222, 179),
    WHITE_SMOKE = RGB(245, 245, 245),
    YELLOW_GREEN = RGB(154, 205, 50)
  } COLOURS;
#endif

#if defined(HAL_CHROMA_KEY) && !defined(BLIT_CHROMA_KEY)
#if !defined(HAL_NO_DEFAULT_COLORS)
#define BLIT_CHROMA_KEY LIME
#else
#define BLIT_CHROMA_KEY -16711936
#endif
#endif

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
#if defined(HAL_BDF)
    BDF_NO_CHAR_SIZE,
    BDF_NO_CHAR_LENGTH,
    BDF_TOO_MANY_BITMAPS,
    BDF_UNKNOWN_CHAR,
#endif
#if defined(HAL_GIF)
    GIF_LOAD_FAILED,
    GIF_SAVE_INVALID_SIZE,
    GIF_SAVE_FAILED,
#endif
#if defined(HAL_OPENGL)
    GL_SHADER_ERROR,
#endif
#if !defined(HAL_OSX)
    GL_LOAD_DL_FAILED,
    GL_GET_PROC_ADDR_FAILED,
#else
    CURSOR_MOD_FAILED,
#if defined(HAL_METAL)
    MTK_LIBRARY_ERROR,
    MTK_CREATE_PIPELINE_FAILED,
#endif
    OSX_WINDOW_CREATION_FAILED,
    OSX_APPDEL_CREATION_FAILED,
    OSX_FULLSCREEN_FAILED,
#endif
#if defined(HAL_WINDOWS)
#if defined(HAL_DX9)
#elif defined(HAL_OPENGL)
    WIN_GL_PF_ERROR,
#endif
    WIN_WINDOW_CREATION_FAILED,
    WIN_FULLSCREEN_FAILED,
#elif defined(HAL_LINUX)
    NIX_CURSOR_PIXMAP_ERROR,
    NIX_OPEN_DISPLAY_FAILED,
#if defined(HAL_OPENGL)
    NIX_GL_FB_ERROR,
    NIX_GL_CONTEXT_ERROR,
#endif
    NIX_WINDOW_CREATION_FAILED,
#endif
    WINDOW_ICON_FAILED,
    CUSTOM_CURSOR_NOT_CREATED
  } ERROR_TYPE;

  /*!
   * @discussion Callback for errors inside library
   * @param cb Function pointer to callback
   */
  HALDEF void  hal_error_callback(void(*cb)(ERROR_TYPE, const char*, const char*, const char*, i32));

  /*!
   * @typedef surface_t
   * @brief An object to hold image data
   * @constant buf Buffer holding pixel data
   * @constant w Width of image
   * @constant h Height of image
   */
  typedef struct surface_t* surface_t;

  /*!
   * @discussion Get size of given surface
   * @param s Surface object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  HALDEF void hal_surface_size(surface_t s, int* w, int* h);
  /*!
   * @discussion Get pointer to buffer of given surface
   * @param s Surface object
   * @return The pointer to surface buffer
   */
  HALDEF i32* hal_surface_raw(surface_t s);
  /*!
   * @discussion Create a new surface
   * @param s Pointer to surface object to create
   * @param w Width of new surface
   * @param h Height of new surface
   * @return Boolean for success
   */
  HALDEF bool hal_surface(surface_t* s, u32 w, u32 h);
  /*!
   * @discussion Destroy a surface
   * @param s Pointer to pointer to surface object
   */
  HALDEF void hal_destroy(surface_t* s);

  /*!
   * @discussion Fill a surface with a given colour
   * @param s Surface object
   * @param col Colour to set
   */
  HALDEF void hal_fill(surface_t s, i32 col);
  /*!
   * @discussion Flood portion of surface with given colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param col Colour to set
   */
  HALDEF void hal_flood(surface_t s, i32 x, i32 y, i32 col);
  /*!
   * @discussion Clear a surface, zero the buffer
   * @param s Surface object
   */
  HALDEF void hal_cls(surface_t s);
  /*!
   * @discussion Set surface pixel colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param col Colour to set
   */
  HALDEF void hal_pset(surface_t s, i32 x, i32 y, i32 col);
  /*!
   * @discussion Get surface pixel colour
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @return Pixel colour
   */
  HALDEF i32  hal_pget(surface_t s, i32 x, i32 y);
  /*!
   * @discussion Blit one surface onto another at point
   * @param dst Surface to blit to
   * @param src Surface to blit
   * @param x X position
   * @param y Y position
   * @return Boolean of success
   */
  HALDEF bool hal_paste(surface_t dst, surface_t src, i32 x, i32 y);
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
  HALDEF bool hal_clip_paste(surface_t dst, surface_t src, i32 x, i32 y, i32 rx, i32 ry, i32 rw, i32 rh);
  /*!
   * @discussion Reallocate a surface
   * @param s Surface object
   * @param nw New width
   * @param nh New height
   * @return Boolean of success
   */
  HALDEF bool hal_reset(surface_t s, i32 nw, i32 nh);
  /*!
   * @discussion Create a copy of a surface
   * @param a Original surface object
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  HALDEF bool hal_copy(surface_t a, surface_t* b);
  /*!
   * @discussion Loop through each pixel of surface and run position and colour through a callback. Return value of the callback is the new colour at the position
   * @param s Surface object
   * @param fn Callback function
   */
  HALDEF void hal_passthru(surface_t s, i32(*fn)(i32 x, i32 y, i32 col));
  /*!
   * @discussion Resize (and scale) surface to given size
   * @param a Original surface object
   * @param nw New width
   * @param nh New height
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  HALDEF bool hal_resize(surface_t a, i32 nw, i32 nh, surface_t* b);
  /*!
   * @discussion Rotate a surface by a given degree
   * @param a Original surface object
   * @param angle Angle to rotate by
   * @param b New surface object to be allocated
   * @return Boolean of success
   */
  HALDEF bool hal_rotate(surface_t a, float angle, surface_t* b);
  /*!
   * @discussion https://en.wikipedia.org/wiki/Color_quantization
   * @param a Original surface object
   * @param n_colors Maximum colours
   * @param b New surface object to be allocated
   */
  HALDEF void hal_quantize(surface_t a, i32 n_colors, surface_t* b);

  /*!
   * @discussion Simple Bresenham line
   * @param s Surface object
   * @param x0 Vector A X position
   * @param y0 Vector A Y position
   * @param x1 Vector B X position
   * @param y1 Vector B Y position
   * @param col Colour of line
   */
  HALDEF void hal_line(surface_t s, i32 x0, i32 y0, i32 x1, i32 y1, i32 col);
  /*!
   * @discussion Draw a circle
   * @param s Surface object
   * @param xc Centre X position
   * @param yc Centre Y position
   * @param r Circle radius
   * @param col Colour of cricle
   * @param fill Fill circle boolean
   */
  HALDEF void hal_circle(surface_t s, i32 xc, i32 yc, i32 r, i32 col, bool fill);
  /*!
   * @discussion Draw a rectangle
   * @param x X position
   * @param y Y position
   * @param w Rectangle width
   * @param h Rectangle height
   * @param col Colour of rectangle
   * @param fill Fill rectangle boolean
   */
  HALDEF void hal_rect(surface_t s, i32 x, i32 y, i32 w, i32 h, i32 col, bool fill);
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
  HALDEF void hal_tri(surface_t s, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2, i32 col, bool fill);

  /*!
   * @discussion Load BMP file from path
   * @param s Surface object to allocate
   * @param path Path to BMP file
   * @return Boolean of success
   */
  HALDEF bool hal_bmp(surface_t* s, const char* path);

#if !defined(HAL_NO_TEXT)
  /*!
   * @discussion Draw a character from ASCII value using default in-built font
   * @param s Surface object
   * @param ch ASCII character code
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   */
  HALDEF void hal_ascii(surface_t s, i8 ch, i32 x, i32 y, i32 fg, i32 bg);
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
  HALDEF i32 hal_character(surface_t s, const char* ch, i32 x, i32 y, i32 fg, i32 bg);
  /*!
   * @discussion Draw a string using default in-built font
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  HALDEF void hal_writeln(surface_t s, i32 x, i32 y, i32 fg, i32 bg, const char* str);
  /*!
   * @discussion Draw a string using default in-built font
   * @param s Surface object
   * @param x X position
   * @param y Y position
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  HALDEF void hal_writelnf(surface_t s, i32 x, i32 y, i32 fg, i32 bg, const char* fmt, ...);
  /*!
   * @discussion Create a surface object for text using default in-built font
   * @param s Surface object to be allocated
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  HALDEF void hal_string(surface_t* s, i32 fg, i32 bg, const char* str);
  /*!
   * @discussion Create a surface object for formatted text using default in-built font
   * @param s Surface object to be allocated
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  HALDEF void hal_stringf(surface_t* s, i32 fg, i32 bg, const char* fmt, ...);
#endif

  /*!
   * @discussion Get current CPU time
   * @return CPU time
   */
  HALDEF i64 hal_ticks(void);
  /*!
   * @discussion Sleep in milliseconds
   * @param ms Durection in milliseconds
   */
  HALDEF void hal_delay(i64 ms);

#if defined(HAL_BDF)
  /*!
   * @typedef bdf_t
   * @brief BDF font object
   */
  typedef struct bdf_t* bdf_t;

  /*!
   * @discussion Destroy a BDF font object
   * @param f Pointer to BDF font object
   */
  HALDEF void hal_bdf_destroy(bdf_t* f);
  /*!
   * @discussion Load a BDF font from path
   * @param out BDF object to be allocated
   * @param path Path of BDF file
   * @return Boolean of success
   */
  HALDEF bool hal_bdf(bdf_t* out, const char* path);
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
  HALDEF i32 hal_bdf_character(surface_t s, bdf_t f, const char* ch, i32 x, i32 y, i32 fg, i32 bg);
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
  HALDEF void hal_bdf_writeln(surface_t s, bdf_t f, i32 x, i32 y, i32 fg, i32 bg, const char* str);
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
  HALDEF void hal_bdf_writelnf(surface_t s, bdf_t f, i32 x, i32 y, i32 fg, i32 bg, const char* fmt, ...);
  /*!
   * @discussion Create a surface object for text using BDF font object
   * @param s Surface object to be allocated
   * @param f BDF font object
   * @param fg Foreground colour
   * @param bg Background colour
   * @param str String to write
   */
  HALDEF void hal_bdf_string(surface_t* s, bdf_t f, i32 fg, i32 bg, const char* str);
  /*!
   * @discussion Create a surface object for formatted text using BDF font object
   * @param s Surface object to be allocated
   * @param f BDF font object
   * @param fg Foreground colour
   * @param bg Background colour
   * @param fmt Format string
   */
  HALDEF void hal_bdf_stringf(surface_t* s, bdf_t f, i32 fg, i32 bg, const char* fmt, ...);
#endif

#if defined(HAL_GIF)
  /*!
   * @typedef gif_t
   * @brief GIF image object
   */
  typedef struct gif_t* gif_t;

  /*!
   * @discussion Get the delay between frames for a GIF
   * @param g GIF object
   * @return Frame delay for GIF
   */
  HALDEF i32 hal_gif_delay(gif_t g);
  /*!
   * @discussion Get the total frames for a GIF
   * @param g GIF object
   * @return Total frames for GIF
   */
  HALDEF i32 hal_gif_total_frames(gif_t g);
  /*!
   * @discussion Get GIF object's current frame
   * @param g GIF object
   * @return Current frame index
   */
  HALDEF i32 hal_gif_current_frame(gif_t g);
  /*!
   * @discussion Get GIF object's dimentions
   * @param g GIF object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  HALDEF void hal_gif_size(gif_t g, i32* w, i32* h);
  /*!
   * @discussion Advance GIF frame
   * @param g GIF object
   * @return New current frame index
   */
  HALDEF i32 hal_gif_next_frame(gif_t g);
  /*!
   * @discussion Get the current frame for GIF object
   * @param g GIF object
   * @param n New index
   */
  HALDEF void hal_gif_set_frame(gif_t g, i32 n);
  /*!
   * @discussion Get the surface object for GIF object's current frame
   * @param g GIF object
   * @return Pointer to surface object for GIF's current frame
   */
  surface_t hal_gif_frame(gif_t g);
  /*!
   * @discussion Create a new GIF object from surface objects
   * @param g GIF object to be allocated
   * @param w GIF object width
   * @param h GIF object height
   * @param delay GIF object's frame delay
   * @param frames Total number of frames
   * @param ... Surface objects
   */
  HALDEF void hal_gif_create(gif_t* g, i32 w, i32 h, i32 delay, i32 frames, ...);

  /*!
   * @discussion Load GIF from path
   * @param g GIF object to be allocated
   * @param path Path to GIF file
   * @return Boolean of success
   */
  HALDEF bool hal_gif(gif_t* g, const char* path);
  /*!
   * @discussion Save GIF to path
   * @param g GIF object
   * @param path Path to save GIF file
   * @return Boolean of success
   */
  HALDEF bool hal_save_gif(gif_t* g, const char* path);
  /*!
   * @discussion Destroy GIF object
   * @param g Pointer to GIF object
   */
  HALDEF void hal_gif_destroy(gif_t* g);
#endif

#if !defined(HAL_NO_ALERTS)
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
  HALDEF bool hal_alert(ALERT_LVL lvl, ALERT_BTNS btns, const char* fmt, ...);
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
  HALDEF char* hal_dialog(DIALOG_ACTION action, const char* path, const char* fname, bool allow_multiple, i32 nfilters, ...);
#endif

#if !defined(HAL_NO_WINDOW)
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

  typedef struct window_t* window_t;

  /*!
   * @discussion Set "parent" for a window object. The parent pointer will be passed to window callbacks.
   * @param s Window object
   * @param p Pointer to parent
   */
  HALDEF void hal_window_set_parent(window_t s, void* p);
  /*!
   * @discussion Get parent point from window object
   * @param s Window object
   * @return Point to parent
   */
  HALDEF void* hal_window_parent(window_t s);

#define XMAP_SCREEN_CB \
  X(keyboard, (void*, KEY_SYM, KEY_MOD, bool)) \
  X(mouse_button, (void*, MOUSE_BTN, KEY_MOD, bool)) \
  X(mouse_move, (void*, i32, i32, i32, i32)) \
  X(scroll, (void*, KEY_MOD, float, float)) \
  X(focus, (void*, bool)) \
  X(resize, (void*, i32, i32)) \
  X(closed, (void*))

#define X(a, b) \
  void(*a##_cb)b,
  /*!
   * @discussion Set callbacks for window object
   * @param keyboard Keyboard callback
   * @param mouse_button Mouse click callback
   * @param mouse_move Mouse movement callback
   * @param scroll Mouse scroll callback
   * @param focus Window focus/blur callback
   * @param resize Window resize callback
   * @param closed Window closed callback
   * @param s Window object
   */
  HALDEF void hal_window_callbacks(XMAP_SCREEN_CB window_t window);
#undef X
#define X(a, b) \
  HALDEF void hal_##a##_callback(window_t window, void(*a##_cb)b);
  XMAP_SCREEN_CB
#undef X

#define DEFAULT 0

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

#define SHOWN true
#define HIDDEN false
#define LOCKED true
#define UNLOCKED false

  /*!
   * @typedef WINDOW_FLAGS
   * @brief A list of window flag options
   */
  typedef enum {
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
  HALDEF bool hal_window(window_t* s, const char* t, i32 w, i32 h, i16 flags);
  /*!
   * @discussion Set window icon from surface object
   * @param s Window object
   * @param b Surface object
   */
  HALDEF void hal_window_icon(window_t s, surface_t b);
  /*!
   * @discussion Set window title
   * @param s Window object
   * @param t New title
   */
  HALDEF void hal_window_title(window_t s, const char* t);
  /*!
   * @discussion Get the position of a window object
   * @param s Window object
   * @param x Pointer to int to set
   * @param y Pointer to int to set
   */
  HALDEF void hal_window_position(window_t s, int* x, int*  y);
  /*!
   * @discussion Get the size of the screen a window is on
   * @param s Window object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  HALDEF void hal_screen_size(window_t s, int* w, int* h);
  /*!
   * @discussion Destroy window object
   * @param s Window object
   */
  HALDEF void hal_window_destroy(window_t* s);
  /*!
   * @discussion Unique window ID for window object
   * @param s Window object
   * @return Unique ID of window object
   */
  HALDEF i32  hal_window_id(window_t s);
  /*!
   * @discussion Get size of window
   * @param s Window object
   * @param w Pointer to int to set
   * @param h Pointer to int to set
   */
  HALDEF void hal_window_size(window_t s, i32* w, i32* h);
  /*!
   * @discussion Check if a window is still open
   * @param s Window object
   * @return Boolean if window is open
   */
  HALDEF bool hal_closed(window_t s);

  /*!
   * @discussion Lock or unlock cursor movement to active window
   * @param locked Turn on or off
   */
  HALDEF void hal_cursor_lock(bool locked);
  /*!
   * @discussion Hide or show system cursor
   * @param show Hide or show
   */
  HALDEF void hal_cursor_visible(bool show);
  /*!
   * @discussion Change cursor icon to system icon
   * @param s Window object
   * @param t Type of cursor
   */
  HALDEF void hal_cursor_icon(window_t s, CURSOR_TYPE t);
  /*!
   * @discussion Change cursor icon to icon from surface object
   * @param s Window object
   * @param b Surface object
   */
  HALDEF void hal_cursor_custom_icon(window_t s, surface_t b);
  /*!
   * @discussion Get cursor position
   * @param x Integer to set
   * @param y Integer to set
   */
  HALDEF void hal_cursor_pos(i32* x, i32* y);
  /*!
   * @discussion Set cursor position
   * @param x X position
   * @param y Y position
   */
  HALDEF void hal_cursor_set_pos(i32 x, i32 y);

  /*!
   * @discussion Poll for window events
   */
  HALDEF void hal_poll(void);
  /*!
   * @discussion Draw surface object to window
   * @param s Window object
   * @param b Surface object
   */
  HALDEF void hal_flush(window_t s, surface_t b);
  /*!
   * @discussion Release anything allocated by this library
   */
  HALDEF void hal_release(void);
#endif

#if defined(__cplusplus)
}
#endif
#endif // graphics_h

