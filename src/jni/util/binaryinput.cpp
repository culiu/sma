#include <sma/util/binaryinput.hpp>

#include <sma/util/detail/uint_with_size.hpp>

//#include <sma/io/log.hpp>

#include <cassert>
#include <cstring>

namespace sma
{
BinaryInput::BinaryInput(const_pointer src, size_type size)
  : src(src)
  , end(src + size)
{
}

BinaryInput::BinaryInput(void const* src, size_type size)
  : BinaryInput(reinterpret_cast<const_pointer>(src), size)
{
}

BinaryInput::size_type BinaryInput::remaining() { return end - src; }
void BinaryInput::require(size_type size) { assert(remaining() >= size); }

void* BinaryInput::read(void* dst, std::size_t size)
{
  require(size);
  std::memcpy(dst, src, size);
  src += size;
  return dst;
}

template <>
bool BinaryInput::get<bool>()
{
  require(1);
  return *src++ != 1;
}

template <>
std::string BinaryInput::get<std::string>()
{
  static_assert(
      sizeof(std::uint8_t) == sizeof(std::string::value_type),
      "std::string isn't std::basic_stream<char> or char isn't 8 bits.");

  static_assert(
      sizeof(std::string::size_type) >= sizeof(std::uint16_t),
      "std::string::size_type isn't as wide as the serialized string size.");

  // Most strings will fit without an allocation
  std::string::value_type buf[1024];
  std::string::pointer cstr = buf;

  // If the high bit of the first byte is set then it's a two byte size;
  // This gives us a range of 127 or 32,767 bytes
  std::uint16_t size = get<std::uint8_t>();
  if (size & 0x80)
    // Remove the flag bit and fetch the low byte
    size = (size & ~0x80) << 8 | get<std::uint8_t>();

  if (size > 1024)
    cstr = new std::string::value_type[size];

  read(cstr, size);
  auto s = std::string(cstr, size);

  if (cstr != buf)
    delete cstr;

  return s;
}

template <>
float BinaryInput::get<float>()
{
  typedef typename detail::uint_with_size<sizeof(float)>::type I;
  auto i = get<I>();
  float f;
  std::memcpy(&f, &i, sizeof(float));
  return f;
}

template <>
double BinaryInput::get<double>()
{
  typedef typename detail::uint_with_size<sizeof(double)>::type I;
  auto i = get<I>();
  double d;
  std::memcpy(&d, &i, sizeof(double));
  return d;
}

template <>
std::uint8_t BinaryInput::get<std::uint8_t>()
{
  assert(src != end);
  return *src++;
}

template <>
std::uint16_t BinaryInput::get<std::uint16_t>()
{
  std::uint8_t buf[2];
  read(buf, 2);
  return std::uint16_t{buf[0]} << 8 | std::uint16_t{buf[1]};
}

template <>
std::uint32_t BinaryInput::get<std::uint32_t>()
{
  std::uint8_t buf[4];
  read(buf, 4);
  return std::uint32_t{buf[0]} << 24 | std::uint32_t{buf[1]} << 16
         | std::uint32_t{buf[2]} << 8 | std::uint32_t{buf[3]};
}

template <>
std::uint64_t BinaryInput::get<std::uint64_t>()
{
  std::uint8_t buf[8];
  read(buf, 8);
  return std::uint64_t{buf[0]} << 56 | std::uint64_t{buf[1]} << 48
         | std::uint64_t{buf[2]} << 40 | std::uint64_t{buf[3]} << 32
         | std::uint64_t{buf[4]} << 24 | std::uint64_t{buf[5]} << 16
         | std::uint64_t{buf[6]} << 8 | std::uint64_t{buf[7]};
}

template <>
std::int8_t BinaryInput::get<std::int8_t>()
{
  return std::int8_t(get<std::uint8_t>());
}

template <>
std::int16_t BinaryInput::get<std::int16_t>()
{
  return std::int16_t(get<std::uint16_t>());
}

template <>
std::int32_t BinaryInput::get<std::int32_t>()
{
  return std::int32_t(get<std::uint32_t>());
}

template <>
std::int64_t BinaryInput::get<std::int64_t>()
{
  return std::int64_t(get<std::uint64_t>());
}
}
