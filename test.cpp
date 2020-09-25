#include "bmp.h"

#include <iostream>
#include <fstream>
using namespace std;

int main() {
  BMP image;
  image.read("input.bmp");
  image.write("copy.bmp");
  BMP image2;
  image2.SetWidth(2);
  image2.SetHeight(2);
  image2[0][0] = RGBColor{0xff, 0, 0};
  image2[0][1] = RGBColor{0xff, 0xff, 0xff};
  image2[1][0] = RGBColor{0, 0, 0xff};
  image2[1][1] = RGBColor{0, 0xff, 0};
  image2.write("image2.bmp");
  return 0;
}
