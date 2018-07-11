//
//  graphics_image.c
//  graphics_osx_test
//
//  Created by Rory B. Bellows on 04/07/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "graphics_image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "3rdparty/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#if defined(_MSC_VER)
#define STBI_MSC_SECURE_CRT
#endif
#include "3rdparty/stb_image_write.h"

static char stb_last_error[1024];

#define SET_LAST_ERROR(MSG, ...) \
memset(stb_last_error, 0, 1024); \
sprintf(stb_last_error, "[ERROR] from %s in %s() at %d -- " MSG, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__);

int image(surface_t* out, const char* path) {
  int w, h, c, x, y;
  unsigned char* data = stbi_load(path, &w, &h, &c, 0);
  if (!data) {
    SET_LAST_ERROR("stbi_load() failed: %s\n", stbi_failure_reason());
    return 0;
  }
  
  if (!surface(out, w, h)) {
    stbi_image_free(data);
    SET_LAST_ERROR("image() failed: %s\n", last_error());
    return 0;
  }
  
  unsigned char* p = NULL;
  for (x = 0; x < w; ++x) {
    for (y = 0; y < h; ++y) {
      p = data + (x + w * y) * c;
      out->buf[y * w + x] = RGBA(p[0], p[1], p[2], (c == 4 ? p[3] : 255));
    }
  }
  
  stbi_image_free(data);
  return 1;
}

static inline const char* extension(const char* path) {
  const char* dot = strrchr(path, '.');
  return (!dot || dot == path ? NULL : dot + 1);
}

int save_image(surface_t* in, const char* path) {
  if (!in || !path) {
    SET_LAST_ERROR("save_image() failed: Invalid parameters");
    return 1;
  }
  
  unsigned char* data = malloc(in->w * in->h * 4 * sizeof(unsigned char));
  if (!data) {
    SET_LAST_ERROR("save_image() failed: Out of memory");
    return 1;
  }
  
  unsigned char* p = NULL;
  int i, j, c;
  for (i = 0; i < in->w; ++i) {
    for (j = 0; j < in->h; ++j) {
      p = data + (i + in->w * j) * 4;
      c = in->buf[j * in->w + i];
      p[0] = R(c);
      p[1] = G(c);
      p[2] = B(c);
      p[3] = A(c);
    }
  }
  
  // Avert your eyes if you don't want to go blind
  int res = 0;
  const char* ext = extension(path);
TRY_AGAIN_BRO:
  if (!ext || !strcmp(ext, "png"))
    res = stbi_write_png(path, in->w, in->h, 4, data, 0);
  else if (!strcmp(ext, "tga"))
    res = stbi_write_tga(path, in->w, in->h, 4, data);
  else if (!strcmp(ext, "bmp"))
    res = stbi_write_bmp(path, in->w, in->h, 4, data);
  else if (!strcmp(ext, "jpg") || !strcmp(ext, "jpeg"))
    stbi_write_jpg(path, in->w, in->h, 4, data, 85);
  else {
    ext = "png";
    goto TRY_AGAIN_BRO;
  }
  free(data);
  
  if (!res) {
    SET_LAST_ERROR("save_image() failed: %s", get_last_stb_error());
    return 0;
  }
  return 1;
}

const char* get_last_stb_error() {
  return stb_last_error;
}
