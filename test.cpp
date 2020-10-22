#include "bmp.h"

#include <iostream>
using namespace std;

int main() {
  BMP image;
  image.read("input.bmp");

  BMP image2(image);
  image2.GrayScale();
  image2.write("grayscale.bmp");

  image.Binarization();
  image.write("bin.bmp");
  return 0;
}
