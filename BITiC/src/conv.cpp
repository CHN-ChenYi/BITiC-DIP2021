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
  NormalizedConv(kernel, kRedChannel);
  NormalizedConv(kernel, kGreenChannel);
  NormalizedConv(kernel, kBlueChannel);
}

void BMP::LaplacianEnhancement() {
  std::vector<std::tuple<int, int, double>> kernel;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      kernel.push_back(std::make_tuple(i, j, !i && !j ? 9.0 : -1.0));
    }
  }
  NormalizedConv(kernel, kRedChannel);
  NormalizedConv(kernel, kGreenChannel);
  NormalizedConv(kernel, kBlueChannel);
}
