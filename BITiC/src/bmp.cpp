#include <cstring>

#include "BITiC.hpp"
using namespace BITiC;

BMP::BMP() {
  memset(&bmp_header_, 0, sizeof(bmp_header_));
  memset(&dib_header_, 0, sizeof(dib_header_));
  // palette_.resize(0);
}

BMP::BMP(const char filename[]) { read(filename); }

BMP::~BMP() {}

int32_t BMP::width() { return dib_header_.width_abs; }
int32_t BMP::height() { return dib_header_.height_abs; }

void BMP::SetWidth(const int32_t width) {
  dib_header_.width_abs = width;
  bitmap_.Resize(width, dib_header_.height_abs);
}

void BMP::SetHeight(const int32_t height) {
  dib_header_.height_abs = height;
  bitmap_.Resize(dib_header_.width_abs, height);
}

void BMP::Resize(const int32_t width, const int32_t height) {
  dib_header_.width_abs = width;
  dib_header_.height_abs = height;
  bitmap_.Resize(width, height);
}
