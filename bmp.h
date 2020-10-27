#pragma once

#include <cstdint>
#include <fstream>
#include <vector>
#include <utility>

struct BMPHeader {
  uint8_t type[2];
  uint32_t size, reserved, offbits;
  void read(std::ifstream &file, std::streampos offset = 0,
            std::ios_base::seekdir dir = std::ios_base::cur);
  void write(std::ofstream &file) const;
};

struct DIBHeader {
  uint32_t size;
  int32_t width, height, width_abs, height_abs;
  uint16_t planes, bit_count;
  uint32_t compression, size_image;
  int32_t x_pels_per_meter, y_pels_per_meter;  // in ppm
  uint32_t clr_used, clr_important;
  void read(std::ifstream &file, std::streampos offset = 0,
            std::ios_base::seekdir dir = std::ios_base::cur);
  void write(std::ofstream &file) const;
};

struct RGBColor {
  uint8_t r, g, b;
};

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
  void ModifyLuminanceLinear(const int delta);
  void ModifyLuminanceExponential(const double ratio);  // > 1 for darker

  void Binarization();  // global version
  void Binarization(const unsigned window_side_length,
                    const unsigned overlap_length);  // local adaptive version

  void Erosion(std::vector<std::pair<int, int>>
                   &structing_element);  // must be binarized before calling
};
