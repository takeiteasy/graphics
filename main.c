#include "app.h"

#define WIDTH  640
#define HEIGHT 480

#define RNDRGB (rand() % 256)

void fill_rnd(surface_t* s) {
  int x, y;
  for (x = 0; x < s->w; ++x)
    for (y = 0; y < s->h; ++y)
      pset(s, x, y, RNDRGB, RNDRGB, RNDRGB);
}

const char* font_b64_str = "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";
const int font_b64_strlen = 2928;

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

unsigned char* base64_decode(const char* ascii, int len, int *flen) {
  int cb = 0, pad = 0, n, A, B, C, D;;
  
  if (len < 2) {
    puts("ERROR! base64_decode() failed. Base64 string too short.");
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
    puts("ERROR! malloc() failed.");
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

int main(int argc, const char* argv[]) {
  surface_t* win = screen("test", WIDTH, HEIGHT);
  if (!win)
    return 1;
  
  surface_t* test = surface(200, 200);
  rect_filled(test, 0, 0, 100, 100, 255, 0, 0);
  rect_filled(test, 100, 0, 100, 100, 0, 255, 0);
  rect_filled(test, 0, 100, 100, 100, 0, 0, 255);
  rect_filled(test, 100, 100, 100, 100, 255, 255, 255);
  
  surface_t* rnd = surface(50, 50);
  
//  unsigned char* a = load_file_to_mem("/Users/roryb/Desktop/Uncompressed-24.bmp");
//  surface_t* b = load_bmp_from_mem(a);
//  free(a);
  surface_t* c = load_bmp_from_file("/Users/roryb/Desktop/charset.bmp");
//  surface_t* c = surface(100, 100);
//  for (int i = 0; i < 100 * 100; ++i) {
//    c->buf[(i - (i % 100)) + (100 - (i % 100) - 1)] = RGB2INT(255 - (i % 100), 255 - (i % 100), 255 - (i % 100));
//    printf("%d\n", (i - (i % 100)) + (100 - (i % 100) - 1));
//  }
  
  int size;
  unsigned char* f = base64_decode(font_b64_str, font_b64_strlen, &size);
  surface_t* d = load_bmp_from_mem(f);
  free(f);
  
  rect_t  tmpr  = { 0, 100, 100, 100 };
  point_t tmpp  = { 0, 22 };
  point_t tmpp1  = { 101, 22 };
  point_t tmpp2 = { 5, 227 };
  point_t tmpp4 = { c->w + 10, 227 };
  point_t tmpp3 = { 475, 175 };
  
  int noise, carry, seed = 0xBEEF, i;
  for (;;) {
    for (i = 0; i < WIDTH * HEIGHT; ++i) {
      noise = seed;
      noise >>= 3;
      noise ^= seed;
      carry = noise & 1;
      noise >>= 1;
      seed >>= 1;
      seed |= (carry << 30);
      noise &= 0xFF;
      win->buf[i] = RGB2INT(noise, noise, noise);
    }
    // fill_surface(win, 0, 0, 0);
    
    blit(win, &tmpp1, test, NULL);
    blit(win, &tmpp, test, &tmpr);
    
    blit(win, &tmpp4, d, NULL);
    blit(win, &tmpp2, c, NULL);
    
    xline(win, 135, 110, 160, 255, 255, 255);
    yline(win, 135, 110, 160, 255, 255, 255);
    line(win, 0, 0, 300, 300, 255, 255, 0);
    circle(win, 300, 300, 30, 255, 255, 0);
    circle_filled(win, 350, 350, 30, 255, 255, 0);
    rect(win, 425, 125, 150, 150, 0, 255, 255);
    rect_filled(win, 450, 150, 100, 100, 0, 255, 255);
    
    fill_rnd(rnd);
    blit(win, &tmpp3, rnd, NULL);
    
    if (!redraw())
      break;
  }
  
//  free_surface(&b);
  destroy(&c);
  destroy(&d);
  destroy(&test);
  destroy(&rnd);
  release();
  return 0;
}

