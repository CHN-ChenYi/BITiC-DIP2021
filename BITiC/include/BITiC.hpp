#pragma once

#include <bitset>
#include <cstdint>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "color.hpp"
namespace BITiC {

struct BMPHeader {
  uint32_t size, reserved, offbits;
  void read(std::ifstream &file, std::streampos offset = 0,
            std::ios_base::seekdir dir = std::ios_base::cur);
  void write(std::ofstream &file) const;
};

struct DIBHeader {
  int32_t width, height;
  uint16_t planes, bit_count;
  uint32_t compression, size_image;
  int32_t x_pels_per_meter, y_pels_per_meter;  // in ppm
  uint32_t clr_used, clr_important;
  int32_t width_abs, height_abs;
  void read(std::ifstream &file, std::streampos offset = 0,
            std::ios_base::seekdir dir = std::ios_base::cur);
  void write(std::ofstream &file) const;
};

// Other channels will overwrite the gray channel
namespace Channel {
static const std::bitset<3> kGrayChannel = 0, kRedChannel = 1,
                            kGreenChannel = 2, kBlueChannel = 4;
}  // namespace Channel

// bottom-left is the origin
class BMP {
  BMPHeader bmp_header_;
  DIBHeader dib_header_;
  // std::vector<RGBColor> palette_;
  Bitmap bitmap_;
  uint8_t OtsuMethod(const unsigned histogram[256], const int pixel_count);

 public:
  BMP();
  BMP(const char filename[]);
  ~BMP();
  operator Bitmap() { return bitmap_; }

  void read(const char *filename);
  void write(const char filename[]);

  int32_t width();
  int32_t height();
  void SetWidth(const int32_t width);
  void SetHeight(const int32_t height);
  void Resize(const int32_t height, const int32_t width);

  void GrayScale();

  /*
  trans_func example:
  * auto linear_transform = [](const double &y) { return y + delta; };
  * auto exponential_transform = [](const double &y) {
  *   return int(exp(log(y / 255.0) * ratio) * 255); // ratio > 1 for darker
  * };
  */
  void ModifyLuminance(std::function<double(const double &)> trans_func);

  // trans_func: [0, 256) -> [0, 256)
  void HistogramTransforming(decltype(Channel::kGrayChannel) channel,
                             std::function<double(const double &)> trans_func);
  // dst_histogram_cumulative_distribution_func: [0, 1] -> [0, 1]
  void HistogramFitting(decltype(Channel::kGrayChannel) channel,
                        const int &sample_size,
                        std::function<double(const double &)>
                            dst_histogram_cumulative_distribution_func_T);
  void LogarithmicEnhancement();
  void LogarithmicEnhancement(const double &a, const double &b,
                              const double &c);
  void HistogramEqualization(
      decltype(Channel::kGrayChannel) channel,
      const int &sample_size =
          255);  // sample_size only works for the gray channel

  void Binarization();  // global version
  void Binarization(const unsigned &window_side_length,
                    const unsigned &overlap_length);  // local adaptive version

  void Erosion(std::vector<std::pair<int, int>>
                   &structing_element);  // must be binarized before calling
  void Dilation(std::vector<std::pair<int, int>>
                    &structing_element);  // must be binarized before calling
  void Opening(std::vector<std::pair<int, int>>
                   &structing_element);  // must be binarized before calling
  void Closing(std::vector<std::pair<int, int>>
                   &structing_element);  // must be binarized before calling

  void Translation(const int &delta_height, const int &delta_width);
  void Mirror(const bool &horizontal, const bool &vertical);
  void Shear(const bool &horizontal_or_vertical,
             const double &d);  // 0 for horizontal, 1 for vertical
  void Scale(const double &ratio_height, const double &ratio_width);
  void Rotate(const double &theta);  // counter-clockwise, in rad

  // kernel example: (0, 0, 1) (-1, 0, 1) (1, 0, 1)
  void NormalizedConv(std::vector<std::tuple<int, int, double>> &kernel,
                      decltype(Channel::kGrayChannel) channel);
  void Conv(std::vector<std::tuple<int, int, double>> &kernel,
            decltype(Channel::kGrayChannel) channel, int padding_number = 0);
  void MeanFilter();
  void LaplacianEnhancement(const double &ratio);

  void BilateralFilter(const double &sigma_s, const double &sigma_r, const unsigned &half_window_side_length);
};

}  // namespace BITiC
