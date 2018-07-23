//
//  heap.c
//  roguelike
//
//  Created by Rory B. Bellows on 24/06/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "heap.h"

heap_t* heap_new(int len, int(*compare)(const void *, const void *)) {
  heap_t* h = (heap_t*)malloc(sizeof(heap_t));
  if (!h)
    return NULL;
  h->data = malloc(len * sizeof(void*));
  h->size = len;
  h->compare = compare;
  h->len = 0;
  return h;
}

void heap_free(heap_t *h) {
  free(h->data);
  free(h);
}

int insert(heap_t *h, void *n) {
  if (h->len >= h->size) {
    void** tmp = (void **)realloc(h->data, sizeof(void *) * h->size * 2);
    if (!tmp)
      return 0;
    h->data = tmp;
    h->size *= 2;
  }
  h->data[h->len++] = n;
  return 1;
}

void* peek(heap_t *h) {
  return (h->len < 1 ? NULL : h->data[0]);
}

static void swap(void** a, void** b) {
  void* c = *a;
  *a = *b;
  *b = c;
}

static void sift(heap_t* h, int i, int(*compare)(const void*, const void*)) {
  if (!i)
    return;
  int pi = (i - 1) / 2;
  if (compare(h->data[i], h->data[pi]) > 0) {
    swap(&h->data[i], &h->data[pi]);
    sift(h, pi, compare);
  }
}

static void sift_down(heap_t* h, int i, int(*compare)(const void*, const void*)) {
  int l = i * 2 + 1;
  int r = i * 2 + 2;
  int mi = i;
  
  if (l <= h->len && compare(h->data[l], h->data[mi]) > 0)
    mi = l;
  if (r <= h->len && compare(h->data[r], h->data[mi]) > 0)
    mi = r;
  
  if (mi != i) {
    swap(&h->data[i], &h->data[mi]);
    sift_down(h, mi, compare);
  }
}

int heap_add(heap_t* h, void* data) {
  int result = insert(h, data);
  if (!result)
    sift(h, h->len - 1, h->compare);
  return result;
}

void* heap_remove(heap_t *h) {
  if (!h->len)
    return NULL;
  void* root = peek(h);
  h->data[0] = h->data[--h->len];
  sift_down(h, 0, h->compare);
  return root;
}
