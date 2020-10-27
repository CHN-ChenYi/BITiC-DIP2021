#include "bmp.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>

template <typename T>
T Clamp(double x) {
  double ret = std::min<double>(std::numeric_limits<T>::max(), x);
  return std::max<double>(std::numeric_limits<T>::min(), ret);
}

template <typename T>
T Clamp(T x, T min, T max) {
  T ret = std::min(max, x);
  return std::max(min, ret);
}

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

  // write image data
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

int32_t BMP::width() { return dib_header_.width_abs; }
int32_t BMP::height() { return dib_header_.height_abs; }

void BMP::SetWidth(const int32_t width) {
  if (width < 0) return;
  dib_header_.width_abs = width;
  for (int32_t i = 0; i < dib_header_.height_abs; i++) bitmap_[i].resize(width);
}

void BMP::SetHeight(const int32_t height) {
  if (height < 0) return;
  int32_t old_height = dib_header_.height_abs;
  dib_header_.height_abs = height;
  bitmap_.resize(height);
  for (int32_t i = old_height; i < height; i++)
    bitmap_[i].resize(dib_header_.width_abs);
}

void BMP::GrayScale() {
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++)
      bitmap_[i][j].r = bitmap_[i][j].g = bitmap_[i][j].b =
          ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
            bitmap_[i][j].b * 25) >>
           8) +
          16;
  }
}

void BMP::ModifyLuminanceLinear(const int delta) {
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      // BT.601 SD TV standard
      // RGB -> Y'UV
      int y = ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
                bitmap_[i][j].b * 25) >>
               8) +
              16;
      int u = ((bitmap_[i][j].r * -38 + bitmap_[i][j].g * -74 +
                bitmap_[i][j].b * 112) >>
               8) +
              128;
      int v = ((bitmap_[i][j].r * 112 + bitmap_[i][j].g * -94 +
                bitmap_[i][j].b * -18) >>
               8) +
              128;

      // ModifyLuminance
      y = Clamp(y + delta, 0, 255);

      // Y'UV -> RGB
      const int c = y - 16;
      const int d = u - 128;
      const int e = v - 128;
      bitmap_[i][j].r = Clamp((c * 298 + e * 409 + 128) >> 8, 0, 255);
      bitmap_[i][j].g = Clamp((c * 298 - d * 100 - e * 208 + 128) >> 8, 0, 255);
      bitmap_[i][j].b = Clamp((c * 298 + d * 516 + 128) >> 8, 0, 255);
    }
  }
}

void BMP::ModifyLuminanceExponential(const double ratio) {
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      // BT.601 SD TV standard
      // RGB -> Y'UV
      int y = ((bitmap_[i][j].r * 66 + bitmap_[i][j].g * 129 +
                bitmap_[i][j].b * 25) >>
               8) +
              16;
      int u = ((bitmap_[i][j].r * -38 + bitmap_[i][j].g * -74 +
                bitmap_[i][j].b * 112) >>
               8) +
              128;
      int v = ((bitmap_[i][j].r * 112 + bitmap_[i][j].g * -94 +
                bitmap_[i][j].b * -18) >>
               8) +
              128;

      // ModifyLuminance
      y = Clamp<double>(exp(log(y / 255.0) * ratio) * 255, 0, 255);

      // Y'UV -> RGB
      const int c = y - 16;
      const int d = u - 128;
      const int e = v - 128;
      bitmap_[i][j].r = Clamp((c * 298 + e * 409 + 128) >> 8, 0, 255);
      bitmap_[i][j].g = Clamp((c * 298 - d * 100 - e * 208 + 128) >> 8, 0, 255);
      bitmap_[i][j].b = Clamp((c * 298 + d * 516 + 128) >> 8, 0, 255);
    }
  }
}

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

void BMP::Erosion(std::vector<std::pair<int, int>> &structing_element) {
  bool *is_foreground =
      new bool[dib_header_.height_abs * dib_header_.width_abs];
  int x_lower_bound = std::numeric_limits<int>::max();
  int x_upper_bound = std::numeric_limits<int>::min();
  int y_lower_bound = std::numeric_limits<int>::max();
  int y_upper_bound = std::numeric_limits<int>::min();
  for (auto &it : structing_element) {
    x_lower_bound = std::min(x_lower_bound, it.first);
    x_upper_bound = std::max(x_upper_bound, it.first);
    y_lower_bound = std::min(y_lower_bound, it.second);
    y_upper_bound = std::max(y_upper_bound, it.second);
  }
  // erosion
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      const int index = i * dib_header_.width_abs + j;
      if (i + x_lower_bound < 0 ||
          i + x_upper_bound >= dib_header_.height_abs ||
          j + y_lower_bound < 0 || j + y_upper_bound >= dib_header_.width_abs) {
        bitmap_[i][j] = RGBColor{0, 0, 0};
        continue;
      }
      is_foreground[index] = true;
      for (auto &it : structing_element) {
        if (!bitmap_[i + it.first][j + it.second].r) {
          is_foreground[index] = false;
          break;
        }
      }
    }
  }
  // write back
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
}
