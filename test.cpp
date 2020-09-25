#include "bmp.h"

#include <iostream>
#include <fstream>
using namespace std;

int main() {
  BMP image;
  image.read("input.bmp");
  image.write("copy.bmp");
  return 0;
}
