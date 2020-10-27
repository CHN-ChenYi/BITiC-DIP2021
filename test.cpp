#include <iostream>

#include "bmp.h"
using namespace std;

int main() {
  BMP image;
  image.read("input.bmp");

  BMP image2(image);
  image2.Binarization();
  image2.write("bin.bmp");

  image.Binarization(image.width() / 2.5, image.width() / 4);
  image.write("local_bin.bmp");

  return 0;
}
