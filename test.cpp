#include "bmp.h"

#include <iostream>
#include <fstream>
using namespace std;

int main() {
  BMP image;
  image.read("input.bmp");
  BMP image2(image);
  image2.write("copy.bmp");
  image2.GrayScale();
  image2.write("grayscale.bmp");
  image.ModifyLuminance(10, true);
  image.write("luminance.bmp");
  return 0;
}
