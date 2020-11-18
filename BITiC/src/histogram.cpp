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
#include <cassert>
#include <iostream>
void BMP::HistogramTransforming(
    decltype(Channel::kGrayChannel) channel,
    std::function<double(const double &)> trans_func) {
  if (channel.none()) {
    for (int i = 0; i < dib_header_.height_abs; i++) {
      for (int j = 0; j < dib_header_.width_abs; j++)
        bitmap_.YUV()[i][j].y = trans_func(bitmap_.YUV()[i][j].y);
    }
    bitmap_.ToRGB();
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
                         const int &sample_size, T_2 dst_curve_T) {
  const double step_length = (max_val - min_val) / sample_size;
  std::vector<double> count;
  count.resize(sample_size + 1);
  for (short i = 0; i <= sample_size; i++) count[i] = 0;
  for (auto &it : curve) count[(it - min_val) / step_length + 0.5]++;
  for (auto &it : count) it /= curve.size();
  for (int i = 1; i <= sample_size; i++) count[i] += count[i - 1];
  for (auto &it : curve)
    it = dst_curve_T(count[(it - min_val) / step_length + 0.5]) * max_val +
         min_val + bias;
}

#define RGBChannelFitting(x)                                            \
  std::vector<decltype(bitmap_[0][0].x)> curve;                         \
  for (int i = 0; i < dib_header_.height_abs; i++) {                    \
    for (int j = 0; j < dib_header_.width_abs; j++)                     \
      curve.push_back(bitmap_[i][j].x);                                 \
  }                                                                     \
  CurveFitting(curve, uint8_t(0), uint8_t(255), 0.5, 255, dst_curve_T); \
  for (int i = 0; i < dib_header_.height_abs; i++) {                    \
    for (int j = 0; j < dib_header_.width_abs; j++)                     \
      bitmap_[i][j].x = curve[i * dib_header_.width_abs + j];           \
  }

void BMP::HistogramFitting(decltype(kGrayChannel) channel,
                           const int &sample_size,
                           std::function<double(const double &)> dst_curve_T) {
  if (channel.none()) {  // grey channel
    std::vector<double> y;
    for (int i = 0; i < dib_header_.height_abs; i++) {
      for (int j = 0; j < dib_header_.width_abs; j++)
        y.push_back(bitmap_.YUV()[i][j].y);
    }
    CurveFitting(y, 0.0, 255.0, 0, sample_size, dst_curve_T);
    for (int i = 0; i < dib_header_.height_abs; i++) {
      for (int j = 0; j < dib_header_.width_abs; j++) {
        const size_t index = i * dib_header_.width_abs + j;
        bitmap_.YUV()[i][j].y = y[index];
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
  double y_max = 0;
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++)
      y_max = std::max(y_max, bitmap_.YUV()[i][j].y);
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

void BMP::HistogramEqualization(decltype(kGrayChannel) channel,
                                const int &sample_size) {
  HistogramFitting(channel, sample_size, [](const double &x) { return x; });
}
