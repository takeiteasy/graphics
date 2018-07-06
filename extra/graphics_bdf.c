//
//  graphics_bdf.c
//  graphics_osx_test
//
//  Created by Rory B. Bellows on 03/07/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "graphics_bdf.h"

static char bdf_last_error[1024];

#define SET_LAST_ERROR(MSG, ...) \
memset(bdf_last_error, 0, 1024); \
sprintf(bdf_last_error, "[ERROR] from %s in %s() at %d -- " MSG, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

#define BDF_READ_INT(x) \
p = strtok(NULL, " \t\n\r"); \
(x) = atoi(p);

//#define BDF_READ_STR(x) \
//p = strtok(NULL, "\t\n\r"); \
//strcpy((x), p);

void bdf_destroy(bdf_t* f) {
  if (f) {
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
  }
}

static inline int htoi(const char* p) {
  return (*p <= '9' ? *p - '0' : (*p <= 'F' ? *p - 'A' + 10 : *p - 'a' + 10));
}

int bdf(bdf_t* out, const char* path) {
  FILE* fp = fopen(path, "r");
  if (!fp) {
    SET_LAST_ERROR("fopen() failed: %s\n", path);
    return 0;
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
    SET_LAST_ERROR("bdf() failed: No character size given for %s\n", path);
    return 0;
  }
  
  if (out->n_chars <= 0) {
    SET_LAST_ERROR("bdf() failed: Unknown number of characters for %s\n", path);
    return 0;
  }
  
  out->encoding_table = malloc(out->n_chars * sizeof(unsigned int));
  if (!out->encoding_table) {
    SET_LAST_ERROR("bdf() failed: Out of memory\n");
    return 0;
  }
  out->chars = malloc(out->n_chars * sizeof(bdf_char_t));
  if (!out->chars) {
    free(out->encoding_table);
    SET_LAST_ERROR("bdf() failed: Out of memory\n");
    return 0;
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
        bdf_destroy(out);
        SET_LAST_ERROR("bdf() failed: More bitmaps than characters for %s\n", path);
        return 0;
      }
      if (width == -1) {
        bdf_destroy(out);
        SET_LAST_ERROR("bdf() failed: Unknown character with for %s\n", path);
        return 0;
      }
      
      if (out->chars[n].bb.x < 0) {
        width -= out->chars[n].bb.x;
        out->chars[n].bb.x = 0;
      }
      if (out->chars[n].bb.x + out->chars[n].bb.w > width)
        width = out->chars[n].bb.x + out->chars[n].bb.w;
      
      out->chars[n].bitmap = malloc(((out->fontbb.w + 7) / 8) * out->fontbb.h * sizeof(unsigned char));
      if (!out->chars[n].bitmap) {
        bdf_destroy(out);
        SET_LAST_ERROR("bdf() failed: Out of memory\n");
        return 0;
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
  return 1;
}

int bdf_character(surface_t* s, bdf_t* f, const char* ch, int x, int y, int fg, int bg) {
  const char* c = (const char*)ch;
  int u = *c, l = 1, i, j, n = 0;
  if ((u & 0xC0) == 0xC0) {
    int a = (u & 0x20) ? ((u & 0x10) ? ((u & 0x08) ? ((u & 0x04) ? 6 : 5) : 4) : 3) : 2;
    if (a < 6 || !(u & 0x02)) {
      u = ((u << (a + 1)) & 0xFF) >> (a + 1);
      for (int b = 1; b < a; ++b)
        u = (u << 6) | (c[l++] & 0x3F);
    }
  }
  
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
        pset(s, x + j, y + yy, (cc & i ? fg : bg));
    }
  }
  
  return l;
}

void bdf_writeln(surface_t* s, bdf_t* f, int x, int y, int fg, int bg, const char* str) {
  const char* c = (const char*)str;
  int u = x, v = y;
  while (c != NULL && *c != '\0') {
    if (*c == '\n') {
      v += f->fontbb.h + 2;
      u  = x;
      c += 1;
    } else {
      c += bdf_character(s, f, c, u, v, fg, bg);
      u += 8;
    }
  }
}

const char* get_last_bdf_error() {
  return bdf_last_error;
}
