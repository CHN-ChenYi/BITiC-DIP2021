#include <queue>

#include "BITiC.hpp"
#include "lib.hpp"
using namespace BITiC;

void BMP::GrayScale() {
  std::queue<double> y;
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++)
      y.push(bitmap_.YUV()[i][j].y);
  }
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      const uint8_t y_val = y.front();
      y.pop();
      bitmap_[i][j] = RGBColor{y_val, y_val, y_val};
    }
  }
}

#include <cassert>
void BMP::ModifyLuminance(std::function<double(const double &)> trans_func) {
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++) {
      bitmap_.YUV()[i][j].y =
          Clamp<double>(trans_func(bitmap_.YUV()[i][j].y), 0, 255);
      assert(0 <= bitmap_.YUV()[i][j].y && bitmap_.YUV()[i][j].y <= 255);
    }
  }
}
