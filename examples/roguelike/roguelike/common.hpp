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