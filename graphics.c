 //  graphics.c
//  graphics
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "graphics.h"

#define XYSET(s, x, y, v) (s->buf[(y) * s->w + (x)] = (v))
#define XYSETSAFE(s, x, y, v) \
if ((x) >= 0 && (y) >= 0 && (x) <= s->w && (y) <= s->h) \
  s->buf[(y) * s->w + (x)] = (v);
#define XYGET(s, x, y) (s->buf[(y) * s->w + (x)])

static char last_error[1024];

const char* get_last_error() {
  return last_error;
}

static short int keycodes[256];
static short int scancodes[KB_KEY_LAST + 1];
static surface_t* buffer;

#define SET_LAST_ERROR(MSG, ...) \
memset(last_error, 0, 1024); \
sprintf(last_error, "[ERROR] from %s in %s at %d -- " MSG, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

static bool ticks_started = false;

#define LINE_HEIGHT 10

char font8x8_basic[128][8] = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0000 (nul)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0001
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0002
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0003
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0004
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0005
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0006
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0007
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0008
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0009
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000A
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000B
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000C
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000D
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000E
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000F
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0010
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0011
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0012
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0013
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0014
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0015
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0016
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0017
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0018
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0019
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001A
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001B
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001C
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001D
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001E
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001F
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0020 (space)
  { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
  { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
  { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
  { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
  { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
  { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
  { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
  { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
  { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
  { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
  { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
  { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
  { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
  { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
  { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
  { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
  { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
  { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
  { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
  { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
  { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
  { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
  { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
  { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
  { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (//)
  { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
  { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
  { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
  { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
  { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
  { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
  { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
  { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
  { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
  { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
  { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
  { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
  { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
  { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
  { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
  { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
  { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
  { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
  { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
  { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
  { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
  { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
  { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
  { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
  { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
  { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
  { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
  { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
  { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
  { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
  { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
  { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
  { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
  { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
  { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
  { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
  { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
  { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
  { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
  { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
  { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
  { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
  { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
  { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
  { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
  { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
  { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
  { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
  { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
  { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
  { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
  { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
  { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
  { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
  { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
  { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
  { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
  { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
  { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
  { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
  { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
  { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
  { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
};

#if defined(GRAPHICS_EXTRA_FONTS)
char font8x8_block[32][8] = {
  { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00},   // U+2580 (top half)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+2581 (box 1/8)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF},   // U+2582 (box 2/8)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF},   // U+2583 (box 3/8)
  { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2584 (bottom half)
  { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2585 (box 5/8)
  { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2586 (box 6/8)
  { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2587 (box 7/8)
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2588 (solid)
  { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F},   // U+2589 (box 7/8)
  { 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F},   // U+258A (box 6/8)
  { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F},   // U+258B (box 5/8)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F},   // U+258C (left half)
  { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07},   // U+258D (box 3/8)
  { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03},   // U+258E (box 2/8)
  { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},   // U+258F (box 1/8)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0},   // U+2590 (right half)
  { 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00},   // U+2591 (25% solid)
  { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA},   // U+2592 (50% solid)
  { 0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55},   // U+2593 (75% solid)
  { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+2594 (box 1/8)
  { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},   // U+2595 (box 1/8)
  { 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F},   // U+2596 (box bottom left)
  { 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0},   // U+2597 (box bottom right)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00},   // U+2598 (box top left)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF},   // U+2599 (boxes left and bottom)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0},   // U+259A (boxes top-left and bottom right)
  { 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F},   // U+259B (boxes top and left)
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0},   // U+259C (boxes top and right)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00},   // U+259D (box top right)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F},   // U+259E (boxes top right and bottom left)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF},   // U+259F (boxes right and bottom)
};

char font8x8_box[128][8] = {
  { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00},   // U+2500 (thin horizontal)
  { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00},   // U+2501 (thick horizontal)
  { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},   // U+2502 (thin vertical)
  { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18},   // U+2503 (thich vertical)
  { 0x00, 0x00, 0x00, 0x00, 0xBB, 0x00, 0x00, 0x00},   // U+2504 (thin horizontal dashed)
  { 0x00, 0x00, 0x00, 0xBB, 0xBB, 0x00, 0x00, 0x00},   // U+2505 (thick horizontal dashed)
  { 0x08, 0x00, 0x08, 0x08, 0x08, 0x00, 0x08, 0x08},   // U+2506 (thin vertical dashed)
  { 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18},   // U+2507 (thich vertical dashed)
  { 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00},   // U+2508 (thin horizontal dotted)
  { 0x00, 0x00, 0x00, 0x55, 0x55, 0x00, 0x00, 0x00},   // U+2509 (thick horizontal dotted)
  { 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08},   // U+250A (thin vertical dotted)
  { 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18},   // U+250B (thich vertical dotted)
  { 0x00, 0x00, 0x00, 0x00, 0xf8, 0x08, 0x08, 0x08},   // U+250C (down L, right L)
  { 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x08, 0x08, 0x08},   // U+250D (down L, right H)
  { 0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0x18, 0x18},   // U+250E (down H, right L)
  { 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x18, 0x18, 0x18},   // U+250F (down H, right H)
  { 0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08, 0x08},   // U+2510 (down L, left L)
  { 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x08, 0x08, 0x08},   // U+2511 (down L, left H)
  { 0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x18, 0x18},   // U+2512 (down H, left L)
  { 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x18, 0x18, 0x18},   // U+2513 (down H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x00, 0x00, 0x00},   // U+2514 (up L, right L)
  { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x00, 0x00, 0x00},   // U+2515 (up L, right H)
  { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x00, 0x00, 0x00},   // U+2516 (up H, right L)
  { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x00, 0x00, 0x00},   // U+2517 (up H, right H)
  { 0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00},   // U+2518 (up L, left L)
  { 0x08, 0x08, 0x08, 0x0f, 0x0f, 0x00, 0x00, 0x00},   // U+2519 (up L, left H)
  { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x00, 0x00, 0x00},   // U+251A (up H, left L)
  { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00, 0x00},   // U+251B (up H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0x08},   // U+251C (down L, right L, up L)
  { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x08, 0x08, 0x08},   // U+251D (down L, right H, up L)
  { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x08, 0x08, 0x08},   // U+251E (down L, right L, up H)
  { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x18, 0x18, 0x18},   // U+251F (down H, right L, up L)
  { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0x18, 0x18},   // U+2520 (down H, right L, up H)
  { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x08, 0x08, 0x08},   // U+2521 (down L, right H, up H)
  { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x18, 0x18, 0x18},   // U+2522 (down H, right H, up L)
  { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18},   // U+2523 (down H, right H, up H)
  { 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08},   // U+2524 (down L, left L, up L)
  { 0x08, 0x08, 0x08, 0x0f, 0x0f, 0x08, 0x08, 0x08},   // U+2525 (down L, left H, up L)
  { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x08, 0x08, 0x08},   // U+2526 (down L, left L, up H)
  { 0x08, 0x08, 0x08, 0x08, 0x1f, 0x18, 0x18, 0x18},   // U+2527 (down H, left L, up L)
  { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x18, 0x18},   // U+2528 (down H, left L, up H)
  { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x08, 0x08, 0x08},   // U+2529 (down L, left H, up H)
  { 0x08, 0x08, 0x08, 0x1f, 0x1f, 0x18, 0x18, 0x18},   // U+252A (down H, left H, up L)
  { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x18},   // U+252B (down H, left H, up H)
  { 0x00, 0x00, 0x00, 0x00, 0xff, 0x08, 0x08, 0x08},   // U+252C (down L, right L, left L)
  { 0x00, 0x00, 0x00, 0x0f, 0xff, 0x08, 0x08, 0x08},   // U+252D (down L, right L, left H)
  { 0x00, 0x00, 0x00, 0xf8, 0xff, 0x08, 0x08, 0x08},   // U+252E (down L, right H, left L)
  { 0x00, 0x00, 0x00, 0xff, 0xff, 0x08, 0x08, 0x08},   // U+252F (down L, right H, left H)
  { 0x00, 0x00, 0x00, 0x00, 0xff, 0x18, 0x18, 0x18},   // U+2530 (down H, right L, left L)
  { 0x00, 0x00, 0x00, 0x1f, 0xff, 0x18, 0x18, 0x18},   // U+2531 (down H, right L, left H)
  { 0x00, 0x00, 0x00, 0xf8, 0xff, 0x18, 0x18, 0x18},   // U+2532 (down H, right H, left L)
  { 0x00, 0x00, 0x00, 0xff, 0xff, 0x18, 0x18, 0x18},   // U+2533 (down H, right H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xff, 0x00, 0x00, 0x00},   // U+2534 (up L, right L, left L)
  { 0x08, 0x08, 0x08, 0x0f, 0xff, 0x00, 0x00, 0x00},   // U+2535 (up L, right L, left H)
  { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x00, 0x00, 0x00},   // U+2536 (up L, right H, left L)
  { 0x08, 0x08, 0x08, 0xff, 0xff, 0x00, 0x00, 0x00},   // U+2537 (up L, right H, left H)
  { 0x18, 0x18, 0x18, 0x18, 0xff, 0x00, 0x00, 0x00},   // U+2538 (up H, right L, left L)
  { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x00, 0x00, 0x00},   // U+2539 (up H, right L, left H)
  { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x00, 0x00, 0x00},   // U+253A (up H, right H, left L)
  { 0x18, 0x18, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00},   // U+253B (up H, right H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08},   // U+253C (up L, right L, left L, down L)
  { 0x08, 0x08, 0x08, 0x0f, 0xff, 0x08, 0x08, 0x08},   // U+253D (up L, right L, left H, down L)
  { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x08, 0x08, 0x08},   // U+253E (up L, right H, left L, down L)
  { 0x08, 0x08, 0x08, 0xff, 0xff, 0x08, 0x08, 0x08},   // U+253F (up L, right H, left H, down L)
  { 0x18, 0x18, 0x18, 0x18, 0xff, 0x08, 0x08, 0x08},   // U+2540 (up H, right L, left L, down L)
  { 0x08, 0x08, 0x08, 0x08, 0xff, 0x18, 0x18, 0x18},   // U+2541 (up L, right L, left L, down H)
  { 0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18},   // U+2542 (up H, right L, left L, down H)
  { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x08, 0x08, 0x08},   // U+2543 (up H, right L, left H, down L)
  { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x08, 0x08, 0x08},   // U+2544 (up H, right H, left L, down L)
  { 0x08, 0x08, 0x08, 0x1f, 0xff, 0x18, 0x18, 0x18},   // U+2545 (up L, right L, left H, down H)
  { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x18, 0x18, 0x18},   // U+2546 (up L, right H, left L, down H)
  { 0x08, 0x08, 0x08, 0xff, 0xff, 0x18, 0x18, 0x18},   // U+2547 (up L, right H, left H, down H)
  { 0x18, 0x18, 0x18, 0xff, 0xff, 0x08, 0x08, 0x08},   // U+254B (up H, right H, left H, down L)
  { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x18, 0x18, 0x18},   // U+254A (up H, right H, left L, down H)
  { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x18, 0x18, 0x18},   // U+2549 (up H, right L, left H, down H)
  { 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18},   // U+254B (up H, right H, left H, down H)
  { 0x00, 0x00, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00},   // U+254C (thin horizontal broken)
  { 0x00, 0x00, 0x00, 0xE7, 0xE7, 0x00, 0x00, 0x00},   // U+254D (thick horizontal broken)
  { 0x08, 0x08, 0x08, 0x00, 0x00, 0x08, 0x08, 0x08},   // U+254E (thin vertical broken)
  { 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18},   // U+254F (thich vertical broken)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00},   // U+2550 (double horizontal)
  { 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14},   // U+2551 (double vertical)
  { 0x00, 0x00, 0x00, 0xF8, 0x08, 0xF8, 0x08, 0x08},   // U+2552 (down L, right D)
  { 0x00, 0x00, 0x00, 0x00, 0xFC, 0x14, 0x14, 0x14},   // U+2553 (down D, right L)
  { 0x00, 0x00, 0x00, 0xFC, 0x04, 0xF4, 0x14, 0x14},   // U+2554 (down D, right D)
  { 0x00, 0x00, 0x00, 0x0F, 0x08, 0x0F, 0x08, 0x08},   // U+2555 (down L, left D)
  { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x14, 0x14, 0x14},   // U+2556 (down D, left L)
  { 0x00, 0x00, 0x00, 0x1F, 0x10, 0x17, 0x14, 0x14},   // U+2557 (down D, left D)
  { 0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x00, 0x00},   // U+2558 (up L, right D)
  { 0x14, 0x14, 0x14, 0x14, 0xFC, 0x00, 0x00, 0x00},   // U+2559 (up D, right L)
  { 0x14, 0x14, 0x14, 0xF4, 0x04, 0xFC, 0x00, 0x00},   // U+255A (up D, right D)
  { 0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x00, 0x00},   // U+255B (up L, left D)
  { 0x14, 0x14, 0x14, 0x14, 0x1F, 0x00, 0x00, 0x00},   // U+255C (up D, left L)
  { 0x14, 0x14, 0x14, 0x17, 0x10, 0x1F, 0x00, 0x00},   // U+255D (up D, left D)
  { 0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x08, 0x08},   // U+255E (up L, down L, right D)
  { 0x14, 0x14, 0x14, 0x14, 0xF4, 0x14, 0x14, 0x14},   // U+255F (up D, down D, right L)
  { 0x14, 0x14, 0x14, 0xF4, 0x04, 0xF4, 0x14, 0x14},   // U+2560 (up D, down D, right D)
  { 0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x08, 0x08},   // U+2561 (up L, down L, left D)
  { 0x14, 0x14, 0x14, 0x14, 0x17, 0x14, 0x14, 0x14},   // U+2562 (up D, down D, left L)
  { 0x14, 0x14, 0x14, 0x17, 0x10, 0x17, 0x14, 0x14},   // U+2563 (up D, down D, left D)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x08, 0x08},   // U+2564 (left D, right D, down L)
  { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x14, 0x14, 0x14},   // U+2565 (left L, right L, down D)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xF7, 0x14, 0x14},   // U+2566 (left D, right D, down D)
  { 0x08, 0x08, 0x08, 0xFF, 0x00, 0xFF, 0x00, 0x00},   // U+2567 (left D, right D, up L)
  { 0x14, 0x14, 0x14, 0x14, 0xFF, 0x00, 0x00, 0x00},   // U+2568 (left L, right L, up D)
  { 0x14, 0x14, 0x14, 0xF7, 0x00, 0xFF, 0x00, 0x00},   // U+2569 (left D, right D, up D)
  { 0x08, 0x08, 0x08, 0xFF, 0x08, 0xFF, 0x08, 0x08},   // U+256A (left D, right D, down L, up L)
  { 0x14, 0x14, 0x14, 0x14, 0xFF, 0x14, 0x14, 0x14},   // U+256B (left L, right L, down D, up D)
  { 0x14, 0x14, 0x14, 0xF7, 0x00, 0xF7, 0x14, 0x14},   // U+256C (left D, right D, down D, up D)
  { 0x00, 0x00, 0x00, 0x00, 0xE0, 0x10, 0x08, 0x08},   // U+256D (curve down-right)
  { 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x08},   // U+256E (curve down-left)
  { 0x08, 0x08, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00},   // U+256F (curve up-left)
  { 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x00},   // U+2570 (curve up-right)
  { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01},   // U+2571 (diagonal bottom-left to top-right)
  { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80},   // U+2572 (diagonal bottom-right to top-left)
  { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81},   // U+2573 (diagonal cross)
  { 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00},   // U+2574 (left L)
  { 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00},   // U+2575 (up L)
  { 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00},   // U+2576 (right L)
  { 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08},   // U+2577 (down L)
  { 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00},   // U+2578 (left H)
  { 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00},   // U+2579 (up H)
  { 0x00, 0x00, 0x00, 0xF8, 0xF8, 0x00, 0x00, 0x00},   // U+257A (right H)
  { 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18},   // U+257B (down H)
  { 0x00, 0x00, 0x00, 0xF8, 0xFF, 0x00, 0x00, 0x00},   // U+257C (right H, left L)
  { 0x08, 0x08, 0x08, 0x08, 0x18, 0x18, 0x18, 0x18},   // U+257D (up L, down H)
  { 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00},   // U+257E (right L, left H)
  { 0x18, 0x18, 0x18, 0x18, 0x08, 0x08, 0x08, 0x08}    // U+257F (up H, down L)
};

char font8x8_greek[58][8] = {
  { 0x2D, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0390 (iota with tonos and diaeresis)
  { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0391 (Alpha)
  { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0392 (Beta)
  { 0x3F, 0x33, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00},   // U+0393 (Gamma)
  { 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x63, 0x7F, 0x00},   // U+0394 (Delta)
  { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0395 (Epsilon)
  { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+0396 (Zeta)
  { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0397 (Eta)
  { 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x36, 0x1C, 0x00},   // U+0398 (Theta)
  { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0399 (Iota)
  { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+039A (Kappa)
  { 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x63, 0x63, 0x00},   // U+039B (Lambda)
  { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+039C (Mu)
  { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+039D (Nu)
  { 0x7F, 0x63, 0x00, 0x3E, 0x00, 0x63, 0x7F, 0x00},   // U+039E (Xi)
  { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+039F (Omikron)
  { 0x7F, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00},   // U+03A0 (Pi)
  { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+03A1 (Rho)
  { 0x00, 0x01, 0x02, 0x04, 0x4F, 0x90, 0xA0, 0x40},   // U+03A2
  { 0x7F, 0x63, 0x06, 0x0C, 0x06, 0x63, 0x7F, 0x00},   // U+03A3 (Sigma 2)
  { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+03A4 (Tau)
  { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+03A5 (Upsilon)
  { 0x18, 0x7E, 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x00},   // U+03A6 (Phi)
  { 0x63, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x63, 0x00},   // U+03A7 (Chi)
  { 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x18, 0x3C, 0x00},   // U+03A8 (Psi)
  { 0x3E, 0x63, 0x63, 0x63, 0x36, 0x36, 0x77, 0x00},   // U+03A9 (Omega)
  { 0x33, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0399 (Iota with diaeresis)
  { 0x33, 0x00, 0x33, 0x33, 0x1E, 0x0C, 0x1E, 0x00},   // U+03A5 (Upsilon with diaeresis)
  { 0x70, 0x00, 0x6E, 0x3B, 0x13, 0x3B, 0x6E, 0x00},   // U+03AC (alpha aigu)
  { 0x38, 0x00, 0x1E, 0x03, 0x0E, 0x03, 0x1E, 0x00},   // U+03AD (epsilon aigu)
  { 0x38, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x30},   // U+03AE (eta aigu)
  { 0x38, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+03AF (iota aigu)
  { 0x2D, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+03B0 (upsilon with tonos and diaeresis)
  { 0x00, 0x00, 0x6E, 0x3B, 0x13, 0x3B, 0x6E, 0x00},   // U+03B1 (alpha)
  { 0x00, 0x1E, 0x33, 0x1F, 0x33, 0x1F, 0x03, 0x03},   // U+03B2 (beta)
  { 0x00, 0x00, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x00},   // U+03B3 (gamma)
  { 0x38, 0x0C, 0x18, 0x3E, 0x33, 0x33, 0x1E, 0x00},   // U+03B4 (delta)
  { 0x00, 0x00, 0x1E, 0x03, 0x0E, 0x03, 0x1E, 0x00},   // U+03B5 (epsilon)
  { 0x00, 0x3F, 0x06, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+03B6 (zeta)
  { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x30},   // U+03B7 (eta)
  { 0x00, 0x00, 0x1E, 0x33, 0x3F, 0x33, 0x1E, 0x00},   // U+03B8 (theta)
  { 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+03B9 (iota)
  { 0x00, 0x00, 0x33, 0x1B, 0x0F, 0x1B, 0x33, 0x00},   // U+03BA (kappa)
  { 0x00, 0x03, 0x06, 0x0C, 0x1C, 0x36, 0x63, 0x00},   // U+03BB (lambda)
  { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x03},   // U+03BC (mu)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+03BD (nu)
  { 0x1E, 0x03, 0x0E, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+03BE (xi)
  { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+03BF (omikron)
  { 0x00, 0x00, 0x7F, 0x36, 0x36, 0x36, 0x36, 0x00},   // U+03C0 (pi)
  { 0x00, 0x00, 0x3C, 0x66, 0x66, 0x36, 0x06, 0x06},   // U+03C1 (rho)
  { 0x00, 0x00, 0x3E, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+03C2 (sigma 1)
  { 0x00, 0x00, 0x7E, 0x1B, 0x1B, 0x1B, 0x0E, 0x00},   // U+03C3 (sigma 2)
  { 0x00, 0x00, 0x7E, 0x18, 0x18, 0x58, 0x30, 0x00},   // U+03C4 (tau)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+03C5 (upsilon)
  { 0x00, 0x00, 0x76, 0xDB, 0xDB, 0x7E, 0x18, 0x00},   // U+03C6 (phi)
  { 0x00, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+03C7 (chi)
  { 0x00, 0x00, 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x00},   // U+03C8 (psi)
  { 0x00, 0x00, 0x36, 0x63, 0x6B, 0x7F, 0x36, 0x00}    // U+03C9 (omega)
};

char font8x8_hiragana[96][8] = {
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+3040
  { 0x04, 0x3F, 0x04, 0x3C, 0x56, 0x4D, 0x26, 0x00},   // U+3041 (Hiragana a)
  { 0x04, 0x3F, 0x04, 0x3C, 0x56, 0x4D, 0x26, 0x00},   // U+3042 (Hiragana A)
  { 0x00, 0x00, 0x00, 0x11, 0x21, 0x25, 0x02, 0x00},   // U+3043 (Hiragana i)
  { 0x00, 0x01, 0x11, 0x21, 0x21, 0x25, 0x02, 0x00},   // U+3044 (Hiragana I)
  { 0x00, 0x1C, 0x00, 0x1C, 0x22, 0x20, 0x18, 0x00},   // U+3045 (Hiragana u)
  { 0x3C, 0x00, 0x3C, 0x42, 0x40, 0x20, 0x18, 0x00},   // U+3046 (Hiragana U)
  { 0x1C, 0x00, 0x3E, 0x10, 0x38, 0x24, 0x62, 0x00},   // U+3047 (Hiragana e)
  { 0x1C, 0x00, 0x3E, 0x10, 0x38, 0x24, 0x62, 0x00},   // U+3048 (Hiragana E)
  { 0x24, 0x4F, 0x04, 0x3C, 0x46, 0x45, 0x22, 0x00},   // U+3049 (Hiragana o)
  { 0x24, 0x4F, 0x04, 0x3C, 0x46, 0x45, 0x22, 0x00},   // U+304A (Hiragana O)
  { 0x04, 0x24, 0x4F, 0x54, 0x52, 0x12, 0x09, 0x00},   // U+304B (Hiragana KA)
  { 0x44, 0x24, 0x0F, 0x54, 0x52, 0x52, 0x09, 0x00},   // U+304C (Hiragana GA)
  { 0x08, 0x1F, 0x08, 0x3F, 0x1C, 0x02, 0x3C, 0x00},   // U+304D (Hiragana KI)
  { 0x44, 0x2F, 0x04, 0x1F, 0x0E, 0x01, 0x1E, 0x00},   // U+304E (Hiragana GI)
  { 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00},   // U+304F (Hiragana KU)
  { 0x28, 0x44, 0x12, 0x21, 0x02, 0x04, 0x08, 0x00},   // U+3050 (Hiragana GU)
  { 0x00, 0x22, 0x79, 0x21, 0x21, 0x22, 0x10, 0x00},   // U+3051 (Hiragana KE)
  { 0x40, 0x22, 0x11, 0x3D, 0x11, 0x12, 0x08, 0x00},   // U+3052 (Hiragana GE)
  { 0x00, 0x00, 0x3C, 0x00, 0x02, 0x02, 0x3C, 0x00},   // U+3053 (Hiragana KO)
  { 0x20, 0x40, 0x16, 0x20, 0x01, 0x01, 0x0E, 0x00},   // U+3054 (Hiragana GO)
  { 0x10, 0x7E, 0x10, 0x3C, 0x02, 0x02, 0x1C, 0x00},   // U+3055 (Hiragana SA)
  { 0x24, 0x4F, 0x14, 0x2E, 0x01, 0x01, 0x0E, 0x00},   // U+3056 (Hiragana ZA)
  { 0x00, 0x02, 0x02, 0x02, 0x42, 0x22, 0x1C, 0x00},   // U+3057 (Hiragana SI)
  { 0x20, 0x42, 0x12, 0x22, 0x02, 0x22, 0x1C, 0x00},   // U+3058 (Hiragana ZI)
  { 0x10, 0x7E, 0x18, 0x14, 0x18, 0x10, 0x0C, 0x00},   // U+3059 (Hiragana SU)
  { 0x44, 0x2F, 0x06, 0x05, 0x06, 0x04, 0x03, 0x00},   // U+305A (Hiragana ZU)
  { 0x20, 0x72, 0x2F, 0x22, 0x1A, 0x02, 0x1C, 0x00},   // U+305B (Hiragana SE)
  { 0x80, 0x50, 0x3A, 0x17, 0x1A, 0x02, 0x1C, 0x00},   // U+305C (Hiragana ZE)
  { 0x1E, 0x08, 0x04, 0x7F, 0x08, 0x04, 0x38, 0x00},   // U+305D (Hiragana SO)
  { 0x4F, 0x24, 0x02, 0x7F, 0x08, 0x04, 0x38, 0x00},   // U+305E (Hiragana ZO)
  { 0x02, 0x0F, 0x02, 0x72, 0x02, 0x09, 0x71, 0x00},   // U+305F (Hiragana TA)
  { 0x42, 0x2F, 0x02, 0x72, 0x02, 0x09, 0x71, 0x00},   // U+3060 (Hiragana DA)
  { 0x08, 0x7E, 0x08, 0x3C, 0x40, 0x40, 0x38, 0x00},   // U+3061 (Hiragana TI)
  { 0x44, 0x2F, 0x04, 0x1E, 0x20, 0x20, 0x1C, 0x00},   // U+3062 (Hiragana DI)
  { 0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x1C, 0x00},   // U+3063 (Hiragana tu)
  { 0x00, 0x1C, 0x22, 0x41, 0x40, 0x20, 0x1C, 0x00},   // U+3064 (Hiragana TU)
  { 0x40, 0x20, 0x1E, 0x21, 0x20, 0x20, 0x1C, 0x00},   // U+3065 (Hiragana DU)
  { 0x00, 0x3E, 0x08, 0x04, 0x04, 0x04, 0x38, 0x00},   // U+3066 (Hiragana TE)
  { 0x00, 0x3E, 0x48, 0x24, 0x04, 0x04, 0x38, 0x00},   // U+3067 (Hiragana DE)
  { 0x04, 0x04, 0x08, 0x3C, 0x02, 0x02, 0x3C, 0x00},   // U+3068 (Hiragana TO)
  { 0x44, 0x24, 0x08, 0x3C, 0x02, 0x02, 0x3C, 0x00},   // U+3069 (Hiragana DO)
  { 0x32, 0x02, 0x27, 0x22, 0x72, 0x29, 0x11, 0x00},   // U+306A (Hiragana NA)
  { 0x00, 0x02, 0x7A, 0x02, 0x0A, 0x72, 0x02, 0x00},   // U+306B (Hiragana NI)
  { 0x08, 0x09, 0x3E, 0x4B, 0x65, 0x55, 0x22, 0x00},   // U+306C (Hiragana NU)
  { 0x04, 0x07, 0x34, 0x4C, 0x66, 0x54, 0x24, 0x00},   // U+306D (Hiragana NE)
  { 0x00, 0x00, 0x3C, 0x4A, 0x49, 0x45, 0x22, 0x00},   // U+306E (Hiragana NO)
  { 0x00, 0x22, 0x7A, 0x22, 0x72, 0x2A, 0x12, 0x00},   // U+306F (Hiragana HA)
  { 0x80, 0x51, 0x1D, 0x11, 0x39, 0x15, 0x09, 0x00},   // U+3070 (Hiragana BA)
  { 0x40, 0xB1, 0x5D, 0x11, 0x39, 0x15, 0x09, 0x00},   // U+3071 (Hiragana PA)
  { 0x00, 0x00, 0x13, 0x32, 0x51, 0x11, 0x0E, 0x00},   // U+3072 (Hiragana HI)
  { 0x40, 0x20, 0x03, 0x32, 0x51, 0x11, 0x0E, 0x00},   // U+3073 (Hiragana BI)
  { 0x40, 0xA0, 0x43, 0x32, 0x51, 0x11, 0x0E, 0x00},   // U+3074 (Hiragana PI)
  { 0x1C, 0x00, 0x08, 0x2A, 0x49, 0x10, 0x0C, 0x00},   // U+3075 (Hiragana HU)
  { 0x4C, 0x20, 0x08, 0x2A, 0x49, 0x10, 0x0C, 0x00},   // U+3076 (Hiragana BU)
  { 0x4C, 0xA0, 0x48, 0x0A, 0x29, 0x48, 0x0C, 0x00},   // U+3077 (Hiragana PU)
  { 0x00, 0x00, 0x04, 0x0A, 0x11, 0x20, 0x40, 0x00},   // U+3078 (Hiragana HE)
  { 0x20, 0x40, 0x14, 0x2A, 0x11, 0x20, 0x40, 0x00},   // U+3079 (Hiragana BE)
  { 0x20, 0x50, 0x24, 0x0A, 0x11, 0x20, 0x40, 0x00},   // U+307A (Hiragana PE)
  { 0x7D, 0x11, 0x7D, 0x11, 0x39, 0x55, 0x09, 0x00},   // U+307B (Hiragana HO)
  { 0x9D, 0x51, 0x1D, 0x11, 0x39, 0x55, 0x09, 0x00},   // U+307C (Hiragana BO)
  { 0x5D, 0xB1, 0x5D, 0x11, 0x39, 0x55, 0x09, 0x00},   // U+307D (Hiragana PO)
  { 0x7E, 0x08, 0x3E, 0x08, 0x1C, 0x2A, 0x04, 0x00},   // U+307E (Hiragana MA)
  { 0x00, 0x07, 0x24, 0x24, 0x7E, 0x25, 0x12, 0x00},   // U+307F (Hiragana MI)
  { 0x04, 0x0F, 0x64, 0x06, 0x05, 0x26, 0x3C, 0x00},   // U+3080 (Hiragana MU)
  { 0x00, 0x09, 0x3D, 0x4A, 0x4B, 0x45, 0x2A, 0x00},   // U+3081 (Hiragana ME)
  { 0x02, 0x0F, 0x02, 0x0F, 0x62, 0x42, 0x3C, 0x00},   // U+3082 (Hiragana MO)
  { 0x00, 0x00, 0x12, 0x1F, 0x22, 0x12, 0x04, 0x00},   // U+3083 (Hiragana ya)
  { 0x00, 0x12, 0x3F, 0x42, 0x42, 0x34, 0x04, 0x00},   // U+3084 (Hiragana YA)
  { 0x00, 0x00, 0x11, 0x3D, 0x53, 0x39, 0x11, 0x00},   // U+3085 (Hiragana yu)
  { 0x00, 0x11, 0x3D, 0x53, 0x51, 0x39, 0x11, 0x00},   // U+3086 (Hiragana YU)
  { 0x00, 0x08, 0x38, 0x08, 0x1C, 0x2A, 0x04, 0x00},   // U+3087 (Hiragana yo)
  { 0x08, 0x08, 0x38, 0x08, 0x1C, 0x2A, 0x04, 0x00},   // U+3088 (Hiragana YO)
  { 0x1E, 0x00, 0x02, 0x3A, 0x46, 0x42, 0x30, 0x00},   // U+3089 (Hiragana RA)
  { 0x00, 0x20, 0x22, 0x22, 0x2A, 0x24, 0x10, 0x00},   // U+308A (Hiragana RI)
  { 0x1F, 0x08, 0x3C, 0x42, 0x49, 0x54, 0x38, 0x00},   // U+308B (Hiragana RU)
  { 0x04, 0x07, 0x04, 0x0C, 0x16, 0x55, 0x24, 0x00},   // U+308C (Hiragana RE)
  { 0x3F, 0x10, 0x08, 0x3C, 0x42, 0x41, 0x30, 0x00},   // U+308D (Hiragana RO)
  { 0x00, 0x00, 0x08, 0x0E, 0x38, 0x4C, 0x2A, 0x00},   // U+308E (Hiragana wa)
  { 0x04, 0x07, 0x04, 0x3C, 0x46, 0x45, 0x24, 0x00},   // U+308F (Hiragana WA)
  { 0x0E, 0x08, 0x3C, 0x4A, 0x69, 0x55, 0x32, 0x00},   // U+3090 (Hiragana WI)
  { 0x06, 0x3C, 0x42, 0x39, 0x04, 0x36, 0x49, 0x00},   // U+3091 (Hiragana WE)
  { 0x04, 0x0F, 0x04, 0x6E, 0x11, 0x08, 0x70, 0x00},   // U+3092 (Hiragana WO)
  { 0x08, 0x08, 0x04, 0x0C, 0x56, 0x52, 0x21, 0x00},   // U+3093 (Hiragana N)
  { 0x40, 0x2E, 0x00, 0x3C, 0x42, 0x40, 0x38, 0x00},   // U+3094 (Hiragana VU)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+3095
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+3096
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+3097
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+3098
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+3099 (voiced combinator mark)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+309A (semivoiced combinator mark)
  { 0x40, 0x80, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00},   // U+309B (Hiragana voiced mark)
  { 0x40, 0xA0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+309C (Hiragana semivoiced mark)
  { 0x00, 0x00, 0x08, 0x08, 0x10, 0x30, 0x0C, 0x00},   // U+309D (Hiragana iteration mark)
  { 0x20, 0x40, 0x14, 0x24, 0x08, 0x18, 0x06, 0x00},   // U+309E (Hiragana voiced iteration mark)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+309F
};

char font8x8_extra[132][8] = {
  { 0x1F, 0x33, 0x33, 0x5F, 0x63, 0xF3, 0x63, 0xE3},   // U+20A7 (Spanish Pesetas/Pt)
  { 0x70, 0xD8, 0x18, 0x3C, 0x18, 0x18, 0x1B, 0x0E},   // U+0192 (dutch florijn)
  { 0x3C, 0x36, 0x36, 0x7C, 0x00, 0x7E, 0x00, 0x00},   // U+ (underlined superscript a)
  { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x3E, 0x00, 0x00},   // U+ (underlined superscript 0)
  { 0x00, 0x00, 0x00, 0x3F, 0x03, 0x03, 0x00, 0x00},   // U+2310 (gun pointing right)
  { 0x30, 0x18, 0x0C, 0x18, 0x30, 0x00, 0x7E, 0x00},   // U+ (less than or equal)
  { 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x00, 0x7E, 0x00},   // U+ (greater than or equal)
  { 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+ (grave)
  { 0x0E, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x00},   // U+ (Y grave)
  { 0x00, 0x07, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+ (y grave)
  { 0x00, 0x00, 0x38, 0x66, 0x06, 0x06, 0x07, 0x00},   // U+E541 (SGA A)
  { 0x00, 0x00, 0x0C, 0x0C, 0x18, 0x30, 0x7F, 0x00},   // U+E542 (SGA B)
  { 0x00, 0x00, 0x0C, 0x00, 0x0C, 0x30, 0x30, 0x00},   // U+E543 (SGA C)
  { 0x00, 0x00, 0x7F, 0x00, 0x03, 0x1C, 0x60, 0x00},   // U+E544 (SGA D)
  { 0x00, 0x00, 0x63, 0x03, 0x03, 0x03, 0x7F, 0x00},   // U+E545 (SGA E)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xDB, 0x00, 0x00},   // U+E546 (SGA F)
  { 0x00, 0x00, 0x30, 0x30, 0x3E, 0x30, 0x30, 0x00},   // U+E547 (SGA G)
  { 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x18, 0x18, 0x00},   // U+E548 (SGA H)
  { 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00},   // U+E549 (SGA I)
  { 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00},   // U+E54A (SGA J)
  { 0x00, 0x00, 0x18, 0x18, 0x5A, 0x18, 0x18, 0x00},   // U+E54B (SGA K)
  { 0x00, 0x00, 0x03, 0x33, 0x03, 0x33, 0x03, 0x00},   // U+E54C (SGA L)
  { 0x00, 0x00, 0x63, 0x60, 0x60, 0x60, 0x7F, 0x00},   // U+E54D (SGA M)
  { 0x00, 0x00, 0x66, 0x60, 0x30, 0x18, 0x0C, 0x00},   // U+E54E (SGA N)
  { 0x00, 0x00, 0x3C, 0x60, 0x30, 0x18, 0x0C, 0x00},   // U+E54F (SGA O)
  { 0x00, 0x00, 0x66, 0x60, 0x66, 0x06, 0x66, 0x00},   // U+E550 (SGA P)
  { 0x00, 0x00, 0x18, 0x00, 0x7E, 0x60, 0x7E, 0x00},   // U+E551 (SGA Q)
  { 0x00, 0x00, 0x00, 0x66, 0x00, 0x66, 0x00, 0x00},   // U+E552 (SGA R)
  { 0x00, 0x00, 0x0C, 0x0C, 0x3C, 0x30, 0x30, 0x00},   // U+E553 (SGA S)
  { 0x00, 0x00, 0x3C, 0x30, 0x30, 0x00, 0x30, 0x00},   // U+E554 (SGA T)
  { 0x00, 0x00, 0x00, 0x36, 0x00, 0x7F, 0x00, 0x00},   // U+E555 (SGA U)
  { 0x00, 0x00, 0x18, 0x18, 0x7E, 0x00, 0x7E, 0x00},   // U+E556 (SGA V)
  { 0x00, 0x00, 0x00, 0x18, 0x00, 0x66, 0x00, 0x00},   // U+E557 (SGA W)
  { 0x00, 0x00, 0x66, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+E558 (SGA X)
  { 0x00, 0x00, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00},   // U+E559 (SGA Y)
  { 0x00, 0x00, 0x18, 0x3C, 0x66, 0x66, 0x66, 0x00},   // U+E55A (SGA Z)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00A0 (no break space)
  { 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00},   // U+00A1 (inverted !)
  { 0x18, 0x18, 0x7E, 0x03, 0x03, 0x7E, 0x18, 0x18},   // U+00A2 (dollarcents)
  { 0x1C, 0x36, 0x26, 0x0F, 0x06, 0x67, 0x3F, 0x00},   // U+00A3 (pound sterling)
  { 0x00, 0x00, 0x63, 0x3E, 0x36, 0x3E, 0x63, 0x00},   // U+00A4 (currency mark)
  { 0x33, 0x33, 0x1E, 0x3F, 0x0C, 0x3F, 0x0C, 0x0C},   // U+00A5 (yen)
  { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+00A6 (broken pipe)
  { 0x7C, 0xC6, 0x1C, 0x36, 0x36, 0x1C, 0x33, 0x1E},   // U+00A7 (paragraph)
  { 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00A8 (diaeresis)
  { 0x3C, 0x42, 0x99, 0x85, 0x85, 0x99, 0x42, 0x3C},   // U+00A9 (copyright symbol)
  { 0x3C, 0x36, 0x36, 0x7C, 0x00, 0x00, 0x00, 0x00},   // U+00AA (superscript a)
  { 0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00},   // U+00AB (<<)
  { 0x00, 0x00, 0x00, 0x3F, 0x30, 0x30, 0x00, 0x00},   // U+00AC (gun pointing left)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00AD (soft hyphen)
  { 0x3C, 0x42, 0x9D, 0xA5, 0x9D, 0xA5, 0x42, 0x3C},   // U+00AE (registered symbol)
  { 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00AF (macron)
  { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00},   // U+00B0 (degree)
  { 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x7E, 0x00},   // U+00B1 (plusminus)
  { 0x1C, 0x30, 0x18, 0x0C, 0x3C, 0x00, 0x00, 0x00},   // U+00B2 (superscript 2)
  { 0x1C, 0x30, 0x18, 0x30, 0x1C, 0x00, 0x00, 0x00},   // U+00B2 (superscript 3)
  { 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+00B2 (aigu)
  { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x03},   // U+00B5 (mu)
  { 0xFE, 0xDB, 0xDB, 0xDE, 0xD8, 0xD8, 0xD8, 0x00},   // U+00B6 (pilcrow)
  { 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00},   // U+00B7 (central dot)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x1E},   // U+00B8 (cedille)
  { 0x08, 0x0C, 0x08, 0x1C, 0x00, 0x00, 0x00, 0x00},   // U+00B9 (superscript 1)
  { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00},   // U+00BA (superscript 0)
  { 0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00},   // U+00BB (>>)
  { 0xC3, 0x63, 0x33, 0xBD, 0xEC, 0xF6, 0xF3, 0x03},   // U+00BC (1/4)
  { 0xC3, 0x63, 0x33, 0x7B, 0xCC, 0x66, 0x33, 0xF0},   // U+00BD (1/2)
  { 0x03, 0xC4, 0x63, 0xB4, 0xDB, 0xAC, 0xE6, 0x80},   // U+00BE (3/4)
  { 0x0C, 0x00, 0x0C, 0x06, 0x03, 0x33, 0x1E, 0x00},   // U+00BF (inverted ?)
  { 0x07, 0x00, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x00},   // U+00C0 (A grave)
  { 0x70, 0x00, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x00},   // U+00C1 (A aigu)
  { 0x1C, 0x36, 0x00, 0x3E, 0x63, 0x7F, 0x63, 0x00},   // U+00C2 (A circumflex)
  { 0x6E, 0x3B, 0x00, 0x3E, 0x63, 0x7F, 0x63, 0x00},   // U+00C3 (A ~)
  { 0x63, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x63, 0x00},   // U+00C4 (A umlaut)
  { 0x0C, 0x0C, 0x00, 0x1E, 0x33, 0x3F, 0x33, 0x00},   // U+00C5 (A ring)
  { 0x7C, 0x36, 0x33, 0x7F, 0x33, 0x33, 0x73, 0x00},   // U+00C6 (AE)
  { 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x18, 0x30, 0x1E},   // U+00C7 (C cedille)
  { 0x07, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00C8 (E grave)
  { 0x38, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00C9 (E aigu)
  { 0x0C, 0x12, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00CA (E circumflex)
  { 0x36, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00},   // U+00CB (E umlaut)
  { 0x07, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CC (I grave)
  { 0x38, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CD (I aigu)
  { 0x0C, 0x12, 0x00, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CE (I circumflex)
  { 0x33, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00CF (I umlaut)
  { 0x3F, 0x66, 0x6F, 0x6F, 0x66, 0x66, 0x3F, 0x00},   // U+00D0 (Eth)
  { 0x3F, 0x00, 0x33, 0x37, 0x3F, 0x3B, 0x33, 0x00},   // U+00D1 (N ~)
  { 0x0E, 0x00, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00},   // U+00D2 (O grave)
  { 0x70, 0x00, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00},   // U+00D3 (O aigu)
  { 0x3C, 0x66, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00},   // U+00D4 (O circumflex)
  { 0x6E, 0x3B, 0x00, 0x3E, 0x63, 0x63, 0x3E, 0x00},   // U+00D5 (O ~)
  { 0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00},   // U+00D6 (O umlaut)
  { 0x00, 0x36, 0x1C, 0x08, 0x1C, 0x36, 0x00, 0x00},   // U+00D7 (multiplicative x)
  { 0x5C, 0x36, 0x73, 0x7B, 0x6F, 0x36, 0x1D, 0x00},   // U+00D8 (O stroke)
  { 0x0E, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},   // U+00D9 (U grave)
  { 0x70, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},   // U+00DA (U aigu)
  { 0x3C, 0x66, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x00},   // U+00DB (U circumflex)
  { 0x33, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+00DC (U umlaut)
  { 0x70, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x00},   // U+00DD (Y aigu)
  { 0x0F, 0x06, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+00DE (Thorn)
  { 0x00, 0x1E, 0x33, 0x1F, 0x33, 0x1F, 0x03, 0x03},   // U+00DF (beta)
  { 0x07, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E0 (a grave)
  { 0x38, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E1 (a aigu)
  { 0x7E, 0xC3, 0x3C, 0x60, 0x7C, 0x66, 0xFC, 0x00},   // U+00E2 (a circumflex)
  { 0x6E, 0x3B, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E3 (a ~)
  { 0x33, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E4 (a umlaut)
  { 0x0C, 0x0C, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00},   // U+00E5 (a ring)
  { 0x00, 0x00, 0xFE, 0x30, 0xFE, 0x33, 0xFE, 0x00},   // U+00E6 (ae)
  { 0x00, 0x00, 0x1E, 0x03, 0x03, 0x1E, 0x30, 0x1C},   // U+00E7 (c cedille)
  { 0x07, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},   // U+00E8 (e grave)
  { 0x38, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},   // U+00E9 (e aigu)
  { 0x7E, 0xC3, 0x3C, 0x66, 0x7E, 0x06, 0x3C, 0x00},   // U+00EA (e circumflex)
  { 0x33, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00},   // U+00EB (e umlaut)
  { 0x07, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00EC (i grave)
  { 0x1C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00ED (i augu)
  { 0x3E, 0x63, 0x1C, 0x18, 0x18, 0x18, 0x3C, 0x00},   // U+00EE (i circumflex)
  { 0x33, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+00EF (i umlaut)
  { 0x1B, 0x0E, 0x1B, 0x30, 0x3E, 0x33, 0x1E, 0x00},   // U+00F0 (eth)
  { 0x00, 0x1F, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x00},   // U+00F1 (n ~)
  { 0x00, 0x07, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F2 (o grave)
  { 0x00, 0x38, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F3 (o aigu)
  { 0x1E, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F4 (o circumflex)
  { 0x6E, 0x3B, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F5 (o ~)
  { 0x00, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+00F6 (o umlaut)
  { 0x18, 0x18, 0x00, 0x7E, 0x00, 0x18, 0x18, 0x00},   // U+00F7 (division)
  { 0x00, 0x60, 0x3C, 0x76, 0x7E, 0x6E, 0x3C, 0x06},   // U+00F8 (o stroke)
  { 0x00, 0x07, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00F9 (u grave)
  { 0x00, 0x38, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00FA (u aigu)
  { 0x1E, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00FB (u circumflex)
  { 0x00, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00},   // U+00FC (u umlaut)
  { 0x00, 0x38, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+00FD (y aigu)
  { 0x00, 0x00, 0x06, 0x3E, 0x66, 0x3E, 0x06, 0x00},   // U+00FE (thorn)
  { 0x00, 0x33, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F}    // U+00FF (y umlaut)
};
#endif

static int mx = 0, my = 0;

void get_mouse_pos(int* x, int* y) {
  if (!x || !y)
    return;
  *x = mx;
  *y = my;
}

surface_t* surface(unsigned int w, unsigned int h) {
  surface_t* ret = malloc(sizeof(surface_t));
  if (!ret) {
    SET_LAST_ERROR("malloc() failed");
    return NULL;
  }

  ret->w = w;
  ret->h = h;
  size_t s = w * h * sizeof(unsigned int) + 1;
  ret->buf = malloc(s);
  if (!ret->buf) {
    SET_LAST_ERROR("malloc() failed");
    return NULL;
  }
  memset(ret->buf, 0, s);

  return ret;
}

void destroy(surface_t** s) {
  if (*s) {
    free((*s)->buf);
    (*s)->buf = NULL;
    free(*s);
    *s = NULL;
  }
}

void fill(surface_t* s, int col) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      XYSET(s, x, y, col);
}

bool pset(surface_t* s, int x, int y, int col) {
  if (x > s->w || y > s->h || x < 0 || y < 0) {
    SET_LAST_ERROR("pset() failed! x/y outside of bounds");
    return false;
  }

  XYSET(s, x, y, col);
  return true;
}

int pget(surface_t* s, int x, int y) {
  if (x > s->w || y > s->h || x < 0 || y < 0) {
    SET_LAST_ERROR("pget() failed! x/y outside of bounds");
    return 0;
  }

  return XYGET(s, x, y);
}

bool blit(surface_t* dst, point_t* p, surface_t* src, rect_t* r, int chroma) {
  int offset_x = 0, offset_y = 0,
  from_x = 0, from_y = 0,
  width = src->w, height = src->h;
  if (p) {
    offset_x = p->x;
    offset_y = p->y;
  }
  if (r) {
    from_x = r->x;
    from_y = r->y;
    width  = r->w;
    height = r->h;
  }

  if (offset_x < 0) {
    from_x += abs(offset_x);
    width -= abs(offset_x);
    offset_x = 0;
  }
  if (offset_y < 0) {
    from_y += abs(offset_y);
    height -= abs(offset_y);
    offset_y = 0;
  }

  int to_x = offset_x + width, to_y = offset_y + height;
  if (to_x > dst->w)
    width += (dst->w - to_x);
  if (to_y > dst->h)
    height += (dst->h - to_y);

  if (offset_x > dst->w || offset_y > dst->h || to_x < 0 || to_y < 0)
    return false;

  int x, y, c;
  for (x = 0; x < width; ++x) {
    for (y = 0; y < height; ++y) {
      c = XYGET(src, from_x + x, from_y + y);
      if (c != chroma)
        XYSET(dst, offset_x + x, offset_y + y, c);
    }
  }
  return true;
}

bool yline(surface_t* s, int x, int y1, int y2, int col) {
  if (x < 0 || x >= s->w)
    return false;

  if (y2 < y1) {
    y1 += y2;
    y2  = y1 - y2;
    y1 -= y2;
  }

  if (y1 < 0)
    y1 = 0;
  if (y2 >= s->h)
    y2 = s->h - 1;

  for(int y = y1; y <= y2; y++)
    XYSET(s, x, y, col);
  return true;
}

bool xline(surface_t* s, int y, int x1, int x2, int col) {
  if (y < 0 || y >= s->h)
    return false;

  if (x2 < x1) {
    x1 += x2;
    x2  = x1 - x2;
    x1 -= x2;
  }

  if (x1 < 0)
    x1 = 0;
  if (x2 >= s->w)
    x2 = s->w - 1;

  for(int x = x1; x <= x2; x++)
    XYSET(s, x, y, col);
  return true;
}

bool line(surface_t* s, int x1, int y1, int x2, int y2, int col) {
  if (x1 == x2)
    return yline(s, x1, y1, y2, col);
  if (y1 == y2)
    return xline(s, y1, x1, x2, col);

  int xi1, xi2, yi1, yi2, d, n, na, np, p;
  if (x2 > x1) {
    if (x2 > s->w)
      x2 = s->w;
    if (x1 < 0)
      x1 = 0;
    xi1 = 1;
    xi2 = 1;
  } else {
    if (x1 > s->w)
      x1 = s->w;
    if (x2 < 0)
      x2 = 0;
    xi1 = -1;
    xi2 = -1;
  }

  if(y2 > y1) {
    if (y2 > s->h)
      y2 = s->h;
    if (y1 < 0)
      y1 = 0;
    yi1 = 1;
    yi2 = 1;
  } else  {
    if (y1 > s->h)
      y1 = s->h;
    if (y2 < 0)
      y2 = 0;
    yi1 = -1;
    yi2 = -1;
  }

  int x = x1, y = y1, dx = abs(x2 - x1), dy = abs(y2 - y1);
  if (dx > dy) {
    xi1 = 0;
    yi2 = 0;
    d = dx;
    n = dx / 2;
    na = dy;
    np = dx;
  } else {
    xi2 = 0;
    yi1 = 0;
    d = dy;
    n = dy / 2;
    na = dx;
    np = dy;
  }

  for (p = 0; p <= np; ++p) {
    XYSET(s, x % s->w, y % s->h, col);
    n += na;
    if (n >= d) {
      n -= d;
      x += xi1;
      y += yi1;
    }
    x += xi2;
    y += yi2;
  }
  return true;
}

bool circle(surface_t* s, int xc, int yc, int r, int col, bool fill) {
  if (xc + r < 0 || yc + r < 0 || xc - r > s->w || yc - r > s->h)
    return false;

  int x = 0, y = r, p = 3 - (r << 1);
  int pb = yc + r + 1, pd = yc + r + 1;
  int a, b, c, d, e, f, g, h;

  while (x <= y) {
    a = xc + x;
    b = yc + y;
    c = xc - x;
    d = yc - y;
    e = xc + y;
    f = yc + x;
    g = xc - y;
    h = yc - x;

    if (fill) {
      if (b != pb)
        xline(s, b, a, c, col);
      if (d != pd)
        xline(s, d, a, c, col);
      if (f != b)
        xline(s, f, e, g, col);
      if (h != d && h != f)
        xline(s, h, e, g, col);
    } else {
      XYSETSAFE(s, a, b, col);
      XYSETSAFE(s, c, d, col);
      XYSETSAFE(s, e, f, col);
      XYSETSAFE(s, g, f, col);

      if (x > 0) {
        XYSETSAFE(s, a, d, col);
        XYSETSAFE(s, c, b, col);
        XYSETSAFE(s, e, h, col);
        XYSETSAFE(s, g, h, col);
      }
    }

    pb = b;
    pd = d;
    p += (p < 0 ? (x++ << 2) + 6 : ((x++ - y--) << 2) + 10);
  }
  return true;
}

bool rect(surface_t* s, int x, int y, int w, int h, int col, bool fill) {
  if (x < 0) {
    w += x;
    x  = 0;
  }
  if (y < 0) {
    h += y;
    y  = 0;
  }

  w += x;
  h += y;
  if (w < 0 || h < 0 || x > s->w || y > s->h)
    return false;

  if (w > s->w)
    w = s->w;
  if (h > s->h)
    h = s->h;

  if (fill) {
    for (; y < h; ++y)
      xline(s, y, x, w, col);
  } else {
    xline(s, y, x, w, col);
    xline(s, h, x, w, col);
    yline(s, x, y, h, col);
    yline(s, w, y, h, col);
  }
  return true;
}

#if defined(_MSC_VER)
#define ALIGN_STRUCT(x) __declspec(align(x))
#else
#define ALIGN_STRUCT(x) __attribute__((aligned(x)))
#endif

#pragma pack(1)
typedef struct {
  unsigned short type; /* Magic identifier */
  unsigned int size; /* File size in bytes */
  unsigned int reserved;
  unsigned int offset; /* Offset to image data, bytes */
} ALIGN_STRUCT(2) BMPHEADER;
#pragma pack()

typedef struct {
  unsigned int size; /* Header size in bytes */
  int width, height; /* Width and height of image */
  unsigned short planes; /* Number of colour planes */
  unsigned short bits; /* Bits per pixel */
  unsigned int compression; /* Compression type */
  unsigned int image_size; /* Image size in bytes */
  int xresolution, yresolution; /* Pixels per meter */
  unsigned int ncolours; /* Number of colours */
  unsigned int important_colours; /* Important colours */
} BMPINFOHEADER;

typedef struct {
  unsigned int size; /* size of bitmap core header */
  unsigned short width; /* image with */
  unsigned short height; /* image height */
  unsigned short planes; /* must be equal to 1 */
  unsigned short count; /* bits per pixel */
} BMPCOREHEADER;

#define BMP_GET(d, b, s) \
memcpy(d, b + off, s); \
off += s;

#define BMP_SET(c) (ret->buf[(i - (i % info.width)) + (info.width - (i % info.width) - 1)] = (c));

surface_t* bmp_mem(unsigned char* data) {
  int off = 0;
  BMPHEADER header;
  BMPINFOHEADER info;
  BMP_GET(&header, data, sizeof(BMPHEADER));
  BMP_GET(&info, data, sizeof(BMPINFOHEADER));

  if (header.type != 0x4D42) {
    SET_LAST_ERROR("loadbmp() failed: invalid BMP signiture '%d'", header.type);
    return NULL;
  }

  unsigned char* color_map = NULL;
  int color_map_size = 0;
  if (info.bits <= 8) {
    color_map_size = (1 << info.bits) * 4;
    color_map = (unsigned char*)malloc (color_map_size * sizeof(unsigned char));
    if (!color_map) {
      SET_LAST_ERROR("malloc() failed");
      return NULL;
    }
    BMP_GET(color_map, data, color_map_size);
  }

  surface_t* ret = surface(info.width, info.height);
  if (!ret) {
    if (color_map)
      free(color_map);
    SET_LAST_ERROR("malloc() failed");
    return NULL;
  }

  off = header.offset;
  int i, j, c, s = info.width * info.height;
  unsigned char color;
  switch (info.compression) {
    case 0: // RGB
      switch (info.bits) { // BPP
        case 1:
          for (i = (s - 1); i != -1; ++off) {
            for (j = 7; j >= 0; --j, --i) {
              c = color_map[((data[off] & (1 << j)) > 0) * 4 + 1];
              BMP_SET(RGB(c, c, c));
            }
          }
          break;
        case 4:
          for (i = (s - 1); i != -1; --i, ++off) {
            color = (data[off] >> 4) * 4;
            BMP_SET(RGB(color_map[color + 2], color_map[color + 1], color_map[color]));
            i--;
            color = (data[off] & 0x0F);
            BMP_SET(RGB(color_map[color + 2], color_map[color + 1], color_map[color]));
          }
          break;
        case 8:
          for (i = (s - 1); i != -1; --i, ++off) {
            color = (data[off] * 4);
            BMP_SET(RGB(color_map[color + 2], color_map[color + 1], color_map[color]));
          }
          break;
        case 24:
        case 32:
          for (i = (s - 1); i != -1; --i, off += (info.bits == 32 ? 4 : 3))
            BMP_SET(RGB(data[off], data[off + 1], data[off + 2]));
          break;
        default:
          SET_LAST_ERROR("load_bmp_from_mem() failed. Unsupported BPP: %d", info.bits);
          destroy(&ret);
          break;
      }
      break;
    case 1: // RLE8
    case 2: // RLE4
    default:
      SET_LAST_ERROR("load_bmp_from_mem() failed. Unsupported compression: %d", info.compression);
      destroy(&ret);
      break;
  }

  if (color_map)
    free(color_map);

  return ret;
}

surface_t* bmp_fp(FILE* fp) {
  if (!fp) {
    SET_LAST_ERROR("bmp_fp() failed: file pointer null\n");
    return NULL;
  }

  fseek(fp, 0, SEEK_END);
  size_t length = ftell(fp);
  rewind(fp);

  unsigned char* data = (unsigned char*)calloc(length + 1, sizeof(unsigned char));
  fread(data, 1, length, fp);

  surface_t* ret = bmp_mem(data);
  free(data);
  return ret;
}

surface_t* bmp(const char* path) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    SET_LAST_ERROR("fopen() failed: %s\n", path);
    return NULL;
  }

  surface_t* ret = bmp_fp(fp);
  fclose(fp);
  return ret;
}

#define PRINT_LETTER(map, max_r, ch) \
int i, j; \
if (ch >= max_r || ch < 0) \
  ch = 0; \
for (i = 0; i < 8; ++i) \
  for (j = 0; j < 8; ++j) \
    if (map[ch][i] & 1 << j) \
      XYSETSAFE(s, x + j, y + i, col);

void letter(surface_t* s, unsigned char ch, unsigned int x, unsigned int y, int col) {
  int c = (int)ch;
  PRINT_LETTER(font8x8_basic, 128, c);
}

#if defined(GRAPHICS_EXTRA_FONTS)
void letter_block(surface_t* s, int ch, unsigned int x, unsigned int y, int col) {
  PRINT_LETTER(font8x8_block, 32, ch);
}

void letter_box(surface_t* s, int ch, unsigned int x, unsigned int y, int col) {
  PRINT_LETTER(font8x8_box, 128, ch);
}

void letter_extra(surface_t* s, int ch, unsigned int x, unsigned int y, int col) {
  PRINT_LETTER(font8x8_extra, 132, ch);
}

void letter_greek(surface_t* s, int ch, unsigned int x, unsigned int y, int col) {
  PRINT_LETTER(font8x8_greek, 58, ch);
}

void letter_hiragana(surface_t* s, int ch, unsigned int x, unsigned int y, int col) {
  PRINT_LETTER(font8x8_hiragana, 96, ch);
}
#endif

void print(surface_t* s, unsigned int x, unsigned int y, int col, const char* str) {
  int u = x, v = y;
  char* c = (char*)str;
  while (c != NULL && *c != '\0') {
    if (*c == '\n') {
      v += LINE_HEIGHT;
      u  = x;
    } else {
      letter(s, *c, u, v, col);
      u += 8;
    }
    ++c;
  }
}

void print_f(surface_t* s, unsigned int x, unsigned int y, int col, const char* fmt, ...) {
  char *buffer = NULL;
  size_t buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  int length = vsnprintf(buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = realloc(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf(buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  print(s, x, y, col, buffer);
  free(buffer);
}

surface_t* string(int col, int bg, const char* str) {
  int w = 0, x = 0, h = 8;
  char* c = (char*)str;
  while (c != NULL && *c != '\0') {
    if (*c == '\n') {
      h += LINE_HEIGHT;
      if (x > w)
        w = x;
      x = 0;
    } else
      x += 8;
    ++c;
  }
  if (x > w)
    w = x;

  surface_t* ret = surface(w, h);
  fill(ret, bg);
  print(ret, 0, 0, col, str);
  return ret;
}

surface_t* string_f(int col, int bg, const char* fmt, ...) {
  char *buffer = NULL;
  size_t buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  int length = vsnprintf(buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = realloc(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf(buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  surface_t* ret = string(col, bg, buffer);
  free(buffer);
  return ret;
}

void rgb(int c, int* r, int* g, int* b) {
  *r = (c >> 16) & 0xFF;
  *g = (c >> 8) & 0xFF;
  *b =  c & 0xFF;
}

surface_t* copy(surface_t* s) {
  surface_t* ret = surface(s->w, s->h);
  memcpy(ret->buf, s->buf, s->w * s->h * sizeof(unsigned int) + 1);
  return ret;
}

void iterate(surface_t* s, int (*fn)(int x, int y, int col)) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      XYSET(s, x, y, fn(x, y, XYGET(s, x, y)));
}

long ticks() {
#if defined(_WIN32)
  static LARGE_INTEGER ticks_start;
  if (!ticks_started) {
    QueryPerformanceCounter(&ticks_start);
    ticks_started = true;
  }

  LARGE_INTEGER ticks_now, freq;
  QueryPerformanceCounter(&ticks_now);
  QueryPerformanceFrequency(&freq);

  return ((ticks_now.QuadPart - ticks_start.QuadPart) * 1000) / freq.QuadPart;
#else
  static struct timespec ticks_start;
  if (!ticks_started) {
    clock_gettime(CLOCK_MONOTONIC, &ticks_start);
    ticks_started = true;
  }

  struct timespec ticks_now;
  clock_gettime(CLOCK_MONOTONIC, &ticks_now);
  return ((ticks_now.tv_sec * 1000) + (ticks_now.tv_nsec / 1000000)) - ((ticks_start.tv_sec * 1000) + (ticks_start.tv_nsec / 1000000));
#endif
}

void delay(long ms) {
#if defined(_WIN32)
  Sleep((DWORD)ms);
#else
  usleep((unsigned int)(ms * 1000));
#endif
}

#if defined(GRAPHICS_OPENGL_BACKEND)
#if defined(__APPLE__)
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl3.h>
#else
#error Not implemented yet
#endif

void print_shader_log(GLuint s) {
  if (glIsShader(s)) {
    int log_len = 0, max_len = 0;
    glGetShaderiv(s, GL_INFO_LOG_LENGTH, &max_len);
    char* log = malloc(sizeof(char) * max_len);
    
    glGetShaderInfoLog(s, max_len, &log_len, log);
    if (log_len >  0)
      printf("ERROR!   %s\n", log);
    
    free(log);
  }
}

GLuint load_shader(const GLchar* src, GLenum type) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src, NULL);
  glCompileShader(s);
  
  GLint res = GL_FALSE;
  glGetShaderiv(s, GL_COMPILE_STATUS, &res);
  if (!res) {
    print_shader_log(s);
    return 0;
  }
  
  return s;
}

GLuint create_shader(const GLchar* vs_src, const GLchar* fs_src) {
  GLuint sp = glCreateProgram();
  GLuint vs = load_shader(vs_src, GL_VERTEX_SHADER);
  GLuint fs = load_shader(fs_src, GL_FRAGMENT_SHADER);
  glAttachShader(sp, vs);
  glAttachShader(sp, fs);
  glLinkProgram(sp);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return sp;
}
#endif

#if defined(__APPLE__)
#import <Cocoa/Cocoa.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < 101200
#define NSWindowStyleMaskBorderless NSBorderlessWindowMask
#define NSWindowStyleMaskClosable NSClosableWindowMask
#define NSWindowStyleMaskMiniaturizable NSMiniaturizableWindowMask
#define NSWindowStyleMaskResizable NSResizableWindowMask
#define NSWindowStyleMaskTitled NSTitledWindowMask
#define NSEventModifierFlagCommand NSCommandKeyMask
#define NSEventModifierFlagControl NSControlKeyMask
#define NSEventModifierFlagOption NSAlternateKeyMask
#define NSEventModifierFlagShift NSShiftKeyMask
#define NSEventModifierFlagDeviceIndependentFlagsMask NSDeviceIndependentModifierFlagsMask
#define NSEventMaskAny NSAnyEventMask
#define NSEventTypeApplicationDefined NSApplicationDefined
#define NSEventTypeKeyUp NSKeyUp
#endif

static int translate_mod(NSUInteger flags) {
  int mods = 0;

  if (flags & NSEventModifierFlagShift)
    mods |= KB_MOD_SHIFT;
  if (flags & NSEventModifierFlagControl)
    mods |= KB_MOD_CONTROL;
  if (flags & NSEventModifierFlagOption)
    mods |= KB_MOD_ALT;
  if (flags & NSEventModifierFlagCommand)
    mods |= KB_MOD_SUPER;
  if (flags & NSEventModifierFlagCapsLock)
    mods |= KB_MOD_CAPS_LOCK;

  return mods;
}

static int translate_key(unsigned int key) {
  return (key >= sizeof(keycodes) / sizeof(keycodes[0]) ?  KEYBOARD_KEY_DOWN : keycodes[key]);
}

@interface osx_app_t : NSWindow {
  NSView* view;
  @public bool closed;
}
@end

#if defined(GRAPHICS_OPENGL_BACKEND)
@interface osx_view_t : NSOpenGLView {
  NSTrackingArea* track;
  GLuint vao, shader, texture;
}
#else
@interface osx_view_t : NSView {
  NSTrackingArea* track;
}
#endif
@end

@interface AppDelegate : NSApplication {}
@end

@implementation AppDelegate
-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication {
  (void)theApplication;
  return YES;
}

-(void)sendEvent:(NSEvent *)event {
  if ([event type] == NSEventTypeKeyUp && ([event modifierFlags] & NSEventModifierFlagCommand))
    [[self keyWindow] sendEvent:event];
  else
    [super sendEvent:event];
}
@end

static osx_app_t* app;

@implementation osx_view_t
extern surface_t* buffer;

-(id)initWithFrame:(CGRect)r {
#if defined(GRAPHICS_OPENGL_BACKEND)
  NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFAColorSize, 24,
    NSOpenGLPFAAlphaSize, 8,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFANoRecovery,
    0
  };
  NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
  self = [super initWithFrame:r pixelFormat:pixelFormat];
  
  if (self != nil) {
    track = nil;
    [self updateTrackingAreas];
    [[self openGLContext] makeCurrentContext];
    
    glViewport(0, 0, r.size.width, r.size.height);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    
    GLfloat vertices_position[8] = {
      -1.0, -1.0,
      1.0, -1.0,
      1.0,  1.0,
      -1.0,  1.0,
    };
    
    GLfloat texture_coord[8] = {
      0.0, 0.0,
      1.0, 0.0,
      1.0, 1.0,
      0.0, 1.0,
    };
    
    GLuint indices[6] = {
      0, 1, 2,
      2, 3, 0
    };
    
    const char* vs_src =
    "#version 150\n"
    "in vec4 position;"
    "in vec2 texture_coord;"
    "out vec2 texture_coord_from_vshader;"
    "void main() {"
    "  gl_Position = position;"
    "  texture_coord_from_vshader = texture_coord;"
    "}";
    
    const char* fs_src =
    "#version 150\n"
    "in vec2 texture_coord_from_vshader;"
    "out vec4 out_color;"
    "uniform sampler2D texture_sampler;"
    "void main() {"
    "  out_color = texture(texture_sampler, texture_coord_from_vshader);"
    "}";
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position) + sizeof(texture_coord), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_position), vertices_position);\
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices_position), sizeof(texture_coord), texture_coord);
    
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    shader = create_shader(vs_src, fs_src);
    glUseProgram(shader);
    
    GLint position_attribute = glGetAttribLocation(shader, "position");
    glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(position_attribute);
    
    GLint texture_coord_attribute = glGetAttribLocation(shader, "texture_coord");
    glVertexAttribPointer(texture_coord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vertices_position));
    glEnableVertexAttribArray(texture_coord_attribute);
    
    glGenTextures(1, &texture);
  }
#else
  self = [super initWithFrame:r];
  if (self != nil) {
    track = nil;
    [self updateTrackingAreas];
  }
#endif
  return self;
}

-(void)updateTrackingAreas {
  if (track != nil) {
    [self removeTrackingArea:track];
    [track release];
  }

  track = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                       options:NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingEnabledDuringMouseDrag | NSTrackingCursorUpdate | NSTrackingInVisibleRect | NSTrackingAssumeInside | NSTrackingMouseMoved
                                         owner:self
                                      userInfo:nil];

  [self addTrackingArea:track];
  [super updateTrackingAreas];
}

-(BOOL)acceptsFirstResponder {
  return YES;
}

-(BOOL)performKeyEquivalent:(NSEvent*)event {
  return YES;
}

-(void)mouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}

-(void)mouseMoved:(NSEvent*)event {
  mx = (int)floor([event locationInWindow].x - 1);
  my = (int)floor(buffer->h - 1 - [event locationInWindow].y);
}

-(void)rightMouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}

-(void)otherMouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}

-(NSRect)resizeRect {
  NSRect v = [[self window] contentRectForFrameRect:[[self window] frame]];
  return NSMakeRect(NSMaxX(v) + 5.5, NSMinY(v) - 16.0 - 5.5, 16.0, 16.0);
}

-(void)drawRect:(NSRect)r {
  (void)r;

  if (!buffer)
    return;

#if defined(GRAPHICS_OPENGL_BACKEND)
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer->w, buffer->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, (GLvoid*)buffer->buf);
  
  [super drawRect:r];
  glClear(GL_COLOR_BUFFER_BIT);
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  [[self openGLContext] flushBuffer];
#else
  CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];

  CGColorSpaceRef s = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef p = CGDataProviderCreateWithData(NULL, buffer->buf, buffer->w * buffer->h * 4, NULL);
  CGImageRef img = CGImageCreate(buffer->w, buffer->h, 8, 32, buffer->w * 4, s, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, p, NULL, false, kCGRenderingIntentDefault);

  CGColorSpaceRelease(s);
  CGDataProviderRelease(p);

  CGContextDrawImage(ctx, CGRectMake(0, 0, buffer->w, buffer->h), img);

  CGImageRelease(img);
#endif
}

-(void)dealloc {
#if defined(GRAPHICS_OPENGL_BACKEND)
  glDeleteTextures(1, &texture);
  glDeleteProgram(shader);
  glDeleteVertexArrays(1, &vao);
#endif
  [track release];
  [super dealloc];
}
@end

@implementation osx_app_t
-(id)initWithContentRect:(NSRect)r styleMask:(NSWindowStyleMask)s backing:(NSBackingStoreType)t defer:(BOOL)d {
  self = [super initWithContentRect:r
                          styleMask:s
                            backing:t
                              defer:d];
  if (self) {
    [self setOpaque:YES];
    [self setBackgroundColor:[NSColor clearColor]];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_changed:)
                                                 name:NSWindowDidBecomeMainNotification
                                               object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_changed:)
                                                 name:NSWindowDidResignMainNotification
                                               object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_close)
                                                 name:NSWindowWillCloseNotification
                                               object:self];

    closed = false;
  }
  return self;
}

-(void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
  [super dealloc];
}

-(void)setContentSize:(NSSize)s {
  NSSize sizeDelta = s;
  NSSize childBoundsSize = [view bounds].size;
  sizeDelta.width -= childBoundsSize.width;
  sizeDelta.height -= childBoundsSize.height;

  osx_view_t* fv = [super contentView];
  NSSize ns  = [fv bounds].size;
  ns.width  += sizeDelta.width;
  ns.height += sizeDelta.height;

  [super setContentSize:ns];
}

-(void)setContentView:(NSView *)v {
  if ([view isEqualTo:v])
    return;

  NSRect b = [self frame];
  b.origin = NSZeroPoint;
  osx_view_t* fv = [super contentView];
  if (!fv) {
    fv = [[[osx_view_t alloc] initWithFrame:b] autorelease];
    [super setContentView:fv];
    [super makeFirstResponder:fv];
  }

  if (view)
    [view removeFromSuperview];

  view = v;
  [view setFrame:[self contentRectForFrameRect:b]];
  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [fv addSubview:view];
}

-(void)win_changed:(NSNotification *)n {
  (void)n;
}

-(void)win_close {
  closed = true;
}

-(NSView*)contentView {
  return view;
}

-(BOOL)canBecomeKeyWindow {
  return YES;
}

-(BOOL)canBecomeMainWindow {
  return YES;
}

-(NSRect)contentRectForFrameRect:(NSRect)f {
  f.origin = NSZeroPoint;
  return NSInsetRect(f, 0, 0);
}

+(NSRect)frameRectForContentRect:(NSRect)r styleMask:(NSWindowStyleMask)s {
  (void)s;
  return NSInsetRect(r, 0, 0);
}
@end

surface_t* screen(const char* t, int w, int h) {
  if (!(buffer = surface(w, h)))
    return NULL;
  buffer->w = w;
  buffer->h = h;

  memset(keycodes,  -1, sizeof(keycodes));
  memset(scancodes, -1, sizeof(scancodes));

  keycodes[0x1D] = KB_KEY_0;
  keycodes[0x12] = KB_KEY_1;
  keycodes[0x13] = KB_KEY_2;
  keycodes[0x14] = KB_KEY_3;
  keycodes[0x15] = KB_KEY_4;
  keycodes[0x17] = KB_KEY_5;
  keycodes[0x16] = KB_KEY_6;
  keycodes[0x1A] = KB_KEY_7;
  keycodes[0x1C] = KB_KEY_8;
  keycodes[0x19] = KB_KEY_9;
  keycodes[0x00] = KB_KEY_A;
  keycodes[0x0B] = KB_KEY_B;
  keycodes[0x08] = KB_KEY_C;
  keycodes[0x02] = KB_KEY_D;
  keycodes[0x0E] = KB_KEY_E;
  keycodes[0x03] = KB_KEY_F;
  keycodes[0x05] = KB_KEY_G;
  keycodes[0x04] = KB_KEY_H;
  keycodes[0x22] = KB_KEY_I;
  keycodes[0x26] = KB_KEY_J;
  keycodes[0x28] = KB_KEY_K;
  keycodes[0x25] = KB_KEY_L;
  keycodes[0x2E] = KB_KEY_M;
  keycodes[0x2D] = KB_KEY_N;
  keycodes[0x1F] = KB_KEY_O;
  keycodes[0x23] = KB_KEY_P;
  keycodes[0x0C] = KB_KEY_Q;
  keycodes[0x0F] = KB_KEY_R;
  keycodes[0x01] = KB_KEY_S;
  keycodes[0x11] = KB_KEY_T;
  keycodes[0x20] = KB_KEY_U;
  keycodes[0x09] = KB_KEY_V;
  keycodes[0x0D] = KB_KEY_W;
  keycodes[0x07] = KB_KEY_X;
  keycodes[0x10] = KB_KEY_Y;
  keycodes[0x06] = KB_KEY_Z;

  keycodes[0x27] = KB_KEY_APOSTROPHE;
  keycodes[0x2A] = KB_KEY_BACKSLASH;
  keycodes[0x2B] = KB_KEY_COMMA;
  keycodes[0x18] = KB_KEY_EQUAL;
  keycodes[0x32] = KB_KEY_GRAVE_ACCENT;
  keycodes[0x21] = KB_KEY_LEFT_BRACKET;
  keycodes[0x1B] = KB_KEY_MINUS;
  keycodes[0x2F] = KB_KEY_PERIOD;
  keycodes[0x1E] = KB_KEY_RIGHT_BRACKET;
  keycodes[0x29] = KB_KEY_SEMICOLON;
  keycodes[0x2C] = KB_KEY_SLASH;
  keycodes[0x0A] = KB_KEY_WORLD_1;

  keycodes[0x33] = KB_KEY_BACKSPACE;
  keycodes[0x39] = KB_KEY_CAPS_LOCK;
  keycodes[0x75] = KB_KEY_DELETE;
  keycodes[0x7D] = KB_KEY_DOWN;
  keycodes[0x77] = KB_KEY_END;
  keycodes[0x24] = KB_KEY_ENTER;
  keycodes[0x35] = KB_KEY_ESCAPE;
  keycodes[0x7A] = KB_KEY_F1;
  keycodes[0x78] = KB_KEY_F2;
  keycodes[0x63] = KB_KEY_F3;
  keycodes[0x76] = KB_KEY_F4;
  keycodes[0x60] = KB_KEY_F5;
  keycodes[0x61] = KB_KEY_F6;
  keycodes[0x62] = KB_KEY_F7;
  keycodes[0x64] = KB_KEY_F8;
  keycodes[0x65] = KB_KEY_F9;
  keycodes[0x6D] = KB_KEY_F10;
  keycodes[0x67] = KB_KEY_F11;
  keycodes[0x6F] = KB_KEY_F12;
  keycodes[0x69] = KB_KEY_F13;
  keycodes[0x6B] = KB_KEY_F14;
  keycodes[0x71] = KB_KEY_F15;
  keycodes[0x6A] = KB_KEY_F16;
  keycodes[0x40] = KB_KEY_F17;
  keycodes[0x4F] = KB_KEY_F18;
  keycodes[0x50] = KB_KEY_F19;
  keycodes[0x5A] = KB_KEY_F20;
  keycodes[0x73] = KB_KEY_HOME;
  keycodes[0x72] = KB_KEY_INSERT;
  keycodes[0x7B] = KB_KEY_LEFT;
  keycodes[0x3A] = KB_KEY_LEFT_ALT;
  keycodes[0x3B] = KB_KEY_LEFT_CONTROL;
  keycodes[0x38] = KB_KEY_LEFT_SHIFT;
  keycodes[0x37] = KB_KEY_LEFT_SUPER;
  keycodes[0x6E] = KB_KEY_MENU;
  keycodes[0x47] = KB_KEY_NUM_LOCK;
  keycodes[0x79] = KB_KEY_PAGE_DOWN;
  keycodes[0x74] = KB_KEY_PAGE_UP;
  keycodes[0x7C] = KB_KEY_RIGHT;
  keycodes[0x3D] = KB_KEY_RIGHT_ALT;
  keycodes[0x3E] = KB_KEY_RIGHT_CONTROL;
  keycodes[0x3C] = KB_KEY_RIGHT_SHIFT;
  keycodes[0x36] = KB_KEY_RIGHT_SUPER;
  keycodes[0x31] = KB_KEY_SPACE;
  keycodes[0x30] = KB_KEY_TAB;
  keycodes[0x7E] = KB_KEY_UP;

  keycodes[0x52] = KB_KEY_KP_0;
  keycodes[0x53] = KB_KEY_KP_1;
  keycodes[0x54] = KB_KEY_KP_2;
  keycodes[0x55] = KB_KEY_KP_3;
  keycodes[0x56] = KB_KEY_KP_4;
  keycodes[0x57] = KB_KEY_KP_5;
  keycodes[0x58] = KB_KEY_KP_6;
  keycodes[0x59] = KB_KEY_KP_7;
  keycodes[0x5B] = KB_KEY_KP_8;
  keycodes[0x5C] = KB_KEY_KP_9;
  keycodes[0x45] = KB_KEY_KP_ADD;
  keycodes[0x41] = KB_KEY_KP_DECIMAL;
  keycodes[0x4B] = KB_KEY_KP_DIVIDE;
  keycodes[0x4C] = KB_KEY_KP_ENTER;
  keycodes[0x51] = KB_KEY_KP_EQUAL;
  keycodes[0x43] = KB_KEY_KP_MULTIPLY;
  keycodes[0x4E] = KB_KEY_KP_SUBTRACT;

  for (int sc = 0;  sc < 256; ++sc) {
    if (keycodes[sc] >= 0)
      scancodes[keycodes[sc]] = sc;
  }

  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  app = [[osx_app_t alloc] initWithContentRect:NSMakeRect(0, 0, w, h + 22)
                                     styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskTitled
                                       backing:NSBackingStoreBuffered
                                         defer:NO];
  if (!app) {
    release();
    SET_LAST_ERROR("[osx_app_t initWithContentRect] failed");
    return NULL;
  }

  id app_del = [AppDelegate alloc];
  if (!app_del) {
    release();
    SET_LAST_ERROR("[AppDelegate alloc] failed");
    [NSApp terminate:nil];
  }

  [app setDelegate:app_del];
  [app setAcceptsMouseMovedEvents:YES];
  [app setRestorable:NO];
  [app setTitle:(t ? [NSString stringWithUTF8String:t] : [[NSProcessInfo processInfo] processName])];
  [app setReleasedWhenClosed:NO];
  [app performSelectorOnMainThread:@selector(makeKeyAndOrderFront:) withObject:nil waitUntilDone:YES];
  [app center];

  [NSApp activateIgnoringOtherApps:YES];
  [pool drain];

  return buffer;
}

bool should_close() {
  return app->closed;
}

bool poll_events(user_event_t* ue) {
  bool ret = true;
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSEvent* e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                  untilDate:[NSDate distantPast]
                                     inMode:NSDefaultRunLoopMode
                                    dequeue:YES];

  if (ue) {
    memset(ue, 0, sizeof(user_event_t));
    if (e) {
      switch ([e type]) {
        case NSEventTypeKeyUp:
          ue->type = KEYBOARD_KEY_UP;
          ue->sym = translate_key([e keyCode]);
          ue->mod = translate_mod([e modifierFlags]);
          break;
        case NSEventTypeKeyDown:
          ue->type = KEYBOARD_KEY_DOWN;
          ue->sym = translate_key([e keyCode]);
          ue->mod = translate_mod([e modifierFlags]);
          break;
        case NSEventTypeLeftMouseUp:
        case NSEventTypeRightMouseUp:
        case NSEventTypeOtherMouseUp:
          ue->type = MOUSE_BTN_UP;
        case NSEventTypeLeftMouseDown:
        case NSEventTypeRightMouseDown:
        case NSEventTypeOtherMouseDown:
          if (ue->type != MOUSE_BTN_UP)
            ue->type = MOUSE_BTN_DOWN;
          ue->btn = (mousebtn_t)[e buttonNumber];
          ue->mod = translate_mod([e modifierFlags]);
          ue->data1 = mx;
          ue->data2 = my;
          break;
        case NSEventTypeScrollWheel:
          ue->type = SCROLL_WHEEL;
          ue->data1 = [e deltaX];
          ue->data2 = [e deltaY];
          break;
        default:
          if (app->closed) {
            ue->type = WINDOW_CLOSED;
            ret = true;
          } else
            ret = false;
          break;
      }
      [NSApp sendEvent:e];
    } else
      ret = false;
  } else {
    [NSApp sendEvent:e];
    ret = false;
  }

  [pool release];
  return ret;
}

void render() {
  [[app contentView] setNeedsDisplay:YES];
}

void release() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  if (app)
    [app close];
  destroy(&buffer);
  [pool drain];
}
#elif defined(_WIN32)
static WNDCLASS wnd;
static HWND hwnd;
static bool closed = false;
static HDC hdc;
static BITMAPINFO* bmpinfo;
static user_event_t* tmp_ue;
static bool event_fired = false;

static int translate_mod() {
  int mods = 0;

  if (GetKeyState(VK_SHIFT) & 0x8000)
    mods |= KB_MOD_SHIFT;
  if (GetKeyState(VK_CONTROL) & 0x8000)
    mods |= KB_MOD_CONTROL;
  if (GetKeyState(VK_MENU) & 0x8000)
    mods |= KB_MOD_ALT;
  if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
    mods |= KB_MOD_SUPER;
  if (GetKeyState(VK_CAPITAL) & 1)
    mods |= KB_MOD_CAPS_LOCK;
  if (GetKeyState(VK_NUMLOCK) & 1)
    mods |= KB_MOD_NUM_LOCK;

  return mods;
}

static int translate_key(WPARAM wParam, LPARAM lParam) {
  if (wParam == VK_CONTROL) {
    MSG next;
    DWORD time;

    if (lParam & 0x01000000)
      return KB_KEY_RIGHT_CONTROL;

    time = GetMessageTime();
    if (PeekMessageW(&next, NULL, 0, 0, PM_NOREMOVE))
      if (next.message == WM_KEYDOWN || next.message == WM_SYSKEYDOWN || next.message == WM_KEYUP || next.message == WM_SYSKEYUP)
        if (next.wParam == VK_MENU && (next.lParam & 0x01000000) && next.time == time)
          return KB_KEY_UNKNOWN;

    return KB_KEY_LEFT_CONTROL;
  }

  if (wParam == VK_PROCESSKEY)
    return KB_KEY_UNKNOWN;

  return keycodes[HIWORD(lParam) & 0x1FF];
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  LRESULT res = 0;
  switch (message) {
    case WM_PAINT:
      if (buffer) {
        StretchDIBits(hdc, 0, 0, buffer->w, buffer->h, 0, 0, buffer->w, buffer->h, buffer->buf, bmpinfo, DIB_RGB_COLORS, SRCCOPY);
        ValidateRect(hWnd, NULL);
      }
      break;
    case WM_CLOSE:
      closed = true;
      if (tmp_ue)
        tmp_ue->type = WINDOW_CLOSED;
      break;
    case WM_CHAR:
    case WM_SYSCHAR:
    case WM_UNICHAR: {
      const bool plain = (message != WM_SYSCHAR);
      if (message == WM_UNICHAR && wParam == UNICODE_NOCHAR)
        return FALSE;
      tmp_ue->type = KEYBOARD_KEY_UP;
      tmp_ue->mod = translate_mod();
      tmp_ue->sym = (unsigned int)wParam;
      break;
    }
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      const int key = translate_key(wParam, lParam);
      const int scancode = (lParam >> 16) & 0x1ff;
      const int action = ((lParam >> 31) & 1) ? KEYBOARD_KEY_UP : KEYBOARD_KEY_DOWN;
      const int mods = translate_mod();

      if (key == KB_KEY_UNKNOWN) {
        tmp_ue = NULL;
        return FALSE;
      }

      if (action == KEYBOARD_KEY_UP && wParam == VK_SHIFT) {
        tmp_ue->type = action;
        tmp_ue->mod = mods;
        tmp_ue->sym = KB_KEY_LEFT_SHIFT;
      } else if (wParam == VK_SNAPSHOT) {
        tmp_ue->type = KEYBOARD_KEY_UP;
        tmp_ue->mod = mods;
        tmp_ue->sym = key;
      } else {
        tmp_ue->type = action;
        tmp_ue->mod = mods;
        tmp_ue->sym = key;
      }
      break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_XBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP: {
      int button, action = MOUSE_BTN_UP;

      switch (message) {
        case WM_LBUTTONDOWN:
          action = MOUSE_BTN_DOWN;
        case WM_LBUTTONUP:
          button = MOUSE_BTN_0;
          break;
        case WM_RBUTTONDOWN:
          action = MOUSE_BTN_DOWN;
        case WM_RBUTTONUP:
          button = MOUSE_BTN_1;
        case WM_MBUTTONDOWN:
          action = MOUSE_BTN_DOWN;
        case WM_MBUTTONUP:
          button = MOUSE_BTN_2;
        default:
          button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MOUSE_BTN_4 : MOUSE_BTN_5);
          if (message == WM_XBUTTONDOWN)
            action = MOUSE_BTN_DOWN;
      }

      tmp_ue->type = action;
      tmp_ue->btn = button;
      tmp_ue->mod = translate_mod();
      tmp_ue->data1 = mx;
      tmp_ue->data2 = my;
      break;
    }
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
      tmp_ue->type = SCROLL_WHEEL;
      tmp_ue->data1 = -((SHORT)HIWORD(wParam) / (double)WHEEL_DELTA);
      tmp_ue->data2 = (SHORT)HIWORD(wParam) / (double)WHEEL_DELTA;
      break;
    case WM_MOUSEMOVE:
      mx = GET_X_LPARAM(lParam);
      my = GET_Y_LPARAM(lParam);
      break;
    default:
      res = DefWindowProc(hWnd, message, wParam, lParam);
  }
  return res;
}

surface_t* screen(const char* t, int w, int h) {
  if (!(buffer = surface(w, h)))
    return NULL;
  buffer->w = w;
  buffer->h = h;

  memset(keycodes, -1, sizeof(keycodes));
  memset(scancodes, -1, sizeof(scancodes));

  keycodes[0x00B] = KB_KEY_0;
  keycodes[0x002] = KB_KEY_1;
  keycodes[0x003] = KB_KEY_2;
  keycodes[0x004] = KB_KEY_3;
  keycodes[0x005] = KB_KEY_4;
  keycodes[0x006] = KB_KEY_5;
  keycodes[0x007] = KB_KEY_6;
  keycodes[0x008] = KB_KEY_7;
  keycodes[0x009] = KB_KEY_8;
  keycodes[0x00A] = KB_KEY_9;
  keycodes[0x01E] = KB_KEY_A;
  keycodes[0x030] = KB_KEY_B;
  keycodes[0x02E] = KB_KEY_C;
  keycodes[0x020] = KB_KEY_D;
  keycodes[0x012] = KB_KEY_E;
  keycodes[0x021] = KB_KEY_F;
  keycodes[0x022] = KB_KEY_G;
  keycodes[0x023] = KB_KEY_H;
  keycodes[0x017] = KB_KEY_I;
  keycodes[0x024] = KB_KEY_J;
  keycodes[0x025] = KB_KEY_K;
  keycodes[0x026] = KB_KEY_L;
  keycodes[0x032] = KB_KEY_M;
  keycodes[0x031] = KB_KEY_N;
  keycodes[0x018] = KB_KEY_O;
  keycodes[0x019] = KB_KEY_P;
  keycodes[0x010] = KB_KEY_Q;
  keycodes[0x013] = KB_KEY_R;
  keycodes[0x01F] = KB_KEY_S;
  keycodes[0x014] = KB_KEY_T;
  keycodes[0x016] = KB_KEY_U;
  keycodes[0x02F] = KB_KEY_V;
  keycodes[0x011] = KB_KEY_W;
  keycodes[0x02D] = KB_KEY_X;
  keycodes[0x015] = KB_KEY_Y;
  keycodes[0x02C] = KB_KEY_Z;

  keycodes[0x028] = KB_KEY_APOSTROPHE;
  keycodes[0x02B] = KB_KEY_BACKSLASH;
  keycodes[0x033] = KB_KEY_COMMA;
  keycodes[0x00D] = KB_KEY_EQUAL;
  keycodes[0x029] = KB_KEY_GRAVE_ACCENT;
  keycodes[0x01A] = KB_KEY_LEFT_BRACKET;
  keycodes[0x00C] = KB_KEY_MINUS;
  keycodes[0x034] = KB_KEY_PERIOD;
  keycodes[0x01B] = KB_KEY_RIGHT_BRACKET;
  keycodes[0x027] = KB_KEY_SEMICOLON;
  keycodes[0x035] = KB_KEY_SLASH;
  keycodes[0x056] = KB_KEY_WORLD_2;

  keycodes[0x00E] = KB_KEY_BACKSPACE;
  keycodes[0x153] = KB_KEY_DELETE;
  keycodes[0x14F] = KB_KEY_END;
  keycodes[0x01C] = KB_KEY_ENTER;
  keycodes[0x001] = KB_KEY_ESCAPE;
  keycodes[0x147] = KB_KEY_HOME;
  keycodes[0x152] = KB_KEY_INSERT;
  keycodes[0x15D] = KB_KEY_MENU;
  keycodes[0x151] = KB_KEY_PAGE_DOWN;
  keycodes[0x149] = KB_KEY_PAGE_UP;
  keycodes[0x045] = KB_KEY_PAUSE;
  keycodes[0x146] = KB_KEY_PAUSE;
  keycodes[0x039] = KB_KEY_SPACE;
  keycodes[0x00F] = KB_KEY_TAB;
  keycodes[0x03A] = KB_KEY_CAPS_LOCK;
  keycodes[0x145] = KB_KEY_NUM_LOCK;
  keycodes[0x046] = KB_KEY_SCROLL_LOCK;
  keycodes[0x03B] = KB_KEY_F1;
  keycodes[0x03C] = KB_KEY_F2;
  keycodes[0x03D] = KB_KEY_F3;
  keycodes[0x03E] = KB_KEY_F4;
  keycodes[0x03F] = KB_KEY_F5;
  keycodes[0x040] = KB_KEY_F6;
  keycodes[0x041] = KB_KEY_F7;
  keycodes[0x042] = KB_KEY_F8;
  keycodes[0x043] = KB_KEY_F9;
  keycodes[0x044] = KB_KEY_F10;
  keycodes[0x057] = KB_KEY_F11;
  keycodes[0x058] = KB_KEY_F12;
  keycodes[0x064] = KB_KEY_F13;
  keycodes[0x065] = KB_KEY_F14;
  keycodes[0x066] = KB_KEY_F15;
  keycodes[0x067] = KB_KEY_F16;
  keycodes[0x068] = KB_KEY_F17;
  keycodes[0x069] = KB_KEY_F18;
  keycodes[0x06A] = KB_KEY_F19;
  keycodes[0x06B] = KB_KEY_F20;
  keycodes[0x06C] = KB_KEY_F21;
  keycodes[0x06D] = KB_KEY_F22;
  keycodes[0x06E] = KB_KEY_F23;
  keycodes[0x076] = KB_KEY_F24;
  keycodes[0x038] = KB_KEY_LEFT_ALT;
  keycodes[0x01D] = KB_KEY_LEFT_CONTROL;
  keycodes[0x02A] = KB_KEY_LEFT_SHIFT;
  keycodes[0x15B] = KB_KEY_LEFT_SUPER;
  keycodes[0x137] = KB_KEY_PRINT_SCREEN;
  keycodes[0x138] = KB_KEY_RIGHT_ALT;
  keycodes[0x11D] = KB_KEY_RIGHT_CONTROL;
  keycodes[0x036] = KB_KEY_RIGHT_SHIFT;
  keycodes[0x15C] = KB_KEY_RIGHT_SUPER;
  keycodes[0x150] = KB_KEY_DOWN;
  keycodes[0x14B] = KB_KEY_LEFT;
  keycodes[0x14D] = KB_KEY_RIGHT;
  keycodes[0x148] = KB_KEY_UP;

  keycodes[0x052] = KB_KEY_KP_0;
  keycodes[0x04F] = KB_KEY_KP_1;
  keycodes[0x050] = KB_KEY_KP_2;
  keycodes[0x051] = KB_KEY_KP_3;
  keycodes[0x04B] = KB_KEY_KP_4;
  keycodes[0x04C] = KB_KEY_KP_5;
  keycodes[0x04D] = KB_KEY_KP_6;
  keycodes[0x047] = KB_KEY_KP_7;
  keycodes[0x048] = KB_KEY_KP_8;
  keycodes[0x049] = KB_KEY_KP_9;
  keycodes[0x04E] = KB_KEY_KP_ADD;
  keycodes[0x053] = KB_KEY_KP_DECIMAL;
  keycodes[0x135] = KB_KEY_KP_DIVIDE;
  keycodes[0x11C] = KB_KEY_KP_ENTER;
  keycodes[0x037] = KB_KEY_KP_MULTIPLY;
  keycodes[0x04A] = KB_KEY_KP_SUBTRACT;

  for (int sc = 0; sc < 512; sc++) {
    if (keycodes[sc] > 0)
      scancodes[keycodes[sc]] = sc;
  }

  RECT rect = { 0 };

  wnd.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
  wnd.lpfnWndProc = WndProc;
  wnd.hCursor = LoadCursor(0, IDC_ARROW);
  wnd.lpszClassName = t;
  RegisterClass(&wnd);

  rect.right = w;
  rect.bottom = h;

  AdjustWindowRect(&rect, WS_POPUP | WS_SYSMENU | WS_CAPTION, 0);

  rect.right -= rect.left;
  rect.bottom -= rect.top;

  if (!(hwnd = CreateWindowEx(0, t, t, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME, CW_USEDEFAULT, CW_USEDEFAULT, rect.right, rect.bottom, 0, 0, 0, 0))) {
    release();
    SET_LAST_ERROR("CreateWindowEx() failed: %s", GetLastError());
    return NULL;
  }

  ShowWindow(hwnd, SW_NORMAL);

  bmpinfo = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3);
  bmpinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmpinfo->bmiHeader.biPlanes = 1;
  bmpinfo->bmiHeader.biBitCount = 32;
  bmpinfo->bmiHeader.biCompression = BI_BITFIELDS;
  bmpinfo->bmiHeader.biWidth = w;
  bmpinfo->bmiHeader.biHeight = -h;
  bmpinfo->bmiColors[0].rgbRed = 0xff;
  bmpinfo->bmiColors[1].rgbGreen = 0xff;
  bmpinfo->bmiColors[2].rgbBlue = 0xff;

  hdc = GetDC(hwnd);

  return buffer;
}

bool should_close() {
  return closed;
}

bool poll_events(user_event_t* ue) {
  if (!ue)
    return false;
  memset(ue, 0, sizeof(user_event_t));
  tmp_ue = ue;

  MSG msg;
  if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return !!tmp_ue;
  }
  return false;
}

void render() {
  InvalidateRect(hwnd, NULL, TRUE);
  SendMessage(hwnd, WM_PAINT, 0, 0);
}

void release() {
  destroy(&buffer);
  ReleaseDC(hwnd, hdc);
  DestroyWindow(hwnd);
}
#else
static Display* display;
static bool closed = false;
static surface_t* buffer;
static Window win;
static GC gc;
static XImage* img;
static XEvent event;
static KeySym sym;

#define Button6 6
#define Button7 7

static int translate_keycode(int scancode) {
  if (scancode < 8 || scancode > 255)
    return KB_KEY_UNKNOWN;

  int _sym = XkbKeycodeToKeysym(display, scancode, 0, 1);
  switch (_sym) {
  case XK_KP_0:           return KB_KEY_KP_0;
  case XK_KP_1:           return KB_KEY_KP_1;
  case XK_KP_2:           return KB_KEY_KP_2;
  case XK_KP_3:           return KB_KEY_KP_3;
  case XK_KP_4:           return KB_KEY_KP_4;
  case XK_KP_5:           return KB_KEY_KP_5;
  case XK_KP_6:           return KB_KEY_KP_6;
  case XK_KP_7:           return KB_KEY_KP_7;
  case XK_KP_8:           return KB_KEY_KP_8;
  case XK_KP_9:           return KB_KEY_KP_9;
  case XK_KP_Separator:
  case XK_KP_Decimal:     return KB_KEY_KP_DECIMAL;
  case XK_KP_Equal:       return KB_KEY_KP_EQUAL;
  case XK_KP_Enter:       return KB_KEY_KP_ENTER;
  default:
    break;
  }
  _sym = XkbKeycodeToKeysym(display, scancode, 0, 0);

  switch (_sym) {
  case XK_Escape:         return KB_KEY_ESCAPE;
  case XK_Tab:            return KB_KEY_TAB;
  case XK_Shift_L:        return KB_KEY_LEFT_SHIFT;
  case XK_Shift_R:        return KB_KEY_RIGHT_SHIFT;
  case XK_Control_L:      return KB_KEY_LEFT_CONTROL;
  case XK_Control_R:      return KB_KEY_RIGHT_CONTROL;
  case XK_Meta_L:
  case XK_Alt_L:          return KB_KEY_LEFT_ALT;
  case XK_Mode_switch: // Mapped to Alt_R on many keyboards
  case XK_ISO_Level3_Shift: // AltGr on at least some machines
  case XK_Meta_R:
  case XK_Alt_R:          return KB_KEY_RIGHT_ALT;
  case XK_Super_L:        return KB_KEY_LEFT_SUPER;
  case XK_Super_R:        return KB_KEY_RIGHT_SUPER;
  case XK_Menu:           return KB_KEY_MENU;
  case XK_Num_Lock:       return KB_KEY_NUM_LOCK;
  case XK_Caps_Lock:      return KB_KEY_CAPS_LOCK;
  case XK_Print:          return KB_KEY_PRINT_SCREEN;
  case XK_Scroll_Lock:    return KB_KEY_SCROLL_LOCK;
  case XK_Pause:          return KB_KEY_PAUSE;
  case XK_Delete:         return KB_KEY_DELETE;
  case XK_BackSpace:      return KB_KEY_BACKSPACE;
  case XK_Return:         return KB_KEY_ENTER;
  case XK_Home:           return KB_KEY_HOME;
  case XK_End:            return KB_KEY_END;
  case XK_Page_Up:        return KB_KEY_PAGE_UP;
  case XK_Page_Down:      return KB_KEY_PAGE_DOWN;
  case XK_Insert:         return KB_KEY_INSERT;
  case XK_Left:           return KB_KEY_LEFT;
  case XK_Right:          return KB_KEY_RIGHT;
  case XK_Down:           return KB_KEY_DOWN;
  case XK_Up:             return KB_KEY_UP;
  case XK_F1:             return KB_KEY_F1;
  case XK_F2:             return KB_KEY_F2;
  case XK_F3:             return KB_KEY_F3;
  case XK_F4:             return KB_KEY_F4;
  case XK_F5:             return KB_KEY_F5;
  case XK_F6:             return KB_KEY_F6;
  case XK_F7:             return KB_KEY_F7;
  case XK_F8:             return KB_KEY_F8;
  case XK_F9:             return KB_KEY_F9;
  case XK_F10:            return KB_KEY_F10;
  case XK_F11:            return KB_KEY_F11;
  case XK_F12:            return KB_KEY_F12;
  case XK_F13:            return KB_KEY_F13;
  case XK_F14:            return KB_KEY_F14;
  case XK_F15:            return KB_KEY_F15;
  case XK_F16:            return KB_KEY_F16;
  case XK_F17:            return KB_KEY_F17;
  case XK_F18:            return KB_KEY_F18;
  case XK_F19:            return KB_KEY_F19;
  case XK_F20:            return KB_KEY_F20;
  case XK_F21:            return KB_KEY_F21;
  case XK_F22:            return KB_KEY_F22;
  case XK_F23:            return KB_KEY_F23;
  case XK_F24:            return KB_KEY_F24;
  case XK_F25:            return KB_KEY_F25;

  // Numeric keypad
  case XK_KP_Divide:      return KB_KEY_KP_DIVIDE;
  case XK_KP_Multiply:    return KB_KEY_KP_MULTIPLY;
  case XK_KP_Subtract:    return KB_KEY_KP_SUBTRACT;
  case XK_KP_Add:         return KB_KEY_KP_ADD;

  // These should have been detected in secondary keysym test above!
  case XK_KP_Insert:      return KB_KEY_KP_0;
  case XK_KP_End:         return KB_KEY_KP_1;
  case XK_KP_Down:        return KB_KEY_KP_2;
  case XK_KP_Page_Down:   return KB_KEY_KP_3;
  case XK_KP_Left:        return KB_KEY_KP_4;
  case XK_KP_Right:       return KB_KEY_KP_6;
  case XK_KP_Home:        return KB_KEY_KP_7;
  case XK_KP_Up:          return KB_KEY_KP_8;
  case XK_KP_Page_Up:     return KB_KEY_KP_9;
  case XK_KP_Delete:      return KB_KEY_KP_DECIMAL;
  case XK_KP_Equal:       return KB_KEY_KP_EQUAL;
  case XK_KP_Enter:       return KB_KEY_KP_ENTER;

  // Last resort: Check for printable keys (should not happen if the XKB
  // extension is available). This will give a layout dependent mapping
  // (which is wrong, and we may miss some keys, especially on non-US
  // keyboards), but it's better than nothing...
  case XK_a:              return KB_KEY_A;
  case XK_b:              return KB_KEY_B;
  case XK_c:              return KB_KEY_C;
  case XK_d:              return KB_KEY_D;
  case XK_e:              return KB_KEY_E;
  case XK_f:              return KB_KEY_F;
  case XK_g:              return KB_KEY_G;
  case XK_h:              return KB_KEY_H;
  case XK_i:              return KB_KEY_I;
  case XK_j:              return KB_KEY_J;
  case XK_k:              return KB_KEY_K;
  case XK_l:              return KB_KEY_L;
  case XK_m:              return KB_KEY_M;
  case XK_n:              return KB_KEY_N;
  case XK_o:              return KB_KEY_O;
  case XK_p:              return KB_KEY_P;
  case XK_q:              return KB_KEY_Q;
  case XK_r:              return KB_KEY_R;
  case XK_s:              return KB_KEY_S;
  case XK_t:              return KB_KEY_T;
  case XK_u:              return KB_KEY_U;
  case XK_v:              return KB_KEY_V;
  case XK_w:              return KB_KEY_W;
  case XK_x:              return KB_KEY_X;
  case XK_y:              return KB_KEY_Y;
  case XK_z:              return KB_KEY_Z;
  case XK_1:              return KB_KEY_1;
  case XK_2:              return KB_KEY_2;
  case XK_3:              return KB_KEY_3;
  case XK_4:              return KB_KEY_4;
  case XK_5:              return KB_KEY_5;
  case XK_6:              return KB_KEY_6;
  case XK_7:              return KB_KEY_7;
  case XK_8:              return KB_KEY_8;
  case XK_9:              return KB_KEY_9;
  case XK_0:              return KB_KEY_0;
  case XK_space:          return KB_KEY_SPACE;
  case XK_minus:          return KB_KEY_MINUS;
  case XK_equal:          return KB_KEY_EQUAL;
  case XK_bracketleft:    return KB_KEY_LEFT_BRACKET;
  case XK_bracketright:   return KB_KEY_RIGHT_BRACKET;
  case XK_backslash:      return KB_KEY_BACKSLASH;
  case XK_semicolon:      return KB_KEY_SEMICOLON;
  case XK_apostrophe:     return KB_KEY_APOSTROPHE;
  case XK_grave:          return KB_KEY_GRAVE_ACCENT;
  case XK_comma:          return KB_KEY_COMMA;
  case XK_period:         return KB_KEY_PERIOD;
  case XK_slash:          return KB_KEY_SLASH;
  case XK_less:           return KB_KEY_WORLD_1; // At least in some layouts...
  default:
    break;
  }

  return KB_KEY_UNKNOWN;
}

static int translate_key(int scancode) {
  if (scancode < 0 || scancode > 255)
    return KB_KEY_UNKNOWN;
  return keycodes[scancode];
}

static int translate_mod(int state) {
  int mods = 0;

  if (state & ShiftMask)
    mods |= KB_MOD_SHIFT;
  if (state & ControlMask)
    mods |= KB_MOD_CONTROL;
  if (state & Mod1Mask)
    mods |= KB_MOD_ALT;
  if (state & Mod4Mask)
    mods |= KB_MOD_SUPER;
  if (state & LockMask)
    mods |= KB_MOD_CAPS_LOCK;
  if (state & Mod2Mask)
    mods |= KB_MOD_NUM_LOCK;

  return mods;
}

surface_t* screen(const char* title, int w, int h) {
  if (!(buffer = surface(w, h)))
    return NULL;
  buffer->w = w;
  buffer->h = h;

  display = XOpenDisplay(0);
  if (!display) {
    release();
    SET_LAST_ERROR("XOpenDisplay(0) failed!");
    return NULL;
  }

  memset(keycodes, -1, sizeof(keycodes));
  memset(scancodes, -1, sizeof(scancodes));

  int scancode, key;
  char name[XkbKeyNameLength + 1];
  XkbDescPtr desc = XkbGetMap(display, 0, XkbUseCoreKbd);
  XkbGetNames(display, XkbKeyNamesMask, desc);

  for (scancode = desc->min_key_code;  scancode <= desc->max_key_code;  scancode++) {
    memcpy(name, desc->names->keys[scancode].name, XkbKeyNameLength);
    name[XkbKeyNameLength] = '\0';

    if      (strcmp(name, "TLDE") == 0) key = KB_KEY_GRAVE_ACCENT;
    else if (strcmp(name, "AE01") == 0) key = KB_KEY_1;
    else if (strcmp(name, "AE02") == 0) key = KB_KEY_2;
    else if (strcmp(name, "AE03") == 0) key = KB_KEY_3;
    else if (strcmp(name, "AE04") == 0) key = KB_KEY_4;
    else if (strcmp(name, "AE05") == 0) key = KB_KEY_5;
    else if (strcmp(name, "AE06") == 0) key = KB_KEY_6;
    else if (strcmp(name, "AE07") == 0) key = KB_KEY_7;
    else if (strcmp(name, "AE08") == 0) key = KB_KEY_8;
    else if (strcmp(name, "AE09") == 0) key = KB_KEY_9;
    else if (strcmp(name, "AE10") == 0) key = KB_KEY_0;
    else if (strcmp(name, "AE11") == 0) key = KB_KEY_MINUS;
    else if (strcmp(name, "AE12") == 0) key = KB_KEY_EQUAL;
    else if (strcmp(name, "AD01") == 0) key = KB_KEY_Q;
    else if (strcmp(name, "AD02") == 0) key = KB_KEY_W;
    else if (strcmp(name, "AD03") == 0) key = KB_KEY_E;
    else if (strcmp(name, "AD04") == 0) key = KB_KEY_R;
    else if (strcmp(name, "AD05") == 0) key = KB_KEY_T;
    else if (strcmp(name, "AD06") == 0) key = KB_KEY_Y;
    else if (strcmp(name, "AD07") == 0) key = KB_KEY_U;
    else if (strcmp(name, "AD08") == 0) key = KB_KEY_I;
    else if (strcmp(name, "AD09") == 0) key = KB_KEY_O;
    else if (strcmp(name, "AD10") == 0) key = KB_KEY_P;
    else if (strcmp(name, "AD11") == 0) key = KB_KEY_LEFT_BRACKET;
    else if (strcmp(name, "AD12") == 0) key = KB_KEY_RIGHT_BRACKET;
    else if (strcmp(name, "AC01") == 0) key = KB_KEY_A;
    else if (strcmp(name, "AC02") == 0) key = KB_KEY_S;
    else if (strcmp(name, "AC03") == 0) key = KB_KEY_D;
    else if (strcmp(name, "AC04") == 0) key = KB_KEY_F;
    else if (strcmp(name, "AC05") == 0) key = KB_KEY_G;
    else if (strcmp(name, "AC06") == 0) key = KB_KEY_H;
    else if (strcmp(name, "AC07") == 0) key = KB_KEY_J;
    else if (strcmp(name, "AC08") == 0) key = KB_KEY_K;
    else if (strcmp(name, "AC09") == 0) key = KB_KEY_L;
    else if (strcmp(name, "AC10") == 0) key = KB_KEY_SEMICOLON;
    else if (strcmp(name, "AC11") == 0) key = KB_KEY_APOSTROPHE;
    else if (strcmp(name, "AB01") == 0) key = KB_KEY_Z;
    else if (strcmp(name, "AB02") == 0) key = KB_KEY_X;
    else if (strcmp(name, "AB03") == 0) key = KB_KEY_C;
    else if (strcmp(name, "AB04") == 0) key = KB_KEY_V;
    else if (strcmp(name, "AB05") == 0) key = KB_KEY_B;
    else if (strcmp(name, "AB06") == 0) key = KB_KEY_N;
    else if (strcmp(name, "AB07") == 0) key = KB_KEY_M;
    else if (strcmp(name, "AB08") == 0) key = KB_KEY_COMMA;
    else if (strcmp(name, "AB09") == 0) key = KB_KEY_PERIOD;
    else if (strcmp(name, "AB10") == 0) key = KB_KEY_SLASH;
    else if (strcmp(name, "BKSL") == 0) key = KB_KEY_BACKSLASH;
    else if (strcmp(name, "LSGT") == 0) key = KB_KEY_WORLD_1;
    else                                key = KB_KEY_UNKNOWN;

    if ((scancode >= 0) && (scancode < 256))
      keycodes[scancode] = key;
  }

  XkbFreeNames(desc, XkbKeyNamesMask, True);
  XkbFreeKeyboard(desc, 0, True);

  for (scancode = 0;  scancode < 256;  scancode++) {
    if (keycodes[scancode] < 0)
      keycodes[scancode] = translate_keycode(scancode);
    if (keycodes[scancode] > 0)
      scancodes[keycodes[scancode]] = scancode;
  }

  int screen = DefaultScreen(display);
  Visual* visual = DefaultVisual(display, screen);
  int format_c;
  XPixmapFormatValues* formats = XListPixmapFormats(display, &format_c);
  int depth = DefaultDepth(display, screen);
  Window default_root_win = DefaultRootWindow(display);

  int c_depth;
  for (int i = 0; i < format_c; ++i) {
    if (depth == formats[i].depth) {
      c_depth = formats[i].bits_per_pixel;
      break;
    }
  }
  XFree(formats);

  if (c_depth != 32) {
    release();
    SET_LAST_ERROR("Invalid display depth: %d", c_depth);
    return NULL;
  }

  int s_width = DisplayWidth(display, screen);
  int s_height = DisplayHeight(display, screen);

  XSetWindowAttributes win_attrib;
  win_attrib.border_pixel = BlackPixel(display, screen);
  win_attrib.background_pixel = BlackPixel(display, screen);
  win_attrib.backing_store = NotUseful;

  win = XCreateWindow(display, default_root_win, (s_width - w) / 2,
		      (s_height - h) / 2, w, h, 0, depth, InputOutput,
		      visual, CWBackPixel | CWBorderPixel | CWBackingStore,
		      &win_attrib);
  if (!win) {
    release();
    SET_LAST_ERROR("XCreateWindow() failed!");
    return NULL;
  }

  XSelectInput(display, win, KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
  XStoreName(display, win, title);

  XSizeHints hints;
  hints.flags = PPosition | PMinSize | PMaxSize;
  hints.x = 0;
  hints.y = 0;
  hints.min_width = w;
  hints.max_width = w;
  hints.min_height = h;
  hints.max_height = h;

  XSetWMNormalHints(display, win, &hints);
  XClearWindow(display, win);
  XMapRaised(display, win);
  XFlush(display);

  gc = DefaultGC(display, screen);

  img = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, NULL, w, h, 32, w * 4);

  return buffer;
}

bool should_close() {
  return closed;
}

bool poll_events(user_event_t* ue) {
  if (!ue || !XPending(display))
    return false;

  XNextEvent(display, &event);
  switch (event.type) {
  case KeyPress:
    ue->type = KEYBOARD_KEY_DOWN;
    ue->sym  = translate_key(event.xkey.keycode);
    ue->mod  = translate_mod(event.xkey.state);
    break;
  case KeyRelease:
    ue->type = KEYBOARD_KEY_UP;
    ue->sym  = translate_key(event.xkey.keycode);
    ue->mod  = translate_mod(event.xkey.state);
    break;
  case ButtonPress:
    ue->type = MOUSE_BTN_DOWN;
    ue->mod  = translate_mod(event.xkey.state);
    switch (event.xbutton.button) {
    case Button1:
      ue->btn = MOUSE_BTN_0;
      break;
    case Button2:
      ue->btn = MOUSE_BTN_1;
      break;
    case Button3:
      ue->btn = MOUSE_BTN_2;
      break;
    case Button4:
      ue->type = SCROLL_WHEEL;
      ue->data1 = 0;
      ue->data2 = 1;
      break;
    case Button5:
      ue->type = SCROLL_WHEEL;
      ue->data1 =  0;
      ue->data2 = -1;
      break;
    case Button6:
      ue->type = SCROLL_WHEEL;
      ue->data1 = 1;
      ue->data2 = 0;
      break;
    case Button7:
      ue->type = SCROLL_WHEEL;
      ue->data1 = -1;
      ue->data2 =  0;
      break;
    default:
      ue->btn = (event.xbutton.button - Button1 - 4);
    }
    break;
  case ButtonRelease:
    ue->type = MOUSE_BTN_UP;
    ue->mod  = translate_mod(event.xkey.state);
    switch (event.xbutton.button) {
    case Button1:
      ue->btn = MOUSE_BTN_0;
      break;
    case Button2:
      ue->btn = MOUSE_BTN_1;
      break;
    case Button3:
      ue->btn = MOUSE_BTN_2;
      break;
    default:
      ue->btn = (event.xbutton.button - Button1 - 4);
    }
    break;
  case MotionNotify:
    mx = event.xmotion.x;
    my = event.xmotion.y;
    break;
  case DestroyNotify:
    ue->type = WINDOW_CLOSED;
    closed = true;
  default:
    break;
  }
  return true;
}

void render() {
  img->data = (char*)buffer->buf;
  XPutImage(display, win, gc, img, 0, 0, 0, 0, buffer->w, buffer->h);
  XFlush(display);
}

void release() {
  destroy(&buffer);
  img->data = NULL;
  XDestroyImage(img);
  XDestroyWindow(display, win);
  XCloseDisplay(display);
}
#endif
