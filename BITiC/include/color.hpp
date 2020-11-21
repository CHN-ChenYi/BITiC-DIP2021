#pragma once
#include <algorithm>
#include <cstdint>
#include <vector>

struct RGBColor {
  uint8_t b, g, r;
};

// BT.601 SD TV standard: Y'UV
struct YUVColor {
  double y, u, v;
};

class Bitmap {
  bool flag;  // true if rgb
  std::vector<std::vector<RGBColor>> rgb_;
  std::vector<std::vector<YUVColor>> yuv_;

 public:
  Bitmap() : flag(true) {}
  Bitmap(const Bitmap &other)
      : flag(other.flag), rgb_(other.rgb_), yuv_(other.yuv_) {}
  Bitmap(const int &width, const int &height) : flag(true) {
    Resize(width, height);
  }

  operator std::vector<std::vector<RGBColor>> &() {
    if (!flag) ToRGB();
    return rgb_;
  }
  operator std::vector<std::vector<YUVColor>> &() {
    if (flag) ToYUV();
    return yuv_;
  }

  std::vector<RGBColor> &operator[](int index) {
    if (!flag) ToRGB();
    return rgb_[index];
  }
  std::vector<std::vector<RGBColor>> &RGB() {
    if (!flag) ToRGB();
    return rgb_;
  }
  std::vector<std::vector<YUVColor>> &YUV() {
    if (flag) ToYUV();
    return yuv_;
  }

  int width() const {
    if (flag)
      return rgb_[0].size();
    else
      return yuv_[0].size();
  }

  int height() const {
    if (flag)
      return rgb_.size();
    else
      return yuv_.size();
  }

  void Resize(const int width, const int height) {
    if (flag) {
      auto old_height = rgb_.size();
      rgb_.resize(height);
      for (int32_t i = old_height; i < height; i++) rgb_[i].resize(width);
    } else {
      auto old_height = yuv_.size();
      yuv_.resize(height);
      for (int32_t i = old_height; i < height; i++) yuv_[i].resize(width);
    }
  }

  void Reverse(const bool &horizontal, const bool &vertical) {
    if (flag) {
      if (vertical) std::reverse(rgb_.begin(), rgb_.end());
      if (horizontal) {
        for (auto &it : rgb_) std::reverse(it.begin(), it.end());
      }
    } else {
      if (vertical) std::reverse(yuv_.begin(), yuv_.end());
      if (horizontal) {
        for (auto &it : yuv_) std::reverse(it.begin(), it.end());
      }
    }
  }

  void ToRGB();
  void ToYUV();

  RGBColor BilinearInterpolate(const double &x, const double &y);
};
