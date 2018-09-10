#pragma once
#include <cmath>

template <class T> class vec2 {
public:
  T x, y;

  vec2() : x(0), y(0) {}
  vec2(T x, T y) : x(x), y(y) {}
  vec2(const vec2& v) : x(v.x), y(v.y) {}

  vec2& operator=(const vec2& v) {
    x = v.x;
    y = v.y;
    return *this;
  }

  vec2 operator+(vec2& v) {
    return vec2(x + v.x, y + v.y);
  }
  vec2 operator-(vec2& v) {
    return vec2(x - v.x, y - v.y);
  }

  vec2& operator+=(vec2& v) {
    x += v.x;
    y += v.y;
    return *this;
  }
  vec2& operator-=(vec2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }

  vec2 operator+(double s) {
    return vec2(x + s, y + s);
  }
  vec2 operator-(double s) {
    return vec2(x - s, y - s);
  }
  vec2 operator*(double s) {
    return vec2(x * s, y * s);
  }
  vec2 operator/(double s) {
    return vec2(x / s, y / s);
  }

  vec2& operator+=(double s) {
    x += s;
    y += s;
    return *this;
  }
  vec2& operator-=(double s) {
    x -= s;
    y -= s;
    return *this;
  }
  vec2& operator*=(double s) {
    x *= s;
    y *= s;
    return *this;
  }
  vec2& operator/=(double s) {
    x /= s;
    y /= s;
    return *this;
  }

  void set(T x, T y) {
    this->x = x;
    this->y = y;
  }

  void rotate(double deg) {
    double t = deg / 180.0 * M_PI;
    double c = cos(t);
    double s = sin(t);
    x = x * c - y * s;
    y = x * s + y * c;
  }

  vec2& normalize() {
    if (not length())
      return *this;
    *this *= (1.0 / length());
    return *this;
  }

  float dist(vec2 v) const {
    return vec2(v.x - x, v.y - y).length();
  }
  float length() const {
    return std::sqrt(x * x + y * y);
  }
  void truncate(double length) {
    double angle = std::atan2f(y, x);
    x = length * cos(angle);
    y = length * sin(angle);
  }

  vec2 ortho() const {
    return vec2(y, -x);
  }

  static float dot(vec2 v1, vec2 v2) {
    return v1.x * v2.x + v1.y * v2.y;
  }
  static float cross(vec2 v1, vec2 v2) {
    return (v1.x * v2.y) - (v1.y * v2.x);
  }
};

typedef vec2<float> vec2f;
typedef vec2<double> vec2d;
typedef vec2<int> vec2i;
