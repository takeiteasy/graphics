#include "../graphics.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

surface_t* load_image(const char* path) {
  int w, h, c;
  unsigned char* data = stbi_load(path, &w, &h, &c, STBI_rgb);
  if (!data)
    abort();
  
  surface_t* s = surface(w, h);
  if (!s)
    abort();
  
  for (int x = 0; x < w; ++x) {
    for (int y = 0; y < h; ++y) {
      unsigned char* p = data + (x + h * y) * c;
      pset(s, x, y, RGB(p[0], p[1], p[2]));
    }
  }
  
  return s;
}

int main(int argc, const char* argv[]) {
  surface_t* img = load_image("lena.png");
  if (!img) {
    fprintf(stderr, "%s", get_last_error());
    return 1;
  }
  
  surface_t* win = screen("stb_image", img->w, img->h);
  if (!win) {
    fprintf(stderr, "%s", get_last_error());
    return 1;
  }
  
  blit(win, NULL, img, NULL, -1);
  
  user_event_t ue;
  while (!should_close()) {
    while (poll_events(&ue));
    render();
  }
  
  
  destroy(&img);
  release();
  return 0;
}

