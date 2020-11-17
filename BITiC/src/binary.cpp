#include <algorithm>
#include <cmath>
#include <cstring>

#include "BITiC.hpp"
using namespace BITiC;

uint8_t BMP::OtsuMethod(const unsigned histogram[256], const int pixel_count) {
  double probability[256], mean[256];
  probability[0] = 1.0 * histogram[0] / pixel_count;
  mean[0] = 0;
  for (int i = 1; i < 256; i++) {
    const double prob = 1.0 * histogram[i] / pixel_count;
    probability[i] = probability[i - 1] + prob;
    mean[i] = mean[i - 1] + i * prob;
  }
  uint8_t threshold = std::numeric_limits<uint8_t>::min();
  double max_between = std::numeric_limits<double>::min(), between;
  for (int i = 0; i < 256 && probability[i] != 1; i++) {
    if (probability[i] != 0) {
      between = pow(mean[255] * probability[i] - mean[i], 2) /
                (probability[i] * (1.0 - probability[i]));
      if (max_between < between) {
        max_between = between;
        threshold = i;
      }
    }
  }
  return threshold;
}

void BMP::Binarization() {
  const int pixel_count = dib_header_.height_abs * dib_header_.width_abs;
  uint8_t *y = new uint8_t[pixel_count];
  unsigned histogram[256];
  memset(histogram, 0, sizeof(histogram));
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      const int index = i * dib_header_.width_abs + j;
      y[index] = ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
                   bitmap_[i][j].b * 25) >>
                  8) +
                 16;
      histogram[y[index]]++;
    }
  }
  const uint8_t threshold = OtsuMethod(histogram, pixel_count);
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      if (y[i * dib_header_.width_abs + j] < threshold)
        bitmap_[i][j] = RGBColor{0, 0, 0};
      else
        bitmap_[i][j] = RGBColor{255, 255, 255};
    }
  }
  delete[] y;
}

void BMP::Binarization(const unsigned window_side_length,
                       const unsigned overlap_length) {
  if (window_side_length <= overlap_length)
    throw std::runtime_error(
        "window_side_length must be greater than overlap_length");
  const unsigned step_length = window_side_length - overlap_length;
  const int pixel_count = dib_header_.height_abs * dib_header_.width_abs;
  uint8_t *y = new uint8_t[pixel_count];
  bool *is_foreground = new bool[pixel_count];
  memset(is_foreground, false, sizeof(bool) * pixel_count);
  unsigned histogram[256];
  // init y
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++)
      y[i * dib_header_.width_abs + j] =
          ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
            bitmap_[i][j].b * 25) >>
           8) +
          16;
  }
  // sliding window
  for (int start_i = 0; start_i < dib_header_.height_abs;
       start_i += step_length) {
    for (int start_j = 0; start_j < dib_header_.width_abs;
         start_j += step_length) {
      // init window
      memset(histogram, 0, sizeof(histogram));
      const int height =
          std::min<int>(window_side_length, dib_header_.height_abs - start_i);
      const int width =
          std::min<int>(window_side_length, dib_header_.width_abs - start_j);
      // update histogram
      for (int i = start_i; i < start_i + height; i++) {
        for (int j = start_j; j < start_j + width; j++)
          histogram[y[i * dib_header_.width_abs + j]]++;
      }
      const uint8_t threshold = OtsuMethod(histogram, height * width);
      // update is_foreground
      for (int i = start_i; i < start_i + height; i++) {
        for (int j = start_j; j < start_j + width; j++) {
          const int index = i * dib_header_.width_abs + j;
          is_foreground[index] |= y[index] >= threshold;
        }
      }
    }
  }
  // binarization
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      const int index = i * dib_header_.width_abs + j;
      if (is_foreground[index])
        bitmap_[i][j] = RGBColor{255, 255, 255};
      else
        bitmap_[i][j] = RGBColor{0, 0, 0};
    }
  }
  delete[] is_foreground;
  delete[] y;
}

#define DILATION_OR_EROSION(init_value, condition_expression)             \
  bool *is_foreground =                                                   \
      new bool[dib_header_.height_abs * dib_header_.width_abs];           \
  int x_lower_bound = std::numeric_limits<int>::max();                    \
  int x_upper_bound = std::numeric_limits<int>::min();                    \
  int y_lower_bound = std::numeric_limits<int>::max();                    \
  int y_upper_bound = std::numeric_limits<int>::min();                    \
  for (auto &it : structing_element) {                                    \
    x_lower_bound = std::min(x_lower_bound, it.first);                    \
    x_upper_bound = std::max(x_upper_bound, it.first);                    \
    y_lower_bound = std::min(y_lower_bound, it.second);                   \
    y_upper_bound = std::max(y_upper_bound, it.second);                   \
  }                                                                       \
  for (int i = 0; i < dib_header_.height_abs; i++) {                      \
    for (int j = 0; j < dib_header_.width_abs; j++) {                     \
      const int index = i * dib_header_.width_abs + j;                    \
      is_foreground[index] = init_value;                                  \
      for (auto &it : structing_element) {                                \
        if (i + it.first < 0 || i + it.first >= dib_header_.height_abs || \
            j + it.second < 0 || j + it.second >= dib_header_.width_abs)  \
          continue;                                                       \
        if (condition_expression) {                                       \
          is_foreground[index] ^= 1;                                      \
          break;                                                          \
        }                                                                 \
      }                                                                   \
    }                                                                     \
  }                                                                       \
  for (int i = 0; i < dib_header_.height_abs; i++) {                      \
    for (int j = 0; j < dib_header_.width_abs; j++) {                     \
      const int index = i * dib_header_.width_abs + j;                    \
      if (is_foreground[index])                                           \
        bitmap_[i][j] = RGBColor{255, 255, 255};                          \
      else                                                                \
        bitmap_[i][j] = RGBColor{0, 0, 0};                                \
    }                                                                     \
  }                                                                       \
  delete[] is_foreground;

void BMP::Erosion(std::vector<std::pair<int, int>> &structing_element) {
  DILATION_OR_EROSION(true, !bitmap_[i + it.first][j + it.second].r);
}

void BMP::Dilation(std::vector<std::pair<int, int>> &structing_element) {
  DILATION_OR_EROSION(false, bitmap_[i + it.first][j + it.second].r);
}

void BMP::Opening(std::vector<std::pair<int, int>> &structing_element) {
  Erosion(structing_element);
  Dilation(structing_element);
}

void BMP::Closing(std::vector<std::pair<int, int>> &structing_element) {
  Dilation(structing_element);
  Erosion(structing_element);
}
