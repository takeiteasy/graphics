//
//  graphics_bdf.h
//  graphics_osx_test
//
//  Created by Rory B. Bellows on 03/07/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#ifndef graphics_bdf_h
#define graphics_bdf_h

#include "graphics.h"

typedef struct {
  unsigned int width;
  unsigned char* bitmap;
  rect_t bb;
} bdf_char_t;

typedef struct {
  rect_t fontbb;
  unsigned int* encoding_table;
  bdf_char_t* chars;
  int n_chars;
} bdf_t;

void bdf_destroy(bdf_t* f);
int bdf(bdf_t* out, const char* path);
int bdf_character(surface_t* s, bdf_t* f, const char* ch, int x, int y, int fg, int bg);
void bdf_writeln(surface_t* s, bdf_t* f, int x, int y, int fg, int bg, const char* str);
const char* get_last_bdf_error(void);

#endif /* graphics_bdf_h */
