#include "BITiC.hpp"

void BMPHeader::read(std::ifstream &file, std::streampos offset,
                     std::ios_base::seekdir dir) {
  file.seekg(offset, dir);
  uint8_t type[2];
  file.read((char *)&type, sizeof(type[0]) * 2);
  if (type[0] != 0x42 || type[1] != 0x4D)
    throw std::runtime_error("Unsupported ID field, possibly not a bmp file.");
  file.read((char *)this, sizeof(BMPHeader));
}

void BMPHeader::write(std::ofstream &file) const {
  const uint8_t type[2] = {0x42, 0x4D};
  file.write((char *)&type, sizeof(type[0]) * 2);
  file.write((char *)this, sizeof(BMPHeader));
}

void DIBHeader::read(std::ifstream &file, std::streampos offset,
                     std::ios_base::seekdir dir) {
  file.seekg(offset, dir);
  uint32_t size;
  file.read((char *)&size, sizeof(size));
  if (size != 40)
    throw std::runtime_error(
        "Unsupported DIB header, only support BITMAPINFOHEADER.");
  file.read((char *)&width, size - sizeof(size));
  if (width < 0)
    throw std::runtime_error("Width in DIB header can't be negative.");
  width_abs = width;
  height_abs = height < 0 ? -height : height;
  if (planes != 1)
    throw std::runtime_error(
        "The number of color planes isn't 1, the image might be broken.");
  if (bit_count != 24) throw std::runtime_error("Unsupported color depth.");
  if (compression != 0)
    throw std::runtime_error(
        "Unsupported compression method, only support BI_RGB.");
}

void DIBHeader::write(std::ofstream &file) const {
  const uint32_t size = 40;
  file.write((char *)&size, sizeof(size));
  file.write((char *)&width, size - sizeof(size));
}

void BMP::read(const char filename[]) {
  std::ifstream file(filename, std::ios_base::binary);
  if (!file) throw std::runtime_error("Cannot open the image file.");

  bmp_header_.read(file);
  dib_header_.read(file);

  // read palette
  for (unsigned i = 0; i < dib_header_.clr_used; i++) {  // TODO: test palette
    RGBColor rgb;
    uint8_t rgb_reserved;
    file.read((char *)&rgb.b, sizeof(rgb.b));
    file.read((char *)&rgb.g, sizeof(rgb.g));
    file.read((char *)&rgb.r, sizeof(rgb.r));
    file.read((char *)&rgb_reserved, sizeof(rgb_reserved));
    palette_.push_back(rgb);
  }

  // read image data
  file.seekg(bmp_header_.offbits);
  bitmap_.resize(dib_header_.height_abs);
  for (int i = 0; i < dib_header_.height_abs; i++)
    bitmap_[i].resize(dib_header_.width_abs);
  size_t line_size = dib_header_.width_abs * dib_header_.bit_count / 8;
  line_size += (4 - line_size % 4) % 4;
  for (int i = dib_header_.height >= 0 ? 0 : dib_header_.height_abs - 1;
       i >= 0 && i < dib_header_.height_abs;
       dib_header_.height >= 0 ? i++ : i--)
    file.read((char *)bitmap_[i].data(), line_size);
  file.close();
}

void BMP::write(const char filename[]) {
  std::ofstream file(filename, std::ios_base::binary);
  if (!file) throw std::runtime_error("Cannot open the image file.");

  dib_header_.bit_count = 24;
  dib_header_.size_image = dib_header_.bit_count / 8 * dib_header_.height_abs *
                           (dib_header_.width_abs % 4 + dib_header_.width_abs);
  bmp_header_.size = 54 + dib_header_.size_image;
  bmp_header_.reserved = 0x0000;
  bmp_header_.offbits = 54;
  bmp_header_.write(file);

  dib_header_.width = dib_header_.width_abs;
  dib_header_.height = dib_header_.height_abs;
  dib_header_.planes = 1;
  dib_header_.compression = 0;
  dib_header_.x_pels_per_meter = 10000;  // TODO: choose a reasonable value
  dib_header_.y_pels_per_meter = 10000;  // TODO: choose a reasonable value
  dib_header_.clr_used = 0;
  dib_header_.clr_important = 0;
  dib_header_.write(file);

  // write image data
  const size_t line_size = dib_header_.width_abs * dib_header_.bit_count / 8;
  const size_t padding_size = (4 - line_size % 4) % 4;
  uint8_t *padding = new uint8_t[padding_size];
  memset(padding, 0, padding_size);
  file.seekp(bmp_header_.offbits);
  for (int i = 0; i < dib_header_.height_abs; i++) {
    file.write((char *)bitmap_[i].data(), line_size);
    file.write((char *)padding, padding_size);
  }
  delete padding;

  file.close();
}
