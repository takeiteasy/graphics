
//  graphics.c
//  graphics
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#include "graphics.h"

#define XYSET(s, x, y, v) (s->buf[(y) * s->w + (x)] = (v))
#define XYSETSAFE(s, x, y, v) \
if (x > 0 && y > 0 && x <= s->w && y <= s->h) \
  s->buf[(y) * s->w + (x)] = (v);
#define XYGET(s, x, y) (s->buf[(y) * s->w + (x)])

static char last_error[1024];

const char* get_last_error() {
  return last_error;
}

#define SET_LAST_ERROR(MSG, ...) \
memset(last_error, 0, 1024); \
sprintf(last_error, "[ERROR] from %s in %s at %d -- " MSG, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

static unsigned int chroma_key = -1;

static bool ticks_started = false;

static const char* font_b64_str = "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";
static const int font_b64_strlen = 2928;
static bool font[256][8][8];

#define LINE_HEIGHT 10

const static unsigned char b64_table[] = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 10
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 20
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 30
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 40
  0,   0,   0,   62,  0,   0,   0,   63,  52,  53, // 50
  54,  55,  56,  57,  58,  59,  60,  61,  0,   0,  // 60
  0,   0,   0,   0,   0,   0,   1,   2,   3,   4,  // 70
  5,   6,   7,   8,   9,   10,  11,  12,  13,  14, // 80
  15,  16,  17,  18,  19,  20,  21,  22,  23,  24, // 90
  25,  0,   0,   0,   0,   0,   0,   26,  27,  28, // 100
  29,  30,  31,  32,  33,  34,  35,  36,  37,  38, // 110
  39,  40,  41,  42,  43,  44,  45,  46,  47,  48, // 120
  49,  50,  51,  0,   0,   0,   0,   0,   0,   0,  // 130
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 140
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 150
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 160
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 170
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 180
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 190
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 200
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 210
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 220
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 230
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 240
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  // 250
  0,   0,   0,   0,   0,   0,
};

#define SWAP_POINTERS(x, y) { \
  void* t; \
  t = x; \
  x = y; \
  y = t; \
}

static int mx = 0, my = 0;
static void (*__mouse_move_cb)(int, int);
static void (*__mouse_enter_cb)(bool);
static void (*__mouse_down_cb)(MOUSE_e, MOD_e);
static void (*__mouse_up_cb)(MOUSE_e, MOD_e);
static void (*__key_down_cb)(KEY_e, MOD_e);
static void (*__key_up_cb)(KEY_e, MOD_e);

void mouse_move_cb(void (*fn)(int, int)) {
  SWAP_POINTERS(__mouse_move_cb, fn);
}

void mouse_entered_cb(void (*fn)(bool)) {
  SWAP_POINTERS(__mouse_enter_cb, fn);
}

void mouse_down_cb(void (*fn)(MOUSE_e, MOD_e)) {
  SWAP_POINTERS(__mouse_down_cb, fn);
}

void mouse_up_cb(void (*fn)(MOUSE_e, MOD_e)) {
  SWAP_POINTERS(__mouse_up_cb, fn);
}

void set_chroma_key(unsigned int c) {
  chroma_key = c;
}

void mouse_pos(int* x, int* y) {
  if (!x || !y)
    return;
  *x = mx;
  *y = my;
}

void key_down_cb(void (*fn)(KEY_e, MOD_e)) {
  SWAP_POINTERS(__key_down_cb, fn);
}

void key_up_cb(void (*fn)(KEY_e, MOD_e)) {
  SWAP_POINTERS(__key_up_cb, fn);
}

unsigned char* base64_decode(const char* ascii, int len, int *flen) {
  int cb = 0, pad = 0, n, A, B, C, D;;
  
  if (len < 2) {
    SET_LAST_ERROR("base64_decode() failed. Base64 string too short.");
    *flen = 0;
    return NULL;
  }
  
  if (ascii[len - 1] == '=')
    ++pad;
  if (ascii[len - 2] == '=')
    ++pad;
  
  *flen = 3 * len / 4 - pad;
  unsigned char* bin = (unsigned char*)malloc(*flen);
  if (!bin) {
    SET_LAST_ERROR("malloc() failed.");
    return NULL;
  }
  
  for (n = 0; n <= len - 4 - pad; n += 4) {
    A = b64_table[ascii[n]];
    B = b64_table[ascii[n + 1]];
    C = b64_table[ascii[n + 2]];
    D = b64_table[ascii[n + 3]];
    
    bin[cb++] = (A << 2) | (B >> 4);
    bin[cb++] = (B << 4) | (C >> 2);
    bin[cb++] = (C << 6) | (D);
  }
  
  if (pad == 1) {
    A = b64_table[ascii[n]];
    B = b64_table[ascii[n + 1]];
    C = b64_table[ascii[n + 2]];
    
    bin[cb++] = (A << 2) | (B >> 4);
    bin[cb++] = (B << 4) | (C >> 2);
  } else if(pad == 2) {
    A = b64_table[ascii[n]];
    B = b64_table[ascii[n + 1]];
    
    bin[cb++] = (A << 2) | (B >> 4);
  }
  
  return bin;
}

void init_default_font() {
  int size;
  unsigned char* f = base64_decode(font_b64_str, font_b64_strlen, &size);
  surface_t* img = bmp_mem(f);
  
  size_t c, x, y;
  for(c = 0; c < 256; ++c)
    for (x = 0; x < 8; ++x)
      for (y = 0; y < 8; ++y)
        font[c][x][y] = (XYGET(img, ((c % 16) * 8) + x, ((c / 16) * 8) + y) != 0);
  
  free(f);
  destroy(&img);
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

bool blit(surface_t* dst, point_t* p, surface_t* src, rect_t* r) {
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
      if (c != chroma_key)
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

typedef struct {
  unsigned short type; /* Magic identifier */
  unsigned int size; /* File size in bytes */
  unsigned int reserved;
  unsigned int offset; /* Offset to image data, bytes */
} __attribute__((packed, aligned(2))) BMPHEADER;

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

//typedef struct {
//  BMPCOREHEADER header;
//  unsigned char colors[3];
//} BMPCOREIFNOHEADER;

#define BMP_GET(d, b, s) \
memcpy(d, b + off, s); \
off += s;

#define BMP_SET(c) (ret->buf[(i - (i % info.width)) + (info.width - (i % info.width) - 1)] = (c));

surface_t* bmp_mem(unsigned char* data) {
  int off = 0;
  BMPHEADER header;
  BMPINFOHEADER info;
//  BMPCOREHEADER core;
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
//      break;
    case 2: // RLE4
//      break;
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

void letter(surface_t* s, unsigned char c, unsigned int x, unsigned int y, int col) {
  int u, v;
  for (u = 0; u < 8; ++u)
    for (v = 0; v < 8; ++v)
      if (font[c][u][v])
        XYSET(s, x + u, y + v, col);
}

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
  
  LARGE_INTEGER ticks_now, freq, elapsed;
  QueryPerformanceCounter(&ticks_now);
  QueryPerformanceFrequency(&freq);
  
  elapsed.QuadPart = ticks_now.QuadPart - ticks_start.QuadPart;
  elapsed.QuadPart *= 1000;
  elapsed.QuadPart /= freq.QuadPart;
  
  return elapsed.QuadPart;
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

static short int keycodes[256];
static short int scancodes[KEY_LAST + 1];

void init_sys_keymap() {
  memset(keycodes,  -1, sizeof(keycodes));
  memset(scancodes, -1, sizeof(scancodes));
  
  keycodes[0x1D] = KEY_0;
  keycodes[0x12] = KEY_1;
  keycodes[0x13] = KEY_2;
  keycodes[0x14] = KEY_3;
  keycodes[0x15] = KEY_4;
  keycodes[0x17] = KEY_5;
  keycodes[0x16] = KEY_6;
  keycodes[0x1A] = KEY_7;
  keycodes[0x1C] = KEY_8;
  keycodes[0x19] = KEY_9;
  keycodes[0x00] = KEY_A;
  keycodes[0x0B] = KEY_B;
  keycodes[0x08] = KEY_C;
  keycodes[0x02] = KEY_D;
  keycodes[0x0E] = KEY_E;
  keycodes[0x03] = KEY_F;
  keycodes[0x05] = KEY_G;
  keycodes[0x04] = KEY_H;
  keycodes[0x22] = KEY_I;
  keycodes[0x26] = KEY_J;
  keycodes[0x28] = KEY_K;
  keycodes[0x25] = KEY_L;
  keycodes[0x2E] = KEY_M;
  keycodes[0x2D] = KEY_N;
  keycodes[0x1F] = KEY_O;
  keycodes[0x23] = KEY_P;
  keycodes[0x0C] = KEY_Q;
  keycodes[0x0F] = KEY_R;
  keycodes[0x01] = KEY_S;
  keycodes[0x11] = KEY_T;
  keycodes[0x20] = KEY_U;
  keycodes[0x09] = KEY_V;
  keycodes[0x0D] = KEY_W;
  keycodes[0x07] = KEY_X;
  keycodes[0x10] = KEY_Y;
  keycodes[0x06] = KEY_Z;

  keycodes[0x27] = KEY_APOSTROPHE;
  keycodes[0x2A] = KEY_BACKSLASH;
  keycodes[0x2B] = KEY_COMMA;
  keycodes[0x18] = KEY_EQUAL;
  keycodes[0x32] = KEY_GRAVE_ACCENT;
  keycodes[0x21] = KEY_LEFT_BRACKET;
  keycodes[0x1B] = KEY_MINUS;
  keycodes[0x2F] = KEY_PERIOD;
  keycodes[0x1E] = KEY_RIGHT_BRACKET;
  keycodes[0x29] = KEY_SEMICOLON;
  keycodes[0x2C] = KEY_SLASH;
  keycodes[0x0A] = KEY_WORLD_1;
  
  keycodes[0x33] = KEY_BACKSPACE;
  keycodes[0x39] = KEY_CAPS_LOCK;
  keycodes[0x75] = KEY_DELETE;
  keycodes[0x7D] = KEY_DOWN;
  keycodes[0x77] = KEY_END;
  keycodes[0x24] = KEY_ENTER;
  keycodes[0x35] = KEY_ESCAPE;
  keycodes[0x7A] = KEY_F1;
  keycodes[0x78] = KEY_F2;
  keycodes[0x63] = KEY_F3;
  keycodes[0x76] = KEY_F4;
  keycodes[0x60] = KEY_F5;
  keycodes[0x61] = KEY_F6;
  keycodes[0x62] = KEY_F7;
  keycodes[0x64] = KEY_F8;
  keycodes[0x65] = KEY_F9;
  keycodes[0x6D] = KEY_F10;
  keycodes[0x67] = KEY_F11;
  keycodes[0x6F] = KEY_F12;
  keycodes[0x69] = KEY_F13;
  keycodes[0x6B] = KEY_F14;
  keycodes[0x71] = KEY_F15;
  keycodes[0x6A] = KEY_F16;
  keycodes[0x40] = KEY_F17;
  keycodes[0x4F] = KEY_F18;
  keycodes[0x50] = KEY_F19;
  keycodes[0x5A] = KEY_F20;
  keycodes[0x73] = KEY_HOME;
  keycodes[0x72] = KEY_INSERT;
  keycodes[0x7B] = KEY_LEFT;
  keycodes[0x3A] = KEY_LEFT_ALT;
  keycodes[0x3B] = KEY_LEFT_CONTROL;
  keycodes[0x38] = KEY_LEFT_SHIFT;
  keycodes[0x37] = KEY_LEFT_SUPER;
  keycodes[0x6E] = KEY_MENU;
  keycodes[0x47] = KEY_NUM_LOCK;
  keycodes[0x79] = KEY_PAGE_DOWN;
  keycodes[0x74] = KEY_PAGE_UP;
  keycodes[0x7C] = KEY_RIGHT;
  keycodes[0x3D] = KEY_RIGHT_ALT;
  keycodes[0x3E] = KEY_RIGHT_CONTROL;
  keycodes[0x3C] = KEY_RIGHT_SHIFT;
  keycodes[0x36] = KEY_RIGHT_SUPER;
  keycodes[0x31] = KEY_SPACE;
  keycodes[0x30] = KEY_TAB;
  keycodes[0x7E] = KEY_UP;
  
  keycodes[0x52] = KEY_KP_0;
  keycodes[0x53] = KEY_KP_1;
  keycodes[0x54] = KEY_KP_2;
  keycodes[0x55] = KEY_KP_3;
  keycodes[0x56] = KEY_KP_4;
  keycodes[0x57] = KEY_KP_5;
  keycodes[0x58] = KEY_KP_6;
  keycodes[0x59] = KEY_KP_7;
  keycodes[0x5B] = KEY_KP_8;
  keycodes[0x5C] = KEY_KP_9;
  keycodes[0x45] = KEY_KP_ADD;
  keycodes[0x41] = KEY_KP_DECIMAL;
  keycodes[0x4B] = KEY_KP_DIVIDE;
  keycodes[0x4C] = KEY_KP_ENTER;
  keycodes[0x51] = KEY_KP_EQUAL;
  keycodes[0x43] = KEY_KP_MULTIPLY;
  keycodes[0x4E] = KEY_KP_SUBTRACT;
  
  for (int sc = 0;  sc < 256; ++sc) {
    if (keycodes[sc] >= 0)
      scancodes[keycodes[sc]] = sc;
  }
}

static int translate_mod(NSUInteger flags) {
  int mods = 0;
  
  if (flags & NSEventModifierFlagShift)
    mods |= MOD_SHIFT;
  if (flags & NSEventModifierFlagControl)
    mods |= MOD_CONTROL;
  if (flags & NSEventModifierFlagOption)
    mods |= MOD_ALT;
  if (flags & NSEventModifierFlagCommand)
    mods |= MOD_SUPER;
  if (flags & NSEventModifierFlagCapsLock)
    mods |= MOD_CAPS_LOCK;
  
  return mods;
}

static int translate_key(unsigned int key) {
  return (key >= sizeof(keycodes) / sizeof(keycodes[0]) ?  KEY_UNKNOWN : keycodes[key]);
}

@interface osx_app_t : NSWindow {
  NSView* view;
  @public bool closed;
}
@end

@interface osx_view_t : NSView {
  NSTrackingArea* track;
}
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

static surface_t* buffer;
static osx_app_t* app;

@implementation osx_view_t
extern surface_t* buffer;

-(id)initWithFrame:(CGRect)r {
  self = [super initWithFrame:r];
  if (self != nil) {
    track = nil;
    [self updateTrackingAreas];
  }
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

//-(BOOL)performKeyEquivalent:(NSEvent*)event {
//  return YES;
//}

-(void)mouseDown:(NSEvent*)event {
  if (__mouse_down_cb)
    __mouse_down_cb(MOUSE_LEFT, translate_mod([event modifierFlags]));
}

-(void)mouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}

-(void)mouseUp:(NSEvent*)event {
  if (__mouse_up_cb)
    __mouse_up_cb(MOUSE_RIGHT, translate_mod([event modifierFlags]));
}

-(void)mouseEntered:(NSEvent*)event {
  if (__mouse_enter_cb)
    __mouse_enter_cb(true);
}

-(void)mouseExited:(NSEvent*)event {
  if (__mouse_enter_cb)
    __mouse_enter_cb(false);
}

-(void)mouseMoved:(NSEvent*)event {
  mx = (int)floor([event locationInWindow].x - 1);
  my = (int)floor(buffer->h - 1 - [event locationInWindow].y);
  if (__mouse_move_cb)
    __mouse_move_cb(mx, my);
}

-(void)rightMouseDown:(NSEvent*)event {
  if (__mouse_down_cb)
    __mouse_down_cb(MOUSE_RIGHT, translate_mod([event modifierFlags]));
}

-(void)rightMouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}

-(void)rightMouseUp:(NSEvent*)event {
  if (__mouse_up_cb)
    __mouse_up_cb(MOUSE_RIGHT, translate_mod([event modifierFlags]));
}

-(void)otherMouseDown:(NSEvent*)event {
  if (__mouse_down_cb)
    __mouse_down_cb((int)[event buttonNumber], translate_mod([event modifierFlags]));
}

-(void)otherMouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}

-(void)otherMouseUp:(NSEvent*)event {
  if (__mouse_up_cb)
    __mouse_up_cb((int)[event buttonNumber], translate_mod([event modifierFlags]));
}

-(void)keyDown:(NSEvent *)event {
  if (__key_down_cb)
    __key_down_cb(translate_key([event keyCode]), translate_mod([event modifierFlags]));
}

-(void)keyUp:(NSEvent *)event {
  if (__key_up_cb)
    __key_up_cb(translate_key([event keyCode]), translate_mod([event modifierFlags]));
}

-(NSRect)resizeRect {
  NSRect v = [[self window] contentRectForFrameRect:[[self window] frame]];
  return NSMakeRect(NSMaxX(v) + 5.5, NSMinY(v) - 16.0 - 5.5, 16.0, 16.0);
}

-(void)drawRect:(NSRect)r {
  (void)r;
  
  if (!buffer)
    return;
  
  CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
  
  CGColorSpaceRef s = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef p = CGDataProviderCreateWithData(NULL, buffer->buf, buffer->w * buffer->h * 4, NULL);
  CGImageRef img = CGImageCreate(buffer->w, buffer->h, 8, 32, buffer->w * 4, s, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, p, NULL, false, kCGRenderingIntentDefault);
  
  CGColorSpaceRelease(s);
  CGDataProviderRelease(p);
  
  CGContextDrawImage(ctx, CGRectMake(0, 0, buffer->w, buffer->h), img);
  
  CGImageRelease(img);
}

-(void)dealloc {
  [track release];
  [super dealloc];
}
@end

@implementation osx_app_t
-(id)initWithContentRect:(NSRect)r
               styleMask:(NSWindowStyleMask)s
                 backing:(NSBackingStoreType)t
                   defer:(BOOL)d {
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

+(NSRect)frameRectForContentRect:(NSRect)r
                       styleMask:(NSWindowStyleMask)s {
  (void)s;
  return NSInsetRect(r, 0, 0);
}
@end

surface_t* screen(const char* t, int w, int h) {
  if (!(buffer = surface(w, h)))
    return NULL;
  buffer->w = w;
  buffer->h = h;
  
  init_default_font();
  init_sys_keymap();
  
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  
  app = [[osx_app_t alloc] initWithContentRect:NSMakeRect(0, 0, w, h + 22)
                                     styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskTitled
                                       backing:NSBackingStoreBuffered
                                         defer:NO];
  if (!app)
    return NULL;
  
  id app_del = [AppDelegate alloc];
  if (!app_del)
    [NSApp terminate:nil];
  
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

bool redraw() {
  bool ret = true;
  
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApp sendEvent:[NSApp nextEventMatchingMask:NSEventMaskAny
                                      untilDate:[NSDate distantPast]
                                         inMode:NSDefaultRunLoopMode
                                        dequeue:YES]];
  [pool release];
  
  if (app->closed)
    ret = false;
  [[app contentView] setNeedsDisplay:YES];

  return ret;
}

void release() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  if (app)
  	[app close];
  [pool drain];
}
#elif defined(_WIN32)
#error WinAPI not implemented
#else
#error X11 not implemented
#endif
