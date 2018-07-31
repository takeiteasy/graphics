//
//  vector.h
//  roguelike
//
//  Created by Rory B. Bellows on 31/07/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef struct {
  int    length;
  void** data;
} vector_t;

#define vector_len(v) v.length
#define vector_pop(v) vector_pull(&v, v.length - 1)
#define vector_shift(v) vector_pull(&v, 0)
#define vector_for_each(v, i) for (int i = 0; i < v.length; ++i)

void  vector_init(vector_t*);
void  vector_free(vector_t*);
int   vector_push(vector_t*, void*);
int   vector_insert(vector_t*, int, void*);
int   vector_insert_va(vector_t*, int, int, ...);
void* vector_get(vector_t*, int);
int   vector_set(vector_t*, int, void*);
void* vector_pull(vector_t*, int);
void* vector_pull_free(vector_t* v, int id);
void* vector_pull_free_with(vector_t* v, int id, void(*)(void*));
int   vector_del(vector_t*, int);
int   vector_del_free(vector_t*, int);
int   vector_del_free_with(vector_t*, int, void(*)(void*));
int   vector_clear(vector_t*);
int   vector_clear_free(vector_t*);
int   vector_clear_free_with(vector_t*, void(*)(void*));
int   vector_free_all(vector_t*);
int   vector_free_all_with(vector_t*, void(*)(void*));

#endif // __VECTOR_H__
