#include "bmp.h"

#include <iostream>
#include <stdexcept>

void BMPHeader::read(std::ifstream &file, std::streampos offset,
                     std::ios_base::seekdir dir) {
  file.seekg(offset, dir);
  file.read((char *)&id_field, sizeof(id_field[0]) * 2);
  if (id_field[0] != 0x42 || id_field[1] != 0x4D)
    throw std::runtime_error("Unsupported ID field, possibly not a bmp file.");
  file.read((char *)&file_size, sizeof(file_size));
  file.read((char *)&application_specific, sizeof(application_specific));
  file.read((char *)&this->offset, sizeof(this->offset));
}

void BMPHeader::write(std::ofstream &file) const {
  file.write((char *)&id_field, sizeof(id_field[0]) * 2);
  file.write((char *)&file_size, sizeof(file_size));
  file.write((char *)&application_specific, sizeof(application_specific));
  file.write((char *)&offset, sizeof(offset));
}

void DIBHeader::read(std::ifstream &file, std::streampos offset,
                     std::ios_base::seekdir dir) {
  file.seekg(offset, dir);
  file.read((char *)&header_size, sizeof(header_size));
  if (header_size != 40)
    throw std::runtime_error(
        "Unsupported DIB header, only support BITMAPINFOHEADER.");
  file.read((char *)&width, sizeof(width));
  width_abs = width < 0 ? -width : width;
  file.read((char *)&height, sizeof(height));
  height_abs = height < 0 ? -height : height;
  file.read((char *)&plane_num, sizeof(plane_num));
  if (plane_num != 1)
    throw std::runtime_error(
        "The number of color planes isn't 1, the image might be broken.");
  file.read((char *)&color_depth, sizeof(color_depth));
  if (color_depth != 24) throw std::runtime_error("Unsupported color depth.");
  file.read((char *)&compression_method, sizeof(compression_method));
  if (compression_method != 0)
    throw std::runtime_error(
        "Unsupported compression method, only support BI_RGB.");
  file.read((char *)&image_size, sizeof(image_size));
  file.read((char *)&horizontal_resolution, sizeof(horizontal_resolution));
  file.read((char *)&vertical_resolution, sizeof(vertical_resolution));
  file.read((char *)&color_palette_size, sizeof(color_palette_size));
  if (color_palette_size != 0) throw std::runtime_error("Unsupported palette.");
  file.read((char *)&important_color_num, sizeof(important_color_num));
}

void DIBHeader::write(std::ofstream &file) const {
  file.write((char *)&header_size, sizeof(header_size));
  file.write((char *)&width, sizeof(width));
  file.write((char *)&height, sizeof(height));
  file.write((char *)&plane_num, sizeof(plane_num));
  file.write((char *)&color_depth, sizeof(color_depth));
  file.write((char *)&compression_method, sizeof(compression_method));
  file.write((char *)&image_size, sizeof(image_size));
  file.write((char *)&horizontal_resolution, sizeof(horizontal_resolution));
  file.write((char *)&vertical_resolution, sizeof(vertical_resolution));
  file.write((char *)&color_palette_size, sizeof(color_palette_size));
  file.write((char *)&important_color_num, sizeof(important_color_num));
}

BMP::BMP() {}

BMP::BMP(const char filename[]) { read(filename); }

BMP::~BMP() {}

void BMP::read(const char filename[]) {
  std::ifstream file(filename, std::ios_base::binary);
  if (!file) throw std::runtime_error("Cannot open the image file.");

  bmp_header_.read(file);
  dib_header_.read(file);

  file.seekg(bmp_header_.offset);
  for (int i = 0; i < dib_header_.height_abs; i++)
    pixels_.push_back(std::vector<Pixel>(dib_header_.width_abs));
  for (int i = dib_header_.height >= 0 ? 0 : dib_header_.height_abs - 1;
       i >= 0 && i < dib_header_.height_abs;
       dib_header_.height >= 0 ? i++ : i--) {
    for (int j = dib_header_.width >= 0 ? 0 : dib_header_.width_abs - 1;
         j >= 0 && j < dib_header_.width_abs;
         dib_header_.width >= 0 ? j++ : j--) {
      file.read((char *)&pixels_[i][j].b, sizeof(pixels_[0][0].b));
      file.read((char *)&pixels_[i][j].g, sizeof(pixels_[0][0].g));
      file.read((char *)&pixels_[i][j].r, sizeof(pixels_[0][0].r));
    }
    file.seekg(dib_header_.width_abs * 3 % 4, std::ios_base::cur);
  }

  file.close();
}

void BMP::write(const char filename[]) {
  std::ofstream file(filename, std::ios_base::binary);
  if (!file) throw std::runtime_error("Cannot open the image file.");

  bmp_header_.id_field[0] = 0x42;
  bmp_header_.id_field[1] = 0x4D;
  dib_header_.color_depth = 24;
  dib_header_.image_size = dib_header_.color_depth / 8 * dib_header_.height_abs *
                           (dib_header_.width_abs % 4 + dib_header_.width_abs);
  bmp_header_.file_size = 54 + dib_header_.image_size;
  bmp_header_.application_specific = 0x0000;
  bmp_header_.offset = 54;
  bmp_header_.write(file);

  dib_header_.header_size = 40;
  dib_header_.width = dib_header_.width_abs;
  dib_header_.height = dib_header_.height_abs;
  dib_header_.plane_num = 1;
  dib_header_.compression_method = 0;
  dib_header_.horizontal_resolution = 10000;  // TODO: choose a reasonable value
  dib_header_.vertical_resolution = 10000;    // TODO: choose a reasonable value
  dib_header_.color_palette_size = 0;
  dib_header_.important_color_num = 0;
  dib_header_.write(file);

  file.seekp(bmp_header_.offset);
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      file.write((char *)&pixels_[i][j].b, sizeof(pixels_[0][0].b));
      file.write((char *)&pixels_[i][j].g, sizeof(pixels_[0][0].g));
      file.write((char *)&pixels_[i][j].r, sizeof(pixels_[0][0].r));
    }
    file.seekp(dib_header_.width_abs * 3 % 4, std::ios_base::cur);
  }

  file.close();
}
