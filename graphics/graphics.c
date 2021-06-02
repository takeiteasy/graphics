/* graphics.c
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

#include "graphics.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#if defined(GRAPHICS_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER)
#define strdup _strdup
#endif

#define __MIN(a, b) (((a) < (b)) ? (a) : (b))
#define __MAX(a, b) (((a) > (b)) ? (a) : (b))
#define __CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#if !defined(M_PI)
#define M_PI 3.14159265358979323846264338327950288f
#endif
#define __D2R(a) ((a) * M_PI / 180.0)
#define __R2D(a) ((a) * 180.0 / M_PI)

#define LINKEDLIST(NAME, TYPE) \
struct NAME##_node_t { \
  TYPE *data; \
  struct NAME##_node_t *next; \
}; \
struct NAME##_node_t* NAME##_push(struct NAME##_node_t *head, TYPE *data) { \
  struct NAME##_node_t *ret = GRAPHICS_MALLOC(sizeof(struct NAME##_node_t)); \
  if (!ret) \
    return NULL; \
  ret->data = data; \
  ret->next = head; \
  return ret; \
} \
struct NAME##_node_t* NAME##_pop(struct NAME##_node_t *head, TYPE *data) { \
  struct NAME##_node_t *cursor = head, *prev = NULL; \
  while (cursor) { \
    if (cursor->data != data) { \
      prev = cursor; \
      cursor = cursor->next; \
      continue; \
    } \
    if (!prev) \
      head = cursor->next; \
    else \
      prev->next = cursor->next; \
    break; \
  } \
  if (cursor) { \
    cursor->next = NULL; \
    GRAPHICS_FREE(cursor); \
  } \
  return head; \
}

int rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
  return ((unsigned int)a << 24) | ((unsigned int)r << 16) | ((unsigned int)g << 8) | b;
}

int rgb(unsigned char r, unsigned char g, unsigned char b) {
  return rgba(r, g, b, 255);
}

int rgba1(unsigned char c) {
  return rgba(c, c, c, c);
}

int rgb1(unsigned char c) {
  return rgb(c, c, c);
}

unsigned char r_channel(int c) {
  return (unsigned char)((c >> 16) & 0xFF);
}

unsigned char g_channel(int c) {
  return (unsigned char)((c >>  8) & 0xFF);
}

unsigned char b_channel(int c) {
  return (unsigned char)(c & 0xFF);
}

unsigned char a_channel(int c) {
  return (unsigned char)((c >> 24) & 0xFF);
}

int rgba_r(int c, unsigned char r) {
  return (c & ~0x00FF0000) | (r << 16);
}

int rgba_g(int c, unsigned char g) {
  return (c & ~0x0000FF00) | (g << 8);
}

int rgba_b(int c, unsigned char b) {
  return (c & ~0x000000FF) | b;
}

int rgba_a(int c, unsigned char a) {
  return (c & ~0x00FF0000) | (a << 24);
}

bool surface(struct surface_t* s, unsigned int w, unsigned int h) {
  s->w = w;
  s->h = h;
  size_t sz = w * h * sizeof(unsigned int) + 1;
  s->buf = GRAPHICS_MALLOC(sz);
  if (!s->buf) {
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  memset(s->buf, 0, sz);

  return true;
}

void surface_destroy(struct surface_t* s) {
  GRAPHICS_SAFE_FREE(s->buf);
  memset(s, 0, sizeof(struct surface_t));
}

static enum draw_mode draw_mode = NORMAL;

void graphics_draw_mode(enum draw_mode m) {
  draw_mode = m;
}

void fill(struct surface_t* s, int col) {
  for (int i = 0; i < s->w * s->h; ++i)
    s->buf[i] = col;
}

static inline void flood_fn(struct surface_t* s, int x, int y, int new, int old) {
  if (new == old || pget(s, x, y) != old)
    return;
  
  int x1 = x;
  while (x1 < s->w && pget(s, x1, y) == old) {
    pset(s, x1, y, new);
    x1++;
  }
  
  x1 = x - 1;
  while (x1 >= 0 && pget(s, x1, y) == old) {
    pset(s, x1, y, new);
    x1--;
  }
  
  x1 = x;
  while (x1 < s->w && pget(s, x1, y) == new) {
    if(y > 0 && pget(s, x1, y - 1) == old)
      flood_fn(s, x1, y - 1, new, old);
    x1++;
  }
  
  x1 = x - 1;
  while(x1 >= 0 && pget(s, x1, y) == new) {
    if(y > 0 && pget(s, x1, y - 1) == old)
      flood_fn(s, x1, y - 1, new, old);
    x1--;
  }
  
  x1 = x;
  while(x1 < s->w && pget(s, x1, y) == new) {
    if(y < s->h - 1 && pget(s, x1, y + 1) == old)
      flood_fn(s, x1, y + 1, new, old);
    x1++;
  }
  
  x1 = x - 1;
  while(x1 >= 0 && pget(s, x1, y) == new) {
    if(y < s->h - 1 && pget(s, x1, y + 1) == old)
      flood_fn(s, x1, y + 1, new, old);
    x1--;
  }
}

void flood(struct surface_t* s, int x, int y, int col) {
  if (x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;
  flood_fn(s, x, y, col, pget(s, x, y));
}

void cls(struct surface_t* s) {
  memset(s->buf, 0, s->w * s->h * sizeof(int));
}

#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

void pset(struct surface_t* s, int x, int y, int c) {
  if (x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;
  switch (draw_mode) {
    case MASK:
      if (a_channel(c) < 255)
        return;
    default:
    case NORMAL:
      s->buf[y * s->w + x] = c;
      break;
    case ALPHA: {
      int  a = a_channel(c);
      int* p = &s->buf[y * s->w + x];
      int  b = a_channel(*p);
      *p = (a == 255 || !b) ? c : rgba(BLEND(r_channel(c), r_channel(*p), a, b),
                                       BLEND(g_channel(c), g_channel(*p), a, b),
                                       BLEND(b_channel(c), b_channel(*p), a, b),
                                       a + (b * (255 - a) >> 8));
      break;
    }
  }
}

int pget(struct surface_t* s, int x, int y) {
  return (x >= 0 && y >= 0 && x < s->w && y < s->h) ? s->buf[y * s->w + x] : 0;
}

bool paste(struct surface_t* dst, struct surface_t* src, int x, int y) {
  int ox, oy, c;
  for (ox = 0; ox < src->w; ++ox) {
    for (oy = 0; oy < src->h; ++oy) {
      if (oy > dst->h)
        break;
      c = pget(src, ox, oy);
      pset(dst, x + ox, y + oy, c);
    }
    if (ox > dst->w)
      break;
  }
  return true;
}

bool clip_paste(struct surface_t* dst, struct surface_t* src, int x, int y, int rx, int ry, int rw, int rh) {
  for (int ox = 0; ox < rw; ++ox)
    for (int oy = 0; oy < rh; ++oy)
      pset(dst, ox + x, oy + y, pget(src, ox + rx, oy + ry));
  return true;
}

bool reset(struct surface_t* s, int nw, int nh) {
    size_t sz = nw * nh * sizeof(unsigned int) + 1;
  int* tmp = GRAPHICS_REALLOC(s->buf, sz);
  if (!tmp) {
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "realloc() failed");
    return false;
  }
  s->buf = tmp;
  s->w = nw;
  s->h = nh;
  memset(s->buf, 0, sz);
  return true;
}

bool copy(struct surface_t* a, struct surface_t* b) {
  if (!surface(b, a->w, a->h))
    return false;
  memcpy(b->buf, a->buf, a->w * a->h * sizeof(unsigned int) + 1);
  return !!b->buf;
}

void passthru(struct surface_t* s, int (*fn)(int x, int y, int col)) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      pset(s, x, y, fn(x, y, pget(s, x, y)));
}

static void __resize(struct surface_t* a, struct surface_t* b) {
  int x_ratio = (int)((a->w << 16) / b->w) + 1;
  int y_ratio = (int)((a->h << 16) / b->h) + 1;
  int x2, y2, i, j;
  for (i = 0; i < b->h; ++i) {
    int* t = b->buf + i * b->w;
    y2 = ((i * y_ratio) >> 16);
    int* p = a->buf + y2 * a->w;
    int rat = 0;
    for (j = 0; j < b->w; ++j) {
      x2 = (rat >> 16);
      *t++ = p[x2];
      rat += x_ratio;
    }
  }
}

bool resize(struct surface_t* a, int nw, int nh, struct surface_t* b) {
  if (!surface(b, nw, nh))
    return false;
  __resize(a, b);
  return true;
}

bool rotate(struct surface_t* a, float angle, struct surface_t* b) {
  float theta = __D2R(angle);
  float c = cosf(theta), s = sinf(theta);
  float r[3][2] = {
    { -a->h * s, a->h * c },
    {  a->w * c - a->h * s, a->h * c + a->w * s },
    {  a->w * c, a->w * s }
  };

  float mm[2][2] = {{
    __MIN(0, __MIN(r[0][0], __MIN(r[1][0], r[2][0]))),
    __MIN(0, __MIN(r[0][1], __MIN(r[1][1], r[2][1])))
  }, {
    (theta > 1.5708  && theta < 3.14159 ? 0.f : __MAX(r[0][0], __MAX(r[1][0], r[2][0]))),
    (theta > 3.14159 && theta < 4.71239 ? 0.f : __MAX(r[0][1], __MAX(r[1][1], r[2][1])))
  }};

  int dw = (int)ceil(fabsf(mm[1][0]) - mm[0][0]);
  int dh = (int)ceil(fabsf(mm[1][1]) - mm[0][1]);
  if (!surface(b, dw, dh))
    return false;

  int x, y, sx, sy;
  for (x = 0; x < dw; ++x)
    for (y = 0; y < dh; ++y) {
      sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
      sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
      if (sx < 0 || sx >= a->w || sy < 0 || sy >= a->h)
        continue;
      pset(b, x, y, pget(a, sx, sy));
    }
  return true;
}

static inline void vline(struct surface_t* s, int x, int y0, int y1, int col) {
  if (y1 < y0) {
    y0 += y1;
    y1  = y0 - y1;
    y0 -= y1;
  }

  if (x < 0 || x >= s->w || y0 >= s->h)
    return;

  if (y0 < 0)
    y0 = 0;
  if (y1 >= s->h)
    y1 = s->h - 1;

  for(int y = y0; y <= y1; y++)
    pset(s, x, y, col);
}

static inline void hline(struct surface_t* s, int y, int x0, int x1, int col) {
  if (x1 < x0) {
    x0 += x1;
    x1  = x0 - x1;
    x0 -= x1;
  }

  if (y < 0 || y >= s->h || x0 >= s->w)
    return;

  if (x0 < 0)
    x0 = 0;
  if (x1 >= s->w)
    x1 = s->w - 1;

  for(int x = x0; x <= x1; x++)
    pset(s, x, y, col);
}

void line(struct surface_t* s, int x0, int y0, int x1, int y1, int col) {
  if (x0 == x1)
    vline(s, x0, y0, y1, col);
  if (y0 == y1)
    hline(s, y0, x0, x1, col);
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = (dx > dy ? dx : -dy) / 2;

  while (pset(s, x0, y0, col), x0 != x1 || y0 != y1) {
    int e2 = err;
    if (e2 > -dx) { err -= dy; x0 += sx; }
    if (e2 <  dy) { err += dx; y0 += sy; }
  }
}

void circle(struct surface_t* s, int xc, int yc, int r, int col, bool fill) {
  int x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
  do {
    pset(s, xc - x, yc + y, col);    /*   I. Quadrant */
    pset(s, xc - y, yc - x, col);    /*  II. Quadrant */
    pset(s, xc + x, yc - y, col);    /* III. Quadrant */
    pset(s, xc + y, yc + x, col);    /*  IV. Quadrant */

    if (fill) {
      hline(s, yc - y, xc - x, xc + x, col);
      hline(s, yc + y, xc - x, xc + x, col);
    }

    r = err;
    if (r <= y)
      err += ++y * 2 + 1; /* e_xy+e_y < 0 */
    if (r > x || err > y)
      err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
  } while (x < 0);
}

void rect(struct surface_t* s, int x, int y, int w, int h, int col, bool fill) {
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
    return;

  if (w > s->w)
    w = s->w;
  if (h > s->h)
    h = s->h;

  if (fill) {
    for (; y < h; ++y)
      hline(s, y, x, w, col);
  } else {
    hline(s, y, x, w, col);
    hline(s, h, x, w, col);
    vline(s, x, y, h, col);
    vline(s, w, y, h, col);
  }
}

#define GRAPHICS_SWAP(a, b) \
  do {                 \
    int temp = a;      \
    a = b;             \
    b = temp;          \
  } while(0)

void tri(struct surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int col, bool fill) {
  if (y0 ==  y1 && y0 ==  y2)
    return;
  if (fill) {
    if (y0 > y1) {
      GRAPHICS_SWAP(x0, x1);
      GRAPHICS_SWAP(y0, y1);
    }
    if (y0 > y2) {
      GRAPHICS_SWAP(x0, x2);
      GRAPHICS_SWAP(y0, y2);
    }
    if (y1 > y2) {
      GRAPHICS_SWAP(x1, x2);
      GRAPHICS_SWAP(y1, y2);
    }

    int total_height = y2 - y0, i, j;
    for (i = 0; i < total_height; ++i) {
      bool second_half = i > y1 - y0 || y1 == y0;
      int segment_height = second_half ? y2 - y1 : y1 - y0;
      float alpha = (float)i / total_height;
      float beta  = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;
      int ax = x0 + (x2 - x0) * alpha;
      int ay = y0 + (y2 - y0) * alpha;
      int bx = second_half ? x1 + (x2 - x1) : x0 + (x1 - x0) * beta;
      int by = second_half ? y1 + (y2 - y1) : y0 + (y1 - y0) * beta;
      if (ax > bx) {
        GRAPHICS_SWAP(ax, bx);
        GRAPHICS_SWAP(ay, by);
      }
      for (j = ax; j <= bx; ++j)
        pset(s, j, y0 + i, col);
    }
  } else {
    line(s, x0, y0, x1, y1, col);
    line(s, x1, y1, x2, y2, col);
    line(s, x2, y2, x0, y0, col);
  }
}

typedef struct {
//unsigned short type; /* Magic identifier */
  unsigned int size; /* File size in bytes */
  unsigned int reserved;
  unsigned int offset; /* Offset to image data, bytes */
} BMPHEADER;

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

bool bmp(struct surface_t* s, const char* path) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    GRAPHICS_ERROR(FILE_OPEN_FAILED, "fopen() failed: %s", path);
    return false;
  }

  fseek(fp, 0, SEEK_END);
  size_t length = ftell(fp);
  rewind(fp);

  unsigned char* data = GRAPHICS_MALLOC((length + 1) * sizeof(unsigned char));
  if (!data) {
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  fread(data, 1, length, fp);
  fclose(fp);

  if (data[0] != 0x42 || data[1] != 0x4D) {
    GRAPHICS_ERROR(INVALID_BMP, "bmp() failed: invalid BMP signiture '0x%x%x'", data[1], data[0]);
    return false;
  }

  int off = sizeof(unsigned short);
  BMPHEADER header;
  BMPINFOHEADER info;
  //BMPCOREHEADER core;
  BMP_GET(&header, data, sizeof(BMPHEADER));
  //int info_pos = off;
  BMP_GET(&info, data, sizeof(BMPINFOHEADER));

#pragma message WARN("TODO: bmp() add support for OS/2 bitmaps")
#pragma message WARN("TODO: bmp() add RLE support")
#pragma message WARN("TODO: bmp() add BITFIELDS support")
#pragma message WARN("TODO: bmp() add 1, 4 & 8 bpp support")

  unsigned char* color_map = NULL;
  int color_map_size = 0;
  if (info.bits <= 8) {
    color_map_size = (1 << info.bits) * 4;
    color_map = GRAPHICS_MALLOC(color_map_size * sizeof(unsigned char));
    if (!color_map) {
      GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
      return false;
    }
    BMP_GET(color_map, data, color_map_size);
  }

  if (!surface(s, info.width, info.height)) {
    GRAPHICS_SAFE_FREE(color_map);
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  off = header.offset;
  int i, p, x, y, sz = info.width * info.height;
  unsigned char color;
  switch (info.compression) {
    case 0: // RGB
      switch (info.bits) { // BPP
        case 1:
        case 4:
        case 8:
          GRAPHICS_ERROR(UNSUPPORTED_BMP, "bmp() failed. Unsupported BPP: %d", info.bits);
          surface_destroy(s);
          break;
        case 24: {
          int pad = (4 - (info.width * 3) % 4) % 4;
          for (int j = info.height; j; --j, off += pad)
            for (int i = 0; i < info.width; ++i, off += 3)
              pset(s, i, j, rgb(data[off + 2], data[off + 1], data[off]));
          break;
        }
        case 32:
          for (int j = info.height; j; --j)
            for (int i = 0; i < info.width; ++i, off += 4)
              pset(s, i, j, rgba(data[off + 2], data[off + 1], data[off], 255 - data[off + 3]));
          break;
        default:
          GRAPHICS_ERROR(UNSUPPORTED_BMP, "bmp() failed. Unsupported BPP: %d", info.bits);
          GRAPHICS_SAFE_FREE(color_map);
          surface_destroy(s);
          return false;
      }
      break;
    case 1: // RLE8
    case 2: // RLE4
    case 3: // BITFIELDS
    case 6: // BI_ALHPABITFIELDS
    default:
      GRAPHICS_ERROR(UNSUPPORTED_BMP, "bmp() failed. Unsupported compression: %d", info.compression);
      GRAPHICS_SAFE_FREE(color_map);
      surface_destroy(s);
      return false;
  }

  GRAPHICS_SAFE_FREE(color_map);
  return true;
}

bool save_bmp(struct surface_t* s, const char* path) {
  const int filesize = 54 + 3 * s->w * s->h;
  unsigned char* img = GRAPHICS_MALLOC(sizeof(unsigned char) * 3 * s->w * s->h);
  if (!img) {
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  memset(img, 0, 3 * s->w * s->h);

  int i, j, y, c;
  for (i = 0; i < s->w; ++i) {
    for (j = s->h; j > 0; --j) {
      y = (s->h - 1) - j;
      c = pget(s, i, y);
      img[(i + y * s->w) * 3 + 2] = (unsigned char)r_channel(c);
      img[(i + y * s->w) * 3 + 1] = (unsigned char)g_channel(c);
      img[(i + y * s->w) * 3 + 0] = (unsigned char)b_channel(c);
    }
  }

  unsigned char header[14] = {
    'B', 'M',
    0,  0, 0, 0,
    0,  0,
    0,  0,
    54, 0, 0, 0
  };
  unsigned char info[40] = {
    40, 0, 0, 0,
    0,  0, 0, 0,
    0,  0, 0, 0,
    1,  0,
    24, 0
  };
  unsigned char pad[3] = { 0, 0, 0 };

  header[2]  = (unsigned char)(filesize);
  header[3]  = (unsigned char)(filesize >> 8);
  header[4]  = (unsigned char)(filesize >> 16);
  header[5]  = (unsigned char)(filesize >> 24);

  info[4]  = (unsigned char)(s->w);
  info[5]  = (unsigned char)(s->w >> 8);
  info[6]  = (unsigned char)(s->w >> 16);
  info[7]  = (unsigned char)(s->w >> 24);
  info[8]  = (unsigned char)(s->h);
  info[9]  = (unsigned char)(s->h >> 8);
  info[10] = (unsigned char)(s->h >> 16);
  info[11] = (unsigned char)(s->h >> 24);

  FILE* fp = fopen(path, "wb");
  if (!fp) {
    GRAPHICS_ERROR(FILE_OPEN_FAILED, "fopen() failed: %s", path);
    GRAPHICS_SAFE_FREE(img);
    return false;
  }

  int padding = (4 - (s->w * 3) % 4) % 4;
  fwrite(header, 1, 14, fp);
  fwrite(info, 1, 40, fp);
  for(i = 0; i < s->h; ++i) {
    fwrite(img + (s->w * (s->h - i - 1) * 3), 3, s->w, fp);
    fwrite(pad, 1, padding, fp);
  }
  
  fclose(fp);
  GRAPHICS_SAFE_FREE(img);
  return true;
}

static unsigned char font[540][8] = {
  // Latin 0 - 94
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0020 (space)
  { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00 },   // U+0021 (!)
  { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0022 (")
  { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00 },   // U+0023 (#)
  { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00 },   // U+0024 ($)
  { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00 },   // U+0025 (%)
  { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00 },   // U+0026 (&)
  { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0027 (')
  { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00 },   // U+0028 (()
  { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00 },   // U+0029 ())
  { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00 },   // U+002A (*)
  { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00 },   // U+002B (+)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06 },   // U+002C (,)
  { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00 },   // U+002D (-)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00 },   // U+002E (.)
  { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00 },   // U+002F (/)
  { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00 },   // U+0030 (0)
  { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00 },   // U+0031 (1)
  { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00 },   // U+0032 (2)
  { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00 },   // U+0033 (3)
  { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00 },   // U+0034 (4)
  { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00 },   // U+0035 (5)
  { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00 },   // U+0036 (6)
  { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00 },   // U+0037 (7)
  { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00 },   // U+0038 (8)
  { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00 },   // U+0039 (9)
  { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00 },   // U+003A (:)
  { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06 },   // U+003B (//)
  { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00 },   // U+003C (<)
  { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00 },   // U+003D (=)
  { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00 },   // U+003E (>)
  { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00 },   // U+003F (?)
  { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00 },   // U+0040 (@)
  { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00 },   // U+0041 (A)
  { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00 },   // U+0042 (B)
  { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00 },   // U+0043 (C)
  { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00 },   // U+0044 (D)
  { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00 },   // U+0045 (E)
  { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00 },   // U+0046 (F)
  { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00 },   // U+0047 (G)
  { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00 },   // U+0048 (H)
  { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+0049 (I)
  { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00 },   // U+004A (J)
  { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00 },   // U+004B (K)
  { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00 },   // U+004C (L)
  { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00 },   // U+004D (M)
  { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00 },   // U+004E (N)
  { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00 },   // U+004F (O)
  { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00 },   // U+0050 (P)
  { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00 },   // U+0051 (Q)
  { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00 },   // U+0052 (R)
  { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00 },   // U+0053 (S)
  { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+0054 (T)
  { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00 },   // U+0055 (U)
  { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00 },   // U+0056 (V)
  { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00 },   // U+0057 (W)
  { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00 },   // U+0058 (X)
  { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00 },   // U+0059 (Y)
  { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00 },   // U+005A (Z)
  { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00 },   // U+005B ([)
  { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00 },   // U+005C (\)
  { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00 },   // U+005D (])
  { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00 },   // U+005E (^)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF },   // U+005F (_)
  { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+0060 (`)
  { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00 },   // U+0061 (a)
  { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00 },   // U+0062 (b)
  { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00 },   // U+0063 (c)
  { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00 },   // U+0064 (d)
  { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00 },   // U+0065 (e)
  { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00 },   // U+0066 (f)
  { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F },   // U+0067 (g)
  { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00 },   // U+0068 (h)
  { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+0069 (i)
  { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E },   // U+006A (j)
  { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00 },   // U+006B (k)
  { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+006C (l)
  { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00 },   // U+006D (m)
  { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00 },   // U+006E (n)
  { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00 },   // U+006F (o)
  { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F },   // U+0070 (p)
  { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78 },   // U+0071 (q)
  { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00 },   // U+0072 (r)
  { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00 },   // U+0073 (s)
  { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00 },   // U+0074 (t)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00 },   // U+0075 (u)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00 },   // U+0076 (v)
  { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00 },   // U+0077 (w)
  { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00 },   // U+0078 (x)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F },   // U+0079 (y)
  { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00 },   // U+007A (z)
  { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00 },   // U+007B ({)
  { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 },   // U+007C (|)
  { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00 },   // U+007D (})
  { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+007E (~)

  // Block 95 - 126
  { 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 },   // U+2580 (top half)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF },   // U+2581 (box 1/8)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF },   // U+2582 (box 2/8)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF },   // U+2583 (box 3/8)
  { 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF },   // U+2584 (bottom half)
  { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },   // U+2585 (box 5/8)
  { 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },   // U+2586 (box 6/8)
  { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },   // U+2587 (box 7/8)
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },   // U+2588 (solid)
  { 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F },   // U+2589 (box 7/8)
  { 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F },   // U+258A (box 6/8)
  { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },   // U+258B (box 5/8)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F },   // U+258C (left half)
  { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 },   // U+258D (box 3/8)
  { 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03 },   // U+258E (box 2/8)
  { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },   // U+258F (box 1/8)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0 },   // U+2590 (right half)
  { 0x55, 0x00, 0xAA, 0x00, 0x55, 0x00, 0xAA, 0x00 },   // U+2591 (25% solid)
  { 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA },   // U+2592 (50% solid)
  { 0xFF, 0xAA, 0xFF, 0x55, 0xFF, 0xAA, 0xFF, 0x55 },   // U+2593 (75% solid)
  { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+2594 (box 1/8)
  { 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 },   // U+2595 (box 1/8)
  { 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x0F, 0x0F },   // U+2596 (box bottom left)
  { 0x00, 0x00, 0x00, 0x00, 0xF0, 0xF0, 0xF0, 0xF0 },   // U+2597 (box bottom right)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00 },   // U+2598 (box top left)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF },   // U+2599 (boxes left and bottom)
  { 0x0F, 0x0F, 0x0F, 0x0F, 0xF0, 0xF0, 0xF0, 0xF0 },   // U+259A (boxes top-left and bottom right)
  { 0xFF, 0xFF, 0xFF, 0xFF, 0x0F, 0x0F, 0x0F, 0x0F },   // U+259B (boxes top and left)
  { 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xF0, 0xF0, 0xF0 },   // U+259C (boxes top and right)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0x00, 0x00, 0x00, 0x00 },   // U+259D (box top right)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0x0F, 0x0F, 0x0F, 0x0F },   // U+259E (boxes top right and bottom left)
  { 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF },   // U+259F (boxes right and bottom)

  // Box 127 - 254
  { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00 },   // U+2500 (thin horizontal)
  { 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00 },   // U+2501 (thick horizontal)
  { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 },   // U+2502 (thin vertical)
  { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 },   // U+2503 (thich vertical)
  { 0x00, 0x00, 0x00, 0x00, 0xBB, 0x00, 0x00, 0x00 },   // U+2504 (thin horizontal dashed)
  { 0x00, 0x00, 0x00, 0xBB, 0xBB, 0x00, 0x00, 0x00 },   // U+2505 (thick horizontal dashed)
  { 0x08, 0x00, 0x08, 0x08, 0x08, 0x00, 0x08, 0x08 },   // U+2506 (thin vertical dashed)
  { 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18 },   // U+2507 (thich vertical dashed)
  { 0x00, 0x00, 0x00, 0x00, 0x55, 0x00, 0x00, 0x00 },   // U+2508 (thin horizontal dotted)
  { 0x00, 0x00, 0x00, 0x55, 0x55, 0x00, 0x00, 0x00 },   // U+2509 (thick horizontal dotted)
  { 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08 },   // U+250A (thin vertical dotted)
  { 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18 },   // U+250B (thich vertical dotted)
  { 0x00, 0x00, 0x00, 0x00, 0xf8, 0x08, 0x08, 0x08 },   // U+250C (down L, right L)
  { 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x08, 0x08, 0x08 },   // U+250D (down L, right H)
  { 0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0x18, 0x18 },   // U+250E (down H, right L)
  { 0x00, 0x00, 0x00, 0xf8, 0xf8, 0x18, 0x18, 0x18 },   // U+250F (down H, right H)
  { 0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08, 0x08 },   // U+2510 (down L, left L)
  { 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x08, 0x08, 0x08 },   // U+2511 (down L, left H)
  { 0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x18, 0x18 },   // U+2512 (down H, left L)
  { 0x00, 0x00, 0x00, 0x1f, 0x1f, 0x18, 0x18, 0x18 },   // U+2513 (down H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x00, 0x00, 0x00 },   // U+2514 (up L, right L)
  { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x00, 0x00, 0x00 },   // U+2515 (up L, right H)
  { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x00, 0x00, 0x00 },   // U+2516 (up H, right L)
  { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x00, 0x00, 0x00 },   // U+2517 (up H, right H)
  { 0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00 },   // U+2518 (up L, left L)
  { 0x08, 0x08, 0x08, 0x0f, 0x0f, 0x00, 0x00, 0x00 },   // U+2519 (up L, left H)
  { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x00, 0x00, 0x00 },   // U+251A (up H, left L)
  { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x00, 0x00, 0x00 },   // U+251B (up H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x08, 0x08, 0x08 },   // U+251C (down L, right L, up L)
  { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x08, 0x08, 0x08 },   // U+251D (down L, right H, up L)
  { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x08, 0x08, 0x08 },   // U+251E (down L, right L, up H)
  { 0x08, 0x08, 0x08, 0x08, 0xf8, 0x18, 0x18, 0x18 },   // U+251F (down H, right L, up L)
  { 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0x18, 0x18 },   // U+2520 (down H, right L, up H)
  { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x08, 0x08, 0x08 },   // U+2521 (down L, right H, up H)
  { 0x08, 0x08, 0x08, 0xf8, 0xf8, 0x18, 0x18, 0x18 },   // U+2522 (down H, right H, up L)
  { 0x18, 0x18, 0x18, 0xf8, 0xf8, 0x18, 0x18, 0x18 },   // U+2523 (down H, right H, up H)
  { 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08 },   // U+2524 (down L, left L, up L)
  { 0x08, 0x08, 0x08, 0x0f, 0x0f, 0x08, 0x08, 0x08 },   // U+2525 (down L, left H, up L)
  { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x08, 0x08, 0x08 },   // U+2526 (down L, left L, up H)
  { 0x08, 0x08, 0x08, 0x08, 0x1f, 0x18, 0x18, 0x18 },   // U+2527 (down H, left L, up L)
  { 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x18, 0x18 },   // U+2528 (down H, left L, up H)
  { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x08, 0x08, 0x08 },   // U+2529 (down L, left H, up H)
  { 0x08, 0x08, 0x08, 0x1f, 0x1f, 0x18, 0x18, 0x18 },   // U+252A (down H, left H, up L)
  { 0x18, 0x18, 0x18, 0x1f, 0x1f, 0x18, 0x18, 0x18 },   // U+252B (down H, left H, up H)
  { 0x00, 0x00, 0x00, 0x00, 0xff, 0x08, 0x08, 0x08 },   // U+252C (down L, right L, left L)
  { 0x00, 0x00, 0x00, 0x0f, 0xff, 0x08, 0x08, 0x08 },   // U+252D (down L, right L, left H)
  { 0x00, 0x00, 0x00, 0xf8, 0xff, 0x08, 0x08, 0x08 },   // U+252E (down L, right H, left L)
  { 0x00, 0x00, 0x00, 0xff, 0xff, 0x08, 0x08, 0x08 },   // U+252F (down L, right H, left H)
  { 0x00, 0x00, 0x00, 0x00, 0xff, 0x18, 0x18, 0x18 },   // U+2530 (down H, right L, left L)
  { 0x00, 0x00, 0x00, 0x1f, 0xff, 0x18, 0x18, 0x18 },   // U+2531 (down H, right L, left H)
  { 0x00, 0x00, 0x00, 0xf8, 0xff, 0x18, 0x18, 0x18 },   // U+2532 (down H, right H, left L)
  { 0x00, 0x00, 0x00, 0xff, 0xff, 0x18, 0x18, 0x18 },   // U+2533 (down H, right H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xff, 0x00, 0x00, 0x00 },   // U+2534 (up L, right L, left L)
  { 0x08, 0x08, 0x08, 0x0f, 0xff, 0x00, 0x00, 0x00 },   // U+2535 (up L, right L, left H)
  { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x00, 0x00, 0x00 },   // U+2536 (up L, right H, left L)
  { 0x08, 0x08, 0x08, 0xff, 0xff, 0x00, 0x00, 0x00 },   // U+2537 (up L, right H, left H)
  { 0x18, 0x18, 0x18, 0x18, 0xff, 0x00, 0x00, 0x00 },   // U+2538 (up H, right L, left L)
  { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x00, 0x00, 0x00 },   // U+2539 (up H, right L, left H)
  { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x00, 0x00, 0x00 },   // U+253A (up H, right H, left L)
  { 0x18, 0x18, 0x18, 0xff, 0xff, 0x00, 0x00, 0x00 },   // U+253B (up H, right H, left H)
  { 0x08, 0x08, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08 },   // U+253C (up L, right L, left L, down L)
  { 0x08, 0x08, 0x08, 0x0f, 0xff, 0x08, 0x08, 0x08 },   // U+253D (up L, right L, left H, down L)
  { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x08, 0x08, 0x08 },   // U+253E (up L, right H, left L, down L)
  { 0x08, 0x08, 0x08, 0xff, 0xff, 0x08, 0x08, 0x08 },   // U+253F (up L, right H, left H, down L)
  { 0x18, 0x18, 0x18, 0x18, 0xff, 0x08, 0x08, 0x08 },   // U+2540 (up H, right L, left L, down L)
  { 0x08, 0x08, 0x08, 0x08, 0xff, 0x18, 0x18, 0x18 },   // U+2541 (up L, right L, left L, down H)
  { 0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18 },   // U+2542 (up H, right L, left L, down H)
  { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x08, 0x08, 0x08 },   // U+2543 (up H, right L, left H, down L)
  { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x08, 0x08, 0x08 },   // U+2544 (up H, right H, left L, down L)
  { 0x08, 0x08, 0x08, 0x1f, 0xff, 0x18, 0x18, 0x18 },   // U+2545 (up L, right L, left H, down H)
  { 0x08, 0x08, 0x08, 0xf8, 0xff, 0x18, 0x18, 0x18 },   // U+2546 (up L, right H, left L, down H)
  { 0x08, 0x08, 0x08, 0xff, 0xff, 0x18, 0x18, 0x18 },   // U+2547 (up L, right H, left H, down H)
  { 0x18, 0x18, 0x18, 0xff, 0xff, 0x08, 0x08, 0x08 },   // U+254B (up H, right H, left H, down L)
  { 0x18, 0x18, 0x18, 0xf8, 0xff, 0x18, 0x18, 0x18 },   // U+254A (up H, right H, left L, down H)
  { 0x18, 0x18, 0x18, 0x1f, 0xff, 0x18, 0x18, 0x18 },   // U+2549 (up H, right L, left H, down H)
  { 0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18 },   // U+254B (up H, right H, left H, down H)
  { 0x00, 0x00, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00 },   // U+254C (thin horizontal broken)
  { 0x00, 0x00, 0x00, 0xE7, 0xE7, 0x00, 0x00, 0x00 },   // U+254D (thick horizontal broken)
  { 0x08, 0x08, 0x08, 0x00, 0x00, 0x08, 0x08, 0x08 },   // U+254E (thin vertical broken)
  { 0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18 },   // U+254F (thich vertical broken)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00 },   // U+2550 (double horizontal)
  { 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14 },   // U+2551 (double vertical)
  { 0x00, 0x00, 0x00, 0xF8, 0x08, 0xF8, 0x08, 0x08 },   // U+2552 (down L, right D)
  { 0x00, 0x00, 0x00, 0x00, 0xFC, 0x14, 0x14, 0x14 },   // U+2553 (down D, right L)
  { 0x00, 0x00, 0x00, 0xFC, 0x04, 0xF4, 0x14, 0x14 },   // U+2554 (down D, right D)
  { 0x00, 0x00, 0x00, 0x0F, 0x08, 0x0F, 0x08, 0x08 },   // U+2555 (down L, left D)
  { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x14, 0x14, 0x14 },   // U+2556 (down D, left L)
  { 0x00, 0x00, 0x00, 0x1F, 0x10, 0x17, 0x14, 0x14 },   // U+2557 (down D, left D)
  { 0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x00, 0x00 },   // U+2558 (up L, right D)
  { 0x14, 0x14, 0x14, 0x14, 0xFC, 0x00, 0x00, 0x00 },   // U+2559 (up D, right L)
  { 0x14, 0x14, 0x14, 0xF4, 0x04, 0xFC, 0x00, 0x00 },   // U+255A (up D, right D)
  { 0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x00, 0x00 },   // U+255B (up L, left D)
  { 0x14, 0x14, 0x14, 0x14, 0x1F, 0x00, 0x00, 0x00 },   // U+255C (up D, left L)
  { 0x14, 0x14, 0x14, 0x17, 0x10, 0x1F, 0x00, 0x00 },   // U+255D (up D, left D)
  { 0x08, 0x08, 0x08, 0xF8, 0x08, 0xF8, 0x08, 0x08 },   // U+255E (up L, down L, right D)
  { 0x14, 0x14, 0x14, 0x14, 0xF4, 0x14, 0x14, 0x14 },   // U+255F (up D, down D, right L)
  { 0x14, 0x14, 0x14, 0xF4, 0x04, 0xF4, 0x14, 0x14 },   // U+2560 (up D, down D, right D)
  { 0x08, 0x08, 0x08, 0x0F, 0x08, 0x0F, 0x08, 0x08 },   // U+2561 (up L, down L, left D)
  { 0x14, 0x14, 0x14, 0x14, 0x17, 0x14, 0x14, 0x14 },   // U+2562 (up D, down D, left L)
  { 0x14, 0x14, 0x14, 0x17, 0x10, 0x17, 0x14, 0x14 },   // U+2563 (up D, down D, left D)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x08, 0x08 },   // U+2564 (left D, right D, down L)
  { 0x00, 0x00, 0x00, 0x00, 0xFF, 0x14, 0x14, 0x14 },   // U+2565 (left L, right L, down D)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xF7, 0x14, 0x14 },   // U+2566 (left D, right D, down D)
  { 0x08, 0x08, 0x08, 0xFF, 0x00, 0xFF, 0x00, 0x00 },   // U+2567 (left D, right D, up L)
  { 0x14, 0x14, 0x14, 0x14, 0xFF, 0x00, 0x00, 0x00 },   // U+2568 (left L, right L, up D)
  { 0x14, 0x14, 0x14, 0xF7, 0x00, 0xFF, 0x00, 0x00 },   // U+2569 (left D, right D, up D)
  { 0x08, 0x08, 0x08, 0xFF, 0x08, 0xFF, 0x08, 0x08 },   // U+256A (left D, right D, down L, up L)
  { 0x14, 0x14, 0x14, 0x14, 0xFF, 0x14, 0x14, 0x14 },   // U+256B (left L, right L, down D, up D)
  { 0x14, 0x14, 0x14, 0xF7, 0x00, 0xF7, 0x14, 0x14 },   // U+256C (left D, right D, down D, up D)
  { 0x00, 0x00, 0x00, 0x00, 0xE0, 0x10, 0x08, 0x08 },   // U+256D (curve down-right)
  { 0x00, 0x00, 0x00, 0x00, 0x03, 0x04, 0x08, 0x08 },   // U+256E (curve down-left)
  { 0x08, 0x08, 0x08, 0x04, 0x03, 0x00, 0x00, 0x00 },   // U+256F (curve up-left)
  { 0x08, 0x08, 0x08, 0x10, 0xE0, 0x00, 0x00, 0x00 },   // U+2570 (curve up-right)
  { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 },   // U+2571 (diagonal bottom-left to top-right)
  { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 },   // U+2572 (diagonal bottom-right to top-left)
  { 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 },   // U+2573 (diagonal cross)
  { 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00 },   // U+2574 (left L)
  { 0x08, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00 },   // U+2575 (up L)
  { 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00 },   // U+2576 (right L)
  { 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08 },   // U+2577 (down L)
  { 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00 },   // U+2578 (left H)
  { 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00 },   // U+2579 (up H)
  { 0x00, 0x00, 0x00, 0xF8, 0xF8, 0x00, 0x00, 0x00 },   // U+257A (right H)
  { 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18 },   // U+257B (down H)
  { 0x00, 0x00, 0x00, 0xF8, 0xFF, 0x00, 0x00, 0x00 },   // U+257C (right H, left L)
  { 0x08, 0x08, 0x08, 0x08, 0x18, 0x18, 0x18, 0x18 },   // U+257D (up L, down H)
  { 0x00, 0x00, 0x00, 0x0F, 0xFF, 0x00, 0x00, 0x00 },   // U+257E (right L, left H)
  { 0x18, 0x18, 0x18, 0x18, 0x08, 0x08, 0x08, 0x08 },   // U+257F (up H, down L)

  // Greek 255 - 312
  { 0x2D, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00 },   // U+0390 (iota with tonos and diaeresis)
  { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00 },   // U+0391 (Alpha)
  { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00 },   // U+0392 (Beta)
  { 0x3F, 0x33, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00 },   // U+0393 (Gamma)
  { 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x63, 0x7F, 0x00 },   // U+0394 (Delta)
  { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00 },   // U+0395 (Epsilon)
  { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00 },   // U+0396 (Zeta)
  { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00 },   // U+0397 (Eta)
  { 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x36, 0x1C, 0x00 },   // U+0398 (Theta)
  { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+0399 (Iota)
  { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00 },   // U+039A (Kappa)
  { 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x63, 0x63, 0x00 },   // U+039B (Lambda)
  { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00 },   // U+039C (Mu)
  { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00 },   // U+039D (Nu)
  { 0x7F, 0x63, 0x00, 0x3E, 0x00, 0x63, 0x7F, 0x00 },   // U+039E (Xi)
  { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00 },   // U+039F (Omikron)
  { 0x7F, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00 },   // U+03A0 (Pi)
  { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00 },   // U+03A1 (Rho)
  { 0x00, 0x01, 0x02, 0x04, 0x4F, 0x90, 0xA0, 0x40 },   // U+03A2
  { 0x7F, 0x63, 0x06, 0x0C, 0x06, 0x63, 0x7F, 0x00 },   // U+03A3 (Sigma 2)
  { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+03A4 (Tau)
  { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00 },   // U+03A5 (Upsilon)
  { 0x18, 0x7E, 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x00 },   // U+03A6 (Phi)
  { 0x63, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x63, 0x00 },   // U+03A7 (Chi)
  { 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x18, 0x3C, 0x00 },   // U+03A8 (Psi)
  { 0x3E, 0x63, 0x63, 0x63, 0x36, 0x36, 0x77, 0x00 },   // U+03A9 (Omega)
  { 0x33, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+0399 (Iota with diaeresis)
  { 0x33, 0x00, 0x33, 0x33, 0x1E, 0x0C, 0x1E, 0x00 },   // U+03A5 (Upsilon with diaeresis)
  { 0x70, 0x00, 0x6E, 0x3B, 0x13, 0x3B, 0x6E, 0x00 },   // U+03AC (alpha aigu)
  { 0x38, 0x00, 0x1E, 0x03, 0x0E, 0x03, 0x1E, 0x00 },   // U+03AD (epsilon aigu)
  { 0x38, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x30 },   // U+03AE (eta aigu)
  { 0x38, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00 },   // U+03AF (iota aigu)
  { 0x2D, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00 },   // U+03B0 (upsilon with tonos and diaeresis)
  { 0x00, 0x00, 0x6E, 0x3B, 0x13, 0x3B, 0x6E, 0x00 },   // U+03B1 (alpha)
  { 0x00, 0x1E, 0x33, 0x1F, 0x33, 0x1F, 0x03, 0x03 },   // U+03B2 (beta)
  { 0x00, 0x00, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x00 },   // U+03B3 (gamma)
  { 0x38, 0x0C, 0x18, 0x3E, 0x33, 0x33, 0x1E, 0x00 },   // U+03B4 (delta)
  { 0x00, 0x00, 0x1E, 0x03, 0x0E, 0x03, 0x1E, 0x00 },   // U+03B5 (epsilon)
  { 0x00, 0x3F, 0x06, 0x03, 0x03, 0x1E, 0x30, 0x1C },   // U+03B6 (zeta)
  { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x30 },   // U+03B7 (eta)
  { 0x00, 0x00, 0x1E, 0x33, 0x3F, 0x33, 0x1E, 0x00 },   // U+03B8 (theta)
  { 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x2C, 0x18, 0x00 },   // U+03B9 (iota)
  { 0x00, 0x00, 0x33, 0x1B, 0x0F, 0x1B, 0x33, 0x00 },   // U+03BA (kappa)
  { 0x00, 0x03, 0x06, 0x0C, 0x1C, 0x36, 0x63, 0x00 },   // U+03BB (lambda)
  { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x03 },   // U+03BC (mu)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00 },   // U+03BD (nu)
  { 0x1E, 0x03, 0x0E, 0x03, 0x03, 0x1E, 0x30, 0x1C },   // U+03BE (xi)
  { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00 },   // U+03BF (omikron)
  { 0x00, 0x00, 0x7F, 0x36, 0x36, 0x36, 0x36, 0x00 },   // U+03C0 (pi)
  { 0x00, 0x00, 0x3C, 0x66, 0x66, 0x36, 0x06, 0x06 },   // U+03C1 (rho)
  { 0x00, 0x00, 0x3E, 0x03, 0x03, 0x1E, 0x30, 0x1C },   // U+03C2 (sigma 1)
  { 0x00, 0x00, 0x7E, 0x1B, 0x1B, 0x1B, 0x0E, 0x00 },   // U+03C3 (sigma 2)
  { 0x00, 0x00, 0x7E, 0x18, 0x18, 0x58, 0x30, 0x00 },   // U+03C4 (tau)
  { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00 },   // U+03C5 (upsilon)
  { 0x00, 0x00, 0x76, 0xDB, 0xDB, 0x7E, 0x18, 0x00 },   // U+03C6 (phi)
  { 0x00, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00 },   // U+03C7 (chi)
  { 0x00, 0x00, 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x00 },   // U+03C8 (psi)
  { 0x00, 0x00, 0x36, 0x63, 0x6B, 0x7F, 0x36, 0x00 },   // U+03C9 (omega)

  // Hiragana 313 - 408
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+3040
  { 0x04, 0x3F, 0x04, 0x3C, 0x56, 0x4D, 0x26, 0x00 },   // U+3041 (Hiragana a)
  { 0x04, 0x3F, 0x04, 0x3C, 0x56, 0x4D, 0x26, 0x00 },   // U+3042 (Hiragana A)
  { 0x00, 0x00, 0x00, 0x11, 0x21, 0x25, 0x02, 0x00 },   // U+3043 (Hiragana i)
  { 0x00, 0x01, 0x11, 0x21, 0x21, 0x25, 0x02, 0x00 },   // U+3044 (Hiragana I)
  { 0x00, 0x1C, 0x00, 0x1C, 0x22, 0x20, 0x18, 0x00 },   // U+3045 (Hiragana u)
  { 0x3C, 0x00, 0x3C, 0x42, 0x40, 0x20, 0x18, 0x00 },   // U+3046 (Hiragana U)
  { 0x1C, 0x00, 0x3E, 0x10, 0x38, 0x24, 0x62, 0x00 },   // U+3047 (Hiragana e)
  { 0x1C, 0x00, 0x3E, 0x10, 0x38, 0x24, 0x62, 0x00 },   // U+3048 (Hiragana E)
  { 0x24, 0x4F, 0x04, 0x3C, 0x46, 0x45, 0x22, 0x00 },   // U+3049 (Hiragana o)
  { 0x24, 0x4F, 0x04, 0x3C, 0x46, 0x45, 0x22, 0x00 },   // U+304A (Hiragana O)
  { 0x04, 0x24, 0x4F, 0x54, 0x52, 0x12, 0x09, 0x00 },   // U+304B (Hiragana KA)
  { 0x44, 0x24, 0x0F, 0x54, 0x52, 0x52, 0x09, 0x00 },   // U+304C (Hiragana GA)
  { 0x08, 0x1F, 0x08, 0x3F, 0x1C, 0x02, 0x3C, 0x00 },   // U+304D (Hiragana KI)
  { 0x44, 0x2F, 0x04, 0x1F, 0x0E, 0x01, 0x1E, 0x00 },   // U+304E (Hiragana GI)
  { 0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x00 },   // U+304F (Hiragana KU)
  { 0x28, 0x44, 0x12, 0x21, 0x02, 0x04, 0x08, 0x00 },   // U+3050 (Hiragana GU)
  { 0x00, 0x22, 0x79, 0x21, 0x21, 0x22, 0x10, 0x00 },   // U+3051 (Hiragana KE)
  { 0x40, 0x22, 0x11, 0x3D, 0x11, 0x12, 0x08, 0x00 },   // U+3052 (Hiragana GE)
  { 0x00, 0x00, 0x3C, 0x00, 0x02, 0x02, 0x3C, 0x00 },   // U+3053 (Hiragana KO)
  { 0x20, 0x40, 0x16, 0x20, 0x01, 0x01, 0x0E, 0x00 },   // U+3054 (Hiragana GO)
  { 0x10, 0x7E, 0x10, 0x3C, 0x02, 0x02, 0x1C, 0x00 },   // U+3055 (Hiragana SA)
  { 0x24, 0x4F, 0x14, 0x2E, 0x01, 0x01, 0x0E, 0x00 },   // U+3056 (Hiragana ZA)
  { 0x00, 0x02, 0x02, 0x02, 0x42, 0x22, 0x1C, 0x00 },   // U+3057 (Hiragana SI)
  { 0x20, 0x42, 0x12, 0x22, 0x02, 0x22, 0x1C, 0x00 },   // U+3058 (Hiragana ZI)
  { 0x10, 0x7E, 0x18, 0x14, 0x18, 0x10, 0x0C, 0x00 },   // U+3059 (Hiragana SU)
  { 0x44, 0x2F, 0x06, 0x05, 0x06, 0x04, 0x03, 0x00 },   // U+305A (Hiragana ZU)
  { 0x20, 0x72, 0x2F, 0x22, 0x1A, 0x02, 0x1C, 0x00 },   // U+305B (Hiragana SE)
  { 0x80, 0x50, 0x3A, 0x17, 0x1A, 0x02, 0x1C, 0x00 },   // U+305C (Hiragana ZE)
  { 0x1E, 0x08, 0x04, 0x7F, 0x08, 0x04, 0x38, 0x00 },   // U+305D (Hiragana SO)
  { 0x4F, 0x24, 0x02, 0x7F, 0x08, 0x04, 0x38, 0x00 },   // U+305E (Hiragana ZO)
  { 0x02, 0x0F, 0x02, 0x72, 0x02, 0x09, 0x71, 0x00 },   // U+305F (Hiragana TA)
  { 0x42, 0x2F, 0x02, 0x72, 0x02, 0x09, 0x71, 0x00 },   // U+3060 (Hiragana DA)
  { 0x08, 0x7E, 0x08, 0x3C, 0x40, 0x40, 0x38, 0x00 },   // U+3061 (Hiragana TI)
  { 0x44, 0x2F, 0x04, 0x1E, 0x20, 0x20, 0x1C, 0x00 },   // U+3062 (Hiragana DI)
  { 0x00, 0x00, 0x00, 0x1C, 0x22, 0x20, 0x1C, 0x00 },   // U+3063 (Hiragana tu)
  { 0x00, 0x1C, 0x22, 0x41, 0x40, 0x20, 0x1C, 0x00 },   // U+3064 (Hiragana TU)
  { 0x40, 0x20, 0x1E, 0x21, 0x20, 0x20, 0x1C, 0x00 },   // U+3065 (Hiragana DU)
  { 0x00, 0x3E, 0x08, 0x04, 0x04, 0x04, 0x38, 0x00 },   // U+3066 (Hiragana TE)
  { 0x00, 0x3E, 0x48, 0x24, 0x04, 0x04, 0x38, 0x00 },   // U+3067 (Hiragana DE)
  { 0x04, 0x04, 0x08, 0x3C, 0x02, 0x02, 0x3C, 0x00 },   // U+3068 (Hiragana TO)
  { 0x44, 0x24, 0x08, 0x3C, 0x02, 0x02, 0x3C, 0x00 },   // U+3069 (Hiragana DO)
  { 0x32, 0x02, 0x27, 0x22, 0x72, 0x29, 0x11, 0x00 },   // U+306A (Hiragana NA)
  { 0x00, 0x02, 0x7A, 0x02, 0x0A, 0x72, 0x02, 0x00 },   // U+306B (Hiragana NI)
  { 0x08, 0x09, 0x3E, 0x4B, 0x65, 0x55, 0x22, 0x00 },   // U+306C (Hiragana NU)
  { 0x04, 0x07, 0x34, 0x4C, 0x66, 0x54, 0x24, 0x00 },   // U+306D (Hiragana NE)
  { 0x00, 0x00, 0x3C, 0x4A, 0x49, 0x45, 0x22, 0x00 },   // U+306E (Hiragana NO)
  { 0x00, 0x22, 0x7A, 0x22, 0x72, 0x2A, 0x12, 0x00 },   // U+306F (Hiragana HA)
  { 0x80, 0x51, 0x1D, 0x11, 0x39, 0x15, 0x09, 0x00 },   // U+3070 (Hiragana BA)
  { 0x40, 0xB1, 0x5D, 0x11, 0x39, 0x15, 0x09, 0x00 },   // U+3071 (Hiragana PA)
  { 0x00, 0x00, 0x13, 0x32, 0x51, 0x11, 0x0E, 0x00 },   // U+3072 (Hiragana HI)
  { 0x40, 0x20, 0x03, 0x32, 0x51, 0x11, 0x0E, 0x00 },   // U+3073 (Hiragana BI)
  { 0x40, 0xA0, 0x43, 0x32, 0x51, 0x11, 0x0E, 0x00 },   // U+3074 (Hiragana PI)
  { 0x1C, 0x00, 0x08, 0x2A, 0x49, 0x10, 0x0C, 0x00 },   // U+3075 (Hiragana HU)
  { 0x4C, 0x20, 0x08, 0x2A, 0x49, 0x10, 0x0C, 0x00 },   // U+3076 (Hiragana BU)
  { 0x4C, 0xA0, 0x48, 0x0A, 0x29, 0x48, 0x0C, 0x00 },   // U+3077 (Hiragana PU)
  { 0x00, 0x00, 0x04, 0x0A, 0x11, 0x20, 0x40, 0x00 },   // U+3078 (Hiragana HE)
  { 0x20, 0x40, 0x14, 0x2A, 0x11, 0x20, 0x40, 0x00 },   // U+3079 (Hiragana BE)
  { 0x20, 0x50, 0x24, 0x0A, 0x11, 0x20, 0x40, 0x00 },   // U+307A (Hiragana PE)
  { 0x7D, 0x11, 0x7D, 0x11, 0x39, 0x55, 0x09, 0x00 },   // U+307B (Hiragana HO)
  { 0x9D, 0x51, 0x1D, 0x11, 0x39, 0x55, 0x09, 0x00 },   // U+307C (Hiragana BO)
  { 0x5D, 0xB1, 0x5D, 0x11, 0x39, 0x55, 0x09, 0x00 },   // U+307D (Hiragana PO)
  { 0x7E, 0x08, 0x3E, 0x08, 0x1C, 0x2A, 0x04, 0x00 },   // U+307E (Hiragana MA)
  { 0x00, 0x07, 0x24, 0x24, 0x7E, 0x25, 0x12, 0x00 },   // U+307F (Hiragana MI)
  { 0x04, 0x0F, 0x64, 0x06, 0x05, 0x26, 0x3C, 0x00 },   // U+3080 (Hiragana MU)
  { 0x00, 0x09, 0x3D, 0x4A, 0x4B, 0x45, 0x2A, 0x00 },   // U+3081 (Hiragana ME)
  { 0x02, 0x0F, 0x02, 0x0F, 0x62, 0x42, 0x3C, 0x00 },   // U+3082 (Hiragana MO)
  { 0x00, 0x00, 0x12, 0x1F, 0x22, 0x12, 0x04, 0x00 },   // U+3083 (Hiragana ya)
  { 0x00, 0x12, 0x3F, 0x42, 0x42, 0x34, 0x04, 0x00 },   // U+3084 (Hiragana YA)
  { 0x00, 0x00, 0x11, 0x3D, 0x53, 0x39, 0x11, 0x00 },   // U+3085 (Hiragana yu)
  { 0x00, 0x11, 0x3D, 0x53, 0x51, 0x39, 0x11, 0x00 },   // U+3086 (Hiragana YU)
  { 0x00, 0x08, 0x38, 0x08, 0x1C, 0x2A, 0x04, 0x00 },   // U+3087 (Hiragana yo)
  { 0x08, 0x08, 0x38, 0x08, 0x1C, 0x2A, 0x04, 0x00 },   // U+3088 (Hiragana YO)
  { 0x1E, 0x00, 0x02, 0x3A, 0x46, 0x42, 0x30, 0x00 },   // U+3089 (Hiragana RA)
  { 0x00, 0x20, 0x22, 0x22, 0x2A, 0x24, 0x10, 0x00 },   // U+308A (Hiragana RI)
  { 0x1F, 0x08, 0x3C, 0x42, 0x49, 0x54, 0x38, 0x00 },   // U+308B (Hiragana RU)
  { 0x04, 0x07, 0x04, 0x0C, 0x16, 0x55, 0x24, 0x00 },   // U+308C (Hiragana RE)
  { 0x3F, 0x10, 0x08, 0x3C, 0x42, 0x41, 0x30, 0x00 },   // U+308D (Hiragana RO)
  { 0x00, 0x00, 0x08, 0x0E, 0x38, 0x4C, 0x2A, 0x00 },   // U+308E (Hiragana wa)
  { 0x04, 0x07, 0x04, 0x3C, 0x46, 0x45, 0x24, 0x00 },   // U+308F (Hiragana WA)
  { 0x0E, 0x08, 0x3C, 0x4A, 0x69, 0x55, 0x32, 0x00 },   // U+3090 (Hiragana WI)
  { 0x06, 0x3C, 0x42, 0x39, 0x04, 0x36, 0x49, 0x00 },   // U+3091 (Hiragana WE)
  { 0x04, 0x0F, 0x04, 0x6E, 0x11, 0x08, 0x70, 0x00 },   // U+3092 (Hiragana WO)
  { 0x08, 0x08, 0x04, 0x0C, 0x56, 0x52, 0x21, 0x00 },   // U+3093 (Hiragana N)
  { 0x40, 0x2E, 0x00, 0x3C, 0x42, 0x40, 0x38, 0x00 },   // U+3094 (Hiragana VU)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+3095
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+3096
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+3097
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+3098
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+3099 (voiced combinator mark)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+309A (semivoiced combinator mark)
  { 0x40, 0x80, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00 },   // U+309B (Hiragana voiced mark)
  { 0x40, 0xA0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+309C (Hiragana semivoiced mark)
  { 0x00, 0x00, 0x08, 0x08, 0x10, 0x30, 0x0C, 0x00 },   // U+309D (Hiragana iteration mark)
  { 0x20, 0x40, 0x14, 0x24, 0x08, 0x18, 0x06, 0x00 },   // U+309E (Hiragana voiced iteration mark)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+309F

  // SGA 409 - 434
  { 0x00, 0x00, 0x38, 0x66, 0x06, 0x06, 0x07, 0x00 },   // U+E541 (SGA A)
  { 0x00, 0x00, 0x0C, 0x0C, 0x18, 0x30, 0x7F, 0x00 },   // U+E542 (SGA B)
  { 0x00, 0x00, 0x0C, 0x00, 0x0C, 0x30, 0x30, 0x00 },   // U+E543 (SGA C)
  { 0x00, 0x00, 0x7F, 0x00, 0x03, 0x1C, 0x60, 0x00 },   // U+E544 (SGA D)
  { 0x00, 0x00, 0x63, 0x03, 0x03, 0x03, 0x7F, 0x00 },   // U+E545 (SGA E)
  { 0x00, 0x00, 0x00, 0xFF, 0x00, 0xDB, 0x00, 0x00 },   // U+E546 (SGA F)
  { 0x00, 0x00, 0x30, 0x30, 0x3E, 0x30, 0x30, 0x00 },   // U+E547 (SGA G)
  { 0x00, 0x00, 0x7E, 0x00, 0x7E, 0x18, 0x18, 0x00 },   // U+E548 (SGA H)
  { 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00 },   // U+E549 (SGA I)
  { 0x00, 0x00, 0x18, 0x00, 0x18, 0x00, 0x18, 0x00 },   // U+E54A (SGA J)
  { 0x00, 0x00, 0x18, 0x18, 0x5A, 0x18, 0x18, 0x00 },   // U+E54B (SGA K)
  { 0x00, 0x00, 0x03, 0x33, 0x03, 0x33, 0x03, 0x00 },   // U+E54C (SGA L)
  { 0x00, 0x00, 0x63, 0x60, 0x60, 0x60, 0x7F, 0x00 },   // U+E54D (SGA M)
  { 0x00, 0x00, 0x66, 0x60, 0x30, 0x18, 0x0C, 0x00 },   // U+E54E (SGA N)
  { 0x00, 0x00, 0x3C, 0x60, 0x30, 0x18, 0x0C, 0x00 },   // U+E54F (SGA O)
  { 0x00, 0x00, 0x66, 0x60, 0x66, 0x06, 0x66, 0x00 },   // U+E550 (SGA P)
  { 0x00, 0x00, 0x18, 0x00, 0x7E, 0x60, 0x7E, 0x00 },   // U+E551 (SGA Q)
  { 0x00, 0x00, 0x00, 0x66, 0x00, 0x66, 0x00, 0x00 },   // U+E552 (SGA R)
  { 0x00, 0x00, 0x0C, 0x0C, 0x3C, 0x30, 0x30, 0x00 },   // U+E553 (SGA S)
  { 0x00, 0x00, 0x3C, 0x30, 0x30, 0x00, 0x30, 0x00 },   // U+E554 (SGA T)
  { 0x00, 0x00, 0x00, 0x36, 0x00, 0x7F, 0x00, 0x00 },   // U+E555 (SGA U)
  { 0x00, 0x00, 0x18, 0x18, 0x7E, 0x00, 0x7E, 0x00 },   // U+E556 (SGA V)
  { 0x00, 0x00, 0x00, 0x18, 0x00, 0x66, 0x00, 0x00 },   // U+E557 (SGA W)
  { 0x00, 0x00, 0x66, 0x30, 0x18, 0x0C, 0x06, 0x00 },   // U+E558 (SGA X)
  { 0x00, 0x00, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00 },   // U+E559 (SGA Y)
  { 0x00, 0x00, 0x18, 0x3C, 0x66, 0x66, 0x66, 0x00 },   // U+E55A (SGA Z)

  // Latin extended 435 - 529
  { 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00 },   // U+00A1 (inverted !)
  { 0x18, 0x18, 0x7E, 0x03, 0x03, 0x7E, 0x18, 0x18 },   // U+00A2 (dollarcents)
  { 0x1C, 0x36, 0x26, 0x0F, 0x06, 0x67, 0x3F, 0x00 },   // U+00A3 (pound sterling)
  { 0x00, 0x00, 0x63, 0x3E, 0x36, 0x3E, 0x63, 0x00 },   // U+00A4 (currency mark)
  { 0x33, 0x33, 0x1E, 0x3F, 0x0C, 0x3F, 0x0C, 0x0C },   // U+00A5 (yen)
  { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 },   // U+00A6 (broken pipe)
  { 0x7C, 0xC6, 0x1C, 0x36, 0x36, 0x1C, 0x33, 0x1E },   // U+00A7 (paragraph)
  { 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+00A8 (diaeresis)
  { 0x3C, 0x42, 0x99, 0x85, 0x85, 0x99, 0x42, 0x3C },   // U+00A9 (copyright symbol)
  { 0x3C, 0x36, 0x36, 0x7C, 0x00, 0x00, 0x00, 0x00 },   // U+00AA (superscript a)
  { 0x00, 0xCC, 0x66, 0x33, 0x66, 0xCC, 0x00, 0x00 },   // U+00AB (<<)
  { 0x00, 0x00, 0x00, 0x3F, 0x30, 0x30, 0x00, 0x00 },   // U+00AC (gun pointing left)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+00AD (soft hyphen)
  { 0x3C, 0x42, 0x9D, 0xA5, 0x9D, 0xA5, 0x42, 0x3C },   // U+00AE (registered symbol)
  { 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+00AF (macron)
  { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00 },   // U+00B0 (degree)
  { 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x7E, 0x00 },   // U+00B1 (plusminus)
  { 0x1C, 0x30, 0x18, 0x0C, 0x3C, 0x00, 0x00, 0x00 },   // U+00B2 (superscript 2)
  { 0x1C, 0x30, 0x18, 0x30, 0x1C, 0x00, 0x00, 0x00 },   // U+00B2 (superscript 3)
  { 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+00B2 (aigu)
  { 0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x03 },   // U+00B5 (mu)
  { 0xFE, 0xDB, 0xDB, 0xDE, 0xD8, 0xD8, 0xD8, 0x00 },   // U+00B6 (pilcrow)
  { 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00 },   // U+00B7 (central dot)
  { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x30, 0x1E },   // U+00B8 (cedille)
  { 0x08, 0x0C, 0x08, 0x1C, 0x00, 0x00, 0x00, 0x00 },   // U+00B9 (superscript 1)
  { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00 },   // U+00BA (superscript 0)
  { 0x00, 0x33, 0x66, 0xCC, 0x66, 0x33, 0x00, 0x00 },   // U+00BB (>>)
  { 0xC3, 0x63, 0x33, 0xBD, 0xEC, 0xF6, 0xF3, 0x03 },   // U+00BC (1/4)
  { 0xC3, 0x63, 0x33, 0x7B, 0xCC, 0x66, 0x33, 0xF0 },   // U+00BD (1/2)
  { 0x03, 0xC4, 0x63, 0xB4, 0xDB, 0xAC, 0xE6, 0x80 },   // U+00BE (3/4)
  { 0x0C, 0x00, 0x0C, 0x06, 0x03, 0x33, 0x1E, 0x00 },   // U+00BF (inverted ?)
  { 0x07, 0x00, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x00 },   // U+00C0 (A grave)
  { 0x70, 0x00, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x00 },   // U+00C1 (A aigu)
  { 0x1C, 0x36, 0x00, 0x3E, 0x63, 0x7F, 0x63, 0x00 },   // U+00C2 (A circumflex)
  { 0x6E, 0x3B, 0x00, 0x3E, 0x63, 0x7F, 0x63, 0x00 },   // U+00C3 (A ~)
  { 0x63, 0x1C, 0x36, 0x63, 0x7F, 0x63, 0x63, 0x00 },   // U+00C4 (A umlaut)
  { 0x0C, 0x0C, 0x00, 0x1E, 0x33, 0x3F, 0x33, 0x00 },   // U+00C5 (A ring)
  { 0x7C, 0x36, 0x33, 0x7F, 0x33, 0x33, 0x73, 0x00 },   // U+00C6 (AE)
  { 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x18, 0x30, 0x1E },   // U+00C7 (C cedille)
  { 0x07, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00 },   // U+00C8 (E grave)
  { 0x38, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00 },   // U+00C9 (E aigu)
  { 0x0C, 0x12, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00 },   // U+00CA (E circumflex)
  { 0x36, 0x00, 0x3F, 0x06, 0x1E, 0x06, 0x3F, 0x00 },   // U+00CB (E umlaut)
  { 0x07, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+00CC (I grave)
  { 0x38, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+00CD (I aigu)
  { 0x0C, 0x12, 0x00, 0x1E, 0x0C, 0x0C, 0x1E, 0x00 },   // U+00CE (I circumflex)
  { 0x33, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+00CF (I umlaut)
  { 0x3F, 0x66, 0x6F, 0x6F, 0x66, 0x66, 0x3F, 0x00 },   // U+00D0 (Eth)
  { 0x3F, 0x00, 0x33, 0x37, 0x3F, 0x3B, 0x33, 0x00 },   // U+00D1 (N ~)
  { 0x0E, 0x00, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00 },   // U+00D2 (O grave)
  { 0x70, 0x00, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00 },   // U+00D3 (O aigu)
  { 0x3C, 0x66, 0x18, 0x3C, 0x66, 0x3C, 0x18, 0x00 },   // U+00D4 (O circumflex)
  { 0x6E, 0x3B, 0x00, 0x3E, 0x63, 0x63, 0x3E, 0x00 },   // U+00D5 (O ~)
  { 0xC3, 0x18, 0x3C, 0x66, 0x66, 0x3C, 0x18, 0x00 },   // U+00D6 (O umlaut)
  { 0x00, 0x36, 0x1C, 0x08, 0x1C, 0x36, 0x00, 0x00 },   // U+00D7 (multiplicative x)
  { 0x5C, 0x36, 0x73, 0x7B, 0x6F, 0x36, 0x1D, 0x00 },   // U+00D8 (O stroke)
  { 0x0E, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 },   // U+00D9 (U grave)
  { 0x70, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 },   // U+00DA (U aigu)
  { 0x3C, 0x66, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x00 },   // U+00DB (U circumflex)
  { 0x33, 0x00, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x00 },   // U+00DC (U umlaut)
  { 0x70, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x00 },   // U+00DD (Y aigu)
  { 0x0F, 0x06, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x0F },   // U+00DE (Thorn)
  { 0x00, 0x1E, 0x33, 0x1F, 0x33, 0x1F, 0x03, 0x03 },   // U+00DF (beta)
  { 0x07, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00 },   // U+00E0 (a grave)
  { 0x38, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00 },   // U+00E1 (a aigu)
  { 0x7E, 0xC3, 0x3C, 0x60, 0x7C, 0x66, 0xFC, 0x00 },   // U+00E2 (a circumflex)
  { 0x6E, 0x3B, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00 },   // U+00E3 (a ~)
  { 0x33, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00 },   // U+00E4 (a umlaut)
  { 0x0C, 0x0C, 0x1E, 0x30, 0x3E, 0x33, 0x7E, 0x00 },   // U+00E5 (a ring)
  { 0x00, 0x00, 0xFE, 0x30, 0xFE, 0x33, 0xFE, 0x00 },   // U+00E6 (ae)
  { 0x00, 0x00, 0x1E, 0x03, 0x03, 0x1E, 0x30, 0x1C },   // U+00E7 (c cedille)
  { 0x07, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00 },   // U+00E8 (e grave)
  { 0x38, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00 },   // U+00E9 (e aigu)
  { 0x7E, 0xC3, 0x3C, 0x66, 0x7E, 0x06, 0x3C, 0x00 },   // U+00EA (e circumflex)
  { 0x33, 0x00, 0x1E, 0x33, 0x3F, 0x03, 0x1E, 0x00 },   // U+00EB (e umlaut)
  { 0x07, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+00EC (i grave)
  { 0x1C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+00ED (i augu)
  { 0x3E, 0x63, 0x1C, 0x18, 0x18, 0x18, 0x3C, 0x00 },   // U+00EE (i circumflex)
  { 0x33, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00 },   // U+00EF (i umlaut)
  { 0x1B, 0x0E, 0x1B, 0x30, 0x3E, 0x33, 0x1E, 0x00 },   // U+00F0 (eth)
  { 0x00, 0x1F, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x00 },   // U+00F1 (n ~)
  { 0x00, 0x07, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00 },   // U+00F2 (o grave)
  { 0x00, 0x38, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00 },   // U+00F3 (o aigu)
  { 0x1E, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00 },   // U+00F4 (o circumflex)
  { 0x6E, 0x3B, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00 },   // U+00F5 (o ~)
  { 0x00, 0x33, 0x00, 0x1E, 0x33, 0x33, 0x1E, 0x00 },   // U+00F6 (o umlaut)
  { 0x18, 0x18, 0x00, 0x7E, 0x00, 0x18, 0x18, 0x00 },   // U+00F7 (division)
  { 0x00, 0x60, 0x3C, 0x76, 0x7E, 0x6E, 0x3C, 0x06 },   // U+00F8 (o stroke)
  { 0x00, 0x07, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00 },   // U+00F9 (u grave)
  { 0x00, 0x38, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00 },   // U+00FA (u aigu)
  { 0x1E, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00 },   // U+00FB (u circumflex)
  { 0x00, 0x33, 0x00, 0x33, 0x33, 0x33, 0x7E, 0x00 },   // U+00FC (u umlaut)
  { 0x00, 0x38, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F },   // U+00FD (y aigu)
  { 0x00, 0x00, 0x06, 0x3E, 0x66, 0x3E, 0x06, 0x00 },   // U+00FE (thorn)
  { 0x00, 0x33, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F },   // U+00FF (y umlaut)

  // Extras 530 - 539
  { 0x1F, 0x33, 0x33, 0x5F, 0x63, 0xF3, 0x63, 0xE3 },   // U+20A7 (Spanish Pesetas/Pt)
  { 0x70, 0xD8, 0x18, 0x3C, 0x18, 0x18, 0x1B, 0x0E },   // U+0192 (dutch florijn)
  { 0x3C, 0x36, 0x36, 0x7C, 0x00, 0x7E, 0x00, 0x00 },   // U+ (underlined superscript a)
  { 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x3E, 0x00, 0x00 },   // U+ (underlined superscript 0)
  { 0x00, 0x00, 0x00, 0x3F, 0x03, 0x03, 0x00, 0x00 },   // U+2310 (gun pointing right)
  { 0x30, 0x18, 0x0C, 0x18, 0x30, 0x00, 0x7E, 0x00 },   // U+ (less than or equal)
  { 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x00, 0x7E, 0x00 },   // U+ (greater than or equal)
  { 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   // U+ (grave)
  { 0x0E, 0x00, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x00 },   // U+ (Y grave)
  { 0x00, 0x07, 0x00, 0x33, 0x33, 0x3E, 0x30, 0x1F }    // U+ (y grave)
};

static int extra_font_lookup[10] = {
  8359,
  402,
  9078,
  9052,
  8976,
  8804,
  8805,
  96,
  7922,
  7923
};

static inline void str_size(const char* str, int* w, int* h) {
  const char* s = (const char*)str;
  int n = 0, m = 0, l = 1, c;
  while (s && *s != '\0') {
    c = *s;
    if (c >= 0 && c <= 127) {
      switch (c) {
        case '\n':
          if (n > m)
            m = n;
          n = 0;
          l++;
        default:
          s++;
          break;
      }
    }
    else if ((c & 0xE0) == 0xC0)
      s += 2;
    else if ((c & 0xF0) == 0xE0)
      s += 3;
    else if ((c & 0xF8) == 0xF0)
      s += 4;
    else
      return;
    n++;
  }
  *w = __MAX(n, m);
  *h = l;
}

int ctoi(const char* c, int* out) {
  int u = *c, l = 1;
  if ((u & 0xC0) == 0xC0) {
    int a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;
    if (a < 6 || !(u & 0x02)) {
      u = ((u << (a + 1)) & 0xFF) >> (a + 1);
      for (int b = 1; b < a; ++b)
        u = (u << 6) | (c[l++] & 0x3F);
    }
  }
  *out = u;
  return l;
}

#define LINE_HEIGHT 10

static inline int letter_index(int c) {
  if (c >= 32 && c <= 126) // Latin
    return c - 32;
  else if (c >= 9600 && c <= 9631) // Blocks
    return (c - 9600) + 95;
  else if (c >= 9472 && c <= 9599) // Box
    return (c - 9472) + 127;
  else if (c >= 912 && c <= 969) // Greek
    return (c - 912) + 255;
  else if (c >= 12352 && c <= 12447) // Hiragana
    return (c - 12352) + 313;
  else if (c >= 58689 && c <= 58714) // SGA
    return (c - 58689) + 409;
  else if (c >= 161 && c <= 255) // Latin extended
    return (c - 161) + 435;
  else {
    for (int i = 0; i < 10; ++i)
      if (extra_font_lookup[i] == c)
        return 530 + i;
    return 0;
  }
}

void ascii(struct surface_t* s, unsigned char ch, int x, int y, int fg, int bg) {
  int c = letter_index((int)ch), i, j;
  for (i = 0; i < 8; ++i)
    for (j = 0; j < 8; ++j) {
      if (font[c][i] & 1 << j) {
        pset(s, x + j, y + i, fg);
      } else {
        if (bg == -1)
          continue;
        pset(s, x + j, y + i, bg);
      }
    }
}

int character(struct surface_t* s, const char* ch, int x, int y, int fg, int bg) {
  int u = -1;
  int l = ctoi(ch, &u);
  int uc = letter_index(u), i, j;
  for (i = 0; i < 8; ++i)
    for (j = 0; j < 8; ++j) {
      if (font[uc][i] & 1 << j)
        pset(s, x + j, y + i, fg);
      else {
        pset(s, x + j, y + i, bg);
      }
    }

  return l;
}

void writeln(struct surface_t* s, int x, int y, int fg, int bg, const char* str) {
    const char* c = str;
  int u = x, v = y;
  while (c && *c != '\0')
    switch (*c) {
      case '\n':
        v += LINE_HEIGHT;
        u  = x;
        c++;
        break;
      default:
        c += character(s, c, u, v, fg, bg);
        u += 8;
        break;
    }
}

void writelnf(struct surface_t* s, int x, int y, int fg, int bg, const char* fmt, ...) {
  char* buffer = NULL;
  int buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  int length = vsnprintf(buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = GRAPHICS_REALLOC(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf(buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  writeln(s, x, y, fg, bg, buffer);
  GRAPHICS_SAFE_FREE(buffer);
}

void string(struct surface_t* s, int fg, int bg, const char* str) {
    int w, h;
  str_size(str, &w, &h);
  surface(s, w * 8, h * LINE_HEIGHT);
  fill(s, (bg == -1 ? 0 : bg));
  writeln(s, 0, 0, fg, bg, str);
}

void stringf(struct surface_t* s, int fg, int bg, const char* fmt, ...) {
    unsigned char* buffer = NULL;
  int buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  int length = vsnprintf((char*)buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = GRAPHICS_REALLOC(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf((char*)buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  string(s, fg, bg, (char*)buffer);
  GRAPHICS_SAFE_FREE(buffer);
}

#if defined(GRAPHICS_OSX)
#include <mach/mach_time.h>
#elif defined(GRAPHICS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(GRAPHICS_LINUX)
#include <time.h>
#ifdef CLOCK_MONOTONIC
#define CLOCKID CLOCK_MONOTONIC
#else
#define CLOCKID CLOCK_REALTIME
#endif
#else
#endif
unsigned long long ticks(void) {
  static int started = 0;
#if defined(GRAPHICS_OSX)
  static mach_timebase_info_data_t info;
  if (!started) {
    mach_timebase_info(&info);
    started = 1;
  }
  return (mach_absolute_time() * info.numer) / info.denom;
#elif defined(GRAPHICS_WINDOWS)
  static LARGE_INTEGER win_frequency;
  if (!started) {
    QueryPerformanceFrequency(&win_frequency);
    started = 1;
  }
  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  return (unsigned long long) ((1e9 * now.QuadPart)  / win_frequency.QuadPart);
#elif defined(GRAPHICS_LINUX)
  static struct timespec linux_rate;
  if (!started) {
    clock_getres(CLOCKID, &linux_rate);
    started = 1;
  }
  struct timespec spec;
  clock_gettime(CLOCKID, &spec);
  return spec.tv_sec * 1.0e9 + spec.tv_nsec;
#else
  return 0; // TODO: Add timing for other platforms
#endif
}

static short keycodes[512];
static bool keycodes_init = false;

void window_set_parent(struct window_t* s, void* p) {
  s->parent = p;
}

void* window_parent(struct window_t* s) {
  return s->parent;
}

#define X(a, b) void(*a##_cb)b,
void window_callbacks(XMAP_SCREEN_CB struct window_t* window) {
#undef X
#define X(a, b) window->a##_callback = a##_cb;
  XMAP_SCREEN_CB
#undef X
}

#define X(a, b) \
void a##_callback(struct window_t* window, void(*a##_cb)b) { \
  window->a##_callback = a##_cb; \
}
XMAP_SCREEN_CB
#undef X

int window_id(struct window_t* s) {
  return s->id;
}

void window_size(struct window_t* s, int* w, int* h) {
  if (w)
    *w = s->w;
  if (h)
    *h = s->h;
}

#define CBCALL(x, ...) \
  if (e_window && e_window->x) \
    e_window->x(e_window->parent, __VA_ARGS__);

#if defined(GRAPHICS_OSX)
#include <Cocoa/Cocoa.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
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
#define NSBitmapFormatAlphaNonpremultiplied NSAlphaNonpremultipliedBitmapFormat
#define CGContext graphicsPort
#endif

static inline int translate_mod(NSUInteger flags) {
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

static inline int translate_key(unsigned int key) {
  return (key >= sizeof(keycodes) / sizeof(keycodes[0]) ?  KB_KEY_UNKNOWN : keycodes[key]);
}

static inline NSImage* create_cocoa_image(struct surface_t* s) {
  NSImage* nsi = [[[NSImage alloc] initWithSize:NSMakeSize(s->w, s->h)] autorelease];
  if (!nsi)
    return nil;
  
  NSBitmapImageRep* nsbir = [[[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                     pixelsWide:s->w
                                                                     pixelsHigh:s->h
                                                                  bitsPerSample:8
                                                                samplesPerPixel:4
                                                                       hasAlpha:YES
                                                                       isPlanar:NO
                                                                 colorSpaceName:NSDeviceRGBColorSpace
                                                                   bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                                                                    bytesPerRow:0
                                                                   bitsPerPixel:0] autorelease];
  if (!nsbir)
    return nil;
  
  char* rgba = GRAPHICS_MALLOC(s->w * s->h * 4);
  int offset = 0, c;
  for(int i = 0; i < s->h; ++i) {
    for (int j = 0; j < s->w; j++) {
      c = pget(s, j, i);
      rgba[4 * offset]     = r_channel(c);
      rgba[4 * offset + 1] = g_channel(c);
      rgba[4 * offset + 2] = b_channel(c);
      rgba[4 * offset + 3] = a_channel(c);
      offset++;
    }
  }
  memcpy([nsbir bitmapData], rgba, s->w * s->h * sizeof(char) * 4);
  GRAPHICS_SAFE_FREE(rgba);
  
  [nsi addRepresentation:nsbir];
  return nsi;
}

@protocol AppViewDelegate;

@interface AppView : NSView
@property (nonatomic, strong) id<AppViewDelegate> delegate;
@property (strong) NSTrackingArea* track;
@property (atomic) struct surface_t* buffer;
@property BOOL mouse_in_window;
@property (nonatomic, strong) NSCursor* cursor;
@property BOOL custom_cursor;
@property BOOL cursor_vis;
@end

@implementation AppView
@synthesize delegate = _delegate;
@synthesize track = _track;
@synthesize buffer = _buffer;
@synthesize mouse_in_window = _mouse_in_window;
@synthesize cursor = _cursor;
@synthesize custom_cursor = _custom_cursor;
@synthesize cursor_vis = _cursor_vis;

-(id)initWithFrame:(NSRect)frameRect {
  _mouse_in_window = NO;
  _cursor = [NSCursor arrowCursor];
  _custom_cursor = NO;
  _cursor_vis = YES;
  
  self = [super initWithFrame:frameRect];
  [self updateTrackingAreas];
  return self;
}

-(void)updateTrackingAreas {
  if (_track) {
    [self removeTrackingArea:_track];
    [_track release];
  }
  _track = [[NSTrackingArea alloc] initWithRect:[self visibleRect]
                                        options:NSTrackingMouseEnteredAndExited
                                               |NSTrackingActiveInKeyWindow
                                               |NSTrackingEnabledDuringMouseDrag
                                               |NSTrackingCursorUpdate
                                               |NSTrackingInVisibleRect
                                               |NSTrackingAssumeInside
                                               |NSTrackingActiveInActiveApp
                                               |NSTrackingActiveAlways
                                               |NSTrackingMouseMoved
                                          owner:self
                                       userInfo:nil];
  [self addTrackingArea:_track];
  [super updateTrackingAreas];
}

-(BOOL)acceptsFirstResponder {
  return YES;
}

-(BOOL)performKeyEquivalent:(NSEvent*)event {
  return YES;
}

-(void)resetCursorRects {
  [super resetCursorRects];
  [self addCursorRect:[self visibleRect] cursor:(_cursor ? _cursor : [NSCursor arrowCursor])];
}

-(void)setCustomCursor:(NSImage*)img {
  if (!img) {
    if (_custom_cursor && _cursor)
      [_cursor release];
    _cursor = [NSCursor arrowCursor];
    return;
  }
  if (_custom_cursor && _cursor)
    [_cursor release];
  _custom_cursor = YES;
  _cursor = [[NSCursor alloc] initWithImage:img hotSpot:NSMakePoint(0.f, 0.f)];
  [_cursor retain];
}

-(void)setRegularCursor:(enum cursor_type)type {
  NSCursor* tmp = nil;
  switch (type) {
    default:
    case CURSOR_ARROW:
    case CURSOR_WAIT:
    case CURSOR_WAITARROW:
      tmp = [NSCursor arrowCursor];
      break;
    case CURSOR_IBEAM:
      tmp = [NSCursor IBeamCursor];
      break;
    case CURSOR_CROSSHAIR:
      tmp = [NSCursor crosshairCursor];
      break;
    case CURSOR_SIZENWSE:
    case CURSOR_SIZENESW:
      tmp = [NSCursor closedHandCursor];
      break;
    case CURSOR_SIZEWE:
      tmp = [NSCursor resizeLeftRightCursor];
      break;
    case CURSOR_SIZENS:
      tmp = [NSCursor resizeUpDownCursor];
      break;
    case CURSOR_SIZEALL:
      tmp = [NSCursor closedHandCursor];
      break;
    case CURSOR_NO:
      tmp = [NSCursor operationNotAllowedCursor];
      break;
    case CURSOR_HAND:
      tmp = [NSCursor pointingHandCursor];
      break;
  }
  if (_custom_cursor && _cursor)
    [_cursor release];
  _custom_cursor = NO;
  _cursor = tmp;
  [_cursor retain];
}

-(void)setCursorVisibility:(BOOL)visibility {
  _cursor_vis = visibility;
}

-(void)mouseEntered:(NSEvent*)event {
  _mouse_in_window = YES;
  if (!_cursor_vis)
    [NSCursor hide];
}

-(void)mouseExited:(NSEvent*)event {
  _mouse_in_window = NO;
  if (!_cursor_vis)
    [NSCursor unhide];
}

-(void)mouseMoved:(NSEvent*)event {
  if (_cursor && _mouse_in_window)
    [_cursor set];
}

-(BOOL)preservesContentDuringLiveResize {
  return NO;
}

-(void)drawRect:(NSRect)dirtyRect {
  if (!_buffer)
    return;
  
  CGContextRef ctx = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
  CGColorSpaceRef s = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef p = CGDataProviderCreateWithData(NULL, _buffer->buf, _buffer->w * _buffer->h * 4, NULL);
  CGImageRef img = CGImageCreate(_buffer->w, _buffer->h, 8, 32, _buffer->w * 4, s, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, p, NULL, 0, kCGRenderingIntentDefault);
  /* This line causes Visual Studio to crash if uncommented. I don't know why and I don't want to know.
   Not the whole line though, just the `[self frame]` parts. This has caused me an issue for over a month.
   `CGContextDrawImage(ctx, CGRectMake(0, 0, [self frame].size.width, [self frame].size.height), img);` */
  CGSize wh = [self frame].size;
  CGContextDrawImage(ctx, CGRectMake(0, 0, wh.width, wh.height), img);
  CGColorSpaceRelease(s);
  CGDataProviderRelease(p);
  CGImageRelease(img);
}

-(void)dealloc {
  [_track release];
  if (_custom_cursor && _cursor)
    [_cursor release];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-missing-super-calls"
}
#pragma clang diagnostic pop
@end

@protocol AppViewDelegate <NSObject>
@end

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate, AppViewDelegate>
@property (unsafe_unretained) NSWindow* window;
@property (unsafe_unretained) AppView* view;
@property (nonatomic) struct window_t* parent;
@property BOOL closed;
@end

struct osx_window_t {
  AppDelegate* delegate;
  NSInteger window_id;
};

LINKEDLIST(window, struct osx_window_t);
static struct window_node_t* windows = NULL;

@implementation AppDelegate
@synthesize window = _window;
@synthesize view = _view;
@synthesize parent = _parent;
@synthesize closed = _closed;

-(id)initWithSize:(NSSize)windowSize styleMask:(short)flags title:(const char*)windowTitle {
  NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
  flags |= (flags & FULLSCREEN ? (BORDERLESS | RESIZABLE | FULLSCREEN_DESKTOP) : 0);
  styleMask |= (flags & RESIZABLE ? NSWindowStyleMaskResizable : 0);
  styleMask |= (flags & BORDERLESS ? NSWindowStyleMaskFullSizeContentView : 0);
  if (flags & FULLSCREEN_DESKTOP) {
    NSRect f = [[NSScreen mainScreen] frame];
    windowSize.width = f.size.width;
    windowSize.height = f.size.height;
    styleMask |= NSWindowStyleMaskFullSizeContentView;
  }
  NSRect frameRect = NSMakeRect(0, 0, windowSize.width, windowSize.height);
  
  _window = [[NSWindow alloc] initWithContentRect:frameRect
                                        styleMask:styleMask
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
  if (!_window) {
    release();
    GRAPHICS_ERROR(OSX_WINDOW_CREATION_FAILED, "[_window initWithContentRect] failed");
    return nil;
  }
  
  if (flags & ALWAYS_ON_TOP)
    [_window setLevel:NSFloatingWindowLevel];
  
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
  if (flags & FULLSCREEN) {
    [_window setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
    [_window performSelectorOnMainThread: @selector(toggleFullScreen:) withObject:_window waitUntilDone:NO];
  }
#else
#pragma message WARN("Fullscreen is unsupported on OSX versions < 10.7")
#endif
  
  [_window setAcceptsMouseMovedEvents:YES];
  [_window setRestorable:NO];
  [_window setTitle:(windowTitle ? @(windowTitle) : [[NSProcessInfo processInfo] processName])];
  [_window setReleasedWhenClosed:NO];
  
  if (flags & BORDERLESS && flags & ~FULLSCREEN) {
    [_window setTitle:@""];
    [_window setTitlebarAppearsTransparent:YES];
    [[_window standardWindowButton:NSWindowZoomButton] setHidden:YES];
    [[_window standardWindowButton:NSWindowCloseButton] setHidden:YES];
    [[_window standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
  }
  
  [_window center];
  _view = [[AppView alloc] initWithFrame:frameRect];
  if (!_view) {
    release();
    GRAPHICS_ERROR(OSX_WINDOW_CREATION_FAILED, "[_view initWithFrame] failed");
    return nil;
  }
  [_view setDelegate:self];
  [_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  
  [_window setContentView:_view];
  [_window setDelegate:self];
  [_window performSelectorOnMainThread:@selector(makeKeyAndOrderFront:)
                            withObject:nil
                         waitUntilDone:YES];
  
  _closed = NO;
  return self;
}

-(void)setParent:(struct window_t*)screen {
  _parent = screen;
}

-(void)windowWillClose:(NSNotification*)notification {
  _closed = YES;
  if (_parent->closed_callback)
    _parent->closed_callback(_parent->parent);
  [[self view] dealloc];
  
  struct window_node_t *head = windows, *cursor = windows, *prev = NULL;
  while (cursor) {
    if (cursor->data->delegate != self) {
      prev = cursor;
      cursor = cursor->next;
      continue;
    }
    
    if (!prev)
      head = cursor->next;
    else
      prev->next = cursor->next;
    break;
  }
  if (cursor) {
    cursor->next = NULL;
    GRAPHICS_FREE(cursor);
  }
  windows = head;
}

-(void)windowDidBecomeKey:(NSNotification*)notification {
  if (_parent->focus_callback)
    _parent->focus_callback(_parent->parent, true);
}

-(void)windowDidResignKey:(NSNotification*)notification {
  if (_parent->focus_callback)
    _parent->focus_callback(_parent->parent, false);
}

-(void)windowDidResize:(NSNotification*)notification {
  static CGSize size;
  size = [_view frame].size;
  _parent->w = (int)roundf(size.width);
  _parent->h = (int)roundf(size.height);
  if (_parent->resize_callback)
  _parent->resize_callback(_parent->resize_callback, _parent->w, _parent->h);
}
@end

bool window(struct window_t* s, const char* t, int w, int h, short flags) {
  if (!keycodes_init) {
    memset(keycodes,  -1, sizeof(keycodes));
    
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
    keycodes[0x18] = KB_KEY_EQUALS;
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
    keycodes[0x51] = KB_KEY_KP_EQUALS;
    keycodes[0x43] = KB_KEY_KP_MULTIPLY;
    keycodes[0x4E] = KB_KEY_KP_SUBTRACT;
    keycodes_init = true;
  }
  
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  
  AppDelegate* app = [[AppDelegate alloc] initWithSize:NSMakeSize(w, h) styleMask:flags title:t];
  if (!app) {
    release();
    GRAPHICS_ERROR(OSX_WINDOW_CREATION_FAILED, "[AppDelegate alloc] failed");
    return false;
  }
  
  struct osx_window_t* win_data = GRAPHICS_MALLOC(sizeof(struct osx_window_t));
  win_data->delegate = app;
  win_data->window_id = [[app window] windowNumber];
  windows = window_push(windows, win_data);
  
  memset(s, 0, sizeof(struct window_t));
  s->id = (int)[[app window] windowNumber];
  s->w  = w;
  s->h  = h;
  s->window = (void*)app;
  [app setParent:s];
  
  [NSApp activateIgnoringOtherApps:YES];
  [pool drain];
  return true;
}

#define SET_DEFAULT_APP_ICON [NSApp setApplicationIconImage:[NSImage imageNamed:@"NSApplicationIcon"]]

void window_icon(struct window_t* s, struct surface_t* b) {
  if (!b || !b->buf) {
    SET_DEFAULT_APP_ICON;
    return;
  }
  
  NSImage* img = create_cocoa_image(b);
  if (!img)  {
    GRAPHICS_ERROR(WINDOW_ICON_FAILED, "window_icon_b() failed: Couldn't set window icon");
    SET_DEFAULT_APP_ICON;
    return;
  }
  [NSApp setApplicationIconImage:img];
}

void window_title(struct window_t* s, const char* t) {
  [[(AppDelegate*)s->window window] setTitle:@(t)];
}

void window_position(struct window_t* s, int* x, int*  y) {
  static NSRect frame;
  frame = [[(AppDelegate*)s->window window] frame];
  if (x)
    *x = frame.origin.x;
  if (y)
    *y = frame.origin.y;
}

void screen_size(struct window_t* s, int* w, int* h) {
  static NSRect frame;
  frame = [[[(AppDelegate*)s->window window] screen] frame];
  if (w)
    *w = frame.size.width;
  if (h)
    *h = frame.size.height;
}


void window_destroy(struct window_t* s) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  AppDelegate* app = (AppDelegate*)s->window;
  [[app view] dealloc];
  [app dealloc];
  memset(s, 0, sizeof(struct window_t));
  [pool drain];
}

bool closed(struct window_t* s) {
  return (bool)[(AppDelegate*)s->window closed];
}

bool closed_va(int n, ...) {
  va_list args;
  va_start(args, n);
  bool ret = true;
  for (int i = 0; i < n; ++i) {
    struct window_t* w = va_arg(args, struct window_t*);
    if (![(AppDelegate*)w->window closed]) {
      ret = false;
      break;
    }
  }
  va_end(args);
  return ret;
}

bool closed_all() {
  return windows == NULL;
}

void cursor_lock(struct window_t* s, bool locked) {
  return;
}

void cursor_visible(struct window_t* s, bool shown) {
  [[(AppDelegate*)s->window view] setCursorVisibility:shown];
}

void cursor_icon(struct window_t* s, enum cursor_type t) {
  AppDelegate* app = (AppDelegate*)s->window;
  if (!app) {
    GRAPHICS_ERROR(CURSOR_MOD_FAILED, "cursor_icon() failed: Invalid window");
    return;
  }
  [[app view] setRegularCursor:t];
}

void cursor_icon_custom(struct window_t* s, struct surface_t* b) {
  NSImage* img = create_cocoa_image(b);
  if (!img) {
    GRAPHICS_ERROR(CURSOR_MOD_FAILED, "cursor_icon_custom_buf() failed: Couldn't set cursor from buffer");
    return;
  }
  
  AppDelegate* app = (AppDelegate*)s->window;
  if (!app) {
    [img release];
    GRAPHICS_ERROR(CURSOR_MOD_FAILED, "cursor_icon_custom_buf() failed: Invalid window");
    return;
  }
  [[app view] setCustomCursor:img];
  [img release];
}

void cursor_pos(int* x, int* y) {
  static NSPoint _p = {0,0};
  _p = [NSEvent mouseLocation];
  if (x)
    *x = _p.x;
  if (y)
    *y = [NSScreen mainScreen].frame.size.height - _p.y;
}

void cursor_set_pos(int x, int y) {
  CGWarpMouseCursorPosition((CGPoint){ x, y });
}

struct window_t* event_delegate(NSInteger window_id) {
  struct window_node_t* cursor = windows;
  while (cursor) {
    if (cursor->data->window_id == window_id)
      return [cursor->data->delegate parent];
    cursor = cursor->next;
  }
  return NULL;
}

void events() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSEvent* e = nil;
  while ((e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                 untilDate:[NSDate distantPast]
                                    inMode:NSDefaultRunLoopMode
                                   dequeue:YES])) {
    struct window_t* e_window = event_delegate([e windowNumber]);
    if (!e_window) {
      [NSApp sendEvent:e];
      continue;
    }
    switch ([e type]) {
      case NSEventTypeKeyUp:
      case NSEventTypeKeyDown:
        CBCALL(keyboard_callback, translate_key([e keyCode]), translate_mod([e modifierFlags]), ([e type] == NSEventTypeKeyDown));
        break;
      case NSEventTypeLeftMouseUp:
      case NSEventTypeRightMouseUp:
      case NSEventTypeOtherMouseUp:
        CBCALL(mouse_button_callback, (enum button)([e buttonNumber] + 1), translate_mod([e modifierFlags]), false);
        break;
      case NSEventTypeLeftMouseDown:
      case NSEventTypeRightMouseDown:
      case NSEventTypeOtherMouseDown:
        CBCALL(mouse_button_callback, (enum button)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
        break;
      case NSEventTypeScrollWheel:
        CBCALL(scroll_callback, translate_mod([e modifierFlags]), [e deltaX], [e deltaY]);
        break;
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged:
        CBCALL(mouse_button_callback, (enum button)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
      case NSEventTypeMouseMoved: {
        AppDelegate* app = (AppDelegate*)e_window->window;
        if ([[app view] mouse_in_window])
          CBCALL(mouse_move_callback, [e locationInWindow].x, (int)([[app view] frame].size.height - roundf([e locationInWindow].y)), [e deltaX], [e deltaY]);
        break;
      }
      default:
        break;
    }
    [NSApp sendEvent:e];
  }
  [pool release];
}

void flush(struct window_t* s, struct surface_t* b) {
  if (!s)
    return;
  AppDelegate* tmp = (AppDelegate*)s->window;
  if (!tmp)
    return;
  [tmp view].buffer = b;
  [[tmp view] setNeedsDisplay:YES];
}

void release() {
  struct window_node_t *cursor = windows, *tmp = NULL;
  while (cursor) {
    tmp = cursor->next;
    [cursor->data->delegate dealloc];
    GRAPHICS_SAFE_FREE(cursor->data);
    GRAPHICS_SAFE_FREE(cursor);
    cursor = tmp;
  }
  [NSApp terminate:nil];
}
#elif defined(GRAPHICS_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct win32_window_t {
  WNDCLASS wnd;
  HWND hwnd;
  HDC hdc;
  BITMAPINFO* bmpinfo;
  TRACKMOUSEEVENT tme;
  HICON icon;
  HCURSOR cursor;
  int cursor_lx, cursor_ly;
  bool mouse_inside, cursor_vis, cursor_locked, closed, refresh_tme, custom_icon, custom_cursor;
  struct surface_t* buffer;
};

static void close_win32_window(struct win32_window_t* window) {
  if (window->closed)
    return;
  window->closed = true;
  if (window->cursor_locked)
    ClipCursor(NULL);
  if (!window->cursor_vis)
    ShowCursor(TRUE);
  GRAPHICS_FREE(window->bmpinfo);
  if (window->custom_icon && window->icon)
    DeleteObject(window->icon);
  if (window->custom_cursor && window->cursor)
    DeleteObject(window->cursor);
  ReleaseDC(window->hwnd, window->hdc);
  DestroyWindow(window->hwnd);
}

static void clip_win32_cursor(HWND hwnd) {
  static RECT r = { 0 };
  GetClientRect(hwnd, &r);
  ClientToScreen(hwnd, (LPPOINT)&r);
  ClientToScreen(hwnd, (LPPOINT)&r + 1);
  ClipCursor(&r);
}

LINKEDLIST(window, struct win32_window_t);
static struct window_node_t* windows = NULL;

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
    static MSG next;
    static DWORD time;

    if (lParam & 0x01000000)
      return KB_KEY_RIGHT_CONTROL;

    ZeroMemory(&next, sizeof(MSG));
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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  static struct win32_window_t* e_data = NULL;
  static struct window_t* e_window = NULL;
  e_window = (struct window_t*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
  if (!e_window || !e_window->window)
    return DefWindowProc(hWnd, message, wParam, lParam);
  e_data = (struct win32_window_t*)e_window->window;
  
  switch (message) {
    case WM_PAINT:
      if (!e_data->buffer)
        break;
      e_data->bmpinfo->bmiHeader.biWidth = e_data->buffer->w;
      e_data->bmpinfo->bmiHeader.biHeight = -e_data->buffer->h;
      StretchDIBits(e_data->hdc, 0, 0, e_window->w, e_window->h, 0, 0, e_data->buffer->w, e_data->buffer->h, e_data->buffer->buf, e_data->bmpinfo, DIB_RGB_COLORS, SRCCOPY);
      ValidateRect(hWnd, NULL);
      break;
    case WM_DESTROY:
    case WM_CLOSE:
      close_win32_window(e_data);
      windows = window_pop(windows, e_data);
      e_data->closed = true;
      if (e_window && e_window->closed_callback)
        e_window->closed_callback(e_window->parent);
      break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      static int kb_key = 0;
      static bool kb_action = false;
      kb_key = translate_key(wParam, lParam);
      kb_action = !((lParam >> 31) & 1);

      if (kb_key == KB_KEY_UNKNOWN)
        break;
      if (!kb_action && wParam == VK_SHIFT) {
        CBCALL(keyboard_callback, KB_KEY_LEFT_SHIFT, translate_mod(), kb_action);
      } else if (wParam == VK_SNAPSHOT) {
        CBCALL(keyboard_callback, kb_key, translate_mod(), false);
      } else {
        CBCALL(keyboard_callback, kb_key, translate_mod(), kb_action);
      }
      break;
    }
    case WM_SETCURSOR:
      if (LOWORD(lParam) == HTCLIENT) {
        SetCursor(e_data->cursor);
        return TRUE;
      }
      break;
    case WM_CHAR:
    case WM_SYSCHAR:
    case WM_UNICHAR:
      // I don't know if this is important or not
      break;
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_MBUTTONUP:
    case WM_XBUTTONUP:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONDBLCLK: {
      static int m_button, m_action = 0;
      switch (message) {
      case WM_LBUTTONDOWN:
        m_action = 1;
      case WM_LBUTTONUP:
        m_button = enum button_1;
        break;
      case WM_RBUTTONDOWN:
        m_action = 1;
      case WM_RBUTTONUP:
        m_button = enum button_2;
        break;
      case WM_MBUTTONDOWN:
        m_action = 1;
      case WM_MBUTTONUP:
        m_button = enum button_3;
        break;
      default:
        m_button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? enum button_5 : enum button_6);
        if (message == WM_XBUTTONDOWN)
          m_action = 1;
      }
      CBCALL(mouse_button_callback, (enum button)m_button, translate_mod(), m_action);
      break;
    }
    case WM_MOUSEWHEEL:
      CBCALL(scroll_callback, translate_mod(), 0.f, (SHORT)HIWORD(wParam) / (float)WHEEL_DELTA);
      break;
    case WM_MOUSEHWHEEL:
      CBCALL(scroll_callback, translate_mod(), -((SHORT)HIWORD(wParam) / (float)WHEEL_DELTA), 0.f);
      break;
    case WM_MOUSEMOVE: {
      if (e_data->refresh_tme) {
        e_data->tme.cbSize = sizeof(e_data->tme);
        e_data->tme.hwndTrack = e_data->hwnd;
        e_data->tme.dwFlags = TME_HOVER | TME_LEAVE;
        e_data->tme.dwHoverTime = 1;
        TrackMouseEvent(&e_data->tme);
      }
      static int cx, cy;
      cx = ((int)(short)LOWORD(lParam));
      cy = ((int)(short)HIWORD(lParam));
      CBCALL(mouse_move_callback, cx, cy, cx - e_data->cursor_lx, cy - e_data->cursor_ly);
      e_data->cursor_lx = cx;
      e_data->cursor_ly = cy;
      break;
    }
    case WM_MOUSEHOVER:
      if (!e_data->mouse_inside) {
        e_data->refresh_tme = true;
        e_data->mouse_inside = true;
        if (!e_data->cursor_vis)
          ShowCursor(FALSE);
      }
      break;
    case WM_MOUSELEAVE:
      if (e_data->mouse_inside) {
        e_data->refresh_tme = true;
        e_data->mouse_inside = false;
        ShowCursor(TRUE);
      }
      break;
    case WM_SIZE:
      e_window->w = LOWORD(lParam);
      e_window->h = HIWORD(lParam);
      CBCALL(resize_callback, e_window->w, e_window->h);
      break;
    case WM_SETFOCUS:
      if (e_data->cursor_locked)
        clip_win32_cursor(e_data->hwnd);
      CBCALL(focus_callback, true);
      break;
    case WM_KILLFOCUS:
      if (e_data->cursor_locked)
        ClipCursor(NULL);
      CBCALL(focus_callback, false);
      break;
    default:
      break;
  }
  return DefWindowProc(hWnd, message, wParam, lParam);
}

static void windows_error(enum graphics_error err, const char* msg) {
  DWORD id = GetLastError();
  LPSTR buf = NULL;
  FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, NULL);
  GRAPHICS_ERROR(err, "%s (%d): %s", msg, id, buf);
}

bool window(struct window_t* s, const char* t, int w, int h, short flags) {
  if (!keycodes_init) {
    memset(keycodes, -1, sizeof(keycodes));
    
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
    keycodes[0x00D] = KB_KEY_EQUALS;
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
    
    keycodes_init = true;
  }

  struct win32_window_t* win_data = GRAPHICS_MALLOC(sizeof(struct win32_window_t));
  if (!win_data) {
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  RECT rect = {0};
  long window_flags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
  if (flags & FULLSCREEN) {
    flags = FULLSCREEN;
    rect.right = GetSystemMetrics(SM_CXSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYSCREEN);
    window_flags = WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

    DEVMODE settings = { 0 };
    EnumDisplaySettings(0, 0, &settings);
    settings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
    settings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
    settings.dmBitsPerPel = 32;
    settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    if (ChangeDisplaySettings(&settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
      flags = FULLSCREEN_DESKTOP;
  }

  if (flags & BORDERLESS)
    window_flags = WS_POPUP;
  if (flags & RESIZABLE)
    window_flags |= WS_MAXIMIZEBOX | WS_SIZEBOX;
  if (flags & FULLSCREEN_DESKTOP) {
    window_flags = WS_OVERLAPPEDWINDOW;

    int width = GetSystemMetrics(SM_CXFULLSCREEN);
    int height = GetSystemMetrics(SM_CYFULLSCREEN);

    rect.right = width;
    rect.bottom = height;
    AdjustWindowRect(&rect, window_flags, 0);
    if (rect.left < 0) {
      width += rect.left * 2;
      rect.right += rect.left;
      rect.left = 0;
    }
    if (rect.bottom > (LONG)height) {
      height -= (rect.bottom - height);
      rect.bottom += (rect.bottom - height);
      rect.top = 0;
    }
  }
  else if (!(flags & FULLSCREEN)) {
    rect.right = w;
    rect.bottom = h;

    AdjustWindowRect(&rect, window_flags, 0);

    rect.right -= rect.left;
    rect.bottom -= rect.top;

    rect.left = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
    rect.top = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) / 2;
  }

  memset(&win_data->wnd, 0, sizeof(win_data->wnd));
  win_data->wnd.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
  win_data->wnd.lpfnWndProc = WndProc;
  win_data->wnd.hCursor = LoadCursor(0, IDC_ARROW);
  win_data->wnd.lpszClassName = t;
  if (!RegisterClass(&win_data->wnd)) {
    windows_error(WIN_WINDOW_CREATION_FAILED, "RegisterClass() failed");
    return false;
  }

  if (!(win_data->hwnd = CreateWindowEx(0, t, t, window_flags, rect.left, rect.top, rect.right, rect.bottom, 0, 0, 0, 0))) {
    windows_error(WIN_WINDOW_CREATION_FAILED, "CreateWindowEx() failed");
    return false;
  }
  if (!(win_data->hdc = GetDC(win_data->hwnd))) {
    windows_error(WIN_WINDOW_CREATION_FAILED, "GetDC() failed");
    return false;
  }
  SetWindowLongPtr(win_data->hwnd, GWLP_USERDATA, (LONG_PTR)s);

  if (flags & ALWAYS_ON_TOP)
    SetWindowPos(win_data->hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  ShowWindow(win_data->hwnd, SW_NORMAL);
  SetFocus(win_data->hwnd);

  size_t bmpinfo_sz = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3;
  if (!(win_data->bmpinfo = GRAPHICS_MALLOC(bmpinfo_sz))) {
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  memset(win_data->bmpinfo, 0, bmpinfo_sz);
  win_data->bmpinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  win_data->bmpinfo->bmiHeader.biPlanes = 1;
  win_data->bmpinfo->bmiHeader.biBitCount = 32;
  win_data->bmpinfo->bmiHeader.biCompression = BI_BITFIELDS;
  win_data->bmpinfo->bmiHeader.biWidth = w;
  win_data->bmpinfo->bmiHeader.biHeight = -(LONG)h;
  win_data->bmpinfo->bmiColors[0].rgbRed = 0xFF;
  win_data->bmpinfo->bmiColors[1].rgbGreen = 0xFF;
  win_data->bmpinfo->bmiColors[2].rgbBlue = 0xff;

  win_data->tme.cbSize = sizeof(win_data->tme);
  win_data->tme.hwndTrack = win_data->hwnd;
  win_data->tme.dwFlags = TME_HOVER | TME_LEAVE;
  win_data->tme.dwHoverTime = HOVER_DEFAULT;
  TrackMouseEvent(&win_data->tme);

  win_data->buffer = NULL;
  win_data->mouse_inside = false;
  win_data->closed = false;
  win_data->refresh_tme = true;

  win_data->icon = NULL;
  win_data->cursor = win_data->wnd.hCursor;
  win_data->custom_icon = false;
  win_data->custom_cursor = false;
  win_data->cursor_vis = true;
  win_data->cursor_locked = false;
  POINT p;
  GetCursorPos(&p);
  win_data->cursor_lx = p.x;
  win_data->cursor_ly = p.y;

  windows = window_push(windows, win_data);
  static int window_id = 0;
  s->w = rect.right;
  s->h = rect.bottom;
  s->id = window_id++;
  s->window = win_data;

  return true;
}

void window_icon(struct window_t* s, struct surface_t* b) {
  struct win32_window_t* win = (struct win32_window_t*)s->window;
  HBITMAP hbmp = NULL, bmp_mask = NULL;

  if (!(hbmp = CreateBitmap(b->w, b->h, 1, 32, b->buf)))
    goto FAILED;
  if (!(bmp_mask = CreateCompatibleBitmap(GetDC(NULL), b->w / 2, b->h / 2)))
    goto FAILED;
  ICONINFO ii = { 0 };
  ii.fIcon = TRUE;
  ii.hbmColor = hbmp;
  ii.hbmMask = bmp_mask;
  win->icon = CreateIconIndirect(&ii);

FAILED:
  if (bmp_mask)
    DeleteObject(bmp_mask);
  if (hbmp)
    DeleteObject(hbmp);

  if (!win->icon) {
    windows_error(WINDOW_ICON_FAILED, "create_windows_icon() failed");
    win->icon = LoadIcon(NULL, IDI_APPLICATION);
    win->custom_icon = false;
  } else
    win->custom_icon = true;

  SetClassLong(win->hwnd, GCLP_HICON, win->icon);
  SendMessage(win->hwnd, WM_SETICON, ICON_SMALL, win->icon);
  SendMessage(win->hwnd, WM_SETICON, ICON_BIG, win->icon);
  SendMessage(GetWindow(win->hwnd, GW_OWNER), WM_SETICON, ICON_SMALL, win->icon);
  SendMessage(GetWindow(win->hwnd, GW_OWNER), WM_SETICON, ICON_BIG, win->icon);
}

void window_title(struct window_t* s, const char* t) {
  SetWindowTextA(((struct win32_window_t*)s->window)->hwnd, t);
}

void window_position(struct window_t* s, int* x, int*  y) {
  static RECT rect = { 0 };
  GetWindowRect(((struct win32_window_t*)s->window)->hwnd, &rect);
  if (x)
    *x = rect.left;
  if (y)
    *y = rect.top;
}

void screen_size(struct window_t* s, int* w, int* h) {
  if (w)
    *w = GetSystemMetrics(SM_CXFULLSCREEN);
  if (h)
    *h = GetSystemMetrics(SM_CYFULLSCREEN);
}

void window_destroy(struct window_t* s) {
  struct win32_window_t* win = (struct win32_window_t*)s->window;
  close_win32_window(win);
  GRAPHICS_SAFE_FREE(win);
  s->window = NULL;
}

bool closed(struct window_t* s) {
  return ((struct win32_window_t*)s->window)->closed;
}

bool closed_va(int n, ...) {
  va_list args;
  va_start(args, n);
  bool ret = true;
  for (int i = 0; i < n; ++i) {
    struct window_t* w = va_arg(args, struct window_t*);
    if (!((struct win32_window_t*)w->window)->closed) {
      ret = false;
      break;
    }
  }
  va_end(args);
  return ret;
}

bool closed_all() {
  return windows == NULL;
}

void cursor_lock(struct window_t* s, bool locked) {
  struct win32_window_t* win = (struct win32_window_t*)s->window;
  if (!s || !locked) {
    ClipCursor(NULL);
    win->cursor_locked = false;
  } else {
    clip_win32_cursor(win->hwnd);
    win->cursor_locked = true;
  }
}

void cursor_visible(struct window_t* s, bool shown) {
  ((struct win32_window_t*)s->window)->cursor_vis = shown;
}

void cursor_icon(struct window_t* s, enum cursor_type t) {
  HCURSOR tmp = NULL;
  switch (t) {
  default:
  case CURSOR_ARROW:
    tmp = LoadCursor(NULL, IDC_ARROW);
    break;
  case CURSOR_WAIT:
    tmp = LoadCursor(NULL, IDC_WAIT);
    break;
  case CURSOR_WAITARROW:
    tmp = LoadCursor(NULL, IDC_APPSTARTING);
    break;
  case CURSOR_IBEAM:
    tmp = LoadCursor(NULL, IDC_IBEAM);
    break;
  case CURSOR_CROSSHAIR:
    tmp = LoadCursor(NULL, IDC_CROSS);
    break;
  case CURSOR_SIZENWSE:
    tmp = LoadCursor(NULL, IDC_SIZENWSE);
    break;
  case CURSOR_SIZENESW:
    tmp = LoadCursor(NULL, IDC_SIZENESW);
    break;
  case CURSOR_SIZEWE:
    tmp = LoadCursor(NULL, IDC_SIZENWSE);
    break;
  case CURSOR_SIZENS:
    tmp = LoadCursor(NULL, IDC_SIZENS);
    break;
  case CURSOR_SIZEALL:
    tmp = LoadCursor(NULL, IDC_SIZEALL);
    break;
  case CURSOR_NO:
    tmp = LoadCursor(NULL, IDC_NO);
    break;
  case CURSOR_HAND:
    tmp = LoadCursor(NULL, IDC_HAND);
    break;
  }
  struct win32_window_t* win = (struct win32_window_t*)s->window;
  if (win->cursor && win->custom_cursor)
    DeleteObject(win->cursor);
  win->custom_cursor = false;
  win->cursor = tmp;
  SetCursor(win->cursor);
}

void cursor_icon_custom(struct window_t* s, struct surface_t* b) {
  struct win32_window_t* win = (struct win32_window_t*)s->window;
  HBITMAP hbmp = NULL, bmp_mask = NULL;

  if (!(hbmp = CreateBitmap(b->w, b->h, 1, 32, b->buf)))
    goto FAILED;
  if (!(bmp_mask = CreateCompatibleBitmap(GetDC(NULL), b->w, b->h)))
    goto FAILED;

  ICONINFO ii;
  ii.fIcon = TRUE;
  ii.xHotspot = 0;
  ii.yHotspot = -b->h;
  ii.hbmMask = bmp_mask;
  ii.hbmColor = hbmp;
  win->cursor = (HCURSOR)CreateIconIndirect(&ii);

FAILED:
  if (bmp_mask)
    DeleteObject(bmp_mask);
  if (hbmp)
    DeleteObject(hbmp);

  if (!win->cursor) {
    windows_error(WINDOW_ICON_FAILED, "cursor_icon_custom() failed");
    win->icon = LoadCursor(NULL, IDC_ARROW);
    win->custom_cursor = false;
  } else
    win->custom_icon = true;
  
  SetCursor(win->cursor);
}

void cursor_pos(int* x, int* y) {
  static POINT p;
  GetCursorPos(&p);
  if (x)
    *x = p.x;
  if (y)
    *y = p.y;
}

void cursor_set_pos(int x, int y) {
  SetCursorPos(x, y);
}

void events() {
  static MSG msg;
  ZeroMemory(&msg, sizeof(MSG));
  if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void flush(struct window_t* s, struct surface_t* b) {
  if (!s)
    return;
  struct win32_window_t* tmp = (struct win32_window_t*)s->window;
  if (!tmp || tmp->closed)
    return;
  tmp->buffer = b;
  InvalidateRect(tmp->hwnd, NULL, TRUE);
  SendMessage(tmp->hwnd, WM_PAINT, 0, 0);
}

void release() {
  struct window_node_t *tmp = NULL, *cursor = windows;
  while (cursor) {
    tmp = cursor->next;
    close_win32_window(tmp->data);
    GRAPHICS_SAFE_FREE(cursor->data);
    GRAPHICS_SAFE_FREE(cursor);
    cursor = tmp;
  }
}
#elif defined(GRAPHICS_LINUX) && !defined(GRAPHICS_EMCC)
#pragma message WARN("TODO: Linux X11/Wayland support not yet implemented")
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#if defined(GRAPHICS_HAS_X11VMEXT)
#include <X11/extensions/xf86vmode.h>
#endif

static Display* display = None;
static int screen = None;
static Window root_window = None;
static Cursor empty_cursor = None;

struct nix_window_t {
  Window window;
  Atom wm_del;
  GC gc;
  XImage* img;
  Cursor cursor;
  bool mouse_inside, cursor_locked, cursor_vis, closed;
  int depth, cursor_lx, cursor_ly;
  struct surface_t scaler;
  struct window_t* parent;
};

static void close_nix_window(struct nix_window_t* w) {
  if (w->closed)
    return;
  w->closed = true;
  if (w->scaler.buf)
    surface_destroy(&w->scaler);
  w->img->data = NULL;
  XDestroyImage(w->img);
  XDestroyWindow(display, w->window);
  XFlush(display);
}

LINKEDLIST(window, struct nix_window_t);
static struct window_node_t* windows = NULL;

#define Button6 6
#define Button7 7

static int translate_key_b(int sym) {
  switch (sym) {
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
    case XK_KP_Equal:       return KB_KEY_KP_EQUALS;
    case XK_KP_Enter:       return KB_KEY_KP_ENTER;
  }
  return KB_KEY_UNKNOWN;
}

static int translate_key_a(int sym) {
  switch (sym) {
    case XK_Escape:         return KB_KEY_ESCAPE;
    case XK_Tab:            return KB_KEY_TAB;
    case XK_Shift_L:        return KB_KEY_LEFT_SHIFT;
    case XK_Shift_R:        return KB_KEY_RIGHT_SHIFT;
    case XK_Control_L:      return KB_KEY_LEFT_CONTROL;
    case XK_Control_R:      return KB_KEY_RIGHT_CONTROL;
    case XK_Meta_L:
    case XK_Alt_L:          return KB_KEY_LEFT_ALT;
    case XK_Mode_switch:      // Mapped to Alt_R on many keyboards
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
    case XK_KP_Equal:       return KB_KEY_KP_EQUALS;
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
    case XK_equal:          return KB_KEY_EQUALS;
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
    default:                break;
  }
  return KB_KEY_UNKNOWN;
}

static int translate_key(int scancode) {
  return scancode < 0 || scancode > 255 ? KB_KEY_UNKNOWN : keycodes[scancode];
}

static int translate_mod(int state) {
  int mod_keys = 0;
  if (state & ShiftMask)
      mod_keys |= KB_MOD_SHIFT;
  if (state & ControlMask)
      mod_keys |= KB_MOD_CONTROL;
  if (state & Mod1Mask)
      mod_keys |= KB_MOD_ALT;
  if (state & Mod4Mask)
      mod_keys |= KB_MOD_SUPER;
  if (state & LockMask)
      mod_keys |= KB_MOD_CAPS_LOCK;
  if (state & Mod2Mask)
      mod_keys |= KB_MOD_NUM_LOCK;
  return mod_keys;
}

static int translate_mod_ex(int key, int state, int is_pressed) {
  int mod_keys = translate_mod(state);
  switch (key) {
    case KB_KEY_LEFT_SHIFT:
    case KB_KEY_RIGHT_SHIFT:
      if (is_pressed)
        mod_keys |= KB_MOD_SHIFT;
      else
        mod_keys &= ~KB_MOD_SHIFT;
      break;
    case KB_KEY_LEFT_CONTROL:
    case KB_KEY_RIGHT_CONTROL:
      if (is_pressed)
        mod_keys |= KB_MOD_CONTROL;
      else
        mod_keys &= ~KB_MOD_CONTROL;
      break;
    case KB_KEY_LEFT_ALT:
    case KB_KEY_RIGHT_ALT:
      if (is_pressed)
        mod_keys |= KB_MOD_ALT;
      else
        mod_keys &= ~KB_MOD_ALT;
      break;
    case KB_KEY_LEFT_SUPER:
    case KB_KEY_RIGHT_SUPER:
      if (is_pressed)
        mod_keys |= KB_MOD_SUPER;
      else
        mod_keys &= ~KB_MOD_SUPER;
      break;
  }
  return mod_keys;
}

static void get_cursor_pos(int* x, int* y) {
  Window in_win, in_child_win;
  Atom type_prop;
  int root_x, root_y, child_x, child_y;
  unsigned int mask, format;
  unsigned long n, sz;
  Window* props;
  XGetWindowProperty(display, root_window, XInternAtom(display, "_NET_ACTIVE_WINDOW", True), 0, 1, False, AnyPropertyType, &type_prop, &format, &n, &sz, (unsigned char**)&props);
  XQueryPointer(display, props[0], &in_win, &in_child_win, &root_x, &root_y, &child_x, &child_y, &mask);
  XFree(props);
  if (x)
    *x = root_x;
  if (y)
    *y = root_y;
}

struct Hints {
  unsigned long flags;
  unsigned long functions;
  unsigned long decorations;
  long input_mode;
  unsigned long status;
};

bool window(struct window_t* s, const char* t, int w, int h, short flags) {
  if (!keycodes_init) {
    if (!(display = XOpenDisplay(NULL))) {
      GRAPHICS_ERROR(NIX_WINDOW_CREATION_FAILED, "XOpenDisplay() failed");
      return false;
    }
    root_window = DefaultRootWindow(display);
    screen = DefaultScreen(display);

    memset(keycodes, -1, sizeof(keycodes));
    for (int i = 0; i < 512; ++i)
      keycodes[i] = KB_KEY_UNKNOWN;
    for (int i = 8; i < 256; ++i)
      if ((keycodes[i] = translate_key_b(XkbKeycodeToKeysym(display, i, 0, 1))) == KB_KEY_UNKNOWN)
        keycodes[i] = translate_key_a(XkbKeycodeToKeysym(display, i, 0, 0));

    char data[1] = { 0 };
    XColor color;
    color.red = color.green = color.blue = 0;
    Pixmap pixmap = XCreateBitmapFromData(display, root_window, data, 1, 1);
    if (!pixmap) {
      GRAPHICS_ERROR(NIX_CURSOR_PIXMAP_ERROR, "XCreateBitmapFromData() failed");
      return false;
    }
    empty_cursor = XCreatePixmapCursor(display, pixmap, pixmap, &color, &color, 0, 0);
    XFreePixmap(display, pixmap);

    keycodes_init = true;
  }

  struct nix_window_t* win_data = GRAPHICS_MALLOC(sizeof(struct nix_window_t));
  if (!win_data) {
    GRAPHICS_ERROR(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  int screen_w = DisplayWidth(display, screen);
  int screen_h = DisplayHeight(display, screen);

  if (flags & FULLSCREEN)
    flags = FULLSCREEN | BORDERLESS;

  int x = 0, y = 0;
  if (flags & FULLSCREEN || flags & FULLSCREEN_DESKTOP) {
    w = screen_w;
    h = screen_h;
  } else {
    x = screen_w / 2 - w / 2;
    y = screen_h / 2 - h / 2;
  }

  Visual* visual = DefaultVisual(display, screen);
  int format_c = 0;
  XPixmapFormatValues* formats = XListPixmapFormats(display, &format_c);
  int depth = DefaultDepth(display, screen);
  int depth_c;
  for (int i = 0; i < format_c; ++i)
    if (depth == formats[i].depth) {
      depth_c = formats[i].bits_per_pixel;
      break;
    }
  XFree(formats);

  if (depth_c != 32) {
    GRAPHICS_ERROR(NIX_WINDOW_CREATION_FAILED, "Invalid display depth: %d", depth_c);
    return false;
  }

  XSetWindowAttributes swa;
  swa.override_redirect = True;
  swa.border_pixel = BlackPixel(display, screen);
  swa.background_pixel = BlackPixel(display, screen);
  swa.backing_store = NotUseful;
  if (!(win_data->window = XCreateWindow(display, root_window, x, y, w, h, 0, depth, InputOutput, visual, CWBackPixel | CWBorderPixel | CWBackingStore, &swa))) {
    GRAPHICS_ERROR(NIX_WINDOW_CREATION_FAILED, "XCreateWindow() failed");
    return false;
  }

  win_data->wm_del = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, win_data->window, &win_data->wm_del, 1);
  
  XSelectInput(display, win_data->window, StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask | ExposureMask | FocusChangeMask | EnterWindowMask | LeaveWindowMask);
  XStoreName(display, win_data->window, t);

  if (flags & FULLSCREEN) {
#if defined(GRAPHICS_HAS_X11VMEXT)
    int modes_n, best_mode = 0;
    XF86VidModeModeInfo** modes;
    XF86VidModeGetAllModeLines(display, screen, &modes_n, &modes);
    for (int i = 0; i < modes_n; ++i) {
      if (modes[i]->hdisplay == w && modes[i]->vdisplay == h)
        best_mode = i;
    }
    XF86VidModeSwitchToMode(display, screen, modes[best_mode]);
    XF86VidModeSetViewPort(display, screen, 0, 0);
    XMoveResizeWindow(display, win_data->win, 0, 0, w, h);
    XMapRaised(display, win_data->win);
    XGrabPointer(display, win_data->win, True, 0, GrabModeAsync, GrabModeAsync, win_data->win, 0L, CurrentTime);
    XGrabKeyboard(display, win_data->win, False, GrabModeAsync, GrabModeAsync, CurrentTime);
#else
    Atom p = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty(display, win_data->window, XInternAtom(display, "_NET_WM_STATE", True), XA_ATOM, 32, PropModeReplace, (unsigned char*)&p, 1);
#endif
  }

  if (flags & BORDERLESS) {
    struct Hints hints;
    hints.flags = 2;
    hints.decorations = 0;
    Atom p = XInternAtom(display, "_MOTIF_WM_HINTS", True);
    XChangeProperty(display, win_data->window, p, p, 32, PropModeReplace, (unsigned char*)&hints, 5);
  }

  if (flags & ALWAYS_ON_TOP) {
    Atom p = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(display, win_data->window, XInternAtom(display, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (unsigned char *)&p, 1);
  }

  XSizeHints hints;
  hints.flags = PPosition | PMinSize | PMaxSize;
  hints.x = 0;
  hints.y = 0;
  if (flags & RESIZABLE) {
    hints.min_width = 0;
    hints.min_height = 0;
    hints.max_width = screen_w;
    hints.max_height = screen_h;
  } else {
    hints.min_width = w;
    hints.min_height = h;
    hints.max_width = w;
    hints.max_height = h;
  }
  XSetWMNormalHints(display, win_data->window, &hints);
  XClearWindow(display, win_data->window);
  XMapRaised(display, win_data->window);
  XFlush(display);
  win_data->gc = DefaultGC(display, screen);
  win_data->cursor = XCreateFontCursor(display, XC_left_ptr);
  get_cursor_pos(&win_data->cursor_lx, &win_data->cursor_ly);
  win_data->img = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, NULL, w, h, 32, w * 4);
  win_data->depth = depth;
  memset(&win_data->scaler, 0, sizeof(struct surface_t));
  win_data->closed = false;

  windows = window_push(windows, win_data);
  s->w = w;
  s->h = h;
  s->id = (int)win_data->window;
  s->window = win_data;
  win_data->parent = s;

  return true;
}

void window_icon(struct window_t* w, struct surface_t* b) {
  return;
}

void window_title(struct window_t* w, const char* t) {
  struct nix_window_t* win = (struct nix_window_t*)w->window;
  XStoreName(display, win->window, t);
}

void window_position(struct window_t* w, int* x, int* y) {
  struct nix_window_t* win = (struct nix_window_t*)w->window;
  static int wx, wy;
  static XWindowAttributes xwa;
  static Window child;
  XTranslateCoordinates(display, win->window, root_window, 0, 0, &wx, &wy, &child);
  XGetWindowAttributes(display, win->window, &xwa);
  if (x)
    *x = wx - xwa.x;
  if (y)
    *y = wy - xwa.y;
}

void screen_size(struct window_t* s, int* w, int* h) {
  if (w)
    *w = DisplayWidth(display, screen);
  if (h)
    *h = DisplayHeight(display, screen);
}

void window_destroy(struct window_t* w) {
  struct nix_window_t* win = (struct nix_window_t*)w->window;
  close_nix_window(win);
  GRAPHICS_SAFE_FREE(win);
  w->window = NULL;
}

bool closed(struct window_t* w) {
  return ((struct nix_window_t*)w->window)->closed;
}

bool closed_va(int n, ...) {
  va_list args;
  va_start(args, n);
  bool ret = true;
  for (int i = 0; i < n; ++i) {
    struct window_t* w = va_arg(args, struct window_t*);
    if (!((struct nix_window_t*)w->window)->closed) {
      ret = false;
      break;
    }
  }
  va_end(args);
  return ret;
}

bool closed_all() {
  return windows == NULL;
}

void cursor_lock(struct window_t* w, bool lock) {
  return;
}

void cursor_visible(struct window_t* w, bool visible) {
  return;
}

void cursor_icon(struct window_t* w, enum cursor_type type) {
  return;
}

void cursor_icon_custom(struct window_t* w, struct surface_t* b) {
  return;
}

void cursor_pos(int* x, int* y) {
  get_cursor_pos(x, y);
}

void cursor_set_pos(int x, int y) {
  return;
}

struct window_t* event_window(Window w) {
  struct window_node_t* cursor = windows;
  while (cursor) {
    if (cursor->data->window == w)
      return cursor->data->parent;
    cursor = cursor->next;
  }
  return NULL;
}

void events() {
  static XEvent e;
  static struct window_t* e_window = NULL;
  static struct nix_window_t* e_data = NULL;
  while (XPending(display)) {
    XNextEvent(display, &e);
    if (!(e_window = event_window(e.xclient.window)))
      continue;
    if (!(e_data = (struct nix_window_t*)e_window->window))
      continue;
    if (e_data->closed)
      continue;
    switch (e.type) {
      case KeyPress:
      case KeyRelease: {
        static bool pressed = false;
        pressed = e.type == KeyPress;
        CBCALL(keyboard_callback, translate_key(e.xkey.keycode), translate_mod_ex(e.xkey.keycode, e.xkey.state, pressed), pressed);
        break;
      }
      case ButtonPress:
      case ButtonRelease:
        switch (e.xbutton.button) {
          case Button1:
          case Button2:
          case Button3:
            CBCALL(mouse_button_callback, (enum button)e.xbutton.button, translate_mod(e.xkey.state), e.type == ButtonPress);
            break;
          case Button4:
            CBCALL(scroll_callback, translate_mod(e.xkey.state), 0.f, 1.f);
            break;
          case Button5:
            CBCALL(scroll_callback, translate_mod(e.xkey.state), 0.f, -1.f);
            break;
          case Button6:
            CBCALL(scroll_callback, translate_mod(e.xkey.state), 1.f, 0.f);
            break;
          case Button7:
            CBCALL(scroll_callback, translate_mod(e.xkey.state), -1.f, 0.f);
            break;
          default:
            CBCALL(mouse_button_callback, (enum button)(e.xbutton.button - 4), translate_mod(e.xkey.state), e.type == ButtonPress);
            break;
        }
        break;
      case ConfigureNotify: {
        static int w = 0, h = 0;
        w = e.xconfigure.width;
        h = e.xconfigure.height;
        if (e_window->w == w && e_window->h == h)
          break;
        CBCALL(resize_callback, w, h);
        e_window->w = w;
        e_window->h = h;
        if (e_data->img) {
          e_data->img->data = NULL;
          XDestroyImage(e_data->img);
        }
        if (e_data->scaler.buf)
          surface_destroy(&e_data->scaler);
        e_data->img = XCreateImage(display, CopyFromParent, e_data->depth, ZPixmap, 0, NULL, w, h, 32, w * 4);
        surface(&e_data->scaler, w, h);
        break;
      }
      case EnterNotify:
      case LeaveNotify:
        e_data->mouse_inside = e.type == EnterNotify;
        break;
      case FocusIn:
      case FocusOut:
        CBCALL(focus_callback, e.type == FocusIn);
        break;
      case MotionNotify: {
        static int cx = 0, cy = 0;
        cx = e.xmotion.x;
        cy = e.xmotion.y;
        CBCALL(mouse_move_callback, cx, cy, cx - e_data->cursor_lx, cy - e_data->cursor_ly);
        e_data->cursor_lx = cx;
        e_data->cursor_ly = cy;
        break;
      }
      case ClientMessage:
        if (e.xclient.data.l[0] != e_data->wm_del)
          break;
        close_nix_window(e_data);
        windows = window_pop(windows, e_data);
        if (e_window && e_window->closed_callback)
          e_window->closed_callback(e_window->parent);
        break;
    }
  }
}

void flush(struct window_t* w, struct surface_t* b) {
  if (!w)
    return;
  struct nix_window_t* tmp = (struct nix_window_t*)w->window;
  if (!tmp || tmp->closed)
    return;
  if (b->w != w->w || b->h != w->h) {
    __resize(b, &tmp->scaler);
    tmp->img->data = (char*)tmp->scaler.buf;
  } else
    tmp->img->data = (char*)b->buf;
  XPutImage(display, tmp->window, tmp->gc, tmp->img, 0, 0, 0, 0, w->w, w->h);
  XFlush(display);
}

void release() {
  struct window_node_t *tmp = NULL, *cursor = windows;
  while (cursor) {
    tmp = cursor->next;
    close_nix_window(tmp->data);
    GRAPHICS_SAFE_FREE(cursor->data);
    GRAPHICS_SAFE_FREE(cursor);
    cursor = tmp;
  }
  if (display)
    XCloseDisplay(display);
}
#elif defined(GRAPHICS_EMCC)
#if !defined(GRAPHICS_CANVAS_NAME)
#if defined(GRAPHICS_CANVAS_ID)
#error GRAPHICS_CANVAS_ID cannot be pre-defined unless GRAPHICS_CANVAS_NAME is also pre-defined
#endif
#define GRAPHICS_CANVAS_NAME "canvas"
#endif
#define GRAPHICS_CANVAS_ID "#" GRAPHICS_CANVAS_NAME

#include <emscripten.h>
#include <emscripten/html5.h>

static int window_w, window_h, canvas_w, canvas_h, canvas_x, canvas_y, cursor_x, cursor_y;
static bool mouse_in_canvas = true, fullscreen = false;

static enum key_mod translate_mod(bool ctrl, bool shift, bool alt, bool meta) {
  return (enum key_mod)((ctrl ? KB_MOD_CONTROL : 0) | (shift ? KB_MOD_SHIFT : 0) | (alt ? KB_MOD_ALT : 0) | (meta ? KB_MOD_SUPER : 0));
}

static enum key_sym translate_key(int key) {
  return (key > 222 || key < 8 ? KB_KEY_UNKNOWN : keycodes[key]);
}

static EM_BOOL key_callback(int type, const EmscriptenKeyboardEvent* e, void* user_data) {
  static enum key_mod mod;
  static enum key_sym sym;
  mod = translate_mod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey);
  sym = translate_key(e->keyCode);
  CBCALL(keyboard_callback, sym, mod, (type == EMSCRIPTEN_EVENT_KEYDOWN));

  switch (sym) {
    case KB_KEY_R: // Reload
    case KB_KEY_W: // Close tab (Just in case)
    case KB_KEY_Q: // Close window (Just in case)
#if defined(GRAPHICS_OSX)
      return (mod == KB_MOD_SUPER);
#else
      return (mod == KB_MOD_CONTROL);
#endif
#if defined(GRAPHICS_DEBUG) && defined(GRAPHICS_EMCC_HTML)
    case KB_KEY_F12: // Developer tools Edge
      return true;
    case KB_KEY_I: // Developer tools Chrome/Firefox
    case KB_KEY_J: // Developer tools Chrome
    case KB_KEY_C: // Developer tools Safari/Firefox
    case KB_KEY_K: // Developer tools Firefox
#if defined(GRAPHICS_OSX)
      return (mod == (KB_MOD_SUPER & KB_MOD_ALT));
#else
      return (mod == (KB_MOD_SUPER & KB_MOD_SHIFT));
#endif
#endif
    default:
      return false;
  }
}

static EM_BOOL mouse_callback(int type, const EmscriptenMouseEvent* e, void* user_data) {
  switch (type) {
    case EMSCRIPTEN_EVENT_MOUSEDOWN:
      if (mouse_in_canvas && e->buttons != 0)
        CBCALL(mouse_button_callback, (enum button)(e->button + 1), translate_mod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), true);
      break;
    case EMSCRIPTEN_EVENT_MOUSEUP:
      if (mouse_in_canvas)
        CBCALL(mouse_button_callback, (enum button)(e->button + 1), translate_mod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), false);
      break;
    case EMSCRIPTEN_EVENT_MOUSEMOVE:
      cursor_x = e->clientX;
      cursor_y = e->clientY;
      if (mouse_in_canvas)
        CBCALL(mouse_move_callback, e->clientX, e->clientY, e->movementX, e->movementY);
      break;
    case EMSCRIPTEN_EVENT_MOUSEENTER:
      mouse_in_canvas = true;
      return true;
    case EMSCRIPTEN_EVENT_MOUSELEAVE:
      mouse_in_canvas = false;
      return false;
    case EMSCRIPTEN_EVENT_CLICK:
    case EMSCRIPTEN_EVENT_DBLCLICK:
    default:
      return true;
  }
  return true;
}

static EM_BOOL wheel_callback(int type, const EmscriptenWheelEvent* e, void* user_data) {
  if (!mouse_in_canvas)
    return false;
  CBCALL(scroll_callback, translate_mod(e->mouse.ctrlKey, e->mouse.shiftKey, e->mouse.altKey, e->mouse.metaKey), e->deltaX, e->deltaY);
  return true;
}

static EM_BOOL uievent_callback(int type, const EmscriptenUiEvent* e, void* user_data) {
  window_w = EM_ASM_INT_V({ return window.innerWidth; });
  window_h = EM_ASM_INT_V({ return window.innerHeight; });
  canvas_x = EM_ASM_INT({ return Module['canvas'].getBoundingClientRect().left });
  canvas_y = EM_ASM_INT({ return Module['canvas'].getBoundingClientRect().top });
  if (fullscreen) {
    canvas_w  = window_w;
    canvas_h = window_h;
  } else {
    static double css_w, css_h;
    emscripten_get_element_css_size(GRAPHICS_CANVAS_ID, &css_w, &css_h);
    canvas_w  = (int)css_w;
    canvas_h = (int)css_h;
  }
  return true;
}

static EM_BOOL focusevent_callback(int type, const EmscriptenFocusEvent* e, void* user_data) {
  CBCALL(focus_callback, (type == EMSCRIPTEN_EVENT_FOCUS));
  return true;
}

static EM_BOOL fullscreenchange_callback(int type, const EmscriptenFullscreenChangeEvent* e, void* user_data) {
  fullscreen = e->isFullscreen;
  return true;
}

static EM_BOOL pointerlockchange_callback(int type, const EmscriptenPointerlockChangeEvent* e, void* user_data) {
  return true;
}

static const char* beforeunload_callback(int type, const void* reserved, void* user_data) {
  return "Do you really want to leave the page?";
}

static EM_BOOL webglcontext_callback(int type, const void* reserved, void* user_data) {
#pragma message WARN("TODO: webglcontext_callback() handle error")
  return 0;
}

static bool window_already_open = false;

bool window(struct window_t* s, const char* t, int w, int h, short flags) {
  if (window_already_open) {
#pragma message WARN("TODO: window() handle error")
    return false;
  }
  
  for (int i = 0; i < KB_KEY_LAST; ++i)
    keycodes[i] = KB_KEY_UNKNOWN;
  
  keycodes[8] = KB_KEY_BACKSPACE;
  keycodes[9] = KB_KEY_TAB;
  keycodes[13] = KB_KEY_ENTER;
  keycodes[16] = KB_KEY_LEFT_SHIFT;
  keycodes[17] = KB_KEY_LEFT_CONTROL;
  keycodes[18] = KB_KEY_LEFT_ALT;
  keycodes[19] = KB_KEY_PAUSE;
  keycodes[20] = KB_KEY_CAPS_LOCK;
  keycodes[27] = KB_KEY_ESCAPE;
  keycodes[32] = KB_KEY_SPACE;
  keycodes[33] = KB_KEY_PAGE_UP;
  keycodes[34] = KB_KEY_PAGE_DOWN;
  keycodes[35] = KB_KEY_END;
  keycodes[36] = KB_KEY_HOME;
  keycodes[37] = KB_KEY_LEFT;
  keycodes[38] = KB_KEY_UP;
  keycodes[39] = KB_KEY_RIGHT;
  keycodes[40] = KB_KEY_DOWN;
  keycodes[45] = KB_KEY_INSERT;
  keycodes[46] = KB_KEY_DELETE;
  keycodes[48] = KB_KEY_0;
  keycodes[49] = KB_KEY_1;
  keycodes[50] = KB_KEY_2;
  keycodes[51] = KB_KEY_3;
  keycodes[52] = KB_KEY_4;
  keycodes[53] = KB_KEY_5;
  keycodes[54] = KB_KEY_6;
  keycodes[55] = KB_KEY_7;
  keycodes[56] = KB_KEY_8;
  keycodes[57] = KB_KEY_9;
  keycodes[59] = KB_KEY_SEMICOLON;
  keycodes[61] = KB_KEY_EQUALS;
  keycodes[65] = KB_KEY_A;
  keycodes[66] = KB_KEY_B;
  keycodes[67] = KB_KEY_C;
  keycodes[68] = KB_KEY_D;
  keycodes[69] = KB_KEY_E;
  keycodes[70] = KB_KEY_F;
  keycodes[71] = KB_KEY_G;
  keycodes[72] = KB_KEY_H;
  keycodes[73] = KB_KEY_I;
  keycodes[74] = KB_KEY_J;
  keycodes[75] = KB_KEY_K;
  keycodes[76] = KB_KEY_L;
  keycodes[77] = KB_KEY_M;
  keycodes[78] = KB_KEY_N;
  keycodes[79] = KB_KEY_O;
  keycodes[80] = KB_KEY_P;
  keycodes[81] = KB_KEY_Q;
  keycodes[82] = KB_KEY_R;
  keycodes[83] = KB_KEY_S;
  keycodes[84] = KB_KEY_T;
  keycodes[85] = KB_KEY_U;
  keycodes[86] = KB_KEY_V;
  keycodes[87] = KB_KEY_W;
  keycodes[88] = KB_KEY_X;
  keycodes[89] = KB_KEY_Y;
  keycodes[90] = KB_KEY_Z;
  keycodes[96] = KB_KEY_KP_0;
  keycodes[97] = KB_KEY_KP_1;
  keycodes[98] = KB_KEY_KP_2;
  keycodes[99] = KB_KEY_KP_3;
  keycodes[100] = KB_KEY_KP_4;
  keycodes[101] = KB_KEY_KP_5;
  keycodes[102] = KB_KEY_KP_6;
  keycodes[103] = KB_KEY_KP_7;
  keycodes[104] = KB_KEY_KP_8;
  keycodes[105] = KB_KEY_KP_9;
  keycodes[106] = KB_KEY_KP_MULTIPLY;
  keycodes[107] = KB_KEY_KP_ADD;
  keycodes[109] = KB_KEY_KP_SUBTRACT;
  keycodes[110] = KB_KEY_KP_DECIMAL;
  keycodes[111] = KB_KEY_KP_DIVIDE;
  keycodes[112] = KB_KEY_F1;
  keycodes[113] = KB_KEY_F2;
  keycodes[114] = KB_KEY_F3;
  keycodes[115] = KB_KEY_F4;
  keycodes[116] = KB_KEY_F5;
  keycodes[117] = KB_KEY_F6;
  keycodes[118] = KB_KEY_F7;
  keycodes[119] = KB_KEY_F8;
  keycodes[120] = KB_KEY_F9;
  keycodes[121] = KB_KEY_F10;
  keycodes[122] = KB_KEY_F11;
  keycodes[123] = KB_KEY_F12;
  keycodes[124] = KB_KEY_F13;
  keycodes[125] = KB_KEY_F14;
  keycodes[126] = KB_KEY_F15;
  keycodes[127] = KB_KEY_F16;
  keycodes[128] = KB_KEY_F17;
  keycodes[129] = KB_KEY_F18;
  keycodes[130] = KB_KEY_F19;
  keycodes[131] = KB_KEY_F20;
  keycodes[132] = KB_KEY_F21;
  keycodes[133] = KB_KEY_F22;
  keycodes[134] = KB_KEY_F23;
  keycodes[135] = KB_KEY_F24;
  keycodes[144] = KB_KEY_NUM_LOCK;
  keycodes[145] = KB_KEY_SCROLL_LOCK;
  keycodes[173] = KB_KEY_MINUS;
  keycodes[186] = KB_KEY_SEMICOLON;
  keycodes[187] = KB_KEY_EQUALS;
  keycodes[188] = KB_KEY_COMMA;
  keycodes[189] = KB_KEY_MINUS;
  keycodes[190] = KB_KEY_PERIOD;
  keycodes[191] = KB_KEY_SLASH;
  keycodes[192] = KB_KEY_GRAVE_ACCENT;
  keycodes[219] = KB_KEY_LEFT_BRACKET;
  keycodes[220] = KB_KEY_BACKSLASH;
  keycodes[221] = KB_KEY_RIGHT_BRACKET;
  keycodes[222] = KB_KEY_APOSTROPHE;
  
  emscripten_set_keypress_callback(0, 0, 1, key_callback);
  emscripten_set_keydown_callback(0, 0, 1, key_callback);
  emscripten_set_keyup_callback(0, 0, 1, key_callback);
  
  emscripten_set_click_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mousedown_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseup_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_dblclick_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mousemove_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseenter_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseleave_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseover_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseout_callback(GRAPHICS_CANVAS_ID, 0, 1, mouse_callback);
  
  emscripten_set_wheel_callback(0, 0, 1, wheel_callback);
  
  emscripten_set_resize_callback(0, 0, 1, uievent_callback);
  emscripten_set_scroll_callback(0, 0, 1, uievent_callback);
  
  emscripten_set_blur_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focus_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focusin_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focusout_callback(0, 0, 1, focusevent_callback);
  
  emscripten_set_fullscreenchange_callback(GRAPHICS_CANVAS_ID, 0, 1, fullscreenchange_callback);
  
  emscripten_set_pointerlockchange_callback(0, 0, 1, pointerlockchange_callback);
  
  emscripten_set_beforeunload_callback(0, beforeunload_callback);
  
  emscripten_set_webglcontextlost_callback(0, 0, 1, webglcontext_callback);
  emscripten_set_webglcontextrestored_callback(0, 0, 1, webglcontext_callback);
  
  EM_ASM(Module['noExitRuntime'] = true);
  
#if defined(GRAPHICS_OPENGL)
#pragma message WARN("TODO: Emscripten OpenGL not yet implemented")
#endif
  
  if (t)
    window_title(NULL, t);
  
  emscripten_set_canvas_element_size(GRAPHICS_CANVAS_ID, w, h);
  EMSCRIPTEN_FULLSCREEN_SCALE fs_scale = EMSCRIPTEN_FULLSCREEN_SCALE_DEFAULT;
  bool fs = false, soft_fs = false;
  if (flags & FULLSCREEN_DESKTOP) {
    fs_scale = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
    fs = true;
  }
  if (flags & FULLSCREEN) {
    fs_scale = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
    fs = soft_fs = true;
  }
  if (fs) {
    EmscriptenFullscreenStrategy fsf;
    memset(&fsf, 0, sizeof(fsf));
    fsf.scaleMode = fs_scale;
    fsf.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
    fsf.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST;
    fsf.canvasResizedCallback = uievent_callback;
    if (soft_fs)
      emscripten_enter_soft_fullscreen(0, &fsf);
    else
      emscripten_request_fullscreen_strategy(0, 1, &fsf);
  }
  uievent_callback(0, NULL, NULL);
  
  window_already_open = true;
  return true;
}

void window_icon(struct window_t* _, struct surface_t* __) {
#pragma message WARN("window_icon() unsupported on emscripten")
}

void window_title(struct window_t* _, const char* t) {
  EM_ASM({
    setWindowTitle(UTF8ToString($0));
  }, t);
}

void window_position(struct window_t* _, int* x, int*  y) {
  if (x)
    *x = canvas_x;
  if (y)
    *y = canvas_y;
}

void screen_size(struct window_t* _, int* w, int* h) {
  if (w)
    *w = window_w;
  if (h)
    *h = window_h;
}

void window_destroy(struct window_t* s) {
  memset(s, 0, sizeof(struct window_t));
}

bool closed(struct window_t* _) {
  return false;
}

bool closed_va(int n, ...) {
  return false;
}

bool closed_all() {
  return false;
}

void cursor_lock(struct window_t* s, bool locked) {
#pragma message WARN("cursor_lock() unsupported on emscripten")
  if (locked)
    emscripten_request_pointerlock(NULL, 1);
  else
    emscripten_exit_pointerlock();
}

static const char* cursor = "default";
static bool cursor_custom = false;

void cursor_visible(struct window_t* s, bool show) {
  EM_ASM({
    if (Module['canvas']) {
      Module['canvas'].style['cursor'] = ($1 ? UTF8ToString($0) : 'none');
    }
  }, cursor, show);
}

void cursor_icon(struct window_t* w, enum cursor_type t) {
  if (cursor_custom && cursor)
    GRAPHICS_SAFE_FREE(cursor);
  cursor_custom = false;
  
  switch (t) {
    default:
    case CURSOR_ARROW:
      cursor = "default";
    case CURSOR_WAIT:
      cursor = "wait";
      break;
    case CURSOR_WAITARROW:
      cursor = "progress";
      break;
    case CURSOR_IBEAM:
      cursor = "text";
      break;
    case CURSOR_CROSSHAIR:
      cursor = "crosshair";
      break;
    case CURSOR_SIZENWSE:
      cursor = "nwse-resize";
      break;
    case CURSOR_SIZENESW:
      cursor = "nesw-resize";
      break;
    case CURSOR_SIZEWE:
      cursor = "ew-resize";
      break;
    case CURSOR_SIZENS:
      cursor = "ns-resize";
      break;
    case CURSOR_SIZEALL:
      cursor = "move";
      break;
    case CURSOR_NO:
      cursor = "not-allowed";
      break;
    case CURSOR_HAND:
      cursor = "pointer";
      break;
  }
  cursor_visible(w, true);
}

void cursor_icon_custom(struct window_t* w, struct surface_t* b) {
  if (cursor_custom && cursor)
    GRAPHICS_SAFE_FREE(cursor);
  
  cursor = (const char*)EM_ASM_INT({
    var w = $0;
    var h = $1;
    var pixels = $2;
    var ctx = canvas.getContext("2d");
    var canvas = document.createElement("canvas");
    canvas.width = w;
    canvas.height = h;
    var image = ctx.createImageData(w, h);
    var data = image.data;
    var src = pixels >> 2;
    var dst = 0;
    var num = data.length;
    while (dst < num) {
      var val = HEAP32[src];
      data[dst  ] = (val >> 16) & 0xFF;
      data[dst+1] = (val >> 8) & 0xFF;
      data[dst+2] = val & 0xFF;
      data[dst+3] = 0xFF;
      src++;
      dst += 4;
    }
    
    ctx.putImageData(image, 0, 0);
    var url = "url(" + canvas.toDataURL() + "), auto";
    var url_buf = _malloc(url.length + 1);
    stringToUTF8(url, url_buf, url.length + 1);
    
    return url_buf;
  }, b->w, b->h, b->buf);
  if (!cursor) {
    GRAPHICS_ERROR(UNKNOWN_ERROR, "cursor_custom_icon() failed");
    cursor = "default";
    cursor_custom = false;
    return;
  }
  cursor_custom = true;
  cursor_visible(w, true);
}

void cursor_pos(int* x, int* y) {
  if (x)
    *x = cursor_x;
  if (y)
    *y = cursor_y;
}

void cursor_set_pos(int _, int __) {
#pragma message WARN("cursor_set_pos() unsupported on emscripten")
}

void events(void) {
#if defined(GRAPHICS_DEBUG) && defined(GRAPHICS_EMCC_HTML)
  EM_ASM({
    stats.begin();
  });
#endif
}

void flush(struct window_t* _, struct surface_t* b) {
  EM_ASM({
    var w = $0;
    var h = $1;
    var buf = $2;
    var src = buf >> 2;
    var canvas = document.getElementById("canvas");
    var ctx = canvas.getContext("2d");
    var img = ctx.createImageData(w, h);
    var data = img.data;
    
    var i = 0;
    var j = data.length;
    while (i < j) {
      var val = HEAP32[src];
      data[i  ] = (val >> 16) & 0xFF;
      data[i+1] = (val >> 8) & 0xFF;
      data[i+2] = val & 0xFF;
      data[i+3] = 0xFF;
      src++;
      i += 4;
    }

    ctx.putImageData(img, 0, 0);
#if defined(GRAPHICS_DEBUG) && defined(GRAPHICS_EMCC_HTML)
    stats.end();
#endif
  }, b->w, b->h, b->buf);
}

void release(void) {
  return;
}
#elif defined(GRAPHICS_SIXEL)
bool window(struct window_t* a, const char* b, int c, int d, short e) {
  static bool window_already_open = false;
  if (window_already_open) {
#pragma message WARN("TODO: window() handle error")
    return false;
  }
  return true;
}

void window_icon(struct window_t* a, struct surface_t* b) {
  return;
}

void window_title(struct window_t* a, const char* b) {
  return;
}

void window_position(struct window_t* a, int* b, int*  c) {
  return;
}

void screen_size(struct window_t* a, int* b, int* c) {
  return;
}

void window_destroy(struct window_t* a) {
  return;
}

bool closed(struct window_t* a) {
  return false;
}

bool closed_va(int n, ...) {
  return false;
}

bool closed_all() {
  return false;
}

void cursor_lock(struct window_t* s, bool a) {
  return;
}

void cursor_visible(struct window_t* s, bool a) {
  return;
}

void cursor_icon(struct window_t* a, enum cursor_type b) {
  return;
}

void cursor_custom_icon(struct window_t* a, struct surface_t* b) {
  return;
}

void cursor_pos(int* a, int* b) {
  return;
}

void cursor_set_pos(int a, int b) {
  return;
}

void events() {
  return;
}

void flush(struct window_t* a, struct surface_t* b) {
  return;
}

void release() {
  return;
}
#else // dummy
bool window(struct window_t* a, const char* b, int c, int d, short e) {
  return true;
}

void window_icon(struct window_t* a, struct surface_t* b) {
  return;
}

void window_title(struct window_t* a, const char* b) {
  return;
}

void window_position(struct window_t* a, int* b, int*  c) {
  return;
}

void screen_size(struct window_t* a, int* b, int* c) {
  return;
}

void window_destroy(struct window_t* a) {
  return;
}

bool closed(struct window_t* a) {
  return false;
}

bool closed_va(int n, ...) {
  return false;
}

bool closed_all() {
  return false;
}

void cursor_lock(struct window_t* s, bool a) {
  return;
}

void cursor_visible(struct window_t* s, bool a) {
  return;
}

void cursor_icon(struct window_t* a, enum cursor_type b) {
  return;
}

void cursor_custom_icon(struct window_t* a, struct surface_t* b) {
  return;
}

void cursor_pos(int* a, int* b) {
  return;
}

void cursor_set_pos(int a, int b) {
  return;
}

void events() {
  return;
}

void flush(struct window_t* a, struct surface_t* b) {
  return;
}

void release() {
  return;
}
#endif

static void(*__error_callback)(enum graphics_error, const char*, const char*, const char*, int) = NULL;

void graphics_error_callback(void(*cb)(enum graphics_error, const char*, const char*, const char*, int)) {
  __error_callback = cb;
}

void graphics_error(enum graphics_error type, const char* file, const char* func, int line, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  static char error[1024];
  vsprintf((char*)error, msg, args);
  va_end(args);
  
#if defined(GRAPHICS_DEBUG)
  fprintf(stderr, "[%d] from %s in %s() at %d -- %s\n", type, file, func, line, error);
#endif
  if (__error_callback) {
    __error_callback(type, (const char*)error, __FILE__, __FUNCTION__, __LINE__);
    return;
  }
  abort();
}
