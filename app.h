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
void fill(surface_t*, int, int , int);
bool pset(surface_t*, int, int, int, int, int);
int pget(surface_t*, int, int);
bool blit(surface_t*, point_t*, surface_t*, rect_t*);
bool yline(surface_t*, int, int, int, int, int, int);
bool xline(surface_t*, int, int, int, int, int, int);
bool line(surface_t*, int, int, int, int, int, int, int);
bool circle(surface_t*, int, int, int, int, int, int);
bool circle_filled(surface_t*, int, int, int, int, int, int);
bool rect(surface_t*, int, int, int, int, int, int, int);
bool rect_filled(surface_t*, int, int, int, int, int, int, int);
unsigned char* load_file_to_mem(const char*);
surface_t* load_bmp_from_mem(unsigned char*);
surface_t* load_bmp_from_file(const char*);
void letter(surface_t*, unsigned char, unsigned int, unsigned int, int, int, int);
void print(surface_t*, unsigned int, unsigned int, int, int, int, const char*);
void print_f(surface_t*, unsigned int, unsigned int, int, int, int, const char*, ...);
void init_default_font();

surface_t* screen(const char*, int, int);
bool redraw(void);
void release(void);

#ifdef __cplusplus
}
#endif
#endif /* app_h */
