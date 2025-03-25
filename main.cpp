#include <iostream>
#include <string>
#include "CommandProcessor.h"


int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <block_size>" << std::endl;
    return 1;
  }

  size_t block_size;
  try {
    block_size = std::stoul(argv[1]);
  } catch (const std::exception & e)
  {
    std::cerr << "Invalid block size: " << argv[1] << std::endl;
    return 1;
  }

  CommandProcessor processor(block_size);
  std::string line;

  while (std::getline(std::cin, line))
    processor.process_command(line);

}
