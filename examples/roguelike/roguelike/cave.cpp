#include "cave.hpp"

vector2d<int> cave_t::cellular_automata() {
  std::uniform_int_distribution<> fill_chance(0, 100);
  auto map = create_vector2d<int>(w, h, 0);

  int i, j;
  for (i = 0; i < w; ++i)
    for (j = 0; j < h; ++j)
      map[i][j] = ((i == 0 or i == w - 1 or j == 0 or j == h - 1) ? 1 : (fill_chance(rnd_eng) < fill_prob));

#if DEBUG
  print_map_test(map, "RANDOM FILL");
#endif

  int k, wc, nx, ny;
  for (k = 0; k < iterations; ++k) {
    for (i = 0; i < w; ++i) {
      for (j = 0; j < h; ++j) {
        wc = 0;

        for (nx = i - 1; nx <= i + 1; ++nx) {
          for (ny = j - 1; ny <= j + 1; ++ny) {
            if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
              if (nx != i || ny != j)
                wc += map[nx][ny];
            }
            else
              wc++;
          }
        }

        if (wc > survival)
          map[i][j] = 1;
        else if (wc < starve)
          map[i][j] = 0;
      }
    }
  }

#if DEBUG
  print_map_test(map, "CELLUAR AUTOMATA");
#endif

  return map;
}

static std::vector<vec2i> find_region_tiles(const vector2d<int>& in, int start_x, int start_y) {
  vector1d<vec2i> ret;
  const int w = in.size(), h = in[0].size();
  auto map_flags = create_vector2d<bool>(w, h, false);
  map_flags[start_x][start_y] = true;
  std::queue<vec2i> queue;
  queue.push(vec2i(start_x, start_y));
  const int type = in[start_x][start_y];

  while (not queue.empty()) {
    vec2i p = queue.front();
    queue.pop();
    ret.push_back(p);

    auto neighbours = nsew(p);
    for (auto&& p : neighbours) {
      if ((p.x < 1 or p.y < 1 or p.x >= w - 1 or p.y >= h - 1) or
        in[p.x][p.y] != type or
        map_flags[p.x][p.y])
        continue;
      map_flags[p.x][p.y] = true;
      queue.push(p);
    }
  }
  return ret;
}

static void find_regions(const vector2d<int>& in, const int type, vector2d<vec2i>& out) {
  const int w = in.size(), h = in[0].size();
  auto map_flags = create_vector2d<bool>(w, h, false);
  for (int i = 1; i < w - 1; ++i) {
    for (int j = 1; j < h - 1; ++j) {
      if (map_flags[i][j] or in[i][j] != type)
        continue;

      auto tiles = find_region_tiles(in, i, j);
      out.push_back(tiles);
      for (auto&& t : tiles)
        map_flags[t.x][t.y] = true;
    }
  }
}

void cave_t::get_rooms(const vector2d<int>& map) {
  int i, j;

  vector2d<vec2i> walls, floors;
  std::thread floors_thrd(find_regions, std::ref(map), 0, std::ref(floors));
  std::thread walls_thrd(find_regions, std::ref(map), 1, std::ref(walls));
  walls_thrd.join();
  floors_thrd.join();

#if DEBUG
  auto wall_map = create_vector2d<int>(w, h, 0);
  for (i = 0; i < walls.size(); ++i)
    for (auto&& v : walls[i])
      wall_map[v.x][v.y] = i + 1;
  print_map_test(wall_map, "WALLS");

  auto floor_map = create_vector2d<int>(w, h, 0);
  for (i = 0; i < floors.size(); ++i)
    for (auto&& v : floors[i])
      floor_map[v.x][v.y] = i + 1;
  print_map_test(floor_map, "FLOORS");
#endif

  main_room = std::distance(std::begin(floors), std::max_element(std::begin(floors), std::end(floors), [](const auto& lhs, const auto& rhs) { return lhs.size() < rhs.size(); }));
  for (i = 0; i < floors.size(); ++i) {
    room_t room;
    room.id = i;
    room.tiles = floors[i];
    for (auto&& v : room.tiles) {
      auto neighbours = nsew(v);
      for (auto&& n : neighbours) {
        if (map[n.x][n.y]) {
          room.edges.push_back(v);
          break;
        }
      }
    }
    rooms.push_back(room);
  }

#if DEBUG
  auto edge_map = create_vector2d<int>(w, h, 0);
  for (i = 0; i < rooms.size(); ++i)
    for (auto&& v : rooms[i].edges)
      edge_map[v.x][v.y] = i + 1;
  print_map_test(edge_map, "EDGES");
#endif
}

#define SIGN(x) ((x) == 0 ? 0 : ((x) < 0 ? -1 : 1))

void cave_t::draw_connection(vector2d<int>& map, const vec2i* a, const vec2i* b) {
  int x = a->x, y = a->y;
  int dx = b->x - a->x, dy = b->y - a->y;
  bool inverted = false;
  int step = SIGN(dx), grad_step = SIGN(dy);
  int longest = std::abs(dx);
  int shortest = std::abs(dy);

  if (longest < shortest) {
    inverted = true;
    std::swap(longest, shortest);
    std::swap(step, grad_step);
  }

  std::uniform_int_distribution<> fill_chance(1, 3);
  int r = fill_chance(rnd_eng);
  int accum = longest / 2, i, cx, cy, nx, ny;
  for (i = 0; i < longest; ++i) {
    for (cx = -r; cx <= r; ++cx) {
      for (cy = -r; cy <= r; ++cy) {
        if (cx * cx + cy * cy <= r * r) {
          nx = x + cx;
          ny = y + cy;
          if (nx >= 1 and ny >= 1 and nx <= map.size() - 1 and ny <= map[0].size() - 1 and map[nx][y])
            map[nx][ny] = 9;
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
        x += grad_step;
      else
        y += grad_step;
      accum -= longest;
    }
  }
}

void cave_t::update_main_room_connections(int r, vector2d<bool>& rci, vector1d<bool>& mci) {
  if (not mci[r]) {
    mci[r] = true;
    for (int i = 0; i < rooms.size(); ++i)
      if (i != r and rci[r][i])
        update_main_room_connections(i, rci, mci);
  }
}

void cave_t::connect_closest_rooms(vector2d<int>& map, const vector1d<room_t>& rooms_a, const vector1d<room_t>& rooms_b, vector2d<bool>& rci, vector1d<bool>& mci) {
  for (int i = 0; i < rooms_a.size(); ++i) {
    bool match = false;
    float best_dist = .0f;
    int best_room_a = -1, best_room_b = -1;
    const vec2i *best_edge_a = nullptr, *best_edge_b = nullptr;
    for (int j = 0; j < rooms_b.size(); ++j) {
      if (rooms_a[i].id == rooms_b[j].id or rci[rooms_a[i].id][rooms_b[j].id])
        continue;

      for (int ii = 0; ii < rooms_a[i].edges.size(); ++ii) {
        for (int jj = 0; jj < rooms_b[j].edges.size(); ++jj) {
          float dist = rooms_a[i].edges[ii].dist(rooms_b[j].edges[jj]);
          if (dist > best_dist && match)
            continue;

          match = true;
          best_dist = dist;
          best_room_a = rooms_a[i].id;
          best_room_b = rooms_b[j].id;
          best_edge_a = &rooms_a[i].edges[ii];
          best_edge_b = &rooms_b[j].edges[jj];
        }
      }
    }

    if (match && best_dist < 50.f) {
      if (mci[best_room_a])
        update_main_room_connections(best_room_b, rci, mci);
      if (mci[best_room_b])
        update_main_room_connections(best_room_a, rci, mci);

      rci[best_room_a][best_room_b] = true;
      rci[best_room_b][best_room_a] = true;

      draw_connection(map, best_edge_a, best_edge_b);
    }
  }
}

void cave_t::connect_rooms(vector2d<int>& map) {
  auto rci = create_vector2d<bool>(rooms.size(), rooms.size(), false);
  auto mci = create_vector1d<bool>(rooms.size(), false);
  mci[main_room] = true;
  connect_closest_rooms(map, rooms, rooms, rci, mci);

  vector1d<room_t> rooms_a, rooms_b;
  for (int i = 0; i < rooms.size(); ++i) {
    if (mci[i])
      rooms_b.push_back(rooms[i]);
    else
      rooms_a.push_back(rooms[i]);
  }
  if (not rooms_a.empty())
    connect_closest_rooms(map, rooms_a, rooms_b, rci, mci);

#if DEBUG
  print_map_test(map, "CONNECTED");
#endif
}

void cave_t::finalize_map(vector2d<int>& map) {
  for (auto&& m : map)
    std::replace(std::begin(m), std::end(m), 1, 8);

  int i, j, nf;
  for (auto&& r : rooms) {
    for (auto&& p : r.edges) {
      nf = 0;
      for (i = p.x - 1; i < p.x + 1; ++i)
        for (j = p.y - 1; j < p.y + 1; ++j)
          if ((i >= 0 and i < map.size() and j >= 0 and j < map[0].size()) and (i != p.x or j != p.y) and !map[i][j])
            nf++;
      map[p.x][p.y] = 8 - nf;
    }
  }
  
  fill_entities();

#if DEBUG
  print_map_test(map, "FINAL");
#endif
}

cave_t::cave_t(std::vector<std::string>* info, int _depth, int _w, int _h, int _fill_prob, int _iterations, int _survival, int _starve): w(_w), h(_h), fill_prob(_fill_prob), iterations(_iterations), survival(_survival), starve(_starve) {
  if (!w or !h) {
    std::uniform_int_distribution<> rnd_wh(50, 200);
    if (!w)
      w = rnd_wh(rnd_eng);
    if (!h)
      h = rnd_wh(rnd_eng);
  }
  depth = _depth;
  info_ref = info;
  if (info_ref)
    info_ref->push_back(fmt("Generating map at %dx%d", w, h));
}

void cave_t::generate() {
  DO_SOMETHING("Cellular, automata...", auto _map = cellular_automata());
  DO_SOMETHING("Finding rooms...", get_rooms(_map));
  DO_SOMETHING("Connecting rooms...", connect_rooms(_map));
  DO_SOMETHING("Finalizing map...", finalize_map(_map));
  DO_SOMETHING("Filling map...", fill_map(_map));
  loaded = true;
}
