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

  BMP image3(image), image4(image), image5(image);
  image3.Dilation(structing_element);
  image3.write("dilation.bmp");

  image.Erosion(structing_element);
  image.write("erosion.bmp");

  image4.Opening(structing_element);
  image4.write("opening.bmp");

  image5.Closing(structing_element);
  image5.write("closing.bmp");

  return 0;
}
