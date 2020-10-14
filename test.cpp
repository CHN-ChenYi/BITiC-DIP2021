#include "bmp.h"

#include <iostream>
using namespace std;

int main() {
  BMP image;
  image.read("input.bmp");

  BMP image2(image);
  image2.GrayScale();
  image2.write("grayscale.bmp");

  int x;
  cout << "Please input the delta of luminance" << endl;
  cin >> x;
  BMP image3(image);
  image3.ModifyLuminanceLinear(x);
  image3.write("luminance_linear.bmp");

  double y;
  cout << "Please input the exponent of luminance" << endl;
  cin >> y;
  BMP image4(image);
  image4.ModifyLuminanceExponential(y);
  image4.write("luminance_exponential.bmp");
  return 0;
}
