
//  graphics.c
//  graphics
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#include "graphics.h"

#define XYSET(s, x, y, v) (s->buf[(y) * s->w + (x)] = (v))
#define XYGET(s, x, y) (s->buf[(y) * s->w + (x)])

static char last_error[1024];

const char* get_last_error() {
  return last_error;
}

#define SET_LAST_ERROR(MSG, ...) \
memset(last_error, 0, 1024); \
sprintf(last_error, "[ERROR] from %s in %s at %d -- " MSG, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

static const char* font_b64_str = "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";
static const int font_b64_strlen = 2928;
static bool font[256][8][8];

#define LINE_HEIGHT 10

const static unsigned char b64_table[] = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //10
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //20
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //30
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //40
  0,   0,   0,   62,  0,   0,   0,   63,  52,  53, //50
  54,  55,  56,  57,  58,  59,  60,  61,  0,   0,  //60
  0,   0,   0,   0,   0,   0,   1,   2,   3,   4,  //70
  5,   6,   7,   8,   9,   10,  11,  12,  13,  14, //80
  15,  16,  17,  18,  19,  20,  21,  22,  23,  24, //90
  25,  0,   0,   0,   0,   0,   0,   26,  27,  28, //100
  29,  30,  31,  32,  33,  34,  35,  36,  37,  38, //110
  39,  40,  41,  42,  43,  44,  45,  46,  47,  48, //120
  49,  50,  51,  0,   0,   0,   0,   0,   0,   0,  //130
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //140
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //150
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //160
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //170
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //180
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //190
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //200
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //210
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //220
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //230
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //240
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  //250
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

void mouse_move_cb(void (*fn)(int, int)) {
  SWAP_POINTERS(__mouse_move_cb, fn);
}

void mouse_entered_cb(void (*fn)(bool)) {
  SWAP_POINTERS(__mouse_enter_cb, fn);
}

void mouse_pos(int* x, int* y) {
  if (!x || !y)
    return;
  *x = mx;
  *y = my;
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
  surface_t* img = load_bmp_from_mem(f);
  
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
  int offset_x = 0,      offset_y = 0,
      from_x   = 0,      from_y   = 0,
      width    = src->w, height   = src->h;
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
  int to_x = offset_x + width, to_y = offset_y + height;
  if (to_x > dst->w || to_y > dst->h) {
    SET_LAST_ERROR("blit() failed! src w/h outside bounds of dst");
    return false;
  }
  
  int x, y;
  for (x = 0; x < width; ++x)
    for (y = 0; y < height; ++y)
      XYSET(dst, offset_x + x, offset_y + y, XYGET(src, from_x + x, from_y + y));
  return true;
}

bool yline(surface_t* s, int x, int y1, int y2, int col) {
  if (y2 < y1) {
    y1 += y2;
    y2  = y1 - y2;
    y1 -= y2;
  }
  if (y2 < 0 || y1 >= s->h  || x < 0 || x >= s->w) {
    SET_LAST_ERROR("yline() failed! x/y outside bounds of dst");
    return false;
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
  if (x2 < x1) {
    x1 += x2;
    x2  = x1 - x2;
    x1 -= x2;
  }
  
  if (x2 < 0 || x1 >= s->w || y < 0 || y >= s->h) {
    SET_LAST_ERROR("xline() failed! x/y outside bounds of dst");
    return false;
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
  if (x1 < 0 || x1 > s->w - 1 || x2 < 0 || x2 > s->w - 1 || y1 < 0 || y1 > s->h - 1 || y2 < 0 || y2 > s->h - 1) {
    SET_LAST_ERROR("line() failed! x1/y1/x2/y2 outside bounds of dst");
    return false;
  }
  
  int dx = abs(x2 - x1), dy = abs(y2 - y1);
  int x = x1, y = y1;
  int xi1, xi2, yi1, yi2, d, n, na, np, p;
  
  if (x2 >= x1) {
    xi1 = 1;
    xi2 = 1;
  } else {
    xi1 = -1;
    xi2 = -1;
  }
  
  if(y2 >= y1) {
    yi1 = 1;
    yi2 = 1;
  } else  {
    yi1 = -1;
    yi2 = -1;
  }
  
  if (dx >= dy) {
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
  if (xc - r < 0 || xc + r >= s->w || yc - r < 0 || yc + r >= s->h) {
    SET_LAST_ERROR("circle() failed! x/y outside bounds of dst");
    return false;
  }
  
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
      XYSET(s, a, b, col);
      XYSET(s, c, d, col);
      XYSET(s, e, f, col);
      XYSET(s, g, f, col);
      
      if (x > 0) {
        XYSET(s, a, d, col);
        XYSET(s, c, b, col);
        XYSET(s, e, h, col);
        XYSET(s, g, h, col);
      }
    }
    
    pb = b;
    pd = d;
    p += (p < 0 ? (x++ << 2) + 6 : ((x++ - y--) << 2) + 10);
  }
  return true;
}

bool rect(surface_t* s, int x, int y, int w, int h, int col, bool fill) {
  w = x + w;
  h = y + h;
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

unsigned char* load_file_to_mem(const char* path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    SET_LAST_ERROR("fopen() failed: %s\n", path);
    return NULL;
  }
  
  fseek(file, 0, SEEK_END);
  size_t length = ftell(file);
  rewind(file);
  
  unsigned char* data = (unsigned char*)calloc(length + 1, sizeof(unsigned char));
  fread(data, 1, length, file);
  fclose(file);
  
  return data;
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

surface_t* load_bmp_from_mem(unsigned char* data) {
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

surface_t* load_bmp_from_file(const char* path) {
  unsigned char* data = load_file_to_mem(path);
  surface_t* ret = load_bmp_from_mem(data);
  free(data);
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

surface_t* string(int col, const char* str) {
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
  print(ret, 0, 0, col, str);
  return ret;
}

surface_t* string_f(int col, const char* fmt, ...) {
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
  
  surface_t* ret = string(col, buffer);
  free(buffer);
  return ret;
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

@interface osx_app_t : NSWindow {
  NSView* view;
  @public bool closed;
}
@end

@interface osx_view_t : NSView {
  NSTrackingArea* track;
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

-(void)mouseDown:(NSEvent*)event {
  
}

-(void)mouseDragged:(NSEvent*)event {
  [self mouseMoved:event];
}

-(void)mouseUp:(NSEvent*)event {
  
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
  }
  
  if (view)
    [view removeFromSuperview];
  
  view = v;
  [view setFrame:[self contentRectForFrameRect:b]];
  [view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [fv addSubview:view];
}

-(void)win_changed:(NSNotification *)n { (void)n; }
-(void)win_close { closed = true; }
-(NSView*)contentView { return view; }
-(BOOL)canBecomeKeyWindow { return YES; }
-(BOOL)canBecomeMainWindow { return YES; }

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
  
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  
  app = [[osx_app_t alloc] initWithContentRect:NSMakeRect(0, 0, w, h + 22)
                                     styleMask:NSWindowStyleMaskClosable | NSWindowStyleMaskTitled
                                       backing:NSBackingStoreBuffered
                                         defer:NO];
  if (!app)
    return NULL;
  
  [app setTitle:[NSString stringWithUTF8String:t]];
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
  NSEvent* e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                  untilDate:[NSDate distantPast]
                                     inMode:NSDefaultRunLoopMode
                                    dequeue:YES];
  if (e) {
    switch ([e type]) {
      case NSEventTypeKeyDown:
      case  NSEventTypeKeyUp:
        ret = false;
        break;
      default :
        [NSApp sendEvent:e];
    }
  }
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdlib.h>

static WNDCLASS wc;
static HWND wnd;
static int close = 0;
static int width;
static int height;
static HDC hdc;
static void* buffer;
static BITMAPINFO* bitmapInfo;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  LRESULT res = 0;
  switch (message) {
    case WM_PAINT:
      if (buffer) {
        StretchDIBits(hdc, 0, 0, width, height, 0, 0, width, height, buffer, bitmapInfo, DIB_RGB_COLORS, SRCCOPY);
        ValidateRect(hWnd, NULL);
      }
      break;
    case WM_KEYDOWN:
      if ((wParam&0xFF) == 27)
        close = 1;
      break;
    case WM_CLOSE:
      close = 1;
      break;
    default:
      res = DefWindowProc(hWnd, message, wParam, lParam);
  }
  return res;
}

int screen(const char* title, int width, int height) {
  wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.lpszClassName = title;
  RegisterClass(&wc);
  
  RECT rect    = { 0 };
  rect.right   = width;
  rect.bottom  = height;
  AdjustWindowRect(&rect, WS_POPUP | WS_SYSMENU | WS_CAPTION, 0);
  rect.right  -= rect.left;
  rect.bottom -= rect.top;
  
  width  = width;
  height = height;
  
  wnd = CreateWindowEx(0, title, title,
                       WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
                       CW_USEDEFAULT, CW_USEDEFAULT,
                       rect.right, rect.bottom,
                       0, 0, 0, 0);
  
  if (!wnd)
    return 0;
  
  ShowWindow(wnd, SW_NORMAL);
  
  bitmapInfo = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3);
  bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bitmapInfo->bmiHeader.biPlanes = 1;
  bitmapInfo->bmiHeader.biBitCount = 32;
  bitmapInfo->bmiHeader.biCompression = BI_BITFIELDS;
  bitmapInfo->bmiHeader.biWidth = width;
  bitmapInfo->bmiHeader.biHeight = -height;
  bitmapInfo->bmiColors[0].rgbRed = 0xff;
  bitmapInfo->bmiColors[1].rgbGreen = 0xff;
  bitmapInfo->bmiColors[2].rgbBlue = 0xff;
  
  hdc = GetDC(wnd);
  
  return 1;
}

int redraw(void* buffer) {
  MSG msg;
  buffer = buffer;
  
  InvalidateRect(wnd, NULL, TRUE);
  SendMessage(wnd, WM_PAINT, 0, 0);
  while (PeekMessage(&msg, wnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  
  if (close == 1)
    return 0;
  
  return 1;
}

void release() {
  buffer = 0;
  free(bitmapInfo);
  ReleaseDC(wnd, hdc);
  DestroyWindow(wnd);
}
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static Display* display;
static int screen;
static int width;
static int height;
static Window window;
static GC gc;
static XImage *ximage;

int screen(const char* title, int width, int height) {
  int depth, i, formatCount, convDepth = -1;
  XPixmapFormatValues* formats;
  XSetWindowAttributes windowAttributes;
  XSizeHints sizeHints;
  Visual* visual;
  
  display = XOpenDisplay(0);
  
  if (!display)
    return -1;
  
  screen  = DefaultScreen(display);
  visual  = DefaultVisual(display, screen);
  formats = XListPixmapFormats(display, &formatCount);
  depth   = DefaultDepth(display, screen);
  Window defaultRootWindow = DefaultRootWindow(display);
  
  for (i = 0; i < formatCount; ++i) {
    if (depth == formats[i].depth) {
      convDepth = formats[i].bitper_pixel;
      break;
    }
  }
  XFree(formats);
  
  if (convDepth != 32) {
    XCloseDisplay(display);
    return -1;
  }
  
  int screenWidth  = DisplayWidth(display, screen);
  int screenHeight = DisplayHeight(display, screen);
  
  windowAttributes.border_pixel     = BlackPixel(display, screen);
  windowAttributes.background_pixel = BlackPixel(display, screen);
  windowAttributes.backing_store    = NotUseful;
  
  window = XCreateWindow(display, defaultRootWindow, (screenWidth - width) / 2,
                         (screenHeight - height) / 2, width, height, 0, depth, InputOutput,
                         visual, CWBackPixel | CWBorderPixel | CWBackingStore,
                         &windowAttributes);
  if (!window)
    return 0;
  
  XSelectInput(display, window, KeyPressMask | KeyReleaseMask);
  XStoreName(display, window, title);
  
  sizeHints.flags = PPosition | PMinSize | PMaxSize;
  sizeHints.x = 0;
  sizeHints.y = 0;
  sizeHints.min_width = width;
  sizeHints.max_width = width;
  sizeHints.min_height = height;
  sizeHints.max_height = height;
  
  XSetWMNormalHints(display, window, &sizeHints);
  XClearWindow(display, window);
  XMapRaised(display, window);
  XFlush(display);
  
  gc = DefaultGC(display, screen);
  ximage = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, NULL, width, height, 32, width * 4);
  
  width  = width;
  height = height;
  
  return 1;
}

int redraw(void* buffer) {
  ximage->data = (char*)buffer;
  
  XPutImage(display, window, gc, ximage, 0, 0, 0, 0, width, height);
  XFlush(display);
  
  if (!XPending(display))
    return 0;
  
  XEvent event;
  XNextEvent(display, &event);
  KeySym sym = XLookupKeysym(&event.xkey, 0);
  
  return 1;
}

void release(void) {
  ximage->data = NULL;
  XDestroyImage(ximage);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
}
#endif
