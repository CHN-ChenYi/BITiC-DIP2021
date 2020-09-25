#include "bmp.h"

#include <cstring>
#include <stdexcept>

void BMPHeader::read(std::ifstream &file, std::streampos offset,
                     std::ios_base::seekdir dir) {
  file.seekg(offset, dir);
  file.read((char *)&type, sizeof(type[0]) * 2);
  if (type[0] != 0x42 || type[1] != 0x4D)
    throw std::runtime_error("Unsupported ID field, possibly not a bmp file.");
  file.read((char *)&size, sizeof(size));
  file.read((char *)&reserved, sizeof(reserved));
  file.read((char *)&offbits, sizeof(offbits));
}

void BMPHeader::write(std::ofstream &file) const {
  file.write((char *)&type, sizeof(type[0]) * 2);
  file.write((char *)&size, sizeof(size));
  file.write((char *)&reserved, sizeof(reserved));
  file.write((char *)&offbits, sizeof(offbits));
}

void DIBHeader::read(std::ifstream &file, std::streampos offset,
                     std::ios_base::seekdir dir) {
  file.seekg(offset, dir);
  file.read((char *)&size, sizeof(size));
  if (size != 40)
    throw std::runtime_error(
        "Unsupported DIB header, only support BITMAPINFOHEADER.");
  file.read((char *)&width, sizeof(width));
  width_abs = width < 0 ? -width : width;
  file.read((char *)&height, sizeof(height));
  height_abs = height < 0 ? -height : height;
  file.read((char *)&planes, sizeof(planes));
  if (planes != 1)
    throw std::runtime_error(
        "The number of color planes isn't 1, the image might be broken.");
  file.read((char *)&bit_count, sizeof(bit_count));
  if (bit_count != 24) throw std::runtime_error("Unsupported color depth.");
  file.read((char *)&compression, sizeof(compression));
  if (compression != 0)
    throw std::runtime_error(
        "Unsupported compression method, only support BI_RGB.");
  file.read((char *)&size_image, sizeof(size_image));
  file.read((char *)&x_pels_per_meter, sizeof(x_pels_per_meter));
  file.read((char *)&y_pels_per_meter, sizeof(y_pels_per_meter));
  file.read((char *)&clr_used, sizeof(clr_used));
  file.read((char *)&clr_important, sizeof(clr_important));
}

void DIBHeader::write(std::ofstream &file) const {
  file.write((char *)&size, sizeof(size));
  file.write((char *)&width, sizeof(width));
  file.write((char *)&height, sizeof(height));
  file.write((char *)&planes, sizeof(planes));
  file.write((char *)&bit_count, sizeof(bit_count));
  file.write((char *)&compression, sizeof(compression));
  file.write((char *)&size_image, sizeof(size_image));
  file.write((char *)&x_pels_per_meter, sizeof(x_pels_per_meter));
  file.write((char *)&y_pels_per_meter, sizeof(y_pels_per_meter));
  file.write((char *)&clr_used, sizeof(clr_used));
  file.write((char *)&clr_important, sizeof(clr_important));
}

BMP::BMP() {
  memset(&bmp_header_, 0, sizeof(bmp_header_));
  memset(&dib_header_, 0, sizeof(dib_header_));
  palette_.resize(0);
  bitmap_.resize(0);
}

BMP::BMP(const char filename[]) { read(filename); }

BMP::~BMP() {}

void BMP::read(const char filename[]) {
  std::ifstream file(filename, std::ios_base::binary);
  if (!file) throw std::runtime_error("Cannot open the image file.");

  bmp_header_.read(file);
  dib_header_.read(file);

  for (unsigned i = 0; i < dib_header_.clr_used; i++) {  // TODO: test palette
    RGBColor rgb;
    uint8_t rgb_reserved;
    file.read((char *)&rgb.b, sizeof(rgb.b));
    file.read((char *)&rgb.g, sizeof(rgb.g));
    file.read((char *)&rgb.r, sizeof(rgb.r));
    file.read((char *)&rgb_reserved, sizeof(rgb_reserved));
    palette_.push_back(rgb);
  }

  file.seekg(bmp_header_.offbits);
  for (int i = 0; i < dib_header_.height_abs; i++)
    bitmap_.push_back(std::vector<RGBColor>(dib_header_.width_abs));
  for (int i = dib_header_.height >= 0 ? 0 : dib_header_.height_abs - 1;
       i >= 0 && i < dib_header_.height_abs;
       dib_header_.height >= 0 ? i++ : i--) {
    for (int j = dib_header_.width >= 0 ? 0 : dib_header_.width_abs - 1;
         j >= 0 && j < dib_header_.width_abs;
         dib_header_.width >= 0 ? j++ : j--) {
      file.read((char *)&bitmap_[i][j].b, sizeof(bitmap_[0][0].b));
      file.read((char *)&bitmap_[i][j].g, sizeof(bitmap_[0][0].g));
      file.read((char *)&bitmap_[i][j].r, sizeof(bitmap_[0][0].r));
    }
    file.seekg(dib_header_.width_abs * 3 % 4, std::ios_base::cur);
  }

  file.close();
}

void BMP::write(const char filename[]) {
  std::ofstream file(filename, std::ios_base::binary);
  if (!file) throw std::runtime_error("Cannot open the image file.");

  bmp_header_.type[0] = 0x42;
  bmp_header_.type[1] = 0x4D;
  dib_header_.bit_count = 24;
  dib_header_.size_image = dib_header_.bit_count / 8 * dib_header_.height_abs *
                           (dib_header_.width_abs % 4 + dib_header_.width_abs);
  bmp_header_.size = 54 + dib_header_.size_image;
  bmp_header_.reserved = 0x0000;
  bmp_header_.offbits = 54;
  bmp_header_.write(file);

  dib_header_.size = 40;
  dib_header_.width = dib_header_.width_abs;
  dib_header_.height = dib_header_.height_abs;
  dib_header_.planes = 1;
  dib_header_.compression = 0;
  dib_header_.x_pels_per_meter = 10000;  // TODO: choose a reasonable value
  dib_header_.y_pels_per_meter = 10000;  // TODO: choose a reasonable value
  dib_header_.clr_used = 0;
  dib_header_.clr_important = 0;
  dib_header_.write(file);

  file.seekp(bmp_header_.offbits);
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      file.write((char *)&bitmap_[i][j].b, sizeof(bitmap_[0][0].b));
      file.write((char *)&bitmap_[i][j].g, sizeof(bitmap_[0][0].g));
      file.write((char *)&bitmap_[i][j].r, sizeof(bitmap_[0][0].r));
    }
    file.seekp(dib_header_.width_abs * 3 % 4, std::ios_base::cur);
  }

  file.close();
}

void BMP::SetWidth(int32_t width) {
  dib_header_.width_abs = width;
  for (int32_t i = 0; i < dib_header_.height_abs; i++) bitmap_[i].resize(width);
}

void BMP::SetHeight(int32_t height) {
  int32_t old_height = dib_header_.height_abs;
  dib_header_.height_abs = height;
  bitmap_.resize(height);
  for (int32_t i = old_height; i < height; i++)
    bitmap_[i].resize(dib_header_.width_abs);
}

void BMP::GrayScale() {
// #pragma omp parallel for schedule(guided)
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++)
      bitmap_[i][j].r = bitmap_[i][j].g = bitmap_[i][j].b =
          bitmap_[i][j].r * 0.299 + bitmap_[i][j].g * 0.587 +
          bitmap_[i][j].b * 0.114;
  }
}
