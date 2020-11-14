#include <algorithm>

#include "BITiC.hpp"

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

void BMP::GrayScale() {
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++)
      bitmap_[i][j].r = bitmap_[i][j].g = bitmap_[i][j].b =
          ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
            bitmap_[i][j].b * 25) >>
           8) +
          16;
  }
}

void BMP::ModifyLuminance(int (*trans_func)(const int &y)) {
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      // BT.601 SD TV standard
      // RGB -> Y'UV
      int y = ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
                bitmap_[i][j].b * 25) >>
               8) +
              16;
      int u = ((bitmap_[i][j].r * -38 + bitmap_[i][j].g * -74 +
                bitmap_[i][j].b * 112) >>
               8) +
              128;
      int v = ((bitmap_[i][j].r * 112 + bitmap_[i][j].g * -94 +
                bitmap_[i][j].b * -18) >>
               8) +
              128;

      // ModifyLuminance
      y = Clamp(trans_func(y), 0, 255);

      // Y'UV -> RGB
      const int c = y - 16;
      const int d = u - 128;
      const int e = v - 128;
      bitmap_[i][j].r = Clamp((c * 298 + e * 409 + 128) >> 8, 0, 255);
      bitmap_[i][j].g = Clamp((c * 298 - d * 100 - e * 208 + 128) >> 8, 0, 255);
      bitmap_[i][j].b = Clamp((c * 298 + d * 516 + 128) >> 8, 0, 255);
    }
  }
}
