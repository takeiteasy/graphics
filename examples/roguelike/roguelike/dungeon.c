//
//  dungeon.c
//  roguelike
//
//  Created by Rory B. Bellows on 24/06/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include "dungeon.h"

void dungeon_free(dungeon_t* d) {
  if (!d)
    return;
  FREE_SAFE(d->map);
  if (d->rooms) {
    for (int i = 0; i < sb_count(d->rooms); ++i) {
      sb_free(d->rooms[i].tiles);
      sb_free(d->rooms[i].edges);
    }
    sb_free(d->rooms);
  }
}

typedef struct {
  int *map, w, h;
} grid_t;

grid_t* grid_new(int w, int h) {
  grid_t* ret = malloc(sizeof(grid_t));
  if (!ret) {
    fprintf(stderr, "ERROR! Out of memory!\n");
    abort();
  }
  size_t sz = w * h * sizeof(int);
  ret->map = malloc(sz);
  if (!ret->map) {
    free(ret);
    fprintf(stderr, "ERROR! Out of memory!\n");
    abort();
  }
  memset(ret->map, 0, w * h * sizeof(int));
  ret->w = w;
  ret->h = h;
  return ret;
}

#define GRID_AT(g, x, y) ((g)->map[(y) * (g)->w + (x)])
#define GRID_IN(g, x, y) ((x) >= 0 && (x) < (g)->w && (y) >= 0 && (y) < (g)->h)
#define GRID_FREE(g) \
if ((g)) { \
  FREE_SAFE((g)->map); \
  FREE_SAFE((g)); \
}

static point_t* get_region_tiles(grid_t* map, int start_x, int start_y) {
  point_t *ret = NULL, *queue = NULL;
  grid_t* map_flags = grid_new(map->w, map->h);
  GRID_AT(map_flags, start_x, start_y) = 1;
  point_t start = { start_x, start_y }, p, tmp;
  sb_push(queue, start);
  
  int queue_i = 0, x, y, type = GRID_AT(map, start_x, start_y);
  while (queue_i < sb_count(queue)) {
    p = queue[queue_i];
    sb_push(ret, p);
    
    for (x = p.x - 1; x <= p.x + 1; ++x) {
      for (y = p.y - 1; y <= p.y + 1; ++y) {
        if (GRID_IN(map, x, y) && (y == p.y || x == p.x)) {
          if (GRID_AT(map_flags, x, y) == 0 && GRID_AT(map, x, y) == type) {
            GRID_AT(map_flags, x, y) = 1;
            tmp.x = x;
            tmp.y = y;
            sb_push(queue, tmp);
          }
        }
      }
    }
    queue_i++;
  }
  
  GRID_FREE(map_flags);
  sb_free(queue);
  return ret;
}

void dungeon_fill(dungeon_t* d, grid_t* map) {
  if (!d)
    return;
  
  d->w = map->w;
  d->h = map->h;
  d->map = malloc(d->w * d->h * sizeof(node_t));
  
  int i, j;
  for (i = 0; i < d->w; ++i)
    for (j = 0; j < d->h; ++j)
      d->map[j * d->w + i] = (node_t) {
        .v = GRID_AT(map, i, j),
        .state = UNVISITED,
        .x = i,
        .y = j,
        .cost_self = INFINITY,
        .cost_goal = INFINITY,
        .next = NULL
      };
}

nodeptr_t dungeon_in(dungeon_t* d, int x, int y) {
  return ((x < 0 || y < 0 || x >= d->w || y >= d->h) ? NULL : &d->map[y * d->w + x]);
}

int dungeon_solid(dungeon_t* d, int x, int y) {
  nodeptr_t tmp = dungeon_in(d, x, y);
  return (!tmp ? 0 : tile_map[tmp->v].solid);
}

static float heuristic(int x0, int y0, int x1, int y1) {
  int dx = ((x0 >= x1) ? x0 - x1 : x1 - x0);
  int dy = ((y0 >= y1) ? y0 - y1 : y1 - y0);
#if defined(DEBUG_ASTAR_NO_DIAG)
  return EDGE_COST * (dx + dy);
#else
  return EDGE_COST * (dx + dy) + (DIAG_COST - 2.0 * EDGE_COST) * (dx < dy ? dx : dy);
#endif
}

int get_neighbours(dungeon_t* d, nodeptr_t current, nodeptr_t neighbours[MAX_NUM_NEIGHBOURS], float edge_costs[MAX_NUM_NEIGHBOURS]) {
  int num_written = 0;
#if defined(DEBUG_ASTAR_NO_DIAG)
  // TODO
#else
  for (int x = current->x - 1; x <= current->x + 1; x++) {
    for (int y = current->y - 1; y <= current->y + 1; y++) {
      nodeptr_t n = dungeon_in(d, x, y);
      
      if (!n || tile_map[n->v].solid || n->state == CLOSED)
        continue;
      
      float ecost = INFINITY;
      if (!(x == current->x || y == current->y)) {
        nodeptr_t c1 = dungeon_in(d, x, current->y);
        nodeptr_t c2 = dungeon_in(d, current->x, y);
        ecost = (!tile_map[c1->v].solid || !tile_map[c2->v].solid) ? DIAG_COST : LEAK_COST;
      } else
        ecost = EDGE_COST;
      
      neighbours[num_written] = n;
      edge_costs[num_written++] = ecost;
    }
  }
#endif
  return num_written;
}

void visit_neighbours(heap_t* h, nodeptr_t current, nodeptr_t goal, nodeptr_t neighbours[], float edge_costs[], int n_neighbors) {
  for (int i = 0; i < n_neighbors; i++) {
    nodeptr_t n = neighbours[i];
    if (current->cost_self + edge_costs[i] >= n->cost_self)
      continue;
    
    n->next = current;
    n->cost_self = current->cost_self + edge_costs[i];
    n->cost_goal = current->cost_self + heuristic(n->x, n->y, goal->x, goal->y);
    if (n->state == UNVISITED) {
      n->state = VISITED;
      heap_add(h, n);
    }
  }
}

static int astar_comp(const void* a, const void* b) {
  nodeptr_t n1 = (nodeptr_t)a, n2 = (nodeptr_t)b;
  float n1_cost = (n1->cost_self + n1->cost_goal);
  float n2_cost = (n2->cost_self + n2->cost_goal);
  return (n1_cost == n2_cost ? 0 : (n1_cost < n2_cost ? 1 : -1));
}

nodeptr_t astar(dungeon_t* d, int x0, int y0, int x1, int y1) {
  nodeptr_t start = dungeon_in(d, x0, y0), end = dungeon_in(d, x1, y1);
  if (!start || !end)
    return NULL;
  
  start->cost_self = 0;
  start->cost_goal = heuristic(x0, y0, x1, y1);
  heap_t* open = heap_new(d->w + d->h, astar_comp);
  heap_add(open, start);
  
  nodeptr_t current = NULL;
  while (peek(open)) {
    if ((current = heap_remove(open)) == end)
      break;
    
    current->state = CLOSED;
    nodeptr_t neighbours[MAX_NUM_NEIGHBOURS];
    float edge_costs[MAX_NUM_NEIGHBOURS];
    int n_neighbours = get_neighbours(d, current, neighbours, edge_costs);
    visit_neighbours(open, current, end, neighbours, edge_costs, n_neighbours);
  }
  
  heap_free(open);
  return end;
}

grid_t* celluar_automata(int w, int h, int fill_prob, int iterations, int survival, int starve) {
  int i, j, k, wc, nx, ny;
  grid_t* map = grid_new(w, h);
  
  for (i = 0; i < w; ++i)
    for (j = 0; j < h; ++j)
      GRID_AT(map, i, j) = ((i == 0 || i == w - 1 || j == 0 || j == h - 1) ? 1 : (RND_RANGE(0, 100) < fill_prob));
  
  for (k = 0; k < iterations; ++k) {
    for (i = 0; i < w; ++i) {
      for (j = 0; j < h; ++j) {
        wc = 0;
        
        for (nx = i - 1; nx <= i + 1; nx++) {
          for (ny = j - 1; ny <= j + 1; ny++) {
            if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
              if (nx != i || ny != j)
                wc += GRID_AT(map, nx, ny);
            }
            else wc++;
          }
        }
        
        if (wc > 4)
          GRID_AT(map, i, j) = 1;
        else if (wc < 3)
          GRID_AT(map, i, j) = 0;
      }
    }
  }
  
  return map;
}

void get_regions(point_t** out[2], grid_t* map, int w, int h) {
  int i, j, k;
  grid_t* map_flags = grid_new(w, h);
  point_t p, *tiles;
  for (i = 0; i < w; ++i) {
    for (j = 0; j < h; ++j) {
      if (!GRID_AT(map_flags, i, j)) {
        tiles = get_region_tiles(map, i, j);
        sb_push(out[GRID_AT(map, i, j)], tiles);
        for (k = 0; k < sb_count(tiles); ++k) {
          p = tiles[k];
          GRID_AT(map_flags, p.x, p.y) = 1;
        }
      }
    }
  }
  GRID_FREE(map_flags);
}

void dungeon_fill_rooms(dungeon_t* d, grid_t* map, point_t** tiles, int n, int* main_room) {
  int i, j, nx, ny, highest_tiles = 0, c = 0, ct = 0;
  d->rooms = NULL;
  point_t p;
  for (i = 0; i < n; ++i) {
    ct = sb_count(tiles[i]);
    if (ct < 10) {
      for (j = 0; j < ct; ++j) {
        p = tiles[i][j];
        GRID_AT(map, p.x, p.y) = 8;
      }
      continue;
    }
    
    room_t room = {
      .id = c,
      .tiles = tiles[i],
      .n_tiles = ct,
      .edges = NULL,
      .n_edges = 0
    };
    
    if (room.n_tiles > highest_tiles) {
      highest_tiles = room.n_tiles;
      *main_room = c;
    }
    c++;
    
    for (j = 0; j < room.n_tiles; ++j) {
      p = room.tiles[j];
      for (nx = p.x - 1; nx <= p.x + 1; nx++)
        for (ny = p.y - 1; ny <= p.y + 1; ny++)
          if ((nx == p.x || ny == p.y) && GRID_AT(map, nx, ny))
            sb_push(room.edges, p);
    }
    room.n_edges = sb_count(room.edges);
    sb_push(d->rooms, room);
  }
  d->n_rooms = sb_count(d->rooms);
}

void update_main_connections(int r, int** rci, int* mci, int n_rooms) {
  if (!mci[r]) {
    mci[r] = 1;
    for (int i = 0; i < n_rooms; ++i)
      if (i != r && rci[r][i])
        update_main_connections(i, rci, mci, n_rooms);
  }
}

static inline void swap(int* a, int* b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void draw_connection_path(grid_t* map, point_t* a, point_t* b) {
  int x = a->x;
  int y = a->y;
  int dx = b->x - a->x;
  int dy = b->y - a->y;
  int inverted = 0;
  int step = SIGN(dx);
  int gradient_step = SIGN(dy);
  int longest = ABS(dx);
  int shortest = ABS(dy);
  
  if (longest < shortest) {
    inverted = 1;
    swap(&longest, &shortest);
    swap(&step, &gradient_step);
  }
  
  int r = RND_RANGE(1, 3);
  int accum = longest / 2, i, cx, cy, nx, ny;
  for (i = 0; i < longest; ++i) {
    for (cx = -r; cx <= r; ++cx) {
      for (cy = -r; cy <= r; ++cy) {
        if (cx * cx + cy * cy <= r * r) {
          nx = x + cx;
          ny = y + cy;
          if (GRID_IN(map, nx, ny))
            GRID_AT(map, nx, ny) = 9;
        }
      }
    }
    
    if (inverted)
      y += step;
    else
      x += step;
    
    accum += shortest;
    if (accum >= longest) {
      if (inverted)
        x += gradient_step;
      else
        y += gradient_step;
      accum -= longest;
    }
  }
}

void connect_closest_rooms(grid_t* map, room_t* ra, int ral, room_t* rb, int rbl, int** rci, int* mci, int n_rooms) {
  int i, j, c, m, n, d, bd = 0, bra = 0, brb = 0;
  point_t *ba = NULL, *bb = NULL;
  for (i = 0; i < ral; ++i) {
    c = 0;
    for (j = 0; j < rbl; ++j) {
      if (ra[i].id == rb[j].id)
        continue;
      if (rci[ra[i].id][rb[j].id])
        continue;
      
      for (m = 0; m < ra[i].n_edges; ++m) {
        for (n = 0; n < rb[j].n_edges; ++n) {
          d = (int)(pow(ra[i].edges[m].x - rb[j].edges[n].x, 2) + pow(ra[i].edges[m].y - rb[j].edges[n].y, 2));
          if (d < bd || !c) {
            c = 1;
            bd = d;
            ba = &ra[i].edges[m];
            bb = &rb[j].edges[n];
            bra = ra[i].id;
            brb = rb[j].id;
          }
        }
      }
    }
    
    if (c == 1 && bd <= 50) {
      if (mci[bra])
        update_main_connections(brb, rci, mci, n_rooms);
      if (mci[brb])
        update_main_connections(bra, rci, mci, n_rooms);
      rci[bra][brb] = 1;
      rci[brb][bra] = 1;
      
      draw_connection_path(map, ba, bb);
    }
  }
}

void dungeon_connect_rooms(dungeon_t* d, grid_t* map, int main_room) {
  int** rci = malloc(d->n_rooms * sizeof(int*));
  int i, j, *mci = malloc(d->n_rooms * sizeof(int));
  
  for (i = 0; i < d->n_rooms; ++i) {
    mci[i] = 0;
    rci[i] = malloc(d->n_rooms * sizeof(int));
    for (j = 0; j < d->n_rooms; ++j)
      rci[i][j] = 0;
  }
  mci[main_room] = 1;
  
  connect_closest_rooms(map, d->rooms, d->n_rooms, d->rooms, d->n_rooms, rci, mci, d->n_rooms);
  update_main_connections(main_room, rci, mci, d->n_rooms);
  
  room_t *a = NULL, *b = NULL;
  for (i = 0; i < d->n_rooms; ++i) {
    if (mci[i])
      sb_push(b, d->rooms[i]);
    else
      sb_push(a, d->rooms[i]);
  }
  if (a && b)
    connect_closest_rooms(map, a, sb_count(a), b, sb_count(b), rci, mci, d->n_rooms);
  
  sb_free(a);
  sb_free(b);
  
  free(mci);
  for (i = 0; i < d->n_rooms; ++i)
    free(rci[i]);
  free(rci);
}

void new_cave(dungeon_t* d, int* progress, int minw, int minh, int maxw, int maxh, int fill_prob, int iterations, int survival, int starve) {
  int map_w = RND_RANGE(minw, maxh), map_h = RND_RANGE(minh, maxh);
  grid_t* map = celluar_automata(map_w, map_h, fill_prob, iterations, survival, starve);
  *progress = 20;
  
  point_t** regions[2] = { NULL, NULL };
  get_regions(regions, map, map_w, map_h);
  *progress = 40;
  
  int main_room = 0;
  dungeon_fill_rooms(d, map, regions[0], sb_count(regions[0]), &main_room);
  *progress = 50;
  dungeon_connect_rooms(d, map, main_room);
  *progress = 80;
  
  
  point_t *p = NULL, *region = NULL;
  int i, j, nw, nf, nx, ny;
  for (i = 0; i < sb_count(regions[1]); ++i) {
    region = regions[1][i];
    nw = sb_count(region);
    if (nw < 50) {
      for (j = 0; j < nw; ++j) {
        p = &region[j];
        GRID_AT(map, p->x, p->y) = 0;
      }
    } else {
      for (j = 0; j < nw; ++j) {
        p = &region[j];
        nf = 0;
        if (GRID_AT(map, p->x, p->y) != 1)
          continue;
        
        for (nx = p->x - 1; nx <= p->x + 1; nx++)
          for (ny = p->y - 1; ny <= p->y + 1; ny++)
            if (GRID_IN(map, nx, ny) && ((nx != p->x || ny != p->y)) && !GRID_AT(map, nx, ny))
              nf++;
        GRID_AT(map, p->x, p->y) = 8 - nf;
      }
    }
    sb_free(region);
  }
  sb_free(regions[0]);
  sb_free(regions[1]);
  *progress = 90;
  
  dungeon_fill(d, map);
  GRID_FREE(map);
  *progress = 100;
}

static int draw_distance = 5;

void draw_dungeon(surface_t* dst, dungeon_t* d) {
  int gmw = dst->w / 8, gmh = dst->h / 8;
  int cx = d->camera.x - (d->camera.x % 8),
  cy = d->camera.y - (d->camera.y % 8);
  int off_cx = cx - (gmw / 2);
  int off_cy = cy - (gmh / 2);
  int off_cgx = cx / 8;
  int off_cgy = cy / 8;
  int r = 95, g = 95, b = 95, c, i, j, k;
  int rgb_inc = (255 - 95) / TILE_MAP_MAX_H;
  
  for (k = 0; k < TILE_MAP_MAX_H; ++k) {
    for (i = 0; i < gmw; ++i) {
      for (j = 0; j < gmh; ++j) {
        int gx = i + off_cgx;
        int gy = j + off_cgy;
        if (gx < 0 || gy < 0 || gx >= d->w || gy >= d->h)
          continue;
        
        c = d->map[gy * d->w + gx].v;
        if (k > tile_map[c].size || tile_map[c].map[k] == ' ')
          continue;
        
        letter(dst, tile_map[c].map[k],
               (i * 8) + (k * ((i + off_cx) - cx)) / draw_distance,
               (j * 8) + (k * ((j + off_cy) - cy)) / draw_distance,
               RGB(r, g, b));
      }
    }
    r += rgb_inc;
    g += rgb_inc;
    b += rgb_inc;
  }
}
