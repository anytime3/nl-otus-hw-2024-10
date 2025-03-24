#include "Matrix.h"
#include <iostream>
#include <memory>

int main() {
  try
  {
    std::unique_ptr<MatrixProxy<int, 0>> matrix = std::make_unique<MatrixProxy<int, 0>>();

    for (int i = 0; i < 10; ++i) {
      (*matrix)[i][i] = i;
    }

    for (int i = 0; i < 10; ++i) {
      (*matrix)[i][9 - i] = 9 - i;
    }

    for (int i = 1; i <= 8; ++i) {
      for (int j = 1; j <= 8; ++j) {
        std::cout << (*matrix)[i][j] << " ";
      }
      std::cout << std::endl;
    }

    std::cout << "Занято ячеек: " << (*matrix).size() << std::endl;
    for(auto c: (*matrix))
    {
      int x;
      int y;
      int v;
      std::tie(x, y, v) = c;
      std::cout << "x: " << x << " y: " << y << " val: " << v << std::endl;
    }
  } catch(const std::exception & err)
  {
    std::cout << "Exception " << err.what() << std::endl;
  }
}
