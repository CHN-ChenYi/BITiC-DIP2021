#include <cmath>

#include "BITiC.hpp"
#include "lib.hpp"

using namespace BITiC;
using namespace BITiC::Channel;

#define normalized_conv(old_one, new_one)                   \
  Bitmap new_bitmap = bitmap_;                              \
  for (int i = 0; i < dib_header_.height_abs; i++) {        \
    for (int j = 0; j < dib_header_.width_abs; j++) {       \
      int h, w;                                             \
      double weight = 0, sum = 0;                           \
      for (const auto &it : kernel) {                       \
        h = i + std::get<0>(it);                            \
        if (h < 0 || dib_header_.height_abs <= h) continue; \
        w = j + std::get<1>(it);                            \
        if (w < 0 || dib_header_.width_abs <= w) continue;  \
        weight += std::get<2>(it);                          \
        sum += std::get<2>(it) * old_one;                   \
      }                                                     \
      new_one = Clamp(sum / weight, 0.0, 255.0);            \
    }                                                       \
  }                                                         \
  bitmap_ = new_bitmap;

void BMP::NormalizedConv(std::vector<std::tuple<int, int, double>> &kernel,
                         decltype(Channel::kGrayChannel) channel) {
  if (channel.none()) {
    normalized_conv(bitmap_.YUV()[h][w].y, new_bitmap.YUV()[i][j].y);
  } else {
    if ((channel & kRedChannel).any()) {
      normalized_conv(bitmap_[h][w].r, new_bitmap[i][j].r);
    }
    if ((channel & kGreenChannel).any()) {
      normalized_conv(bitmap_[h][w].g, new_bitmap[i][j].g);
    }
    if ((channel & kBlueChannel).any()) {
      normalized_conv(bitmap_[h][w].b, new_bitmap[i][j].b);
    }
  }
}

#define conv(old_one, new_one)                              \
  Bitmap new_bitmap = bitmap_;                              \
  for (int i = 0; i < dib_header_.height_abs; i++) {        \
    for (int j = 0; j < dib_header_.width_abs; j++) {       \
      int h, w;                                             \
      double sum = 0;                                       \
      for (const auto &it : kernel) {                       \
        h = i + std::get<0>(it);                            \
        if (h < 0 || dib_header_.height_abs <= h) continue; \
        w = j + std::get<1>(it);                            \
        if (w < 0 || dib_header_.width_abs <= w) continue;  \
        sum += std::get<2>(it) * old_one;                   \
      }                                                     \
      new_one = Clamp(sum, 0.0, 255.0);                     \
    }                                                       \
  }                                                         \
  bitmap_ = new_bitmap;

void BMP::Conv(std::vector<std::tuple<int, int, double>> &kernel,
               decltype(Channel::kGrayChannel) channel, int padding_number) {
  if (channel.none()) {
    conv(bitmap_.YUV()[h][w].y, new_bitmap.YUV()[i][j].y);
  } else {
    if ((channel & kRedChannel).any()) {
      conv(bitmap_[h][w].r, new_bitmap[i][j].r);
    }
    if ((channel & kGreenChannel).any()) {
      conv(bitmap_[h][w].g, new_bitmap[i][j].g);
    }
    if ((channel & kBlueChannel).any()) {
      conv(bitmap_[h][w].b, new_bitmap[i][j].b);
    }
  }
}

void BMP::MeanFilter() {
  std::vector<std::tuple<int, int, double>> kernel;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      kernel.push_back(std::make_tuple(i, j, 1.0));
    }
  }
  NormalizedConv(kernel, kRedChannel | kGreenChannel | kBlueChannel);
}

void BMP::LaplacianEnhancement(const double &ratio) {
  std::vector<std::tuple<int, int, double>> kernel;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      kernel.push_back(
          std::make_tuple(i, j, !i && !j ? 1 + 8 * ratio : -ratio));
    }
  }
  NormalizedConv(kernel, kRedChannel | kGreenChannel | kBlueChannel);
}

inline double G(const double &x_square, const double &sigma) {
  // static const double ratio = sqrt(2 * acos(-1.));
  return exp(-x_square / (2 * pow(sigma, 2)));  // / (sigma * ratio);
}

void BMP::BilateralFilter(const double &sigma_s, const double &sigma_r) {
  Bitmap new_bitmap = bitmap_;
  // int max_delta = 0;
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      printf("\rBilateralFiltering: %5d %5d", i, j);
      const int now_r = bitmap_[i][j].r, now_g = bitmap_[i][j].g,
                now_b = bitmap_[i][j].b;
      double sum_r = 0, sum_g = 0, sum_b = 0, w_r = 0, w_g = 0, w_b = 0,
             tmp = 0, tmp_r = 0, tmp_g = 0, tmp_b = 0;
      for (int h = 0; h < dib_header_.height; h++) {
        for (int w = 0; w < dib_header_.width; w++) {
          tmp = G(pow(i - h, 2) + pow(j - w, 2), sigma_s);
          tmp_r = tmp * G(pow(now_r - bitmap_[h][w].r, 2), sigma_r);
          tmp_g = tmp * G(pow(now_g - bitmap_[h][w].g, 2), sigma_r);
          tmp_b = tmp * G(pow(now_b - bitmap_[h][w].b, 2), sigma_r);
          w_r += tmp_r;
          w_g += tmp_g;
          w_b += tmp_b;
          sum_r += tmp_r * bitmap_[h][w].r;
          sum_g += tmp_g * bitmap_[h][w].g;
          sum_b += tmp_b * bitmap_[h][w].b;
        }
      }
      new_bitmap[i][j] = RGBColor{decltype(RGBColor::b)(sum_b / w_b),
                                  decltype(RGBColor::g)(sum_g / w_g),
                                  decltype(RGBColor::r)(sum_r / w_r)};
      // max_delta = std::max(max_delta, new_bitmap[i][j].r - now_r);
      // max_delta = std::max(max_delta, new_bitmap[i][j].g - now_g);
      // max_delta = std::max(max_delta, new_bitmap[i][j].b - now_b);
    }
  }
  printf("\nBilateralFilter Done!\n");
  // printf("%d\n", max_delta);
  bitmap_ = new_bitmap;
}
