#pragma once
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <array>
#include "vec2.hpp"

template<typename... Args> static std::string fmt(const std::string& format, Args ... args) {
  std::size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
  std::unique_ptr<char[]> buf(new char[size]);
  std::snprintf(buf.get(), size, format.c_str(), args ...);
  return std::string(buf.get(), buf.get() + size - 1);
}

template <class T> using vector1d = std::vector<T>;
template <class T> using vector2d = vector1d<vector1d<T>>;

template <class T> static vector1d<T> create_vector1d(int sz, const T& v) {
  vector1d<T> ret;
  ret.resize(sz);
  std::fill(std::begin(ret), std::end(ret), v);
  return ret;
}

template <class T> static vector2d<T> create_vector2d(int w, int h, const T& v) {
  vector2d<T> ret;
  ret.resize(w);
  for (auto&& r : ret)
    r = create_vector1d<T>(h, v);
  return ret;
}

#if DEBUG
static void print_map_test(vector2d<int>& map, const char* tag) {
  printf("%s:\n", tag);
  for (int i = 0; i < map.size(); ++i) {
    for (int j = 0; j < map[i].size(); ++j)
      printf("%d", map[i][j]);
    printf("\n");
  }
  printf("\n");
}
#endif

static std::array<vec2i, 4> nsew(const vec2i& p) {
  return {
    vec2i(p.x, p.y + 1), // North
    vec2i(p.x, p.y - 1), // South
    vec2i(p.x + 1, p.y), // East
    vec2i(p.x - 1, p.y)  // West
  };
}

static int rnd_range(int a, int b) {
  static std::random_device rnd_dev;
  static std::mt19937 rnd_eng = std::mt19937(rnd_dev());
  std::uniform_int_distribution<> rnd_dist(a, b);
  return rnd_dist(rnd_eng);
}

static vec2i rnd_grid_pos(int w, int h) {
  return vec2i(rnd_range(0, w), rnd_range(0, h));
}

static vec2i rnd_grid_edge(int w, int h) {
  switch (rnd_range(0, 3)) {
    case 0:
      return vec2i(0, rnd_range(0, h));
    case 1:
      return vec2i(rnd_range(0, w), 0);
    case 2:
      return vec2i(w, rnd_range(0, h));
    case 3:
    default:
      return vec2i(rnd_range(0, w), h);
  }
}
