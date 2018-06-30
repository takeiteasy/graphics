#include "../graphics.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void load_image(surface_t* s, const char* path) {
  int w, h, c;
  unsigned char* data = stbi_load(path, &w, &h, &c, STBI_rgb);
  if (!data) {
    fprintf(stderr, "ERROR: %s\n", stbi_failure_reason());
    abort();
  }
  
  surface(s, w, h);
  if (!s) {
    fprintf(stderr, "ERROR: %s\n", stbi_failure_reason());
    abort();
  }
  
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      unsigned char* p = data + (x + h * y) * c;
      pset(s, x, y, RGB(p[0], p[1], p[2]));
    }
  }
}

int main(int argc, const char* argv[]) {
  surface_t img;
  load_image(&img, "/Users/roryb/Documents/git/graphics.h/tests/lena.png");
  
  if (!screen("stb_image", img.w, img.h)) {
    fprintf(stderr, "%s", get_last_error());
    return 1;
  }
  
  user_event_t ue;
  while (!should_close()) {
    while (poll_events(&ue));
    render(&img);
  }
  
  destroy(&img);
  release();
  return 0;
}

