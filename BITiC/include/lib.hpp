#pragma once

#include <algorithm>

template <typename T>
T Clamp(double x) {
  double ret = std::min<double>(std::numeric_limits<T>::max(), x);
  return std::max<double>(std::numeric_limits<T>::min(), ret);
}

template <typename T>
T Clamp(T x, T min, T max) {
  T ret = std::min(max, x);
  return std::max(min, ret);
}
