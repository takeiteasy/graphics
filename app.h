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

typedef struct {
  unsigned int* buf, w, h;
} surface_t;
  
typedef struct {
  unsigned int r, g, b;
} rgb_t;

typedef struct {
  unsigned int x, y, w, h;
} rect_t;

typedef struct {
  unsigned int x, y;
} point_t;
  
surface_t* create_surface(unsigned int, unsigned int);
void free_surface(surface_t**);
void fill_surface(surface_t*, int, int , int);
bool pset(surface_t*, int, int, int, int, int);
int pget(surface_t*, int, int);
  
surface_t* app_open(const char*, int, int);
bool app_update(void);
void app_close(void);

#ifdef __cplusplus
}
#endif
#endif /* app_h */
