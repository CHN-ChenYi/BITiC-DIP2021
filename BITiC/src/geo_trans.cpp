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
  dib_header_.width_abs = new_bitmap.width();
  dib_header_.height_abs = new_bitmap.height();
}

void BMP::Mirror(const bool &horizontal, const bool &vertical) {
  bitmap_.Reverse(horizontal, vertical);
}

void BMP::Shear(const bool &horizontal_or_vertical, const double &d) {
  Bitmap new_bitmap(
      dib_header_.width_abs +
          ceil((!horizontal_or_vertical) * dib_header_.height_abs * fabs(d)),
      dib_header_.height_abs +
          ceil(horizontal_or_vertical * dib_header_.width_abs * fabs(d)));
  if (!horizontal_or_vertical) {  // horizontal
    const double bias = d >= 0 ? 0 : dib_header_.height_abs * fabs(d);
    for (int i = 0; i < dib_header_.height_abs; i++) {
      for (int j = 0; j < dib_header_.width_abs; j++)
        new_bitmap[i][j + ceil(i * d + bias)] = bitmap_[i][j];
    }
  } else {  // vertical
    const double bias = d >= 0 ? 0 : dib_header_.width_abs * fabs(d);
    for (int i = 0; i < dib_header_.height_abs; i++) {
      for (int j = 0; j < dib_header_.width_abs; j++)
        new_bitmap[i + ceil(j * d + bias)][j] = bitmap_[i][j];
    }
  }
  bitmap_ = new_bitmap;
  dib_header_.width_abs = new_bitmap.width();
  dib_header_.height_abs = new_bitmap.height();
}
