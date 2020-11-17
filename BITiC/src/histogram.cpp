#include <algorithm>
#include <cmath>

#include "BITiC.hpp"
#include "lib.hpp"
using namespace BITiC;
using namespace BITiC::Channel;

#define RGBChannelTransforming(x)                    \
  for (int i = 0; i < dib_header_.height_abs; i++) { \
    for (int j = 0; j < dib_header_.width_abs; j++)  \
      bitmap_[i][j].x = trans_func(bitmap_[i][j].x); \
  }

void BMP::HistogramTransforming(
    decltype(Channel::kGrayChannel) channel,
    std::function<double(const double &)> trans_func) {
  if (channel.none()) {
    std::vector<decltype(bitmap_[0][0].r)> y, u, v;
    for (int i = 0; i < dib_header_.height_abs; i++) {
      for (int j = 0; j < dib_header_.width_abs; j++) {
        y.push_back(((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
                      bitmap_[i][j].b * 25) >>
                     8) +
                    16);
        u.push_back(((bitmap_[i][j].r * -38 + bitmap_[i][j].g * -74 +
                      bitmap_[i][j].b * 112) >>
                     8) +
                    128);
        v.push_back(((bitmap_[i][j].r * 112 + bitmap_[i][j].g * -94 +
                      bitmap_[i][j].b * -18) >>
                     8) +
                    128);
      }
    }
    for (auto &it : y) it = trans_func(it);
    for (auto i = 0; i < dib_header_.height_abs; i++) {
      for (auto j = 0; j < dib_header_.width_abs; j++) {
        const size_t index = i * dib_header_.width_abs + j;
        const int c = y[index] - 16;
        const int d = u[index] - 128;
        const int e = v[index] - 128;
        bitmap_[i][j].r = Clamp((c * 298 + e * 409 + 128) >> 8, 0, 255);
        bitmap_[i][j].g =
            Clamp((c * 298 - d * 100 - e * 208 + 128) >> 8, 0, 255);
        bitmap_[i][j].b = Clamp((c * 298 + d * 516 + 128) >> 8, 0, 255);
      }
    }
  } else {
    if ((channel & kRedChannel).any()) {
      RGBChannelTransforming(r);
    }
    if ((channel & kGreenChannel).any()) {
      RGBChannelTransforming(g);
    }
    if ((channel & kBlueChannel).any()) {
      RGBChannelTransforming(b);
    }
  }
}

template <typename T, typename T_2>
inline void CurveFitting(std::vector<T> &curve, const T &min_val,
                         const T &max_val, const double &bias,
                         T_2 dst_curve_T) {
  // const T max_val = *std::max_element(curve.begin(), curve.end()),
  //         min_val = *std::min_element(curve.begin(), curve.end());
  const double step_length = (max_val - min_val) / 255.0;
  std::vector<double> count;
  count.resize(256);
  for (auto i = 0; i < 256; i++) count[i] = 0;
  for (auto &it : curve) count[(it - min_val) / step_length + 0.5]++;
  for (auto &it : count) it /= curve.size();
  for (int i = 1; i < 256; i++) count[i] += count[i - 1];
  for (auto &it : curve)
    it = dst_curve_T(count[(it - min_val) / step_length + 0.5]) *
             (max_val - min_val) +
         min_val + bias;
}

#define RGBChannelFitting(x)                                       \
  std::vector<decltype(bitmap_[0][0].x)> curve;                    \
  for (auto i = 0; i < dib_header_.height_abs; i++) {              \
    for (auto j = 0; j < dib_header_.width_abs; j++)               \
      curve.push_back(bitmap_[i][j].x);                            \
  }                                                                \
  CurveFitting(curve, uint8_t(0), uint8_t(255), 0.5, dst_curve_T); \
  for (auto i = 0; i < dib_header_.height_abs; i++) {              \
    for (auto j = 0; j < dib_header_.width_abs; j++)               \
      bitmap_[i][j].x = curve[i * dib_header_.width_abs + j];      \
  }

void BMP::HistogramFitting(decltype(kGrayChannel) channel,
                           std::function<double(const double &)> dst_curve_T) {
  if (channel.none()) {  // grey channel
    std::vector<decltype(bitmap_[0][0].r)> y, u, v;
    for (int i = 0; i < dib_header_.height_abs; i++) {
      for (int j = 0; j < dib_header_.width_abs; j++) {
        y.push_back(((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
                      bitmap_[i][j].b * 25) >>
                     8) +
                    16);
        u.push_back(((bitmap_[i][j].r * -38 + bitmap_[i][j].g * -74 +
                      bitmap_[i][j].b * 112) >>
                     8) +
                    128);
        v.push_back(((bitmap_[i][j].r * 112 + bitmap_[i][j].g * -94 +
                      bitmap_[i][j].b * -18) >>
                     8) +
                    128);
      }
    }
    CurveFitting(y, uint8_t(0), uint8_t(255), 0.5, dst_curve_T);
    for (auto i = 0; i < dib_header_.height_abs; i++) {
      for (auto j = 0; j < dib_header_.width_abs; j++) {
        const size_t index = i * dib_header_.width_abs + j;
        const int c = y[index] - 16;
        const int d = u[index] - 128;
        const int e = v[index] - 128;
        bitmap_[i][j].r = Clamp((c * 298 + e * 409 + 128) >> 8, 0, 255);
        bitmap_[i][j].g =
            Clamp((c * 298 - d * 100 - e * 208 + 128) >> 8, 0, 255);
        bitmap_[i][j].b = Clamp((c * 298 + d * 516 + 128) >> 8, 0, 255);
      }
    }
  } else {
    if ((channel & kRedChannel).any()) {
      RGBChannelFitting(r);
    }
    if ((channel & kGreenChannel).any()) {
      RGBChannelFitting(g);
    }
    if ((channel & kBlueChannel).any()) {
      RGBChannelFitting(b);
    }
  }
}

void BMP::LogarithmicEnhancement() {
  int y_max = 0;
  for (auto i = 0; i < dib_header_.height_abs; i++) {
    for (auto j = 0; j < dib_header_.width_abs; j++)
      y_max = std::max(y_max, ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
                                bitmap_[i][j].b * 25) >>
                               8) +
                                  16);
  }
  HistogramTransforming(Channel::kGrayChannel, [y_max](const double &x) {
    return log(x / 255.0 + 1) / log(y_max / 255.0 + 1) * 255;
  });
}

void BMP::LogarithmicEnhancement(const double &a, const double &b,
                                 const double &c) {
  HistogramTransforming(Channel::kGrayChannel, [a, b, c](const double &x) {
    return Clamp<double>(a + log(x + 1) / b / log(c), 0, 255);
  });
}

void BMP::HistogramEqualization(decltype(kGrayChannel) channel) {
  HistogramFitting(channel, [](const double &x) { return x; });
}
