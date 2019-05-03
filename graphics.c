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

  fprintf(stderr, "[%s:%d] from %s in %s() at %d -- %s\n", errprio_str(pri), (int)type, __FILE__, __FUNCTION__, __LINE__, error);
  if (__error_callback)
    __error_callback(userdata, pri, type, error, __FILE__, __FUNCTION__, __LINE__);

  va_end(args);
}

#if defined(SGL_ENABLE_FREETYPE) || defined(SGL_ENABLE_GIF)
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
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "realloc() failed");
    return (void *) (2*sizeof(int)); // try to force a NULL pointer exception later
  }
}
#endif

struct surface_t {
  int *buf, w, h;
};

void sgl_surface_size(surface_t s, int* w, int* h) {
  if (w)
    *w = s->w;
  if (h)
    *h = s->h;
}

int sgl_surface_width(surface_t s) {
  return s->w;
}

int sgl_surface_height(surface_t s) {
  return s->h;
}

int* sgl_surface_raw(surface_t s) {
  return s->buf;
}

bool sgl_surface(surface_t* _s, unsigned int w, unsigned int h) {
  surface_t s = *_s = malloc(sizeof(struct surface_t));
  if (!s) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
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

void sgl_destroy(surface_t* _s) {
  surface_t s = *_s;
  if (!s)
    return;
  FREE_SAFE(s->buf);
  FREE_SAFE(s);
  *_s = NULL;
}

void sgl_fill(surface_t s, int col) {
  for (int i = 0; i < s->w * s->h; ++i)
    s->buf[i] = col;
}

#define XYGET(s, x, y) (s->buf[(y) * s->w + (x)])

static inline void flood_fn(surface_t s, int x, int y, int new, int old) {
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

void sgl_flood(surface_t s, int x, int y, int col) {
  if (!s || x < 0 || y < 0 || x >= s->w || y >= s->h)
    return;
  flood_fn(s, x, y, col, XYGET(s, x, y));
}

void sgl_cls(surface_t s) {
  memset(s->buf, 0, s->w * s->h * sizeof(int));
}

void sgl_pset(surface_t s, int x, int y, int c)  {
  if (x >= 0 && y >= 0 && x < s->w && y < s->h)
    s->buf[y * s->w + x] = c;
}

#define BLEND(c0, c1, a0, a1) (c0 * a0 / 255) + (c1 * a1 * (255 - a0) / 65025)

void sgl_psetb(surface_t s, int x, int y, int c) {
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

int sgl_pget(surface_t s, int x, int y) {
  return (x < 0 || y < 0 || x >= s->w || y >= s->h ? 0 : XYGET(s, x, y));
}

bool sgl_blit(surface_t dst, point_t* p, surface_t src, rect_t* r) {
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
      sgl_psetb(dst, offset_x + x, offset_y + y, c);
    }
  return true;
}

bool sgl_reset(surface_t s, int nw, int nh) {
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

bool sgl_copy(surface_t a, surface_t* b) {
  if (!sgl_surface(b, a->w, a->h))
    return false;
  surface_t tmp = *b;
  memcpy(tmp->buf, a->buf, a->w * a->h * sizeof(unsigned int) + 1);
  return !!tmp->buf;
}

void sgl_filter(surface_t s, int (*fn)(int x, int y, int col)) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      sgl_pset(s, x, y, fn(x, y, XYGET(s, x, y)));
}

bool sgl_resize(surface_t a, int nw, int nh, surface_t* b) {
  if (!sgl_surface(b, nw, nh))
    return false;

  surface_t tmp = *b;
  int x_ratio = (int)((a->w << 16) / nw) + 1;
  int y_ratio = (int)((a->h << 16) / nh) + 1;
  int x2, y2, i, j;
  for (i = 0; i < nh; ++i) {
    int* t = tmp->buf + i * nw;
    y2 = ((i * y_ratio) >> 16);
    int* p = a->buf + y2 * a->w;
    int rat = 0;
    for (j = 0; j < nw; ++j) {
      x2 = (rat >> 16);
      *t++ = p[x2];
      rat += x_ratio;
    }
  }
  return true;
}

bool sgl_rotate(surface_t a, float angle, surface_t* b) {
  float theta = DEG2RAD(angle);
  float c = cosf(theta), s = sinf(theta);
  float r[3][2] = {
    { -a->h * s, a->h * c },
    {  a->w * c - a->h * s, a->h * c + a->w * s },
    {  a->w * c, a->w * s }
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
  if (!sgl_surface(b, dw, dh))
    return false;

// p'x = cos(theta) * (px - ox) - sin(theta) * (py-oy) + ox
// p'y = sin(theta) * (px - ox) + cos(theta) * (py-oy) + oy
// psetb(dst, x + offset_x - dw / 2  + src->w / 2, y + offset_y - dh / 2 + src->h / 2, pget(src, sx, sy));

  int x, y, sx, sy;
  for (x = 0; x < dw; ++x)
    for (y = 0; y < dh; ++y) {
      sx = ((x + mm[0][0]) * c + (y + mm[0][1]) * s);
      sy = ((y + mm[0][1]) * c - (x + mm[0][0]) * s);
      if (sx < 0 || sx >= a->w || sy < 0 || sy >= a->h)
        continue;
      sgl_psetb(*b, x, y, XYGET(a, sx, sy));
    }
  return true;
}

typedef struct oct_node_t{
  int64_t r, g, b; /* sum of all child node colors */
  int count, heap_idx;
  unsigned char n_kids, kid_idx, flags, depth;
  struct oct_node_t *kids[8], *parent;
} oct_node_t;

typedef struct {
  int alloc, n;
  oct_node_t** buf;
} node_heap;

typedef struct {
  oct_node_t* pool;
  int len;
} oct_node_pool_t;

int cmp_node(oct_node_t* a, oct_node_t* b) {
  if (a->n_kids < b->n_kids)
    return -1;
  if (a->n_kids > b->n_kids)
    return 1;
  
  int ac = a->count >> a->depth;
  int bc = b->count >> b->depth;
  return ac < bc ? -1 : ac > bc;
}

void down_heap(node_heap* h, oct_node_t* p) {
  int n = p->heap_idx, m;
  
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

void up_heap(node_heap* h, oct_node_t* p) {
  int n = p->heap_idx;
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

void heap_add(node_heap* h, oct_node_t* p) {
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
    h->buf = realloc(h->buf, sizeof(oct_node_t*) * h->alloc);
  }
  
  p->heap_idx = h->n;
  h->buf[h->n++] = p;
  up_heap(h, p);
}

oct_node_t* pop_heap(node_heap* h) {
  if (h->n <= 1)
    return 0;
  
  oct_node_t* ret = h->buf[1];
  h->buf[1] = h->buf[--h->n];
  
  h->buf[h->n] = 0;
  
  h->buf[1]->heap_idx = 1;
  down_heap(h, h->buf[1]);
  
  return ret;
}

oct_node_t* node_new(oct_node_pool_t* pool, unsigned char idx, unsigned char depth, oct_node_t* p) {
  if (pool->len <= 1) {
    oct_node_t* p = calloc(sizeof(oct_node_t), 2048);
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

void free_node(oct_node_pool_t* pool) {
  oct_node_t* p;
  while (pool->pool) {
    p = pool->pool->parent;
    free(pool->pool);
    pool->pool = p;
  }
}

oct_node_t* node_insert(oct_node_pool_t* pool, oct_node_t* root, int* buf) {
  unsigned char i, bit, depth = 0;
  int c = *buf;
  int r = R(c), g = G(c), b = B(c);
  
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

oct_node_t* node_fold(oct_node_t* p) {
  if (p->n_kids) {
    error_handle(HIGH_PRIORITY, INVALID_PARAMETERS, "I don't know to be honest.");
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

void color_replace(oct_node_t* root, int* buf) {
  unsigned char i, bit;
  int c = *buf;
  int r = R(c), g = G(c), b = B(c);
  
  for (bit = 1 << 7; bit; bit >>= 1) {
    i = !!(r & bit) * 4 + !!(g & bit) * 2 + !!(b & bit);
    if (!root->kids[i])
      break;
    root = root->kids[i];
  }
  
  *buf = RGB((int)root->r, (int)root->g, (int)root->b);
}

void sgl_quantization(surface_t a, int n_colors, surface_t* b) {
  surface_t tmp = (b ? *b : NULL);
  if (!tmp)
    tmp = a;
  if (tmp && a != tmp) {
    if (tmp->w != a->w || tmp->h != a->h)
      sgl_reset(tmp, a->w, a->h);
    if (!tmp->buf)
      sgl_surface(b, a->w, a->h);
    sgl_copy(a, b);
  }
  
  int i, *buf = a->buf;
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
  free(heap.buf);
}

void sgl_vline(surface_t s, int x, int y0, int y1, int col) {
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
    sgl_psetb(s, x, y, col);
}

void sgl_hline(surface_t s, int y, int x0, int x1, int col) {
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
    sgl_psetb(s, x, y, col);
}

void sgl_line(surface_t s, int x0, int y0, int x1, int y1, int col) {
  int a = A(col);
  if (!a)
    return;
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
    sgl_psetb(s, x0, y0, col);
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

void sgl_circle(surface_t s, int xc, int yc, int r, int col, bool fill) {
  int a = A(col);
  if (!a)
    return;

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
    sgl_psetb(s, xc - x, yc + y, col);
    sgl_psetb(s, xc - y, yc - x, col);
    sgl_psetb(s, xc + x, yc - y, col);
    sgl_psetb(s, xc + y, yc + x, col);

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

void sgl_ellipse(surface_t s, int xc, int yc, int rx, int ry, int col, bool fill) {
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

void sgl_ellipse_rect(surface_t s, int x0, int y0, int x1, int y1, int col, bool fill) {
  long a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;
#if defined(SGL_ENABLE_AA)
  float dx = 4 * (a - 1.0) * b * b, dy = 4 * (b1 + 1) * a * a;
  float err = b1 * a * a - dx + dy;
  int f, ed, i;
#else
  double dx = 4 * (1.0 - a) * b * b, dy = 4 * (b1 + 1) * a * a;
  double err = dx + dy + b1 * a * a, e2;
#endif
  
  if (a == 0 || b == 0)
    sgl_line(s, x0, y0, x1, y1, col);
  if (x0 > x1) {
    x0 = x1;
    x1 += a;
  }
  if (y0 > y1)
    y0 = y1;
  y0 += (b + 1) / 2;
  y1 = y0 - b1;
  a = 8 * a * a;
  b1 = 8 * b * b;

#if defined(SGL_ENABLE_AA)
  for (;;) {
    i  = fminf(dx, dy);
    ed = fmaxf(dx, dy);
    if (y0 == y1 + 1 && err > dy && a > b1)
      ed = 255 * 4. / a;
    else
      ed = 255 / (ed + 2 * ed * i * i / (4 * ed * ed + i * i));
    
    i = ACHAN(col, 255 - (ed * (int)fabsf(err + dx - dy)));
    sgl_psetb(s, x0, y0, i);
    sgl_psetb(s, x0, y1, i);
    sgl_psetb(s, x1, y0, i);
    sgl_psetb(s, x1, y1, i);
    
    if (fill) {
      sgl_hline(s, y0, x0, x1, col);
      sgl_hline(s, y1, x0, x1, col);
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
      x0++;
      x1--;
      err -= dx -= b1;
    }
  }
  
  if (--x0 == x1++)
    while (y0-y1 < b) {
      i = 255 * 4 * fabs(err + dx) / b1;
      sgl_psetb(s, x0, ++y0, i);
      sgl_psetb(s, x1, y0, i);
      sgl_psetb(s, x0, --y1, i);
      sgl_psetb(s, x1, y1, i);
      err += dy += a;
    }
#else
  do {
    sgl_psetb(s, x1, y0, col);
    sgl_psetb(s, x0, y0, col);
    sgl_psetb(s, x0, y1, col);
    sgl_psetb(s, x1, y1, col);
    
    if (fill) {
      sgl_hline(s, y0, x0, x1, col);
      sgl_hline(s, y1, x0, x1, col);
    }
    
    e2 = 2*err;
    if (e2 <= dy) {
      y0++;
      y1--;
      err += dy += a;
    }
    if (e2 >= dx || 2*err > dy) {
      x0++;
      x1--;
      err += dx += b1;
    }
  } while (x0 <= x1);
  
  while (y0-y1 <= b) {
    sgl_psetb(s, x0-1, y0, col);
    sgl_psetb(s, x1+1, y0++, col);
    sgl_psetb(s, x0-1, y1, col);
    sgl_psetb(s, x1+1, y1--, col);
  }
#endif
}

static inline void bezier_seg(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, int col) {
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
      sgl_psetb(s, x0, y0, col);
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

void sgl_bezier(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, int col) {
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

static inline void bezier_seg_rational(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, float w, int col) {
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
      sgl_psetb(s, x0, y0, col);
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

void sgl_bezier_rational(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, float w, int col) {
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

void sgl_ellipse_rect_rotated(surface_t s, int x0, int y0, int x1, int y1, long zd, int col) {
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

void sgl_ellipse_rotated(surface_t s, int x, int y, int a, int b, float angle, int col) {
  float xd = (long)a * a, yd = (long)b * b;
  float q = sinf(angle), zd = (xd - yd) * q;
  xd = sqrtf(xd - zd * q);
  yd = sqrtf(yd + zd * q);
  a = xd + .5f;
  b = yd + .5f;
  zd = zd * a * b / (xd * yd);
  sgl_ellipse_rect_rotated(s, x - a, y - b, x + a, y + b, (long)(4 * zd * cosf(angle)), col);
}

static inline void bezier_seg_cubic(surface_t s, int x0, int y0, float x1, float y1, float x2, float y2, int x3, int y3, int col) {
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
      sgl_psetb(s, x0, y0, col);
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

void sgl_bezier_cubic(surface_t s, int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, int col) {
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

void sgl_rect(surface_t s, int x, int y, int w, int h, int col, bool fill) {
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

#define BMP_SET(c) ((*s)->buf[(i - (i % info.width)) + (info.width - (i % info.width) - 1)] = (c));

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

#warning TODO: Add support for OS/2 bitmaps

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
#warning TODO: Add 1 & 4 bpp support
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
          FREE_SAFE(color_map);
          sgl_destroy(s);
          return false;
      }
      break;
    case 1: // RLE8
    case 2: // RLE4
    default:
#warning TODO: Add RLE support
      error_handle(NORMAL_PRIORITY, UNSUPPORTED_BMP, "bmp() failed. Unsupported compression: %d", info.compression);
      FREE_SAFE(color_map);
      sgl_destroy(s);
      return false;
  }

  FREE_SAFE(color_map);
  return true;
}

bool sgl_save_bmp(surface_t s, const char* path) {
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
#warning FIXME: Alpha value wrong?
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
    case 9472 ... 9599: // Box
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

void sgl_ascii(surface_t s, char ch, int x, int y, int fg, int bg) {
  int c = letter_index((int)ch), i, j;
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 8; ++j) {
      if (font[c][i] & 1 << j) {
        sgl_psetb(s, x + j, y + i, fg);
      } else {
        if (bg == -1)
          continue;
        sgl_psetb(s, x + j, y + i, bg);
      }
    }
  }
}

int sgl_character(surface_t s, const char* ch, int x, int y, int fg, int bg) {
  int u = -1;
  int l = ctoi(ch, &u);
  int uc = letter_index(u), i, j;
  for (i = 0; i < 8; ++i)
    for (j = 0; j < 8; ++j) {
      if (font[uc][i] & 1 << j)
        sgl_psetb(s, x + j, y + i, fg);
      else {
        if (bg == -1)
          continue;
        sgl_psetb(s, x + j, y + i, bg);
      }
    }

  return l;
}

void sgl_writeln(surface_t s, int x, int y, int fg, int bg, const char* str) {
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

void sgl_writelnf(surface_t s, int x, int y, int fg, int bg, const char* fmt, ...) {
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

void sgl_string(surface_t* s, int fg, int bg, const char* str) {
  int w, h;
  str_size(str, &w, &h);
  sgl_surface(s, w * 8, h * LINE_HEIGHT);
  sgl_fill(*s, (bg == -1 ? 0 : bg));
  sgl_writeln(*s, 0, 0, fg, bg, str);
}

void sgl_stringf(surface_t* s, int fg, int bg, const char* fmt, ...) {
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

  sgl_string(s, fg, bg, buffer);
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

int sgl_bdf_character(surface_t s, struct bdf_t* f, const char* ch, int x, int y, int fg, int bg) {
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
        sgl_psetb(s, x + j, y + yy, (cc & i ? fg : bg));
    }
  }

  return l;
}

void sgl_bdf_writeln(surface_t s, struct bdf_t* f, int x, int y, int fg, int bg, const char* str) {
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

void sgl_bdf_writelnf(surface_t s, struct bdf_t* f, int x, int y, int fg, int bg, const char* fmt, ...) {
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

void sgl_bdf_string(surface_t* s, struct bdf_t* f, int fg, int bg, const char* str) {
  int w, h;
  str_size(str, &w, &h);
  sgl_surface(s, w * 8, h * LINE_HEIGHT);
  sgl_fill(*s, (bg == -1 ? 0 : bg));
  sgl_bdf_writeln(*s, f, 0, 0, fg, bg, str);
}

void sgl_bdf_stringf(surface_t* s, struct bdf_t* f, int fg, int bg, const char* fmt, ...) {
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

  sgl_bdf_string(s, f, fg, bg, buffer);
  free(buffer);
}
#endif

#if defined(SGL_ENABLE_FREETYPE)
#include <ft2build.h>
#include FT_FREETYPE_H

#warning TODO: Fixed unicode for FreeType

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

bool sgl_ft_init() {
  if (FT_Init_FreeType(&ft_library)) {
    error_handle(HIGH_PRIORITY, FT_INIT_FAILED, "FT_Init_FreeType() failed");
    return false;
  }
  return true;
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
  
  if (ftfont_char_index(font, u) >= 0) {
    error_handle(HIGH_PRIORITY, FT_LOAD_CHAR_FAILED, "ftfont_char_index() failed");
    return 0;
  }
  if (FT_Load_Char(font->face, u, FT_LOAD_RENDER)) {
    error_handle(HIGH_PRIORITY, FT_LOAD_CHAR_FAILED, "FT_Load_Char() failed");
    return 0;
  }
  
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
      sgl_psetb(new.buffer, i, j, b ? RGBA1(0, b) : 0);
    }
  
  stb_sb_push(font->encoding_table, u);
  stb_sb_push(font->chars, new);
  return stb_sb_count(font->encoding_table) - 1;
}

void sgl_ftfont(struct ftfont_t** _font, const char* path, unsigned int size) {
  struct ftfont_t* font = *_font = malloc(sizeof(struct ftfont_t));
  if (!font) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return;
  }
  
  if (FT_New_Face(ft_library, path, 0, &font->face)) {
    error_handle(HIGH_PRIORITY, FT_LOAD_FONT_FAILED, "FT_New_Face() failed");
    free(font);
    *_font = NULL;
    return;
  }
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
    return;
  
  struct ftfont_t* font = *_font;
  FT_Done_Face(font->face);
  stb_sb_free(font->encoding_table);
  for (int i = 0; i < stb_sb_count(font->chars); ++i)
    sgl_destroy(&font->chars[i].buffer);
  stb_sb_free(font->chars);
  free(font);
}

int sgl_ftfont_character(surface_t s, ftfont_t f, const char* ch, int x, int y, int fg, int bg, int* w, int* h) {
  int u = -1, i, j;
  int l = ctoi(ch, &u);
  int index = ftfont_char_index(f, u);
  if (index == -1)
    index = load_ftfont_char(f, ch);
  
  ft_char_t* c = &f->chars[index];
  y -= c->bearing.y;
  for (i = 0; i < c->size.x; ++i) {
    for (j = 0; j < c->size.y; ++j) {
      sgl_psetb(s, x + i, y + j, bg);
#warning TODO: Find better solution than called pset twice
#warning TODO: Update other font renderers to fix alpha
      sgl_psetb(s, x + i, y + j, ACHAN(fg, CLAMP(A(fg) - (255 - A(XYGET(((c->buffer)), i, j))), 0, 255)));
    }
  }
  
  if (w)
    *w = (int)(c->advance >> 6);
  if (h)
    *h = c->size.y;
  return l;
}

void sgl_ftfont_writeln(surface_t s, ftfont_t f, int x, int y, int fg, int bg, const char* str) {
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

void sgl_ftfont_writelnf(surface_t s, ftfont_t f, int x, int y, int fg, int bg, const char* fmt, ...) {
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

void sgl_ftfont_string(surface_t* s, ftfont_t f, int fg, int bg, const char* str) {
  const char* _str = (const char*)str;
  int n = 0, m = 0, nn = 0, mm = 6, c, len, index;
  ft_char_t* ch = NULL;
  while (_str && *_str != '\0') {
    c = *_str;
    if (c >= 0 && c <= 127) {
      switch (c) {
        case '\n':
          if (ch)
            nn -= (int)(ch->advance >> 6);
          if (nn > n)
            n = nn;
          nn = 0;
          m += (mm + 6);
          _str++;
          break;
        case '\f':
        case '\b':
          if (read_color(_str, NULL, &len)) {
            _str += len;
            continue;
          }
          else
            _str++;
          break;
        default:
          index = ftfont_char_index(f, c);
          if (index == -1)
            index = load_ftfont_char(f, _str);
          
          ch = &f->chars[index];
          nn += (int)(ch->advance >> 6);
          if (ch->size.y > mm)
            mm = ch->size.y;
          _str++;
          break;
      }
    } else {
      len = ctoi(_str, &c);
      index = ftfont_char_index(f, c);
      if (index == -1)
        index = load_ftfont_char(f, _str);
#warning TODO: Unicode support for FreeType
    }
  }
  if (nn > n)
    n = nn;
  m += mm + 6;
  
  sgl_surface(s, n, m);
  sgl_fill(*s, bg);
  sgl_ftfont_writeln(*s, f, 0, mm, fg, 0, str);
}

void sgl_ftfont_stringf(surface_t* s, ftfont_t f, int fg, int bg, const char* fmt, ...) {
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
  
  sgl_ftfont_string(s, f, fg, bg, buffer);
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

bool sgl_image(surface_t* s, const char* path) {
  int w, h, c, x, y;
  unsigned char* data = stbi_load(path, &w, &h, &c, 0);
  if (!data) {
    error_handle(NORMAL_PRIORITY, STBI_LOAD_FAILED, "stbi_load() failed: %s", stbi_failure_reason());
    return false;
  }

  if (!sgl_surface(s, w, h)) {
    stbi_image_free(data);
    return false;
  }

  surface_t tmp = *s;
  unsigned char* p = NULL;
  for (x = 0; x < w; ++x) {
    for (y = 0; y < h; ++y) {
      p = data + (x + w * y) * c;
      tmp->buf[y * w + x] = RGBA(p[0], p[1], p[2], (c == 4 ? p[3] : 255));
    }
  }

  stbi_image_free(data);
  return true;
}

bool sgl_save_image(surface_t a, const char* path, SAVEFORMAT type) {
  if (!a || !path) {
    error_handle(NORMAL_PRIORITY, INVALID_PARAMETERS, "save_image() failed: Invalid parameters");
    return false;
  }
  
  unsigned char* data = malloc(a->w * a->h * 4 * sizeof(unsigned char));
  if (!data) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "save_image() failed: Out of memory");
    return false;
  }

  unsigned char* p = NULL;
  int i, j, c;
  for (i = 0; i < a->w; ++i) {
    for (j = 0; j < a->h; ++j) {
      p = data + (i + a->w * j) * 4;
      c = a->buf[j * a->w + i];
      p[0] = R(c);
      p[1] = G(c);
      p[2] = B(c);
      p[3] = A(c);
    }
  }

  int res = 0;
  switch (type) {
  default:
  case PNG:
    res = stbi_write_png(path, a->w, a->h, 4, data, 0);
    break;
  case TGA:
    res = stbi_write_tga(path, a->w, a->h, 4, data);
    break;
  case BMP:
    res = stbi_write_bmp(path, a->w, a->h, 4, data);
    break;
  case JPG:
    res = stbi_write_jpg(path, a->w, a->h, 4, data, 85);
    break;
  }
  FREE_SAFE(data);
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
#define GIF_MGET(m,s,a,c) m = (unsigned char*)realloc((c)? 0 : m, (c)? s : 0UL);
#endif
#ifndef GIF_BIGE
#define GIF_BIGE 0
#endif
#ifndef GIF_EXTR
#define GIF_EXTR static
#endif
#define _GIF_SWAP(h) ((GIF_BIGE)? ((unsigned short)(h << 8) | (h >> 8)) : h)

struct gif_t {
  int delay, frames, frame, w, h;
  surface_t* surfaces;
};

int sgl_gif_delay(gif_t g) {
  return g->delay;
}

int sgl_gif_total_frames(gif_t g) {
  return g->frames;
}

int sgl_gif_current_frame(gif_t g) {
  return g->frame;
}

void sgl_gif_size(gif_t g, int* w, int* h) {
  if (w)
    *w = g->w;
  if (h)
    *h = g->h;
}

int sgl_gif_next_frame(gif_t g) {
  g->frame++;
  if (g->frame >= g->frames)
    g->frame = 0;
  return g->frame;
}

void sgl_gif_set_frame(gif_t g, int n) {
  if (n >= g->frames || n < 0)
    return;
  g->frame = n;
}

surface_t sgl_gif_frame(gif_t g) {
  return g->surfaces[g->frame];
}

void sgl_gif_create(gif_t* _g, int w, int h, int delay, int frames, ...) {
  gif_t g = *_g = malloc(sizeof(gif_t));
  if (!g) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    *_g = NULL;
    return;
  }
  g->delay = delay;
  g->frame = 0;
  g->frames = frames;
  g->h = h;
  g->w = w;
  g->surfaces = calloc(g->frames, sizeof(surface_t));
  if (!g->surfaces) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    FREE_SAFE(g);
    *_g = NULL;
    return;
  }
  
  va_list ap;
  va_start(ap, frames);
  for (int i = 0; i < frames; ++i) {
    surface_t tmp = va_arg(ap, surface_t);
    sgl_copy(tmp, &g->surfaces[i]);
  }
  va_end(ap);
}

#pragma pack(push, 1)
struct GIF_WHDR {         /** ======== frame writer info: ======== **/
  long xdim, ydim, clrs,  /** global dimensions, palette size      **/
  bkgd, tran,             /** background index, transparent index  **/
  intr, mode,             /** interlace flag, frame blending mode  **/
  frxd, fryd, frxo, fryo, /** current frame dimensions and offset  **/
  time, ifrm, nfrm;       /** delay, frame number, frame count     **/
  unsigned char *bptr;          /** frame pixel indices or metadata      **/
  struct {                /** [==== GIF RGB palette element: ====] **/
    unsigned char R, G, B;      /** [color values - red, green, blue   ] **/
  } *cpal;                /** current palette                      **/
};
#pragma pack(pop)

enum {GIF_NONE = 0, GIF_CURR = 1, GIF_BKGD = 2, GIF_PREV = 3};

static long _GIF_SkipChunk(unsigned char **buff, long size) {
  long skip;
  
  for (skip = 2, ++size, ++(*buff); ((size -= skip) > 0) && (skip > 1);
       *buff += (skip = 1 + **buff));
  return size;
}

static long _GIF_LoadHeader(unsigned gflg, unsigned char **buff, void **rpal,
                            unsigned fflg, long *size, long flen) {
  if (flen && (!(*buff += flen) || ((*size -= flen) <= 0)))
    return -2;  /** v--[ 0x80: "palette is present" flag ]--, **/
  if (flen && (fflg & 0x80)) { /** local palette has priority | **/
    *rpal = *buff; /** [ 3L: 3 unsigned char color channels ]--,  | **/
    *buff += (flen = 2 << (fflg & 7)) * 3L;       /** <--|  | **/
    return ((*size -= flen * 3L) > 0)? flen : -1; /** <--'  | **/
  } /** no local palette found, checking for the global one   | **/
  return (gflg & 0x80)? (2 << (gflg & 7)) : 0;      /** <-----' **/
}

static long _GIF_LoadFrame(unsigned char **buff, long *size,
                           unsigned char *bptr, unsigned char *blen) {
  typedef unsigned short GIF_H;
  const long GIF_HLEN = sizeof(GIF_H), /** to rid the scope of sizeof **/
  GIF_CLEN = 1 << 12;    /** code table length: 4096 items **/
  GIF_H accu, mask; /** bit accumulator / bit mask                    **/
  long  ctbl, iter, /** last code table index / index string iterator **/
  prev, curr, /** codes from the stream: previous / current     **/
  ctsz, ccsz, /** code table bit sizes: min LZW / current       **/
  bseq, bszc; /** counters: block sequence / bit size           **/
  unsigned int *code = (unsigned int*)bptr - GIF_CLEN; /** code table pointer **/
  
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
            code[ctbl] = (unsigned int)prev + (code[prev] & 0xFFF000);
          } /** appending SP / MP decoded pixels to the frame **/
          prev = (long)code[iter = (ctbl > curr)? curr : prev];
          if ((bptr += (prev = (prev >> 12) & 0xFFF)) > blen)
            continue; /** skipping pixels above frame capacity **/
          for (prev++; (iter &= 0xFFF) >> ctsz;
               *bptr-- = (unsigned char)((iter = (long)code[iter]) >> 24));
          (bptr += prev)[-prev] = (unsigned char)iter;
          if (ctbl < GIF_CLEN) { /** appending the code table **/
            if (ctbl == curr)
              *bptr++ = (unsigned char)iter;
            else if (ctbl < curr)
              return -5; /** wrong code in the stream **/
            code[ctbl++] += ((unsigned int)iter << 24) + 0x1000;
          }
        } /** 0: no ED before end-of-stream mark; -4: see above **/
  return (++(*size) >= 0)? 0 : -4; /** ^- N.B.: 0 error is recoverable **/
}

GIF_EXTR long GIF_Load(void *data, long size,
                       void (*gwfr)(void*, struct GIF_WHDR*),
                       void (*eamf)(void*, struct GIF_WHDR*),
                       void *anim, long skip) {
  const long    GIF_BLEN = (1 << 12) * sizeof(unsigned int);
  const unsigned char GIF_EHDM = 0x21, /** extension header mark **/
  GIF_FHDM = 0x2C, /** frame header mark                  **/
  GIF_EOFM = 0x3B, /** end-of-file mark                   **/
  GIF_EGCM = 0xF9, /** extension: graphics control mark   **/
  GIF_EAMM = 0xFF; /** extension: app metadata mark       **/
#pragma pack(push, 1)
  struct GIF_GHDR {      /** ========== GLOBAL GIF HEADER: ========== **/
    unsigned char head[6];     /** 'GIF87a' / 'GIF89a' header signature     **/
    unsigned short xdim, ydim; /** total image width, total image height    **/
    unsigned char flgs;        /** FLAGS:
                          GlobalPlt    bit 7     1: global palette exists
                          0: local in each frame
                          ClrRes       bit 6-4   bits/channel = ClrRes+1
                          [reserved]   bit 3     0
                          PixelBits    bit 2-0   |Plt| = 2 * 2^PixelBits
                          **/
    unsigned char bkgd, aspr;  /** background color index, aspect ratio     **/
  } *ghdr = (struct GIF_GHDR*)data;
  struct GIF_FHDR {      /** ======= GIF FRAME MASTER HEADER: ======= **/
    unsigned short frxo, fryo; /** offset of this frame in a "full" image   **/
    unsigned short frxd, fryd; /** frame width, frame height                **/
    unsigned char flgs;        /** FLAGS:
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
    unsigned char flgs;        /** FLAGS:
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
    unsigned short time;       /** delay in GIF time units; 1 unit = 10 ms  **/
    unsigned char tran;        /** transparent color index                  **/
  } *egch = 0;
#pragma pack(pop)
  struct GIF_WHDR wtmp, whdr = {0};
  long desc, blen;
  unsigned char *buff;
  
  /** checking if the stream is not empty and has a 'GIF8[79]a' signature,
   the data has sufficient size and frameskip value is non-negative **/
  if (!ghdr || (size <= (long)sizeof(*ghdr)) || (*(buff = ghdr->head) != 71)
      || (buff[1] != 73) || (buff[2] != 70) || (buff[3] != 56) || (skip < 0)
      || ((buff[4] != 55) && (buff[4] != 57)) || (buff[5] != 97) || !gwfr)
    return 0;
  
  buff = (unsigned char*)(ghdr + 1) /** skipping the global header and palette **/
  + _GIF_LoadHeader(ghdr->flgs, 0, 0, 0, 0, 0L) * 3L;
  if ((size -= buff - (unsigned char*)ghdr) <= 0)
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
  gif_t out;
} gif_data_t;
#pragma pack(pop)

void load_gif_frame(void* data, struct GIF_WHDR* whdr) {
  unsigned int *pict, *prev, x, y, yoff, iter, ifin, dsrc, ddst;
  gif_data_t* gif = (gif_data_t*)data;
  
#define BGRA(i) \
  ((whdr->bptr[i] == whdr->tran)? 0 : \
    ((unsigned int)(whdr->cpal[whdr->bptr[i]].R << ((GIF_BIGE)? 8 : 16)) \
  |  (unsigned int)(whdr->cpal[whdr->bptr[i]].G << ((GIF_BIGE)? 16 : 8)) \
  |  (unsigned int)(whdr->cpal[whdr->bptr[i]].B << ((GIF_BIGE)? 24 : 0)) \
  |  ((GIF_BIGE)? 0xFF : 0xFF000000)))
  
  if (!whdr->ifrm) {
    gif->out->delay = (int)whdr->time;
    gif->out->w = (int)whdr->xdim;
    gif->out->h = (int)whdr->ydim;
    gif->out->frames = (int)whdr->nfrm;
    gif->out->frame  = 0;
    ddst = (unsigned int)(whdr->xdim * whdr->ydim);
    gif->pict = calloc(sizeof(unsigned int), ddst);
    gif->prev = calloc(sizeof(unsigned int), ddst);
    gif->out->surfaces = calloc(gif->out->frames, sizeof(surface_t));
  }
  
  pict = (unsigned int*)gif->pict;
  ddst = (unsigned int)(whdr->xdim * whdr->fryo + whdr->frxo);
  ifin = (!(iter = (whdr->intr)? 0 : 4))? 4 : 5; /** interlacing support **/
  for (dsrc = (unsigned int)-1; iter < ifin; iter++)
    for (yoff = 16U >> ((iter > 1)? iter : 1), y = (8 >> iter) & 7;
         y < (unsigned int)whdr->fryd; y += yoff)
      for (x = 0; x < (unsigned int)whdr->frxd; x++)
        if (whdr->tran != (long)whdr->bptr[++dsrc])
          pict[(unsigned int)whdr->xdim * y + x + ddst] = BGRA(dsrc);
  
  int this = (int)whdr->ifrm;
  sgl_surface(&gif->out->surfaces[this], gif->out->w, gif->out->h);
  memcpy(gif->out->surfaces[this]->buf, pict, whdr->xdim * whdr->ydim * sizeof(unsigned int));
  
  if ((whdr->mode == GIF_PREV) && !gif->last) {
    whdr->frxd = whdr->xdim;
    whdr->fryd = whdr->ydim;
    whdr->mode = GIF_BKGD;
    ddst = 0;
  }
  else {
    gif->last = (whdr->mode == GIF_PREV)?
    gif->last : (unsigned long)(whdr->ifrm + 1);
    pict = (unsigned int*)((whdr->mode == GIF_PREV)? gif->pict : gif->prev);
    prev = (unsigned int*)((whdr->mode == GIF_PREV)? gif->prev : gif->pict);
    for (x = (unsigned int)(whdr->xdim * whdr->ydim); --x;
         pict[x - 1] = prev[x - 1]);
  }
  
  if (whdr->mode == GIF_BKGD) /** cutting a hole for the next frame **/
    for (whdr->bptr[0] = (unsigned char)((whdr->tran >= 0)?
                                   whdr->tran : whdr->bkgd), y = 0,
         pict = (unsigned int*)gif->pict; y < (unsigned int)whdr->fryd; y++)
      for (x = 0; x < (unsigned int)whdr->frxd; x++)
        pict[(unsigned int)whdr->xdim * y + x + ddst] = BGRA(0);
#undef BGRA
}

bool sgl_gif(gif_t* _g, const char* path) {
  gif_t g = *_g = malloc(sizeof(struct gif_t));
  
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

/* helper to write a little-endian 16-bit number portably */
#define gif_write_num(fd, n) fwrite((unsigned char []) {(n) & 0xFF, (n) >> 8}, 2, 1, fd)

static unsigned char vga[0x30] = {
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
  unsigned short key;
  struct ge_Node* children[];
} ge_Node;

typedef struct ge_GIF {
  unsigned short w, h;
  int depth;
  FILE* fd;
  int offset;
  int nframes;
  unsigned char *frame, *back;
  unsigned int partial;
  unsigned char buffer[0xFF];
} ge_GIF;

static ge_Node* new_node(unsigned short key, int degree) {
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

static void put_loop(ge_GIF* gif, unsigned short loop) {
  fwrite((unsigned char []) {'!', 0xFF, 0x0B}, 3, 1, gif->fd);
  fwrite("NETSCAPE2.0", 11, 1, gif->fd);
  fwrite((unsigned char []) {0x03, 0x01}, 2, 1, gif->fd);
  gif_write_num(gif->fd, loop);
  fwrite("\0", 1, 1, gif->fd);
}

ge_GIF* ge_new_gif(const char *fname, unsigned short width, unsigned short height, unsigned char* palette, int depth, int loop) {
  int i, r, g, b, v;
  ge_GIF *gif = calloc(1, sizeof(*gif) + 2*width*height);
  if (!gif)
    goto no_gif;
  gif->w = width; gif->h = height;
  gif->depth = depth > 1 ? depth : 2;
  gif->frame = (unsigned char *) &gif[1];
  gif->back = &gif->frame[width*height];
  gif->fd = fopen(fname, "wb");
  if (!gif->fd)
    goto no_fd;
#ifdef _WIN32
  setmode(gif->fd, O_BINARY);
#endif
  fwrite("GIF89a", 6, 1, gif->fd);
  gif_write_num(gif->fd, width);
  gif_write_num(gif->fd, height);
  fwrite((unsigned char []) {0xF0 | (depth-1), 0x00, 0x00}, 3, 1, gif->fd);
  if (palette) {
    fwrite(palette, 3 << depth, 1, gif->fd);
  } else if (depth <= 4) {
    fwrite(vga, 3 << depth, 1, gif->fd);
  } else {
    fwrite(vga, sizeof(vga), 1, gif->fd);
    i = 0x10;
    for (r = 0; r < 6; r++) {
      for (g = 0; g < 6; g++) {
        for (b = 0; b < 6; b++) {
          fwrite((unsigned char []) {r*51, g*51, b*51}, 3, 1, gif->fd);
          if (++i == 1 << depth)
            goto done_gct;
        }
      }
    }
    for (i = 1; i <= 24; i++) {
      v = i * 0xFF / 25;
      fwrite((unsigned char []) {v, v, v}, 3, 1, gif->fd);
    }
  }
done_gct:
  if (loop >= 0 && loop <= 0xFFFF)
    put_loop(gif, (unsigned short) loop);
  return gif;
no_fd:
  free(gif);
no_gif:
  return NULL;
}

/* Add packed key to buffer, updating offset and partial.
 *   gif->offset holds position to put next *bit*
 *   gif->partial holds bits to include in next byte */
static void put_key(ge_GIF* gif, unsigned short key, int key_size) {
  int byte_offset, bit_offset, bits_to_write;
  byte_offset = gif->offset / 8;
  bit_offset = gif->offset % 8;
  gif->partial |= ((unsigned int) key) << bit_offset;
  bits_to_write = bit_offset + key_size;
  while (bits_to_write >= 8) {
    gif->buffer[byte_offset++] = gif->partial & 0xFF;
    if (byte_offset == 0xFF) {
      fwrite("\xFF", 1, 1, gif->fd);
      fwrite(gif->buffer, 0xFF, 1, gif->fd);
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
  fwrite((unsigned char []) {byte_offset}, 1, 1, gif->fd);
  fwrite(gif->buffer, byte_offset, 1, gif->fd);
  fwrite("\0", 1, 1, gif->fd);
  gif->offset = gif->partial = 0;
}

static void put_image(ge_GIF* gif, unsigned short w, unsigned short h, unsigned short x, unsigned short y) {
  int nkeys, key_size, i, j;
  ge_Node *node, *child, *root;
  int degree = 1 << gif->depth;
  
  fwrite(",", 1, 1, gif->fd);
  gif_write_num(gif->fd, x);
  gif_write_num(gif->fd, y);
  gif_write_num(gif->fd, w);
  gif_write_num(gif->fd, h);
  fwrite((unsigned char []) {0x00, gif->depth}, 2, 1, gif->fd);
  root = node = new_trie(degree, &nkeys);
  key_size = gif->depth + 1;
  put_key(gif, degree, key_size); /* clear code */
  for (i = y; i < y+h; i++) {
    for (j = x; j < x+w; j++) {
      unsigned char pixel = gif->frame[i*gif->w+j] & (degree - 1);
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

static int get_bbox(ge_GIF* gif, unsigned short* w, unsigned short* h, unsigned short* x, unsigned short* y) {
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

static void set_delay(ge_GIF* gif, unsigned short d) {
  fwrite((unsigned char []) {'!', 0xF9, 0x04, 0x04}, 4, 1, gif->fd);
  gif_write_num(gif->fd, d);
  fwrite("\0\0", 2, 1, gif->fd);
}

void ge_add_frame(ge_GIF* gif, unsigned short delay) {
  unsigned short w, h, x, y;
  unsigned char *tmp;
  
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
  fwrite(";", 1, 1, gif->fd);
  fclose(gif->fd);
  free(gif);
}

bool sgl_save_gif(gif_t* __g, const char* path) {
  gif_t _g = *__g;
  int i, j, k, c, cp;
  unsigned char r, g, b;
  
  for (i = 0; i < _g->frames; ++i) {
    if (_g->surfaces[i]->w != _g->w || _g->surfaces[i]->h != _g->h) {
      error_handle(NORMAL_PRIORITY, GIF_SAVE_INVALID_SIZE, "Sizes of surfaces in GIF don't match");
      return false;
    }
  }
  
  unsigned char* palette = NULL;
  for (i = 0; i < _g->frames; ++i) {
    for (j = 0; j < _g->w * _g->h; ++j) {
      c = _g->surfaces[i]->buf[j];
      r = (unsigned char)R(c);
      g = (unsigned char)G(c);
      b = (unsigned char)B(c);
      
      cp = -1;
      for (k = 0; k < stb_sb_count(palette); k += 3) {
        if (r == palette[k] && g == palette[k + 1] && b == palette[k + 2]) {
          cp = k;
          break;
        }
      }
      
      if (cp < 0) {
        cp = stb_sb_count(palette);
        stb_sb_push(palette, r);
        stb_sb_push(palette, g);
        stb_sb_push(palette, b);
      }
    }
  }
  
  int depth = (int)log2((double)(stb_sb_count(palette) / 3));
  if (!depth) {
    error_handle(LOW_PRIORITY, GIF_SAVE_FAILED, "sgl_save_gif() failed: Couldn't get depth of the image?");
    return false;
  }
  else if (depth > 8) {
    surface_t tmp_s;
    sgl_surface(&tmp_s, _g->w, _g->h * _g->frames);

    point_t tmp_p = {0};
    for (i = 0; i < _g->frames; ++i) {
      tmp_p.y = _g->h * i;
      sgl_blit(tmp_s, &tmp_p, _g->surfaces[i], NULL);
    }

    sgl_quantization(tmp_s, 257, NULL);

    rect_t tmp_r = { 0, 0, _g->w, _g->h };
    for (i = 0; i < _g->frames; ++i) {
      tmp_r.y = _g->h * i;
      sgl_blit(_g->surfaces[i], NULL, tmp_s, &tmp_r);
    }
    sgl_destroy(&tmp_s);
    stb_sb_free(palette);
    
    return sgl_save_gif(__g, path);
  }
  
  int** frames = malloc(sizeof(int*) * _g->frames);
  for (i = 0; i < _g->frames; ++i) {
    frames[i] = malloc(_g->w * _g->h * sizeof(int));
    for (j = 0; j < _g->w * _g->h; ++j) {
      c = _g->surfaces[i]->buf[j];
      r = (unsigned char)R(c);
      g = (unsigned char)G(c);
      b = (unsigned char)B(c);
      
      for (k = 0; k < stb_sb_count(palette); k += 3) {
        if (r == palette[k] && g == palette[k + 1] && b == palette[k + 2]) {
          frames[i][j] = k / 3;
          break;
        }
      }
    }
  }
  
  ge_GIF* out = ge_new_gif(path, _g->w, _g->h, palette, depth, 0);
  if (!out) {
    for (i = 0; i < _g->frames; ++i)
      free(frames[i]);
    free(frames);
    error_handle(HIGH_PRIORITY, GIF_SAVE_FAILED, "Failed to create GIF");
    return false;
  }
  
  for (i = 0; i < _g->frames; ++i) {
    for (j = 0; j < _g->w * _g->h; ++j)
      out->frame[j] = frames[i][j];
    ge_add_frame(out, _g->delay);
  }
  ge_close_gif(out);
  
  for (i = 0; i < _g->frames; ++i)
    free(frames[i]);
  free(frames);
  stb_sb_free(palette);
  return true;
}

void sgl_gif_destroy(gif_t* _g) {
  gif_t g = *_g;
  if (!g)
    return;
  for (int i = 0; i < g->frames; ++i)
    sgl_destroy(&g->surfaces[i]);
  FREE_SAFE(g->surfaces);
  FREE_SAFE(g);
}
#endif

#if defined(SGL_ENABLE_JOYSTICKS)
#if defined(SGL_OSX)
#include <IOKit/hid/IOHIDLib.h>
#include <limits.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

struct HIDGamepadAxis {
  IOHIDElementCookie cookie;
  CFIndex logicalMin;
  CFIndex logicalMax;
  bool hasNullState;
  bool isHatSwitch;
  bool isHatSwitchSecondAxis;
};

struct HIDGamepadButton {
  IOHIDElementCookie cookie;
};
#endif

struct joystick_t {
  const char* description;
  int device_id, vendor_id, product_id;
  
  unsigned int n_axes, n_buttons;
  float* axes;
  int* buttons;
  
#if defined(SGL_OSX)
  IOHIDDeviceRef deviceRef;
  struct HIDGamepadAxis* axisElements;
  struct HIDGamepadButton* buttonElements;
#endif
};
static joystick_t* joysticks = NULL;
static int joysticks_sz = 0;

#define X(a, b, c) \
static void(*joy_##a##_callback)b = NULL;
XMAP_JOYSTICK_CB
#undef X

#define X(a, b, c) \
void(*a##_cb)b,
void sgl_joystick_callbacks(XMAP_JOYSTICK_CB int dummy) {
#undef X
#define X(a, b, c) \
joy_##a##_callback = a##_cb;
  XMAP_JOYSTICK_CB
#undef X
}

#define X(a, b, c)\
void sgl_##c##_callback(void(*a##_cb)b) { \
  joy_##a##_callback = a##_cb; \
}
XMAP_JOYSTICK_CB
#undef X

#if defined(SGL_OSX)
static IOHIDManagerRef hid = NULL;
#define GAMEPAD_RUN_LOOP_MODE CFSTR("GamepadRunLoopMode")

static int IOHIDDeviceGetIntProperty(IOHIDDeviceRef deviceRef, CFStringRef key) {
  CFTypeRef typeRef;
  int value;
  
  typeRef = IOHIDDeviceGetProperty(deviceRef, key);
  if (typeRef == NULL || CFGetTypeID(typeRef) != CFNumberGetTypeID()) {
    return 0;
  }
  
  CFNumberGetValue((CFNumberRef) typeRef, kCFNumberSInt32Type, &value);
  return value;
}

static int IOHIDDeviceGetVendorID(IOHIDDeviceRef deviceRef) {
  return IOHIDDeviceGetIntProperty(deviceRef, CFSTR(kIOHIDVendorIDKey));
}

static int IOHIDDeviceGetProductID(IOHIDDeviceRef deviceRef) {
  return IOHIDDeviceGetIntProperty(deviceRef, CFSTR(kIOHIDProductIDKey));
}

static void hat_val_xy(CFIndex v, CFIndex r, int* x, int* y) {
  *x = (v == r ? 0 : (v > 0 && v < r / 2 ? 1 : (v > r / 2 ? -1 : 0)));
  *y = (v == r ? 0 : (v > r / 4 * 3 || v < r / 4 ? -1 : (v > r / 4 && v < r / 4 * 3 ? 1 : 0)));
}

static void device_input(void* ctx, IOReturn result, void* src, IOHIDValueRef value) {
  joystick_t device;
  IOHIDElementRef element;
  IOHIDElementCookie cookie;
  unsigned int axisIndex, buttonIndex;
  
  static mach_timebase_info_data_t timebaseInfo;
  if (timebaseInfo.denom == 0) {
    mach_timebase_info(&timebaseInfo);
  }
  
  device = (joystick_t)ctx;
  element = IOHIDValueGetElement(value);
  cookie = IOHIDElementGetCookie(element);
  
  for (axisIndex = 0; axisIndex < device->n_axes; axisIndex++) {
    if (!device->axisElements[axisIndex].isHatSwitchSecondAxis && device->axisElements[axisIndex].cookie == cookie) {
      CFIndex integerValue;
      
      if (IOHIDValueGetLength(value) > 4) {
        // Workaround for a strange crash that occurs with PS3 controller; was getting lengths of 39 (!)
        continue;
      }
      integerValue = IOHIDValueGetIntegerValue(value);
      
      if (device->axisElements[axisIndex].isHatSwitch) {
        int x, y;
        
        // Fix for Saitek X52
        if (!device->axisElements[axisIndex].hasNullState) {
          if (integerValue < device->axisElements[axisIndex].logicalMin) {
            integerValue = device->axisElements[axisIndex].logicalMax - device->axisElements[axisIndex].logicalMin + 1;
          } else {
            integerValue--;
          }
        }
        
        hat_val_xy(integerValue, device->axisElements[axisIndex].logicalMax - device->axisElements[axisIndex].logicalMin + 1, &x, &y);
        
        if (x != device->axes[axisIndex]) {
          CALL(joy_axis_callback, device, device->device_id, axisIndex, x, device->axes[axisIndex], IOHIDValueGetTimeStamp(value) * timebaseInfo.numer / timebaseInfo.denom * 0.000000001)
          device->axes[axisIndex] = x;
        }
        
        if (y != device->axes[axisIndex + 1]) {
          CALL(joy_axis_callback, device, device->device_id, axisIndex + 1, y, device->axes[axisIndex + 1], IOHIDValueGetTimeStamp(value) * timebaseInfo.numer / timebaseInfo.denom * 0.000000001)
          device->axes[axisIndex + 1] = y;
        }
        
      } else {
        float floatValue;
        
        if (integerValue < device->axisElements[axisIndex].logicalMin) {
          device->axisElements[axisIndex].logicalMin = integerValue;
        }
        if (integerValue > device->axisElements[axisIndex].logicalMax) {
          device->axisElements[axisIndex].logicalMax = integerValue;
        }
        floatValue = (integerValue - device->axisElements[axisIndex].logicalMin) / (float) (device->axisElements[axisIndex].logicalMax - device->axisElements[axisIndex].logicalMin) * 2.0f - 1.0f;
        
        CALL(joy_axis_callback, device, device->device_id, axisIndex, floatValue, device->axes[axisIndex], IOHIDValueGetTimeStamp(value) * timebaseInfo.numer / timebaseInfo.denom * 0.000000001)
        device->axes[axisIndex] = floatValue;
      }
      
      return;
    }
  }
  
  for (buttonIndex = 0; buttonIndex < device->n_buttons; buttonIndex++) {
    if (device->buttonElements[buttonIndex].cookie == cookie) {
      bool down;
      
      down = IOHIDValueGetIntegerValue(value);
      CALL(joy_btn_callback, device, device->device_id, buttonIndex, down, IOHIDValueGetTimeStamp(value) * timebaseInfo.numer / timebaseInfo.denom * 0.000000001);
      device->buttons[buttonIndex] = down;
      
      return;
    }
  }
}

static void device_added(void* ctx, IOReturn result, void* src, IOHIDDeviceRef device) {
  CFArrayRef elements;
  CFIndex elementIndex;
  IOHIDElementRef element;
  CFStringRef cfProductName;
  joystick_t joystick;
  IOHIDElementType type;
  char* description;
  
  joystick = malloc(sizeof(struct joystick_t));
  if (!joystick) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return;
  }
  int i = 0;
  for (i = 0; i < joysticks_sz; ++i)
    if (!joysticks[i])
      break;
  if (i == joysticks_sz)
    return;
  joystick->device_id = i;
  joystick->vendor_id = IOHIDDeviceGetVendorID(device);
  joystick->product_id = IOHIDDeviceGetProductID(device);
  joystick->n_axes = 0;
  joystick->n_buttons = 0;
  joystick->deviceRef = device;
  joystick->axisElements = NULL;
  joystick->buttonElements = NULL;
  joysticks[i] = joystick;
  
  cfProductName = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
  if (cfProductName == NULL || CFGetTypeID(cfProductName) != CFStringGetTypeID()) {
    description = malloc(strlen("[Unknown]" + 1));
    strcpy(description, "[Unknown]");
    
  } else {
    CFIndex length;
    
    CFStringGetBytes(cfProductName, CFRangeMake(0, CFStringGetLength(cfProductName)), kCFStringEncodingUTF8, '?', false, NULL, 100, &length);
    description = malloc(length + 1);
    CFStringGetBytes(cfProductName, CFRangeMake(0, CFStringGetLength(cfProductName)), kCFStringEncodingUTF8, '?', false, (UInt8 *) description, length + 1, NULL);
    description[length] = '\x00';
  }
  joystick->description = description;
  
  elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
  for (elementIndex = 0; elementIndex < CFArrayGetCount(elements); elementIndex++) {
    element = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, elementIndex);
    type = IOHIDElementGetType(element);
    if (type == kIOHIDElementTypeInput_Misc ||
        type == kIOHIDElementTypeInput_Axis) {
      
      joystick->axisElements = realloc(joystick->axisElements, sizeof(struct HIDGamepadAxis) * (joystick->n_axes + 1));
      joystick->axisElements[joystick->n_axes].cookie = IOHIDElementGetCookie(element);
      joystick->axisElements[joystick->n_axes].logicalMin = IOHIDElementGetLogicalMin(element);
      joystick->axisElements[joystick->n_axes].logicalMax = IOHIDElementGetLogicalMax(element);
      joystick->axisElements[joystick->n_axes].hasNullState = !!IOHIDElementHasNullState(element);
      joystick->axisElements[joystick->n_axes].isHatSwitch = IOHIDElementGetUsage(element) == kHIDUsage_GD_Hatswitch;
      joystick->axisElements[joystick->n_axes].isHatSwitchSecondAxis = false;
      joystick->n_axes++;
      
      if (joystick->axisElements[joystick->n_axes - 1].isHatSwitch) {
        joystick->axisElements = realloc(joystick->axisElements, sizeof(struct HIDGamepadAxis) * (joystick->n_axes + 1));
        joystick->axisElements[joystick->n_axes].isHatSwitchSecondAxis = true;
        joystick->n_axes++;
      }
      
    } else if (type == kIOHIDElementTypeInput_Button) {
      joystick->buttonElements = realloc(joystick->buttonElements, sizeof(struct HIDGamepadButton) * (joystick->n_buttons + 1));
      joystick->buttonElements[joystick->n_buttons].cookie = IOHIDElementGetCookie(element);
      joystick->n_buttons++;
    }
  }
  CFRelease(elements);
  
  joystick->axes = malloc(sizeof(float) * joystick->n_axes);
  joystick->buttons = malloc(sizeof(int) * joystick->n_buttons);
  
  IOHIDDeviceRegisterInputValueCallback(device, device_input, joystick);
  CALL(joy_connect_callback, joystick, joystick->device_id);
}

static void release_joystick(int i) {
  joystick_t j = joysticks[i];
  if (!j)
    return;
  
  IOHIDDeviceRegisterInputValueCallback(j->deviceRef, NULL, NULL);
  FREE_SAFE(j->axisElements);
  FREE_SAFE(j->buttonElements);
  if (j->description) {
    free((void*)j->description);
    j->description = NULL;
  }
  FREE_SAFE(j->axes);
  FREE_SAFE(j->buttons);
  FREE_SAFE(j);
  joysticks[i] = NULL;
}

static void device_removed(void* ctx, IOReturn result, void* src, IOHIDDeviceRef device) {
  for (int i = 0; i < joysticks_sz; ++i) {
    if (joysticks[i]->deviceRef == device) {
      CALL(joy_removed_callback, joysticks[i], i);
      release_joystick(i);
      return;
    }
  }
}
#endif

joystick_t sgl_joystick(int n) {
  if ((!hid && !joysticks) || n < 0 || n >= joysticks_sz)
    return NULL;
  return joysticks[n];
}

void sgl_joystick_info(joystick_t j, const char** description, int* device_id, int* vendor_id, int* product_id) {
  if (description)
    *description = j->description;
  if (device_id)
    *device_id = j->device_id;
  if (vendor_id)
    *vendor_id = j->vendor_id;
  if (product_id)
    *product_id = j->product_id;
}

bool sgl_joystick_init(int max) {
  if (hid && joysticks)
    return false;
  
  CFStringRef keys[2];
  int value;
  CFNumberRef values[2];
  CFDictionaryRef dictionaries[3];
  CFArrayRef array;
  
  hid = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
  if (!max)
    max = 4;
  joysticks = malloc(sizeof(joystick_t) * max);
  if (!joysticks) {
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc() failed");
    return false;
  }
  for (int i = 0; i < max; ++i)
    joysticks[i] = NULL;
  joysticks_sz = max;
  
  keys[0] = CFSTR(kIOHIDDeviceUsagePageKey);
  keys[1] = CFSTR(kIOHIDDeviceUsageKey);
  
  value = kHIDPage_GenericDesktop;
  values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
  value = kHIDUsage_GD_Joystick;
  values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
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
  
  array = CFArrayCreate(kCFAllocatorDefault, (const void**)dictionaries, 3, &kCFTypeArrayCallBacks);
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
  return true;
}

void sgl_joystick_release() {
  if (!hid && !joysticks)
    return;
  
  IOHIDManagerUnscheduleFromRunLoop(hid, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
  for (int i = 0; i < joysticks_sz; ++i)
    release_joystick(i);
  FREE_SAFE(joysticks);
  IOHIDManagerClose(hid, 0);
  CFRelease(hid);
  hid = NULL;
}

bool sgl_joystick_remove(int n) {
  if ((!hid && !joysticks) || n < 0 || n >= joysticks_sz)
    return false;
  release_joystick(n);
  return true;
}

void sgl_joystick_poll() {
  if (!hid && !joysticks)
    return;
  CFRunLoopRunInMode(GAMEPAD_RUN_LOOP_MODE, 0, true);
}
#endif

#if defined(SGL_ENABLE_DIALOGS)
#include <AppKit/AppKit.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_12
#define NSAlertStyleInformational NSInformationalAlertStyle
#define NSAlertStyleWarning NSWarningAlertStyle
#define NSAlertStyleCritical NSCriticalAlertStyle
#endif

bool sgl_alert(ALERTLVL lvl, ALERTBTNS btns, const char* fmt, ...) {
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
  
  char buffer[BUFSIZ];
  va_list args;
  va_start(args, fmt);
  vsprintf(buffer, fmt, args);
  va_end(args);
  [alert setMessageText:@(buffer)];
  
  bool result = ([alert runModal] == NSAlertFirstButtonReturn);
  [alert release];
  return result;
}

char* sgl_dialog(DIALOGACTION action, const char* path, const char* fname, bool allow_multiple, int nfilters, ...) {
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
  for (int i = 0; i < nfilters; ++i)
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
#endif

#if !defined(SGL_DISABLE_WINDOW)
static short int keycodes[512];
static bool keycodes_init = false;

struct screen_t {
  int id, w, h;
  
#define X(a, b) \
void(*a##_callback)b;
  XMAP_SCREEN_CB
#undef X
  
  void* window;
};

#define X(a, b) \
void(*a##_cb)b,
void sgl_screen_callbacks(XMAP_SCREEN_CB screen_t screen) {
#undef X
#define X(a, b) \
  screen->a##_callback = a##_cb;
  XMAP_SCREEN_CB
#undef X
}

#define X(a, b)\
void sgl_##a##_callback(screen_t screen, void(*a##_cb)b) { \
  screen->a##_callback = a##_cb; \
}
XMAP_SCREEN_CB
#undef X

int sgl_screen_id(screen_t s) {
  return s->id;
}

void sgl_screen_size(screen_t s, int* w, int* h) {
  if (w)
    *w = s->w;
  if (h)
    *h = s->h;
}

#define CBCALL(x, ...) \
  if (active_window && active_window->x) \
    active_window->x(userdata, __VA_ARGS__);

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

static int gl3_available = 1;
static bool dll_loaded = false;

bool init_gl(int w, int h, GLuint* _vao, GLuint* _shader, GLuint* _texture) {
  if (!dll_loaded) {
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
    dll_loaded = true;
  }

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
#if !defined(SGL_OSX)
  }
#endif

  glGenTextures(1, &texture);
  *_texture = texture;
  
  return true;
}

void draw_gl(GLuint vao, GLuint texture, surface_t buffer) {
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

void free_gl(GLuint vao, GLuint shader, GLuint texture) {
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
                                                                   bitmapFormat:NSAlphaNonpremultipliedBitmapFormat
                                                                    bytesPerRow:0
                                                                   bitsPerPixel:0] autorelease];
  if (!nsbir)
    return nil;
  
  char* rgba = (char*)malloc(s->w * s->h * 4);
  int offset = 0, c;
  for(int i = 0; i < s->h; ++i) {
    for (int j = 0; j < s->w; j++) {
      c = XYGET(s, j, i);
      rgba[4 * offset]     = R(c);
      rgba[4 * offset + 1] = G(c);
      rgba[4 * offset + 2] = B(c);
      rgba[4 * offset + 3] = A(c);
      offset++;
    }
  }
  memcpy([nsbir bitmapData], rgba, s->w * s->h * sizeof(char) * 4);
  free(rgba);
  
  [nsi addRepresentation:nsbir];
  return nsi;
}

static screen_t active_window = NULL;

@protocol AppViewDelegate;

#if defined(SGL_ENABLE_OPENGL)
@interface AppView : NSOpenGLView
@property GLuint vao;
@property GLuint shader;
@property GLuint texture;
#elif defined(SGL_ENABLE_METAL)
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
#if defined(SGL_ENABLE_OPENGL)
@synthesize vao = _vao;
@synthesize shader = _shader;
@synthesize texture = _texture;
#elif defined(SGL_ENABLE_METAL)
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
  NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
  self = [super initWithFrame:frameRect
                  pixelFormat:pixelFormat];
  [[self openGLContext] makeCurrentContext];
  init_gl(frameRect.size.width, frameRect.size.height, &_vao, &_shader, &_texture);
  [pixelFormat release];
#elif defined(SGL_ENABLE_METAL)
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

- (void)setRegularCursor:(CURSORTYPE)type {
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

#if defined(SGL_ENABLE_METAL)
- (void)updateMTKViewport:(CGSize)size {
  _mtk_viewport.x = size.width * _scale_f;
  _mtk_viewport.y = (size.height * _scale_f) + (4 * _scale_f);
}
#endif

- (void)drawRect:(NSRect)dirtyRect {
  if (!_buffer)
    return;
  
#if defined(SGL_ENABLE_OPENGL)
  draw_gl(_vao, _texture, _buffer);
  [[self openGLContext] flushBuffer];
#elif defined(SGL_ENABLE_METAL)
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
#if defined(SGL_ENABLE_OPENGL)
  free_gl(_vao, _shader, _texture);
#elif defined(SGL_ENABLE_METAL)
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
#if defined(SGL_ENABLE_METAL)
- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size;
#endif
@end

@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate, AppViewDelegate>
@property (unsafe_unretained) NSWindow* window;
@property (weak) AppView* view;
@property (nonatomic) screen_t parent;
@property BOOL closed;
@end

@implementation AppDelegate
@synthesize window = _window;
@synthesize view = _view;
@synthesize parent = _parent;
@synthesize closed = _closed;

-(id)initWithSize:(NSSize)windowSize styleMask:(short)flags title:(const char*)windowTitle {
  NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable;
  NSRect frameRect = NSMakeRect(0, 0, windowSize.width, windowSize.height);
  
  _window = [[NSWindow alloc] initWithContentRect:frameRect
                                        styleMask:styleMask
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
  if (!_window) {
    sgl_release();
    error_handle(HIGH_PRIORITY, OSX_WINDOW_CREATION_FAILED, "[_window initWithContentRect] failed");
    return nil;
  }
  
  [_window setAcceptsMouseMovedEvents:YES];
  [_window setRestorable:NO];
  [_window setTitle:(windowTitle ? @(windowTitle) : [[NSProcessInfo processInfo] processName])];
  [_window setReleasedWhenClosed:NO];
  
  if (!active_window)
    [_window center];
  else {
    AppDelegate* tmp = (AppDelegate*)active_window->window;
    NSPoint tmp_p = [[tmp window] frame].origin;
    [_window setFrameOrigin:NSMakePoint(tmp_p.x + 20, tmp_p.y - 20 - [tmp titlebarHeight])];
  }
  
  _view = [[AppView alloc] initWithFrame:frameRect];
  if (!_view) {
    sgl_release();
    error_handle(HIGH_PRIORITY, OSX_WINDOW_CREATION_FAILED, "[_view initWithFrame] failed");
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

- (void)setParent:(screen_t)screen {
  _parent = screen;
}

- (CGFloat)titlebarHeight {
  return _window.frame.size.height - [_window contentRectForFrameRect: _window.frame].size.height;
}

- (void)windowWillClose:(NSNotification*)notification {
  _closed = YES;
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
  active_window = _parent;
}

- (void)windowDidResignKey:(NSNotification*)notification {
  active_window = NULL;
}

- (void)windowDidResize:(NSNotification*)notification {
  static CGSize size;
  size = [_view frame].size;
#if defined(SGL_ENABLE_METAL)
  [_view updateMTKViewport:size];
#endif
  _parent->w = (int)roundf(size.width);
  _parent->h = (int)roundf(size.height);
  CBCALL(resize_callback, _parent->w, _parent->h);
}

#if defined(SGL_ENABLE_METAL)
- (void)mtkView:(MTKView*)mtkView drawableSizeWillChange:(CGSize)size; {}
#endif
@end

bool sgl_screen(struct screen_t** s, const char* t, int w, int h, short flags) {
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
    keycodes_init = true;
  }
  
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
  
  AppDelegate* app = [[AppDelegate alloc] initWithSize:NSMakeSize(w, h) styleMask:flags title:t];
  if (!app) {
    sgl_release();
    error_handle(HIGH_PRIORITY, OSX_WINDOW_CREATION_FAILED, "[AppDelegate alloc] failed");
    return false;
  }
  
  struct screen_t* screen = *s = malloc(sizeof(struct screen_t));
  if (!screen) {
    sgl_release();
    error_handle(HIGH_PRIORITY, OUT_OF_MEMEORY, "malloc failed");
    return false;
  }
  memset(screen, 0, sizeof(*screen));
  screen->id = (int)[[app window] windowNumber];
  screen->w  = w;
  screen->h  = h;
  screen->window = (void*)app;
  active_window = screen;
  [app setParent:screen];
  
  [NSApp activateIgnoringOtherApps:YES];
  [pool drain];
  return true;
}
  
#define SET_DEFAULT_APP_ICON [NSApp setApplicationIconImage:[NSImage imageNamed:@"NSApplicationIcon"]]

void sgl_screen_icon_buf(screen_t s, surface_t b) {
  if (!b || !b->buf) {
    SET_DEFAULT_APP_ICON;
    return;
  }
  
  NSImage* img = create_cocoa_image(b);
  if (!img)  {
    error_handle(LOW_PRIORITY, WINDOW_ICON_FAILED, "sgl_screen_icon_b() failed: Couldn't set window icon");
    SET_DEFAULT_APP_ICON;
    return;
  }
  [NSApp setApplicationIconImage:img];
}
  
void sgl_screen_icon(screen_t s, const char* p) {
  if (!p) {
    SET_DEFAULT_APP_ICON;
    return;
  }
  
  NSImage* img = [[NSImage alloc] initWithContentsOfFile:@(p)];
  if (!img) {
    error_handle(LOW_PRIORITY, WINDOW_ICON_FAILED, "sgl_screen_icon() failed: Couldn't set window icon from \"%s\"\n", p);
    SET_DEFAULT_APP_ICON;
    return;
  }
  [NSApp setApplicationIconImage:img];
  [img release];
}

void sgl_screen_title(screen_t s, const char* t) {
  [[(AppDelegate*)s->window window] setTitle:@(t)];
}
  
void sgl_screen_destroy(struct screen_t** s) {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  struct screen_t* screen = *s;
  AppDelegate* app = (AppDelegate*)screen->window;
  if (app) {
    [[app view] dealloc];
    [[app window] close];
  }
  free(app);
  free(screen);
  [pool drain];
}

bool sgl_closed(screen_t s) {
  return (bool)[(AppDelegate*)s->window closed];
}
  
void sgl_cursor_lock(bool locked) {
  // TODO: Figure this out
}

void sgl_cursor_visible(bool shown) {
  if (shown)
    [NSCursor unhide];
  else
    [NSCursor hide];
}

void sgl_cursor_icon(screen_t s, CURSORTYPE t) {
  if (!s) {
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon() failed: Invalid screen");
    return;
  }
  
  AppDelegate* app = (AppDelegate*)s->window;
  if (!app) {
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon() failed: Invalid screen");
    return;
  }
  [[app view] setRegularCursor:t];
}

void sgl_cursor_icon_custom(screen_t s, const char* p) {
  if (!s || !p) {
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon_custom() failed: Invalid parameters");
    return;
  }
  
  NSImage* img = [[NSImage alloc] initWithContentsOfFile:@(p)];
  if (!img) {
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon_custom() failed: Couldn't set cursor from \"%s\"\n", p);
    return;
  }
  
  AppDelegate* app = (AppDelegate*)s->window;
  if (!app) {
    [img release];
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon_custom() failed: Invalid screen");
    return;
  }
  [[app view] setCustomCursor:img];
  [img release];
}

void sgl_cursor_icon_custom_buf(screen_t s, surface_t b) {
  if (!s || !b) {
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon_custom_buf() failed: Invalid parameters");
    return;
  }
  
  NSImage* img = create_cocoa_image(b);
  if (!img) {
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon_custom_buf() failed: Couldn't set cursor from buffer");
    return;
  }
  
  AppDelegate* app = (AppDelegate*)s->window;
  if (!app) {
    [img release];
    error_handle(LOW_PRIORITY, CURSOR_MOD_FAILED, "sgl_cursor_icon_custom_buf() failed: Invalid screen");
    return;
  }
  [[app view] setCustomCursor:img];
  [img release];
}

void sgl_cursor_pos(point_t* p) {
  const NSPoint _p = [NSEvent mouseLocation];
  p->x = _p.x;
  p->y = [[(AppDelegate*)active_window->window window] screen].frame.size.height - _p.y;
}

void sgl_cursor_set_pos(point_t* p) {
  CGPoint _p;
  _p.x = p->x;
  _p.y = p->y;
  CGWarpMouseCursorPosition(_p);
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
        CBCALL(keyboard_callback, translate_key([e keyCode]), translate_mod([e modifierFlags]), ([e type] == NSEventTypeKeyDown));
        break;
      case NSEventTypeLeftMouseUp:
      case NSEventTypeRightMouseUp:
      case NSEventTypeOtherMouseUp:
        CBCALL(mouse_button_callback, (MOUSEBTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), false);
        break;
      case NSEventTypeLeftMouseDown:
      case NSEventTypeRightMouseDown:
      case NSEventTypeOtherMouseDown:
        CBCALL(mouse_button_callback, (MOUSEBTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
        break;
      case NSEventTypeScrollWheel:
        CBCALL(scroll_callback, translate_mod([e modifierFlags]), [e deltaX], [e deltaY]);
        break;
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged:
        CBCALL(mouse_button_callback, (MOUSEBTN)([e buttonNumber] + 1), translate_mod([e modifierFlags]), true);
      case NSEventTypeMouseMoved: {
        static AppDelegate* app = NULL;
        if (!active_window)
          break;
        app = (AppDelegate*)active_window->window;
        if ([[app view] mouse_in_window])
          CBCALL(mouse_move_callback, [e locationInWindow].x, (int)([[app view] frame].size.height - roundf([e locationInWindow].y)), 0, 0);
        break;
      }
    }
    [NSApp sendEvent:e];
  }
  [pool release];
}

void sgl_flush(screen_t s, surface_t b) {
  if (!s || !b)
    return;
  AppDelegate* tmp = (AppDelegate*)s->window;
  if (!tmp)
    return;
  if (b)
    [tmp view].buffer = b;
  [[tmp view] setNeedsDisplay:YES];
}

void sgl_release() {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  // Felt cute might delete later
  [pool drain];
}
#elif defined(SGL_WINDOWS)
// TODO: Reimplement
#elif defined(SGL_LINUX)
// TODO: Reimplement
#else
#error Unsupported operating system
#endif
#endif
