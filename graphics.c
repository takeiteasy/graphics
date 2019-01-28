//  graphics.c
//  graphics
//
//  Created by Rory B. Bellows on 26/11/2017.
//  Copyright © 2017 Rory B. Bellows. All rights reserved.
//

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "graphics.h"

#if defined(_MSC_VER)
#define strdup _strdup
#endif

#if !defined(MIN)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#if !defined(MAX)
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#if !defined(CLAMP)
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#endif

#if !defined(M_PI)
#define M_PI 3.14159265358979323846264338327950288f
#endif
#define DEG2RAD(a) ((a) * M_PI / 180.0)
#define RAD2DEG(a) ((a) * 180.0 / M_PI)

#define FREE_SAFE(x) \
if ((x)) {  \
  free((x)); \
  (x) = NULL; \
}

static void* userdata = NULL;

void sgl_set_userdata(void* data) {
  userdata = data;
}

void* sgl_get_userdata() {
  return userdata;
}

#define CALL(x, ...) \
if ((x)) \
  (x)(userdata, __VA_ARGS__);

static void(*__error_callback)(void*, ERRORLVL, ERRORTYPE, const char*, const char*, const char*, int) = NULL;

void sgl_error_callback(void (*cb)(void*, ERRORLVL, ERRORTYPE, const char*, const char*, const char*, int)) {
  __error_callback = cb;
}

static inline const char* errprio_str(ERRORLVL pri) {
  switch (pri) {
    case HIGH_PRIORITY:
      return "SERIOUS";
    case NORMAL_PRIORITY:
      return "ERROR";
    case LOW_PRIORITY:
      return "WARNING";
    default:
      return "LOG";
  }
}

void error_handle(ERRORLVL pri, ERRORTYPE type, const char* msg, ...) {
  va_list args;
  va_start(args, msg);

  static char error[1024];
  vsprintf(error, msg, args);

  if (__error_callback)
    __error_callback(userdata, pri, type, error, __FILE__, __FUNCTION__, __LINE__);
  else
    fprintf(stderr, "[%s:%d] from %s in %s() at %d -- %s\n", errprio_str(pri), (int)type, __FILE__, __FUNCTION__, __LINE__, error);

  va_end(args);
}

#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") : "

#define NOTE(x)  message(x)
#define FILE_LINE  message(__FILE__LINE__)

#define TODO(x)  message(__FILE__LINE__"\n" \
" ------------------------------------------------\n" \
"|  TODO " __FUNCTION__ "() -> " #x "\n" \
" ------------------------------------------------")
#define FIXME(x) message(__FILE__LINE__"\n" \
" ------------------------------------------------\n" \
"|  FIXME " __FUNCTION__ "() -> " #x "\n" \
" ------------------------------------------------")
#define todo(x) message(__FILE__LINE__" TODO " __FUNCTION__ "() -> " #x)
#define fixme(x) message(__FILE__LINE__" FIXME " __FUNCTION__ "() -> " #x)

#define stb_sb_free(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define stb_sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define stb_sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      (*((void **)&(a)) = stb__sbgrowf((a), (n), sizeof(*(a))))

static void* stb__sbgrowf(void *arr, int increment, int itemsize) {
  int dbl_cur = arr ? 2*stb__sbm(arr) : 0;
  int min_needed = stb_sb_count(arr) + increment;
  int m = dbl_cur > min_needed ? dbl_cur : min_needed;
  int *p = (int *) realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int)*2);
  if (p) {
    if (!arr)
      p[1] = 0;
    p[0] = m;
    return p+2;
  } else {
#ifdef STRETCHY_BUFFER_OUT_OF_MEMORY
    STRETCHY_BUFFER_OUT_OF_MEMORY ;
#endif
    return (void *) (2*sizeof(int)); // try to force a NULL pointer exception later
  }
}

bool sgl_surface(surface_t* s, unsigned int w, unsigned int h) {
  s->w = w;
  s->h = h;
  size_t sz = w * h * sizeof(unsigned int) + 1;
  s->buf = malloc(sz);
  if (!s->buf) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  memset(s->buf, 0, sz);

  return true;
}

void sgl_destroy(surface_t* s) {
  if (!s)
    return;
  FREE_SAFE(s->buf);
  s->w = 0;
  s->h = 0;
}

void sgl_fill(surface_t* s, int col) {
  for (int i = 0; i < s->w * s->h; ++i)
    s->buf[i] = col;
}

#define XYGET(s, x, y) (s->buf[(y) * s->w + (x)])

static inline void flood_fn(surface_t* s, int x, int y, int new, int old) {
  if (new == old || XYGET(s, x, y) != old)
    return;

  int x1 = x;
  while (x1 < s->w && XYGET(s, x1, y) == old) {
    XYGET(s, x1, y) = new;
    x1++;
  }

  x1 = x - 1;
  while (x1 >= 0 && XYGET(s, x1, y) == old) {
    XYGET(s, x1, y) = new;
    x1--;
  }

  x1 = x;
  while (x1 < s->w && XYGET(s, x1, y) == new) {
    if(y > 0 && XYGET(s, x1, y - 1) == old)
      flood_fn(s, x1, y - 1, new, old);
    x1++;
  }

  x1 = x - 1;
  while(x1 >= 0 && XYGET(s, x1, y) == new) {
    if(y > 0 && XYGET(s, x1, y - 1) == old)
      flood_fn(s, x1, y - 1, new, old);
    x1--;
  }

  x1 = x;
  while(x1 < s->w && XYGET(s, x1, y) == new) {
    if(y < s->h - 1 && XYGET(s, x1, y + 1) == old)
      flood_fn(s, x1, y + 1, new, old);
    x1++;
  }

  x1 = x - 1;
  while(x1 >= 0 && XYGET(s, x1, y) == new) {
    if(y < s->h - 1 && XYGET(s, x1, y + 1) == old)
      flood_fn(s, x1, y + 1, new, old);
    x1--;
  }
}

void sgl_flood(surface_t* s, int x, int y, int col) {
  if (!s || x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;
  flood_fn(s, x, y, col, XYGET(s, x, y));
}

void sgl_cls(surface_t* s) {
  memset(s->buf, 0, s->w * s->h * sizeof(int));
}

void sgl_pset(surface_t* s, int x, int y, int c)  {
  if (x >= 0 && y >= 0 && x < s->w && y < s->h)
    s->buf[y * s->w + x] = c;
}

#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

#if !defined(SGL_DISABLE_RGBA)
void sgl_psetb(surface_t* s, int x, int y, int c) {
  int a = A(c);
  if (!a || x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;

  int* p = &s->buf[y * s->w + x];
  int  b = A(*p);
  *p = (a == 255 || !b) ? c : RGBA(BLEND(R(c), R(*p), a, b),
                                   BLEND(G(c), G(*p), a, b),
                                   BLEND(B(c), B(*p), a, b),
                                   a + (b * (255 - a) >> 8));
}

static void(*pset_fn)(surface_t*, int, int, int) = sgl_psetb;
#else
void sgl_psetb(surface_t* s, int x, int y, int c) {
  int a = A(c);
  if (!a || x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;

  int b = XYGET(s, x, y);
  float i = (float)a / 255.f;
  sgl_pset(s, x, y, (i >= 1.f || i <= 0.f) ? c : RGB((int)roundf(R(c) * (1 - i) + R(b) * i),
                                                     (int)roundf(G(c) * (1 - i) + G(b) * i),
                                                     (int)roundf(B(c) * (1 - i) + B(b) * i)));
}

static void(*pset_fn)(surface_t*, int, int, int) = sgl_pset;
#endif

int sgl_pget(surface_t* s, int x, int y) {
  return (x < 0 || y < 0 || x >= s->w || y >= s->h ? 0 : XYGET(s, x, y));
}

bool sgl_blit(surface_t* dst, point_t* p, surface_t* src, rect_t* r) {
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
  for (x = 0; x < width; ++x)
    for (y = 0; y < height; ++y) {
#if !defined(SGL_DISABLE_CHROMA_KEY) && defined(BLIT_CHROMA_KEY)
      c = XYGET(src, from_x + x, from_y + y);
      if (c == BLIT_CHROMA_KEY)
        continue;
#endif
      pset_fn(dst, offset_x + x, offset_y + y, c);
    }
  return true;
}

bool sgl_reset(surface_t* s, int nw, int nh) {
  size_t sz = nw * nh * sizeof(unsigned int) + 1;
  int* tmp = realloc(s->buf, sz);
  if (!tmp) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "realloc() failed");
    return false;
  }
  s->buf = tmp;
  s->w = nw;
  s->h = nh;
  memset(s->buf, 0, sz);
  return true;
}

bool sgl_copy(surface_t* in, surface_t* out) {
  if (!sgl_surface(out, in->w, in->h))
    return false;
  memcpy(out->buf, in->buf, in->w * in->h * sizeof(unsigned int) + 1);
  return !!out->buf;
}

void sgl_filter(surface_t* s, int (*fn)(int x, int y, int col)) {
  if (!s || !s->buf)
    return;

  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      sgl_pset(s, x, y, fn(x, y, XYGET(s, x, y)));
}

bool sgl_resize(surface_t* in, int nw, int nh, surface_t* out) {
  if (!sgl_surface(out, nw, nh))
    return false;

  int x_ratio = (int)((in->w << 16) / nw) + 1;
  int y_ratio = (int)((in->h << 16) / nh) + 1;
  int x2, y2, i, j;
  for (i = 0; i < nh; ++i) {
    int* t = out->buf + i * nw;
    y2 = ((i * y_ratio) >> 16);
    int* p = in->buf + y2 * in->w;
    int rat = 0;
    for (j = 0; j < nw; ++j) {
      x2 = (rat >> 16);
      *t++ = p[x2];
      rat += x_ratio;
    }
  }
  return true;
}

bool sgl_rotate(surface_t* in, float angle, surface_t* out) {
  float theta = DEG2RAD(angle);
  float c = cosf(theta), s = sinf(theta);
  float r[3][2] = {
    { -in->h * s, in->h * c },
    {  in->w * c - in->h * s, in->h * c + in->w * s },
    {  in->w * c, in->w * s }
  };

  float mm[2][2] = { {
      MIN(0, MIN(r[0][0], MIN(r[1][0], r[2][0]))),
      MIN(0, MIN(r[0][1], MIN(r[1][1], r[2][1])))
    }, {
      (theta > 1.5708  && theta < 3.14159 ? 0.f : MAX(r[0][0], MAX(r[1][0], r[2][0]))),
      (theta > 3.14159 && theta < 4.71239 ? 0.f : MAX(r[0][1], MAX(r[1][1], r[2][1])))
    }
  };

  int dw = (int)ceil(fabsf(mm[1][0]) - mm[0][0]);
  int dh = (int)ceil(fabsf(mm[1][1]) - mm[0][1]);
  if (!sgl_surface(out, dw, dh))
    return false;

// p'x = cos(theta) * (px - ox) - sin(theta) * (py-oy) + ox
// p'y = sin(theta) * (px - ox) + cos(theta) * (py-oy) + oy
// psetb(dst, x + offset_x - dw / 2  + src->w / 2, y + offset_y - dh / 2 + src->h / 2, pget(src, sx, sy));

  int x, y, sx, sy;
  for (x = 0; x < dw; ++x)
    for (y = 0; y < dh; ++y) {
      sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
      sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
      if (sx < 0 || sx >= in->w || sy < 0 || sy >= in->h)
        continue;
      pset_fn(out, x, y, XYGET(in, sx, sy));
    }
  return true;
}

void sgl_vline(surface_t* s, int x, int y0, int y1, int col) {
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
    pset_fn(s, x, y, col);
}

void sgl_hline(surface_t* s, int y, int x0, int x1, int col) {
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
    pset_fn(s, x, y, col);
}

void sgl_line(surface_t* s, int x0, int y0, int x1, int y1, int col) {
#if !defined(SGL_DISABLE_RGBA)
  int a = A(col);
  if (!a)
    return;
#else
  int a = 255;
#endif

  if (x0 == x1)
    sgl_vline(s, x0, y0, y1, col);
  if (y0 == y1)
    sgl_hline(s, y0, x0, x1, col);

  int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
#if defined(SGL_ENABLE_AA)
  int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx - dy, e2, x2;
  int ed = dx + dy == 0 ? 1 : sqrtf((float)dx * dx + (float)dy * dy);
#else
  int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int err = dx + dy, e2;
#endif

  for (;;) {
#if defined(SGL_ENABLE_AA)
    sgl_psetb(s, x0, y0, ACHAN(col, a - (a * abs(err - dx + dy) / ed)));
    e2 = err;
    x2 = x0;

    if (2 * e2 >= -dx) {
      if (x0 == x1)
        break;
      if (e2 + dy < ed)
        sgl_psetb(s, x0, y0 + sy, ACHAN(col, a - (a * (e2 + dy) / ed)));
      err -= dy;
      x0 += sx;
    }

    if (2 * e2 <= dy) {
      if (y0 == y1)
        break;
      if (dx - e2 < ed)
        sgl_psetb(s, x2 + sx, y0, ACHAN(col, a - (a * (dx - e2) / ed)));
      err += dx;
      y0 += sy;
    }
#else
    pset_fn(s, x0, y0, col);
    e2 = 2 * err;

    if (e2 >= dy) {
      if (x0 == x1)
        break;
      err += dy;
      x0 += sx;
    }

    if (e2 <= dx) {
      if (y0 == y1)
        break;
      err += dx;
      y0 += sy;
    }
#endif
  }
}

void sgl_circle(surface_t* s, int xc, int yc, int r, int col, int fill) {
#if !defined(SGL_DISABLE_RGBA)
  int a = A(col);
  if (!a)
    return;
#else
  int a = 255;
#endif

  if (xc + r < 0 || yc + r < 0 || xc - r > s->w || yc - r > s->h)
    return;

  int x = -r, y = 0, err = 2 - 2 * r;
#if defined(SGL_ENABLE_AA)
  int i, x2, e2;
  r = 1 - err;
#endif

  do {
#if defined(SGL_ENABLE_AA)
    i = ACHAN(col, 255 - (255 * abs(err - 2 * (x + y) - 2) / r));
    sgl_psetb(s, xc - x, yc + y, i);
    sgl_psetb(s, xc - y, yc - x, i);
    sgl_psetb(s, xc + x, yc - y, i);
    sgl_psetb(s, xc + y, yc + x, i);

    if (fill) {
      sgl_hline(s, yc - y, xc - x - 1, xc + x + 1, col);
      sgl_hline(s, yc + y, xc - x - 1, xc + x + 1, col);
    }

    e2 = err;
    x2 = x;
    if (err + y > 0) {
      i = 255 * (err - 2 * x - 1) / r;
      if (i < 256) {
        i = ACHAN(col, 255 - i);
        sgl_psetb(s, xc - x, yc + y + 1, i);
        sgl_psetb(s, xc - y - 1, yc - x, i);
        sgl_psetb(s, xc + x, yc - y - 1, i);
        sgl_psetb(s, xc + y + 1, yc + x, i);
      }
      err += ++x * 2 + 1;
    }

    if (e2 + x2 <= 0) {
      i = 255 * (2 * y + 3 - e2) / r;
      if (i < 256) {
        i = ACHAN(col, 255 - i);
        sgl_psetb(s, xc - x2 - 1, yc + y, i);
        sgl_psetb(s, xc - y, yc - x2 - 1, i);
        sgl_psetb(s, xc + x2 + 1, yc - y, i);
        sgl_psetb(s, xc + y, yc + x2 + 1, i);
      }
      err += ++y * 2 + 1;
    }
#else
    pset_fn(s, xc - x, yc + y, col);
    pset_fn(s, xc - y, yc - x, col);
    pset_fn(s, xc + x, yc - y, col);
    pset_fn(s, xc + y, yc + x, col);

    if (fill) {
      sgl_hline(s, yc - y, xc - x, xc + x, col);
      sgl_hline(s, yc + y, xc - x, xc + x, col);
    }

    r = err;
    if (r <= y)
      err += ++y * 2 + 1;
    if (r > x || err > y)
      err += ++x * 2 + 1;
#endif
  } while (x < 0);
}

void sgl_ellipse(surface_t* s, int xc, int yc, int rx, int ry, int col, int fill) {
#if defined(SGL_ENABLE_AA)
#pragma TODO(Add AA option);
#endif

  int x = -rx, y = 0;
  long e2 = ry, dx = (1 + 2 * x) * e2 * e2;
  long dy = x * x, err = dx + dy;

  do {
    sgl_psetb(s, xc - x, yc + y, col);
    sgl_psetb(s, xc + x, yc + y, col);
    sgl_psetb(s, xc + x, yc - y, col);
    sgl_psetb(s, xc - x, yc - y, col);

    if (fill) {
      sgl_hline(s, yc - y, xc - x, xc + x, col);
      sgl_hline(s, yc + y, xc - x, xc + x, col);
    }

    e2 = 2 * err;
    if (e2 >= dx) {
      x++;
      err += dx += 2 * (long)ry * ry;
    }
    if (e2 <= dy) {
      y++;
      err += dy += 2 * (long)rx * rx;
    }
  } while (x <= 0);
}

void sgl_ellipse_rect(surface_t* s, int x0, int y0, int x1, int y1, int col, int fill) {
#pragma FIXME(This is borked without AA)
#pragma FIXME(Arithmic error when too big with AA)

  long a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
  float dx = 4 * (a - 1.) * b * b, dy = 4 * (b1 + 1) * a * a;
  float err = b1 * a * a - dx + dy;

#if defined(SGL_ENABLE_AA)
  int f, ed, i;

  if (a == 0 || b == 0)
    sgl_line(s, x0, y0, x1, y1, col);
#else
  long e2;
#endif

  if (x0 > x1) {
    x0 = x1;
    x1 += a;
  }
  if (y0 > y1)
    y0 = y1;
  y0 += (b + 1) / 2;
  y1 = y0 - (int)b1;
  a = 8 * a * a;
  b1 = 8 * b * b;

#if defined(SGL_ENABLE_AA)
  for (;;) {
    i = fminf(dx, dy);
    ed = fmaxf(dx, dy);
    if (!i || !ed)
      break;
    if (y0 == y1 + 1 && err > dy && a > b1)
      ed = 255 * 4. / a;
    else
      ed = 255 / (ed + 2 * ed * i * i / (4 * ed * ed + i * i)); // Fix overflow

    i = ACHAN(col, 255 - (ed * (int)fabsf(err + dx - dy)));
    sgl_psetb(s, x0, y0, i);
    sgl_psetb(s, x0, y1, i);
    sgl_psetb(s, x1, y0, i);
    sgl_psetb(s, x1, y1, i);

    if (fill) {
      sgl_hline(s, y0, x0 + 1, x1 - 1, col);
      sgl_hline(s, y1, x0 + 1, x1 - 1, col);
    }

    if ((f = 2 * err + dy >= 0)) {
      if (x0 >= x1)
        break;

      i = ed * (err + dx);
      if (i < 255) {
        i = ACHAN(col, 255 - i);
        sgl_psetb(s, x0, y0 + 1, i);
        sgl_psetb(s, x0, y1 - 1, i);
        sgl_psetb(s, x1, y0 + 1, i);
        sgl_psetb(s, x1, y1 - 1, i);
      }
    }

    if (2 * err <= dx) {
      i = ed * (dy - err);
      if (i < 255) {
        i = ACHAN(col, 255 - i);
        sgl_psetb(s, x0 + 1, y0, i);
        sgl_psetb(s, x1 - 1, y0, i);
        sgl_psetb(s, x0 + 1, y1, i);
        sgl_psetb(s, x1 - 1, y1, i);
      }

      y0++;
      y1--;
      err += dy += a;
    }

    if (f) {
      x0++; x1--;
      err -= dx -= b1;
    }
  }

  if (--x0 == x1++)
    while (y0 - y1 < b) {
      i = ACHAN(col, 255 - (255 * 4 * (int)fabsf(err + dx) / (int)b1));
      sgl_psetb(s, x0, ++y0, i);
      sgl_psetb(s, x1, y0, i);
      sgl_psetb(s, x0, --y1, i);
      sgl_psetb(s, x1, y1, i);
      err += dy += a;
    }
#else
  do {
    pset_fn(s, x1, y0, col);
    pset_fn(s, x0, y0, col);
    pset_fn(s, x0, y1, col);
    pset_fn(s, x1, y1, col);

    if (fill) {
      sgl_hline(s, y0, x0, x1, col);
      sgl_hline(s, y1, x0, x1, col);
    }

    e2 = 2 * err;
    if (e2 <= dy) {
      y0++;
      y1--;
      err += dy += a;
    }

    if (e2 >= dx || 2 * err > dy) {
      x0++;
      x1--;
      err += dx += b1;
    }
  } while (x0 <= x1);
#endif
}

static inline void bezier_seg(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int col) {
  int sx = x2 - x1, sy = y2 - y1;
  long xx = x0 - x1, yy = y0 - y1, xy;
  float dx, dy, err, cur = xx * sy - yy * sx;

  if (sx * (long)sx + sy * (long)sy > xx * xx + yy * yy) {
    x2 = x0;
    x0 = sx + x1;
    y2 = y0;
    y0 = sy + y1;
    cur = -cur;
  }

  if (cur != 0.f) {
    xx += sx;
    xx *= sx = x0 < x2 ? 1 : -1;
    yy += sy;
    yy *= sy = y0 < y2 ? 1 : -1;
    xy = 2 * xx * yy;
    xx *= xx;
    yy *= yy;
    if (cur * sx * sy < 0) {
      xx = -xx;
      yy = -yy;
      xy = -xy;
      cur = -cur;
    }

    dx = 4.f * sy * (x1 - x0) * cur + xx - xy;
    dy = 4.f * sx * (y0 - y1) * cur + yy - xy;
    xx += xx;
    yy += yy;
    err = dx + dy + xy;

#if defined(SGL_ENABLE_AA)
    float ed;
#endif

    do {
#if defined(SGL_ENABLE_AA)
      cur = fminf(dx + xy, -xy - dy);
      ed = fmaxf(dx + xy, -xy - dy);
      ed += 2 * ed * cur * cur / (4 * ed * ed + cur * cur);
      sgl_psetb(s, x0, y0, ACHAN(col, 255 - (int)(255 * fabsf(err - dx - dy - xy) / ed)));
      if (x0 == x2 || y0 == y2)
        break;

      x1 = x0;
      cur = dx - err;
      y1 = 2 * err + dy < 0;
      if (2 * err + dx > 0) {
        if (err - dy < ed)
          sgl_psetb(s, x0, y0 + sy, ACHAN(col, 255 - (int)(255 * fabsf(err - dy) / ed)));
        x0 += sx;
        dx -= xy;
        err += dy += yy;
      }

      if (y1) {
        if (cur < ed)
          sgl_psetb(s, x1 + sx, y0, ACHAN(col, 255 - (int)(255 * fabsf(cur) / ed)));
        y0 += sy;
        dy -= xy;
        err += dx += xx;
      }
    } while (dy < dx);
#else
    pset_fn(s, x0, y0, col);
    if (x0 == x2 && y0 == y2)
      return;

    y1 = 2 * err < dx;
    if (2 * err > dy) {
      x0 += sx;
      dx -= xy;
      err += dy += yy;
    }

    if (y1) {
      y0 += sy;
      dy -= xy;
      err += dx += xx;
    }
  } while (dy < 0 && dx > 0);
#endif
}

sgl_line(s, x0, y0, x2, y2, col);
}

void sgl_bezier(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int col) {
  int x = x0 - x1, y = y0 - y1;
  float t = x0 - 2 * x1 + x2, r;

  if ((long)x * (x2 - x1) > 0) {
    if ((long)y * (y2 - y1) > 0)
      if (fabsf((y0 - 2 * y1 + y2) / t * x) > abs(y)) {
        x0 = x2;
        x2 = x + x1;
        y0 = y2;
        y2 = y + y1;
      }

    t = (x0 - x1) / t;
    r = (1 - t) * ((1 - t) * y0 + 2.f * t * y1) + t * t*y2;
    t = (x0 * x2 - x1 * x1) * t / (x0 - x1);
    x = floorf(t + .5f);
    y = floorf(r + .5f);
    r = (y1 - y0) * (t - x0) / (x1 - x0) + y0;
    bezier_seg(s, x0, y0, x, floorf(r + .5f), x, y, col);
    r = (y1 - y2) * (t - x2) / (x1 - x2) + y2;
    x0 = x1 = x;
    y0 = y;
    y1 = floorf(r + .5f);
  }

  if ((long)(y0 - y1) * (y2 - y1) > 0) {
    t = y0 - 2 * y1 + y2; t = (y0 - y1) / t;
    r = (1 - t) * ((1 - t) * x0 + 2.f * t * x1) + t * t * x2;
    t = (y0 * y2 - y1 * y1) * t / (y0 - y1);
    x = floorf(r + .5f);
    y = floorf(t + .5f);
    r = (x1 - x0) * (t - y0) / (y1 - y0) + x0;
    bezier_seg(s, x0, y0, floorf(r + .5f), y, x, y, col);
    r = (x1 - x2) * (t - y2) / (y1 - y2) + x2;
    x0 = x;
    x1 = floorf(r + .5f);
    y0 = y1 = y;
  }

  bezier_seg(s, x0, y0, x1, y1, x2, y2, col);
}

static inline void bezier_seg_rational(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, float w, int col) {
  int sx = x2 - x1, sy = y2 - y1;
  float dx = x0 - x2, dy = y0 - y2, xx = x0 - x1, yy = y0 - y1;
  float xy = xx * sy + yy * sx, cur = xx * sy - yy * sx, err;

#if defined(SGL_ENABLE_AA)
  float ed;
  int f;
#endif

  if (cur != .0f && w > .0f) {
    if (sx * (long)sx + sy * (long)sy > xx * xx + yy * yy) {
      x2 = x0;
      x0 -= dx;
      y2 = y0;
      y0 -= dy;
      cur = -cur;
    }
    xx = 2.f * (4.f * w * sx * xx + dx * dx);
    yy = 2.f * (4.f * w * sy * yy + dy * dy);
    sx = x0 < x2 ? 1 : -1;
    sy = y0 < y2 ? 1 : -1;
    xy = -2.f * sx * sy * (2.f * w * xy + dx * dy);

    if (cur * sx * sy < .0f) {
      xx = -xx;
      yy = -yy;
      cur = -cur;
      xy = -xy;
    }
    dx = 4.f * w * (x1 - x0) * sy * cur + xx / 2.f + xy;
    dy = 4.f * w * (y0 - y1) * sx * cur + yy / 2.f + xy;

#if defined(SGL_ENABLE_AA)
    if (w < .5f && dy > dx) {
#else
      if (w < .5f && (dy > xy || dx < xy)) {
#endif
        cur = (w + 1.f) / 2.f;
        w = sqrtf(w);
        xy = 1.f / (w + 1.f);
        sx = floorf((x0 + 2.f * w * x1 + x2) * xy / 2.f + .5f);
        sy = floorf((y0 + 2.f * w * y1 + y2) * xy / 2.f + .5f);
        dx = floorf((w * x1 + x0) * xy + .5f);
        dy = floorf((y1 * w + y0) * xy + .5f);
        bezier_seg_rational(s, x0, y0, dx, dy, sx, sy, cur, col);
        dx = floorf((w * x1 + x2) * xy + .5f);
        dy = floorf((y1 * w + y2) * xy + .5f);
        bezier_seg_rational(s, sx, sy, dx, dy, x2, y2, cur, col);
        return;
      }

      err = dx + dy - xy;
      do {
#if defined(SGL_ENABLE_AA)
        cur = fminf(dx - xy, xy - dy);
        ed = fmaxf(dx - xy, xy - dy);
        ed += 2 * ed * cur * cur / (4.f * ed * ed + cur * cur);
        x1 = 255 * fabsf(err - dx - dy + xy) / ed;
        if (x1 < 256)
          sgl_psetb(s, x0, y0, ACHAN(col, 255 - x1));

        if ((f = 2 * err + dy < 0)) {
          if (y0 == y2)
            return;
          if (dx - err < ed)
            sgl_psetb(s, x0 + sx, y0, ACHAN(col, 255 - (int)(255 * fabsf(dx - err) / ed)));
        }

        if (2 * err + dx > 0) {
          if (x0 == x2)
            return;
          if (err - dy < ed)
            sgl_psetb(s, x0, y0 + sy, ACHAN(col, 255 - (int)(255 * fabsf(err - dy) / ed)));
          x0 += sx;
          dx += xy;
          err += dy += yy;
        }

        if (f) {
          y0 += sy;
          dy += xy;
          err += dx += xx;
        }
      } while (dy < dx);
#else
      pset_fn(s, x0, y0, col);
      if (x0 == x2 && y0 == y2)
        return;

      x1 = 2 * err > dy;
      y1 = 2 * (err + yy) < -dy;
      if (2 * err < dx || y1) {
        y0 += sy;
        dy += xy;
        err += dx += xx;
      }

      if (2 * err > dx || x1) {
        x0 += sx;
        dx += xy;
        err += dy += yy;
      }
    } while (dy < dx);
#endif
  }
  sgl_line(s, x0, y0, x2, y2, col);
}

void sgl_bezier_rational(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, float w, int col) {
  int x = x0 - 2 * x1 + x2, y = y0 - 2 * y1 + y2;
  float xx = x0 - x1, yy = y0 - y1, ww, t, q;

  if (xx * (x2 - x1) > 0) {
    if (yy * (y2 - y1) > 0)
      if (fabsf(xx * y) > fabsf(yy * x)) {
        x0 = x2;
        x2 = xx + x1;
        y0 = y2;
        y2 = yy + y1;
      }

    if (x0 == x2 || w == 1.f)
      t = (x0 - x1) / (double)x;
    else {
      q = sqrtf(4.f * w * w * (x0 - x1) * (x2 - x1) + (x2 - x0) * (long)(x2 - x0));
      if (x1 < x0)
        q = -q;
      t = (2.f * w * (x0 - x1) - x0 + x2 + q) / (2.f * (1.f - w) * (x2 - x0));
    }

    q = 1.f / (2.f * t * (1.f - t) * (w - 1.f) + 1.f);
    xx = (t * t * (x0 - 2.f * w * x1 + x2) + 2.f * t * (w * x1 - x0) + x0) * q;
    yy = (t * t * (y0 - 2.f * w * y1 + y2) + 2.f * t * (w * y1 - y0) + y0) * q;
    ww = t * (w - 1.f) + 1.f;
    ww *= ww * q;
    w = ((1.f - t) * (w - 1.f) + 1.f) * sqrtf(q);
    x = floorf(xx + .5f);
    y = floorf(yy + .5f);
    yy = (xx - x0) * (y1 - y0) / (x1 - x0) + y0;
    bezier_seg_rational(s, x0, y0, x, floorf(yy + .5f), x, y, ww, col);
    yy = (xx - x2) * (y1 - y2) / (x1 - x2) + y2;
    y1 = floorf(yy + .5f);
    x0 = x1 = x;
    y0 = y;
  }

  if ((y0 - y1) * (long)(y2 - y1) > 0) {
    if (y0 == y2 || w == 1.f)
      t = (y0 - y1) / (y0 - 2.f * y1 + y2);
    else {
      q = sqrtf(4.f * w * w * (y0 - y1) * (y2 - y1) + (y2 - y0) * (long)(y2 - y0));
      if (y1 < y0)
        q = -q;
      t = (2.f * w * (y0 - y1) - y0 + y2 + q) / (2.f * (1.f - w) * (y2 - y0));
    }

    q = 1.f / (2.f * t * (1.f - t) * (w - 1.f) + 1.f);
    xx = (t * t * (x0 - 2.f * w * x1 + x2) + 2.f * t * (w * x1 - x0) + x0) * q;
    yy = (t * t * (y0 - 2.f * w * y1 + y2) + 2.f * t * (w * y1 - y0) + y0) * q;
    ww = t * (w - 1.f) + 1.f;
    ww *= ww * q;
    w = ((1.f - t) * (w - 1.f) + 1.f) * sqrtf(q);
    x = floorf(xx + .5f);
    y = floorf(yy + .5f);
    xx = (x1 - x0) * (yy - y0) / (y1 - y0) + x0;
    bezier_seg_rational(s, x0, y0, floorf(xx + .5f), y, x, y, ww, col);
    xx = (x1 - x2) * (yy - y2) / (y1 - y2) + x2;
    x1 = floorf(xx + .5f);
    x0 = x;
    y0 = y1 = y;
  }

  bezier_seg_rational(s, x0, y0, x1, y1, x2, y2, w*w, col);
}

void sgl_ellipse_rect_rotated(surface_t* s, int x0, int y0, int x1, int y1, long zd, int col) {
#pragma TODO(Add fill option);
  int xd = x1 - x0, yd = y1 - y0;
  float w = xd * (long)yd;
  if (zd == 0)
    sgl_ellipse_rect(s, x0, y0, x1, y1, col, 0);
  if (w != 0.f)
    w = (w - zd) / (w + w);

  xd = floorf(xd * w + .5f);
  yd = floorf(yd * w + .5f);

  bezier_seg_rational(s, x0, y0 + yd, x0, y0, x0 + xd, y0, 1.f - w, col);
  bezier_seg_rational(s, x0, y0 + yd, x0, y1, x1 - xd, y1, w, col);
  bezier_seg_rational(s, x1, y1 - yd, x1, y1, x1 - xd, y1, 1.f - w, col);
  bezier_seg_rational(s, x1, y1 - yd, x1, y0, x0 + xd, y0, w, col);
}

void sgl_ellipse_rotated(surface_t* s, int x, int y, int a, int b, float angle, int col) {
#pragma TODO(Add fill option);
  float xd = (long)a * a, yd = (long)b * b;
  float q = sinf(angle), zd = (xd - yd) * q;
  xd = sqrtf(xd - zd * q);
  yd = sqrtf(yd + zd * q);
  a = xd + .5f;
  b = yd + .5f;
  zd = zd * a * b / (xd * yd);
  sgl_ellipse_rect_rotated(s, x - a, y - b, x + a, y + b, (long)(4 * zd * cosf(angle)), col);
}

static inline void bezier_seg_cubic(surface_t* s, int x0, int y0, float x1, float y1, float x2, float y2, int x3, int y3, int col) {
  int f, fx, fy, leg = 1;
  int sx = x0 < x3 ? 1 : -1, sy = y0 < y3 ? 1 : -1;
  float xc = -fabsf(x0 + x1 - x2 - x3), xa = xc - 4 * sx * (x1 - x2), xb = sx * (x0 - x1 - x2 + x3);
  float yc = -fabsf(y0 + y1 - y2 - y3), ya = yc - 4 * sy * (y1 - y2), yb = sy * (y0 - y1 - y2 + y3);
  float ab, ac, bc, ba, xx, xy, yy, dx, dy, ex, EP = .01f;

#if defined(SGL_ENABLE_AA)
  float px, py, ed, ip;
#else
  float* pxy;
#endif

  if (xa == 0.f && ya == 0.f) {
    sx = floorf((3 * x1 - x0 + 1) / 2); sy = floorf((3 * y1 - y0 + 1) / 2);
    bezier_seg(s, x0, y0, sx, sy, x3, y3, col);
    return;
  }

  x1 = (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0) + 1;
  x2 = (x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3) + 1;

  do {
    ab = xa * yb - xb * ya;
    ac = xa * yc - xc * ya;
    bc = xb * yc - xc * yb;
    ex = ab * (ab + ac - 3 * bc) + ac * ac;
    f = ex > 0 ? 1 : sqrtf(1 + 1024 / x1);

    ab *= f; ac *= f;
    bc *= f;
    ex *= f * f;
    xy = 9 * (ab + ac + bc) / 8;
#if defined(SGL_ENABLE_AA)
    ip = 4 * ab * bc - ac * ac;
#endif
    ba = 8 * (xa - ya);
    dx = 27 * (8 * ab * (yb * yb - ya * yc) + ex * (ya + 2 * yb + yc)) / 64 - ya * ya * (xy - ya);
    dy = 27 * (8 * ab * (xb * xb - xa * xc) - ex * (xa + 2 * xb + xc)) / 64 - xa * xa * (xy + xa);

    xx = 3 * (3 * ab * (3 * yb * yb - ya * ya - 2 * ya * yc) - ya * (3 * ac * (ya + yb) + ya * ba)) / 4;
    yy = 3 * (3 * ab * (3 * xb * xb - xa * xa - 2 * xa * xc) - xa * (3 * ac * (xa + xb) + xa * ba)) / 4;
    xy = xa * ya * (6 * ab + 6 * ac - 3 * bc + ba);
    ac = ya * ya;
    ba = xa * xa;
    xy = 3 * (xy + 9 * f * (ba * yb * yc - xb * xc * ac) - 18 * xb * yb * ab) / 8;

    if (ex < 0) {
      dx = -dx;
      dy = -dy;
      xx = -xx;
      yy = -yy;
      xy = -xy;
      ac = -ac;
      ba = -ba;
    }
    ab = 6 * ya * ac;
    ac = -6 * xa * ac;
    bc = 6 * ya * ba;
    ba = -6 * xa * ba;
    dx += xy;
    ex = dx + dy;
    dy += xy;

#if defined(SGL_ENABLE_AA)
    for (fx = fy = f; x0 != x3 && y0 != y3; ) {
      y1 = fminf(fabsf(xy - dx), fabsf(dy - xy));
      ed = fmaxf(fabsf(xy - dx), fabsf(dy - xy));
      ed = f * (ed + 2 * ed * y1 * y1 / (4 * ed * ed + y1 * y1));
      y1 = 255 * fabsf(ex - (f - fx + 1) * dx - (f - fy + 1) * dy + f * xy) / ed;
      if (y1 < 256)
        sgl_psetb(s, x0, y0, ACHAN(col, 255 - (int)y1));
      px = fabsf(ex - (f - fx + 1) * dx + (fy - 1) * dy);
      py = fabsf(ex + (fx - 1) * dx - (f - fy + 1) * dy);
      y2 = y0;
      do {
        if (ip >= -EP)
          if (dx + xx > xy || dy + yy < xy)
            goto exit;

        y1 = 2 * ex + dx;
        if (2 * ex + dy > 0) {
          fx--;
          ex += dx += xx;
          dy += xy += ac;
          yy += bc;
          xx += ab;
        }
        else if (y1 > 0)
          goto exit;
        if (y1 <= 0) {
          fy--;
          ex += dy += yy;
          dx += xy += bc;
          xx += ac;
          yy += ba;
        }
      } while (fx > 0 && fy > 0);

      if (2 * fy <= f) {
        if (py < ed)
          sgl_psetb(s, x0 + sx, y0, ACHAN(col, 255 - (int)(255 * px / ed)));
        y0 += sy;
        fy += f;
      }

      if (2 * fx <= f) {
        if (px < ed)
          sgl_psetb(s, x0, (int)y2 + sy, ACHAN(col, 255 - (int)(255 * px / ed)));
        x0 += sx;
        fx += f;
      }
    }
    break;

  exit:
    if (2 * ex < dy && 2 * fy <= f + 2) {
      if (py < ed)
        sgl_psetb(s, x0 + sx, y0, ACHAN(col, 255 - (int)(255 * px / ed)));
      y0 += sy;
    }

    if (2 * ex > dx && 2 * fx <= f + 2) {
      if (px < ed)
        sgl_psetb(s, x0, (int)y2 + sy, ACHAN(col, 255 - (int)(255 * px / ed)));
      x0 += sx;
    }

    xx = x0;
    x0 = x3;
    x3 = xx;
    sx = -sx;
    xb = -xb;
    yy = y0;
    y0 = y3;
    y3 = yy;
    sy = -sy;
    yb = -yb;
    x1 = x2;
#else
    for (pxy = &xy, fx = fy = f; x0 != x3 && y0 != y3;) {
      pset_fn(s, x0, y0, col);
      do {
        if (dx > *pxy || dy < *pxy)
          goto exit;

        y1 = 2 * ex - dy;
        if (2 * ex >= dx) {
          fx--;
          ex += dx += xx;
          dy += xy += ac;
          yy += bc;
          xx += ab;
        }

        if (y1 <= 0) {
          fy--;
          ex += dy += yy;
          dx += xy += bc;
          xx += ac;
          yy += ba;
        }
      } while (fx > 0 && fy > 0);

      if (2 * fx <= f) {
        x0 += sx;
        fx += f;
      }
      if (2 * fy <= f) {
        y0 += sy;
        fy += f;
      }
      if (pxy == &xy && dx < 0 && dy > 0)
        pxy = &EP;
    }

  exit:
    xx = x0;
    x0 = x3;
    x3 = xx;
    sx = -sx;
    xb = -xb;
    yy = y0;
    y0 = y3;
    y3 = yy;
    sy = -sy;
    yb = -yb;
    x1 = x2;
#endif
  } while (leg--);

  sgl_line(s, x0, y0, x3, y3, col);
}

void sgl_bezier_cubic(surface_t* s, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, int col) {
  int n = 0, i = 0;
  long xc = x0 + x1 - x2 - x3, xa = xc - 4 * (x1 - x2);
  long xb = x0 - x1 - x2 + x3, xd = xb + 4 * (x1 + x2);
  long yc = y0 + y1 - y2 - y3, ya = yc - 4 * (y1 - y2);
  long yb = y0 - y1 - y2 + y3, yd = yb + 4 * (y1 + y2);
  float fx0 = x0, fx1, fx2, fx3, fy0 = y0, fy1, fy2, fy3;
  float t1 = xb * xb - xa * xc, t2, t[5];

  if (xa == 0) {
    if (labs(xc) < 2 * labs(xb))
      t[n++] = xc / (2.f * xb);
  } else if (t1 > .0f) {
    t2 = sqrtf(t1);
    t1 = (xb - t2) / xa;
    if (fabsf(t1) < 1.f)
      t[n++] = t1;
    t1 = (xb + t2) / xa;
    if (fabsf(t1) < 1.f)
      t[n++] = t1;
  }

  t1 = yb * yb - ya * yc;
  if (ya == 0) {
    if (labs(yc) < 2 * labs(yb))
      t[n++] = yc / (2.f * yb);
  } else if (t1 > .0f) {
    t2 = sqrtf(t1);
    t1 = (yb - t2) / ya;
    if (fabsf(t1) < 1.f)
      t[n++] = t1;
    t1 = (yb + t2) / ya;
    if (fabsf(t1) < 1.f)
      t[n++] = t1;
  }

  for (i = 1; i < n; i++)
    if ((t1 = t[i - 1]) > t[i]) {
      t[i - 1] = t[i];
      t[i] = t1; i = 0;
    }

  t1 = -1.; t[n] = 1.;
  for (i = 0; i <= n; i++) {
    t2 = t[i];
    fx1 = (t1 * (t1 * xb - 2 * xc) - t2 * (t1 * (t1 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
    fy1 = (t1 * (t1 * yb - 2 * yc) - t2 * (t1 * (t1 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
    fx2 = (t2 * (t2 * xb - 2 * xc) - t1 * (t2 * (t2 * xa - 2 * xb) + xc) + xd) / 8 - fx0;
    fy2 = (t2 * (t2 * yb - 2 * yc) - t1 * (t2 * (t2 * ya - 2 * yb) + yc) + yd) / 8 - fy0;
    fx0 -= fx3 = (t2 * (t2 * (3 * xb - t2 * xa) - 3 * xc) + xd) / 8;
    fy0 -= fy3 = (t2 * (t2 * (3 * yb - t2 * ya) - 3 * yc) + yd) / 8;
    x3 = floorf(fx3 + .5f);
    y3 = floorf(fy3 + .5f);

    if (fx0 != .0f) {
      fx1 *= fx0 = (x0 - x3) / fx0;
      fx2 *= fx0;
    }
    if (fy0 != .0f) {
      fy1 *= fy0 = (y0 - y3) / fy0;
      fy2 *= fy0;
    }
    if (x0 != x3 || y0 != y3)
      bezier_seg_cubic(s, x0, y0, x0 + fx1, y0 + fy1, x0 + fx2, y0 + fy2, x3, y3, col);

    x0 = x3;
    y0 = y3;
    fx0 = fx3;
    fy0 = fy3;
    t1 = t2;
  }
}

void sgl_rect(surface_t* s, int x, int y, int w, int h, int col, int fill) {
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
      sgl_hline(s, y, x, w, col);
  } else {
    sgl_hline(s, y, x, w, col);
    sgl_hline(s, h, x, w, col);
    sgl_vline(s, x, y, h, col);
    sgl_vline(s, w, y, h, col);
  }
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

#define BMP_SET(c) (s->buf[(i - (i % info.width)) + (info.width - (i % info.width) - 1)] = (c));

bool sgl_bmp(surface_t* s, const char* path) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    error_handle(NORMAL_PRIORITY, FILE_OPEN_FAILED, "fopen() failed: %s", path);
    return false;
  }

  fseek(fp, 0, SEEK_END);
  size_t length = ftell(fp);
  rewind(fp);

  unsigned char* data = (unsigned char*)calloc(length + 1, sizeof(unsigned char));
  if (!data) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "calloc() failed");
    return false;
  }
  fread(data, 1, length, fp);
  fclose(fp);

  int off = 0;
  BMPHEADER header;
  BMPINFOHEADER info;
  //BMPCOREHEADER core;
  BMP_GET(&header, data, sizeof(BMPHEADER));
  //int info_pos = off;
  BMP_GET(&info, data, sizeof(BMPINFOHEADER));

  if (header.type != 0x4D42) {
    error_handle(NORMAL_PRIORITY, INVALID_BMP, "bmp() failed: invalid BMP signiture '%d'", header.type);
    return false;
  }

#pragma TODO(Add support for OS/2 bitmaps)

  unsigned char* color_map = NULL;
  int color_map_size = 0;
  if (info.bits <= 8) {
    color_map_size = (1 << info.bits) * 4;
    color_map = (unsigned char*)malloc(color_map_size * sizeof(unsigned char));
    if (!color_map) {
      error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
      return false;
    }
    BMP_GET(color_map, data, color_map_size);
  }

  if (!sgl_surface(s, info.width, info.height)) {
    FREE_SAFE(color_map);
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  off = header.offset;
  int i, sz = info.width * info.height;
  unsigned char color;
  switch (info.compression) {
    case 0: // RGB
      switch (info.bits) { // BPP
        case 1:
        case 4:
#pragma TODO(Add 1 & 4 bpp support);
          error_handle(NORMAL_PRIORITY, UNSUPPORTED_BMP, "bmp() failed. Unsupported BPP: %d", info.bits);
          sgl_destroy(s);
          break;
        case 8:
          for (i = (sz - 1); i != -1; --i, ++off) {
            color = data[off];
            BMP_SET(RGB(color_map[(color * 4) + 2], color_map[(color * 4) + 1], color_map[(color * 4)]));
          }
          break;
        case 24:
        case 32:
          for (i = (sz - 1); i != -1; --i, off += (info.bits == 32 ? 4 : 3))
            BMP_SET(RGB(data[off + 2], data[off + 1], data[off]));
          break;
        default:
          error_handle(NORMAL_PRIORITY, UNSUPPORTED_BMP, "bmp() failed. Unsupported BPP: %d", info.bits);
          sgl_destroy(s);
          return false;
      }
      break;
    case 1: // RLE8
    case 2: // RLE4
    default:
#pragma TODO(Add RLE support);
      error_handle(NORMAL_PRIORITY, UNSUPPORTED_BMP, "bmp() failed. Unsupported compression: %d", info.compression);
      sgl_destroy(s);
      return false;
  }

  FREE_SAFE(color_map);
  return true;
}

bool sgl_save_bmp(surface_t* s, const char* path) {
  const int filesize = 54 + 3 * s->w * s->h;
  unsigned char* img = (unsigned char *)malloc(3 * s->w * s->h);
  if (!img) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  memset(img, 0, 3 * s->w * s->h);

  int i, j, y, c;
  for (i = 0; i < s->w; ++i) {
    for (j = s->h; j > 0; --j) {
      y = (s->h - 1) - j;
      c = XYGET(s, i, y);
      img[(i + y * s->w) * 3 + 2] = (unsigned char)R(c);
      img[(i + y * s->w) * 3 + 1] = (unsigned char)G(c);
      img[(i + y * s->w) * 3 + 0] = (unsigned char)B(c);
    }
  }

  unsigned char header[14] = {'B', 'M',
    0,  0, 0, 0,
    0,  0,
    0,  0,
    54, 0, 0, 0};
  unsigned char info[40] = {40, 0, 0, 0,
    0,  0, 0, 0,
    0,  0, 0, 0,
    1,  0,
    24, 0};
  unsigned char pad[3] = {0, 0, 0};

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
    error_handle(NORMAL_PRIORITY, FILE_OPEN_FAILED, "fopen() failed: %s", path);
    free(img);
    return false;
  }

  fwrite(header, 1, 14, fp);
  fwrite(info, 1, 40, fp);
  for(i = 0; i < s->h; ++i) {
    fwrite(img + (s->w * (s->h - i - 1) * 3), 3, s->w, fp);
    fwrite(pad, 1, (4 - (s->w * 3) % 4) % 4,fp);
  }

  free(img);
  fclose(fp);
  return true;
}

#if !defined(SGL_DISABLE_TEXT) || defined(SGL_ENABLE_BDF) || defined(SGL_ENABLE_FREETYPE)
int read_color(const char* str, int* col, int* len) {
#pragma FIXME(Alpha value wrong?)
  const char* c = str;
  if (*c != '(')
    return 0;
  int _len = 0;
  while (c) {
    _len++;
    if (*c == ')' || _len > 17)
      break;
    c++;
  }
  if (_len > 17)
    return 0;
  *len = _len + 1;
  if (!col)
    return 1; // Skip colour parsing

  c = str + 1;
  char rgba[4][4] = {
    "0",
    "0",
    "0",
    "255"
  };
  int n = 0;
  for (int i = 0, j = 0, k = 0; i < (_len - 2); ++i) {
    if (*c == ',') {
      k++;
      n++;
      j = 0;
    }
    else if (isdigit(*c) != -1) {
      rgba[k][j++] = *c;
    }
    else
      return 0;
    c++;
  }

  *col = RGBA(atoi(rgba[0]), atoi(rgba[1]), atoi(rgba[2]), atoi(rgba[3]));
  return 1;
}

static inline void str_size(const char* str, int* w, int* h) {
  const char* s = (const char*)str;
  int n = 0, m = 0, l = 1, c, len;
  while (s && *s != '\0') {
    c = *s;
    if (c >= 0 && c <= 127) {
      switch (c) {
      case '\f':
      case '\b':
        if (read_color(s, NULL, &len)) {
          s += len;
          continue;
        }
        else
          s++;
        break;
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
  *w = MAX(n, m);
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
#endif

#if !defined(SGL_DISABLE_TEXT)
static char font[540][8] = {
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

#define LINE_HEIGHT 10

static inline int letter_index(int c) {
#if defined(_MSC_VER)
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
  }
#else
  switch (c) {
    case 32 ... 126: // Latin
      return c - 32;
    case 9600 ... 9631: // Blocks
      return (c - 9600) + 95;
    case 9472 ...9599: // Box
      return (c - 9472) + 127;
    case 912 ... 969: // Greek
      return (c - 912) + 255;
    case 12352 ... 12447: // Hiragana
      return (c - 12352) + 313;
    case 58689 ... 58714: // SGA
      return (c - 58689) + 409;
    case 161 ... 255: // Latin extended
      return (c - 161) + 435;
    default: // Extra
      for (int i = 0; i < 10; ++i)
        if (extra_font_lookup[i] == c)
          return 530 + i;
      break;
  }
#endif
  return 0;
}

void sgl_ascii(surface_t* s, char ch, int x, int y, int fg, int bg) {
  int c = letter_index((int)ch), i, j;
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j) {
      if (font[c][i] & 1 << j) {
        pset_fn(s, x + j, y + i, fg);
      } else {
        if (bg == -1)
          continue;
        pset_fn(s, x + j, y + i, bg);
      }
    }
  }
}

int sgl_character(surface_t* s, const char* ch, int x, int y, int fg, int bg) {
  int u = -1;
  int l = ctoi(ch, &u);
  int uc = letter_index(u), i, j;
  for (i = 0; i < 8; ++i)
    for (j = 0; j < 8; ++j) {
      if (font[uc][i] & 1 << j)
        pset_fn(s, x + j, y + i, fg);
      else {
        if (bg == -1)
          continue;
        pset_fn(s, x + j, y + i, bg);
      }
    }

  return l;
}

#pragma TODO(All writeln/string functions similar, combine somehow)
void sgl_writeln(surface_t* s, int x, int y, int fg, int bg, const char* str) {
  const char* c = str;
  int u = x, v = y, col, len;
  while (c && *c != '\0')
    switch (*c) {
      case '\n':
        v += LINE_HEIGHT;
        u  = x;
        c++;
        break;
      case '\f':
        if (read_color(c + 1, &col, &len)) {
          fg = col;
          c += len;
        } else
          c++;
        break;
      case '\b':
        if (read_color(c + 1, &col, &len)) {
          bg = col;
          c += len;
        } else
          c++;
        break;
      default:
        c += sgl_character(s, c, u, v, fg, bg);
        u += 8;
        break;
    }
}

void sgl_writelnf(surface_t* s, int x, int y, int fg, int bg, const char* fmt, ...) {
  char *buffer = NULL;
  int buffer_size = 0;

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

  sgl_writeln(s, x, y, fg, bg, buffer);
  free(buffer);
}

void sgl_string(surface_t* out, int fg, int bg, const char* str) {
  int w, h;
  str_size(str, &w, &h);
  sgl_surface(out, w * 8, h * LINE_HEIGHT);
  sgl_fill(out, (bg == -1 ? 0 : bg));
  sgl_writeln(out, 0, 0, fg, bg, str);
}

void sgl_stringf(surface_t* out, int fg, int bg, const char* fmt, ...) {
  char *buffer = NULL;
  int buffer_size = 0;

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

  sgl_string(out, fg, bg, buffer);
  free(buffer);
}
#endif

static int ticks_started = 0;

long sgl_ticks() {
#if defined(SGL_WINDOWS)
  static LARGE_INTEGER ticks_start;
  if (!ticks_started) {
    QueryPerformanceCounter(&ticks_start);
    ticks_started = 1;
  }

  LARGE_INTEGER ticks_now, freq;
  QueryPerformanceCounter(&ticks_now);
  QueryPerformanceFrequency(&freq);

  return ((ticks_now.QuadPart - ticks_start.QuadPart) * 1000) / freq.QuadPart;
#else
  static struct timespec ticks_start;
  if (!ticks_started) {
    clock_gettime(CLOCK_MONOTONIC, &ticks_start);
    ticks_started = 1;
  }

  struct timespec ticks_now;
  clock_gettime(CLOCK_MONOTONIC, &ticks_now);
  return ((ticks_now.tv_sec * 1000) + (ticks_now.tv_nsec / 1000000)) - ((ticks_start.tv_sec * 1000) + (ticks_start.tv_nsec / 1000000));
#endif
}

void sgl_delay(long ms) {
#if defined(SGL_WINDOWS)
  Sleep((DWORD)ms);
#else
  usleep((unsigned int)(ms * 1000));
#endif
}

#if defined(SGL_ENABLE_BDF)
#if defined(SGL_WINDOWS)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#define BDF_READ_INT(x) \
p = strtok(NULL, " \t\n\r"); \
(x) = atoi(p);

typedef struct {
  unsigned int width;
  unsigned char* bitmap;
  rect_t bb;
} bdf_char_t;

struct bdf_t {
  rect_t fontbb;
  unsigned int* encoding_table;
  bdf_char_t* chars;
  int n_chars;
};

void sgl_bdf_destroy(struct bdf_t** _f) {
  if (!*_f)
    return;
  
  struct bdf_t* f = *_f;
  for (int i = 0; i < f->n_chars; ++i)
    if (f->chars[i].bitmap) {
      free(f->chars[i].bitmap);
      f->chars[i].bitmap = NULL;
    }
  if (f->chars) {
    free(f->chars);
    f->chars = NULL;
  }
  if (f->encoding_table) {
    free(f->encoding_table);
    f->encoding_table = NULL;
  }
  f->n_chars = 0;
  memset(&f->fontbb, 0, sizeof(rect_t));
  free(f);
}

static inline int htoi(const char* p) {
  return (*p <= '9' ? *p - '0' : (*p <= 'F' ? *p - 'A' + 10 : *p - 'a' + 10));
}

bool sgl_bdf(struct bdf_t** _out, const char* path) {
  struct bdf_t* out = *_out = malloc(sizeof(struct bdf_t));
  if (!out) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  
  FILE* fp = fopen(path, "r");
  if (!fp) {
    error_handle(NORMAL_PRIORITY, FILE_OPEN_FAILED, "fopen() failed: %s", path);
    return false;
  }

  char *s, *p, buf[BUFSIZ];
  for (;;) {
    if (!fgets(buf, sizeof(buf), fp))
      break;
    if (!(s = strtok(buf, " \t\n\r")))
      break;

    if (!strcasecmp(s, "FONTBOUNDINGBOX")) {
      BDF_READ_INT(out->fontbb.w);
      BDF_READ_INT(out->fontbb.h);
      BDF_READ_INT(out->fontbb.x);
      BDF_READ_INT(out->fontbb.y);
    } else if (!strcasecmp(s, "CHARS")) {
      BDF_READ_INT(out->n_chars);
      break;
    }
  }

  if (out->fontbb.w <= 0 || out->fontbb.h <= 0) {
    error_handle(NORMAL_PRIORITY, BDF_NO_CHAR_SIZE, "bdf() failed: No character size given for %s", path);
    return false;
  }

  if (out->n_chars <= 0) {
    error_handle(NORMAL_PRIORITY, BDF_NO_CHAR_LENGTH, "bdf() failed: Unknown number of characters for %s", path);
    return false;
  }

  out->encoding_table = malloc(out->n_chars * sizeof(unsigned int));
  if (!out->encoding_table) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  out->chars = malloc(out->n_chars * sizeof(bdf_char_t));
  if (!out->chars) {
    free(out->encoding_table);
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  int encoding = 0, width = -1, scanline = -1, i, j, n = 0;
  for (;;) {
    if (!fgets(buf, sizeof(buf), fp))
      break;
    if (!(s = strtok(buf, " \t\n\r")))
      break;

    if (!strcasecmp(s, "ENCODING")) {
      BDF_READ_INT(encoding);
    } else if (!strcasecmp(s, "DWIDTH")) {
      BDF_READ_INT(width);
    } else if (!strcasecmp(s, "BBX")) {
      BDF_READ_INT(out->chars[n].bb.w);
      BDF_READ_INT(out->chars[n].bb.h);
      BDF_READ_INT(out->chars[n].bb.x);
      BDF_READ_INT(out->chars[n].bb.y);
    } else if (!strcasecmp(s, "BITMAP")) {
      if (n == out->n_chars) {
        sgl_bdf_destroy(_out);
        error_handle(NORMAL_PRIORITY, BDF_TOO_MANY_BITMAPS, "bdf() failed: More bitmaps than characters for %s", path);
        return false;
      }
      if (width == -1) {
        sgl_bdf_destroy(_out);
        error_handle(NORMAL_PRIORITY, BDF_UNKNOWN_CHAR, "bdf() failed: Unknown character with for %s", path);
        return false;
      }

      if (out->chars[n].bb.x < 0) {
        width -= out->chars[n].bb.x;
        out->chars[n].bb.x = 0;
      }
      if (out->chars[n].bb.x + out->chars[n].bb.w > width)
        width = out->chars[n].bb.x + out->chars[n].bb.w;

      out->chars[n].bitmap = malloc(((out->fontbb.w + 7) / 8) * out->fontbb.h * sizeof(unsigned char));
      if (!out->chars[n].bitmap) {
        sgl_bdf_destroy(_out);
        error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
        return false;
      }
      out->chars[n].width = width;
      out->encoding_table[n] = encoding;

      scanline = 0;
      memset(out->chars[n].bitmap, 0, ((out->fontbb.w + 7) / 8) * out->fontbb.h);
    } else if (!strcasecmp(s, "ENDCHAR")) {
      if (out->chars[n].bb.x) {
        if (out->chars[n].bb.x < 0 || out->chars[n].bb.x > 7)
          continue;

        int x, y, c, o;
        for (y = 0; y < out->fontbb.h; ++y) {
          o = 0;
          for (x = 0; x < out->fontbb.w; x += 8) {
            c = out->chars[n].bitmap[y * ((out->fontbb.w + 7) / 8) + x / 8];
            out->chars[n].bitmap[y * ((out->fontbb.w + 7) / 8) + x / 8] = c >> out->chars[n].bb.x | o;
            o = c << (8 - out->chars[n].bb.x);
          }
        }
      }

      scanline = -1;
      width = -1;
      ++n;
    } else {
      if (n >= out->n_chars || !out->chars[n].bitmap || scanline < 0)
        continue;

      p = s;
      j = 0;
      while (*p) {
        i = htoi(p);
        ++p;
        if (*p)
          i = htoi(p) | i * 16;
        else {
          out->chars[n].bitmap[j + scanline * ((out->fontbb.w + 7) / 8)] = i;
          break;
        }

        out->chars[n].bitmap[j + scanline * ((out->fontbb.w + 7) / 8)] = i;
        ++j;
        ++p;
      }
      ++scanline;
    }
  }

  fclose(fp);
  return true;
}

int sgl_bdf_character(surface_t* s, struct bdf_t* f, const char* ch, int x, int y, int fg, int bg) {
  int u = -1, i, j, n;
  int l = ctoi(ch, &u);
  for (i = 0; i < f->n_chars; ++i)
    if (f->encoding_table[i] == u) {
      n = i;
      break;
    }

  int yoffset = f->fontbb.h - f->chars[n].bb.h + (f->fontbb.y - f->chars[n].bb.y), xx, yy, cc;
  for (yy = 0; yy < f->fontbb.h; ++yy) {
    for (xx = 0; xx < f->fontbb.w; xx += 8) {
      cc = (yy < yoffset || yy > yoffset + f->chars[n].bb.h ? 0 : f->chars[n].bitmap[(yy - yoffset) * ((f->fontbb.w + 7) / 8) + xx / 8]);

      for (i = 128, j = 0; i; i /= 2, ++j)
        pset_fn(s, x + j, y + yy, (cc & i ? fg : bg));
    }
  }

  return l;
}

void sgl_bdf_writeln(surface_t* s, struct bdf_t* f, int x, int y, int fg, int bg, const char* str) {
  const char* c = (const char*)str;
  int u = x, v = y, col, len;
  while (c && *c != '\0') {
    switch (*c) {
      case '\n':
        v += f->fontbb.h + 2;
        u = x;
        c++;
        break;
      case '\f':
        if (read_color(c + 1, &col, &len)) {
          fg = col;
          c += len;
        }
        else
          c++;
        break;
      case '\b':
        if (read_color(c + 1, &col, &len)) {
          bg = col;
          c += len;
        }
        else
          c++;
        break;
      default:
        c += sgl_bdf_character(s, f, c, u, v, fg, bg);
        u += 8;
        break;
    }
  }
}

void sgl_bdf_writelnf(surface_t* s, struct bdf_t* f, int x, int y, int fg, int bg, const char* fmt, ...) {
  char *buffer = NULL;
  int buffer_size = 0;

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

  sgl_bdf_writeln(s, f, x, y, fg, bg, buffer);
  free(buffer);
}

void sgl_bdf_string(surface_t* out, struct bdf_t* f, int fg, int bg, const char* str) {
  int w, h;
  str_size(str, &w, &h);
  sgl_surface(out, w * 8, h * LINE_HEIGHT);
  sgl_fill(out, (bg == -1 ? 0 : bg));
  sgl_bdf_writeln(out, f, 0, 0, fg, bg, str);
}

void sgl_bdf_stringf(surface_t* out, struct bdf_t* f, int fg, int bg, const char* fmt, ...) {
  char *buffer = NULL;
  int buffer_size = 0;

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

  sgl_bdf_string(out, f, fg, bg, buffer);
  free(buffer);
}
#endif

#if defined(SGL_ENABLE_FREETYPE)
#include <ft2build.h>
#include FT_FREETYPE_H

#pragma TODO(Add error handling to Freetype stuff)

static FT_Library ft_library;

typedef struct {
  point_t size, bearing;
  long advance;
  surface_t buffer;
} ft_char_t;

struct ftfont_t {
  FT_Face face;
  unsigned int* encoding_table;
  ft_char_t* chars;
};

void sgl_ft_init() {
  if (FT_Init_FreeType(&ft_library))
    abort();
}

void sgl_ft_release() {
  FT_Done_FreeType(ft_library);
}

static int ftfont_char_index(struct ftfont_t* font, int c) {
  int i, x = -1;
  for (i = 0; i < stb_sb_count(font->encoding_table); ++i)
    if (font->encoding_table[i] == c) {
      x = i;
      break;
    }
  return x;
}

static int load_ftfont_char(struct ftfont_t* font, const char* ch) {
  int u = -1;
  ctoi(ch, &u);
  
  if (ftfont_char_index(font, u) >= 0)
    abort(); // This probably shouldn't happen
  if (FT_Load_Char(font->face, u, FT_LOAD_RENDER))
    abort();
  
  ft_char_t new;
  new.size.x    = font->face->glyph->bitmap.width;
  new.size.y    = font->face->glyph->bitmap.rows;
  new.bearing.x = font->face->glyph->bitmap_left;
  new.bearing.y = font->face->glyph->bitmap_top;
  new.advance = font->face->glyph->advance.x;
  sgl_surface(&new.buffer, new.size.x, new.size.y);
  
  int i, j, b;
  for (i = 0; i < new.size.x; ++i)
    for (j = 0; j < new.size.y; ++j) {
      b = B(font->face->glyph->bitmap.buffer[j * new.size.x + i]);
      sgl_psetb(&new.buffer, i, j, b ? RGBA1(0, b) : 0);
    }
  
  stb_sb_push(font->encoding_table, u);
  stb_sb_push(font->chars, new);
  return stb_sb_count(font->encoding_table) - 1;
}

void sgl_ftfont(struct ftfont_t** _font, const char* path, unsigned int size) {
  struct ftfont_t* font = *_font = malloc(sizeof(struct ftfont_t));
  if (!font)
    abort();
  
  if (FT_New_Face(ft_library, path, 0, &font->face))
    abort();
  FT_Set_Pixel_Sizes(font->face, 0, size);
  
  font->encoding_table = NULL;
  font->chars = NULL;
  
  static const char* default_chars = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}";
  const char* c = default_chars;
  while (*c) {
    load_ftfont_char(font, c);
    c++;
  }
}

void sgl_ftfont_destroy(struct ftfont_t** _font) {
  if (!*_font)
    abort();
  
  struct ftfont_t* font = *_font;
  FT_Done_Face(font->face);
  stb_sb_free(font->encoding_table);
  for (int i = 0; i < stb_sb_count(font->chars); ++i)
    sgl_destroy(&font->chars[i].buffer);
  stb_sb_free(font->chars);
  free(font);
}

int sgl_ftfont_character(surface_t* s, ftfont_t f, const char* ch, int x, int y, int fg, int bg, int* w, int* h) {
  int u = -1, i, j;
  int l = ctoi(ch, &u);
  int index = ftfont_char_index(f, u);
  if (index == -1)
    index = load_ftfont_char(f, ch);
  
  ft_char_t* c = &f->chars[index];
  y -= c->bearing.y;
  for (i = 0; i < c->size.x; ++i) {
    for (j = 0; j < c->size.y; ++j) {
      pset_fn(s, x + i, y + j, bg);
#pragma TODO(Find better solution than called pset twice)
#pragma TODO(Update other font renderers to fix alpha)
      pset_fn(s, x + i, y + j, ACHAN(fg, CLAMP(A(fg) - (255 - A(XYGET((&(c->buffer)), i, j))), 0, 255)));
    }
  }
  
  if (w)
    *w = (int)(c->advance >> 6);
  if (h)
    *h = c->size.y;
  return l;
}

void sgl_ftfont_writeln(surface_t* s, ftfont_t f, int x, int y, int fg, int bg, const char* str) {
  const char* c = (const char*)str;
  int u = x, v = y, w, lh, h = 0, col, len;
  while (c && *c != '\0') {
    switch (*c) {
      case '\n':
        v += h + 6;
        h  = 0;
        u  = x;
        c++;
        break;
      case '\f':
        if (read_color(c + 1, &col, &len)) {
          fg = col;
          c += len;
        }
        else
          c++;
        break;
      case '\b':
        if (read_color(c + 1, &col, &len)) {
          bg = col;
          c += len;
        }
        else
          c++;
        break;
      default:
        c += sgl_ftfont_character(s, f, c, u, v, fg, bg, &w, &lh);
        if (lh > h)
          h = lh;
        u += w;
        break;
    }
  }
}

void sgl_ftfont_writelnf(surface_t* s, ftfont_t f, int x, int y, int fg, int bg, const char* fmt, ...) {
  char *buffer = NULL;
  int buffer_size = 0;
  
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
  
  sgl_ftfont_writeln(s, f, x, y, fg, bg, buffer);
  free(buffer);
}

void sgl_ftfont_string(surface_t* out, ftfont_t f, int fg, int bg, const char* str) {
  const char* s = (const char*)str;
  int n = 0, m = 0, nn = 0, mm = 6, c, len, index;
  ft_char_t* ch = NULL;
  while (s && *s != '\0') {
    c = *s;
    if (c >= 0 && c <= 127) {
      switch (c) {
        case '\n':
          if (ch)
            nn -= (int)(ch->advance >> 6);
          if (nn > n)
            n = nn;
          nn = 0;
          m += (mm + 6);
          s++;
          break;
        case '\f':
        case '\b':
          if (read_color(s, NULL, &len)) {
            s += len;
            continue;
          }
          else
            s++;
          break;
        default:
          index = ftfont_char_index(f, c);
          if (index == -1)
            index = load_ftfont_char(f, s);
          
          ch = &f->chars[index];
          nn += (int)(ch->advance >> 6);
          if (ch->size.y > mm)
            mm = ch->size.y;
          s++;
          break;
      }
    } else {
      len = ctoi(s, &c);
      index = ftfont_char_index(f, c);
      if (index == -1)
        index = load_ftfont_char(f, s);
    }
  }
  if (nn > n)
    n = nn;
  m += mm + 6;
  
  sgl_surface(out, n, m);
  sgl_fill(out, bg);
  sgl_ftfont_writeln(out, f, 0, mm, fg, 0, str);
}

void sgl_ftfont_stringf(surface_t* out, ftfont_t f, int fg, int bg, const char* fmt, ...) {
  char *buffer = NULL;
  int buffer_size = 0;
  
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
  
  sgl_ftfont_string(out, f, fg, bg, buffer);
  free(buffer);
}
#endif

#if defined(SGL_ENABLE_STB_IMAGE)
#define STB_IMAGE_IMPLEMENTATION
#if !defined(STB_IMAGE_PATH)
#include <stb_image.h>
#else
#include STB_IMAGE_PATH
#endif
#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(_MSC_VER)
#define STBI_MSC_SECURE_CRT
#endif
#if !defined(STB_IMAGE_WRITE_PATH)
#include <stb_image_write.h>
#else
#include STB_IMAGE_WRITE_PATH
#endif

bool sgl_image(surface_t* out, const char* path) {
  int w, h, c, x, y;
  unsigned char* data = stbi_load(path, &w, &h, &c, 0);
  if (!data) {
    error_handle(NORMAL_PRIORITY, STBI_LOAD_FAILED, "stbi_load() failed: %s", stbi_failure_reason());
    return false;
  }

  if (!sgl_surface(out, w, h)) {
    stbi_image_free(data);
    return false;
  }

  unsigned char* p = NULL;
  for (x = 0; x < w; ++x) {
    for (y = 0; y < h; ++y) {
      p = data + (x + w * y) * c;
#if !defined(SGL_DISABLE_RGBA)
      out->buf[y * w + x] = RGBA(p[0], p[1], p[2], (c == 4 ? p[3] : 255));
#else
      out->buf[y * w + x] = (c == 4 && !p[3] ? BLIT_CHROMA_KEY : RGB(p[0], p[1], p[2]));
#endif
    }
  }

  stbi_image_free(data);
  return true;
}

bool sgl_save_image(surface_t* in, const char* path, SAVEFORMAT type) {
  if (!in || !path) {
    error_handle(NORMAL_PRIORITY, INVALID_PARAMETERS, "save_image() failed: Invalid parameters");
    return false;
  }

#if !defined(SGL_DISABLE_RGBA)
#define NC 4
#else
#define NC 3
#endif

  unsigned char* data = malloc(in->w * in->h * NC * sizeof(unsigned char));
  if (!data) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "save_image() failed: Out of memory");
    return false;
  }

  unsigned char* p = NULL;
  int i, j, c;
  for (i = 0; i < in->w; ++i) {
    for (j = 0; j < in->h; ++j) {
      p = data + (i + in->w * j) * NC;
      c = in->buf[j * in->w + i];
      p[0] = R(c);
      p[1] = G(c);
      p[2] = B(c);
#if !defined(SGL_DISABLE_RGBA)
      p[3] = A(c);
#endif
    }
  }

  int res = 0;
  switch (type) {
  default:
  case PNG:
    res = stbi_write_png(path, in->w, in->h, NC, data, 0);
    break;
  case TGA:
    res = stbi_write_tga(path, in->w, in->h, NC, data);
    break;
  case BMP:
    res = stbi_write_bmp(path, in->w, in->h, NC, data);
    break;
  case JPG:
    res = stbi_write_jpg(path, in->w, in->h, NC, data, 85);
    break;
  }
#undef NC

  if (!res) {
    error_handle(NORMAL_PRIORITY, STBI_WRITE_FAILED, "save_image() failed: stbi_write() failed");
    return false;
  }
  return true;
}
#endif

#if defined(SGL_ENABLE_GIF)
#include <stdint.h>
#ifndef GIF_MGET
#define GIF_MGET(m,s,a,c) m = (uint8_t*)realloc((c)? 0 : m, (c)? s : 0UL);
#endif
#ifndef GIF_BIGE
#define GIF_BIGE 0
#endif
#ifndef GIF_EXTR
#define GIF_EXTR static
#endif
#define _GIF_SWAP(h) ((GIF_BIGE)? ((uint16_t)(h << 8) | (h >> 8)) : h)

#pragma pack(push, 1)
struct GIF_WHDR {         /** ======== frame writer info: ======== **/
  long xdim, ydim, clrs,  /** global dimensions, palette size      **/
  bkgd, tran,             /** background index, transparent index  **/
  intr, mode,             /** interlace flag, frame blending mode  **/
  frxd, fryd, frxo, fryo, /** current frame dimensions and offset  **/
  time, ifrm, nfrm;       /** delay, frame number, frame count     **/
  uint8_t *bptr;          /** frame pixel indices or metadata      **/
  struct {                /** [==== GIF RGB palette element: ====] **/
    uint8_t R, G, B;      /** [color values - red, green, blue   ] **/
  } *cpal;                /** current palette                      **/
};
#pragma pack(pop)

enum {GIF_NONE = 0, GIF_CURR = 1, GIF_BKGD = 2, GIF_PREV = 3};

static long _GIF_SkipChunk(uint8_t **buff, long size) {
  long skip;
  
  for (skip = 2, ++size, ++(*buff); ((size -= skip) > 0) && (skip > 1);
       *buff += (skip = 1 + **buff));
  return size;
}

static long _GIF_LoadHeader(unsigned gflg, uint8_t **buff, void **rpal,
                            unsigned fflg, long *size, long flen) {
  if (flen && (!(*buff += flen) || ((*size -= flen) <= 0)))
    return -2;  /** v--[ 0x80: "palette is present" flag ]--, **/
  if (flen && (fflg & 0x80)) { /** local palette has priority | **/
    *rpal = *buff; /** [ 3L: 3 uint8_t color channels ]--,  | **/
    *buff += (flen = 2 << (fflg & 7)) * 3L;       /** <--|  | **/
    return ((*size -= flen * 3L) > 0)? flen : -1; /** <--'  | **/
  } /** no local palette found, checking for the global one   | **/
  return (gflg & 0x80)? (2 << (gflg & 7)) : 0;      /** <-----' **/
}

static long _GIF_LoadFrame(uint8_t **buff, long *size,
                           uint8_t *bptr, uint8_t *blen) {
  typedef uint16_t GIF_H;
  const long GIF_HLEN = sizeof(GIF_H), /** to rid the scope of sizeof **/
  GIF_CLEN = 1 << 12;    /** code table length: 4096 items **/
  GIF_H accu, mask; /** bit accumulator / bit mask                    **/
  long  ctbl, iter, /** last code table index / index string iterator **/
  prev, curr, /** codes from the stream: previous / current     **/
  ctsz, ccsz, /** code table bit sizes: min LZW / current       **/
  bseq, bszc; /** counters: block sequence / bit size           **/
  uint32_t *code = (uint32_t*)bptr - GIF_CLEN; /** code table pointer **/
  
  /** preparing initial values **/
  if ((--(*size) <= GIF_HLEN) || !*++(*buff))
    return -4; /** unexpected end of the stream: insufficient size **/
  mask = (GIF_H)((1 << (ccsz = (ctsz = *(*buff - 1)) + 1)) - 1);
  if ((ctsz < 2) || (ctsz > 8))
    return -3; /** min LZW size is out of its nominal [2; 8] bounds **/
  if ((ctbl = (1L << ctsz)) != (mask & _GIF_SWAP(*(GIF_H*)(*buff + 1))))
    return -2; /** initial code is not equal to min LZW size **/
  for (curr = ++ctbl; curr; code[--curr] = 0); /** actual color codes **/
  
  /** getting codes from stream (--size makes up for end-of-stream mark) **/
  for (--(*size), bszc = -ccsz, prev = curr = 0;
       ((*size -= (bseq = *(*buff)++) + 1) >= 0) && bseq; *buff += bseq)
    for (; bseq > 0; bseq -= GIF_HLEN, *buff += GIF_HLEN)
      for (accu = (GIF_H)(_GIF_SWAP(*(GIF_H*)*buff)
                          & ((bseq < GIF_HLEN)? ((1U << (8 * bseq)) - 1U) : ~0U)),
           curr |= accu << (ccsz + bszc), accu = (GIF_H)(accu >> -bszc),
           bszc += 8 * ((bseq < GIF_HLEN)? bseq : GIF_HLEN);
           bszc >= 0; bszc -= ccsz, prev = curr, curr = accu,
           accu = (GIF_H)(accu >> ccsz))
        if (((curr &= mask) & ~1L) == (1L << ctsz)) {
          if (~(ctbl = curr + 1) & 1) /** end-of-data code (ED). **/
          /** -1: no end-of-stream mark after ED; 1: decoded **/
            return (*((*buff += bseq + 1) - 1))? -1 : 1;
          mask = (GIF_H)((1 << (ccsz = ctsz + 1)) - 1);
        } /** ^- table drop code (TD). TD = 1 << ctsz, ED = TD + 1 **/
        else { /** single-pixel (SP) or multi-pixel (MP) code. **/
          if (ctbl < GIF_CLEN) { /** is the code table full? **/
            if ((ctbl == mask) && (ctbl < GIF_CLEN - 1)) {
              mask = (GIF_H)(mask + mask + 1);
              ccsz++; /** yes; extending **/
            } /** prev = TD? => curr < ctbl = prev **/
            code[ctbl] = (uint32_t)prev + (code[prev] & 0xFFF000);
          } /** appending SP / MP decoded pixels to the frame **/
          prev = (long)code[iter = (ctbl > curr)? curr : prev];
          if ((bptr += (prev = (prev >> 12) & 0xFFF)) > blen)
            continue; /** skipping pixels above frame capacity **/
          for (prev++; (iter &= 0xFFF) >> ctsz;
               *bptr-- = (uint8_t)((iter = (long)code[iter]) >> 24));
          (bptr += prev)[-prev] = (uint8_t)iter;
          if (ctbl < GIF_CLEN) { /** appending the code table **/
            if (ctbl == curr)
              *bptr++ = (uint8_t)iter;
            else if (ctbl < curr)
              return -5; /** wrong code in the stream **/
            code[ctbl++] += ((uint32_t)iter << 24) + 0x1000;
          }
        } /** 0: no ED before end-of-stream mark; -4: see above **/
  return (++(*size) >= 0)? 0 : -4; /** ^- N.B.: 0 error is recoverable **/
}

GIF_EXTR long GIF_Load(void *data, long size,
                       void (*gwfr)(void*, struct GIF_WHDR*),
                       void (*eamf)(void*, struct GIF_WHDR*),
                       void *anim, long skip) {
  const long    GIF_BLEN = (1 << 12) * sizeof(uint32_t);
  const uint8_t GIF_EHDM = 0x21, /** extension header mark              **/
  GIF_FHDM = 0x2C, /** frame header mark                  **/
  GIF_EOFM = 0x3B, /** end-of-file mark                   **/
  GIF_EGCM = 0xF9, /** extension: graphics control mark   **/
  GIF_EAMM = 0xFF; /** extension: app metadata mark       **/
#pragma pack(push, 1)
  struct GIF_GHDR {      /** ========== GLOBAL GIF HEADER: ========== **/
    uint8_t head[6];     /** 'GIF87a' / 'GIF89a' header signature     **/
    uint16_t xdim, ydim; /** total image width, total image height    **/
    uint8_t flgs;        /** FLAGS:
                          GlobalPlt    bit 7     1: global palette exists
                          0: local in each frame
                          ClrRes       bit 6-4   bits/channel = ClrRes+1
                          [reserved]   bit 3     0
                          PixelBits    bit 2-0   |Plt| = 2 * 2^PixelBits
                          **/
    uint8_t bkgd, aspr;  /** background color index, aspect ratio     **/
  } *ghdr = (struct GIF_GHDR*)data;
  struct GIF_FHDR {      /** ======= GIF FRAME MASTER HEADER: ======= **/
    uint16_t frxo, fryo; /** offset of this frame in a "full" image   **/
    uint16_t frxd, fryd; /** frame width, frame height                **/
    uint8_t flgs;        /** FLAGS:
                          LocalPlt     bit 7     1: local palette exists
                          0: global is used
                          Interlaced   bit 6     1: interlaced frame
                          0: non-interlaced frame
                          Sorted       bit 5     usually 0
                          [reserved]   bit 4-3   [undefined]
                          PixelBits    bit 2-0   |Plt| = 2 * 2^PixelBits
                          **/
  } *fhdr;
  struct GIF_EGCH {        /** ==== [EXT] GRAPHICS CONTROL HEADER: ==== **/
    uint8_t flgs;        /** FLAGS:
                          [reserved]   bit 7-5   [undefined]
                          BlendMode    bit 4-2   000: not set; static GIF
                          001: leave result as is
                          010: restore background
                          011: restore previous
                          1--: [undefined]
                          UserInput    bit 1     1: show frame till input
                          0: default; ~99% of GIFs
                          TransColor   bit 0     1: got transparent color
                          0: frame is fully opaque
                          **/
    uint16_t time;       /** delay in GIF time units; 1 unit = 10 ms  **/
    uint8_t tran;        /** transparent color index                  **/
  } *egch = 0;
#pragma pack(pop)
  struct GIF_WHDR wtmp, whdr = {0};
  long desc, blen;
  uint8_t *buff;
  
  /** checking if the stream is not empty and has a 'GIF8[79]a' signature,
   the data has sufficient size and frameskip value is non-negative **/
  if (!ghdr || (size <= (long)sizeof(*ghdr)) || (*(buff = ghdr->head) != 71)
      || (buff[1] != 73) || (buff[2] != 70) || (buff[3] != 56) || (skip < 0)
      || ((buff[4] != 55) && (buff[4] != 57)) || (buff[5] != 97) || !gwfr)
    return 0;
  
  buff = (uint8_t*)(ghdr + 1) /** skipping the global header and palette **/
  + _GIF_LoadHeader(ghdr->flgs, 0, 0, 0, 0, 0L) * 3L;
  if ((size -= buff - (uint8_t*)ghdr) <= 0)
    return 0;
  
  whdr.xdim = _GIF_SWAP(ghdr->xdim);
  whdr.ydim = _GIF_SWAP(ghdr->ydim);
  for (whdr.bptr = buff, whdr.bkgd = ghdr->bkgd, blen = --size;
       (blen >= 0) && ((desc = *whdr.bptr++) != GIF_EOFM); /** sic: '>= 0' **/
       blen = _GIF_SkipChunk(&whdr.bptr, blen) - 1) /** count all frames **/
    if (desc == GIF_FHDM) {
      fhdr = (struct GIF_FHDR*)whdr.bptr;
      if (_GIF_LoadHeader(ghdr->flgs, &whdr.bptr, (void**)&whdr.cpal,
                          fhdr->flgs, &blen, sizeof(*fhdr)) <= 0)
        break;
      whdr.frxd = _GIF_SWAP(fhdr->frxd);
      whdr.fryd = _GIF_SWAP(fhdr->fryd);
      whdr.frxo = (whdr.frxd > whdr.frxo)? whdr.frxd : whdr.frxo;
      whdr.fryo = (whdr.fryd > whdr.fryo)? whdr.fryd : whdr.fryo;
      whdr.ifrm++;
    }
  blen = whdr.frxo * whdr.fryo * (long)sizeof(*whdr.bptr);
  GIF_MGET(whdr.bptr, (unsigned long)(blen + GIF_BLEN + 2), anim, 1)
  whdr.nfrm = (desc != GIF_EOFM)? -whdr.ifrm : whdr.ifrm;
  for (whdr.bptr += GIF_BLEN, whdr.ifrm = -1; blen /** load all frames **/
       && (skip < ((whdr.nfrm < 0)? -whdr.nfrm : whdr.nfrm)) && (size >= 0);
       size = (desc != GIF_EOFM)? ((desc != GIF_FHDM) || (skip > whdr.ifrm))?
       _GIF_SkipChunk(&buff, size) - 1 : size - 1 : -1)
    if ((desc = *buff++) == GIF_FHDM) { /** found a frame **/
      whdr.intr = !!((fhdr = (struct GIF_FHDR*)buff)->flgs & 0x40);
      *(void**)&whdr.cpal = (void*)(ghdr + 1); /** interlaced? -^ **/
      whdr.clrs = _GIF_LoadHeader(ghdr->flgs, &buff, (void**)&whdr.cpal,
                                  fhdr->flgs, &size, sizeof(*fhdr));
      if ((skip <= ++whdr.ifrm) && ((whdr.clrs <= 0)
                                    ||  (_GIF_LoadFrame(&buff, &size,
                                                        whdr.bptr, whdr.bptr + blen) < 0)))
        size = -(whdr.ifrm--) - 1; /** failed to load the frame **/
      else if (skip <= whdr.ifrm) {
        whdr.frxd = _GIF_SWAP(fhdr->frxd);
        whdr.fryd = _GIF_SWAP(fhdr->fryd);
        whdr.frxo = _GIF_SWAP(fhdr->frxo);
        whdr.fryo = _GIF_SWAP(fhdr->fryo);
        whdr.time = (egch)? _GIF_SWAP(egch->time) : 0;
        whdr.tran = (egch && (egch->flgs & 0x01))? egch->tran : -1;
        whdr.time = (egch && (egch->flgs & 0x02))? -whdr.time - 1
        : whdr.time;
        whdr.mode = (egch && !(egch->flgs & 0x10))?
        (egch->flgs & 0x0C) >> 2 : GIF_NONE;
        egch = 0;
        wtmp = whdr;
        gwfr(anim, &wtmp); /** passing the frame to the caller **/
      }
    }
    else if (desc == GIF_EHDM) { /** found an extension **/
      if (*buff == GIF_EGCM) /** graphics control ext. **/
        egch = (struct GIF_EGCH*)(buff + 1 + 1);
      else if ((*buff == GIF_EAMM) && eamf) { /** app metadata ext. **/
        wtmp = whdr;
        wtmp.bptr = buff + 1 + 1; /** just passing the raw chunk **/
        eamf(anim, &wtmp);
      }
    }
  whdr.bptr -= GIF_BLEN; /** for excess pixel codes ----v (here & above) **/
  GIF_MGET(whdr.bptr, (unsigned long)(blen + GIF_BLEN + 2), anim, 0)
  return (whdr.nfrm < 0)? (skip - whdr.ifrm - 1) : (whdr.ifrm + 1);
}

#undef _GIF_SWAP

#pragma pack(push, 1)
typedef struct {
  void *data, *pict, *prev;
  unsigned long size, last;
  gif_t* out;
} gif_data_t;
#pragma pack(pop)

void load_gif_frame(void* data, struct GIF_WHDR* whdr) {
  uint32_t *pict, *prev, x, y, yoff, iter, ifin, dsrc, ddst;
  gif_data_t* gif = (gif_data_t*)data;
  
#define BGRA(i) \
((whdr->bptr[i] == whdr->tran)? 0 : \
((uint32_t)(whdr->cpal[whdr->bptr[i]].R << ((GIF_BIGE)? 8 : 16)) \
| (uint32_t)(whdr->cpal[whdr->bptr[i]].G << ((GIF_BIGE)? 16 : 8)) \
| (uint32_t)(whdr->cpal[whdr->bptr[i]].B << ((GIF_BIGE)? 24 : 0)) \
| ((GIF_BIGE)? 0xFF : 0xFF000000)))
  
  if (!whdr->ifrm) {
    gif->out->delay = (int)whdr->time;
    gif->out->w = (int)whdr->xdim;
    gif->out->h = (int)whdr->ydim;
    gif->out->frames = (int)whdr->nfrm;
    gif->out->frame  = 0;
    ddst = (uint32_t)(whdr->xdim * whdr->ydim);
    gif->pict = calloc(sizeof(uint32_t), ddst);
    gif->prev = calloc(sizeof(uint32_t), ddst);
    gif->out->surfaces = calloc(gif->out->frames, sizeof(surface_t));
  }
  
  pict = (uint32_t*)gif->pict;
  ddst = (uint32_t)(whdr->xdim * whdr->fryo + whdr->frxo);
  ifin = (!(iter = (whdr->intr)? 0 : 4))? 4 : 5; /** interlacing support **/
  for (dsrc = (uint32_t)-1; iter < ifin; iter++)
    for (yoff = 16U >> ((iter > 1)? iter : 1), y = (8 >> iter) & 7;
         y < (uint32_t)whdr->fryd; y += yoff)
      for (x = 0; x < (uint32_t)whdr->frxd; x++)
        if (whdr->tran != (long)whdr->bptr[++dsrc])
          pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(dsrc);
  
  int this = (int)whdr->ifrm;
  sgl_surface(&gif->out->surfaces[this], gif->out->w, gif->out->h);
  memcpy(gif->out->surfaces[this].buf, pict, whdr->xdim * whdr->ydim * sizeof(uint32_t));
  
  if ((whdr->mode == GIF_PREV) && !gif->last) {
    whdr->frxd = whdr->xdim;
    whdr->fryd = whdr->ydim;
    whdr->mode = GIF_BKGD;
    ddst = 0;
  }
  else {
    gif->last = (whdr->mode == GIF_PREV)?
    gif->last : (unsigned long)(whdr->ifrm + 1);
    pict = (uint32_t*)((whdr->mode == GIF_PREV)? gif->pict : gif->prev);
    prev = (uint32_t*)((whdr->mode == GIF_PREV)? gif->prev : gif->pict);
    for (x = (uint32_t)(whdr->xdim * whdr->ydim); --x;
         pict[x - 1] = prev[x - 1]);
  }
  
  if (whdr->mode == GIF_BKGD) /** cutting a hole for the next frame **/
    for (whdr->bptr[0] = (uint8_t)((whdr->tran >= 0)?
                                   whdr->tran : whdr->bkgd), y = 0,
         pict = (uint32_t*)gif->pict; y < (uint32_t)whdr->fryd; y++)
      for (x = 0; x < (uint32_t)whdr->frxd; x++)
        pict[(uint32_t)whdr->xdim * y + x + ddst] = BGRA(0);
#undef BGRA
}

bool sgl_gif(gif_t* g, const char* path) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    error_handle(NORMAL_PRIORITY, FILE_OPEN_FAILED, "fopen() failed: %s", path);
    return false;
  }
  
  fseek(fp, 0, SEEK_END);
  size_t length = ftell(fp);
  rewind(fp);
  
  unsigned char* data = (unsigned char*)calloc(length + 1, sizeof(unsigned char));
  if (!data) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "calloc() failed");
    return false;
  }
  fread(data, 1, length, fp);
  fclose(fp);
  
  gif_data_t tmp;
  tmp.data = data;
  tmp.size = length;
  tmp.out  = g;
  if (!GIF_Load(data, length, load_gif_frame, 0, (void*)&tmp, 0L)) {
    error_handle(HIGH_PRIORITY, GIF_LOAD_FAILED, "GIF_Load() failed");
    return false;
  }
  
  free(tmp.data);
  free(tmp.prev);
  free(tmp.pict);
  return true;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

/* helper to write a little-endian 16-bit number portably */
#define write_num(fd, n) write((fd), (uint8_t []) {(n) & 0xFF, (n) >> 8}, 2)

static uint8_t vga[0x30] = {
  0x00, 0x00, 0x00,
  0xAA, 0x00, 0x00,
  0x00, 0xAA, 0x00,
  0xAA, 0x55, 0x00,
  0x00, 0x00, 0xAA,
  0xAA, 0x00, 0xAA,
  0x00, 0xAA, 0xAA,
  0xAA, 0xAA, 0xAA,
  0x55, 0x55, 0x55,
  0xFF, 0x55, 0x55,
  0x55, 0xFF, 0x55,
  0xFF, 0xFF, 0x55,
  0x55, 0x55, 0xFF,
  0xFF, 0x55, 0xFF,
  0x55, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF,
};

typedef struct ge_Node {
  uint16_t key;
  struct ge_Node* children[];
} ge_Node;

typedef struct ge_GIF {
  uint16_t w, h;
  int depth;
  int fd;
  int offset;
  int nframes;
  uint8_t *frame, *back;
  uint32_t partial;
  uint8_t buffer[0xFF];
} ge_GIF;

static ge_Node* new_node(uint16_t key, int degree) {
  ge_Node* node = calloc(1, sizeof(*node) + degree * sizeof(ge_Node *));
  if (node)
    node->key = key;
  return node;
}

static ge_Node* new_trie(int degree, int *nkeys) {
  ge_Node *root = new_node(0, degree);
  /* Create nodes for single pixels. */
  for (*nkeys = 0; *nkeys < degree; (*nkeys)++)
    root->children[*nkeys] = new_node(*nkeys, degree);
  *nkeys += 2; /* skip clear code and stop code */
  return root;
}

static void del_trie(ge_Node* root, int degree) {
  if (!root)
    return;
  for (int i = 0; i < degree; i++)
    del_trie(root->children[i], degree);
  free(root);
}

static void put_loop(ge_GIF* gif, uint16_t loop) {
  write(gif->fd, (uint8_t []) {'!', 0xFF, 0x0B}, 3);
  write(gif->fd, "NETSCAPE2.0", 11);
  write(gif->fd, (uint8_t []) {0x03, 0x01}, 2);
  write_num(gif->fd, loop);
  write(gif->fd, "\0", 1);
}

ge_GIF* ge_new_gif(const char *fname, uint16_t width, uint16_t height, uint8_t* palette, int depth, int loop) {
  int i, r, g, b, v;
  ge_GIF *gif = calloc(1, sizeof(*gif) + 2*width*height);
  if (!gif)
    goto no_gif;
  gif->w = width; gif->h = height;
  gif->depth = depth > 1 ? depth : 2;
  gif->frame = (uint8_t *) &gif[1];
  gif->back = &gif->frame[width*height];
  gif->fd = creat(fname, 0666);
  if (gif->fd == -1)
    goto no_fd;
#ifdef _WIN32
  setmode(gif->fd, O_BINARY);
#endif
  write(gif->fd, "GIF89a", 6);
  write_num(gif->fd, width);
  write_num(gif->fd, height);
  write(gif->fd, (uint8_t []) {0xF0 | (depth-1), 0x00, 0x00}, 3);
  if (palette) {
    write(gif->fd, palette, 3 << depth);
  } else if (depth <= 4) {
    write(gif->fd, vga, 3 << depth);
  } else {
    write(gif->fd, vga, sizeof(vga));
    i = 0x10;
    for (r = 0; r < 6; r++) {
      for (g = 0; g < 6; g++) {
        for (b = 0; b < 6; b++) {
          write(gif->fd, (uint8_t []) {r*51, g*51, b*51}, 3);
          if (++i == 1 << depth)
            goto done_gct;
        }
      }
    }
    for (i = 1; i <= 24; i++) {
      v = i * 0xFF / 25;
      write(gif->fd, (uint8_t []) {v, v, v}, 3);
    }
  }
done_gct:
  if (loop >= 0 && loop <= 0xFFFF)
    put_loop(gif, (uint16_t) loop);
  return gif;
no_fd:
  free(gif);
no_gif:
  return NULL;
}

/* Add packed key to buffer, updating offset and partial.
 *   gif->offset holds position to put next *bit*
 *   gif->partial holds bits to include in next byte */
static void put_key(ge_GIF* gif, uint16_t key, int key_size) {
  int byte_offset, bit_offset, bits_to_write;
  byte_offset = gif->offset / 8;
  bit_offset = gif->offset % 8;
  gif->partial |= ((uint32_t) key) << bit_offset;
  bits_to_write = bit_offset + key_size;
  while (bits_to_write >= 8) {
    gif->buffer[byte_offset++] = gif->partial & 0xFF;
    if (byte_offset == 0xFF) {
      write(gif->fd, "\xFF", 1);
      write(gif->fd, gif->buffer, 0xFF);
      byte_offset = 0;
    }
    gif->partial >>= 8;
    bits_to_write -= 8;
  }
  gif->offset = (gif->offset + key_size) % (0xFF * 8);
}

static void end_key(ge_GIF* gif) {
  int byte_offset;
  byte_offset = gif->offset / 8;
  if (gif->offset % 8)
    gif->buffer[byte_offset++] = gif->partial & 0xFF;
  write(gif->fd, (uint8_t []) {byte_offset}, 1);
  write(gif->fd, gif->buffer, byte_offset);
  write(gif->fd, "\0", 1);
  gif->offset = gif->partial = 0;
}

static void put_image(ge_GIF* gif, uint16_t w, uint16_t h, uint16_t x, uint16_t y) {
  int nkeys, key_size, i, j;
  ge_Node *node, *child, *root;
  int degree = 1 << gif->depth;
  
  write(gif->fd, ",", 1);
  write_num(gif->fd, x);
  write_num(gif->fd, y);
  write_num(gif->fd, w);
  write_num(gif->fd, h);
  write(gif->fd, (uint8_t []) {0x00, gif->depth}, 2);
  root = node = new_trie(degree, &nkeys);
  key_size = gif->depth + 1;
  put_key(gif, degree, key_size); /* clear code */
  for (i = y; i < y+h; i++) {
    for (j = x; j < x+w; j++) {
      uint8_t pixel = gif->frame[i*gif->w+j] & (degree - 1);
      child = node->children[pixel];
      if (child) {
        node = child;
      } else {
        put_key(gif, node->key, key_size);
        if (nkeys < 0x1000) {
          if (nkeys == (1 << key_size))
            key_size++;
          node->children[pixel] = new_node(nkeys++, degree);
        } else {
          put_key(gif, degree, key_size); /* clear code */
          del_trie(root, degree);
          root = node = new_trie(degree, &nkeys);
          key_size = gif->depth + 1;
        }
        node = root->children[pixel];
      }
    }
  }
  put_key(gif, node->key, key_size);
  put_key(gif, degree + 1, key_size); /* stop code */
  end_key(gif);
  del_trie(root, degree);
}

static int get_bbox(ge_GIF* gif, uint16_t* w, uint16_t* h, uint16_t* x, uint16_t* y) {
  int i, j, k;
  int left, right, top, bottom;
  left = gif->w; right = 0;
  top = gif->h; bottom = 0;
  k = 0;
  for (i = 0; i < gif->h; i++) {
    for (j = 0; j < gif->w; j++, k++) {
      if (gif->frame[k] != gif->back[k]) {
        if (j < left)   left    = j;
        if (j > right)  right   = j;
        if (i < top)    top     = i;
        if (i > bottom) bottom  = i;
      }
    }
  }
  if (left != gif->w && top != gif->h) {
    *x = left; *y = top;
    *w = right - left + 1;
    *h = bottom - top + 1;
    return 1;
  } else {
    return 0;
  }
}

static void set_delay(ge_GIF* gif, uint16_t d) {
  write(gif->fd, (uint8_t []) {'!', 0xF9, 0x04, 0x04}, 4);
  write_num(gif->fd, d);
  write(gif->fd, "\0\0", 2);
}

void ge_add_frame(ge_GIF* gif, uint16_t delay) {
  uint16_t w, h, x, y;
  uint8_t *tmp;
  
  if (delay)
    set_delay(gif, delay);
  if (gif->nframes == 0) {
    w = gif->w;
    h = gif->h;
    x = y = 0;
  } else if (!get_bbox(gif, &w, &h, &x, &y)) {
    /* image's not changed; save one pixel just to add delay */
    w = h = 1;
    x = y = 0;
  }
  put_image(gif, w, h, x, y);
  gif->nframes++;
  tmp = gif->back;
  gif->back = gif->frame;
  gif->frame = tmp;
}

void ge_close_gif(ge_GIF* gif) {
  write(gif->fd, ";", 1);
  close(gif->fd);
  free(gif);
}

bool sgl_save_gif(gif_t* _g, const char* path) {
  int nframes = sizeof(_g->surfaces) / sizeof(surface_t);
  int w = _g->w, h = _g->h, i, x, y, j, c, cp;
  uint8_t r, g, b;
  
  for (i = 0; i < nframes; ++i) {
    if (_g->surfaces[i].w != w || _g->surfaces[i].h != h) {
      error_handle(NORMAL_PRIORITY, GIF_SAVE_INVALID_SIZE, "Sizes of surfaces in GIF don't match");
      return false;
    }
  }
  
  int** frames = malloc(sizeof(int*) * nframes);
  uint8_t* pallet = NULL;
  for (i = 0; i < nframes; ++i) {
    frames[i] = malloc(sizeof(int) * w * h);
    for (x = 0; x < w; ++x) {
      for (y = 0; y < h; ++y) {
        c = sgl_pget(&_g->surfaces[i], x, y);
        r = (uint8_t)R(c);
        g = (uint8_t)G(c);
        b = (uint8_t)B(c);
        
        cp = -1;
        for (j = 0; j < stb_sb_count(pallet); j += 3) {
          if (r == pallet[j] && g == pallet[j + 1] && b == pallet[j + 2]) {
            cp = j;
            break;
          }
        }
        
        if (cp >= 0) {
          cp = stb_sb_count(pallet);
          stb_sb_push(pallet, r);
          stb_sb_push(pallet, g);
          stb_sb_push(pallet, b);
        }
        
        frames[i][y * w + x] = cp;
      }
    }
  }
  
  ge_GIF* out = ge_new_gif(path, w, h, pallet, log2(stb_sb_count(pallet)), 0);
  if (!out) {
    error_handle(HIGH_PRIORITY, GIF_SAVE_FAILED, "Failed to create GIF");
    return false;
  }
  
  for (i = 0; i < nframes; ++i) {
    memcpy(out->frame, frames[i], sizeof(uint8_t) * w * h);
    ge_add_frame(out, _g->delay);
  }
  ge_close_gif(out);;
  
  for (i = 0; i < nframes; ++i)
    free(frames[i]);
  free(frames);
  stb_sb_free(pallet);
  return true;
}

void sgl_gif_destroy(gif_t** g) {
  if (!*g)
    return;
  
  gif_t* _g = *g;
  int nframes = sizeof(_g->surfaces) / sizeof(surface_t);
  for (int i = 0; i < nframes; ++i)
    sgl_destroy(&_g->surfaces[i]);
  free(_g->surfaces);
}
#endif

#if !defined(SGL_DISABLE_WINDOW)
static short int keycodes[512];
static surface_t* buffer;
static int mx = 0, my = 0, win_w, win_h;
static bool __closed = false;

#if defined(SGL_ENABLE_JOYSTICKS)
static struct {
  int size;
  joystick_t* head;
} joy_devices;
static int next_device_id = 0;

static void(*joy_removed_callback)(void*, joystick_t*, int) = NULL;
static void(*joy_connect_callback)(void*, joystick_t*, int) = NULL;
static void(*joy_btn_callback)(void*, joystick_t*, int, bool, long) = NULL;
static void(*joy_axis_callback)(void*, joystick_t*, int, float, float, long) = NULL;

void sgl_joystick_callbacks(
    void(*connect_cb)(void*, joystick_t*, int),
    void(*remove_cb)(void*, joystick_t*, int),
    void(*btn_cb)(void*, joystick_t*, int, bool, long),
    void(*axis_cb)(void*, joystick_t*, int, float, float, long)) {
  joy_connect_callback = connect_cb;
  joy_removed_callback = remove_cb;
  joy_btn_callback = btn_cb;
  joy_axis_callback = axis_cb;
}

#if defined(SGL_WINDOWS) || defined(SGL_OSX)
joystick_t* sgl_joystick(int id) {
  if (id < 0 || !joy_devices.head)
    return NULL;

  joystick_t* current = joy_devices.head;
  while (current) {
    if (current->device_id == id)
      return current;
    current = current->next;
  }
  return NULL;
}
#endif

static inline void add_joystick(joystick_t* d) {
  CALL(joy_connect_callback, d, d->device_id);

  joystick_t* current = joy_devices.head;
  if (!current)
    joy_devices.head = d;
  else {
    while (current->next)
      current = current->next;
    current->next = d;
  }
  joy_devices.size++;
}

#if defined(SGL_ENABLE_FORCE_MM)
#define SGL_DISABLE_DIRECTINPUT
#endif
#endif

static void(*__kb_callback)(void*, KEYSYM, KEYMOD, bool) = NULL;
static void(*__mouse_btn_callback)(void*, MOUSEBTN, KEYMOD, bool) = NULL;
static void(*__mouse_move_callback)(void*, int, int, int, int) = NULL;
static void(*__scroll_callback)(void*, KEYMOD, float, float) = NULL;
static void(*__focus_callback)(void*, bool) = NULL;
static void(*__resize_callback)(void*, int, int) = NULL;

void sgl_screen_callbacks(
    void(*kb_cb)(void*, KEYSYM, KEYMOD, bool),
    void(*mouse_btn_cb)(void*, MOUSEBTN, KEYMOD, bool),
    void(*mouse_move_cb)(void*, int, int, int, int),
    void(*scroll_cb)(void*, KEYMOD, float, float),
    void(*focus_cb)(void*, bool),
    void(*resize_cb)(void*, int, int)) {
  __kb_callback = kb_cb;
  __mouse_btn_callback = mouse_btn_cb;
  __mouse_move_callback = mouse_move_cb;
  __scroll_callback = scroll_cb;
  __focus_callback = focus_cb;
  __resize_callback = resize_cb;
}

void sgl_keyboard_callback(void(*kb_cb)(void*, KEYSYM, KEYMOD, bool)) {
  __kb_callback = kb_cb;
}

void sgl_mouse_button_callback(void(*mouse_btn_cb)(void*, MOUSEBTN, KEYMOD, bool)) {
  __mouse_btn_callback = mouse_btn_cb;
}

void sgl_mouse_move_callback(void(*mouse_move_cb)(void*, int, int, int, int)) {
  __mouse_move_callback = mouse_move_cb;
}

void sgl_scroll_callback(void(*scroll_cb)(void*, KEYMOD, float, float)) {
  __scroll_callback = scroll_cb;
}

void sgl_active_callback(void(*active_cb)(void*, bool)) {
  __focus_callback = active_cb;
}

void sgl_resize_callback(void(*resize_cb)(void*, int, int)) {
  __resize_callback = resize_cb;
}

#if defined(SGL_ENABLE_OPENGL)
#if defined(SGL_OSX)
#include <OpenGL/gl3.h>
#endif

#if defined(SGL_LINUX)
#define GLDECL // Empty define
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <dlfcn.h>
#endif

#if defined(SGL_WINDOWS)
#define GLDECL WINAPI

#define GL_ARRAY_BUFFER                   0x8892
#define GL_COMPILE_STATUS                 0x8B81
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_STATIC_DRAW                    0x88E4
#define GL_TEXTURE0                       0x84C0
#define GL_VERTEX_SHADER                  0x8B31
#define GL_INFO_LOG_LENGTH                0x8B84
#define GL_BGRA                           0x80E1
#define GL_UNSIGNED_INT_8_8_8_8_REV       0x8367

typedef char GLchar;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;

#include <gl/GL.h>
#include <gl/GLU.h>

#pragma comment(lib, "opengl32.lib")
#endif

#if defined(SGL_WINDOWS) || defined(SGL_LINUX)
#define GL_LIST \
    /* ret, name, params */ \
    GLE(void,      AttachShader,            GLuint program, GLuint shader) \
    GLE(void,      BindBuffer,              GLenum target, GLuint buffer) \
    GLE(void,      BufferData,              GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage) \
    GLE(void,      BufferSubData,           GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data) \
    GLE(void,      CompileShader,           GLuint shader) \
    GLE(GLuint,    CreateProgram,           void) \
    GLE(GLuint,    CreateShader,            GLenum type) \
    GLE(void,      DeleteBuffers,           GLsizei n, const GLuint *buffers) \
    GLE(void,      EnableVertexAttribArray, GLuint index) \
    GLE(void,      FramebufferTexture2D,    GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) \
    GLE(void,      GenBuffers,              GLsizei n, GLuint *buffers) \
    GLE(GLint,     GetAttribLocation,       GLuint program, const GLchar *name) \
    GLE(void,      GetShaderInfoLog,        GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog) \
    GLE(void,      GetShaderiv,             GLuint shader, GLenum pname, GLint *params) \
    GLE(void,      LinkProgram,             GLuint program) \
    GLE(void,      ShaderSource,            GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length) \
    GLE(void,      UseProgram,              GLuint program) \
    GLE(void,      VertexAttribPointer,     GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer) \
    GLE(GLboolean, IsShader,                GLuint shader) \
    GLE(void,      DeleteProgram,           GLuint program) \
    GLE(void,      DeleteShader,            GLuint shader) \
    GLE(void,      BindVertexArray,         GLuint array) \
    GLE(void,      GenVertexArrays,         GLsizei n, GLuint *arrays) \
    GLE(void,      DeleteVertexArrays,      GLsizei n, const GLuint *arrays) \
    /* end */

#define GLE(ret, name, ...) typedef ret GLDECL name##proc(__VA_ARGS__); extern name##proc * gl##name;
GL_LIST
#undef GLE

#define GLE(ret, name, ...) name##proc * gl##name;
GL_LIST
#undef GLE
#endif

void print_shader_log(GLuint s) {
  if (glIsShader(s)) {
    int log_len = 0, max_len = 0;
    glGetShaderiv(s, GL_INFO_LOG_LENGTH, &max_len);
    char* log = malloc(sizeof(char) * max_len);

    glGetShaderInfoLog(s, max_len, &log_len, log);
    if (log_len > 0)
      error_handle(HIGH_PRIORITY, GL_SHADER_ERROR, "load_shader() failed: %s", log);

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

static GLuint vao, shader, texture;
static int gl3_available = 1;

bool init_gl(int w, int h) {
#if defined(SGL_WINDOWS)
  HINSTANCE dll = LoadLibraryA("opengl32.dll");
  typedef PROC WINAPI wglGetProcAddressproc(LPCSTR lpszProc);
  if (!dll) {
    sgl_release();
    error_handle(LOW_PRIORITY, GL_LOAD_DL_FAILED, "LoadLibraryA() failed: opengl32.dll not found");
    return false;
  }
  wglGetProcAddressproc* wglGetProcAddress = (wglGetProcAddressproc*)GetProcAddress(dll, "wglGetProcAddress");

#define GLE(ret, name, ...) \
  gl##name = (name##proc*)wglGetProcAddress("gl" #name); \
  if (!gl##name) { \
    error_handle(LOW_PRIORITY, GL_GET_PROC_ADDR_FAILED, "wglGetProcAddress() failed: Function gl" #name " couldn't be loaded from opengl32.dll"); \
    gl3_available -= 1; \
  }
  GL_LIST
#undef GLE
#elif defined(SGL_LINUX)
  void* libGL = dlopen("libGL.so", RTLD_LAZY);
  if (!libGL) {
    sgl_release();
    error_handle(LOW_PRIORITY, GL_LOAD_DL_FAILED, "dlopen() failed: libGL.so couldn't be loaded");
    return false;
  }

#define GLE(ret, name, ...) \
  gl##name = (name##proc *) dlsym(libGL, "gl" #name); \
  if (!gl##name) { \
    error_handle(LOW_PRIORITY, GL_GET_PROC_ADDR_FAILED, "dlsym() failed: Function gl" #name " couldn't be loaded from libGL.so"); \
    gl3_available -= 1; \
  }
  GL_LIST
#undef GLE
#endif

  glClearColor(0.f, 0.f, 0.f, 1.f);

#if !defined(SGL_OSX)
  if (gl3_available < 0) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.f, w, 0.f, h, -1.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
  } else {
#endif
    glViewport(0, 0, w, h);

    GLfloat vertices_position[8] = {
      -1., -1.,
       1., -1.,
       1.,  1.,
      -1.,  1.,
    };

    GLfloat texture_coord[8] = {
      .0,  .0,
       1., .0,
       1., 1.,
      .0,  1.,
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
      "  texture_coord_from_vshader = vec2(texture_coord.s, 1.f - texture_coord.t);"
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
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_position), vertices_position);
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
#if !defined(SGL_OSX)
  }
#endif

  glGenTextures(1, &texture);

  return true;
}

void draw_gl() {
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer->w, buffer->h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, (GLvoid*)buffer->buf);

  glClear(GL_COLOR_BUFFER_BIT);

#if !defined(SGL_OSX)
  if (gl3_available < 0) {
    glBegin(GL_QUADS);
      glTexCoord2f(0, 1); glVertex3f(0, 0, 0);
      glTexCoord2f(0, 0); glVertex3f(0, buffer->h, 0);
      glTexCoord2f(1, 0); glVertex3f(buffer->w, buffer->h, 0);
      glTexCoord2f(1, 1); glVertex3f(buffer->w, 0, 0);
    glEnd();
  } else {
#endif
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
#if !defined(SGL_OSX)
  }

  glFlush();
#endif
}

void free_gl() {
  if (texture)
    glDeleteTextures(1, &texture);
  if (!gl3_available) {
    if (shader)
      glDeleteProgram(shader);
    if (vao)
      glDeleteVertexArrays(1, &vao);
  }
}
#endif

#if defined(SGL_OSX)
#include <Cocoa/Cocoa.h>
#if defined(SGL_ENABLE_METAL)
#include <MetalKit/MetalKit.h>
#include <simd/simd.h>

typedef enum AAPLVertexInputIndex {
  AAPLVertexInputIndexVertices     = 0,
  AAPLVertexInputIndexViewportSize = 1,
} AAPLVertexInputIndex;

typedef enum AAPLTextureIndex {
  AAPLTextureIndexBaseColor = 0,
} AAPLTextureIndex;

typedef struct {
  vector_float2 position;
  vector_float2 textureCoordinate;
} AAPLVertex;

static const AAPLVertex quad_vertices[] = {
  {{  1.f,  -1.f  }, { 1.f, 0.f }},
  {{ -1.f,  -1.f  }, { 0.f, 0.f }},
  {{ -1.f,   1.f  }, { 0.f, 1.f }},
  {{  1.f,  -1.f  }, { 1.f, 0.f }},
  {{ -1.f,   1.f  }, { 0.f, 1.f }},
  {{  1.f,   1.f  }, { 1.f, 1.f }},
};

static vector_uint2 mtk_viewport;
static CGFloat scale_f = 1.;
#endif

#if defined(SGL_ENABLE_JOYSTICKS)
#include <IOKit/hid/IOHIDLib.h>
#include <limits.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

typedef struct {
  IOHIDElementCookie cookie;
  CFIndex min, max;
  bool has_null_state, is_hat_switch, is_hat_switch_2nd_axis;
} hid_joystick_axis_t;

typedef struct {
  IOHIDElementCookie cookie;
} hid_joystick_btn_t;

typedef struct {
  IOHIDDeviceRef device;
  hid_joystick_axis_t* axes;
  hid_joystick_btn_t* buttons;
} joystick_private_t;

typedef enum {
  JOY_DEVICE_ATTACHED,
  JOY_DEVICE_REMOVED,
  JOY_BTN_DOWN,
  JOY_BTN_UP,
  JOY_AXIS_MOVED
} JOYEVENTTYPE;

typedef struct {
  joystick_t* device;
  long timestamp;
  unsigned int btn_id;
  bool down;
} joystick_btn_event_t;

typedef struct {
  joystick_t* device;
  long timestamp;
  unsigned int axis_id;
  float value, last_value;
} joystick_axis_event_t;

typedef struct __joy_q_event_t {
  int device_id;
  JOYEVENTTYPE event;
  struct __joy_q_event_t* next;
  void* data;
} joystick_queued_event_t;

typedef struct {
  int size;
  joystick_queued_event_t* head;
} joystick_queue_t;

static IOHIDManagerRef hid = NULL;
static joystick_queue_t input_event_q, device_event_q;

#define GAMEPAD_RUN_LOOP_MODE CFSTR("GamepadRunLoopMode")

static void queue_input_event(int device_id, JOYEVENTTYPE type, void* data) {
  joystick_queued_event_t* e = malloc(sizeof(joystick_queued_event_t));
  e->device_id = device_id;
  e->event = type;
  e->data = data;
  e->next = NULL;
  
  joystick_queued_event_t* current = input_event_q.head;
  if (!current)
    input_event_q.head = e;
  else {
    while (current)
      current = current->next;
    current = e;
  }
  input_event_q.size++;
}

static void queue_axis_event(joystick_t* device, long time, int id, float v, float lv) {
  joystick_axis_event_t* e = malloc(sizeof(joystick_axis_event_t));
  e->device = device;
  e->axis_id = id;
  e->value = v;
  e->last_value = lv;
  e->timestamp = time;
  queue_input_event(device->device_id, JOY_AXIS_MOVED, (void*)e);
}


static void queue_btn_event(joystick_t* device, long time, int id, bool down) {
  joystick_btn_event_t* e = malloc(sizeof(joystick_btn_event_t));
  e->device = device;
  e->btn_id = id;
  e->down = down;
  e->timestamp = time;
  queue_input_event(device->device_id, down ? JOY_BTN_DOWN : JOY_BTN_UP, (void*)e);
}

static void hat_val_xy(CFIndex v, CFIndex r, int* x, int* y) {
  *x = (v == r ? 0 : (v > 0 && v < r / 2 ? 1 : (v > r / 2 ? -1 : 0)));
  *y = (v == r ? 0 : (v > r / 4 * 3 || v < r / 4 ? -1 : (v > r / 4 && v < r / 4 * 3 ? 1 : 0)));
}

static void device_val_changed(void* ctx, IOReturn result, void* sender, IOHIDValueRef value) {
  static mach_timebase_info_data_t tbi;
  if (!tbi.denom)
    mach_timebase_info(&tbi);
  
  joystick_t* device = (joystick_t*)ctx;
  joystick_private_t* private = device->__private;
  IOHIDElementRef element = IOHIDValueGetElement(value);
  IOHIDElementCookie cookie = IOHIDElementGetCookie(element);
  
  for (int a = 0; a < device->n_axes; ++a) {
    if (!private->axes[a].is_hat_switch_2nd_axis &&
         private->axes[a].cookie == cookie) {
      if (IOHIDValueGetLength(value) > 4)
        continue;
      
      CFIndex iv = IOHIDValueGetIntegerValue(value);
      if (private->axes[a].is_hat_switch) {
        if (!private->axes[a].has_null_state)
          iv = (iv < private->axes[a].min ? private->axes[a].max - private->axes[a].min + 1 : iv - 1);
        
        int x, y;
        hat_val_xy(iv, private->axes[a].max - private->axes[a].min + 1, &x, &y);
        if (x != device->axes[a]) {
          queue_axis_event(device, IOHIDValueGetTimeStamp(value) * tbi.numer / tbi.denom * 0.000000001, a, x, device->axes[a]);
          device->axes[a] = x;
        }
        if (y != device->axes[a + 1]) {
          queue_axis_event(device, IOHIDValueGetTimeStamp(value) * tbi.numer / tbi.denom * 0.000000001, a + 1, y, device->axes[a + 1]);
          device->axes[a + 1] = y;
        }
      } else {
        if (iv < private->axes[a].min)
          private->axes[a].min = iv;
        if (iv > private->axes[a].max)
          private->axes[a].max = iv;
        float fv = (iv - private->axes[a].min) / (float) (private->axes[a].max - private->axes[a].min) * 2.0f - 1.0f;
        
        queue_axis_event(device, IOHIDValueGetTimeStamp(value) * tbi.numer / tbi.denom * 0.000000001, a, fv, device->axes[a]);
        device->axes[a] = fv;
      }
      return;
    }
  }
  
  for (int b = 0; b < device->n_buttons; ++b) {
    if (private->buttons[b].cookie == cookie) {
      bool down = IOHIDValueGetIntegerValue(value);
      queue_btn_event(device, IOHIDValueGetTimeStamp(value) * tbi.numer / tbi.denom * 0.000000001, b, down);
      device->buttons[b] = down;
      return;
    }
  }
}

static int IOHIDDeviceGetIntProperty(IOHIDDeviceRef ref, CFStringRef key) {
  CFTypeRef ref_type = IOHIDDeviceGetProperty(ref, key);
  if (!ref_type || CFGetTypeID(ref_type) != CFNumberGetTypeID())
    return 0;
  int value;
  CFNumberGetValue((CFNumberRef)ref_type, kCFNumberSInt32Type, &value);
  return value;
}

#define IOHIDDeviceGetProductID(r) (IOHIDDeviceGetIntProperty(r, CFSTR(kIOHIDProductIDKey)))
#define IOHIDDeviceGetVendorID(r) (IOHIDDeviceGetIntProperty(r, CFSTR(kIOHIDVendorIDKey)))

static void device_added(void* ctx, IOReturn result, void* sender, IOHIDDeviceRef device_ref) {
  joystick_private_t* private = malloc(sizeof(joystick_private_t));
  private->device = device_ref;
  private->axes = NULL;
  private->buttons = NULL;
  
  joystick_t* device = malloc(sizeof(joystick_t));
  device->__private = private;
  device->next = NULL;
  device->device_id = next_device_id++;
  device->product_id = IOHIDDeviceGetProductID(device_ref);
  device->vendor_id = IOHIDDeviceGetVendorID(device_ref);
  device->n_axes = 0;
  device->n_buttons = 0;
  
  char* description;
  CFStringRef product_name = IOHIDDeviceGetProperty(device_ref, CFSTR(kIOHIDProductKey));
  if (!product_name || CFGetTypeID(product_name) != CFStringGetTypeID()) {
    description = malloc(sizeof(char) * 10);
    strcpy(description, "[Unknown]");
  } else {
    CFIndex pn_len;
    CFStringGetBytes(product_name, CFRangeMake(0, CFStringGetLength(product_name)), kCFStringEncodingUTF8, '?', false, NULL, 100, &pn_len);
    description = malloc(sizeof(char) * pn_len + 1);
    CFStringGetBytes(product_name, CFRangeMake(0, CFStringGetLength(product_name)), kCFStringEncodingUTF8, '?', false, (UInt8*)description, pn_len + 1, NULL);
    description[pn_len] = '\x00';
  }
  device->description = description;
  
  CFArrayRef elements = IOHIDDeviceCopyMatchingElements(device_ref, NULL, kIOHIDOptionsTypeNone);
  for (int e = 0; e < CFArrayGetCount(elements); ++e) {
    IOHIDElementRef element = (IOHIDElementRef)CFArrayGetValueAtIndex(elements, e);
    switch(IOHIDElementGetType(element)) {
      case kIOHIDElementTypeInput_Misc:
      case kIOHIDElementTypeInput_Axis:
        private->axes = realloc(private->axes, sizeof(hid_joystick_axis_t) * (device->n_axes + 1));
        private->axes[device->n_axes].cookie = IOHIDElementGetCookie(element);
        private->axes[device->n_axes].min = IOHIDElementGetLogicalMin(element);
        private->axes[device->n_axes].max = IOHIDElementGetLogicalMax(element);
        private->axes[device->n_axes].has_null_state = !!IOHIDElementHasNullState(element);
        private->axes[device->n_axes].is_hat_switch = IOHIDElementGetUsage(element) == kHIDUsage_GD_Hatswitch;
        private->axes[device->n_axes].is_hat_switch_2nd_axis = false;
        device->n_axes++;
        
        if (private->axes[device->n_axes - 1].is_hat_switch) {
          private->axes = realloc(private->axes, sizeof(hid_joystick_axis_t) * (device->n_axes + 1));
          private->axes[device->n_axes].is_hat_switch_2nd_axis = true;
          device->n_axes++;
        }
        break;
      case kIOHIDElementTypeInput_Button:
        private->buttons = realloc(private->buttons, sizeof(hid_joystick_btn_t) * (device->n_buttons + 1));
        private->buttons[device->n_buttons].cookie = IOHIDElementGetCookie(element);
        device->n_buttons++;
        break;
    }
  }
  CFRelease(elements);
  
  device->axes = malloc(sizeof(float) * device->n_axes);
  device->buttons = malloc(sizeof(int) * device->n_buttons);
  
  IOHIDDeviceRegisterInputValueCallback(device_ref, device_val_changed, device);
  
  joystick_queued_event_t* q_event = malloc(sizeof(joystick_queued_event_t));
  q_event->device_id = device->device_id;
  q_event->event = JOY_DEVICE_ATTACHED;
  q_event->data = (void*)device;
  q_event->next = NULL;
  
  joystick_queued_event_t* current = device_event_q.head;
  if (!current)
    device_event_q.head = q_event;
  else {
    while (current->next)
      current = current->next;
    current->next = q_event;
  }
  device_event_q.size++;
}

static void device_removed(void* ctx, IOReturn result, void* sender, IOHIDDeviceRef device_ref) {
  joystick_t* current = joy_devices.head;
  while (current) {
    if (((joystick_private_t*)current->__private)->device == device_ref)
      break;
    current = current->next;
  }
  
  joystick_queued_event_t* q_event = malloc(sizeof(joystick_queued_event_t));
  q_event->device_id = current->device_id;
  q_event->event = JOY_DEVICE_REMOVED;
  q_event->data = (void*)current;
  q_event->next = NULL;

  joystick_queued_event_t* e_current = device_event_q.head;
  if (!e_current)
    device_event_q.head = q_event;
  else {
    while (e_current->next)
      e_current = e_current->next;
    e_current->next = q_event;
  }
  device_event_q.size++;
}

bool sgl_joystick_init(bool scan_too) {
  hid = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
  
  CFStringRef keys[2] = {
    CFSTR(kIOHIDDeviceUsagePageKey),
    CFSTR(kIOHIDDeviceUsageKey)
  };
  
  int value = kHIDPage_GenericDesktop;
  CFNumberRef values[2];
  values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
  value = kHIDUsage_GD_Joystick;
  values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
  
  CFDictionaryRef dictionaries[3];
  dictionaries[0] = CFDictionaryCreate(kCFAllocatorDefault, (const void**)keys, (const void**)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFRelease(values[0]);
  CFRelease(values[1]);
  
  value = kHIDPage_GenericDesktop;
  values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
  value = kHIDUsage_GD_GamePad;
  values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
  dictionaries[1] = CFDictionaryCreate(kCFAllocatorDefault, (const void**)keys, (const void**)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFRelease(values[0]);
  CFRelease(values[1]);
  
  value = kHIDPage_GenericDesktop;
  values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
  value = kHIDUsage_GD_MultiAxisController;
  values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
  dictionaries[2] = CFDictionaryCreate(kCFAllocatorDefault, (const void**)keys, (const void**)values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
  CFRelease(values[0]);
  CFRelease(values[1]);
  
  CFArrayRef array = CFArrayCreate(kCFAllocatorDefault, (const void**)dictionaries, 3, &kCFTypeArrayCallBacks);
  CFRelease(dictionaries[0]);
  CFRelease(dictionaries[1]);
  CFRelease(dictionaries[2]);
  IOHIDManagerSetDeviceMatchingMultiple(hid, array);
  CFRelease(array);
  
  IOHIDManagerRegisterDeviceMatchingCallback(hid, device_added, NULL);
  IOHIDManagerRegisterDeviceRemovalCallback(hid, device_removed, NULL);
  
  IOHIDManagerOpen(hid, kIOHIDOptionsTypeNone);
  IOHIDManagerScheduleWithRunLoop(hid, CFRunLoopGetCurrent(), GAMEPAD_RUN_LOOP_MODE);
  CFRunLoopRunInMode(GAMEPAD_RUN_LOOP_MODE, 0, true);
  
  return (scan_too ? sgl_joystick_scan() : true);
}

static void process_event_queue(joystick_queued_event_t* e) {
  switch (e->event) {
    case JOY_DEVICE_ATTACHED:
      add_joystick((joystick_t*)e->data);
      break;
    case JOY_DEVICE_REMOVED: {
      joystick_t* d = (joystick_t*)e->data;
      CALL(joy_removed_callback, d, d->device_id);
      sgl_joystick_remove(d->device_id);
      break;
    }
    case JOY_BTN_UP:
    case JOY_BTN_DOWN: {
      joystick_btn_event_t* _e = (joystick_btn_event_t*)e->data;
      CALL(joy_btn_callback, _e->device, _e->btn_id, _e->down, _e->timestamp);
      break;
    }
    case JOY_AXIS_MOVED: {
      joystick_axis_event_t* _e = (joystick_axis_event_t*)e->data;
      CALL(joy_axis_callback, _e->device, _e->axis_id, _e->value, _e->last_value, _e->timestamp);
      break;
    }
  }
}

bool sgl_joystick_scan(void) {
  CFRunLoopRunInMode(GAMEPAD_RUN_LOOP_MODE, 0, true);
  joystick_queued_event_t* current = device_event_q.head;
  joystick_queued_event_t* next = current;
  
  while (current) {
    next = current->next;
    process_event_queue(current);
    FREE_SAFE(current);
    current = next;
  }
  device_event_q.head = NULL;
  device_event_q.size = 0;
  
  return true;
}

static inline void release_joystick(joystick_t** d) {
  joystick_t* _d = *d;
  joystick_private_t* _p = (joystick_private_t*)_d->__private;
  
  IOHIDDeviceRegisterInputValueCallback(_p->device, NULL, NULL);
  
  joystick_queued_event_t* current = input_event_q.head;
  joystick_queued_event_t* previous = current;
  while (current) {
    if (current->device_id == _d->device_id) {
      previous->next = current->next;
      if (current == input_event_q.head)
        input_event_q.head = current->next;
      FREE_SAFE(current->data);
      FREE_SAFE(current);
      input_event_q.size--;
    }
  }
  
  current = device_event_q.head;
  previous = current;
  while (current) {
    if (current->device_id == _d->device_id) {
      previous->next = current->next;
      if (current == input_event_q.head)
        device_event_q.head = current->next;
      FREE_SAFE(current->data);
      FREE_SAFE(current);
      device_event_q.size--;
    }
  }
  
  FREE_SAFE(_p->buttons);
  FREE_SAFE(_p->axes);
  FREE_SAFE(_p);
  if (_d->description)
    free((void*)_d->description);
  FREE_SAFE(_d->buttons);
  FREE_SAFE(_d->axes);
  FREE_SAFE(_d);
}

void sgl_joystick_release() {
  IOHIDManagerUnscheduleFromRunLoop(hid, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
  
  joystick_t* current = joy_devices.head;
  joystick_t* next = current;
  while (current) {
    next = current->next;
    release_joystick(&current);
    current = next;
  }
  joy_devices.head = NULL;
  joy_devices.size = 0;
  
  IOHIDManagerClose(hid, 0);
  CFRelease(hid);
  hid = NULL;
}

bool sgl_joystick_remove(int id) {
  joystick_t* current = joy_devices.head;
  joystick_t* previous = current;
  
  while (current) {
    if (current->device_id == id) {
      previous->next = current->next;
      if (current == joy_devices.head)
        joy_devices.head = current->next;
      CALL(joy_removed_callback, current, current->device_id);
      release_joystick(&current);
      return true;
    }
    previous = current;
    current = current->next;
  }
  
  return false;
}

void sgl_joystick_poll() {
  static bool processing_events = false;
  if (processing_events)
    return;
  
  processing_events = true;
  
  CFRunLoopRunInMode(GAMEPAD_RUN_LOOP_MODE, 0, true);
  joystick_queued_event_t* current = input_event_q.head;
  joystick_queued_event_t* next = current;
  while (current) {
    next = current->next;
    process_event_queue(current);
    free(current);
    current = next;
  }
  input_event_q.head = NULL;
  input_event_q.size = 0;
  
  processing_events = false;
}
#endif

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
  return (key >= sizeof(keycodes) / sizeof(keycodes[0]) ?  KB_KEY_UNKNOWN : keycodes[key]);
}

@interface osx_app_t : NSWindow {
  NSView* view;
  @public bool closed;
}
@end

#if defined(SGL_ENABLE_OPENGL)
@interface osx_view_t : NSOpenGLView {
#elif defined(SGL_ENABLE_METAL)
@interface osx_view_t : MTKView {
  id<MTLDevice> _device;
  id<MTLRenderPipelineState> _pipeline;
  id<MTLCommandQueue> _cmd_queue;
  id<MTLLibrary> _library;
  id<MTLTexture> _texture;
  id<MTLBuffer> _vertices;
  NSUInteger _n_vertices;
#else
@interface osx_view_t : NSView {
#endif
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

static osx_app_t* app;
static int border_off = 22;

static NSCursor *__custom_cursor = nil, *__cursor = nil;
static bool cursor_locked = false, cursor_in_win = false;
static CGFloat lmx = 0, lmy = 0, wdx = 0, wdy = 0;

NSPoint cursor_pos_abs() {
  const NSPoint p = [NSEvent mouseLocation];
  return (NSPoint){ p.x, [app screen].frame.size.height - p.y };
}

void sgl_cursor(bool shown, bool locked, CURSORTYPE type) {
  if (!app) {
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "cursor() failed: Called before screen is set up");
    return;
  }
  
  if (shown)
    [NSCursor unhide];
  else
    [NSCursor hide];
  cursor_locked = locked;

  NSCursor* tmp = NULL;
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
    case CURSOR_CUSTOM:
      if (!__custom_cursor) {
        error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "cursor() failed: Custom cursor not loaded");
        return;
      }
  }
  
  if (__cursor && __cursor != __custom_cursor)
    [__cursor release];

  __cursor = (tmp ? tmp : __custom_cursor);
  [__cursor retain];

  if (app && [app contentView])
    [[app contentView] resetCursorRects];
}

static inline NSImage* create_cocoa_image(surface_t* s) {
  NSImage* nsi = [[[NSImage alloc] initWithSize: NSMakeSize(s->w, s->h)] autorelease];
  NSBitmapImageRep* nsbir = nil;

  if (nsi && nsbir)
    [nsi addRepresentation:nsbir];
  return nsi;
}

void sgl_custom_cursor(surface_t* s) {
  NSImage* nsi = create_cocoa_image(s);
  if (!nsi)
    return;

  if (__custom_cursor)
    [__custom_cursor release];

  __custom_cursor = [[NSCursor alloc] initWithImage:nsi
                                            hotSpot:NSMakePoint(0, 0)];
  if (!__custom_cursor) {
    __custom_cursor = nil;
    return;
  }
  [__custom_cursor retain];
}

@implementation osx_view_t
extern surface_t* buffer;

-(id)initWithFrame:(CGRect)r {
#if defined(SGL_ENABLE_OPENGL)
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

    init_gl(r.size.width, r.size.height);
  }
#elif defined(SGL_ENABLE_METAL)
  _device = MTLCreateSystemDefaultDevice();
  self = [super initWithFrame:r device:_device];
  if (self != nil) {
    track = nil;
    [self updateTrackingAreas];

    self.clearColor =  MTLClearColorMake(0., 0., 0., 0.);
    NSScreen *screen = [NSScreen mainScreen];
    scale_f = [screen backingScaleFactor];
    mtk_viewport.x = r.size.width * scale_f;
    mtk_viewport.y = ((r.size.height - border_off) * scale_f) + (4 * scale_f);
    _cmd_queue = [_device newCommandQueue];
    _vertices  = [_device newBufferWithBytes:quad_vertices
                                      length:sizeof(quad_vertices)
                                     options:MTLResourceStorageModeShared];
    _n_vertices = sizeof(quad_vertices) / sizeof(AAPLVertex);

    NSString *library = @""
      "#include <metal_stdlib>\n"
      "#include <simd/simd.h>\n"
      "using namespace metal;"
      "typedef struct {"
      " float4 clipSpacePosition [[position]];"
      " float2 textureCoordinate;"
      "} RasterizerData;"
      "typedef enum AAPLVertexInputIndex {"
      " AAPLVertexInputIndexVertices     = 0,"
      " AAPLVertexInputIndexViewportSize = 1,"
      "} AAPLVertexInputIndex;"
      "typedef enum AAPLTextureIndex {"
      "  AAPLTextureIndexBaseColor = 0,"
      "} AAPLTextureIndex;"
      "typedef struct {"
      "  vector_float2 position;"
      "  vector_float2 textureCoordinate;"
      "} AAPLVertex;"
      "vertex RasterizerData vertexShader(uint vertexID [[ vertex_id ]], constant AAPLVertex *vertexArray [[ buffer(AAPLVertexInputIndexVertices) ]], constant vector_uint2 *viewportSizePointer  [[ buffer(AAPLVertexInputIndexViewportSize) ]]) {"
      " RasterizerData out;"
      " float2 pixelSpacePosition = float2(vertexArray[vertexID].position.x, -vertexArray[vertexID].position.y);"
      " out.clipSpacePosition.xy = pixelSpacePosition;"
      " out.clipSpacePosition.z = .0;"
      " out.clipSpacePosition.w = 1.;"
      " out.textureCoordinate = vertexArray[vertexID].textureCoordinate;"
      " return out;"
      "}"
      "fragment float4 samplingShader(RasterizerData in [[stage_in]], texture2d<half> colorTexture [[ texture(AAPLTextureIndexBaseColor) ]]) {"
      " constexpr sampler textureSampler(mag_filter::nearest, min_filter::linear);"
      " const half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);"
      " return float4(colorSample);"
      "}";

    NSError *err = nil;
    _library = [_device newLibraryWithSource:library
                                     options:nil
                                       error:&err];
    if (err || !_library) {
      sgl_release();
      error_handle(HIGH_PRIORITY, MTK_LIBRARY_ERROR, "[device newLibraryWithSource] failed: %s", [[err localizedDescription] UTF8String]);
      return nil;
    }

    id<MTLFunction> vs = [_library newFunctionWithName:@"vertexShader"];
    id<MTLFunction> fs = [_library newFunctionWithName:@"samplingShader"];

    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"[Texturing Pipeline]";
    pipelineStateDescriptor.vertexFunction = vs;
    pipelineStateDescriptor.fragmentFunction = fs;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = [self colorPixelFormat];

    _pipeline = [_device newRenderPipelineStateWithDescriptor:pipelineStateDescriptor
                                                        error:&err];
    if (err || !_pipeline) {
      sgl_release();
      error_handle(HIGH_PRIORITY, MTK_CREATE_PIPELINE_FAILED, "[device newRenderPipelineStateWithDescriptor] failed: %s", [[err localizedDescription] UTF8String]);
      return nil;
    }
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

  track = [[NSTrackingArea alloc] initWithRect:[self visibleRect]
                                       options:NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingEnabledDuringMouseDrag | NSTrackingCursorUpdate | NSTrackingInVisibleRect | NSTrackingAssumeInside | NSTrackingActiveInActiveApp
                                         owner:self
                                      userInfo:nil];

  [self addTrackingArea:track];
  [super updateTrackingAreas];
}

-(void)resetCursorRects {
  [super resetCursorRects];
  [self addCursorRect:[self visibleRect] cursor:(__custom_cursor ? __custom_cursor : [NSCursor arrowCursor])];
}

-(void)cursorUpdate:(NSEvent*)event {
  if (__cursor)
    [__cursor set];
}

-(BOOL)acceptsFirstResponder {
  return YES;
}

-(BOOL)performKeyEquivalent:(NSEvent*)event {
  return YES;
}

-(void)mouseEntered: (NSEvent*)event {
  cursor_in_win = true;
#pragma TODO(Add mouse entered event cb)
}

-(void)mouseExited: (NSEvent*)event {
  cursor_in_win = false;
#pragma TODO(Add mouse exited event cb)
}

-(void)drawRect:(NSRect)r {
  if (!buffer || !buffer->buf)
    return;

#if defined(SGL_ENABLE_OPENGL)
  [super drawRect: r];
  draw_gl();
  [[self openGLContext] flushBuffer];
#elif defined(SGL_ENABLE_METAL)
  [super drawRect: r];

  MTLTextureDescriptor* td = [[MTLTextureDescriptor alloc] init];
  td.pixelFormat = MTLPixelFormatBGRA8Unorm;
  td.width = buffer->w;
  td.height = buffer->h;

  _texture = [_device newTextureWithDescriptor:td];
  [_texture replaceRegion:(MTLRegion){{ 0, 0, 0 }, { buffer->w, buffer->h, 1 }}
              mipmapLevel:0
                withBytes:buffer->buf
              bytesPerRow:buffer->w * 4];

  id <MTLCommandBuffer> cmd_buf = [_cmd_queue commandBuffer];
  cmd_buf.label = @"[Command Buffer]";
  MTLRenderPassDescriptor* rpd = [self currentRenderPassDescriptor];
  if (rpd) {
    id<MTLRenderCommandEncoder> re = [cmd_buf renderCommandEncoderWithDescriptor:rpd];
    re.label = @"[Render Encoder]";

    [re setViewport:(MTLViewport){ .0, .0, mtk_viewport.x, mtk_viewport.y, -1., 1. }];
    [re setRenderPipelineState:_pipeline];
    [re setVertexBuffer:_vertices
                 offset:0
                atIndex:AAPLVertexInputIndexVertices];
    [re setVertexBytes:&mtk_viewport
                length:sizeof(mtk_viewport)
               atIndex:AAPLVertexInputIndexViewportSize];
    [re setFragmentTexture:_texture
                   atIndex:AAPLTextureIndexBaseColor];
    [re drawPrimitives:MTLPrimitiveTypeTriangle
           vertexStart:0
           vertexCount:_n_vertices];
    [re endEncoding];

    [cmd_buf presentDrawable:[self currentDrawable]];
  }

  [_texture release];
  [td release];
  [cmd_buf commit];
#else
  CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];

  CGColorSpaceRef s = CGColorSpaceCreateDeviceRGB();
  CGDataProviderRef p = CGDataProviderCreateWithData(NULL, buffer->buf, buffer->w * buffer->h * 3, NULL);
  CGImageRef img = CGImageCreate(buffer->w, buffer->h, 8, 32, buffer->w * 4, s, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, p, NULL, 0, kCGRenderingIntentDefault);

  CGColorSpaceRelease(s);
  CGDataProviderRelease(p);

  CGContextDrawImage(ctx, CGRectMake(0, 0, win_w, win_h), img);

  CGImageRelease(img);
#endif
}

-(void)dealloc {
#if defined(SGL_ENABLE_OPENGL)
  free_gl();
#elif defined(SGL_ENABLE_METAL)
  [_device release];
  [_pipeline release];
  [_cmd_queue release];
  [_library release];
  [_vertices release];
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
                                                 name:NSWindowDidBecomeKeyNotification
                                               object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_changed:)
                                                 name:NSWindowDidResignMainNotification
                                               object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_close)
                                                 name:NSWindowWillCloseNotification
                                               object:self];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(win_resize:)
                                                 name:NSWindowDidResizeNotification
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
  CALL(__focus_callback, false);
}

-(void)win_close {
  closed = true;
}

-(void)win_resize:(NSNotification *)n {
  CGSize size = [app contentRectForFrameRect:[app frame]].size;
  win_w = size.width;
  win_h = size.height - border_off;
#if defined(SGL_ENABLE_OPENGL)
  glViewport(0, 0, win_w, win_h);
#elif defined(SGL_ENABLE_METAL)
  mtk_viewport.x = win_w * scale_f;
  mtk_viewport.y = (win_h * scale_f) + (4 * scale_f);
#endif
  CALL(__resize_callback, win_w, win_h);
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

-(void)becomeKeyWindow {
  CALL(__focus_callback, true);
}

-(NSRect)contentRectForFrameRect:(NSRect)f {
  f.origin = NSZeroPoint;
  return NSInsetRect(f, 0, 0);
}

+(NSRect)frameRectForContentRect:(NSRect)r styleMask:(NSWindowStyleMask)s {
  return NSInsetRect(r, 0, 0);
}
@end

bool sgl_screen(const char* t, surface_t* s, int w, int h, short flags) {
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

  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  NSWindowStyleMask _flags = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
  if (flags & FULLSCREEN)
    flags |= (BORDERLESS | RESIZABLE | FULLSCREEN_DESKTOP);
  _flags |= (flags & RESIZABLE ? NSWindowStyleMaskResizable : 0);
  if (flags & BORDERLESS) {
    _flags |= NSWindowStyleMaskFullSizeContentView;
    border_off = 0;
  }
  if (flags & FULLSCREEN_DESKTOP) {
    NSRect f = [[NSScreen mainScreen] frame];
    w = win_w = f.size.width;
    h = win_h = f.size.height - border_off;
  } else {
    win_w = w;
    win_h = h;
  }

  if (s)
    if (!sgl_surface(s, w, h))
      return false;

  app = [[osx_app_t alloc] initWithContentRect:NSMakeRect(0, 0, w, h + border_off)
                                     styleMask:_flags
                                       backing:NSBackingStoreBuffered
                                         defer:NO];
  if (!app) {
    sgl_release();
    error_handle(HIGH_PRIORITY, OSX_WINDOW_CREATION_FAILED, "[osx_app_t initWithContentRect] failed");
    return false;
  }
  
  if (flags & ALWAYS_ON_TOP)
    [app setLevel:NSFloatingWindowLevel];

  id app_del = [AppDelegate alloc];
  if (!app_del) {
    sgl_release();
    error_handle(HIGH_PRIORITY, OSX_APPDEL_CREATION_FAILED, "[AppDelegate alloc] failed");
    [NSApp terminate:nil];
  }

  if (flags & FULLSCREEN) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    [app toggleFullScreen:nil];
    [[NSApplication sharedApplication] setPresentationOptions:NSApplicationPresentationFullScreen];
#else
    error_handle(LOW_PRIORITY, OSX_FULLSCREEN_FAILED, "screen() failed: Fullscreen flag is only supported on OSX v10.7 and above");
#endif
  }

  [app setDelegate:app_del];
  [app setAcceptsMouseMovedEvents:YES];
  [app setRestorable:NO];
  [app setTitle:(t ? [NSString stringWithUTF8String:t] : [[NSProcessInfo processInfo] processName])];
  [app setReleasedWhenClosed:NO];
  [app performSelectorOnMainThread:@selector(makeKeyAndOrderFront:) withObject:nil waitUntilDone:YES];
  [app center];

  if (!border_off && flags & ~FULLSCREEN) {
    [app setTitle:@""];
    [app setTitlebarAppearsTransparent:YES];
    [[app standardWindowButton:NSWindowZoomButton] setHidden:YES];
    [[app standardWindowButton:NSWindowCloseButton] setHidden:YES];
    [[app standardWindowButton:NSWindowMiniaturizeButton] setHidden:YES];
  }

  NSPoint mp = [NSEvent mouseLocation];
  lmx = mx = mp.x;
  lmy = my = mp.y;

  [NSApp activateIgnoringOtherApps:YES];
  [pool drain];

  return true;
}

bool sgl_closed() {
  return app->closed;
}

void sgl_poll(void) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSEvent* e = nil;
  while ((e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                 untilDate:[NSDate distantPast]
                                    inMode:NSDefaultRunLoopMode
                                   dequeue:YES])) {
    switch ([e type]) {
      case NSEventTypeKeyUp:
      case NSEventTypeKeyDown:
        CALL(__kb_callback, translate_key([e keyCode]), translate_mod([e modifierFlags]), ([e type] == NSEventTypeKeyDown));
        break;
      case NSEventTypeLeftMouseUp:
      case NSEventTypeRightMouseUp:
      case NSEventTypeOtherMouseUp:
        CALL(__mouse_btn_callback, (MOUSEBTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), false);
        break;
      case NSEventTypeLeftMouseDown:
      case NSEventTypeRightMouseDown:
      case NSEventTypeOtherMouseDown:
        CALL(__mouse_btn_callback, (MOUSEBTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
        break;
      case NSEventTypeScrollWheel:
        CALL(__scroll_callback, translate_mod([e modifierFlags]), [e deltaX], [e deltaY]);
        break;
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged:
        if (cursor_in_win) {
          CALL(__mouse_btn_callback, (MOUSEBTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
          CALL(__mouse_move_callback, [e locationInWindow].x, [app frame].size.height - border_off - [e locationInWindow].y, 0, 0);
        }
        break;
      case NSEventTypeMouseMoved:
        if (cursor_in_win)
          CALL(__mouse_move_callback, [e locationInWindow].x, [app frame].size.height - border_off - [e locationInWindow].y, 0, 0);
        break;
    }
    [NSApp sendEvent:e];
  }
  [pool release];
}

void sgl_flush(surface_t* s) {
  if (s && s->buf)
    buffer = s;
  [[app contentView] setNeedsDisplay:YES];
}

void sgl_release() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  if (app)
    [app close];
  if (__cursor)
    [__cursor release];
  if (__custom_cursor)
    [__custom_cursor release];
  [pool drain];
}
#elif defined(SGL_WINDOWS)
static WNDCLASS wnd;
static HWND hwnd;
static HDC hdc = 0;
#if defined(SGL_ENABLE_OPENGL)
static PIXELFORMATDESCRIPTOR pfd;
static HGLRC hrc;
static PAINTSTRUCT ps;
#elif defined(SGL_ENABLE_DX9)
#define COBJMACROS 1
#include <d3d9.h>
#pragma comment (lib, "d3d9.lib")

static LPDIRECT3D9 d3d;
static LPDIRECT3DDEVICE9 d3ddev;

#pragma message("WARNING: DirectX implementation not ready")
#else
static BITMAPINFO* bmpinfo;
#endif
static int adjusted_win_w, adjusted_win_h;
static BOOL ifuckinghatethewin32api = FALSE; // Should always be true because I do
static BOOL is_focused = TRUE;
static HCURSOR __cursor = NULL, ____custom_cursor = NULL;
static BOOL cursor_locked = FALSE;
static RECT rc = { 0 };
static long adjust_flags = WS_POPUP | WS_SYSMENU | WS_CAPTION;

#if defined(SGL_ENABLE_JOYSTICKS)
#if defined(SGL_DISABLE_DIRECTINPUT)
#include <regstr.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

typedef struct {
  UINT joystick_id;
  JOYINFOEX last_state;
  int xAxisIndex;
  int yAxisIndex;
  int zAxisIndex;
  int rAxisIndex;
  int uAxisIndex;
  int vAxisIndex;
  int povXAxisIndex;
  int povYAxisIndex;
  UINT(*axis_ranges)[2];
} joystick_private_t;

#define REG_STRING_MAX 256
#else
#if !defined(STRICT)
#define STRICT
#endif
#define INITGUID
#define DIRECTINPUT_VERSION 0x0800
#define _CRT_SECURE_NO_DEPRECATE
#if defined(_MSC_VER)
#undef UNICODE
#else
#define __in
#define __out
#define __reserved
#endif
#if !defined(_WIN32_DCOM)
#define _WIN32_DCOM
#endif
#if !defined(COBJMACROS)
#define COBJMACROS 1
#endif

#include <dinput.h>
#include <dinputd.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#define INPUT_QUEUE_SIZE 32

#if !defined(SGL_DISABLE_XINPUT)
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN7)
#include <XInput.h>
#pragma comment(lib, "xinput.lib")
#else
#include <XInput.h>
#pragma comment(lib, "xinput9_1_0.lib")
#endif

#define XINPUT_GAMEPAD_GUIDE 0x400

typedef struct {
  WORD wButtons;
  BYTE bLeftTrigger;
  BYTE bRightTrigger;
  SHORT sThumbLX;
  SHORT sThumbLY;
  SHORT sThumbRX;
  SHORT sThumbRY;
  DWORD dwPaddingReserved;
} XINPUT_GAMEPAD_EX;

typedef struct {
  DWORD dwPacketNumber;
  XINPUT_GAMEPAD_EX Gamepad;
} XINPUT_STATE_EX;

#define MAX_XINPUT_DEVICES 4
static joystick_t* xinput_devices[MAX_XINPUT_DEVICES];
static const char* xinput_device_names[MAX_XINPUT_DEVICES] = {
  "XInput Controller 1",
  "XInput Controller 2",
  "XInput Controller 3",
  "XInput Controller 4"
};
static DWORD(WINAPI* XInputGetStateEx_proc)(DWORD dwUserIndex, XINPUT_STATE_EX* pState);
static DWORD(WINAPI* XInputGetState_proc)(DWORD dwUserIndex, XINPUT_STATE* pState);
static DWORD(WINAPI* XInputGetCapabilities_proc)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pCapabilities);
#endif

typedef struct {
  DWORD offset;
  BOOL is_pov;
  BOOL is_pov_2nd_axis;
} di_axis_info_t;

typedef struct {
  BOOL is_xinput;

  // DInput only
  GUID guid;
  IDirectInputDevice8* di8dev;
  BOOL buffered;
  unsigned int slider_c, pov_c;
  di_axis_info_t* axis_info;
  DWORD* button_offsets;

  // XInput only
  unsigned int player_index;
} joystick_private_t;

static LPDIRECTINPUT8 did;

#if !defined(SGL_DISABLE_XINPUT)
static bool xinput_available = true;

DEFINE_GUID(IID_ValveStreamingGamepad, MAKELONG(0x28DE, 0x11FF), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);
DEFINE_GUID(IID_X360WiredGamepad, MAKELONG(0x045E, 0x02A1), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);
DEFINE_GUID(IID_X360WirelessGamepad, MAKELONG(0x045E, 0x028E), 0x0000, 0x0000, 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44);

static PRAWINPUTDEVICELIST raw_dev_list = NULL;
static UINT raw_dev_list_c = 0;

static bool is_xinput_device(const GUID* pGuidProductFromDirectInput) {
  static const GUID* xinput_product_ids[] = {
    &IID_ValveStreamingGamepad,
    &IID_X360WiredGamepad,   // Microsoft's wired X360 controller for Windows
    &IID_X360WirelessGamepad // Microsoft's wireless X360 controller for Windows
  };

  for (int i = 0; i < sizeof(xinput_product_ids) / sizeof(xinput_product_ids[0]); ++i)
    if (!memcmp(pGuidProductFromDirectInput, xinput_product_ids[i], sizeof(GUID)))
      return true;

  if (!raw_dev_list) {
    if ((GetRawInputDeviceList(NULL, &raw_dev_list_c, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) || raw_dev_list_c == 0) {
      return false;
    }

    raw_dev_list = malloc(sizeof(RAWINPUTDEVICELIST) * raw_dev_list_c);
    if (GetRawInputDeviceList(raw_dev_list, &raw_dev_list_c, sizeof(RAWINPUTDEVICELIST)) == (UINT)-1) {
      free(raw_dev_list);
      raw_dev_list = NULL;
      return false;
    }
  }

  for (int i = 0; i < raw_dev_list_c; ++i) {
    RID_DEVICE_INFO rdi;
    char devName[128];
    UINT rdiSize = sizeof(rdi);
    UINT nameSize = sizeof(devName);
    rdi.cbSize = sizeof(rdi);
    if (raw_dev_list[i].dwType == RIM_TYPEHID &&
      GetRawInputDeviceInfoA(raw_dev_list[i].hDevice, RIDI_DEVICEINFO, &rdi, &rdiSize) != (UINT)-1 &&
      MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId) == (LONG)pGuidProductFromDirectInput->Data1 &&
      GetRawInputDeviceInfoA(raw_dev_list[i].hDevice, RIDI_DEVICENAME, devName, &nameSize) != (UINT)-1 &&
      strstr(devName, "IG_"))
      return true;
  }

  return false;
}
#endif

#if defined(_MSC_VER)
#if !defined(DIDFT_OPTIONAL)
#define DIDFT_OPTIONAL      0x80000000
#endif

DIOBJECTDATAFORMAT dfDIJoystick2[] = {
  { &GUID_XAxis, DIJOFS_X, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_YAxis, DIJOFS_Y, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_ZAxis, DIJOFS_Z, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RxAxis, DIJOFS_RX, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RyAxis, DIJOFS_RY, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RzAxis, DIJOFS_RZ, DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, DIJOFS_SLIDER(0), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, DIJOFS_SLIDER(1), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_POV, DIJOFS_POV(0), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
  { &GUID_POV, DIJOFS_POV(1), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
  { &GUID_POV, DIJOFS_POV(2), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
  { &GUID_POV, DIJOFS_POV(3), DIDFT_OPTIONAL | DIDFT_POV | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(0), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(1), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(2), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(3), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(4), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(5), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(6), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(7), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(8), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(9), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(10), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(11), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(12), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(13), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(14), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(15), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(16), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(17), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(18), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(19), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(20), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(21), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(22), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(23), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(24), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(25), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(26), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(27), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(28), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(29), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(30), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(31), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(32), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(33), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(34), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(35), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(36), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(37), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(38), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(39), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(40), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(41), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(42), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(43), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(44), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(45), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(46), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(47), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(48), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(49), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(50), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(51), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(52), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(53), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(54), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(55), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(56), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(57), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(58), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(59), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(60), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(61), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(62), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(63), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(64), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(65), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(66), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(67), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(68), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(69), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(70), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(71), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(72), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(73), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(74), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(75), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(76), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(77), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(78), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(79), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(80), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(81), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(82), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(83), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(84), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(85), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(86), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(87), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(88), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(89), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(90), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(91), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(92), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(93), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(94), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(95), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(96), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(97), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(98), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(99), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(100), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(101), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(102), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(103), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(104), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(105), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(106), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(107), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(108), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(109), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(110), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(111), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(112), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(113), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(114), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(115), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(116), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(117), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(118), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(119), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(120), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(121), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(122), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(123), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(124), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(125), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(126), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { NULL, DIJOFS_BUTTON(127), DIDFT_OPTIONAL | DIDFT_BUTTON | DIDFT_ANYINSTANCE, 0 },
  { &GUID_XAxis, FIELD_OFFSET(DIJOYSTATE2, lVX), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_YAxis, FIELD_OFFSET(DIJOYSTATE2, lVY), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_ZAxis, FIELD_OFFSET(DIJOYSTATE2, lVZ), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lVRx), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lVRy), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lVRz), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglVSlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglVSlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_XAxis, FIELD_OFFSET(DIJOYSTATE2, lAX), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_YAxis, FIELD_OFFSET(DIJOYSTATE2, lAY), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_ZAxis, FIELD_OFFSET(DIJOYSTATE2, lAZ), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lARx), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lARy), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lARz), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglASlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglASlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_XAxis, FIELD_OFFSET(DIJOYSTATE2, lFX), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_YAxis, FIELD_OFFSET(DIJOYSTATE2, lFY), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_ZAxis, FIELD_OFFSET(DIJOYSTATE2, lFZ), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RxAxis, FIELD_OFFSET(DIJOYSTATE2, lFRx), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RyAxis, FIELD_OFFSET(DIJOYSTATE2, lFRy), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_RzAxis, FIELD_OFFSET(DIJOYSTATE2, lFRz), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglFSlider[0]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
  { &GUID_Slider, FIELD_OFFSET(DIJOYSTATE2, rglFSlider[1]), DIDFT_OPTIONAL | DIDFT_AXIS | DIDFT_ANYINSTANCE, 0 },
};

const DIDATAFORMAT c_dfDIJoystick2 = {
  sizeof(DIDATAFORMAT),
  sizeof(DIOBJECTDATAFORMAT),
  DIDF_ABSAXIS,
  sizeof(DIJOYSTATE2),
  sizeof(dfDIJoystick2) / sizeof(dfDIJoystick2[0]),
  dfDIJoystick2
};
#endif

static BOOL CALLBACK count_axes_cb(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
  joystick_t* device = (joystick_t*)context;
  device->n_axes++;
  if (instance->dwType & DIDFT_POV)
    device->n_axes++;
  return DIENUM_CONTINUE;
}

static BOOL CALLBACK count_buttons_cb(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
  joystick_t* device = (joystick_t*)context;
  device->n_buttons++;
  return DIENUM_CONTINUE;
}

#define AXIS_MIN -32768
#define AXIS_MAX 32767

static BOOL CALLBACK enum_axes_cb(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
  joystick_t* device = (joystick_t*)context;
  joystick_private_t* private = (joystick_private_t*)device->__private;

  DWORD offset;
  device->n_axes++;
  if (instance->dwType & DIDFT_POV) {
    offset = DIJOFS_POV(private->pov_c);
    private->axis_info[device->n_axes - 1].offset = offset;
    private->axis_info[device->n_axes - 1].is_pov = TRUE;
    device->n_axes++;
    private->axis_info[device->n_axes - 1].offset = offset;
    private->axis_info[device->n_axes - 1].is_pov = TRUE;
    private->pov_c++;
  }
  else {
    if (!memcmp(&instance->guidType, &GUID_XAxis, sizeof(instance->guidType)))
      offset = DIJOFS_X;
    else if (!memcmp(&instance->guidType, &GUID_YAxis, sizeof(instance->guidType)))
      offset = DIJOFS_Y;
    else if (!memcmp(&instance->guidType, &GUID_ZAxis, sizeof(instance->guidType)))
      offset = DIJOFS_Z;
    else if (!memcmp(&instance->guidType, &GUID_RxAxis, sizeof(instance->guidType)))
      offset = DIJOFS_RX;
    else if (!memcmp(&instance->guidType, &GUID_RyAxis, sizeof(instance->guidType)))
      offset = DIJOFS_RY;
    else if (!memcmp(&instance->guidType, &GUID_RzAxis, sizeof(instance->guidType)))
      offset = DIJOFS_RZ;
    else if (!memcmp(&instance->guidType, &GUID_Slider, sizeof(instance->guidType)))
      offset = DIJOFS_SLIDER(private->slider_c++);
    else
      offset = -1;

    private->axis_info[device->n_axes - 1].offset = offset;
    private->axis_info[device->n_axes - 1].is_pov = FALSE;

    DIPROPRANGE range;
    range.diph.dwSize = sizeof(range);
    range.diph.dwHeaderSize = sizeof(range.diph);
    range.diph.dwObj = instance->dwType;
    range.diph.dwHow = DIPH_BYID;
    range.lMin = AXIS_MIN;
    range.lMax = AXIS_MAX;

    if (IDirectInputDevice8_SetProperty(private->di8dev, DIPROP_RANGE, &range.diph) != DI_OK)
      error_handle(LOW_PRIORITY, JOY_DI_SETPROP_FAILED, "IDirectInputDevice8_SetProperty() failed: %s", GetLastError());

    DIPROPDWORD dead_zone;
    dead_zone.diph.dwSize = sizeof(dead_zone);
    dead_zone.diph.dwHeaderSize = sizeof(dead_zone.diph);
    dead_zone.diph.dwObj = instance->dwType;
    dead_zone.diph.dwHow = DIPH_BYID;
    dead_zone.dwData = 0;

    if (IDirectInputDevice8_SetProperty(private->di8dev, DIPROP_DEADZONE, &dead_zone.diph) != DI_OK)
      error_handle(LOW_PRIORITY, JOY_DI_SETPROP_FAILED, "IDirectInputDevice8_SetProperty() failed: %s", GetLastError());
  }
  return DIENUM_CONTINUE;
}

static BOOL CALLBACK enum_buttons_cb(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context) {
  joystick_t* device = (joystick_t*)context;
  joystick_private_t* private = (joystick_private_t*)device->__private;
  private->button_offsets[device->n_buttons] = DIJOFS_BUTTON(device->n_buttons);
  device->n_buttons++;
  return DIENUM_CONTINUE;
}

static BOOL CALLBACK enum_devices_cb(const DIDEVICEINSTANCE* instance, LPVOID context) {
  joystick_t* current = joy_devices.head;
  while (current) {
    if (!memcmp(&((joystick_private_t*)current->__private)->guid, &instance->guidInstance, sizeof(GUID)))
      return DIENUM_CONTINUE;
    current = current->next;
  }

#if !defined(SGL_DISABLE_XINPUT)
  if (xinput_available && is_xinput_device(&instance->guidProduct))
    return DIENUM_CONTINUE;
#endif

  IDirectInputDevice* didev;
  IDirectInputDevice8* di8dev;

  if (IDirectInput8_CreateDevice(did, &instance->guidInstance, &didev, NULL) != DI_OK) {
    error_handle(LOW_PRIORITY, JOY_DI_CREATE_DEVICE_FAILED, "IDirectInput8_CreateDevice() failed: %s", GetLastError());
    return DIENUM_CONTINUE;
  }
  if (IDirectInputDevice8_QueryInterface(didev, &IID_IDirectInputDevice8, (LPVOID *)&di8dev) != DI_OK) {
    error_handle(LOW_PRIORITY, JOY_DI_SETPROP_FAILED, "IDirectInputDevice8_QueryInterface() failed: %s", GetLastError());
    return DIENUM_CONTINUE;
  }
  IDirectInputDevice8_Release(didev);

  if (IDirectInputDevice8_SetCooperativeLevel(di8dev, GetActiveWindow(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
    error_handle(LOW_PRIORITY, JOY_DI_SETPROP_FAILED, "IDirectInputDevice8_SetCooperativeLevel() failed: %s", GetLastError());
    return DIENUM_CONTINUE;
  }
  if (IDirectInputDevice8_SetDataFormat(di8dev, &c_dfDIJoystick2) != DI_OK) {
    error_handle(LOW_PRIORITY, JOY_DI_SETPROP_FAILED, "IDirectInputDevice8_SetDataFormat() failed: %s", GetLastError());
    return DIENUM_CONTINUE;
  }

  BOOL buffered = TRUE;
  DIPROPDWORD bufsize_prop;
  bufsize_prop.diph.dwSize = sizeof(DIPROPDWORD);
  bufsize_prop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
  bufsize_prop.diph.dwObj = 0;
  bufsize_prop.diph.dwHow = DIPH_DEVICE;
  bufsize_prop.dwData = INPUT_QUEUE_SIZE;
  HRESULT hr = IDirectInputDevice8_SetProperty(di8dev, DIPROP_BUFFERSIZE, &bufsize_prop.diph);
  if (hr == DI_POLLEDDEVICE)
    buffered = FALSE;
  else if (hr != DI_OK) {
    error_handle(LOW_PRIORITY, JOY_DI_SETPROP_FAILED, "IDirectInputDevice8_SetProperty() failed: %s", GetLastError());
    return DIENUM_CONTINUE;
  }

  joystick_private_t* private = malloc(sizeof(joystick_private_t));
  private->guid = instance->guidInstance;
  private->is_xinput = FALSE;
  private->di8dev = di8dev;
  private->buffered = buffered;
  private->slider_c = 0;
  private->pov_c = 0;
  private->button_offsets = NULL;

  joystick_t* device = malloc(sizeof(joystick_t));
  device->__private = (void*)private;
  device->next = NULL;
  device->device_id = next_device_id++;
  device->description = strdup(instance->tszProductName);
  device->vendor_id = instance->guidProduct.Data1 & 0xFFFF;
  device->product_id = instance->guidProduct.Data1 >> 16 & 0xFFFF;
  device->n_axes = 0;
  IDirectInputDevice_EnumObjects(di8dev, count_axes_cb, (void*)device, DIDFT_AXIS | DIDFT_POV);
  device->axes = malloc(sizeof(float) * device->n_axes);
  private->axis_info = malloc(sizeof(di_axis_info_t) * device->n_axes);
  device->n_buttons = 0;
  IDirectInputDevice_EnumObjects(di8dev, count_buttons_cb, (void*)device, DIDFT_BUTTON);
  device->buttons = malloc(sizeof(int) * device->n_buttons);
  private->button_offsets = malloc(sizeof(DWORD) * device->n_buttons);
  device->n_axes = 0;
  IDirectInputDevice_EnumObjects(di8dev, enum_axes_cb, (void*)device, DIDFT_AXIS | DIDFT_POV);
  device->n_buttons = 0;
  IDirectInputDevice_EnumObjects(di8dev, enum_buttons_cb, (void*)device, DIDFT_BUTTON);

  add_joystick(device);

  return DIENUM_CONTINUE;
}
#endif

bool sgl_joystick_init(bool scan_too) {
#if !defined(SGL_DISABLE_DIRECTINPUT)
#if !defined(SGL_DISABLE_XINPUT)
  HMODULE dll_xi = LoadLibrary("XInput1_4.dll");
  if (!dll_xi) {
    dll_xi = LoadLibrary("XInput1_3.dll");
    if (!dll_xi)
      dll_xi = LoadLibrary("bin\\XInput1_3.dll");
  }
  if (!dll_xi) {
    xinput_available = false;
    error_handle(LOW_PRIORITY, JOY_XI_LOADDL_FAILED, "sgl_joystick_init() failed: Couldn't find XInput DLL");
  } else {
    XInputGetStateEx_proc = (DWORD(WINAPI*)(DWORD, XINPUT_STATE_EX*)) GetProcAddress(dll_xi, (LPCSTR)100);
    XInputGetState_proc = (DWORD(WINAPI*)(DWORD, XINPUT_STATE *)) GetProcAddress(dll_xi, "XInputGetState");
    XInputGetCapabilities_proc = (DWORD(WINAPI*)(DWORD, DWORD, XINPUT_CAPABILITIES*)) GetProcAddress(dll_xi, "XInputGetCapabilities");
  }
#endif

  HMODULE dll_di = LoadLibrary("DINPUT8.dll");
  if (!dll_di) {
    error_handle(LOW_PRIORITY, JOY_DI_LOADDL_FAILED, "LoadLibrary() failed: DINPUT8.dll not found");
    return false;
  }
  HRESULT(WINAPI* DirectInput8Create_proc)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN) = (HRESULT(WINAPI *)(HINSTANCE, DWORD, REFIID, LPVOID *, LPUNKNOWN)) GetProcAddress(dll_di, "DirectInput8Create");
  if (DirectInput8Create_proc(GetModuleHandle(NULL), DIRECTINPUT_VERSION, &IID_IDirectInput8, (VOID**)&did, NULL) != DI_OK) {
    error_handle(LOW_PRIORITY, JOY_DI_INIT_FAILED, "DirectInput8Create() failed: %s", GetLastError());
    return false;
  }
#endif
  return (scan_too ? sgl_joystick_scan() : true);
}

#if defined(SGL_DISABLE_DIRECTINPUT)
static char* get_mm_description(UINT id, JOYCAPS caps) {
  char sub_key[REG_STRING_MAX];
  snprintf(sub_key, REG_STRING_MAX, "%s\\%s\\%s", REGSTR_PATH_JOYCONFIG, caps.szRegKey, REGSTR_KEY_JOYCURR);
  LONG result;
  HKEY top_key, key;
  char* description = NULL;

  if ((result = RegOpenKeyEx(top_key = HKEY_LOCAL_MACHINE, sub_key, 0, KEY_READ, &key)) != ERROR_SUCCESS)
    result = RegOpenKeyEx(top_key = HKEY_CURRENT_USER, sub_key, 0, KEY_READ, &key);
  if (result == ERROR_SUCCESS) {
    char value[REG_STRING_MAX];
    char name[REG_STRING_MAX];
    snprintf(value, REG_STRING_MAX, "Joystick%d%s", id + 1, REGSTR_VAL_JOYOEMNAME);
    DWORD name_sz = sizeof(name);
    result = RegQueryValueEx(key, value, NULL, NULL, (LPBYTE)name, &name_sz);
    RegCloseKey(key);

    if (result == ERROR_SUCCESS) {
      snprintf(sub_key, REG_STRING_MAX, "%s\\%s", REGSTR_PATH_JOYOEM, name);
      result = RegOpenKeyEx(top_key, sub_key, 0, KEY_READ, &key);

      if (result == ERROR_SUCCESS) {
        name_sz = sizeof(name);
        result = RegQueryValueEx(key, REGSTR_VAL_JOYOEMNAME, NULL, NULL, NULL, &name_sz);

        if (result == ERROR_SUCCESS) {
          description = malloc(name_sz);
          result = RegQueryValueEx(key, REGSTR_VAL_JOYOEMNAME, NULL, NULL, (LPBYTE)description, &name_sz);
        }
        RegCloseKey(key);

        if (result == ERROR_SUCCESS)
          return description;
        free(description);
      }
    }
  }

  description = malloc(strlen(caps.szPname) + 1);
  strcpy(description, caps.szPname);
  return description;
}

static void handle_btn_change(joystick_t* device, DWORD lv, DWORD v) {
  for (int b = 0; b < device->n_buttons; ++b) {
    if (!((lv ^ v) & (1 << b)))
      continue;

    bool down = !!(v & (1 << b));
    device->buttons[b] = down;
    CALL(joy_btn_callback, device, b, down, ticks());
  }
}

static void handle_axis_change(joystick_t* device, int index, DWORD iv) {
  if (index < 0 || index >= device->n_axes)
    return;

  joystick_private_t* private = (joystick_private_t*)device->__private;

  float v  = (iv - private->axis_ranges[index][0]) / (float)(private->axis_ranges[index][1] - private->axis_ranges[index][0]) * 2.0f - 1.0f;
  float lv = device->axes[index];
  device->axes[index] = v;

  CALL(joy_axis_callback, device, index, v, lv, ticks());
}

static void pov_to_xy(DWORD pov, int* x, int* y) {
  if (pov == JOY_POVCENTERED)
    *x = *y = 0;
  else {
    if (pov > JOY_POVFORWARD && pov < JOY_POVBACKWARD)
      *x = 1;
    else if (pov > JOY_POVBACKWARD)
      *x = -1;
    else
      *x = 0;

    if (pov > JOY_POVLEFT || pov < JOY_POVRIGHT)
      *y = -1;
    else if (pov > JOY_POVRIGHT && pov < JOY_POVLEFT)
      *y = 1;
    else
      *y = 0;
  }
}

static void handle_pov_change(joystick_t* device, DWORD lv, DWORD v) {
  joystick_private_t* private = (joystick_private_t*)device->__private;
  if (private->povXAxisIndex == -1 || private->povYAxisIndex == -1)
    return;

  int lx, ly, nx, ny;
  pov_to_xy(lv, &lx, &ly);
  pov_to_xy(v, &nx, &ny);

  if (nx != lx) {
    device->axes[private->povXAxisIndex] = nx;
    CALL(joy_axis_callback, device, private->povXAxisIndex, nx, lx, ticks());
  }
  if (ny != ly) {
    device->axes[private->povYAxisIndex] = ny;
    CALL(joy_axis_callback, device, private->povYAxisIndex, ny, ly, ticks());
  }
}
#else
static void update_btn(joystick_t* device, unsigned int index, bool down, long time) {
  device->buttons[index] = down;
  CALL(joy_btn_callback, device, index, down, time);
}

static void update_axis_float(joystick_t* device, unsigned int index, float val, long time) {
  float last_val = device->axes[index];
  device->axes[index] = val;
  CALL(joy_axis_callback, device, index, val, last_val, time);
}

#define UPDATE_AXIS_IVAL(d, i, iv, t) (update_axis_float((d), (i), ((iv) - AXIS_MIN) / (float)(AXIS_MAX - AXIS_MIN) * 2.0f - 1.0f, (t)))

#define POV_UP 0
#define POV_RIGHT 9000
#define POV_DOWN 18000
#define POV_LEFT 27000

static void update_axis_pov(joystick_t* device, unsigned int index, DWORD iv, long time) {
  float x = 0.0f, y = 0.0f;
  if (LOWORD(iv) == 0xFFFF)
    x = y = 0.0f;
  else {
    if (iv > POV_UP && iv < POV_DOWN)
      x = 1.0f;
    else if (iv > POV_DOWN)
      x = -1.0f;
    else
      x = 0.0f;

    if (iv > POV_LEFT || iv < POV_RIGHT)
      y = -1.0f;
    else if (iv > POV_RIGHT && iv < POV_LEFT)
      y = 1.0f;
    else
      y = 0.0f;
  }
  update_axis_float(device, index, x, time);
  update_axis_float(device, index + 1, y, time);
}
#endif

bool sgl_joystick_scan() {
#if defined(SGL_DISABLE_DIRECTINPUT)
  unsigned int n_supported = joyGetNumDevs();
  JOYINFOEX info;
  JOYCAPS caps;

  for (int i = 0; i < n_supported; ++i) {
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNALL;
    int id = JOYSTICKID1 + i;
    if (joyGetPosEx(id, &info) == JOYERR_NOERROR && joyGetDevCaps(id, &caps, sizeof(JOYCAPS)) == JOYERR_NOERROR) {
      bool is_dupe = false;
      joystick_t* current = joy_devices.head;
      while (current) {
        if (((joystick_private_t*)current->__private)->joystick_id == id) {
          is_dupe = true;
          break;
        }
        current = current->next;
      }
      if (is_dupe)
        continue;

      joystick_private_t* private = malloc(sizeof(joystick_private_t));
      private->joystick_id = id;
      private->last_state = info;
      private->xAxisIndex = 0;
      private->yAxisIndex = 1;
      int a = 2;
      private->zAxisIndex = (caps.wCaps & JOYCAPS_HASZ) ? a++ : -1;
      private->rAxisIndex = (caps.wCaps & JOYCAPS_HASR) ? a++ : -1;
      private->uAxisIndex = (caps.wCaps & JOYCAPS_HASU) ? a++ : -1;
      private->vAxisIndex = (caps.wCaps & JOYCAPS_HASV) ? a++ : -1;
      private->axis_ranges = malloc(sizeof(UINT[2]) * a);
      private->axis_ranges[0][0] = caps.wXmin;
      private->axis_ranges[0][1] = caps.wXmax;
      private->axis_ranges[1][0] = caps.wYmin;
      private->axis_ranges[1][1] = caps.wYmax;
      if (private->zAxisIndex != -1) {
        private->axis_ranges[private->zAxisIndex][0] = caps.wZmin;
        private->axis_ranges[private->zAxisIndex][1] = caps.wZmax;
      }
      if (private->rAxisIndex != -1) {
        private->axis_ranges[private->rAxisIndex][0] = caps.wRmin;
        private->axis_ranges[private->rAxisIndex][1] = caps.wRmax;
      }
      if (private->uAxisIndex != -1) {
        private->axis_ranges[private->uAxisIndex][0] = caps.wUmin;
        private->axis_ranges[private->uAxisIndex][1] = caps.wUmax;
      }
      if (private->vAxisIndex != -1) {
        private->axis_ranges[private->vAxisIndex][0] = caps.wVmin;
        private->axis_ranges[private->vAxisIndex][1] = caps.wVmax;
      }
      private->povXAxisIndex = (caps.wCaps & JOYCAPS_HASPOV) ? a++ : -1;
      private->povYAxisIndex = (caps.wCaps & JOYCAPS_HASPOV) ? a++ : -1;

      joystick_t* device = malloc(sizeof(joystick_t));
      device->__private = private;
      device->next = NULL;
      device->device_id = next_device_id++;
      device->description = get_mm_description(id, caps);
      device->vendor_id = caps.wMid;
      device->product_id = caps.wPid;
      device->n_axes = caps.wNumAxes + ((caps.wCaps & JOYCAPS_HASPOV) ? 2 : 0);
      device->n_buttons = caps.wNumButtons;
      device->axes = malloc(sizeof(float) * device->n_axes);
      device->buttons = malloc(sizeof(int) * device->n_buttons);

      add_joystick(device);
    }
  }
#else
  if (!did) {
    error_handle(LOW_PRIORITY, JOY_DI_ENUM_DEVICE_FAILED, "IDirectInput_EnumDevices() failed: DirectInput not initiated, call sgl_joystick_init first");
    return false;
  }

  if (IDirectInput_EnumDevices(did, DI8DEVCLASS_GAMECTRL, enum_devices_cb, NULL, DIEDFL_ALLDEVICES) != DI_OK) {
    error_handle(LOW_PRIORITY, JOY_DI_ENUM_DEVICE_FAILED, "IDirectInput_EnumDevices() failed: %s", GetLastError());
    return false;
  }

#if !defined(SGL_DISABLE_XINPUT)
  DWORD xresult;
  XINPUT_CAPABILITIES capabilities;
  if (xinput_available) {
    for (unsigned int i = 0; i < MAX_XINPUT_DEVICES; ++i) {
      xresult = XInputGetCapabilities_proc(i, 0, &capabilities);
      if (xresult == ERROR_SUCCESS && xinput_devices[i] == NULL) {
        joystick_t* device = malloc(sizeof(joystick_t));
        joystick_private_t* private = malloc(sizeof(joystick_private_t));

        private->is_xinput = TRUE;
        private->player_index = i;
        device->__private = private;
        device->next = NULL;
        device->device_id = next_device_id++;
        device->description = xinput_device_names[i];
        device->vendor_id = 0x45E;
        device->product_id = 0x28E;
        device->n_axes = 6;
        device->n_buttons = 15;
        device->axes = malloc(sizeof(float) * device->n_axes);
        device->buttons = malloc(sizeof(int) * device->n_buttons);

        add_joystick(device);
        xinput_devices[i] = device;
      }
      else if (xresult != ERROR_SUCCESS && xinput_devices[i]) {
        sgl_joystick_remove(xinput_devices[i]->device_id);
        xinput_devices[i] = NULL;
      }
    }
  }
#endif
#endif

  return true;
}

static inline void release_joystick(joystick_t** d) {
  joystick_t* _d = *d;
  joystick_private_t* _p = _d->__private;

#if defined(SGL_DISABLE_DIRECTINPUT)
  FREE_SAFE(_p->axis_ranges);
  FREE_SAFE((void*)_d->description);
#else
  if (!_p->is_xinput) {
    IDirectInputDevice8_Release(_p->di8dev);
    FREE_SAFE(_p->axis_info);
    FREE_SAFE(_p->button_offsets);
    FREE_SAFE((void*)_d->description);
  }
#endif
  FREE_SAFE(_p);
  FREE_SAFE(_d->axes);
  FREE_SAFE(_d->buttons);
  FREE_SAFE(_d);
}

void sgl_joystick_release() {
  joystick_t* current = joy_devices.head;
  joystick_t* next = current;
  while (current) {
    next = current->next;
    release_joystick(&current);
    current = next;
  }
  joy_devices.head = NULL;
  joy_devices.size = 0;
}

bool sgl_joystick_remove(int id) {
  joystick_t* current = joy_devices.head;
  joystick_t* previous = current;
  while (current) {
    if (current->device_id == id) {
      previous->next = current->next;
      if (current == joy_devices.head)
        joy_devices.head = current->next;
      CALL(joy_removed_callback, current, current->device_id);
      release_joystick(&current);
      return true;
    }
    previous = current;
    current = current->next;
  }
  return false;
}

void sgl_joystick_poll() {
  joystick_t* device = joy_devices.head;
  joystick_private_t* private = NULL;
  HRESULT result;
#if defined(SGL_DISABLE_DIRECTINPUT)
  JOYINFOEX info;
#endif

  while (device) {
    private = device->__private;

#if defined(SGL_DISABLE_DIRECTINPUT)
    info.dwSize = sizeof(info);
    info.dwFlags = JOY_RETURNALL;
    result = joyGetPosEx(private->joystick_id, &info);
    if (result == JOYERR_UNPLUGGED)
      sgl_joystick_remove(device->device_id);
    else if (result == JOYERR_NOERROR) {
      if (info.dwXpos != private->last_state.dwXpos)
        handle_axis_change(device, private->xAxisIndex, info.dwXpos);
      if (info.dwYpos != private->last_state.dwYpos)
        handle_axis_change(device, private->yAxisIndex, info.dwYpos);
      if (info.dwZpos != private->last_state.dwZpos)
        handle_axis_change(device, private->zAxisIndex, info.dwZpos);
      if (info.dwRpos != private->last_state.dwRpos)
        handle_axis_change(device, private->rAxisIndex, info.dwRpos);
      if (info.dwUpos != private->last_state.dwUpos)
        handle_axis_change(device, private->uAxisIndex, info.dwUpos);
      if (info.dwVpos != private->last_state.dwVpos)
        handle_axis_change(device, private->vAxisIndex, info.dwVpos);
      if (info.dwPOV != private->last_state.dwPOV)
        handle_pov_change(device, private->last_state.dwPOV, info.dwPOV);
      if (info.dwButtons != private->last_state.dwButtons)
        handle_btn_change(device, private->last_state.dwButtons, info.dwButtons);
      private->last_state = info;
    }
#else
#if !defined(SGL_DISABLE_XINPUT)
    if (private->is_xinput) {
      XINPUT_STATE state;
      DWORD xresult;

      if (XInputGetStateEx_proc) {
        XINPUT_STATE_EX state_ex;
        xresult = XInputGetStateEx_proc(private->player_index, &state_ex);
        state.Gamepad.wButtons = state_ex.Gamepad.wButtons;
        state.Gamepad.sThumbLX = state_ex.Gamepad.sThumbLX;
        state.Gamepad.sThumbLY = state_ex.Gamepad.sThumbLY;
        state.Gamepad.sThumbRX = state_ex.Gamepad.sThumbRX;
        state.Gamepad.sThumbRY = state_ex.Gamepad.sThumbRY;
        state.Gamepad.bLeftTrigger = state_ex.Gamepad.bLeftTrigger;
        state.Gamepad.bRightTrigger = state_ex.Gamepad.bRightTrigger;
      } else
        xresult = XInputGetState_proc(private->player_index, &state);

      if (xresult == ERROR_SUCCESS) {
        update_btn(device, 0, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP), ticks());
        update_btn(device, 1, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN), ticks());
        update_btn(device, 2, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT), ticks());
        update_btn(device, 3, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT), ticks());
        update_btn(device, 4, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_START), ticks());
        update_btn(device, 5, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK), ticks());
        update_btn(device, 6, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB), ticks());
        update_btn(device, 7, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB), ticks());
        update_btn(device, 8, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER), ticks());
        update_btn(device, 9, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER), ticks());
        update_btn(device, 10, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_A), ticks());
        update_btn(device, 11, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_B), ticks());
        update_btn(device, 12, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_X), ticks());
        update_btn(device, 13, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y), ticks());
        update_btn(device, 14, !!(state.Gamepad.wButtons & XINPUT_GAMEPAD_GUIDE), ticks());
        UPDATE_AXIS_IVAL(device, 0, state.Gamepad.sThumbLX, ticks());
        UPDATE_AXIS_IVAL(device, 1, state.Gamepad.sThumbLY, ticks());
        UPDATE_AXIS_IVAL(device, 2, state.Gamepad.sThumbRX, ticks());
        UPDATE_AXIS_IVAL(device, 3, state.Gamepad.sThumbRY, ticks());
        update_axis_float(device, 4, state.Gamepad.bLeftTrigger / 127.5f - 1.0f, ticks());
        update_axis_float(device, 5, state.Gamepad.bRightTrigger / 127.5f - 1.0f, ticks());
      } else {
        sgl_joystick_remove(device->device_id);
        xinput_devices[private->player_index] = NULL;
      }
    } else {
#endif
#pragma FIXME(Crash when disconnecting device)
      result = IDirectInputDevice8_Poll(private->di8dev);
      if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
        IDirectInputDevice8_Acquire(private->di8dev);
        IDirectInputDevice8_Poll(private->di8dev);
      }

      if (private->buffered) {
        DWORD event_c = INPUT_QUEUE_SIZE;
        DIDEVICEOBJECTDATA events[INPUT_QUEUE_SIZE];

        result = IDirectInputDevice8_GetDeviceData(private->di8dev, sizeof(DIDEVICEOBJECTDATA), events, &event_c, 0);
        if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
          IDirectInputDevice8_Acquire(private->di8dev);
          result = IDirectInputDevice8_GetDeviceData(private->di8dev, sizeof(DIDEVICEOBJECTDATA), events, &event_c, 0);
        }
        if (result != DI_OK) {
          sgl_joystick_remove(device->device_id);
          continue;
        }

        int e, b, a;
        for (e = 0; e < event_c; ++e) {
          for (b = 0; b < device->n_buttons; ++b)
            if (events[e].dwOfs == private->button_offsets[b])
              update_btn(device, b, !!events[e].dwData, events[e].dwTimeStamp / 1000);
          for (a = 0; a < device->n_axes; ++a) {
            if (events[e].dwOfs == private->axis_info[a].offset) {
              if (private->axis_info[a].is_pov) {
                update_axis_pov(device, a, events[e].dwData, events[e].dwTimeStamp / 1000);
                a++;
              } else
                UPDATE_AXIS_IVAL(device, a, events[e].dwData, events[e].dwTimeStamp / 1000);
            }
          }
        }
      } else {
        DIJOYSTATE2 state;

        result = IDirectInputDevice8_GetDeviceState(private->di8dev, sizeof(DIJOYSTATE2), &state);
        if (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
          IDirectInputDevice8_Acquire(private->di8dev);
          result = IDirectInputDevice8_GetDeviceState(private->di8dev, sizeof(DIJOYSTATE2), &state);
        }

        if (result != DI_OK) {
          sgl_joystick_remove(device->device_id);
          continue;
        }

        for (int b = 0; b < device->n_buttons; ++b)
          update_btn(device, b, !!state.rgbButtons[b], sgl_ticks());

        for (int a = 0; a < device->n_axes; ++a) {
          switch (private->axis_info[a].offset) {
            case DIJOFS_X:
              UPDATE_AXIS_IVAL(device, a, state.lX, sgl_ticks());
              break;
            case DIJOFS_Y:
              UPDATE_AXIS_IVAL(device, a, state.lY, sgl_ticks());
              break;
            case DIJOFS_Z:
              UPDATE_AXIS_IVAL(device, a, state.lZ, sgl_ticks());
              break;
            case DIJOFS_RX:
              UPDATE_AXIS_IVAL(device, a, state.lRx, sgl_ticks());
              break;
            case DIJOFS_RY:
              UPDATE_AXIS_IVAL(device, a, state.lRy, sgl_ticks());
              break;
            case DIJOFS_RZ:
              UPDATE_AXIS_IVAL(device, a, state.lRz, sgl_ticks());
              break;
            case DIJOFS_SLIDER(0):
              UPDATE_AXIS_IVAL(device, a, state.rglSlider[0], sgl_ticks());
              break;
            case DIJOFS_SLIDER(1):
              UPDATE_AXIS_IVAL(device, a, state.rglSlider[1], sgl_ticks());
              break;
            case DIJOFS_POV(0):
              update_axis_pov(device, a, state.rgdwPOV[0], sgl_ticks());
              a++;
              break;
            case DIJOFS_POV(1):
              update_axis_pov(device, a, state.rgdwPOV[1], sgl_ticks());
              a++;
              break;
            case DIJOFS_POV(2):
              update_axis_pov(device, a, state.rgdwPOV[2], sgl_ticks());
              a++;
              break;
            case DIJOFS_POV(3):
              update_axis_pov(device, a, state.rgdwPOV[3], sgl_ticks());
              a++;
              break;
          }
        }
      }
#if !defined(SGL_DISABLE_XINPUT)
    }
#endif
#endif

    device = device->next;
  }
}
#endif

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

void set_adjusted_win_wh(int w, int h) {
  RECT rect = { 0 };
  rect.right = w;
  rect.bottom = h;
  AdjustWindowRect(&rect, adjust_flags, 0);
  adjusted_win_w = rect.right - rect.left;
  adjusted_win_h = rect.bottom - rect.top;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  LRESULT res = 0;
  switch (message) {
    case WM_PAINT:
      if (buffer) {
#if defined(SGL_ENABLE_OPENGL)
        draw_gl();
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
#elif defined(SGL_ENABLE_DX9)
        IDirect3DDevice9_Clear(d3ddev, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(255, 0, 0), 1.0f, 0);
        IDirect3DDevice9_BeginScene(d3ddev);
        IDirect3DDevice9_EndScene(d3ddev);
        IDirect3DDevice9_Present(d3ddev, NULL, NULL, NULL, NULL);
#else
        bmpinfo->bmiHeader.biWidth = buffer->w;
        bmpinfo->bmiHeader.biHeight = -buffer->h;
        StretchDIBits(hdc, 0, 0, win_w, win_h, 0, 0, buffer->w, buffer->h, buffer->buf, bmpinfo, DIB_RGB_COLORS, SRCCOPY);
        ValidateRect(hWnd, NULL);
#endif
      }
      break;
    case WM_SIZE:
      if (!ifuckinghatethewin32api)
        break;
      RECT rect = { 0 };
      rect.right = LOWORD(lParam);
      rect.bottom = HIWORD(lParam);
      AdjustWindowRect(&rect, adjust_flags, 0);
      win_w = rect.right + rect.left;
      win_h = rect.bottom - rect.top;
      CALL(__resize_callback, win_w, win_h);
#if defined(SGL_ENABLE_OPENGL)
      glViewport(rect.left, rect.top, rect.right, win_h);
      PostMessage(hWnd, WM_PAINT, 0, 0);
#endif
      break;
    case WM_CLOSE:
      __closed = true;
      break;
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP: {
      int kb_key = translate_key(wParam, lParam);
      bool kb_action = !((lParam >> 31) & 1);
      int kb_mods = translate_mod();

      if (kb_key == KB_KEY_UNKNOWN)
        return FALSE;
      if (!kb_action && wParam == VK_SHIFT) {
        CALL(__kb_callback, KB_KEY_LEFT_SHIFT, kb_mods, kb_action);
      } else if (wParam == VK_SNAPSHOT) {
        CALL(__kb_callback, kb_key, kb_mods, false);
      } else {
        CALL(__kb_callback, kb_key, kb_mods, kb_action);
      }
      break;
    }
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
      int m_button, m_action = 0;
      switch (message) {
        case WM_LBUTTONDOWN:
          m_action = 1;
        case WM_LBUTTONUP:
          m_button = MOUSE_BTN_1;
          break;
        case WM_RBUTTONDOWN:
          m_action = 1;
        case WM_RBUTTONUP:
          m_button = MOUSE_BTN_2;
          break;
        case WM_MBUTTONDOWN:
          m_action = 1;
        case WM_MBUTTONUP:
          m_button = MOUSE_BTN_3;
          break;
        default:
          m_button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MOUSE_BTN_5 : MOUSE_BTN_6);
          if (message == WM_XBUTTONDOWN)
            m_action = 1;
      }
      CALL(__mouse_btn_callback, (MOUSEBTN)m_button, translate_mod(), m_action);
      break;
    }
    case WM_MOUSEWHEEL:
    case WM_MOUSEHWHEEL:
      CALL(__scroll_callback, translate_mod(), -((SHORT)HIWORD(wParam) / (float)WHEEL_DELTA), (SHORT)HIWORD(wParam) / (float)WHEEL_DELTA);
      break;
    case WM_MOUSEMOVE:
      CALL(__mouse_move_callback, ((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)), 0, 0);
      if (cursor_locked && is_focused) {
        GetWindowRect(hwnd, &rc);
        rc.left += 3;
        rc.right -= 3;
        rc.top += 3;
        rc.bottom -= 4;
        ClipCursor(&rc);
      }
      break;
    case WM_SETCURSOR:
      SetCursor(__cursor);
      break;
    case WM_SETFOCUS:
      is_focused = TRUE;
      CALL(__focus_callback, is_focused);
      break;
    case WM_KILLFOCUS:
      is_focused = FALSE;
      CALL(__focus_callback, is_focused);
      break;
    default:
      res = DefWindowProc(hWnd, message, wParam, lParam);
  }
  return res;
}

void sgl_cursor(bool shown, bool locked, CURSORTYPE type) {
  ShowCursor((BOOL)shown);
  cursor_locked = (BOOL)locked;

  LPCSTR c = NULL;
  switch (type) {
    default:
    case CURSOR_NO_CHANGE:
      return;
    case CURSOR_ARROW:
      c = IDC_ARROW;
      break;
    case CURSOR_IBEAM:
      c = IDC_IBEAM;
      break;
    case CURSOR_WAIT:
    case CURSOR_WAITARROW:
      c = IDC_WAIT;
      break;
    case CURSOR_SIZENWSE:
      c = IDC_SIZENWSE;
      break;
    case CURSOR_SIZENESW:
      c = IDC_SIZENESW;
      break;
    case CURSOR_SIZEWE:
      c = IDC_SIZEWE;
      break;
    case CURSOR_SIZENS:
      c = IDC_SIZENS;
      break;
    case CURSOR_SIZEALL:
      c = IDC_SIZEALL;
      break;
    case CURSOR_NO:
      c = IDC_NO;
      break;
    case CURSOR_HAND:
      c = IDC_HAND;
      break;
    case CURSOR_CUSTOM:
      if (!____custom_cursor) {
        error_handle(LOW_PRIORITY, CUSTOM_CURSOR_NOT_CREATED, "cursor() failed: No custom cursor loaded");
        return;
      }
  }

  if (__cursor && __cursor != ____custom_cursor)
    DestroyCursor(__cursor);
  __cursor = (c ? LoadCursor(NULL, c) : ____custom_cursor);
}

void sgl_custom_cursor(surface_t* s) {
#pragma TODO(Custom cursor loading for Windows)
}

bool sgl_screen(const char* title, surface_t* s, int w, int h, short flags) {
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

  long _flags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
  if (flags & FULLSCREEN) {
    flags |= (BORDERLESS | FULLSCREEN_DESKTOP);

    DEVMODE settings;
    EnumDisplaySettings(0, 0, &settings);
    settings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
    settings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
    settings.dmBitsPerPel = 32;
    settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
    if (ChangeDisplaySettings(&settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
      cursor_locked = 1;
      error_handle(LOW_PRIORITY, WIN_FULLSCREEN_FAILED, "screen() failed: Failed to go to fullscreen mode: Defaulting to fullscreen desktop");
    }
  }
  _flags |= (flags & RESIZABLE ? WS_THICKFRAME : 0);
  if (flags & BORDERLESS) {
    _flags |= ~(WS_OVERLAPPED | WS_CAPTION);
    _flags |= WS_POPUP;
    adjust_flags = WS_POPUP;
  }
  if (flags & FULLSCREEN_DESKTOP) {
    w = win_w = GetSystemMetrics(SM_CXSCREEN);
    h = win_h = GetSystemMetrics(SM_CYSCREEN);
  } else {
    win_w = w;
    win_h = h;
  }

  set_adjusted_win_wh(w, h);

  if (s)
    if (!sgl_surface(s, w, h))
      return false;

  int cx = GetSystemMetrics(SM_CXSCREEN) / 2 - adjusted_win_w / 2,
      cy = GetSystemMetrics(SM_CYSCREEN) / 2 - adjusted_win_h / 2;

  HINSTANCE hinst = GetModuleHandle(NULL);
#if defined(SGL_ENABLE_OPENGL)
  wnd.style = CS_OWNDC;
#else
  wnd.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
#endif
  wnd.lpfnWndProc = (WNDPROC)WndProc;
  wnd.lpszClassName = title;
  wnd.hInstance = hinst;

  if (!RegisterClass(&wnd)) {
    sgl_release();
    error_handle(HIGH_PRIORITY, WIN_WINDOW_CREATION_FAILED, "RegisterClass() failed: %s", GetLastError());
    return false;
  }

  if (!(hwnd = CreateWindow(title, title, _flags, cx, cy, adjusted_win_w, adjusted_win_h, NULL, NULL, hinst, NULL))) {
    sgl_release();
    error_handle(HIGH_PRIORITY, WIN_WINDOW_CREATION_FAILED, "CreateWindowEx() failed: %s", GetLastError());
    return false;
  }

  hdc = GetDC(hwnd);

#if defined(SGL_ENABLE_OPENGL)
  memset(&pfd, 0, sizeof(pfd));
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  int pf = ChoosePixelFormat(hdc, &pfd);
  if (pf == 0) {
    sgl_release();
    error_handle(HIGH_PRIORITY, WIN_GL_PF_ERROR, "ChoosePixelFormat() failed: %s", GetLastError());
    return false;
  }

  if (SetPixelFormat(hdc, pf, &pfd) == FALSE) {
    sgl_release();
    error_handle(HIGH_PRIORITY, WIN_GL_PF_ERROR, "SetPixelFormat() failed: %s", GetLastError());
    return false;
  }

  DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  hrc = wglCreateContext(hdc);
  wglMakeCurrent(hdc, hrc);

  if (!init_gl(w, h))
    return false;
#elif defined(SGL_ENABLE_DX9)
  d3d = Direct3DCreate9(D3D_SDK_VERSION);

  D3DPRESENT_PARAMETERS d3dpp;
  ZeroMemory(&d3dpp, sizeof(d3dpp));
  d3dpp.Windowed = TRUE;
  d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3dpp.hDeviceWindow = hwnd;

  IDirect3D9_CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);
#else
  bmpinfo = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3);
  bmpinfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmpinfo->bmiHeader.biPlanes = 1;
  bmpinfo->bmiHeader.biBitCount = 32;
  bmpinfo->bmiHeader.biCompression = BI_BITFIELDS;
  bmpinfo->bmiHeader.biWidth = w;
  bmpinfo->bmiHeader.biHeight = -h;
  bmpinfo->bmiColors[0].rgbRed = 0xFF;
  bmpinfo->bmiColors[1].rgbGreen = 0xFF;
  bmpinfo->bmiColors[2].rgbBlue = 0xFF;
#endif

  ShowWindow(hwnd, SW_NORMAL);

  if (flags & ALWAYS_ON_TOP)
    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

  SetFocus(hwnd);

  __cursor = LoadCursor(NULL, IDC_ARROW);
  ifuckinghatethewin32api = TRUE; // because I do

  return true;
}

void sgl_poll() {
  MSG msg;
  if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

void sgl_flush(surface_t* s) {
  if (s && s->buf)
    buffer = s;
  InvalidateRect(hwnd, NULL, TRUE);
  SendMessage(hwnd, WM_PAINT, 0, 0);
}

void sgl_release() {
#if defined(SGL_ENABLE_OPENGL)
  free_gl();
  wglMakeCurrent(NULL, NULL);
#elif defined(SGL_ENABLE_DX9)
  IDirect3DDevice9_Release(d3ddev);
  IDirect3D9_Release(d3d);
#else
  FREE_SAFE(bmpinfo);
#endif

  if (__cursor)
    DestroyCursor(__cursor);
  if (____custom_cursor)
    DestroyCursor(____custom_cursor);
  ReleaseDC(hwnd, hdc);
  DestroyWindow(hwnd);
}

bool sgl_closed() {
  return __closed;
}
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

static Display* display;
static surface_t* buffer;
static Window win;
static GC gc;
#if defined(SGL_ENABLE_OPENGL)
static GLXContext ctx;
static Colormap cmap;
#else
static XImage* img;
#endif
static XEvent event;
static KeySym sym;
static Cursor __cursor = 0, ____custom_cursor = 0;

#define Button6 6
#define Button7 7

#if defined(SGL_ENABLE_OPENGL)
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

static int check_ext(const char* list, const char* exts) {
  const char *start, *where = strchr(exts, ' '), *terminator;
  if (where || *exts == '\0')
    return 0;

  for (start = list;;) {
    where = strstr(start, exts);

    if (!where)
      break;

    terminator = where + strlen(exts);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return 1;

    start = terminator;
  }

  return 0;
}
#endif

#if defined(SGL_ENABLE_JOYSTICKS)
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/input.h>
#define __USE_UNIX98
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

typedef enum {
  JOY_DEVICE_ATTACHED,
  JOY_DEVICE_REMOVED,
  JOY_BTN_DOWN,
  JOY_BTN_UP,
  JOY_AXIS_MOVED
} JOYEVENTTYPE;

typedef struct {
	pthread_t thrd;
	int fd;
	char* path;
	int buttons[KEY_CNT - BTN_MISC];
	int axes[ABS_CNT];
	struct input_absinfo axis_info[ABS_CNT];
} joystick_private_t;

typedef struct __joy_q_event_t {
  unsigned int device_id;
  JOYEVENTTYPE event;
  struct __joy_q_event_t* next;
  void* data;
} joystick_queued_event_t;

static struct {
  int size;
  joystick_queued_event_t* head;
} event_queue;

typedef struct {
  joystick_t* device;
  long timestamp;
  unsigned int btn_id;
  bool down;
} joystick_btn_event_t;

typedef struct {
  joystick_t* device;
  long timestamp;
  unsigned int axis_id;
  float value, last_value;
} joystick_axis_event_t;

static pthread_mutex_t devices_mtx;
static pthread_mutex_t event_queue_mtx;

#if !defined(MAX_PATH)
#define MAX_PATH 128
#endif

#define test_bit(b, a) ((a[(b) / (sizeof(int) * 8)] >> ((b) % (sizeof(int) * 8))) & 0x1)

joystick_t* sgl_joystick(int id) {
  if (id < 0 || !joy_devices.head)
    return NULL;
  joystick_t* ret = NULL;
  pthread_mutex_lock(&devices_mtx);
  joystick_t* current = joy_devices.head;
  while (current) {
    if (current->device_id == id) {
      ret = current;
      break;
    }
    current = current->next;
  }
  pthread_mutex_lock(&devices_mtx);
  return ret;
}

static void queue_joy_event(unsigned int id, JOYEVENTTYPE type, void* data) {
  pthread_mutex_lock(&event_queue_mtx);

  joystick_queued_event_t* e = malloc(sizeof(joystick_queued_event_t));
  e->device_id = id;
  e->event = type;
  e->data = data;
  e->next = NULL;

  joystick_queued_event_t* current = event_queue.head;
  if (!current)
    event_queue.head = e;
  else {
    while (current)
      current = current->next;
    current->next = e;
  }
  event_queue.size++;

  pthread_mutex_unlock(&event_queue_mtx);
}

static void queue_axis_event(joystick_t* device, long time, unsigned int id, float v, float lv) {
  joystick_axis_event_t* e = malloc(sizeof(joystick_axis_event_t));
  e->device = device;
  e->timestamp = time;
  e->axis_id = id;
  e->value = v;
  e->last_value = lv;
  queue_joy_event(device->device_id, JOY_AXIS_MOVED, (void*)e);
}

static void queue_btn_event(joystick_t* device, long time, unsigned int id, bool down) {
  joystick_btn_event_t* e = malloc(sizeof(joystick_btn_event_t));
  e->device = device;
  e->timestamp = time;
  e->btn_id = id;
  e->down = down;
  queue_joy_event(device->device_id, down ? JOY_BTN_DOWN : JOY_BTN_UP, (void*)e);
}

static void* device_thrd(void* ctx) {
  joystick_t* device = (joystick_t*)ctx;
  joystick_private_t* private = (joystick_private_t*)device->__private;
  struct input_event event;
  while (read(private->fd, &event, sizeof(struct input_event)) > 0) {
    if (event.type == EV_ABS) {
      if (event.code > ABS_MAX || private->axes[event.code] == -1)
        continue;

      float value = (event.value - private->axis_info[event.code].minimum) / (float)(private->axis_info[event.code].maximum - private->axis_info[event.code].minimum) * 2.0f - 1.0f;
      queue_axis_event(device, (long)(event.time.tv_sec + event.time.tv_usec * 0.000001), private->axes[event.code], value, device->axes[private->axes[event.code]]);
      device->axes[private->axes[event.code]] = value;
    } else if (event.type == EV_KEY) {
      if (event.code < BTN_MISC || event.code > KEY_MAX || private->buttons[event.code - BTN_MISC] == -1)
        continue;
      queue_btn_event(device, (long)(event.time.tv_sec + event.time.tv_usec * 0.000001), private->buttons[event.code - BTN_MISC], !!event.value);
      device->buttons[private->buttons[event.code - BTN_MISC]] = !!event.value;
    }
  }

  queue_joy_event(device->device_id, JOY_DEVICE_REMOVED, device);
  sgl_joystick_remove(device->device_id);
  return NULL;
}

bool sgl_joystick_init(bool scan_too) {
  pthread_mutexattr_t rlock;
  pthread_mutexattr_init(&rlock);
  pthread_mutexattr_settype(&rlock, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&devices_mtx, &rlock);
  pthread_mutex_init(&event_queue_mtx, &rlock);
  return (scan_too ? sgl_joystick_scan() : true);
}

bool sgl_joystick_scan() {
  pthread_mutex_lock(&devices_mtx);
  DIR* dev_input = opendir("/dev/input");
  if (!dev_input) {
    error_handle(PRIO_LOW, JOY_NIX_SCAN_FAILED, "sgl_joystick_scan() failed: Couldn't open /dev/input");
    return false;
  }

  time_t c_time = time(NULL);
  static time_t l_time;
  struct stat sb;
  struct input_id id;
  char fname[MAX_PATH], name[128];
  struct dirent* de;
  int e_cap_bits[(EV_CNT  - 1) / sizeof(int) * 8 + 1];
	int e_key_bits[(KEY_CNT - 1) / sizeof(int) * 8 + 1];
	int e_abs_bits[(ABS_CNT - 1) / sizeof(int) * 8 + 1];
  while ((de = readdir(dev_input))) {
    int cc = 0, n;
    if (!sscanf(de->d_name, "event%d%n", &n, &cc) || cc != strlen(de->d_name))
      continue;

    snprintf(fname, PATH_MAX, "/dev/input/%s", de->d_name);
    if (stat(fname, &sb) ||sb.st_mtime < l_time)
      continue;

    bool duplicate = false;
    joystick_t* current = joy_devices.head;
    while (current) {
      if (!strcmp(((joystick_private_t*)current->__private)->path, fname)) {
        duplicate = true;
        break;
      }
      current = current->next;
    }
    if (duplicate)
      continue;

    int fd = open(fname, O_RDONLY, 0);
    memset(e_cap_bits, 0, sizeof(e_cap_bits));
    memset(e_key_bits, 0, sizeof(e_key_bits));
    memset(e_abs_bits, 0, sizeof(e_abs_bits));
    if (ioctl(fd, EVIOCGBIT(0, sizeof(e_cap_bits)), e_cap_bits) < 0 ||
        ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(e_key_bits)), e_key_bits) < 0 ||
        ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(e_abs_bits)), e_abs_bits) < 0) {
      close(fd);
      continue;
    }
    if (!test_bit(EV_KEY, e_cap_bits) || !test_bit(EV_ABS, e_cap_bits) ||
        !test_bit(ABS_X, e_abs_bits) || !test_bit(ABS_Y, e_abs_bits) ||
        (!test_bit(BTN_TRIGGER, e_key_bits) && !test_bit(BTN_A, e_key_bits) && !test_bit(BTN_1, e_key_bits))) {
      close(fd);
      continue;
    }

    joystick_private_t* private = malloc(sizeof(joystick_private_t));
    private->fd = fd;
    private->path = strdup(fname);
    memset(private->buttons, 0xFF, sizeof(private->buttons));
    memset(private->axes, 0xFF, sizeof(private->axes));

    joystick_t* device = malloc(sizeof(joystick_t));
    device->next = NULL;
    device->device_id = next_device_id++;
    device->__private = (void*)private;
    device->description = strdup((ioctl(fd, EVIOCGNAME(sizeof(name)), name) > 0 ? name : fname));
    if (!ioctl(fd, EVIOCGID, &id)) {
      device->vendor_id = id.vendor;
      device->product_id = id.product;
    } else
      device->vendor_id = device->product_id = 0;

    memset(e_key_bits, 0, sizeof(e_key_bits));
    memset(e_abs_bits, 0, sizeof(e_abs_bits));
    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(e_key_bits)), e_key_bits);
    ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(e_abs_bits)), e_abs_bits);

    device->n_axes = 0;
    for (int b = 0; b < ABS_CNT; ++b) {
      if (!test_bit(b, e_abs_bits))
        continue;
      if (ioctl(fd, EVIOCGABS(b), &private->axis_info[b]) < 0 ||
          private->axis_info[b].minimum == private->axis_info[b].maximum)
        continue;
      private->axes[b] = device->n_axes;
      device->n_axes++;
    }
    device->n_buttons = 0;
    for (int b = BTN_MISC; b < KEY_CNT; ++b) {
      if (!test_bit(b, e_key_bits))
        continue;
      private->buttons[b - BTN_MISC] = device->n_buttons;
      device->n_buttons++;
    }
    device->axes = malloc(sizeof(float) * device->n_axes);
    device->buttons = malloc(sizeof(int) * device->n_buttons);

    add_joystick(device);

    pthread_create(&private->thrd, NULL, device_thrd, device);
  }

  closedir(dev_input);
  l_time = c_time;
  pthread_mutex_unlock(&devices_mtx);
  return true;
}

static inline void release_joystick(joystick_t** d) {
  joystick_t* _d = *d;
  joystick_private_t* _p = _d->__private;
  close(_p->fd);
  FREE_SAFE(_p->path);
  FREE_SAFE(_p);
  pthread_cancel(_p->thrd);
  pthread_join(_p->thrd, NULL);
  if (_d->description)
    free((void*)_d->description);
  FREE_SAFE(_d->axes);
  FREE_SAFE(_d->buttons);
  FREE_SAFE(_d);
}

void sgl_joystick_release() {
  pthread_mutex_lock(&devices_mtx);
  joystick_t* current_d = joy_devices.head;
  joystick_t* next_d = current_d;
  while (current_d) {
    next_d = current_d->next;
    release_joystick(&current_d);
    current_d = next_d;
  }
  joy_devices.head = NULL;
  joy_devices.size = 0;
  pthread_mutex_unlock(&devices_mtx);

  pthread_mutex_lock(&event_queue_mtx);
  joystick_queued_event_t* current_e = event_queue.head;
  joystick_queued_event_t* next_e = current_e;
  while (current_e) {
    next_e = current_e->next;
    if (current_e->event != JOY_DEVICE_REMOVED)
      free(current_e->data);
    free(current_e);
    current_e = next_e;
  }
  event_queue.head = NULL;
  event_queue.size = 0;
  pthread_mutex_unlock(&event_queue_mtx);
}

bool sgl_joystick_remove(int id) {
  pthread_mutex_lock(&devices_mtx);
  joystick_t* current = joy_devices.head;
  joystick_t* previous = current;
  while (current) {
    if (current->device_id == id) {
      previous->next = current->next;
      if (current == joy_devices.head)
        joy_devices.head = current->next;
      CALL(joy_removed_callback, current, current->device_id);
      release_joystick(&current);
      return true;
    }
    previous = current;
    current = current->next;
  }
  pthread_mutex_lock(&devices_mtx);
  return false;
}

void sgl_joystick_poll() {
  static bool processing_events = false;
  if (processing_events || !event_queue.size)
    return;

  processing_events = true;
  pthread_mutex_lock(&event_queue_mtx);

  joystick_queued_event_t* current = event_queue.head;
  joystick_queued_event_t* next = current;
  while (current) {
    next = current->next;
    switch(current->event) {
      case JOY_DEVICE_ATTACHED:
        CALL(joy_connect_callback, (joystick_t*)current->data, current->device_id);
        break;
      case JOY_DEVICE_REMOVED: {
        joystick_t* d = (joystick_t*)current->data;
        CALL(joy_removed_callback, d, d->device_id);
        sgl_joystick_remove(d->device_id);
        break;
      }
      case JOY_BTN_DOWN:
      case JOY_BTN_UP: {
        joystick_btn_event_t* e = (joystick_btn_event_t*)current->data;
        CALL(joy_btn_callback, e->device, e->btn_id, e->down, e->timestamp);
        free(e);
        break;
      }
      case JOY_AXIS_MOVED: {
        joystick_axis_event_t* e = (joystick_axis_event_t*)current->data;
        CALL(joy_axis_callback, e->device, e->axis_id, e->value, e->last_value, e->timestamp);
        free(e);
        break;
      }
    }
    free(current);
    current = next;
  }
  event_queue.head = NULL;
  event_queue.size = 0;

  pthread_mutex_unlock(&event_queue_mtx);
  processing_events = false;
}
#endif

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

static Cursor x11_empty_cursor = None;
static Cursor get_x11_empty_cursor() {
  if (x11_empty_cursor == None) {
    char data[1];
    memset(data, 0, sizeof(data));
    XColor color;
    color.red = color.green = color.blue = 0;
    Pixmap pixmap = XCreateBitmapFromData(display, DefaultRootWindow(display), data, 1, 1);
    if (pixmap) {
      x11_empty_cursor = XCreatePixmapCursor(display, pixmap, pixmap, &color, &color, 0, 0);
      XFreePixmap(display, pixmap);
    } else
      error_handle(PRIO_LOW, NIX_CURSOR_PIXMAP_ERROR, "get_x11_empty_cursor() failed: Couldn't create X11 Pixmap");
  }
  return x11_empty_cursor;
}

static void x11_set_cursor(Cursor c) {
  if (!c)
    c = get_x11_empty_cursor();

  XDefineCursor(display, win, c);
  XFlush(display);
}

void sgl_cursor(bool shown, bool locked, CURSORTYPE type) {
  x11_set_cursor(shown ? __cursor : 0);

  unsigned int shape = -1;
  switch (type) {
    default:
    case CURSOR_NO_CHANGE:
      return;
    case CURSOR_ARROW:
      shape = XC_left_ptr;
      break;
    case CURSOR_IBEAM:
      shape = XC_xterm;
      break;
    case CURSOR_WAIT:
    case CURSOR_WAITARROW:
      shape = XC_watch;
      break;
    case CURSOR_CROSSHAIR:
      shape = XC_tcross;
      break;
    case CURSOR_SIZENWSE:
    case CURSOR_SIZENESW:
    case CURSOR_SIZEALL:
      shape = XC_fleur;
      break;
    case CURSOR_SIZEWE:
      shape = XC_sb_h_double_arrow;
      break;
    case CURSOR_SIZENS:
      shape = XC_sb_v_double_arrow;
      break;
    case CURSOR_NO:
      shape = XC_pirate;
      break;
    case CURSOR_HAND:
      shape = XC_hand2;
      break;
    case CURSOR_CUSTOM:
      if (!____custom_cursor) {
        error_handle(PRIO_LOW, CUSTOM_CURSOR_NOT_CREATED, "cursor() failed: No custom cursor loaded");
        return;
      }
  }

  if (__cursor && __cursor != ____custom_cursor)
    XFreeCursor(display, __cursor);
  __cursor = (shape == -1 ? ____custom_cursor : XCreateFontCursor(display, shape));
  if (shown)
    x11_set_cursor(__cursor);
}

void sgl_custom_cursor(surface_t* s) {
#pragma TODO(Add custom cursors for X11)
}

bool sgl_screen(const char* title, surface_t* s, int w, int h, short flags) {
  display = XOpenDisplay(0);
  if (!display) {
    sgl_release();
    error_handle(PRIO_HIGH, NIX_OPEN_DISPLAY_FAILED, "XOpenDisplay(0) failed!");
    return false;
  }

  memset(keycodes, -1, sizeof(keycodes));
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

  int __screen = DefaultScreen(display);
  int s_width = DisplayWidth(display, __screen);
  int s_height = DisplayHeight(display, __screen);

  if (flags & FULLSCREEN)
    flags |= (FULLSCREEN_DESKTOP | BORDERLESS | ~RESIZABLE);

  if (flags & FULLSCREEN_DESKTOP) {
    w = s_width;
    h = s_height;
  }

  int px = s_width / 2 - w / 2;
  int py = s_height / 2 - h / 2;

#if defined(SGL_ENABLE_OPENGL)
  static int visual_attribs[] = {
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DOUBLEBUFFER, False,
    None
  };

  int fb_count;
  GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fb_count);
  if (!fbc) {
    sgl_release();
    error_handle(PRIO_HIGH, NIX_GL_FB_ERROR, "glXChooseFBConfig() failed: Failed to retreive framebuffer config");
    return false;
  }

  int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;
  for (int i = 0; i < fb_count; ++i) {
    XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[i]);
    if (vi != 0) {
      int samp_buf, samples;
      glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
      glXGetFBConfigAttrib(display, fbc[i], GLX_SAMPLE_BUFFERS, &samples);

      if (best_fbc < 0 || (samp_buf && samples > best_num_samp)) {
        best_fbc = i;
        best_num_samp = samples;
      }

      if (worst_fbc < 0 || !samp_buf || samples < worst_num_samp) {
        worst_fbc = i;
        worst_num_samp = samples;
      }
    }
    XFree(vi);
  }
  GLXFBConfig fbc_best = fbc[best_fbc];
  XFree(fbc);

  XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc_best);
  if (!vi) {
    sgl_release();
    error_handle(PRIO_HIGH, NIX_GL_FB_ERROR, "glXGetVisualFromFBConfig() failed: Could not create correct visual window");
    return false;
  }

  XSetWindowAttributes swa;
  swa.override_redirect = True;
  swa.colormap = cmap = XCreateColormap(display, RootWindow(display, vi->screen), vi->visual, AllocNone);
  swa.background_pixmap = None;
  swa.border_pixel = 0;
  swa.event_mask = StructureNotifyMask;

  win = XCreateWindow(display, RootWindow(display, vi->screen),
                      px, py, w, h, 0, vi->depth,
                      InputOutput, vi->visual,
                      CWBorderPixel | CWColormap | CWEventMask, &swa);
#else
  Visual* visual = DefaultVisual(display, __screen);
  int format_c;
  XPixmapFormatValues* formats = XListPixmapFormats(display, &format_c);
  int depth = DefaultDepth(display, __screen);
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
    sgl_release();
    error_handle(PRIO_HIGH, NIX_WINDOW_CREATION_FAILED, "Invalid display depth: %d", c_depth);
    return false;
  }

  XSetWindowAttributes swa;
  swa.override_redirect = True;
  swa.border_pixel = BlackPixel(display, __screen);
  swa.background_pixel = BlackPixel(display, __screen);
  swa.backing_store = NotUseful;

  win = XCreateWindow(display, default_root_win,
                      px, py, w, h, 0, depth,
                      InputOutput, visual,
                      CWBackPixel | CWBorderPixel | CWBackingStore,
                      &swa);
#endif

  if (!win) {
    sgl_release();
    error_handle(PRIO_HIGH, NIX_WINDOW_CREATION_FAILED, "XCreateWindow() failed!");
    return false;
  }

  if (s)
    if (!sgl_surface(s, w, h))
      return false;

  XSelectInput(display, win, StructureNotifyMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask);
  XStoreName(display, win, title);

  if (flags & BORDERLESS) {
    struct StyleHints {
      unsigned long   flags;
      unsigned long   functions;
      unsigned long   decorations;
      long            inputMode;
      unsigned long   status;
    } sh = {
      .flags = 2,
      .decorations = 0
    };
    Atom sh_p = XInternAtom(display, "_MOTIF_WM_HINTS", True);
    XChangeProperty(display, win, sh_p, sh_p, 32, PropModeReplace, (unsigned char*)&sh, 5);
  }

  if (flags & ALWAYS_ON_TOP) {
    Atom sa_p = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
    XChangeProperty(display, win, XInternAtom(display, "_NET_WM_STATE", False), XA_ATOM, 32, PropModeReplace, (unsigned char *)&sa_p, 1);
  }

  if (flags & FULLSCREEN) {
    Atom sf_p = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);
    XChangeProperty(display, win, XInternAtom(display, "_NET_WM_STATE", True), XA_ATOM, 32, PropModeReplace, (unsigned char*)&sf_p, 1);
  }

  XSizeHints hints;
  hints.flags = PPosition;
  hints.x = 0;
  hints.y = 0;
#if !defined(SGL_ENABLE_OPENGL)
  hints.flags |= PMinSize | PMaxSize;
  hints.min_width = w;
  hints.max_width = w;
  hints.min_height = h;
  hints.max_height = h;
#endif

  XSetWMNormalHints(display, win, &hints);
  XClearWindow(display, win);
  XMapRaised(display, win);
  XFlush(display);

  gc = DefaultGC(display, __screen);

#if defined(SGL_ENABLE_OPENGL)
  XFree(vi);

  glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
  const char* glx_exts = glXQueryExtensionsString(display, DefaultScreen(display));
  if (!check_ext(glx_exts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB)
    ctx = glXCreateNewContext(display, fbc_best, GLX_RGBA_TYPE, 0, True);
  else {
    int context_attribs[] = {
      GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
      GLX_CONTEXT_MINOR_VERSION_ARB, 2,
      None
    };
    ctx = glXCreateContextAttribsARB(display, fbc_best, 0, True, context_attribs);
  }
  XSync(display, False);

  if (!ctx) {
    sgl_release();
    error_handle(PRIO_HIGH, NIX_GL_CONTEXT_ERROR, "glXCreateContextAttribsARB() failed: Couldn't create OpenGL context");
    return false;
  }

  glXMakeCurrent(display, win, ctx);
  init_gl(w, h);
#else
  img = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, NULL, w, h, 32, w * 4);
#endif

  __cursor = XCreateFontCursor(display, XC_left_ptr);

  return true;
}

bool sgl_closed() {
  return __closed;
}

void poll() {
  while (XPending(display)) {
    XNextEvent(display, &event);
    switch (event.type) {
      case KeyPress:
        CALL(__kb_callback, translate_key(event.xkey.keycode), translate_mod(event.xkey.state), true);
        break;
      case KeyRelease:
        CALL(__kb_callback, translate_key(event.xkey.keycode), translate_mod(event.xkey.state), false);
        break;
      case ButtonPress: {
        int btn_mod = translate_mod(event.xkey.state);
        switch (event.xbutton.button) {
        case Button1:
          CALL(__mouse_btn_callback, MOUSE_BTN_1, btn_mod, true);
          break;
        case Button2:
          CALL(__mouse_btn_callback, MOUSE_BTN_2, btn_mod, true);
          break;
        case Button3:
          CALL(__mouse_btn_callback, MOUSE_BTN_3, btn_mod, true);
          break;
        case Button4:
          CALL(__scroll_callback, btn_mod, 0.f, 1.f);
          break;
        case Button5:
          CALL(__scroll_callback, btn_mod, 0.f, -1.f);
          break;
        case Button6:
          CALL(__scroll_callback, btn_mod, 1.f, 0.f);
          break;
        case Button7:
          CALL(__scroll_callback, btn_mod, -1.f, 0.f);
          break;
        default:
          CALL(__mouse_btn_callback, (MOUSEBTN)(event.xbutton.button - 4), btn_mod, true);
        }
        break;
      }
      case ButtonRelease: {
        int btn_mod = translate_mod(event.xkey.state);
        switch (event.xbutton.button) {
          case Button1:
            CALL(__mouse_btn_callback, MOUSE_BTN_1, btn_mod, false);
            break;
          case Button2:
            CALL(__mouse_btn_callback, MOUSE_BTN_2, btn_mod, false);
            break;
          case Button3:
            CALL(__mouse_btn_callback, MOUSE_BTN_3, btn_mod, false);
            break;
          default:
            CALL(__mouse_btn_callback, (MOUSEBTN)(event.xbutton.button - 4), btn_mod, false);
        }
        break;
      }
    #if defined(SGL_ENABLE_OPENGL)
      case ConfigureNotify:
        win_w = event.xconfigure.width;
        win_h = event.xconfigure.height;
        glViewport(0, 0, win_w, win_h);
        break;
    #endif
      case MotionNotify:
        CALL(__mouse_move_callback, event.xmotion.x, event.xmotion.y, 0, 0);
        break;
      case DestroyNotify:
        __closed = true;
      default:
        break;
    }
  }
}

void sgl_flush(surface_t* s) {
  if (s && s->buf)
    buffer = s;
#if defined(SGL_ENABLE_OPENGL)
  draw_gl();
  glXSwapBuffers(display, win);
#else
  img->data = (char*)buffer->buf;
  XPutImage(display, win, gc, img, 0, 0, 0, 0, buffer->w, buffer->h);
  XFlush(display);
#endif
}

void sgl_release() {
#if defined(SGL_ENABLE_OPENGL)
  glXMakeCurrent(display, 0, 0);
  glXDestroyContext(display, ctx);
  XFreeColormap(display, cmap);
#else
  img->data = NULL;
  XDestroyImage(img);
#endif
  if (__cursor)
    XFreeCursor(display, __cursor);
  if (____custom_cursor)
    XFreeCursor(display, ____custom_cursor);
  if (x11_empty_cursor)
    XFreeCursor(display, x11_empty_cursor);
  XDestroyWindow(display, win);
  XCloseDisplay(display);
}
#endif
#endif
