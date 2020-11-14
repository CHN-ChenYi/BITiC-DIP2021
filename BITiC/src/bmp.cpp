#include <cstring>

#include "BITiC.hpp"
using namespace BITiC;

BMP::BMP() {
  memset(&bmp_header_, 0, sizeof(bmp_header_));
  memset(&dib_header_, 0, sizeof(dib_header_));
  palette_.resize(0);
  bitmap_.resize(0);
}

BMP::BMP(const char filename[]) { read(filename); }

BMP::~BMP() {}

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
