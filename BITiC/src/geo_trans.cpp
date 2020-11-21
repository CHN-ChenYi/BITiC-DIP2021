#include <algorithm>
#include <cmath>

#include "BITiC.hpp"
using namespace BITiC;

void BMP::Translation(const int &delta_height, const int &delta_width) {
  Bitmap new_bitmap(dib_header_.height_abs + abs(delta_height),
                    dib_header_.width_abs + abs(delta_width));
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
      dib_header_.height_abs +
          ceil(horizontal_or_vertical * dib_header_.width_abs * fabs(d)),
      dib_header_.width_abs +
          ceil((!horizontal_or_vertical) * dib_header_.height_abs * fabs(d)));
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

void BMP::Scale(const double &ratio_height, const double &ratio_width) {
  const int new_width = dib_header_.width_abs * ratio_width,
            new_height = dib_header_.height_abs * ratio_height;
  Bitmap new_bitmap(new_height, new_width);
  for (int i = 0; i < new_height; i++) {
    for (int j = 0; j < new_width; j++)
      new_bitmap[i][j] =
          bitmap_.BilinearInterpolate(i / ratio_height, j / ratio_width);
  }
  bitmap_ = new_bitmap;
  dib_header_.width_abs = new_width;
  dib_header_.height_abs = new_height;
}

#define UpdateMinMax(x, y)                                  \
  x_min = std::min(x_min, x * cos(theta) - y * sin(theta)); \
  x_max = std::max(x_max, x * cos(theta) - y * sin(theta)); \
  y_min = std::min(y_min, x * sin(theta) + y * cos(theta)); \
  y_max = std::max(y_max, x * sin(theta) + y * cos(theta));

void BMP::Rotate(const double &theta) {
  double x_min = 0, x_max = 0;
  double y_min = 0, y_max = 0;
  UpdateMinMax(dib_header_.width_abs, 0);
  UpdateMinMax(0, dib_header_.height_abs);
  UpdateMinMax(dib_header_.width_abs, dib_header_.height_abs);
  const int new_width = int(x_max - x_min) + 1,
            new_height = int(y_max - y_min) + 1;
  Bitmap new_bitmap(new_height, new_width);
  for (int i = 0; i < new_height; i++) {
    for (int j = 0; j < new_width; j++) {
      const int x_ = j + x_min;
      const int y_ = i + y_min;
      new_bitmap[i][j] =
          bitmap_.BilinearInterpolate(x_ * sin(-theta) + y_ * cos(-theta),
                                      x_ * cos(-theta) - y_ * sin(-theta));
    }
  }
  bitmap_ = new_bitmap;
  dib_header_.width_abs = new_width;
  dib_header_.height_abs = new_height;
}
