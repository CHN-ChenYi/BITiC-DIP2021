#include "bmp.h"

#include <iostream>
#include <fstream>
using namespace std;

int main() {
  BMP image;
  image.read("233.bmp");
  BMP image2(image);
  image2.write("copy.bmp");
  image2.GrayScale();
  image2.write("grayscale.bmp");
  return 0;
}
