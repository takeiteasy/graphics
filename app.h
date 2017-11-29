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
  
#define Map_RGB(r, g, b) (((unsigned int)r) << 16) | (((unsigned int)g) << 8) | b

typedef struct {
  unsigned int* buf;
  unsigned int w, h;
} surface_t;
  
surface_t* create_surface(unsigned int, unsigned int);
void free_surface(surface_t**);
void fill_surface(surface_t*, int, int , int);
unsigned int pset(surface_t*, int, int, int, int, int);
int pget(surface_t*, int, int);
  
surface_t* app_open(const char*, int, int);
int app_update(void);
void app_close(void);

#ifdef __cplusplus
}
#endif
#endif /* app_h */
