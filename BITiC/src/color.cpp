#include "color.hpp"

#include "lib.hpp"

void Bitmap::ToRGB() {
  if (flag) return;
  const int height = yuv_.size();
  const int width = yuv_[0].size();
  flag = true;
  Resize(width, height);
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      // const int c = yuv_[i][j].y - 16;
      // const int d = yuv_[i][j].u - 128;
      // const int e = yuv_[i][j].v - 128;
      // rgb_[i][j] =
      //     RGBColor{Clamp((c * 298 + d * 516 + 128) >> 8, 0, 255),
      //              Clamp((c * 298 - d * 100 - e * 208 + 128) >> 8, 0, 255),
      //              Clamp((c * 298 + e * 409 + 128) >> 8, 0, 255)};
      rgb_[i][j] = RGBColor{
          Clamp<decltype(RGBColor::b)>(yuv_[i][j].y + 2.03211 * (yuv_[i][j].u - 128)),
          Clamp<decltype(RGBColor::g)>(yuv_[i][j].y - 0.39465 * (yuv_[i][j].u - 128) -
                                0.58060 * (yuv_[i][j].v - 128)),
          Clamp<decltype(RGBColor::r)>(yuv_[i][j].y + 1.13983 * (yuv_[i][j].v - 128))};
    }
  }
  yuv_.resize(0);
}

void Bitmap::ToYUV() {
  if (!flag) return;
  const int height = rgb_.size();
  const int width = rgb_[0].size();
  flag = false;
  Resize(width, height);
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
