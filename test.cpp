#include "bmp.h"

#include <iostream>
#include <fstream>
using namespace std;

int main() {
  BMP image;
  image.read("red.bmp");
  return 0;
}
