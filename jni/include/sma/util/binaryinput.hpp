#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <string>
#include <cstdint>
#include <cstdlib>

namespace sma
{
class BinaryInput
{
  using const_pointer = std::uint8_t const*;
  using size_type = std::size_t;

  const_pointer src;
  const_pointer end;

public:
  BinaryInput(const_pointer src, size_type size);
  BinaryInput(void const* src, size_type size);

  BinaryInput(BinaryInput&& r) = default;
  BinaryInput(BinaryInput const& r) = default;

  BinaryInput& operator=(BinaryInput&& r) = default;
  BinaryInput& operator=(BinaryInput const& r) = default;

  size_type remaining();

  void* read(void* dst, std::size_t size);

  template <typename T>
  inline T get();

  template <typename T>
  BinaryInput operator>>(T& t)
  {
    t = get<T>();
    return *this;
  }

private:
  void require(size_type size);
};

// clang-format off
template <> bool          BinaryInput::get<bool>();
template <> std::string   BinaryInput::get<std::string>();
template <> float         BinaryInput::get<float>();
template <> double        BinaryInput::get<double>();
template <> std::uint8_t  BinaryInput::get<std::uint8_t>();
template <> std::uint16_t BinaryInput::get<std::uint16_t>();
template <> std::uint32_t BinaryInput::get<std::uint32_t>();
template <> std::uint64_t BinaryInput::get<std::uint64_t>();

template <> std::int8_t   BinaryInput::get<std::int8_t>();
template <> std::int16_t  BinaryInput::get<std::int16_t>();
template <> std::int32_t  BinaryInput::get<std::int32_t>();
template <> std::int64_t  BinaryInput::get<std::int64_t>();
// clang-format on
}
