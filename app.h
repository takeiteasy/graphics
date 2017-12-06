//
//  app.h
//  app
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
#include <errno.h>
#include <math.h>

#define RGB(r, g, b) (((unsigned int)r) << 16) | (((unsigned int)g) << 8) | b
  
#define RED RGB(255, 0, 0)
#define BLUE RGB(0, 0, 255)
#define LIME RGB(0, 255, 0)
#define BLACK RGB(0, 0, 0)
#define WHITE RGB(255, 255, 255)
#define YELLOW RGB(255, 255, 0)
#define CYAN RGB(0, 255, 255)
#define MAGENTA RGB(255, 0, 255)
#define SILVER RGB(192,192,192)
#define GRAY RGB(128,128,128)
#define MAROON RGB(128,0,0)
#define OLIVE RGB(128,128,0)
#define GREEN RGB(0,128,0)
#define PURPLE RGB(128,0,128)
#define TEAL RGB(0,128,128)
#define NAVY RGB(0,0,128)

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
void init_default_font(void);

surface_t* screen(const char* title, int w, int h);
bool redraw(void);
void release(void);

#ifdef __cplusplus
}
#endif
#endif /* app_h */
