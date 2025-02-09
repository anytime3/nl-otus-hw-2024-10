#include <iostream>
#include "Traits.cpp"


template<typename T, typename Fake = std::enable_if_t<std::is_integral_v<T>>>
void print_ip(T value, [[maybe_unused]] std::integral_constant<int, 0> fake = {})
{
  unsigned char * p = (unsigned char*) &value;
  std::cout << static_cast<int>(p[sizeof(T) - 1]);
  for(size_t i = 2 ; i < sizeof(T); i++)
    std::cout << "." << static_cast<int>(p[sizeof(T) - i]);
  std::cout << std::endl;
}


template<typename T, typename Fake = std::enable_if_t<std::is_same_v<T, std::string>>>
void print_ip(const T & value, [[maybe_unused]] std::integral_constant<int, 1> fake = {})
{
  std::cout << value << std::endl;
}

template<typename T, typename Fake = std::enable_if_t<is_iterable_v<T> && std::is_same_v<T, std::string> == false>>
void print_ip(const T & value, [[maybe_unused]] std::integral_constant<int, 2> fake = {})
{
  auto i = value.begin();
  if (i != value.end())
    std::cout << *i;
  i++;

  for(; i != value.end(); i++)
    std::cout << "." << *i;
  std::cout << std::endl;
}


template<size_t index, size_t size, typename T>
void tuple_print(const T& t)
{
  std::cout << std::get<index>(t);
  if constexpr (index < size - 1)
    {
      std::cout << ".";
      tuple_print<index + 1, size>(t);
    }
}

template <typename ...Args, typename Fake = std::enable_if_t< is_tuple_v<std::tuple<Args...>>>>
void print_ip(const std::tuple<Args...> & value,[[maybe_unused]] std::integral_constant<int, 3> fake = {})
{
  constexpr size_t size = std::tuple_size<std::tuple<Args...>>();
  tuple_print<0, size, std::tuple<Args...>>(value);
  std::cout << std::endl;
}
