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

  std::vector<std::pair<int, int>> structing_element;
  for (int i = -1; i < 2; i++) {
    for (int j = -1; j < 2; j++)
      structing_element.push_back(std::make_pair(i, j));
  }
  image.Erosion(structing_element);
  image.write("erosion.bmp");

  return 0;
}
