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

#if defined(__gnu_linux__) || defined(__linux__) || defined(__unix__)
# define SGL_LINUX
#elif defined(macintosh) || defined(Macintosh) || (defined(__APPLE__) && defined(__MACH__))
# define SGL_OSX
#elif defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WINDOWS__)
# define SGL_WINDOWS
#else
# error Unsupported operating system
#endif

#if defined(SGL_OSX)
# if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_13
#   if !defined(SGL_ENABLE_OPENGL)
#     define SGL_ENABLE_METAL // Force Metal on 10.13 until CoreGraphics works again
#   endif
# else
#   if defined(SGL_ENABLE_METAL) && defined(SGL_ENABLE_OPENGL)
#     undef SGL_ENABLE_OPENGL
#   endif
# endif
#elif defined(SGL_WINDOWS)
# if (defined(SGL_ENABLE_DX9) || defined(SGL_ENABLE_DX11) || defined(SGL_ENABLE_VULKAN)) && defined(SGL_ENABLE_OPENGL)
#   undef SGL_ENABLE_OPENGL
# endif
# if (defined(SGL_ENABLE_DX9) || defined(SGL_ENABLE_DX11)) && defined(SGL_ENABLE_VULKAN)
#   undef SGL_ENABLE_VULKAN
# endif
# if defined(SGL_ENABLE_DX9) && defined(SGL_ENABLE_DX11)
#   undef SGL_ENABLE_DX9
# endif
#else
# if defined(SGL_ENABLE_VULKAN) && defined(SGL_ENABLE_OPENGL)
#   undef SGL_ENABLE_OPENGL
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if defined(_MSC_VER) && _MSC_VER <= 1600
#define bool int
#define true 1
#define false 0
#else
#include <stdbool.h>
#endif
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#if defined(SGL_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#if !defined(SGL_DISABLE_RGBA)
#define RGBA(r, g, b, a) ((((unsigned int)(a)) << 24) | (((unsigned int)(r)) << 16) | (((unsigned int)(g)) << 8) | (b))
#define RGB(r, g, b) (RGBA((r), (g), (b), 255))
#else
#define RGB(r, g, b) ((((unsigned int)(r)) << 16) | (((unsigned int)(g)) << 8) | (b))
#define RGBA(r, g, b, a) (RGB(r, g, b))
#endif
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

#if !defined(SGL_DISABLE_DEFAULT_COLORS)
#define BLACK RGB(0, 0, 0)
#define BLUE RGB(0, 0, 255)
#define CYAN (0, 255, 255)
#define GRAY RGB(128, 128, 128)
#define GREEN RGB(0, 128, 0)
#define LIME RGB(0, 255, 0)
#define MAGENTA RGB(255, 0, 255)
#define MAROON RGB(128, 0, 0)
#define NAVY RGB(0, 0, 128)
#define PURPLE RGB(128, 0, 128)
#define RED RGB(255, 0, 0)
#define TEAL RGB(0, 128, 128)
#define WHITE RGB(255, 255, 255)
#define YELLOW RGB(255, 255, 0)

#define ALICE_BLUE RGB(240, 248, 255)
#define ANTIQUE_WHITE RGB(250, 235, 215)
#define AQUA RGB(0, 255, 255)
#define AQUA_MARINE RGB(127, 255, 212)
#define AZURE RGB(240, 255, 255)
#define BEIGE RGB(245, 245, 220)
#define BISQUE RGB(255, 228, 196)
#define BLANCHED_ALMOND RGB(255, 235, 205)
#define BLUE_VIOLET RGB(138, 43, 226)
#define BROWN RGB(165, 42, 42)
#define BURLY_WOOD RGB(222, 184, 135)
#define CADET_BLUE RGB(95, 158, 160)
#define CHART_REUSE RGB(127, 255, 0)
#define CHOCOLATE RGB(210, 105, 30)
#define CORAL RGB(255, 127, 80)
#define CORN_FLOWER_BLUE RGB(100, 149, 237)
#define CORN_SILK RGB(255, 248, 220)
#define CRIMSON RGB(220, 20, 60)
#define DARK_BLUE RGB(0, 0, 139)
#define DARK_CYAN RGB(0, 139, 139)
#define DARK_GOLDEN_ROD RGB(184, 134, 11)
#define DARK_GRAY RGB(169, 169, 169)
#define DARK_GREEN RGB(0, 100, 0)
#define DARK_KHAKI RGB(189, 183, 107)
#define DARK_MAGENTA RGB(139, 0, 139)
#define DARK_OLIVE_GREEN RGB(85, 107, 47)
#define DARK_ORANGE RGB(255, 140, 0)
#define DARK_ORCHID RGB(153, 50, 204)
#define DARK_RED RGB(139, 0, 0)
#define DARK_SALMON RGB(233, 150, 122)
#define DARK_SEA_GREEN RGB(143, 188, 143)
#define DARK_SLATE_BLUE RGB(72, 61, 139)
#define DARK_SLATE_GRAY RGB(47, 79, 79)
#define DARK_TURQUOISE RGB(0, 206, 209)
#define DARK_VIOLET RGB(148, 0, 211)
#define DEEP_PINK RGB(255, 20, 147)
#define DEEP_SKY_BLUE RGB(0, 191, 255)
#define DIM_GRAY RGB(105, 105, 105)
#define DODGER_BLUE RGB(30, 144, 255)
#define FIREBRICK RGB(178, 34, 34)
#define FLORAL_WHITE RGB(255, 250, 240)
#define FOREST_GREEN RGB(34, 139, 34)
#define GAINSBORO RGB(220, 220, 220)
#define GHOST_WHITE RGB(248, 248, 255)
#define GOLD RGB(255, 215, 0)
#define GOLDEN_ROD RGB(218, 165, 32)
#define GREEN_YELLOW RGB(173, 255, 47)
#define HONEYDEW RGB(240, 255, 240)
#define HOT_PINK RGB(255, 105, 180)
#define INDIAN_RED RGB(205, 92, 92)
#define INDIGO RGB(75, 0, 130)
#define IVORY RGB(255, 255, 240)
#define KHAKI RGB(240, 230, 140)
#define LAVENDER RGB(230, 230, 250)
#define LAVENDER_BLUSH RGB(255, 240, 245)
#define LAWN_GREEN RGB(124, 252, 0)
#define LEMON_CHIFFON RGB(255, 250, 205)
#define LIGHT_BLUE RGB(173, 216, 230)
#define LIGHT_CORAL RGB(240, 128, 128)
#define LIGHT_CYAN RGB(224, 255, 255)
#define LIGHT_GOLDEN ROD YELLOW RGB(250, 250, 210)
#define LIGHT_GRAY RGB(211, 211, 211)
#define LIGHT_GREEN RGB(144, 238, 144)
#define LIGHT_PINK RGB(255, 182, 193)
#define LIGHT_SALMON RGB(255, 160, 122)
#define LIGHT_SEA_GREEN RGB(32, 178, 170)
#define LIGHT_SKY_BLUE RGB(135, 206, 250)
#define LIGHT_SLATE_GRAY RGB(119, 136, 153)
#define LIGHT_STEEL_BLUE RGB(176, 196, 222)
#define LIGHT_YELLOW RGB(255, 255, 224)
#define LIME_GREEN RGB(50, 205, 50)
#define LINEN RGB(250, 240, 230)
#define MEDIUM_AQUA_MARINE RGB(102, 205, 170)
#define MEDIUM_BLUE RGB(0, 0, 205)
#define MEDIUM_ORCHID RGB(186, 85, 211)
#define MEDIUM_PURPLE RGB(147, 112, 219)
#define MEDIUM_SEA_GREEN RGB(60, 179, 113)
#define MEDIUM_SLATE_BLUE RGB(123, 104, 238)
#define MEDIUM_SPRING_GREEN RGB(0, 250, 154)
#define MEDIUM_TURQUOISE RGB(72, 209, 204)
#define MEDIUM_VIOLET_RED RGB(199, 21, 133)
#define MIDNIGHT_BLUE RGB(25, 25, 112)
#define MINT_CREAM RGB(245, 255, 250)
#define MISTY_ROSE RGB(255, 228, 225)
#define MOCCASIN RGB(255, 228, 181)
#define NAVAJO_WHITE RGB(255, 222, 173)
#define OLD_LACE RGB(253, 245, 230)
#define OLIVE_DRAB RGB(107, 142, 35)
#define ORANGE RGB(255, 165, 0)
#define ORANGE_RED RGB(255, 69, 0)
#define ORCHID RGB(218, 112, 214)
#define PALE_GOLDEN_ROD RGB(238, 232, 170)
#define PALE_GREEN RGB(152, 251, 152)
#define PALE_TURQUOISE RGB(175, 238, 238)
#define PALE_VIOLET_RED RGB(219, 112, 147)
#define PAPAYA_WHIP RGB(255, 239, 213)
#define PEACH_PUFF RGB(255, 218, 185)
#define PERU RGB(205, 133, 63)
#define PINK RGB(255, 192, 203)
#define PLUM RGB(221, 160, 221)
#define POWDER_BLUE RGB(176, 224, 230)
#define ROSY_BROWN RGB(188, 143, 143)
#define ROYAL_BLUE RGB(65, 105, 225)
#define SADDLE_BROWN RGB(139, 69, 19)
#define SALMON RGB(250, 128, 114)
#define SANDY_BROWN RGB(244, 164, 96)
#define SEA_GREEN RGB(46, 139, 87)
#define SEA_SHELL RGB(255, 245, 238)
#define SIENNA RGB(160, 82, 45)
#define SKY_BLUE RGB(135, 206, 235)
#define SLATE_BLUE RGB(106, 90, 205)
#define SLATE_GRAY RGB(112, 128, 144)
#define SNOW RGB(255, 250, 250)
#define SPRING_GREEN RGB(0, 255, 127)
#define STEEL_BLUE RGB(70, 130, 180)
#define TAN RGB(210, 180, 140)
#define THISTLE RGB(216, 191, 216)
#define TOMATO RGB(255, 99, 71)
#define TURQUOISE RGB(64, 224, 208)
#define VIOLET RGB(238, 130, 238)
#define WHEAT RGB(245, 222, 179)
#define WHITE_SMOKE RGB(245, 245, 245)
#define YELLOW_GREEN RGB(154, 205, 50)
#endif

#if !defined(SGL_DISABLE_CHROMA_KEY) && !defined(BLIT_CHROMA_KEY)
#define BLIT_CHROMA_KEY LIME
#endif
  
  typedef struct {
    int x, y, w, h;
  } rect_t;

  typedef struct {
    int x, y;
  } point_t;

  typedef enum {
    LOW_PRIORITY = 0,
    NORMAL_PRIORITY = 1,
    HIGH_PRIORITY = 2
  } ERRORLVL;

  typedef enum {
    OUT_OF_MEMEORY,
    FILE_OPEN_FAILED,
    INVALID_BMP,
    UNSUPPORTED_BMP,
    INVALID_PARAMETERS,
#if defined(SGL_ENABLE_BDF)
    BDF_NO_CHAR_SIZE,
    BDF_NO_CHAR_LENGTH,
    BDF_TOO_MANY_BITMAPS,
    BDF_UNKNOWN_CHAR,
#endif
#if defined(SGL_ENABLE_STB_IMAGE)
    STBI_LOAD_FAILED,
    STBI_WRITE_FAILED,
#endif
#if defined(SGL_ENABLE_GIF)
    GIF_LOAD_FAILED,
    GIF_SAVE_INVALID_SIZE,
    GIF_SAVE_FAILED,
#endif
#if defined(SGL_ENABLE_OPENGL)
    GL_SHADER_ERROR,
#endif
#if !defined(SGL_OSX)
    GL_LOAD_DL_FAILED,
    GL_GET_PROC_ADDR_FAILED,
#else
    CURSOR_MOD_FAILED,
#if defined(SGL_ENABLE_METAL)
    MTK_LIBRARY_ERROR,
    MTK_CREATE_PIPELINE_FAILED,
#endif
    OSX_WINDOW_CREATION_FAILED,
    OSX_APPDEL_CREATION_FAILED,
    OSX_FULLSCREEN_FAILED,
#endif
#if defined(SGL_WINDOWS)
#if defined(SGL_ENABLE_JOYSTICKS)
    JOY_DI_SETPROP_FAILED,
    JOY_DI_CREATE_DEVICE_FAILED,
    JOY_DI_LOADDL_FAILED,
    JOY_DI_INIT_FAILED,
    JOY_DI_ENUM_DEVICE_FAILED,
    JOY_XI_LOADDL_FAILED,
#endif
#if defined(SGL_ENABLE_DX9)
#elif defined(SGL_ENABLE_OPENGL)
    WIN_GL_PF_ERROR,
#endif
    WIN_WINDOW_CREATION_FAILED,
    WIN_FULLSCREEN_FAILED,
#elif defined(SGL_LINUX)
#if defined(SGL_ENABLE_JOYSTICKS)
    JOY_NIX_SCAN_FAILED,
#endif
    NIX_CURSOR_PIXMAP_ERROR,
    NIX_OPEN_DISPLAY_FAILED,
#if defined(SGL_ENABLE_OPENGL)
    NIX_GL_FB_ERROR,
    NIX_GL_CONTEXT_ERROR,
#endif
    NIX_WINDOW_CREATION_FAILED,
#endif
    WINDOW_ICON_FAILED,
    CUSTOM_CURSOR_NOT_CREATED
  } ERRORTYPE;

  void sgl_set_userdata(void* data);
  void* sgl_get_userdata(void);
  void sgl_error_callback(void(*cb)(void*, ERRORLVL, ERRORTYPE, const char*, const char*, const char*, int));

  typedef struct surface_t* surface_t;
  
  void sgl_surface_size(surface_t s, int* w, int* h);
  int sgl_surface_width(surface_t s);
  int sgl_surface_height(surface_t s);
  int* sgl_surface_raw(surface_t s);
  bool sgl_surface(surface_t* s, unsigned int w, unsigned int h);
  void sgl_destroy(surface_t*);
  void sgl_fill(surface_t s, int col);
  void sgl_flood(surface_t s, int x, int y, int col);
  void sgl_cls(surface_t s);
  void sgl_pset(surface_t s, int x, int y, int col);
  void sgl_psetb(surface_t s, int x, int y, int col);
  int sgl_pget(surface_t s, int x, int y);
  bool sgl_blit(surface_t dst, point_t* p, surface_t src, rect_t* rect);
  bool sgl_reset(surface_t s, int nw, int nh);
  bool sgl_copy(surface_t a, surface_t* b);
  void sgl_filter(surface_t s, int(*fn)(int x, int y, int col));
  bool sgl_resize(surface_t a, int nw, int nh, surface_t* b);
  bool sgl_rotate(surface_t a, float angle, surface_t* b);
  void sgl_quantization(surface_t a, int n_colors, surface_t* b);

  void sgl_vline(surface_t s, int x, int y0, int y1, int col);
  void sgl_hline(surface_t s, int y, int x0, int x1, int col);
  void sgl_line(surface_t s, int x0, int y0, int x1, int y1, int col);
  void sgl_circle(surface_t s, int xc, int yc, int r, int col, int fill);
  void sgl_ellipse(surface_t s, int xc, int yc, int rx, int ry, int col, int fill);
  void sgl_ellipse_rotated(surface_t s, int x, int y, int a, int b, float angle, int col);
  void sgl_ellipse_rect(surface_t s, int x0, int y0, int x1, int y1, int col, int fill);
  void sgl_ellipse_rect_rotated(surface_t s, int x0, int y0, int x1, int y1, long zd, int col);
  void sgl_bezier(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, int col);
  void sgl_bezier_rational(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, float w, int col);
  void sgl_bezier_cubic(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, int col);
  void sgl_rect(surface_t s, int x, int y, int w, int h, int col, int fill);

  bool sgl_bmp(surface_t* s, const char* path);
  bool sgl_save_bmp(surface_t s, const char* path);

#if !defined(SGL_DISABLE_TEXT)
  void sgl_ascii(surface_t s, char ch, int x, int y, int fg, int bg);
  int sgl_character(surface_t s, const char* ch, int x, int y, int fg, int bg);
  void sgl_writeln(surface_t s, int x, int y, int fg, int bg, const char* str);
  void sgl_writelnf(surface_t s, int x, int y, int fg, int bg, const char* fmt, ...);
  void sgl_string(surface_t* s, int fg, int bg, const char* str);
  void sgl_stringf(surface_t* s, int fg, int bg, const char* fmt, ...);
#endif

  long sgl_ticks(void);
  void sgl_delay(long ms);

#if defined(SGL_ENABLE_BDF)
  typedef struct bdf_t* bdf_t;
  void sgl_bdf_destroy(bdf_t* f);
  bool sgl_bdf(bdf_t* out, const char* path);
  int sgl_bdf_character(surface_t s, bdf_t f, const char* ch, int x, int y, int fg, int bg);
  void sgl_bdf_writeln(surface_t s, bdf_t f, int x, int y, int fg, int bg, const char* str);
  void sgl_bdf_writelnf(surface_t s, bdf_t f, int x, int y, int fg, int bg, const char* fmt, ...);
  void sgl_bdf_string(surface_t* s, bdf_t f, int fg, int bg, const char* str);
  void sgl_bdf_stringf(surface_t* s, bdf_t f, int fg, int bg, const char* fmt, ...);
#endif
  
#if defined(SGL_ENABLE_FREETYPE)
  typedef struct ftfont_t* ftfont_t;
  
  void sgl_ft_init(void);
  void sgl_ft_release(void);
  void sgl_ftfont(ftfont_t* font, const char* path, unsigned int size);
  void sgl_ftfont_destroy(ftfont_t* font);
  int sgl_ftfont_character(surface_t s, ftfont_t f, const char* ch, int x, int y, int fg, int bg, int* w, int* h);
  void sgl_ftfont_writeln(surface_t s, ftfont_t f, int x, int y, int fg, int bg, const char* str);
  void sgl_ftfont_writelnf(surface_t s, ftfont_t f, int x, int y, int fg, int bg, const char* fmt, ...);
  void sgl_ftfont_string(surface_t* s, ftfont_t f, int fg, int bg, const char* str);
  void sgl_ftfont_stringf(surface_t* s, ftfont_t f, int fg, int bg, const char* fmt, ...);
#endif

#if defined(SGL_ENABLE_STB_IMAGE)
  typedef enum {
    PNG,
    TGA,
    BMP,
    JPG
  } SAVEFORMAT;
  
  bool sgl_image(surface_t* s, const char* path);
  bool sgl_save_image(surface_t a, const char* path, SAVEFORMAT type);
#endif
  
#if defined(SGL_ENABLE_GIF)
  typedef struct gif_t* gif_t;
  
  int sgl_gif_delay(gif_t g);
  int sgl_gif_total_frames(gif_t g);
  int sgl_gif_current_frame(gif_t g);
  void sgl_gif_size(gif_t g, int* w, int* h);
  int sgl_gif_next_frame(gif_t g);
  void sgl_gif_set_frame(gif_t g, int n);
  surface_t sgl_gif_frame(gif_t g);
  
  bool sgl_gif(gif_t* g, const char* path);
  bool sgl_save_gif(gif_t* g, const char* path);
  void sgl_gif_destroy(gif_t* g);
#endif
  
#if defined(SGL_ENABLE_JOYSTICKS)
  typedef struct __joystick_t {
    const char* description;
    int device_id, vendor_id, product_id;
    
    unsigned int n_axes, n_buttons;
    float* axes;
    int* buttons;
    
    struct __joystick_t* next;
    
    void* __private;
  } joystick_t;
  
#define XMAP_JOYSTICK_CB \
  X(connect, (void*, joystick_t*, int), connect) \
  X(removed, (void*, joystick_t*, int), remove) \
  X(btn, (void*, joystick_t*, int, bool, long), button) \
  X(axis, (void*, joystick_t*, int, float, float, long), axis)
  
#define X(a, b, c) \
void(*a##_cb)b,
  void sgl_joystick_callbacks(XMAP_JOYSTICK_CB int dummy);
#undef X
#define X(a, b, c) \
void sgl_##c##_callback(void(*a##_cb)b);
  XMAP_JOYSTICK_CB
#undef X
  
  joystick_t* sgl_joystick(int id);
  bool sgl_joystick_init(bool scan_too);
  bool sgl_joystick_scan(void);
  void sgl_joystick_release(void);
  bool sgl_joystick_remove(int id);
  void sgl_joystick_poll(void);
#endif
  
#if defined(SGL_ENABLE_DIALOGS)
  typedef enum {
    ALERT_INFO,
    ALERT_WARNING,
    ALERT_ERROR
  } ALERTLVL;
  
  typedef enum {
    ALERT_OK,
    ALERT_OK_CANCEL,
    ALERT_YES_NO
  } ALERTBTNS;
  
  typedef enum {
    DIALOG_OPEN,
    DIALOG_OPEN_DIR,
    DIALOG_SAVE
  } DIALOGACTION;

  bool sgl_alert(ALERTLVL lvl, ALERTBTNS btns, const char* fmt, ...);
  char* sgl_dialog(DIALOGACTION action, const char* path, const char* fname, bool allow_multiple, int nfilters, ...);
#endif

#if !defined(SGL_DISABLE_WINDOW)
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
  } MOUSEBTN;

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
  } KEYSYM;

#define KB_KEY_UNKNOWN -1
#define KB_KEY_LAST KB_KEY_MENU

  typedef enum {
    KB_MOD_SHIFT = 0x0001,
    KB_MOD_CONTROL = 0x0002,
    KB_MOD_ALT = 0x0004,
    KB_MOD_SUPER = 0x0008,
    KB_MOD_CAPS_LOCK = 0x0010,
    KB_MOD_NUM_LOCK = 0x0020
  } KEYMOD;
  
  typedef struct screen_t* screen_t;
  
#define XMAP_SCREEN_CB \
  X(keyboard, (void*, KEYSYM, KEYMOD, bool)) \
  X(mouse_button, (void*, MOUSEBTN, KEYMOD, bool)) \
  X(mouse_move, (void*, int, int, int, int)) \
  X(scroll, (void*, KEYMOD, float, float)) \
  X(focus, (void*, bool)) \
  X(resize, (void*, int, int))

#define X(a, b) \
void(*a##_cb)b,
  void sgl_screen_callbacks(XMAP_SCREEN_CB screen_t screen);
#undef X
#define X(a, b) \
void sgl_##a##_callback(screen_t screen, void(*a##_cb)b);
  XMAP_SCREEN_CB
#undef X

#define DEFAULT 0

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
  } CURSORTYPE;

#define SHOWN true
#define HIDDEN false
#define LOCKED true
#define UNLOCKED false

  typedef enum {
    RESIZABLE = 0x01,
    FULLSCREEN = 0x02,
    FULLSCREEN_DESKTOP = 0x04,
    BORDERLESS = 0x08,
    ALWAYS_ON_TOP = 0x10,
  } WINDOWFLAGS;

  bool sgl_screen(screen_t* s, const char* t, int w, int h, short flags);
  void sgl_screen_icon_buf(screen_t s, surface_t b);
  void sgl_screen_icon(screen_t s, const char* p);
  void sgl_screen_title(screen_t s, const char* t);
  void sgl_screen_destroy(screen_t* s);
  int  sgl_screen_id(screen_t s);
  void sgl_screen_size(screen_t s, int* w, int* h);
  bool sgl_closed(screen_t s);
  
  void sgl_cursor_lock(bool locked);
  void sgl_cursor_visible(bool show);
  void sgl_cursor_icon(screen_t s, CURSORTYPE t);
  void sgl_cursor_icon_custom(screen_t s, const char* p);
  void sgl_cursor_icon_custom_buf(screen_t s, surface_t b);
  void sgl_cursor_pos(point_t* p);
  void sgl_cursor_set_pos(point_t* p);
  
  void sgl_poll(void);
  void sgl_flush(screen_t s, surface_t b);
  void sgl_release(void);
#endif

#if defined(__cplusplus)
}
#endif
#endif /* graphics_h */
