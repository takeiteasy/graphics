//
//  graphics.h
//  graphics
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#ifndef app_h
#define app_h
#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

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

#if defined(GRAPHICS_EXTRA_COLORS)
#define DARKRED RGB(139,0,0)
#define BROWN RGB(165,42,42)
#define FIREBRICK RGB(178,34,34)
#define CRIMSON RGB(220,20,60)
#define TOMATO RGB(255,99,71)
#define CORAL RGB(255,127,80)
#define INDIANRED RGB(205,92,92)
#define LIGHTCORAL RGB(240,128,128)
#define DARKSALMON RGB(233,150,122)
#define SALMON RGB(250,128,114)
#define LIGHTSALMON RGB(255,160,122)
#define ORANGERED RGB(255,69,0)
#define DARKORANGE RGB(255,140,0)
#define ORANGE RGB(255,165,0)
#define GOLD RGB(255,215,0)
#define DARKGOLDENROD RGB(184,134,11)
#define GOLDENROD RGB(218,165,32)
#define PALEGOLDENROD RGB(238,232,170)
#define DARKKHAKI RGB(189,183,107)
#define KHAKI RGB(240,230,140)
#define YELLOWGREEN RGB(154,205,50)
#define DARKOLIVEGREEN RGB(85,107,47)
#define OLIVEDRAB RGB(107,142,35)
#define LAWNGREEN RGB(124,252,0)
#define CHARTREUSE RGB(127,255,0)
#define GREENYELLOW RGB(173,255,47)
#define DARKGREEN RGB(0,100,0)
#define FORESTGREEN RGB(34,139,34)
#define LIMEGREEN RGB(50,205,50)
#define LIGHTGREEN RGB(144,238,144)
#define PALEGREEN RGB(152,251,152)
#define DARKSEAGREEN RGB(143,188,143)
#define MEDIUMSPRINGGREEN RGB(0,250,154)
#define SPRINGGREEN RGB(0,255,127)
#define SEAGREEN RGB(46,139,87)
#define MEDIUMAQUAMARINE RGB(102,205,170)
#define MEDIUMSEAGREEN RGB(60,179,113)
#define LIGHTSEAGREEN RGB(32,178,170)
#define DARKSLATEGRAY RGB(47,79,79)
#define DARKCYAN RGB(0,139,139)
#define AQUA RGB(0,255,255)
#define LIGHTCYAN RGB(224,255,255)
#define DARKTURQUOISE RGB(0,206,209)
#define TURQUOISE RGB(64,224,208)
#define MEDIUMTURQUOISE RGB(72,209,204)
#define PALETURQUOISE RGB(175,238,238)
#define AQUAMARINE RGB(127,255,212)
#define POWDERBLUE RGB(176,224,230)
#define CADETBLUE RGB(95,158,160)
#define STEELBLUE RGB(70,130,180)
#define CORNFLOWERBLUE RGB(100,149,237)
#define DEEPSKYBLUE RGB(0,191,255)
#define DODGERBLUE RGB(30,144,255)
#define LIGHTBLUE RGB(173,216,230)
#define SKYBLUE RGB(135,206,235)
#define LIGHTSKYBLUE RGB(135,206,250)
#define MIDNIGHTBLUE RGB(25,25,112)
#define DARKBLUE RGB(0,0,139)
#define MEDIUMBLUE RGB(0,0,205)
#define ROYALBLUE RGB(65,105,225)
#define BLUEVIOLET RGB(138,43,226)
#define INDIGO RGB(75,0,130)
#define DARKSLATEBLUE RGB(72,61,139)
#define SLATEBLUE RGB(106,90,205)
#define MEDIUMSLATEBLUE RGB(123,104,238)
#define MEDIUMPURPLE RGB(147,112,219)
#define DARKMAGENTA RGB(139,0,139)
#define DARKVIOLET RGB(148,0,211)
#define DARKORCHID RGB(153,50,204)
#define MEDIUMORCHID RGB(186,85,211)
#define THISTLE RGB(216,191,216)
#define PLUM RGB(221,160,221)
#define VIOLET RGB(238,130,238)
#define ORCHID RGB(218,112,214)
#define MEDIUMVIOLETRED RGB(199,21,133)
#define PALEVIOLETRED RGB(219,112,147)
#define DEEPPINK RGB(255,20,147)
#define HOTPINK RGB(255,105,180)
#define LIGHTPINK RGB(255,182,193)
#define PINK RGB(255,192,203)
#define ANTIQUEWHITE RGB(250,235,215)
#define BEIGE RGB(245,245,220)
#define BISQUE RGB(255,228,196)
#define BLANCHEDALMOND RGB(255,235,205)
#define WHEAT RGB(245,222,179)
#define CORNSILK RGB(255,248,220)
#define LEMONCHIFFON RGB(255,250,205)
#define LIGHTGOLDENRODYELLOW RGB(250,250,210)
#define LIGHTYELLOW RGB(255,255,224)
#define SADDLEBROWN RGB(139,69,19)
#define SIENNA RGB(160,82,45)
#define CHOCOLATE RGB(210,105,30)
#define PERU RGB(205,133,63)
#define SANDYBROWN RGB(244,164,96)
#define BURLYWOOD RGB(222,184,135)
#define TAN RGB(210,180,140)
#define ROSYBROWN RGB(188,143,143)
#define MOCCASIN RGB(255,228,181)
#define NAVAJOWHITE RGB(255,222,173)
#define PEACHPUFF RGB(255,218,185)
#define MISTYROSE RGB(255,228,225)
#define LAVENDERBLUSH RGB(255,240,245)
#define LINEN RGB(250,240,230)
#define OLDLACE RGB(253,245,230)
#define PAPAYAWHIP RGB(255,239,213)
#define SEASHELL RGB(255,245,238)
#define MINTCREAM RGB(245,255,250)
#define SLATEGRAY RGB(112,128,144)
#define LIGHTSLATEGRAY RGB(119,136,153)
#define LIGHTSTEELBLUE RGB(176,196,222)
#define LAVENDER RGB(230,230,250)
#define FLORALWHITE RGB(255,250,240)
#define ALICEBLUE RGB(240,248,255)
#define GHOSTWHITE RGB(248,248,255)
#define HONEYDEW RGB(240,255,240)
#define IVORY RGB(255,255,240)
#define AZURE RGB(240,255,255)
#define SNOW RGB(255,250,250)
#define DIMGRAY RGB(105,105,105)
#define DARKGRAY RGB(169,169,169)
#define LIGHTGRAY RGB(211,211,211)
#define GAINSBORO RGB(220,220,220)
#define WHITESMOKE RGB(245,245,245)
#endif

typedef struct {
  unsigned int* buf, w, h;
} surface_t;

typedef struct {
  unsigned int x, y, w, h;
} rect_t;

typedef struct {
  unsigned int x, y;
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
bool circle(surface_t* s, int xc, int yc, int r, int col);
bool circle_filled(surface_t* s, int xc, int yc, int r, int col);
bool rect(surface_t* s, int x, int y, int w, int h, int col);
bool rect_filled(surface_t* s, int x, int y, int w, int h, int col);
unsigned char* load_file_to_mem(const char* path);
surface_t* load_bmp_from_mem(unsigned char* data);
surface_t* load_bmp_from_file(const char* path);
void letter(surface_t* s, unsigned char c, unsigned int x, unsigned int y, int col);
void print(surface_t* s, unsigned int x, unsigned int y, int col, const char* str);
void print_f(surface_t* s, unsigned int x, unsigned int y, int col, const char* fmt, ...);
surface_t* string(int col, const char* str);
surface_t* string_f(int col, const char* fmt, ...);
void init_default_font(void);

surface_t* screen(const char* title, int w, int h);
bool redraw(void);
void release(void);
const char* get_last_error(void);

#ifdef __cplusplus
}
#endif
#endif /* app_h */
