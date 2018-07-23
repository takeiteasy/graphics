//
//  dungeon.h
//  roguelike
//
//  Created by Rory B. Bellows on 24/06/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#ifndef dungeon_h
#define dungeon_h

#include "common.h"
#include "../../../graphics.h"
#include "heap.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define TILE_MAP_MAX_H 8
#define TILE_MAP_SIZE 10

// CLEAN UP
static struct tile_map_t {
  int solid, size;
  char map[TILE_MAP_MAX_H + 1];
} tile_map[TILE_MAP_SIZE] = {
  { .solid = 0, .size = 1, { '.', '\0' } },
  { .solid = 1, .size = 1, { '#', '\0' } },
  { .solid = 1, .size = 2, { '#', '#', '\0' } },
  { .solid = 1, .size = 3, { '#', '#', '#', '\0' } },
  { .solid = 1, .size = 4, { '#', '#', '#', '#', '\0' } },
  { .solid = 1, .size = 5, { '#', '#', '#', '#', '#', '\0' } },
  { .solid = 1, .size = 6, { '#', '#', '#', '#', '#', '#', '\0' } },
  { .solid = 1, .size = 7, { '#', '#', '#', '#', '#', '#', '#', '\0' } },
  { .solid = 1, .size = 8, { '#', '#', '#', '#', '#', '#', '#', '#', '\0' } },
  { .solid = 1, .size = 8, { ' ', '#', ' ', '#', ' ', '#', ' ', '#', '\0' } }
};

#define EDGE_COST 1
#define DIAG_COST sqrt(2)
#define LEAK_COST INFINITY
#if defined(ASTAR_NO_DIAG)
#define MAX_NUM_NEIGHBOURS 4
#else
#define MAX_NUM_NEIGHBOURS 8
#endif

typedef enum {
  CLOSED,
  UNVISITED,
  VISITED
} node_state;

typedef struct __node {
  int v;
  node_state state;
  int x, y;
  float cost_self, cost_goal;
  struct __node* next;
} node_t, *nodeptr_t;

typedef struct {
  point_t *tiles, *edges;
  int id, n_tiles, n_edges;
} room_t;

typedef struct {
  int w, h;
  nodeptr_t map;
  room_t* rooms;
  int n_rooms;
  point_t camera;
} dungeon_t;

void dungeon_free(dungeon_t* d);
nodeptr_t dungeon_in(dungeon_t* d, int x, int y);
int dungeon_solid(dungeon_t* d, int x, int y);
nodeptr_t astar(dungeon_t* d, int x0, int y0, int x1, int y1);
void new_cave(dungeon_t* d, int* progress, int minw, int minh, int maxw, int maxh, int fill_prob, int iterations, int survival, int starve);
#define new_cave_default(m, p) (new_cave((m), (p), 50, 50, 200, 200, RND_RANGE(38, 46), 5, 4, 3))
void draw_dungeon(surface_t* dst, dungeon_t* d);

#endif /* dungeon_h */
