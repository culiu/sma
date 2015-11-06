#include <sma/util/binaryoutput.hpp>

#include <sma/util/detail/uint_with_size.hpp>

//#include <sma/io/log.hpp>

#include <cassert>
#include <cstring>

namespace sma
{
BinaryOutput::BinaryOutput(pointer dst, size_type size)
  : dst(dst)
  , cur(dst)
  , end(dst + size)
{
}

BinaryOutput::BinaryOutput(void* dst, size_type size)
  : BinaryOutput(reinterpret_cast<pointer>(dst), size)
{
}

BinaryOutput::size_type BinaryOutput::remaining() { return end - cur; }

BinaryOutput::size_type BinaryOutput::size() { return cur - dst; }

void BinaryOutput::require(size_type size) { assert(remaining() >= size); }

BinaryOutput& BinaryOutput::write(void const* src, std::size_t size)
{
  require(size);
  std::memcpy(cur, src, size);
  cur += size;
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(bool const& t)
{
  assert(cur != end);
  *cur++ = (t ? 1 : 0);
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(std::string const& t)
{
  static_assert(
      sizeof(std::uint8_t) == sizeof(std::string::value_type),
      "std::string isn't std::basic_stream<char> or char isn't 8 bits.");

  auto size = t.size();
  assert(size <= 32767);

  if (size >= 127)
    // 2^15 - 1, or two bytes with the highest bit as the extension flag
    *this << std::uint16_t(size | (1 << 15));
  else
    // 2^7 - 1, or one byte with the highest bit as the extension flag
    *this << std::uint8_t(size);

  write(t.c_str(), size);
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(float const& t)
{
  typename detail::uint_with_size<sizeof(float)>::type i = 0;
  std::memcpy(&i, &t, sizeof i);
  *this << i;
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(double const& t)
{
  typename detail::uint_with_size<sizeof(double)>::type i = 0;
  std::memcpy(&i, &t, sizeof i);
  *this << i;
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(std::uint8_t const& t)
{
  assert(cur != end);
  *cur++ = t;
  return *this;
}

BinaryOutput& BinaryOutput::operator<<(std::uint16_t const& t)
{
  char buf[]{char(t >> 8 & 0xff), char(t & 0xff)};
  return write(buf, 2);
}

BinaryOutput& BinaryOutput::operator<<(std::uint32_t const& t)
{
  char buf[]{char(t >> 24 & 0xff),
             char(t >> 16 & 0xff),
             char(t >> 8 & 0xff),
             char(t & 0xff)};
  return write(buf, 4);
}

BinaryOutput& BinaryOutput::operator<<(std::uint64_t const& t)
{
  char buf[]{char(t >> 56 & 0xff),
             char(t >> 48 & 0xff),
             char(t >> 40 & 0xff),
             char(t >> 32 & 0xff),
             char(t >> 24 & 0xff),
             char(t >> 16 & 0xff),
             char(t >> 8 & 0xff),
             char(t & 0xff)};
  return write(buf, 8);
}

BinaryOutput& BinaryOutput::operator<<(std::int8_t const& t)
{
  return operator<<(std::uint8_t(t));
}

BinaryOutput& BinaryOutput::operator<<(std::int16_t const& t)
{
  return operator<<(std::uint16_t(t));
}

BinaryOutput& BinaryOutput::operator<<(std::int32_t const& t)
{
  return operator<<(std::uint32_t(t));
}

BinaryOutput& BinaryOutput::operator<<(std::int64_t const& t)
{
  return operator<<(std::uint64_t(t));
}
}
