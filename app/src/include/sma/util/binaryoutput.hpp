#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <limits>


namespace sma
{
class BinaryOutput
{
  using pointer = std::uint8_t*;
  using size_type = std::size_t;

  pointer dst;
  pointer cur;
  pointer end;

public:
  BinaryOutput(pointer dst, size_type size);
  BinaryOutput(void* dst, size_type size);

  BinaryOutput(BinaryOutput&& r) = default;
  BinaryOutput(BinaryOutput const& r) = default;

  BinaryOutput& operator=(BinaryOutput&& r) = default;
  BinaryOutput& operator=(BinaryOutput const& r) = default;

  size_type remaining();
  size_type size();

  BinaryOutput& write(void const* src, std::size_t size);

  template <typename T>
  BinaryOutput& operator<<(T const& t)
  {
    t.write_fields(*this);
    return *this;
  }

  template <typename T>
  BinaryOutput& operator<<(std::vector<T> const& v)
  {
    assert(v.size() <= std::numeric_limits<std::uint8_t>::max());
    operator<<(std::uint8_t(v.size()));
    for (auto const& t : v)
      operator<<(t);
    return *this;
  }

  // clang-format off
  BinaryOutput& operator<< (bool const& t);
  BinaryOutput& operator<< (std::string const& t);
  BinaryOutput& operator<< (float const& t);
  BinaryOutput& operator<< (double const& t);
  BinaryOutput& operator<< (std::uint8_t const& t);
  BinaryOutput& operator<< (std::uint16_t const& t);
  BinaryOutput& operator<< (std::uint32_t const& t);
  BinaryOutput& operator<< (std::uint64_t const& t);

  BinaryOutput& operator<< (std::int8_t const& t);
  BinaryOutput& operator<< (std::int16_t const& t);
  BinaryOutput& operator<< (std::int32_t const& t);
  BinaryOutput& operator<< (std::int64_t const& t);
  // clang-format on

private:
  void require(size_type size);
};
}
