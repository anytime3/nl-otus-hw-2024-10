#include <map>
#include <vector>
#include "Allocators.cpp"
#include "Containers.cpp"

int factorial(const int n)
{
  int result = 1;
  for (int i = 1; i <= n; i++)
    result *= i;
  return result;
}

int main()
{
  try
  {
    // standardMap with StandardAllocator
    std::map<int, int> standardMap;
    for (int i = 0; i < 10; i++)
      standardMap.insert(std::pair<int, int> (i, factorial(i)));

    // standardMap with FixedAllocator
    using fixedAllocatorAlias = FixedAllocator<std::pair<const int, int>, 10>;
    std::map<const int, int, std::less<>, fixedAllocatorAlias> fixedStandardMap;
    for(int i = 0; i < 10; i++)
      fixedStandardMap.emplace(std::pair<int, int> (i, factorial(i)));

    std::cout << "Map with FixedAllocator" << std::endl;
    for(const auto & pair : fixedStandardMap)
      std::cout << pair.first << " -> " << pair.second << std::endl;

    // customContainer with StandardAllocator
    CustomContainer<int> customContainer;
    for(int i = 0; i < 10; i++)
      customContainer.push_back(i);

    std::cout << "CustomContainer with standart allocator" << std::endl;
    for (const auto & elem : customContainer)
      std::cout << elem << std::endl;

    // customContainer with FixedAllocator
    CustomContainer<int, FixedAllocator<int, 10>> fixedCustomContainer;
    for(int i = 0; i < 10; i++)
      fixedCustomContainer.push_back(i);

    std::cout << "CustomContainer with CustomAllocator" << std::endl;
    for (const auto & elem : fixedCustomContainer)
      std::cout << elem << std::endl;

  } catch (const std::exception & err)
  {
    std::cout << "Exception: " << err.what() << std::endl;
  }
}
