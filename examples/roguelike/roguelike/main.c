//
//  main.c
//  roguelike
//
//  Created by Rory B. Bellows on 22/06/2018.
//  Copyright © 2018 Rory B. Bellows. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "threads.h"
#include "dungeon.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

/* TODO:
 * - Replace heap & stb_sb
 * - Implement no diag pathfinding in A*
 * - Mouse movement
 * - Better way to store tile map
 */

//static int resized = 0;
static int running = 1, win_w = SCREEN_WIDTH, win_h = SCREEN_HEIGHT;
static int mx = 0, my = 0, lmx = 0, lmy = 0, mdown = 0, free_cam = 0;
static point_t mp, mgp;
static surface_t win;

static void on_resize(int w, int h) {
  win_w = w;
  win_h = h;
  
//  reset(&win, w, h);
//  print_f(&win, 4, 5, WHITE, "%dx%d\n", w, h);
//  resized = 1;
}

typedef struct {
  char c;
  point_t pos;
} entity_t;

void move_entity(entity_t* o, dungeon_t* d, int by_x, int by_y) {
  by_x = CLAMP(o->pos.x + by_x, 0, d->w - 1);
  by_y = CLAMP(o->pos.y + by_y, 0, d->h - 1);
  if (by_x == o->pos.x && by_y == o->pos.y)
    return;
  if (!dungeon_solid(d, by_x, by_y)) {
    o->pos.x = by_x;
    o->pos.y = by_y;
  }
}

typedef struct {
  surface_t s;
  point_t pos;
} panel_t;

void set_panel_pos(panel_t* p, int x, int y) {
  if (!p)
    return;
  p->pos.x = x;
  p->pos.y = y;
}

void draw_panel(surface_t* dst, panel_t* p) {
  blit(dst, &p->pos, &p->s, NULL, -1, -1);
}

void set_vec2_relative(point_t* pos, surface_t* dst, point_t* out) {
  out->x = (pos->x * 8) - (dst->w / 2);
  out->y = (pos->y * 8) -  (dst->h / 2);
}

void get_vec2_relative(point_t* cam, point_t* in, point_t* out) {
  out->x = (in->x / 8) + (cam->x - (cam->x % 8)) / 8;
  out->y = (in->y / 8) + (cam->y - (cam->y % 8)) / 8;
}

void grid_to_screen(int x0, int y0, int* x1, int* y1, point_t* cam) {
  *x1 = (x0 * 8) - (cam->x - (cam->x % 8));
  *y1 = (y0 * 8) - (cam->y - (cam->y % 8));
}

#define MOVE_PLAYER(x, y) \
move_entity(&player, &map, x, y); \
set_vec2_relative(&player.pos, &game_view.s, &map.camera); \
free_cam = 0;

typedef struct {
  dungeon_t* map;
  int* progress;
} map_load_thrd_t;

typedef struct {
  void (*init)(void*);
  void (*update)(long);
  void (*render)(void);
} state_t;

int load_map_func(void* _arg) {
  map_load_thrd_t* arg = (map_load_thrd_t*)_arg;
  new_cave_default(arg->map, arg->progress);
  return 0;
}

int main(int argc, const char * argv[]) {
  srand(((argc > 1) ? atoi(argv[1]) : (unsigned int)time(NULL)));
  
  if (!screen("roguelike", SCREEN_WIDTH, SCREEN_HEIGHT) || !surface(&win, SCREEN_WIDTH, SCREEN_HEIGHT)) {
    fprintf(stderr, "%s\n", get_last_error());
    return -1;
  }
  resize_callback(on_resize);
  
//  int map_load_progress = 0;
//  dungeon_t map;
//  thrd_t load_map_thrd;
//  map_load_thrd_t load_map_arg = {
//    .map = &map,
//    .progress = &map_load_progress
//  };
//  thrd_create(&load_map_thrd, load_map_func, (void*)(&load_map_arg));
  dungeon_t map;
  int p = 0;
  new_cave_default(&map, &p);
  
  int px = -1, py = -1;
  for (int i = 0; i < map.w; ++i) {
    for (int j = 0; j < map.h; ++j) {
      if (!dungeon_solid(&map, i, j)) {
        px = i;
        py = j;
        break;
      }
    }
    if (px > 0 && py > 0)
      break;
  }
  entity_t player = {
    .c = '@',
    .pos = { px, py }
  };
  
  panel_t game_view, inv_view;
  surface(&game_view.s, (int)(75 * win_w / 100), win_h);
  set_panel_pos(&game_view, 0, 0);
  surface(&inv_view.s, win_w - game_view.s.w, win_h);
  set_panel_pos(&inv_view, game_view.s.w, 0);
  set_vec2_relative(&player.pos, &game_view.s, &map.camera);

  user_event_t ue;
  long prev_frame_tick;
  long curr_frame_tick = ticks();
  int enable_mouse = 1;
  while (running) {
    prev_frame_tick = curr_frame_tick;
    curr_frame_tick = ticks();
    
    while (poll_events(&ue)) {
      switch (ue.type) {
        case WINDOW_CLOSED:
          running = 0;
          break;
        case KEYBOARD_KEY_DOWN:
          switch (ue.sym) {
#if defined(__APPLE__)
            case KB_KEY_Q:
            case KB_KEY_W:
              if (ue.mod & KB_MOD_SUPER)
                running = 0;
              break;
#else
            case KB_KEY_F4:
              if (ue.mod & KB_MOD_ALT)
                running = 0;
              break;
#endif
              break;
            default:
              break;
          }
          break;
        case KEYBOARD_KEY_UP:
          switch (ue.sym) {
            case KB_KEY_UP:
            case KB_KEY_W:
              MOVE_PLAYER(0, -1);
              break;
            case KB_KEY_DOWN:
            case KB_KEY_S:
              MOVE_PLAYER(0, 1);
              break;
            case KB_KEY_LEFT:
            case KB_KEY_A:
              MOVE_PLAYER(-1, 0);
              break;
            case KB_KEY_RIGHT:
            case KB_KEY_D:
              MOVE_PLAYER(1, 0);
              break;
            case KB_KEY_M:
              enable_mouse = !enable_mouse;
              break;
            default:
              break;
          }
        case MOUSE_BTN_DOWN:
          if (ue.btn != MOUSE_BTN_0 && ue.mod == KB_MOD_SHIFT)
            mdown = ue.btn;
          break;
        case MOUSE_BTN_UP:
          mdown = 0;
        default:
          break;
      }
    }
    
    long speed = curr_frame_tick - prev_frame_tick;

    get_mouse_pos(&mx, &my);
    mp.x = (int)(((float)mx / (float)win_w) * win.w);
    mp.y = (int)(((float)my / (float)win_h) * win.h);
    get_vec2_relative(&map.camera, &mp, &mgp);
    
    if (mdown && (lmx != mp.x || lmy != mp.y)) {
      map.camera.x -= (mp.x - lmx);
      map.camera.y -= (mp.y - lmy);
      free_cam = 1;
    }
    lmx = mp.x;
    lmy = mp.y;
    
    fill(&game_view.s, BLACK);
    fill(&inv_view.s, BLACK);
    
    yline(&inv_view.s, 0, 0, win_h, WHITE);
    
    print_f(&inv_view.s, 3, 2,  RED, "mouse:  %d %d", mp.x, mp.y);
    print_f(&inv_view.s, 3, 10, RED, "        %d %d", mgp.x, mgp.y);
    print_f(&inv_view.s, 3, 18, RED, "player: %d %d", player.pos.x, player.pos.y);
    print_f(&inv_view.s, 3, 26, RED, "camera: %d %d", map.camera.x, map.camera.y);
    
    
    // CLEAN UP
    if (enable_mouse) {
      nodeptr_t target = astar(&map, player.pos.x, player.pos.y, mgp.x, mgp.y);
      if (target && target->next) {
        int tx, ty, nx, ny;
        grid_to_screen(target->x, target->y, &tx, &ty, &map.camera);
        rect(&game_view.s, tx, ty, 8, 8, RGB(95, 95, 95), 0);
        for (nodeptr_t current = target; current != NULL; current = current->next) {
          grid_to_screen(current->x, current->y, &tx, &ty, &map.camera);
          if (current->next)
            grid_to_screen(current->next->x, current->next->y, &nx, &ny, &map.camera);
          else
            grid_to_screen(player.pos.x, player.pos.y, &nx, &ny, &map.camera);
          line(&game_view.s, tx, ty, nx, ny, RGB(95, 95, 95));
        }
        for (int i = 0; i < map.w * map.h; ++i) {
          map.map[i].state = UNVISITED;
          map.map[i].cost_self = INFINITY;
          map.map[i].cost_goal = INFINITY;
          map.map[i].next = NULL;
        }
      }
    }
    
    draw_panel(&win, &inv_view);

    draw_dungeon(&game_view.s, &map);
    
    int px, py;
    grid_to_screen(player.pos.x, player.pos.y, &px, &py, &map.camera);
    letter(&game_view.s, '@', px, py, WHITE);
    
    draw_panel(&win, &game_view);
    
    render(&win);
  }
  
  destroy(&game_view.s);
  destroy(&inv_view.s);
  destroy(&win);
  release();
  dungeon_free(&map);

  return 0;
}
