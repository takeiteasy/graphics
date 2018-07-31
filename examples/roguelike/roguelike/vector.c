//
//  vector.c
//  roguelike
//
//  Created by Rory B. Bellows on 31/07/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "vector.h"

#define __VECTOR_ALLOC(v) \
v->length = 0; \
v->data   = (void**)malloc(sizeof(void*));

#define __VECTOR_CHECK(r) if (id < 0 || id > v->length || !v) return r

#define __VECTOR_DEL(v, id) \
v->data[id] = NULL; \
for (int __i__ = id + 1; __i__ < v->length; ++__i__) \
  v->data[__i__ - 1] = v->data[__i__]; \
v->data[v->length - 1] = NULL; \
__vector_resize(v, --v->length);

void vector_init(vector_t* v) {
  __VECTOR_ALLOC(v);
}

void vector_free(vector_t* v) {
  free(v->data);
}

static int __vector_resize(vector_t* v, int size) {
  if (!v)
    return -1;
  
  void** new = realloc(v->data, size * sizeof(void*));
  if (!new)
    return -1;
  v->data = new;
  
  return 1;
}

int vector_push(vector_t* v, void* data) {
  if (!__vector_resize(v, ++v->length))
    return -1;
  v->data[v->length - 1] = data;
  return 1;
}

int vector_insert(vector_t* v, int id, void* data) {
  if (!__vector_resize(v, ++v->length))
    return -1;
  
  for (int i = v->length - 1; i >= id; --i)
    v->data[i] = v->data[i - 1];
  v->data[id] = data;
  return 1;
}

int vector_insert_va(vector_t* v, int id, int n, ...) {
  v->length += n;
  if (!__vector_resize(v, v->length))
    return -1;
  
  for (int i = v->length - 1; i >= v->length - n; --i)
    v->data[i] = v->data[i - n - 1];
  
  va_list va;
  va_start(va, n);
  for (int i = id; i < id + n; ++i)
    v->data[i] = va_arg(va, void*);
  va_end(va);
  
  return 1;
}

void* vector_get(vector_t* v, int id) {
  __VECTOR_CHECK(NULL);
  return v->data[id];
}

int vector_set(vector_t* v, int id, void* data) {
  __VECTOR_CHECK(-1);
  v->data[id] = data;
  return 1;
}

void* vector_pull(vector_t* v, int id) {
  __VECTOR_CHECK(NULL);
  void* new = malloc(sizeof(void*));
  memcpy(new, v->data[id], sizeof(v->data[id]));
  __VECTOR_DEL(v, id);
  return new;
}

void* vector_pull_free(vector_t* v, int id) {
  __VECTOR_CHECK(NULL);
  void* new = malloc(sizeof(void*));
  memcpy(new, v->data[id], sizeof(v->data[id]));
  free(v->data[id]);
  __VECTOR_DEL(v, id);
  return new;
}

void* vector_pull_free_with(vector_t* v, int id, void(*free_fn)(void*)) {
  __VECTOR_CHECK(NULL);
  void* new = malloc(sizeof(void*));
  memcpy(new, v->data[id], sizeof(v->data[id]));
  free_fn(v->data[id]);
  __VECTOR_DEL(v, id);
  return new;
}

int vector_del(vector_t* v, int id) {
  __VECTOR_CHECK(-1);
  __VECTOR_DEL(v, id);
  return 1;
}

int vector_del_free(vector_t* v, int id) {
  __VECTOR_CHECK(-1);
  free(v->data[id]);
  __VECTOR_DEL(v, id);
  return 1;
}

int vector_del_free_with(vector_t* v, int id, void(*free_fn)(void*)) {
  __VECTOR_CHECK(-1);
  free_fn(v->data[id]);
  __VECTOR_DEL(v, id);
  return 1;
}

int vector_clear(vector_t* v) {
  if (!v)
    return 0;
  
  free(v->data);
  v->length = 0;
  v->data   = (void**)malloc(sizeof(void*));
  
  return 1;
}

int vector_clear_free(vector_t* v) {
  if (!v)
    return 0;
  
  for (int i = 0; i < v->length; ++i)
    free(v->data[i]);
  
  free(v->data);
  __VECTOR_ALLOC(v);
  
  return 1;
}

int vector_clear_free_with(vector_t* v, void(*free_fn)(void*)) {
  if (!v)
    return 0;
  
  for (int i = 0; i < v->length; ++i)
    free_fn(v->data[i]);
  
  free(v->data);
  __VECTOR_ALLOC(v);
  
  return 1;
}

int vector_free_all(vector_t* v){
  if (!v)
    return 0;
  
  for (int i = 0; i < v->length; ++i)
    free(v->data[i]);
  
  free(v->data);
  
  return 1;
}

int vector_free_all_with(vector_t* v, void(*free_fn)(void*)) {
  if (!v)
    return 0;
  
  for (int i = 0; i < v->length; ++i)
    free_fn(v->data[i]);
  
  free(v->data);
  
  return 1;
}
