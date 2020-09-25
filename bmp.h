#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

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

// TODO: palette

struct Pixel {
  uint8_t r, g, b;
};

class BMP {
  BMPHeader bmp_header_;
  DIBHeader dib_header_;
  std::vector<std::vector<Pixel>> bitmap_;

 public:
  BMP();
  BMP(const char filename[]);
  ~BMP();
  void read(const char *filename);
  void write(const char filename[]);
};
