#pragma once

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <vector>

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

struct RGBColor {
  uint8_t b, g, r;  // in the storage order of BMP file
};

// Other channels will overwrite the gray channel
enum Channel {
  kGrayChannel,
  kRedChannel = 1,
  kGreenChannel = 2,
  kBlueChannel = 4
};

// bottom-left is the origin
class BMP {
  BMPHeader bmp_header_;
  DIBHeader dib_header_;
  std::vector<RGBColor> palette_;
  std::vector<std::vector<RGBColor>> bitmap_;
  uint8_t OtsuMethod(const unsigned histogram[256], const int pixel_count);

 public:
  BMP();
  BMP(const char filename[]);
  ~BMP();
  operator std::vector<std::vector<RGBColor>>() { return bitmap_; }
  std::vector<RGBColor> &operator[](int index) { return bitmap_[index]; }

  void read(const char *filename);
  void write(const char filename[]);

  int32_t width();
  int32_t height();
  void SetWidth(const int32_t width);
  void SetHeight(const int32_t height);

  void GrayScale();

  /*
  trans_func example:
  * auto linear_transform = [](const int &y) { return y + delta; };
  * auto exponential_transform = [](const int &y) {
  *   return int(exp(log(y / 255.0) * ratio) * 255); // ratio > 1 for darker
  * };
  */
  void ModifyLuminance(int (*trans_func)(const int &y));
  void LogarithmicEnhancement();
  void HistogramEqualization(Channel channel);

  void Binarization();  // global version
  void Binarization(const unsigned window_side_length,
                    const unsigned overlap_length);  // local adaptive version

  void Erosion(std::vector<std::pair<int, int>>
                   &structing_element);  // must be binarized before calling
  void Dilation(std::vector<std::pair<int, int>>
                    &structing_element);  // must be binarized before calling
  void Opening(std::vector<std::pair<int, int>>
                   &structing_element);  // must be binarized before calling
  void Closing(std::vector<std::pair<int, int>>
                   &structing_element);  // must be binarized before calling
};

}  // namespace BITiC
