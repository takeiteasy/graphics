//
//  heap.h
//  roguelike
//
//  Created by Rory B. Bellows on 24/06/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#ifndef heap_h
#define heap_h

#include <stdlib.h>

typedef struct {
  int len, size;
  int (*compare)(const void *, const void *);
  void** data;
} heap_t;

heap_t* heap_new(int len, int(*compare)(const void *, const void *));
void heap_free(heap_t *h);
int heap_add(heap_t* h, void* data);
void* heap_remove(heap_t *h);
void* peek(heap_t *h);

#endif /* heap_h */
