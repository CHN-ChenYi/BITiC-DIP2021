#pragma once

#include <cstdint>
#include <fstream>
#include <vector>

struct BMPHeader {
  uint8_t id_field[2];
  uint32_t file_size, application_specific, offset;
  void read(std::ifstream &file, std::streampos offset = 0,
            std::ios_base::seekdir dir = std::ios_base::cur);
};

struct DIBHeader {
  uint32_t header_size;
  int32_t width, height, width_abs, height_abs;
  uint16_t plane_num, color_depth;
  uint32_t compression_method, image_size;
  int32_t horizontal_resolution, vertical_resolution;  // in ppm
  uint32_t color_palette_size, important_color_num;
  void read(std::ifstream &file, std::streampos offset = 0,
            std::ios_base::seekdir dir = std::ios_base::cur);
};

// TODO: palette

struct Pixel {
  uint8_t r, g, b;
};

class BMP {
  BMPHeader bmp_header_;
  DIBHeader dib_header_;
  std::vector<std::vector<Pixel>> pixels_;

 public:
  BMP();
  BMP(const char filename[]);
  ~BMP();
  void read(const char *filename);
  void write(const char filename[]);
};
