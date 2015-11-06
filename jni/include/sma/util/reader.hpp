#pragma once

#include <cstdint>
#include <utility>
#include <vector>

namespace sma
{
namespace detail
{
  template <typename T, typename = void>
  struct is_vector {
    static constexpr bool value = false;
  };

  // clang-format off
  template <typename T>
  struct is_vector<
    T,
    typename std::enable_if<
      std::is_same<
        T,
        std::vector<typename T::value_type, typename T::allocator_type>
      >::value>
    ::type>
  {
    static constexpr bool value = true;
  };
  // clang-format on
}

template <typename Formatter>
class Reader
{
  using Myt = Reader<Formatter>;

  Formatter fmat;

public:
  template <typename... Args>
  Reader(Args&&... args)
    : fmat(std::forward<Args>(args)...)
  {
  }

  void read(void* dst, std::size_t size) { fmat.read(dst, size); }

  template <typename T>
  std::size_t fill(std::vector<T>& v);

  template <typename T>
  typename std::enable_if<std::is_constructible<T, Myt&>::value, T>::type get()
  {
    return T(*this);
  }

  template <typename T>
  typename std::enable_if<not std::is_constructible<T, Myt&>::value
                          && not detail::is_vector<T>::value,
                          T>::type
  get()
  {
    return fmat.template get<T>();
  }

  template <typename T>
  typename std::enable_if<detail::is_vector<T>::value, T>::type get()
  {
    T v;
    fill(v);
    return v;
  }

  template <typename T>
  Myt& operator>>(T& t)
  {
    t = get<T>();
    return *this;
  }

  template <typename T>
  Myt& operator>>(std::vector<T>& v)
  {
    fill(v);
    return *this;
  }
};

template <typename Formatter>
template <typename T>
std::size_t Reader<Formatter>::fill(std::vector<T>& v)
{
  std::size_t size = get<std::uint8_t>();
  if (size == 0)
    return 0;

  v.reserve(size);
  for (std::size_t i = 0; i < size; ++i)
    v.emplace_back(*this);
  return size;
}
}
