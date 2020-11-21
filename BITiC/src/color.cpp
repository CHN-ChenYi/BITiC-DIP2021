#include "color.hpp"

#include <cmath>

#include "lib.hpp"

void Bitmap::ToRGB() {
  if (flag) return;
  const int height = yuv_.size();
  const int width = yuv_[0].size();
  flag = true;
  Resize(height, width);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      // const int c = yuv_[i][j].y - 16;
      // const int d = yuv_[i][j].u - 128;
      // const int e = yuv_[i][j].v - 128;
      // rgb_[i][j] =
      //     RGBColor{Clamp((c * 298 + d * 516 + 128) >> 8, 0, 255),
      //              Clamp((c * 298 - d * 100 - e * 208 + 128) >> 8, 0, 255),
      //              Clamp((c * 298 + e * 409 + 128) >> 8, 0, 255)};
      rgb_[i][j] = RGBColor{Clamp<decltype(RGBColor::b)>(
                                yuv_[i][j].y + 2.03211 * (yuv_[i][j].u - 128)),
                            Clamp<decltype(RGBColor::g)>(
                                yuv_[i][j].y - 0.39465 * (yuv_[i][j].u - 128) -
                                0.58060 * (yuv_[i][j].v - 128)),
                            Clamp<decltype(RGBColor::r)>(
                                yuv_[i][j].y + 1.13983 * (yuv_[i][j].v - 128))};
    }
  }
  yuv_.resize(0);
}

void Bitmap::ToYUV() {
  if (!flag) return;
  const int height = rgb_.size();
  const int width = rgb_[0].size();
  flag = false;
  Resize(height, width);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      // yuv_[i][j] = YUVColor{
      //     ((rgb_[i][j].r * 66 + rgb_[i][j].g * 129 + rgb_[i][j].b * 25) >> 8)
      //     +
      //         16,
      //     ((rgb_[i][j].r * -38 + rgb_[i][j].g * -74 + rgb_[i][j].b * 112) >>
      //      8) +
      //         128,
      //     ((rgb_[i][j].r * 112 + rgb_[i][j].g * -94 + rgb_[i][j].b * -18) >>
      //      8) +
      //         128};
      yuv_[i][j] = YUVColor{
          0.299 * rgb_[i][j].r + 0.587 * rgb_[i][j].g + 0.114 * rgb_[i][j].b,
          -0.169 * rgb_[i][j].r - 0.331 * rgb_[i][j].g + 0.5 * rgb_[i][j].b +
              128,
          0.5 * rgb_[i][j].r - 0.419 * rgb_[i][j].g - 0.081 * rgb_[i][j].b +
              128};
    }
  }
  rgb_.resize(0);
}

#define CalcBilinearInterpolate(ret, name, uuid)                          \
  const double a_0##uuid = +1. * rgb_[x_1][y_1].name * x_2 * y_2 +        \
                           -1. * rgb_[x_1][y_2].name * x_2 * y_1 +        \
                           -1. * rgb_[x_2][y_1].name * x_1 * y_2 +        \
                           +1. * rgb_[x_2][y_2].name * x_1 * y_1;         \
  const double a_1##uuid =                                                \
      -1. * rgb_[x_1][y_1].name * y_2 + +1. * rgb_[x_1][y_2].name * y_1 + \
      +1. * rgb_[x_2][y_1].name * y_2 + -1. * rgb_[x_2][y_2].name * y_1;  \
  const double a_2##uuid =                                                \
      -1. * rgb_[x_1][y_1].name * x_2 + +1. * rgb_[x_1][y_2].name * x_2 + \
      +1. * rgb_[x_2][y_1].name * x_1 + -1. * rgb_[x_2][y_2].name * x_1;  \
  const double a_3##uuid =                                                \
      +1. * rgb_[x_1][y_1].name + -1. * rgb_[x_1][y_2].name +             \
      -1. * rgb_[x_2][y_1].name + +1. * rgb_[x_2][y_2].name;              \
  ret = a_0##uuid + a_1##uuid * x + a_2##uuid * y + a_3##uuid * x * y;

RGBColor Bitmap::BilinearInterpolate(const double &x, const double &y) {
  if (x < 0 || x > height() - 1 || y < 0 || y > width() - 1)
    return RGBColor{0, 0, 0};
  const int x_1 = floor(x), x_2 = ceil(x);
  const int y_1 = floor(y), y_2 = ceil(y);
  if (x_1 == x_2 && y_1 == y_2) {
    return rgb_[x_1][y_1];
  } else if (x_1 == x_2 || y_1 == y_2) {
    const double ratio = (x_1 == x_2) ? 1. * (y - y_1) / (y_2 - y_1)
                                      : 1. * (x - x_1) / (x_2 - x_1);
    return RGBColor{
        Clamp<decltype(RGBColor::b)>(ratio * rgb_[x_1][y_1].b +
                                     (1 - ratio) * rgb_[x_2][y_2].b + 0.5),
        Clamp<decltype(RGBColor::g)>(ratio * rgb_[x_1][y_1].g +
                                     (1 - ratio) * rgb_[x_2][y_2].g + 0.5),
        Clamp<decltype(RGBColor::r)>(ratio * rgb_[x_1][y_1].r +
                                     (1 - ratio) * rgb_[x_2][y_2].r + 0.5)};
  } else {
    double r, g, b;
    CalcBilinearInterpolate(r, r, 0);
    CalcBilinearInterpolate(g, g, 1);
    CalcBilinearInterpolate(b, b, 2);
    return RGBColor{Clamp<decltype(RGBColor::b)>(b + 0.5),
                    Clamp<decltype(RGBColor::g)>(g + 0.5),
                    Clamp<decltype(RGBColor::r)>(r + 0.5)};
  }
}
