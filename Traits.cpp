#include <type_traits>
#include <vector>
#include <list>
#include <tuple>

template<typename T, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T&>())),
                                  decltype(std::end(std::declval<T&>()))>>
      : std::true_type {};

template<typename T>
constexpr bool is_iterable_v = is_iterable<T>::value;

//------------------------------------------------------------------------
template <typename T, typename = void>
struct is_tuple : std::false_type {};

template <typename ...T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename T>
constexpr bool is_tuple_v = is_tuple<T>::value;


