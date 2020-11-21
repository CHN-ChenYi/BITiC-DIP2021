#include <algorithm>
#include <cmath>

#include "BITiC.hpp"
using namespace BITiC;

void BMP::Translation(const int &delta_width, const int &delta_height) {
  Bitmap new_bitmap(dib_header_.width_abs + abs(delta_width),
                    dib_header_.height_abs + abs(delta_height));
  const int width_bias = delta_width < 0 ? 0 : delta_width;
  const int height_bias = delta_height < 0 ? 0 : delta_height;
  for (int i = 0; i < dib_header_.height_abs; i++) {
    for (int j = 0; j < dib_header_.width_abs; j++)
      new_bitmap[i + height_bias][j + width_bias] = bitmap_[i][j];
  }
  bitmap_ = new_bitmap;
  dib_header_.width_abs += abs(delta_width);
  dib_header_.height_abs += abs(delta_height);
}

void BMP::Mirror(const bool &horizontal, const bool &vertical) {
  bitmap_.Reverse(horizontal, vertical);
}
