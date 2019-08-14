/* graphics.c
 *
 *           _____                     _____                     _____
 *          /\    \                   /\    \                   /\    \
 *         /::\____\                 /::\    \                 /::\____\
 *        /:::/    /                /::::\    \               /:::/    /
 *       /:::/    /                /::::::\    \             /:::/    /
 *      /:::/    /                /:::/\:::\    \           /:::/    /
 *     /:::/____/                /:::/__\:::\    \         /:::/    /
 *    /::::\    \               /::::\   \:::\    \       /:::/    /
 *   /::::::\    \   _____     /::::::\   \:::\    \     /:::/    /
 *  /:::/\:::\    \ /\    \   /:::/\:::\   \:::\    \   /:::/    /
 * /:::/  \:::\    /::\____\ /:::/  \:::\   \:::\____\ /:::/____/
 * \::/    \:::\  /:::/    / \::/    \:::\  /:::/    / \:::\    \
 *  \/____/ \:::\/:::/    /   \/____/ \:::\/:::/    /   \:::\    \
 *           \::::::/    /             \::::::/    /     \:::\    \
 *            \::::/    /               \::::/    /       \:::\    \
 *            /:::/    /                /:::/    /         \:::\    \
 *           /:::/    /                /:::/    /           \:::\    \
 *          /:::/    /                /:::/    /             \:::\    \
 *         /:::/    /                /:::/    /               \:::\____\
 *         \::/    /                 \::/    /                 \::/    /
 *          \/____/                   \/____/                   \/____/
 *
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
 *
 */

#include "graphics.h"

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#if defined(HAL_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

#if defined(_MSC_VER)
#define strdup _strdup
#endif

#define HAL_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define HAL_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define HAL_CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

#if !defined(M_PI)
#define M_PI 3.14159265358979323846264338327950288f
#endif
#define HAL_D2R(a) ((a) * M_PI / 180.0)
#define HAL_R2D(a) ((a) * 180.0 / M_PI)

#define HAL_SAFE_FREE(x) \
  if ((x)) {  \
    HAL_FREE((x)); \
    (x) = NULL; \
  }

static void(*__error_callback)(ERROR_TYPE, const char*, const char*, const char*, i32) = NULL;

void hal_error_callback(void(*cb)(ERROR_TYPE, const char*, const char*, const char*, i32)) {
  __error_callback = cb;
}

static inline void error_handle(ERROR_TYPE type, const char* msg, ...) {
  va_list args;
  va_start(args, msg);

  static i8 error[1024];
  vsprintf(error, msg, args);

#if defined(HAL_DEBUG)
  fprintf(stderr, "[%d] from %s in %s() at %d -- %s\n", (i32)type, __FILE__, __FUNCTION__, __LINE__, error);
#endif
  if (__error_callback)
    __error_callback(type, error, __FILE__, __FUNCTION__, __LINE__);

  va_end(args);
}

struct surface_t {
  i32 *buf, w, h;
};

void hal_surface_size(surface_t s, i32* w, i32* h) {
  if (w)
    *w = s->w;
  if (h)
    *h = s->h;
}

i32* hal_surface_raw(surface_t s) {
  return s->buf;
}

bool hal_surface(surface_t* _s, u32 w, u32 h) {
  surface_t s = *_s = HAL_MALLOC(sizeof(struct surface_t));
  if (!s) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  s->w = w;
  s->h = h;
  size_t sz = w * h * sizeof(u32) + 1;
  s->buf = HAL_MALLOC(sz);
  if (!s->buf) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  memset(s->buf, 0, sz);

  return true;
}

void hal_destroy(surface_t* _s) {
  surface_t s = *_s;
  if (!s)
    return;
  HAL_SAFE_FREE(s->buf);
  HAL_SAFE_FREE(s);
  *_s = NULL;
}

void hal_fill(surface_t s, i32 col) {
  for (i32 i = 0; i < s->w * s->h; ++i)
    s->buf[i] = col;
}

#define XYGET(s, x, y) (s->buf[(y) * s->w + (x)])

static inline void flood_fn(surface_t s, i32 x, i32 y, i32 new, i32 old) {
  if (new == old || XYGET(s, x, y) != old)
    return;

  i32 x1 = x;
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

void hal_flood(surface_t s, i32 x, i32 y, i32 col) {
  if (x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;
  flood_fn(s, x, y, col, XYGET(s, x, y));
}

void hal_cls(surface_t s) {
  memset(s->buf, 0, s->w * s->h * sizeof(i32));
}

void hal_pset(surface_t s, i32 x, i32 y, i32 c) {
  if (x >= 0 && y >= 0 && x < s->w && y < s->h)
    s->buf[y * s->w + x] = c;
}

#if defined(HAL_NO_ALPHA)
#define blend(s, x, y, c) hal_pset((s), (x), (y), (c))
#else
#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

static inline void blend(surface_t s, i32 x, i32 y, i32 c) {
  i32 a = HAL_A(c);
  if (!a || x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;

  i32* p = &s->buf[y * s->w + x];
  i32  b = HAL_A(*p);
  *p = (a == 255 || !b) ? c : HAL_RGBA(BLEND(HAL_R(c), HAL_R(*p), a, b),
                                       BLEND(HAL_G(c), HAL_G(*p), a, b),
                                       BLEND(HAL_B(c), HAL_B(*p), a, b),
                                       a + (b * (255 - a) >> 8));
}
#endif

i32 hal_pget(surface_t s, i32 x, i32 y) {
  return (!s || !s->buf || x < 0 || y < 0 || x >= s->w || y >= s->h ? 0 : XYGET(s, x, y));
}

bool hal_paste(surface_t dst, surface_t src, i32 x, i32 y) {
  if (x > dst->w || x < 0 || y > dst->h || y < 0)
    return false;

  i32 ox, oy, c;
  for (ox = 0; ox < src->w; ++ox) {
    for (oy = 0; oy < src->h; ++oy) {
      if (oy > dst->h)
        break;
      c = XYGET(src, ox, oy);
#if defined(HAL_CHROMA_KEY) && defined(BLIT_CHROMA_KEY)
      if (c == BLIT_CHROMA_KEY)
        continue;
#endif
      blend(dst, x + ox, y + oy, c);
    }
    if (ox > dst->w)
      break;
  }
  return true;
}

bool hal_clip_paste(surface_t dst, surface_t src, i32 x, i32 y, i32 rx, i32 ry, i32 rw, i32 rh) {
  if (rx == rw || ry == rh)
    return false;

  i32 ox = x,  oy = y,
      fx = rx, fy = ry,
      w  = rw, h  = rh;

  if (ox < 0) {
    fx += abs(ox);
    w -= abs(ox);
    ox = 0;
  }
  if (oy < 0) {
    fy += abs(oy);
    h -= abs(oy);
    oy = 0;
  }

  i32 to_x = ox + w, to_y = oy + h;
  if (to_x > dst->w)
    w += (dst->w - to_x);
  if (to_y > dst->h)
    h += (dst->h - to_y);

  if (ox > dst->w || oy > dst->h || to_x < 0 || to_y < 0)
    return false;

  i32 _x, _y, c;
  for (_x = 0; _x < w; ++_x)
    for (_y = 0; _y < h; ++_y) {
      c = XYGET(src, fx + _x, fy + _y);
#if defined(HAL_CHROMA_KEY) && defined(BLIT_CHROMA_KEY)
      if (c == BLIT_CHROMA_KEY)
        continue;
#endif
      blend(dst, ox + _x, oy + _y, c);
    }
  return true;
}

bool hal_reset(surface_t s, i32 nw, i32 nh) {
  size_t sz = nw * nh * sizeof(u32) + 1;
  i32* tmp = HAL_REALLOC(s->buf, sz);
  if (!tmp) {
    error_handle(OUT_OF_MEMEORY, "realloc() failed");
    return false;
  }
  s->buf = tmp;
  s->w = nw;
  s->h = nh;
  memset(s->buf, 0, sz);
  return true;
}

bool hal_copy(surface_t a, surface_t* b) {
  if (!hal_surface(b, a->w, a->h))
    return false;
  surface_t tmp = *b;
  memcpy(tmp->buf, a->buf, a->w * a->h * sizeof(u32) + 1);
  return !!tmp->buf;
}

void hal_passthru(surface_t s, i32 (*fn)(i32 x, i32 y, i32 col)) {
  i32 x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      hal_pset(s, x, y, fn(x, y, XYGET(s, x, y)));
}

bool hal_resize(surface_t a, i32 nw, i32 nh, surface_t* b) {
  if (!hal_surface(b, nw, nh))
    return false;

  surface_t tmp = *b;
  i32 x_ratio = (i32)((a->w << 16) / nw) + 1;
  i32 y_ratio = (i32)((a->h << 16) / nh) + 1;
  i32 x2, y2, i, j;
  for (i = 0; i < nh; ++i) {
    i32* t = tmp->buf + i * nw;
    y2 = ((i * y_ratio) >> 16);
    i32* p = a->buf + y2 * a->w;
    i32 rat = 0;
    for (j = 0; j < nw; ++j) {
      x2 = (rat >> 16);
      *t++ = p[x2];
      rat += x_ratio;
    }
  }
  return true;
}

bool hal_rotate(surface_t a, float angle, surface_t* b) {
  float theta = HAL_D2R(angle);
  float c = cosf(theta), s = sinf(theta);
  float r[3][2] = {
    { -a->h * s, a->h * c },
    {  a->w * c - a->h * s, a->h * c + a->w * s },
    {  a->w * c, a->w * s }
  };

  float mm[2][2] = {{
    HAL_MIN(0, HAL_MIN(r[0][0], HAL_MIN(r[1][0], r[2][0]))),
    HAL_MIN(0, HAL_MIN(r[0][1], HAL_MIN(r[1][1], r[2][1])))
  }, {
    (theta > 1.5708  && theta < 3.14159 ? 0.f : HAL_MAX(r[0][0], HAL_MAX(r[1][0], r[2][0]))),
    (theta > 3.14159 && theta < 4.71239 ? 0.f : HAL_MAX(r[0][1], HAL_MAX(r[1][1], r[2][1])))
  }};

  i32 dw = (i32)ceil(fabsf(mm[1][0]) - mm[0][0]);
  i32 dh = (i32)ceil(fabsf(mm[1][1]) - mm[0][1]);
  if (!hal_surface(b, dw, dh))
    return false;

  i32 x, y, sx, sy;
  for (x = 0; x < dw; ++x)
    for (y = 0; y < dh; ++y) {
      sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
      sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
      if (sx < 0 || sx >= a->w || sy < 0 || sy >= a->h)
        continue;
      blend(*b, x, y, XYGET(a, sx, sy));
    }
  return true;
}

typedef struct oct_node_t {
  i64 r, g, b; /* sum of all child node colors */
  i32 count, heap_idx;
  u8 n_kids, kid_idx, flags, depth;
  struct oct_node_t *kids[8], *parent;
} oct_node_t;

typedef struct {
  i32 alloc, n;
  oct_node_t** buf;
} node_heap;

typedef struct {
  oct_node_t* pool;
  i32 len;
} oct_node_pool_t;

static inline i32 cmp_node(oct_node_t* a, oct_node_t* b) {
  if (a->n_kids < b->n_kids)
    return -1;
  if (a->n_kids > b->n_kids)
    return 1;

  i32 ac = a->count >> a->depth;
  i32 bc = b->count >> b->depth;
  return ac < bc ? -1 : ac > bc;
}

static inline void down_heap(node_heap* h, oct_node_t* p) {
  i32 n = p->heap_idx, m;

  while (1) {
    m = n * 2;
    if (m >= h->n)
      break;
    if (m + 1 < h->n && cmp_node(h->buf[m], h->buf[m + 1]) > 0)
      m++;

    if (cmp_node(p, h->buf[m]) <= 0)
      break;

    h->buf[n] = h->buf[m];
    h->buf[n]->heap_idx = n;
    n = m;
  }

  h->buf[n] = p;
  p->heap_idx = n;
}

static inline void up_heap(node_heap* h, oct_node_t* p) {
  i32 n = p->heap_idx;
  oct_node_t* prev;

  while (n > 1) {
    prev = h->buf[n / 2];
    if (cmp_node(p, prev) >= 0)
      break;

    h->buf[n] = prev;
    prev->heap_idx = n;
    n /= 2;
  }

  h->buf[n] = p;
  p->heap_idx = n;
}

static inline void heap_add(node_heap* h, oct_node_t* p) {
  if ((p->flags & 1)) {
    down_heap(h, p);
    up_heap(h, p);
    return;
  }

  p->flags |= 1;
  if (!h->n) h->n = 1;
  if (h->n >= h->alloc) {
    while (h->n >= h->alloc)
      h->alloc += 1024;
    h->buf = HAL_REALLOC(h->buf, sizeof(oct_node_t*) * h->alloc);
  }

  p->heap_idx = h->n;
  h->buf[h->n++] = p;
  up_heap(h, p);
}

static inline oct_node_t* pop_heap(node_heap* h) {
  if (h->n <= 1)
    return 0;

  oct_node_t* ret = h->buf[1];
  h->buf[1] = h->buf[--h->n];

  h->buf[h->n] = 0;

  h->buf[1]->heap_idx = 1;
  down_heap(h, h->buf[1]);

  return ret;
}

static inline oct_node_t* node_new(oct_node_pool_t* pool, u8 idx, u8 depth, oct_node_t* p) {
  if (pool->len <= 1) {
    oct_node_t* p = HAL_MALLOC(2048 * sizeof(oct_node_t));
    p->parent = pool->pool;
    pool->pool = p;
    pool->len  = 2047;
  }

  oct_node_t* x = pool->pool + pool->len--;
  x->kid_idx = idx;
  x->depth = depth;
  x->parent = p;
  if (p)
    p->n_kids++;
  return x;
}

static inline void free_node(oct_node_pool_t* pool) {
  oct_node_t* p;
  while (pool->pool) {
    p = pool->pool->parent;
    HAL_SAFE_FREE(pool->pool);
    pool->pool = p;
  }
}

static inline oct_node_t* node_insert(oct_node_pool_t* pool, oct_node_t* root, i32* buf) {
  u8 i, bit, depth = 0;
  i32 c = *buf;
  i32 r = HAL_R(c), g = HAL_G(c), b = HAL_B(c);

  for (bit = 1 << 7; ++depth < 8; bit >>= 1) {
    i = !!(r & bit) * 4 + !!(g & bit) * 2 + !!(b & bit);
    if (!root->kids[i])
      root->kids[i] = node_new(pool, i, depth, root);
    root = root->kids[i];
  }

  root->r += r;
  root->g += g;
  root->b += b;;
  root->count++;
  return root;
}

static inline oct_node_t* node_fold(oct_node_t* p) {
  if (p->n_kids) {
    error_handle(INVALID_PARAMETERS, "I don't know to be honest.");
    return NULL;
  }

  oct_node_t* q = p->parent;
  q->count += p->count;

  q->r += p->r;
  q->g += p->g;
  q->b += p->b;
  q->n_kids --;
  q->kids[p->kid_idx] = 0;
  return q;
}

void color_replace(oct_node_t* root, i32* buf) {
  u8 i, bit;
  i32 c = *buf;
  i32 r = HAL_R(c), g = HAL_G(c), b = HAL_B(c);

  for (bit = 1 << 7; bit; bit >>= 1) {
    i = !!(r & bit) * 4 + !!(g & bit) * 2 + !!(b & bit);
    if (!root->kids[i])
      break;
    root = root->kids[i];
  }

  *buf = HAL_RGB((i32)root->r, (i32)root->g, (i32)root->b);
}

void hal_quantize(surface_t a, i32 n_colors, surface_t* b) {
  surface_t tmp = (b ? *b : NULL);
  if (!tmp)
    tmp = a;
  if (tmp && a != tmp) {
    if (tmp->w != a->w || tmp->h != a->h)
      hal_reset(tmp, a->w, a->h);
    if (!tmp->buf)
      hal_surface(b, a->w, a->h);
    hal_copy(a, b);
  }

  i32 i, *buf = a->buf;
  node_heap heap = { 0, 0, 0 };
  oct_node_pool_t pool = { NULL, 0 };

  oct_node_t *root = node_new(&pool, 0, 0, 0), *got;
  for (i = 0, buf = tmp->buf; i < tmp->w * tmp->h; i++, buf++)
    heap_add(&heap, node_insert(&pool, root, buf));

  while (heap.n > n_colors + 1)
    heap_add(&heap, node_fold(pop_heap(&heap)));

  double c;
  for (i = 1; i < heap.n; i++) {
    got = heap.buf[i];
    c = got->count;
    got->r = got->r / c + .5;
    got->g = got->g / c + .5;
    got->b = got->b / c + .5;
  }

  for (i = 0, buf = tmp->buf; i < tmp->w * tmp->h; i++, buf++)
    color_replace(root, buf);

  free_node(&pool);
  HAL_SAFE_FREE(heap.buf);
}

static inline void hal_vline(surface_t s, i32 x, i32 y0, i32 y1, i32 col) {
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

  for(i32 y = y0; y <= y1; y++)
    blend(s, x, y, col);
}

static inline void hal_hline(surface_t s, i32 y, i32 x0, i32 x1, i32 col) {
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

  for(i32 x = x0; x <= x1; x++)
    blend(s, x, y, col);
}

void hal_line(surface_t s, i32 x0, i32 y0, i32 x1, i32 y1, i32 col) {
  if (x0 == x1)
    hal_vline(s, x0, y0, y1, col);
  if (y0 == y1)
    hal_hline(s, y0, x0, x1, col);

  i32 dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  i32 dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  i32 err = (dx > dy ? dx : -dy) / 2;

  while (blend(s, x0, y0, col), x0 != x1 || y0 != y1) {
    i32 e2 = err;
    if (e2 > -dx) { err -= dy; x0 += sx; }
    if (e2 <  dy) { err += dx; y0 += sy; }
  }
}

void hal_circle(surface_t s, i32 xc, i32 yc, i32 r, i32 col, bool fill) {
  i32 x = -r, y = 0, err = 2 - 2 * r; /* II. Quadrant */
  do {
    blend(s, xc - x, yc + y, col);    /*   I. Quadrant */
    blend(s, xc - y, yc - x, col);    /*  II. Quadrant */
    blend(s, xc + x, yc - y, col);    /* III. Quadrant */
    blend(s, xc + y, yc + x, col);    /*  IV. Quadrant */

    if (fill) {
      hal_hline(s, yc - y, xc - x, xc + x, col);
      hal_hline(s, yc + y, xc - x, xc + x, col);
    }

    r = err;
    if (r <= y)
      err += ++y * 2 + 1; /* e_xy+e_y < 0 */
    if (r > x || err > y)
      err += ++x * 2 + 1; /* e_xy+e_x > 0 or no 2nd y-step */
  } while (x < 0);
}

void hal_rect(surface_t s, i32 x, i32 y, i32 w, i32 h, i32 col, bool fill) {
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
      hal_hline(s, y, x, w, col);
  } else {
    hal_hline(s, y, x, w, col);
    hal_hline(s, h, x, w, col);
    hal_vline(s, x, y, h, col);
    hal_vline(s, w, y, h, col);
  }
}

#define HAL_SWAP(a, b) \
  do {                 \
    i32 temp = a;      \
    a = b;             \
    b = temp;          \
  } while(0)

void hal_tri(surface_t s, i32 x0, i32 y0, i32 x1, i32 y1, i32 x2, i32 y2, i32 col, bool fill) {
  if (y0 ==  y1 && y0 ==  y2)
    return;
  if (fill) {
    if (y0 > y1) {
      HAL_SWAP(x0, x1);
      HAL_SWAP(y0, y1);
    }
    if (y0 > y2) {
      HAL_SWAP(x0, x2);
      HAL_SWAP(y0, y2);
    }
    if (y1 > y2) {
      HAL_SWAP(x1, x2);
      HAL_SWAP(y1, y2);
    }

    i32 total_height = y2 - y0, i, j;
    for (i = 0; i < total_height; ++i) {
      bool second_half = i > y1 - y0 || y1 == y0;
      i32 segment_height = second_half ? y2 - y1 : y1 - y0;
      float alpha = (float)i / total_height;
      float beta  = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;
      i32 ax = x0 + (x2 - x0) * alpha;
      i32 ay = y0 + (y2 - y0) * alpha;
      i32 bx = second_half ? x1 + (x2 - x1) : x0 + (x1 - x0) * beta;
      i32 by = second_half ? y1 + (y2 - y1) : y0 + (y1 - y0) * beta;
      if (ax > bx) {
        HAL_SWAP(ax, bx);
        HAL_SWAP(ay, by);
      }
      for (j = ax; j <= bx; ++j)
        blend(s, j, y0 + i, col);
    }
  } else {
    hal_line(s, x0, y0, x1, y1, col);
    hal_line(s, x1, y1, x2, y2, col);
    hal_line(s, x2, y2, x0, y0, col);
  }
}

#if defined(_MSC_VER)
#define ALIGN_STRUCT(x) __declspec(align(x))
#else
#define ALIGN_STRUCT(x) __attribute__((aligned(x)))
#endif

#pragma pack(1)
typedef struct {
  u16 type; /* Magic identifier */
  u32 size; /* File size in bytes */
  u32 reserved;
  u32 offset; /* Offset to image data, bytes */
} ALIGN_STRUCT(2) BMPHEADER;
#pragma pack()

typedef struct {
  u32 size; /* Header size in bytes */
  i32 width, height; /* Width and height of image */
  u16 planes; /* Number of colour planes */
  u16 bits; /* Bits per pixel */
  u32 compression; /* Compression type */
  u32 image_size; /* Image size in bytes */
  i32 xresolution, yresolution; /* Pixels per meter */
  u32 ncolours; /* Number of colours */
  u32 important_colours; /* Important colours */
} BMPINFOHEADER;

typedef struct {
  u32 size; /* size of bitmap core header */
  u16 width; /* image with */
  u16 height; /* image height */
  u16 planes; /* must be equal to 1 */
  u16 count; /* bits per pixel */
} BMPCOREHEADER;

#define BMP_GET(d, b, s) \
  memcpy(d, b + off, s); \
  off += s;

bool hal_bmp(surface_t* s, const char* path) {
  FILE* fp = fopen(path, "rb");
  if (!fp) {
    error_handle(FILE_OPEN_FAILED, "fopen() failed: %s", path);
    return false;
  }

  fseek(fp, 0, SEEK_END);
  size_t length = ftell(fp);
  rewind(fp);

  u8* data = HAL_MALLOC((length + 1) * sizeof(u8));
  if (!data) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  fread(data, 1, length, fp);
  fclose(fp);

  i32 off = 0;
  BMPHEADER header;
  BMPINFOHEADER info;
  //BMPCOREHEADER core;
  BMP_GET(&header, data, sizeof(BMPHEADER));
  //i32 info_pos = off;
  BMP_GET(&info, data, sizeof(BMPINFOHEADER));

  if (header.type != 0x4D42) {
    error_handle(INVALID_BMP, "bmp() failed: invalid BMP signiture '%d'", header.type);
    return false;
  }

#pragma message WARN("TODO: hal_bmp() add support for OS/2 bitmaps")

  u8* color_map = NULL;
  i32 color_map_size = 0;
  if (info.bits <= 8) {
    color_map_size = (1 << info.bits) * 4;
    color_map = HAL_MALLOC(color_map_size * sizeof(u8));
    if (!color_map) {
      error_handle(OUT_OF_MEMEORY, "malloc() failed");
      return false;
    }
    BMP_GET(color_map, data, color_map_size);
  }

  if (!hal_surface(s, info.width, info.height)) {
    HAL_SAFE_FREE(color_map);
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  off = header.offset;
  i32 i, p, x, y, sz = info.width * info.height;
  u8 color;
  switch (info.compression) {
    case 0: // RGB
      switch (info.bits) { // BPP
        case 1:
        case 4:
        case 8:
#pragma message WARN("TODO: hal_bmp() add 1, 4 & 8 bpp support")
          error_handle(UNSUPPORTED_BMP, "bmp() failed. Unsupported BPP: %d", info.bits);
          hal_destroy(s);
          break;
        case 24:
        case 32:
          p = (info.bits == 32 ? 4 : 3);
          for (y = 0; y < info.height; ++y) {
            for (x = 0; x < info.width; ++x) {
              i = (x + y * info.width) * p;
              (*s)->buf[(info.height - y) * info.width + x] = HAL_RGBA(data[off + i + 2], data[off + i + 1], data[off + i], (p == 3 ? 255 : data[off + i + 3]));
            }
            off += ((4 - (info.width * p) % 4) % 4);
          }
          break;
        default:
          error_handle(UNSUPPORTED_BMP, "bmp() failed. Unsupported BPP: %d", info.bits);
          HAL_SAFE_FREE(color_map);
          hal_destroy(s);
          return false;
      }
      break;
    case 1: // RLE8
    case 2: // RLE4
    default:
#pragma message WARN("TODO: hal_bmp() add RLE support")
      error_handle(UNSUPPORTED_BMP, "bmp() failed. Unsupported compression: %d", info.compression);
      HAL_SAFE_FREE(color_map);
      hal_destroy(s);
      return false;
  }

  HAL_SAFE_FREE(color_map);
  return true;
}

bool hal_save_bmp(surface_t s, const char* path) {
  const i32 filesize = 54 + 3 * s->w * s->h;
  u8* img = HAL_MALLOC(3 * s->w * s->h);
  if (!img) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  memset(img, 0, 3 * s->w * s->h);

  i32 i, j, y, c;
  for (i = 0; i < s->w; ++i) {
    for (j = s->h; j > 0; --j) {
      y = (s->h - 1) - j;
      c = XYGET(s, i, y);
      img[(i + y * s->w) * 3 + 2] = (u8)HAL_R(c);
      img[(i + y * s->w) * 3 + 1] = (u8)HAL_G(c);
      img[(i + y * s->w) * 3 + 0] = (u8)HAL_B(c);
    }
  }

  u8 header[14] = {
    'B', 'M',
    0,  0, 0, 0,
    0,  0,
    0,  0,
    54, 0, 0, 0
  };
  u8 info[40] = {
    40, 0, 0, 0,
    0,  0, 0, 0,
    0,  0, 0, 0,
    1,  0,
    24, 0
  };
  u8 pad[3] = { 0, 0, 0 };

  header[2]  = (u8)(filesize);
  header[3]  = (u8)(filesize >> 8);
  header[4]  = (u8)(filesize >> 16);
  header[5]  = (u8)(filesize >> 24);

  info[4]  = (u8)(s->w);
  info[5]  = (u8)(s->w >> 8);
  info[6]  = (u8)(s->w >> 16);
  info[7]  = (u8)(s->w >> 24);
  info[8]  = (u8)(s->h);
  info[9]  = (u8)(s->h >> 8);
  info[10] = (u8)(s->h >> 16);
  info[11] = (u8)(s->h >> 24);

  FILE* fp = fopen(path, "wb");
  if (!fp) {
    error_handle(FILE_OPEN_FAILED, "fopen() failed: %s", path);
    HAL_SAFE_FREE(img);
    return false;
  }

  fwrite(header, 1, 14, fp);
  fwrite(info, 1, 40, fp);
  for(i = 0; i < s->h; ++i) {
    fwrite(img + (s->w * (s->h - i - 1) * 3), 3, s->w, fp);
    fwrite(pad, 1, (4 - (s->w * 3) % 4) % 4,fp);
  }

  HAL_SAFE_FREE(img);
  fclose(fp);
  return true;
}

#if !defined(HAL_NO_TEXT)
static i8 font[540][8] = {
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

static i32 extra_font_lookup[10] = {
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

#if !defined(HAL_NO_TEXT_COLOR_PARSING)
static inline i32 read_color(const char* str, i32* col, i32* len) {
#pragma message WARN("TODO: read_color() alpha value wrong?")
  const char* c = str;
  if (*c != '(')
    return 0;
  i32 _len = 0;
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
  i8 rgba[4][4] = {
    "0",
    "0",
    "0",
    "255"
  };
  i32 n = 0;
  for (i32 i = 0, j = 0, k = 0; i < (_len - 2); ++i) {
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

  *col = HAL_RGBA(atoi(rgba[0]), atoi(rgba[1]), atoi(rgba[2]), atoi(rgba[3]));
  return 1;
}
#endif // !defined(HAL_NO_TEXT_COLOR_PARSING)

static inline void str_size(const char* str, i32* w, i32* h) {
  const char* s = (const char*)str;
  i32 n = 0, m = 0, l = 1, c, len;
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
  *w = HAL_MAX(n, m);
  *h = l;
}

i32 ctoi(const char* c, i32* out) {
  i32 u = *c, l = 1;
  if ((u & 0xC0) == 0xC0) {
    i32 a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;
    if (a < 6 || !(u & 0x02)) {
      u = ((u << (a + 1)) & 0xFF) >> (a + 1);
      for (i32 b = 1; b < a; ++b)
        u = (u << 6) | (c[l++] & 0x3F);
    }
  }
  *out = u;
  return l;
}

#define LINE_HEIGHT 10

static inline i32 letter_index(i32 c) {
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
    for (i32 i = 0; i < 10; ++i)
      if (extra_font_lookup[i] == c)
        return 530 + i;
    return 0;
  }
}

void hal_ascii(surface_t s, i8 ch, i32 x, i32 y, i32 fg, i32 bg) {
  i32 c = letter_index((i32)ch), i, j;
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j) {
      if (font[c][i] & 1 << j) {
        blend(s, x + j, y + i, fg);
      } else {
        if (bg == -1)
          continue;
        blend(s, x + j, y + i, bg);
      }
    }
  }
}

i32 hal_character(surface_t s, const char* ch, i32 x, i32 y, i32 fg, i32 bg) {
  i32 u = -1;
  i32 l = ctoi(ch, &u);
  i32 uc = letter_index(u), i, j;
  for (i = 0; i < 8; ++i)
    for (j = 0; j < 8; ++j) {
      if (font[uc][i] & 1 << j)
        blend(s, x + j, y + i, fg);
      else {
        if (bg == -1)
          continue;
        blend(s, x + j, y + i, bg);
      }
    }

  return l;
}

void hal_writeln(surface_t s, i32 x, i32 y, i32 fg, i32 bg, const char* str) {
  const char* c = str;
  i32 u = x, v = y, col, len;
  while (c && *c != '\0')
    switch (*c) {
      case '\n':
        v += LINE_HEIGHT;
        u  = x;
        c++;
        break;
#if !defined(HAL_NO_TEXT_COLOR_PARSING)
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
#endif
      default:
        c += hal_character(s, c, u, v, fg, bg);
        u += 8;
        break;
    }
}

void hal_writelnf(surface_t s, i32 x, i32 y, i32 fg, i32 bg, const char* fmt, ...) {
  i8* buffer = NULL;
  i32 buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  i32 length = vsnprintf(buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = HAL_REALLOC(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf(buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  hal_writeln(s, x, y, fg, bg, buffer);
  HAL_SAFE_FREE(buffer);
}

void hal_string(surface_t* s, i32 fg, i32 bg, const char* str) {
  i32 w, h;
  str_size(str, &w, &h);
  hal_surface(s, w * 8, h * LINE_HEIGHT);
  hal_fill(*s, (bg == -1 ? 0 : bg));
  hal_writeln(*s, 0, 0, fg, bg, str);
}

void hal_stringf(surface_t* s, i32 fg, i32 bg, const char* fmt, ...) {
  i8* buffer = NULL;
  i32 buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  i32 length = vsnprintf(buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = HAL_REALLOC(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf(buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  hal_string(s, fg, bg, buffer);
  HAL_SAFE_FREE(buffer);
}
#endif // !defined(HAL_NO_TEXT)

static i32 ticks_started = 0;

i64 hal_ticks() {
#if defined(HAL_WINDOWS)
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

void hal_delay(i64 ms) {
#if defined(HAL_WINDOWS)
  Sleep((DWORD)ms);
#else
  usleep((u32)(ms * 1000));
#endif
}

#if defined(HAL_BDF) && !defined(HAL_NO_TEXT)
#if defined(HAL_WINDOWS)
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

#define BDF_READ_INT(x) \
  p = strtok(NULL, " \t\n\r"); \
  (x) = atoi(p);

typedef struct {
  u32 width;
  u8* bitmap;
  rect_t bb;
} bdf_char_t;

struct bdf_t {
  rect_t fontbb;
  u32* encoding_table;
  bdf_char_t* chars;
  i32 n_chars;
};

void hal_bdf_destroy(struct bdf_t** _f) {
  if (!*_f)
    return;

  struct bdf_t* f = *_f;
  for (i32 i = 0; i < f->n_chars; ++i)
    if (f->chars[i].bitmap) {
      HAL_SAFE_FREE(f->chars[i].bitmap);
      f->chars[i].bitmap = NULL;
    }
  if (f->chars) {
    HAL_SAFE_FREE(f->chars);
    f->chars = NULL;
  }
  if (f->encoding_table) {
    HAL_SAFE_FREE(f->encoding_table);
    f->encoding_table = NULL;
  }
  f->n_chars = 0;
  memset(&f->fontbb, 0, sizeof(rect_t));
  HAL_SAFE_FREE(f);
}

static inline i32 htoi(const char* p) {
  return (*p <= '9' ? *p - '0' : (*p <= 'F' ? *p - 'A' + 10 : *p - 'a' + 10));
}

bool hal_bdf(struct bdf_t** _out, const char* path) {
  struct bdf_t* out = *_out = HAL_MALLOC(sizeof(struct bdf_t));
  if (!out) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  FILE* fp = fopen(path, "r");
  if (!fp) {
    error_handle(FILE_OPEN_FAILED, "fopen() failed: %s", path);
    return false;
  }

  i8 *s, *p, buf[BUFSIZ];
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
    error_handle(BDF_NO_CHAR_SIZE, "bdf() failed: No character size given for %s", path);
    return false;
  }

  if (out->n_chars <= 0) {
    error_handle(BDF_NO_CHAR_LENGTH, "bdf() failed: Unknown number of characters for %s", path);
    return false;
  }

  out->encoding_table = HAL_MALLOC(out->n_chars * sizeof(u32));
  if (!out->encoding_table) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  out->chars = HAL_MALLOC(out->n_chars * sizeof(bdf_char_t));
  if (!out->chars) {
    HAL_SAFE_FREE(out->encoding_table);
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }

  i32 encoding = 0, width = -1, scanline = -1, i, j, n = 0;
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
        hal_bdf_destroy(_out);
        error_handle(BDF_TOO_MANY_BITMAPS, "bdf() failed: More bitmaps than characters for %s", path);
        return false;
      }
      if (width == -1) {
        hal_bdf_destroy(_out);
        error_handle(BDF_UNKNOWN_CHAR, "bdf() failed: Unknown character with for %s", path);
        return false;
      }

      if (out->chars[n].bb.x < 0) {
        width -= out->chars[n].bb.x;
        out->chars[n].bb.x = 0;
      }
      if (out->chars[n].bb.x + out->chars[n].bb.w > width)
        width = out->chars[n].bb.x + out->chars[n].bb.w;

      out->chars[n].bitmap = HAL_MALLOC(((out->fontbb.w + 7) / 8) * out->fontbb.h * sizeof(u8));
      if (!out->chars[n].bitmap) {
        hal_bdf_destroy(_out);
        error_handle(OUT_OF_MEMEORY, "malloc() failed");
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

        i32 x, y, c, o;
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

i32 hal_bdf_character(surface_t s, struct bdf_t* f, const char* ch, i32 x, i32 y, i32 fg, i32 bg) {
  i32 u = -1, i, j, n;
  i32 l = ctoi(ch, &u);
  for (i = 0; i < f->n_chars; ++i)
    if (f->encoding_table[i] == u) {
      n = i;
      break;
    }

  i32 yoffset = f->fontbb.h - f->chars[n].bb.h + (f->fontbb.y - f->chars[n].bb.y), xx, yy, cc;
  for (yy = 0; yy < f->fontbb.h; ++yy) {
    for (xx = 0; xx < f->fontbb.w; xx += 8) {
      cc = (yy < yoffset || yy > yoffset + f->chars[n].bb.h ? 0 : f->chars[n].bitmap[(yy - yoffset) * ((f->fontbb.w + 7) / 8) + xx / 8]);

      for (i = 128, j = 0; i; i /= 2, ++j)
        hal_psetb(s, x + j, y + yy, (cc & i ? fg : bg));
    }
  }

  return l;
}

void hal_bdf_writeln(surface_t s, struct bdf_t* f, i32 x, i32 y, i32 fg, i32 bg, const char* str) {
  const char* c = (const char*)str;
  i32 u = x, v = y, col, len;
  while (c && *c != '\0') {
    switch (*c) {
      case '\n':
        v += f->fontbb.h + 2;
        u = x;
        c++;
        break;
#if !defined(HAL_NO_TEXT_COLOR_PARSING)
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
#endif
      default:
        c += hal_bdf_character(s, f, c, u, v, fg, bg);
        u += 8;
        break;
    }
  }
}

void hal_bdf_writelnf(surface_t s, struct bdf_t* f, i32 x, i32 y, i32 fg, i32 bg, const char* fmt, ...) {
  i8* buffer = NULL;
  i32 buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  i32 length = vsnprintf(buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = HAL_REALLOC(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf(buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  hal_bdf_writeln(s, f, x, y, fg, bg, buffer);
  HAL_SAFE_FREE(buffer);
}

void hal_bdf_string(surface_t* s, struct bdf_t* f, i32 fg, i32 bg, const char* str) {
  i32 w, h;
  str_size(str, &w, &h);
  hal_surface(s, w * 8, h * LINE_HEIGHT);
  hal_fill(*s, (bg == -1 ? 0 : bg));
  hal_bdf_writeln(*s, f, 0, 0, fg, bg, str);
}

void hal_bdf_stringf(surface_t* s, struct bdf_t* f, i32 fg, i32 bg, const char* fmt, ...) {
  i8* buffer = NULL;
  i32 buffer_size = 0;

  va_list argptr;
  va_start(argptr, fmt);
  i32 length = vsnprintf(buffer, buffer_size, fmt, argptr);
  va_end(argptr);

  if (length + 1 > buffer_size) {
    buffer_size = length + 1;
    buffer = HAL_REALLOC(buffer, buffer_size);
    va_start(argptr, fmt);
    vsnprintf(buffer, buffer_size, fmt, argptr);
    va_end(argptr);
  }

  hal_bdf_string(s, f, fg, bg, buffer);
  HAL_SAFE_FREE(buffer);
}
#endif // HAL_BDF

#if defined(HAL_GIF)
#include <stdint.h>
#ifndef GIF_MGET
#define GIF_MGET(m,s,a,c) m = HAL_REALLOC((c)? 0 : m, (c)? s : 0UL);
#endif
#ifndef GIF_BIGE
#define GIF_BIGE 0
#endif
#define _GIF_SWAP(h) ((GIF_BIGE)? ((u16)(h << 8) | (h >> 8)) : h)

struct gif_t {
  i32 delay, frames, frame, w, h;
  surface_t* surfaces;
};

i32 hal_gif_delay(gif_t g) {
  return g->delay;
}

i32 hal_gif_total_frames(gif_t g) {
  return g->frames;
}

i32 hal_gif_current_frame(gif_t g) {
  return g->frame;
}

void hal_gif_size(gif_t g, i32* w, i32* h) {
  if (w)
    *w = g->w;
  if (h)
    *h = g->h;
}

i32 hal_gif_next_frame(gif_t g) {
  g->frame++;
  if (g->frame >= g->frames)
    g->frame = 0;
  return g->frame;
}

void hal_gif_set_frame(gif_t g, i32 n) {
  if (n >= g->frames || n < 0)
    return;
  g->frame = n;
}

surface_t hal_gif_frame(gif_t g) {
  return g->surfaces[g->frame];
}

void hal_gif_create(gif_t* _g, i32 w, i32 h, i32 delay, i32 frames, ...) {
  gif_t g = *_g = HAL_MALLOC(sizeof(gif_t));
  if (!g) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    *_g = NULL;
    return;
  }
  g->delay = delay;
  g->frame = 0;
  g->frames = frames;
  g->h = h;
  g->w = w;
  g->surfaces = HAL_MALLOC(g->frames * sizeof(surface_t));
  if (!g->surfaces) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    HAL_SAFE_FREE(g);
    *_g = NULL;
    return;
  }

  va_list ap;
  va_start(ap, frames);
  for (i32 i = 0; i < frames; ++i) {
    surface_t tmp = va_arg(ap, surface_t);
    hal_copy(tmp, &g->surfaces[i]);
  }
  va_end(ap);
}

#pragma pack(push, 1)
struct GIF_WHDR {              /** ======== frame writer info: ======== **/
  i64 xdim, ydim, clrs,        /** global dimensions, palette size      **/
      bkgd, tran,              /** background index, transparent index  **/
      intr, mode,              /** interlace flag, frame blending mode  **/
      frxd, fryd, frxo, fryo,  /** current frame dimensions and offset  **/
      time, ifrm, nfrm;        /** delay, frame number, frame count     **/
  u8 *bptr;                    /** frame pixel indices or metadata      **/
  struct {                     /** [==== GIF RGB palette element: ====] **/
    u8 R, G, B;                /** [color values - red, green, blue   ] **/
  } *cpal;                     /** current palette                      **/
};
#pragma pack(pop)

enum {GIF_NONE = 0, GIF_CURR = 1, GIF_BKGD = 2, GIF_PREV = 3};

static inline i64 _GIF_SkipChunk(u8 **buff, i64 size) {
  i64 skip;

  for (skip = 2, ++size, ++(*buff); ((size -= skip) > 0) && (skip > 1);
      *buff += (skip = 1 + **buff));
  return size;
}

static inline i64 _GIF_LoadHeader(unsigned gflg, u8 **buff, void **rpal,
    unsigned fflg, i64 *size, i64 flen) {
  if (flen && (!(*buff += flen) || ((*size -= flen) <= 0)))
    return -2;        /** v--[ 0x80: "palette is present" flag ]--, **/
  if (flen && (fflg & 0x80)) {     /** local palette has priority | **/
    *rpal = *buff;            /** [ 3L: 3 u8 color channels ]--,  | **/
    *buff += (flen = 2 << (fflg & 7)) * 3L;             /** <--|  | **/
    return ((*size -= flen * 3L) > 0)? flen : -1;       /** <--'  | **/
  }     /** no local palette found, checking for the global one   | **/
  return (gflg & 0x80)? (2 << (gflg & 7)) : 0;          /** <-----' **/
}

static inline i64 _GIF_LoadFrame(u8 **buff, i64 *size,
    u8 *bptr, u8 *blen) {
  typedef u16 GIF_H;
  const i64 GIF_HLEN = sizeof(GIF_H),  /** to rid the scope of sizeof **/
        GIF_CLEN = 1 << 12;         /** code table length: 4096 items **/
  GIF_H accu, mask; /** bit accumulator / bit mask                    **/
  i64  ctbl, iter,  /** last code table index / index string iterator **/
       prev, curr,  /** codes from the stream: previous / current     **/
       ctsz, ccsz,  /** code table bit sizes: min LZW / current       **/
       bseq, bszc;  /** counters: block sequence / bit size           **/
  u32 *code = (u32*)bptr - GIF_CLEN;           /** code table pointer **/

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
            code[ctbl] = (u32)prev + (code[prev] & 0xFFF000);
          } /** appending SP / MP decoded pixels to the frame **/
          prev = (i64)code[iter = (ctbl > curr)? curr : prev];
          if ((bptr += (prev = (prev >> 12) & 0xFFF)) > blen)
            continue; /** skipping pixels above frame capacity **/
          for (prev++; (iter &= 0xFFF) >> ctsz;
              *bptr-- = (u8)((iter = (i64)code[iter]) >> 24));
          (bptr += prev)[-prev] = (u8)iter;
          if (ctbl < GIF_CLEN) { /** appending the code table **/
            if (ctbl == curr)
              *bptr++ = (u8)iter;
            else if (ctbl < curr)
              return -5; /** wrong code in the stream **/
            code[ctbl++] += ((u32)iter << 24) + 0x1000;
          }
        } /** 0: no ED before end-of-stream mark; -4: see above **/
  return (++(*size) >= 0)? 0 : -4; /** ^- N.B.: 0 error is recoverable **/
}

static inline i64 GIF_Load(void *data, i64 size,
    void (*gwfr)(void*, struct GIF_WHDR*),
    void (*eamf)(void*, struct GIF_WHDR*),
    void *anim, i64 skip) {
  const i64    GIF_BLEN = (1 << 12) * sizeof(u32);
  const u8 GIF_EHDM = 0x21, /** extension header mark **/
        GIF_FHDM = 0x2C,    /** frame header mark **/
        GIF_EOFM = 0x3B,    /** end-of-file mark **/
        GIF_EGCM = 0xF9,    /** extension: graphics control mark **/
        GIF_EAMM = 0xFF;    /** extension: app metadata mark **/
#pragma pack(push, 1)
  struct GIF_GHDR {            /** ========== GLOBAL GIF HEADER: ========== **/
    u8 head[6];                /** 'GIF87a' / 'GIF89a' header signature     **/
    u16 xdim, ydim;            /** total image width, total image height    **/
    u8 flgs;                   /** FLAGS:
                                   GlobalPlt    bit 7     1: global palette exists
                                   0: local in each frame
                                   ClrRes       bit 6-4   bits/channel = ClrRes+1
                                   [reserved]   bit 3     0
                                   PixelBits    bit 2-0   |Plt| = 2 * 2^PixelBits **/
    u8 bkgd, aspr;             /** background color index, aspect ratio     **/
  } *ghdr = (struct GIF_GHDR*)data;
  struct GIF_FHDR {            /** ======= GIF FRAME MASTER HEADER: ======= **/
    u16 frxo, fryo;            /** offset of this frame in a "full" image   **/
    u16 frxd, fryd;            /** frame width, frame height                **/
    u8 flgs;                   /** FLAGS:
                                   LocalPlt     bit 7     1: local palette exists
                                   0: global is used
                                   Interlaced   bit 6     1: interlaced frame
                                   0: non-interlaced frame
                                   Sorted       bit 5     usually 0
                                   [reserved]   bit 4-3   [undefined]
                                   PixelBits    bit 2-0   |Plt| = 2 * 2^PixelBits **/
  } *fhdr;
  struct GIF_EGCH {             /** ==== [EXT] GRAPHICS CONTROL HEADER: ==== **/
    u8 flgs;                    /** FLAGS:
                                    [reserved]   bit 7-5   [undefined]
                                    BlendMode    bit 4-2   000: not set; static GIF
                                    001: leave result as is
                                    010: restore background
                                    011: restore previous
                                    1--: [undefined]
                                    UserInput    bit 1     1: show frame till input
                                    0: default; ~99% of GIFs
                                    TransColor   bit 0     1: got transparent color
                                    0: frame is fully opaque **/
    u16 time;                    /** delay in GIF time units; 1 unit = 10 ms  **/
    u8 tran;                     /** transparent color index                  **/
  } *egch = 0;
#pragma pack(pop)
  struct GIF_WHDR wtmp, whdr = {0};
  i64 desc, blen;
  u8 *buff;

  /** checking if the stream is not empty and has a 'GIF8[79]a' signature,
    the data has sufficient size and frameskip value is non-negative **/
  if (!ghdr || (size <= (i64)sizeof(*ghdr)) || (*(buff = ghdr->head) != 71)
      || (buff[1] != 73) || (buff[2] != 70) || (buff[3] != 56) || (skip < 0)
      || ((buff[4] != 55) && (buff[4] != 57)) || (buff[5] != 97) || !gwfr)
    return 0;

  buff = (u8*)(ghdr + 1) /** skipping the global header and palette **/
    + _GIF_LoadHeader(ghdr->flgs, 0, 0, 0, 0, 0L) * 3L;
  if ((size -= buff - (u8*)ghdr) <= 0)
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
  blen = whdr.frxo * whdr.fryo * (i64)sizeof(*whdr.bptr);
  GIF_MGET(whdr.bptr, (u64)(blen + GIF_BLEN + 2), anim, 1)
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
  GIF_MGET(whdr.bptr, (u64)(blen + GIF_BLEN + 2), anim, 0)
    return (whdr.nfrm < 0)? (skip - whdr.ifrm - 1) : (whdr.ifrm + 1);
}

#undef _GIF_SWAP

#pragma pack(push, 1)
typedef struct {
  void *data, *pict, *prev;
  u64 size, last;
  gif_t out;
} gif_data_t;
#pragma pack(pop)

void load_gif_frame(void* data, struct GIF_WHDR* whdr) {
  u32 *pict, *prev, x, y, yoff, iter, ifin, dsrc, ddst;
  gif_data_t* gif = (gif_data_t*)data;

#define BGRA(i) \
  ((whdr->bptr[i] == whdr->tran)? 0 : \
   ((u32)(whdr->cpal[whdr->bptr[i]].R << ((GIF_BIGE)? 8 : 16)) \
|  (u32)(whdr->cpal[whdr->bptr[i]].G << ((GIF_BIGE)? 16 : 8))  \
|  (u32)(whdr->cpal[whdr->bptr[i]].B << ((GIF_BIGE)? 24 : 0))  \
|  ((GIF_BIGE)? 0xFF : 0xFF000000)))

  if (!whdr->ifrm) {
    gif->out->delay = (i32)whdr->time;
    gif->out->w = (i32)whdr->xdim;
    gif->out->h = (i32)whdr->ydim;
    gif->out->frames = (i32)whdr->nfrm;
    gif->out->frame  = 0;
    ddst = (u32)(whdr->xdim * whdr->ydim);
    gif->pict = HAL_MALLOC(ddst * sizeof(u32));
    gif->prev = HAL_MALLOC(ddst * sizeof(u32));
    gif->out->surfaces = HAL_MALLOC(gif->out->frames * sizeof(surface_t));
  }

  pict = (u32*)gif->pict;
  ddst = (u32)(whdr->xdim * whdr->fryo + whdr->frxo);
  ifin = (!(iter = (whdr->intr)? 0 : 4))? 4 : 5; /** interlacing support **/
  for (dsrc = (u32)-1; iter < ifin; iter++)
    for (yoff = 16U >> ((iter > 1)? iter : 1), y = (8 >> iter) & 7;
        y < (u32)whdr->fryd; y += yoff)
      for (x = 0; x < (u32)whdr->frxd; x++)
        if (whdr->tran != (i64)whdr->bptr[++dsrc])
          pict[(u32)whdr->xdim * y + x + ddst] = BGRA(dsrc);

  i32 this = (i32)whdr->ifrm;
  hal_surface(&gif->out->surfaces[this], gif->out->w, gif->out->h);
  memcpy(gif->out->surfaces[this]->buf, pict, whdr->xdim * whdr->ydim * sizeof(u32));

  if ((whdr->mode == GIF_PREV) && !gif->last) {
    whdr->frxd = whdr->xdim;
    whdr->fryd = whdr->ydim;
    whdr->mode = GIF_BKGD;
    ddst = 0;
  }
  else {
    gif->last = (whdr->mode == GIF_PREV) ? gif->last : (u64)(whdr->ifrm + 1);
    pict = (u32*)((whdr->mode == GIF_PREV)? gif->pict : gif->prev);
    prev = (u32*)((whdr->mode == GIF_PREV)? gif->prev : gif->pict);
    for (x = (u32)(whdr->xdim * whdr->ydim); --x;
        pict[x - 1] = prev[x - 1]);
  }

  if (whdr->mode == GIF_BKGD) /** cutting a hole for the next frame **/
    for (whdr->bptr[0] = (u8)((whdr->tran >= 0)?
          whdr->tran : whdr->bkgd), y = 0,
        pict = (u32*)gif->pict; y < (u32)whdr->fryd; y++)
      for (x = 0; x < (u32)whdr->frxd; x++)
        pict[(u32)whdr->xdim * y + x + ddst] = BGRA(0);
#undef BGRA
}

bool hal_gif(gif_t* _g, const char* path) {
  gif_t g = *_g = HAL_MALLOC(sizeof(struct gif_t));

  FILE* fp = fopen(path, "rb");
  if (!fp) {
    error_handle(FILE_OPEN_FAILED, "fopen() failed: %s", path);
    return false;
  }

  fseek(fp, 0, SEEK_END);
  size_t length = ftell(fp);
  rewind(fp);

  u8* data = HAL_MALLOC((length + 1) * sizeof(u8));
  if (!data) {
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  fread(data, 1, length, fp);
  fclose(fp);

  gif_data_t tmp;
  tmp.data = data;
  tmp.size = length;
  tmp.out  = g;
  if (!GIF_Load(data, length, load_gif_frame, 0, (void*)&tmp, 0L)) {
    error_handle(GIF_LOAD_FAILED, "GIF_Load() failed");
    return false;
  }

  HAL_SAFE_FREE(tmp.data);
  HAL_SAFE_FREE(tmp.prev);
  HAL_SAFE_FREE(tmp.pict);
  return true;
}

bool hal_save_gif(gif_t* g, const char* path) {
#pragma message WARN("TODO: hal_save_gif() reimplement")
  return false
}

void hal_gif_destroy(gif_t* _g) {
  gif_t g = *_g;
  if (!g)
    return;
  for (i32 i = 0; i < g->frames; ++i)
    hal_destroy(&g->surfaces[i]);
  HAL_SAFE_FREE(g->surfaces);
  HAL_SAFE_FREE(g);
}
#endif // HAL_GIF

#if !defined(HAL_NO_ALERTS)
#if defined(HAL_OSX) && !defined(HAL_EXTERNAL_WINDOW)
#include <AppKit/AppKit.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
#define NSAlertStyleInformational NSInformationalAlertStyle
#define NSAlertStyleWarning NSWarningAlertStyle
#define NSAlertStyleCritical NSCriticalAlertStyle
#endif

bool hal_alert(ALERT_LVL lvl, ALERT_BTNS btns, const char* fmt, ...) {
  NSAlert* alert = [[NSAlert alloc] init];

  switch (lvl) {
    default:
    case ALERT_INFO:
      [alert setAlertStyle:NSAlertStyleInformational];
      break;
    case ALERT_WARNING:
      [alert setAlertStyle:NSAlertStyleWarning];
      break;
    case ALERT_ERROR:
      [alert setAlertStyle:NSAlertStyleCritical];
      break;
  }

  switch (btns) {
    default:
    case ALERT_OK:
      [alert addButtonWithTitle:@"OK"];
      break;
    case ALERT_OK_CANCEL:
      [alert addButtonWithTitle:@"OK"];
      [alert addButtonWithTitle:@"Cancel"];
      break;
    case ALERT_YES_NO:
      [alert addButtonWithTitle:@"Yes"];
      [alert addButtonWithTitle:@"No"];
      break;
  }

  i8 buffer[BUFSIZ];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
  [alert setMessageText:[NSString stringWithUTF8String:buffer]];

  bool result = ([alert runModal] == NSAlertFirstButtonReturn);
  [alert release];
  return result;
}

char* hal_dialog(DIALOG_ACTION action, const char* path, const char* fname, bool allow_multiple, i32 nfilters, ...) {
  NSSavePanel* panel = nil;
  NSOpenPanel* open_panel = nil;
  NSMutableArray* file_types = nil;

  switch (action) {
    case DIALOG_OPEN:
    case DIALOG_OPEN_DIR:
      open_panel = [NSOpenPanel openPanel];
      panel = open_panel;
      break;
    case DIALOG_SAVE:
      panel = [NSSavePanel savePanel];
      break;
    default:
      // Error message here
      return NULL;
  }
  [panel setLevel:CGShieldingWindowLevel()];

  if (!nfilters || action == DIALOG_SAVE)
    goto SKIP_FILTERS;

  file_types = [[NSMutableArray alloc] init];
  va_list args;
  va_start(args, nfilters);
  for (i32 i = 0; i < nfilters; ++i)
    [file_types addObject:@(va_arg(args, const char*))];
  va_end(args);
  [panel setAllowedFileTypes:file_types];

SKIP_FILTERS:
  if (path)
    panel.directoryURL = [NSURL fileURLWithPath:@(path)];

  if (fname)
    panel.nameFieldStringValue = @(fname);

  switch (action) {
    case DIALOG_OPEN:
      open_panel.allowsMultipleSelection = allow_multiple;
      open_panel.canChooseDirectories = NO;
      open_panel.canChooseFiles = YES;
      break;
    case DIALOG_OPEN_DIR:
      open_panel.allowsMultipleSelection = allow_multiple;
      open_panel.canCreateDirectories = YES;
      open_panel.canChooseDirectories = YES;
      open_panel.canChooseFiles = NO;
      break;
    case DIALOG_SAVE:
      break;
  }

  char* result = ([panel runModal] == NSModalResponseOK ? strdup(action == DIALOG_SAVE || !allow_multiple ? [[[panel URL] path] UTF8String] : [[[open_panel URLs] componentsJoinedByString:@","] UTF8String]) : NULL);
  if (file_types)
    [file_types release];
  return result;
}
#elif defined(HAL_EMCC)
bool hal_alert(ALERT_LVL lvl, ALERT_BTNS btns, const char* fmt, ...) {
  i8 buffer[BUFSIZ];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
  return EM_ASM_INT({ confirm(UTF8ToString($0)); }, buffer);
}

char* hal_dialog(DIALOG_ACTION action, const char* path, const char* fname, bool allow_multiple, i32 nfilters, ...) {
  return NULL;
}
#elif defined(HAL_WINDOWS)
bool hal_alert(ALERT_LVL lvl, ALERT_BTNS btns, const char* fmt, ...) {
  return false;
}

char* hal_dialog(DIALOG_ACTION action, const char* path, const char* fname, bool allow_multiple, i32 nfilters, ...) {
  return NULL;
}
#else
#error Dialogs are unsupported on this platform
#endif
#endif // HAL_NO_ALERTS

#if !defined(HAL_NO_WINDOW)
static i16 keycodes[512];
static bool keycodes_init = false;

struct window_t {
  i32 id, w, h;

#define X(a, b) void(*a##_callback)b;
  XMAP_SCREEN_CB
#undef X

  void *window, *parent;
};

static window_t active_window = NULL;

#define X(a, b) void(*a##_cb)b,
void hal_window_callbacks(XMAP_SCREEN_CB window_t window) {
#undef X
#define X(a, b) window->a##_callback = a##_cb;
  XMAP_SCREEN_CB
#undef X
}

#define X(a, b) \
void hal_##a##_callback(window_t window, void(*a##_cb)b) { \
  window->a##_callback = a##_cb; \
}
XMAP_SCREEN_CB
#undef X

i32 hal_window_id(window_t s) {
  return s->id;
}

void hal_window_size(window_t s, i32* w, i32* h) {
  if (w)
    *w = s->w;
  if (h)
    *h = s->h;
}

#define CBCALL(x, ...) \
  if (active_window && active_window->x) \
    active_window->x(active_window->parent, __VA_ARGS__);

#if defined(HAL_OPENGL)
#if defined(HAL_OSX)
#include <OpenGL/gl3.h>
#endif

#if defined(HAL_LINUX)
#define GLDECL // Empty define
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include <dlfcn.h>
#endif

#if defined(HAL_WINDOWS)
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

#if defined(HAL_WINDOWS) || defined(HAL_LINUX)
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

static inline void print_shader_log(GLuint s) {
  if (glIsShader(s)) {
    i32 log_len = 0, max_len = 0;
    glGetShaderiv(s, GL_INFO_LOG_LENGTH, &max_len);
    char* log = HAL_MALLOC(sizeof(char) * max_len);

    glGetShaderInfoLog(s, max_len, &log_len, log);
    if (log_len > 0)
      error_handle(GL_SHADER_ERROR, "load_shader() failed: %s", log);

    HAL_SAFE_FREE(log);
  }
}

static inline GLuint load_shader(const GLchar* src, GLenum type) {
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

static inline GLuint create_shader(const GLchar* vs_src, const GLchar* fs_src) {
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

static i32 gl3_available = 1;
static bool dll_loaded = false;

static inline bool init_gl(i32 w, i32 h, GLuint* _vao, GLuint* _shader, GLuint* _texture) {
  if (!dll_loaded) {
#if defined(HAL_WINDOWS)
    HINSTANCE dll = LoadLibraryA("opengl32.dll");
    typedef PROC WINAPI wglGetProcAddressproc(LPCSTR lpszProc);
    if (!dll) {
      hal_release();
      error_handle(GL_LOAD_DL_FAILED, "LoadLibraryA() failed: opengl32.dll not found");
      return false;
    }
    wglGetProcAddressproc* wglGetProcAddress = (wglGetProcAddressproc*)GetProcAddress(dll, "wglGetProcAddress");

#define GLE(ret, name, ...) \
    gl##name = (name##proc*)wglGetProcAddress("gl" #name); \
    if (!gl##name) { \
      error_handle(GL_GET_PROC_ADDR_FAILED, "wglGetProcAddress() failed: Function gl" #name " couldn't be loaded from opengl32.dll"); \
      gl3_available -= 1; \
    }
    GL_LIST
#undef GLE
#elif defined(HAL_LINUX)
      void* libGL = dlopen("libGL.so", RTLD_LAZY);
    if (!libGL) {
      hal_release();
      error_handle(GL_LOAD_DL_FAILED, "dlopen() failed: libGL.so couldn't be loaded");
      return false;
    }

#define GLE(ret, name, ...) \
    gl##name = (name##proc *) dlsym(libGL, "gl" #name); \
    if (!gl##name) { \
      error_handle(GL_GET_PROC_ADDR_FAILED, "dlsym() failed: Function gl" #name " couldn't be loaded from libGL.so"); \
      gl3_available -= 1; \
    }
    GL_LIST
#undef GLE
#endif
    dll_loaded = true;
  }

  glClearColor(0.f, 0.f, 0.f, 1.f);

#if !defined(HAL_OSX)
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

    static GLfloat vertices_position[8] = {
      -1., -1.,
       1., -1.,
       1.,  1.,
      -1.,  1.,
    };

    static GLfloat texture_coord[8] = {
      .0,  .0,
       1., .0,
       1., 1.,
      .0,  1.,
    };

    static GLuint indices[6] = {
      0, 1, 2,
      2, 3, 0
    };

    static const char* vs_src =
      "#version 150\n"
      "in vec4 position;"
      "in vec2 texture_coord;"
      "out vec2 texture_coord_from_vshader;"
      "void main() {"
      "  gl_Position = position;"
      "  texture_coord_from_vshader = vec2(texture_coord.s, 1.f - texture_coord.t);"
      "}";

    static const char* fs_src =
      "#version 150\n"
      "in vec2 texture_coord_from_vshader;"
      "out vec4 out_color;"
      "uniform sampler2D texture_sampler;"
      "void main() {"
      "  out_color = texture(texture_sampler, texture_coord_from_vshader);"
      "}";

    static GLuint vao, shader, texture;

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

    *_vao = vao;
    *_shader = shader;
#if !defined(HAL_OSX)
  }
#endif

  glGenTextures(1, &texture);
  *_texture = texture;

  return true;
}

static inline void draw_gl(GLuint vao, GLuint texture, surface_t buffer) {
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer->w, buffer->h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, (GLvoid*)buffer->buf);

  glClear(GL_COLOR_BUFFER_BIT);

#if !defined(HAL_OSX)
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
#if !defined(HAL_OSX)
  }

  glFlush();
#endif
}

static inline void free_gl(GLuint vao, GLuint shader, GLuint texture) {
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

#if defined(HAL_OSX) && !defined(HAL_EXTERNAL_WINDOW)
#include <Cocoa/Cocoa.h>
#if defined(HAL_METAL)
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
#define NSBitmapFormatAlphaNonpremultiplied NSAlphaNonpremultipliedBitmapFormat
#endif

static inline i32 translate_mod(NSUInteger flags) {
  i32 mods = 0;

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

static inline i32 translate_key(u32 key) {
  return (key >= sizeof(keycodes) / sizeof(keycodes[0]) ?  KB_KEY_UNKNOWN : keycodes[key]);
}

static inline NSImage* create_cocoa_image(surface_t s) {
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

  char* rgba = HAL_MALLOC(s->w * s->h * 4);
  i32 offset = 0, c;
  for(i32 i = 0; i < s->h; ++i) {
    for (i32 j = 0; j < s->w; j++) {
      c = XYGET(s, j, i);
      rgba[4 * offset]     = HAL_R(c);
      rgba[4 * offset + 1] = HAL_G(c);
      rgba[4 * offset + 2] = HAL_B(c);
      rgba[4 * offset + 3] = HAL_A(c);
      offset++;
    }
  }
  memcpy([nsbir bitmapData], rgba, s->w * s->h * sizeof(char) * 4);
  HAL_SAFE_FREE(rgba);

  [nsi addRepresentation:nsbir];
  return nsi;
}

@protocol AppViewDelegate;

#if defined(HAL_OPENGL)
@interface AppView : NSOpenGLView
@property GLuint vao;
@property GLuint shader;
@property GLuint texture;
#elif defined(HAL_METAL)
@interface AppView : MTKView
@property (nonatomic, weak) id<MTLDevice> device;
@property (nonatomic, weak) id<MTLRenderPipelineState> pipeline;
@property (nonatomic, weak) id<MTLCommandQueue> cmd_queue;
@property (nonatomic, weak) id<MTLLibrary> library;
@property (nonatomic, weak) id<MTLTexture> texture;
@property (nonatomic, weak) id<MTLBuffer> vertices;
@property NSUInteger n_vertices;
@property vector_uint2 mtk_viewport;
@property CGFloat scale_f;
#else
@interface AppView : NSView
#endif
@property (nonatomic, weak) id<AppViewDelegate> delegate;
@property (strong) NSTrackingArea* track;
@property (nonatomic) surface_t buffer;
@property BOOL mouse_in_window;
@property (nonatomic, strong) NSCursor* cursor;
@property BOOL custom_cursor;
@end

@implementation AppView
#if defined(HAL_OPENGL)
@synthesize vao = _vao;
@synthesize shader = _shader;
@synthesize texture = _texture;
#elif defined(HAL_METAL)
@synthesize device = _device;
@synthesize pipeline = _pipeline;
@synthesize cmd_queue = _cmd_queue;
@synthesize library = _library;
@synthesize texture = _texture;
@synthesize vertices = _vertices;
@synthesize n_vertices = _n_vertices;
@synthesize mtk_viewport = _mtk_viewport;
@synthesize scale_f = _scale_f;
#endif
@synthesize delegate = _delegate;
@synthesize track = _track;
@synthesize buffer = _buffer;
@synthesize mouse_in_window = _mouse_in_window;
@synthesize cursor = _cursor;
@synthesize custom_cursor = _custom_cursor;

- (id)initWithFrame:(NSRect)frameRect {
  _mouse_in_window = NO;
  _cursor = [NSCursor arrowCursor];
  _custom_cursor = NO;
  
#if defined(HAL_OPENGL)
  NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    NSOpenGLPFAColorSize, 24,
    NSOpenGLPFAAlphaSize, 8,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFANoRecovery,
    0
  };
  NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
  self = [super initWithFrame:frameRect
                  pixelFormat:pixelFormat];
  [[self openGLContext] makeCurrentContext];
  init_gl(frameRect.size.width, frameRect.size.height, &_vao, &_shader, &_texture);
  [pixelFormat release];
#elif defined(HAL_METAL)
  _device = MTLCreateSystemDefaultDevice();
  self = [super initWithFrame:frameRect device:_device];
  _track = nil;
  
  self.clearColor =  MTLClearColorMake(0., 0., 0., 0.);
  NSScreen *screen = [NSScreen mainScreen];
  _scale_f = [screen backingScaleFactor];
  _mtk_viewport.x = frameRect.size.width * _scale_f;
  _mtk_viewport.y = ((frameRect.size.height) * _scale_f) + (4 * _scale_f);
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
  "  float4 clipSpacePosition [[position]];"
  "  float2 textureCoordinate;"
  "} RasterizerData;"
  "typedef enum AAPLVertexInputIndex {"
  "  AAPLVertexInputIndexVertices     = 0,"
  "  AAPLVertexInputIndexViewportSize = 1,"
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
  "  float2 pixelSpacePosition = float2(vertexArray[vertexID].position.x, -vertexArray[vertexID].position.y);"
  "  out.clipSpacePosition.xy = pixelSpacePosition;"
  "  out.clipSpacePosition.z = .0;"
  "  out.clipSpacePosition.w = 1.;"
  "  out.textureCoordinate = vertexArray[vertexID].textureCoordinate;"
  "  return out;"
  "}"
  "fragment float4 samplingShader(RasterizerData in [[stage_in]], texture2d<half> colorTexture [[ texture(AAPLTextureIndexBaseColor) ]]) {"
  "  constexpr sampler textureSampler(mag_filter::nearest, min_filter::linear);"
  "  const half4 colorSample = colorTexture.sample(textureSampler, in.textureCoordinate);"
  "  return float4(colorSample);"
  "}";
  
  NSError *err = nil;
  _library = [_device newLibraryWithSource:library
                                   options:nil
                                     error:&err];
  if (err || !_library) {
    hal_release();
    error_handle(MTK_LIBRARY_ERROR, "[device newLibraryWithSource] failed: %s", [[err localizedDescription] UTF8String]);
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
    hal_release();
    error_handle(MTK_CREATE_PIPELINE_FAILED, "[device newRenderPipelineStateWithDescriptor] failed: %s", [[err localizedDescription] UTF8String]);
    return nil;
  }
#else
  self = [super initWithFrame:frameRect];
#endif
  [self updateTrackingAreas];
  
  return self;
}

- (void)updateTrackingAreas {
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

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (BOOL)performKeyEquivalent:(NSEvent*)event {
  return YES;
}

- (void)resetCursorRects {
  [super resetCursorRects];
  [self addCursorRect:[self visibleRect] cursor:(_cursor ? _cursor : [NSCursor arrowCursor])];
}

- (void)setCustomCursor:(NSImage*)img {
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

- (void)setRegularCursor:(CURSOR_TYPE)type {
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

- (void)mouseEntered:(NSEvent*)event {
  _mouse_in_window = YES;
}

- (void)mouseExited:(NSEvent*)event {
  _mouse_in_window = NO;
}

- (void)mouseMoved:(NSEvent*)event {
  if (_cursor && _mouse_in_window)
    [_cursor set];
}

- (BOOL)preservesContentDuringLiveResize {
  return NO;
}

#if defined(HAL_METAL)
- (void)updateMTKViewport:(CGSize)size {
  _mtk_viewport.x = size.width * _scale_f;
  _mtk_viewport.y = (size.height * _scale_f) + (4 * _scale_f);
}
#endif

- (void)drawRect:(NSRect)dirtyRect {
  if (!_buffer)
    return;
  
#if defined(HAL_OPENGL)
  draw_gl(_vao, _texture, _buffer);
  [[self openGLContext] flushBuffer];
#elif defined(HAL_METAL)
  MTLTextureDescriptor* td = [[MTLTextureDescriptor alloc] init];
  td.pixelFormat = MTLPixelFormatBGRA8Unorm;
  td.width = _buffer->w;
  td.height = _buffer->h;
  
  _texture = [_device newTextureWithDescriptor:td];
  [_texture replaceRegion:(MTLRegion){{ 0, 0, 0 }, { _buffer->w, _buffer->h, 1 }}
              mipmapLevel:0
                withBytes:_buffer->buf
              bytesPerRow:_buffer->w * 4];
  
  id <MTLCommandBuffer> cmd_buf = [_cmd_queue commandBuffer];
  cmd_buf.label = @"[Command Buffer]";
  MTLRenderPassDescriptor* rpd = [self currentRenderPassDescriptor];
  if (rpd) {
    id<MTLRenderCommandEncoder> re = [cmd_buf renderCommandEncoderWithDescriptor:rpd];
    re.label = @"[Render Encoder]";
    
    [re setViewport:(MTLViewport){ .0, .0, _mtk_viewport.x, _mtk_viewport.y, -1., 1. }];
    [re setRenderPipelineState:_pipeline];
    [re setVertexBuffer:_vertices
                 offset:0
                atIndex:AAPLVertexInputIndexVertices];
    [re setVertexBytes:&_mtk_viewport
                length:sizeof(_mtk_viewport)
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
  CGDataProviderRef p = CGDataProviderCreateWithData(NULL, _buffer->buf, _buffer->w * _buffer->h * 3, NULL);
  CGImageRef img = CGImageCreate(_buffer->w, _buffer->h, 8, 32, _buffer->w * 4, s, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, p, NULL, 0, kCGRenderingIntentDefault);
  CGContextDrawImage(ctx, CGRectMake(0, 0, [self frame].size.width, [self frame].size.height), img);
  CGColorSpaceRelease(s);
  CGDataProviderRelease(p);
  CGImageRelease(img);
#endif
}

-(void)dealloc {
#if defined(HAL_OPENGL)
  free_gl(_vao, _shader, _texture);
#elif defined(HAL_METAL)
  [_device release];
  [_pipeline release];
  [_cmd_queue release];
  [_library release];
  [_vertices release];
#endif
  [_track release];
  if (_custom_cursor && _cursor)
    [_cursor release];
}
@end

@protocol AppViewDelegate <NSObject>
#if defined(HAL_METAL)
- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size;
#endif
@end

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate, AppViewDelegate>
@property (unsafe_unretained) NSWindow* window;
@property (weak) AppView* view;
@property (nonatomic) window_t parent;
@property BOOL closed;
@end

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
    hal_release();
    error_handle(OSX_WINDOW_CREATION_FAILED, "[_window initWithContentRect] failed");
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
  
  if (!active_window)
    [_window center];
  else {
    AppDelegate* tmp = (AppDelegate*)active_window->window;
    NSPoint tmp_p = [[tmp window] frame].origin;
    [_window setFrameOrigin:NSMakePoint(tmp_p.x + 20, tmp_p.y - 20 - [tmp titlebarHeight])];
  }
  
  _view = [[AppView alloc] initWithFrame:frameRect];
  if (!_view) {
    hal_release();
    error_handle(OSX_WINDOW_CREATION_FAILED, "[_view initWithFrame] failed");
    return nil;
  }
  [_view setDelegate:self];
  [_view setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  
  [_window setContentView:_view];
  [_window setDelegate:self];
  [_window performSelectorOnMainThread:@selector(makeKeyAndOrderFront:) withObject:nil waitUntilDone:YES];
  
  _closed = NO;
  return self;
}

- (void)setParent:(window_t)screen {
  _parent = screen;
}

- (CGFloat)titlebarHeight {
  return _window.frame.size.height - [_window contentRectForFrameRect: _window.frame].size.height;
}

- (void)windowWillClose:(NSNotification*)notification {
  if (active_window && active_window->closed_callback)
    active_window->closed_callback(active_window->parent);
  _closed = YES;
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
  active_window = _parent;
  CBCALL(focus_callback, true);
}

- (void)windowDidResignKey:(NSNotification*)notification {
  CBCALL(focus_callback, false);
  active_window = NULL;
}

- (void)windowDidResize:(NSNotification*)notification {
  static CGSize size;
  size = [_view frame].size;
#if defined(HAL_METAL)
  [_view updateMTKViewport:size];
#endif
  _parent->w = (int)roundf(size.width);
  _parent->h = (int)roundf(size.height);
  CBCALL(resize_callback, _parent->w, _parent->h);
}

#if defined(HAL_METAL)
- (void)mtkView:(MTKView*)mtkView drawableSizeWillChange:(CGSize)size; {}
#endif
@end

bool hal_window(struct window_t** s, const char* t, int w, int h, short flags) {
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
    hal_release();
    error_handle(OSX_WINDOW_CREATION_FAILED, "[AppDelegate alloc] failed");
    return false;
  }
  
  struct window_t* window = *s = HAL_MALLOC(sizeof(struct window_t));
  if (!window) {
    hal_release();
    error_handle(OUT_OF_MEMEORY, "malloc failed");
    return false;
  }
  memset(window, 0, sizeof(*window));
  window->id = (int)[[app window] windowNumber];
  window->w  = w;
  window->h  = h;
  window->window = (void*)app;
  active_window = window;
  [app setParent:window];
  
  [NSApp activateIgnoringOtherApps:YES];
  [pool drain];
  return true;
}

#define SET_DEFAULT_APP_ICON [NSApp setApplicationIconImage:[NSImage imageNamed:@"NSApplicationIcon"]]

void hal_window_icon(window_t s, surface_t b) {
  if (!b || !b->buf) {
    SET_DEFAULT_APP_ICON;
    return;
  }
  
  NSImage* img = create_cocoa_image(b);
  if (!img)  {
    error_handle(WINDOW_ICON_FAILED, "hal_window_icon_b() failed: Couldn't set window icon");
    SET_DEFAULT_APP_ICON;
    return;
  }
  [NSApp setApplicationIconImage:img];
}

void hal_window_title(window_t s, const char* t) {
  [[(AppDelegate*)s->window window] setTitle:@(t)];
}

void hal_window_position(window_t s, int* x, int*  y) {
  static NSRect frame;
  frame = [[(AppDelegate*)s->window window] frame];
  if (x)
    *x = frame.origin.x;
  if (y)
    *y = frame.origin.y;
}

void hal_screen_size(window_t s, int* w, int* h) {
  static NSRect frame;
  frame = [[[(AppDelegate*)s->window window] screen] frame];
  if (w)
    *w = frame.size.width;
  if (h)
    *h = frame.size.height;
}


void hal_window_destroy(struct window_t** s) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  struct window_t* window = *s;
  AppDelegate* app = (AppDelegate*)window->window;
  if (app) {
    [[app view] dealloc];
    [[app window] close];
  }
  HAL_SAFE_FREE(app);
  HAL_SAFE_FREE(window);
  [pool drain];
}

bool hal_closed(window_t s) {
  return (bool)[(AppDelegate*)s->window closed];
}

static bool cursor_locked = false;

void hal_cursor_lock(bool locked) {
  cursor_locked = locked;
  CGAssociateMouseAndMouseCursorPosition(!locked);
}

void hal_cursor_visible(bool shown) {
  if (shown)
    [NSCursor unhide];
  else
    [NSCursor hide];
}

void hal_cursor_icon(window_t s, CURSOR_TYPE t) {
  AppDelegate* app = (AppDelegate*)s->window;
  if (!app) {
    error_handle(CURSOR_MOD_FAILED, "hal_cursor_icon() failed: Invalid window");
    return;
  }
  [[app view] setRegularCursor:t];
}

void hal_cursor_icon_custom(window_t s, surface_t b) {
  NSImage* img = create_cocoa_image(b);
  if (!img) {
    error_handle(CURSOR_MOD_FAILED, "hal_cursor_icon_custom_buf() failed: Couldn't set cursor from buffer");
    return;
  }
  
  AppDelegate* app = (AppDelegate*)s->window;
  if (!app) {
    [img release];
    error_handle(CURSOR_MOD_FAILED, "hal_cursor_icon_custom_buf() failed: Invalid window");
    return;
  }
  [[app view] setCustomCursor:img];
  [img release];
}

void hal_cursor_pos(int* x, int* y) {
  static NSPoint _p = {0,0};
  _p = [NSEvent mouseLocation];
  if (x)
    *x = _p.x;
  if (y)
    *y = [[(AppDelegate*)active_window->window window] screen].frame.size.height - _p.y;
}

void hal_cursor_set_pos(int x, int y) {
  CGWarpMouseCursorPosition((CGPoint){ x, y });
}

void hal_poll() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSEvent* e = nil;
  while ((e = [NSApp nextEventMatchingMask:NSEventMaskAny
                                 untilDate:[NSDate distantPast]
                                    inMode:NSDefaultRunLoopMode
                                   dequeue:YES])) {
    switch ([e type]) {
      case NSEventTypeKeyUp:
      case NSEventTypeKeyDown:
        CBCALL(keyboard_callback, translate_key([e keyCode]), translate_mod([e modifierFlags]), ([e type] == NSEventTypeKeyDown));
        break;
      case NSEventTypeLeftMouseUp:
      case NSEventTypeRightMouseUp:
      case NSEventTypeOtherMouseUp:
        CBCALL(mouse_button_callback, (MOUSE_BTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), false);
        break;
      case NSEventTypeLeftMouseDown:
      case NSEventTypeRightMouseDown:
      case NSEventTypeOtherMouseDown:
        CBCALL(mouse_button_callback, (MOUSE_BTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
        break;
      case NSEventTypeScrollWheel:
        CBCALL(scroll_callback, translate_mod([e modifierFlags]), [e deltaX], [e deltaY]);
        break;
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged:
        CBCALL(mouse_button_callback, (MOUSE_BTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
      case NSEventTypeMouseMoved: {
        static AppDelegate* app = NULL;
        if (!active_window)
          break;
        app = (AppDelegate*)active_window->window;
        if (cursor_locked) {
          static NSScreen* s = nil;
          s = [NSScreen mainScreen];
          hal_cursor_set_pos((int)[s frame].size.width / 2, (int)[s frame].size.height / 2);
        }
        if ([[app view] mouse_in_window] || cursor_locked)
          CBCALL(mouse_move_callback, [e locationInWindow].x, (int)([[app view] frame].size.height - roundf([e locationInWindow].y)), [e deltaX], [e deltaY]);
        break;
      }
    }
    [NSApp sendEvent:e];
  }
  [pool release];
}

void hal_flush(window_t s, surface_t b) {
  AppDelegate* tmp = (AppDelegate*)s->window;
  if (!tmp)
    return;
  if (b)
    [tmp view].buffer = b;
  [[tmp view] setNeedsDisplay:YES];
}

void hal_release() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  // Felt cute might delete later
  [pool drain];
}
#elif defined(HAL_WINDOWS) && !defined(HAL_EXTERNAL_WINDOW)
bool hal_window(struct window_t** s, const char* t, int w, int h, short flags) {
  return true;
}

void hal_window_icon(window_t s, surface_t b) {
  
}

void hal_window_title(window_t s, const char* t) {
  
}

void hal_window_position(window_t s, int* x, int*  y) {
  
}

void hal_screen_size(window_t s, int* w, int* h) {
  
}


void hal_window_destroy(struct window_t** s) {
  
}

bool hal_closed(window_t s) {
  return false;
}

void hal_cursor_lock(bool locked) {
  
}

void hal_cursor_visible(bool shown) {
  
}

void hal_cursor_icon(window_t s, CURSOR_TYPE t) {
  
}

void hal_cursor_icon_custom(window_t s, surface_t b) {
  
}

void hal_cursor_pos(int* x, int* y) {
  
}

void hal_cursor_set_pos(int x, int y) {
  
}

void hal_poll() {
  
}

void hal_flush(window_t s, surface_t b) {
  
}

void hal_release() {
  
}
#elif defined(HAL_LINUX) && !defined(HAL_EXTERNAL_WINDOW)
#error TODO: HAL_LINUX reimplement
#elif defined(HAL_EMCC)

#if !defined(HAL_CANVAS_NAME)
#if defined(HAL_CANVAS_ID)
#error HAL_CANVAS_ID cannot be pre-defined unless HAL_CANVAS_NAME is also pre-defined
#endif
#define HAL_CANVAS_NAME "canvas"
#endif
#define HAL_CANVAS_ID "#" HAL_CANVAS_NAME

#include <stdio.h>
#include <emscripten.h>
#include <string.h>
#include <emscripten/html5.h>

static i32 window_w, window_h, canvas_w, canvas_h, canvas_x, canvas_y, cursor_x, cursor_y;
static bool mouse_in_canvas = true, fullscreen = false;

static KEY_MOD translate_mod(bool ctrl, bool shift, bool alt, bool meta) {
  return (KEY_MOD)((ctrl ? KB_MOD_CONTROL : 0) | (shift ? KB_MOD_SHIFT : 0) | (alt ? KB_MOD_ALT : 0) | (meta ? KB_MOD_SUPER : 0));
}

static KEY_SYM translate_key(i32 key) {
  return (key > 222 || key < 8 ? KB_KEY_UNKNOWN : keycodes[key]);
}

static EM_BOOL key_callback(int type, const EmscriptenKeyboardEvent* e, void* user_data) {
  static KEY_MOD mod;
  static KEY_SYM sym;
  mod = translate_mod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey);
  sym = translate_key(e->keyCode);
  CBCALL(keyboard_callback, sym, mod, (type == EMSCRIPTEN_EVENT_KEYDOWN));

  switch (sym) {
    case KB_KEY_R: // Reload
    case KB_KEY_W: // Close tab (Just in case)
    case KB_KEY_Q: // Close window (Just in case)
#if defined(HAL_OSX)
      return (mod == KB_MOD_SUPER);
#else
      return (mod == KB_MOD_CONTROL);
#endif
#if defined(HAL_DEBUG) && defined(HAL_EMCC_HTML)
    case KB_KEY_F12: // Developer tools Edge
      return true;
    case KB_KEY_I: // Developer tools Chrome/Firefox
    case KB_KEY_J: // Developer tools Chrome
    case KB_KEY_C: // Developer tools Safari/Firefox
    case KB_KEY_K: // Developer tools Firefox
#if defined(HAL_OSX)
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
        CBCALL(mouse_button_callback, (MOUSE_BTN)(e->button + 1), translate_mod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), true);
      break;
    case EMSCRIPTEN_EVENT_MOUSEUP:
      if (mouse_in_canvas)
        CBCALL(mouse_button_callback, (MOUSE_BTN)(e->button + 1), translate_mod(e->ctrlKey, e->shiftKey, e->altKey, e->metaKey), false);
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
    emscripten_get_element_css_size(HAL_CANVAS_ID, &css_w, &css_h);
    canvas_w  = (i32)css_w;
    canvas_h = (i32)css_h;
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

bool hal_window(window_t* s, const char* t, i32 w, i32 h, i16 flags) {
  static bool window_already_open = false;
  if (window_already_open || active_window) {
#pragma message WARN("TODO: hal_window() handle error")
    return false;
  }
  
  struct window_t* window = *s = active_window = HAL_MALLOC(sizeof(struct window_t));
  if (!window) {
    hal_release();
    error_handle(OUT_OF_MEMEORY, "malloc() failed");
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
  
  emscripten_set_click_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mousedown_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseup_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_dblclick_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mousemove_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseenter_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseleave_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseover_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  emscripten_set_mouseout_callback(HAL_CANVAS_ID, 0, 1, mouse_callback);
  
  emscripten_set_wheel_callback(0, 0, 1, wheel_callback);
  
  emscripten_set_resize_callback(0, 0, 1, uievent_callback);
  emscripten_set_scroll_callback(0, 0, 1, uievent_callback);
  
  emscripten_set_blur_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focus_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focusin_callback(0, 0, 1, focusevent_callback);
  emscripten_set_focusout_callback(0, 0, 1, focusevent_callback);
  
  emscripten_set_fullscreenchange_callback(HAL_CANVAS_ID, 0, 1, fullscreenchange_callback);
  
  emscripten_set_pointerlockchange_callback(0, 0, 1, pointerlockchange_callback);
  
  emscripten_set_beforeunload_callback(0, beforeunload_callback);
  
  emscripten_set_webglcontextlost_callback(0, 0, 1, webglcontext_callback);
  emscripten_set_webglcontextrestored_callback(0, 0, 1, webglcontext_callback);
  
  EM_ASM(Module['noExitRuntime'] = true);
  
#if defined(HAL_OPENGL)
  
#endif
  
  if (t)
    hal_window_title(NULL, t);
  
  emscripten_set_canvas_element_size(HAL_CANVAS_ID, w, h);
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

void hal_window_icon(window_t _, surface_t __) {
#pragma message WARN("hal_window_icon() unsupported on emscripten")
}

void hal_window_title(window_t _, const char* t) {
  EM_ASM({
    setWindowTitle(UTF8ToString($0));
  }, t);
}

void hal_window_position(window_t _, int* x, int*  y) {
  if (x)
    *x = canvas_x;
  if (y)
    *y = canvas_y;
}

void hal_screen_size(window_t _, int* w, int* h) {
  if (w)
    *w = window_w;
  if (h)
    *h = window_h;
}

void hal_window_destroy(window_t* s) {
  struct window_t* window = *s;
  HAL_SAFE_FREE(window);
}

bool hal_closed(window_t _) {
  return false;
}

void hal_cursor_lock(bool locked) {
#pragma message WARN("hal_cursor_lock() unsupported on emscripten")
  if (locked)
    emscripten_request_pointerlock(NULL, 1);
  else
    emscripten_exit_pointerlock();
}

static const char* cursor = "default";
static bool cursor_custom = false;

void hal_cursor_visible(bool show) {
  EM_ASM({
    if (Module['canvas']) {
      Module['canvas'].style['cursor'] = ($1 ? UTF8ToString($0) : 'none');
    }
  }, cursor, show);
}

void hal_cursor_icon(window_t _, CURSOR_TYPE t) {
  if (cursor_custom && cursor)
    HAL_SAFE_FREE(cursor);
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
  hal_cursor_visible(true);
}

void hal_cursor_custom_icon(window_t _, surface_t b) {
  if (cursor_custom && cursor)
    HAL_SAFE_FREE(cursor);
  
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
    error_handle(UNKNOWN_ERROR, "hal_cursor_custom_icon() failed");
    cursor = "default";
    cursor_custom = false;
    return;
  }
  cursor_custom = true;
  hal_cursor_visible(true);
}

void hal_cursor_pos(i32* x, i32* y) {
  if (x)
    *x = cursor_x;
  if (y)
    *y = cursor_y;
}

void hal_cursor_set_pos(i32 _, i32 __) {
#pragma message WARN("hal_cursor_set_pos() unsupported on emscripten")
}

void hal_poll(void) {
#if defined(HAL_DEBUG) && defined(HAL_EMCC_HTML)
  EM_ASM({
    stats.begin();
  });
#endif
}

void hal_flush(window_t _, surface_t b) {
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
#if defined(HAL_DEBUG) && defined(HAL_EMCC_HTML)
    stats.end();
#endif
  }, b->w, b->h, b->buf);
}

void hal_release(void) {
  return;
}
#else
#error Unsupported operating system, sorry
#endif
#else // This is just a dummy
struct window_t {};

#define X(a, b) void(*a##_cb)b,
void hal_window_callbacks(XMAP_SCREEN_CB window_t a) {
#undef X
  return;
}

#define X(a, b) \
void hal_##a##_callback(window_t window, void(*a##_cb)b) { \
  return; \
}
XMAP_SCREEN_CB
#undef X

i32 hal_window_id(window_t a) {
  return 0;
}

void hal_window_size(window_t a, i32* b, i32* c) {
  return;
}

bool hal_window(window_t* a, const char* b, i32 c, i32 d, i16 e) {
  return true;
}

void hal_window_icon(window_t a, surface_t b) {
  return;
}

void hal_window_title(window_t a, const char* b) {
  return;
}

void hal_window_position(window_t a, int* b, int*  c) {
  return;
}

void hal_screen_size(window_t a, int* b, int* c) {
  return;
}

void hal_window_destroy(window_t* a) {
  return;
}

bool hal_closed(window_t a) {
  return false;
}

void hal_cursor_lock(bool a) {
  return;
}

void hal_cursor_visible(bool a) {
  return;
}

void hal_cursor_icon(window_t a, CURSOR_TYPE b) {
  return;
}

void hal_cursor_custom_icon(window_t a, surface_t b) {
  return;
}

void hal_cursor_pos(i32* a, i32* b) {
  return;
}

void hal_cursor_set_pos(i32 a, i32 b) {
}

void hal_poll() {
  return;
}

void hal_flush(window_t a, surface_t b) {
  return;
}

void hal_release() {
  return;
}
#endif
