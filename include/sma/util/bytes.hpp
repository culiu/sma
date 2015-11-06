#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>


namespace sma
{
// clang-format off
static std::uint8_t* uint8_p(char* s) { return reinterpret_cast<std::uint8_t*>(s); }
static std::uint8_t* uint8_p(unsigned char* s) { return reinterpret_cast<std::uint8_t*>(s); }

static const std::uint8_t* uint8_cp(std::string s) { return reinterpret_cast<const std::uint8_t*>(s.c_str()); }
static const std::uint8_t* uint8_cp(const char* s) { return reinterpret_cast<const std::uint8_t*>(s); }
static const std::uint8_t* uint8_cp(const unsigned char* s) { return reinterpret_cast<const std::uint8_t*>(s); }

static char* char_p(std::uint8_t* b) { return reinterpret_cast<char*>(b); }
static unsigned char* uchar_p(std::uint8_t* b) { return reinterpret_cast<unsigned char*>(b); }

static const char* char_cp(const std::uint8_t* b) { return reinterpret_cast<const char*>(b); }
static const unsigned char* uchar_cp(const std::uint8_t* b) { return reinterpret_cast<const unsigned char*>(b); }

static std::string copy_string(const std::uint8_t* s) { return std::string(char_cp(s)); }
static std::string copy_string(const std::uint8_t* s, std::size_t len) { return std::string(char_cp(s), char_cp(s)+len); }

template<typename T>
struct streamarr_S_ {
  const T* tarr;
  std::size_t len;
};

template<typename T>
std::ostream& operator<<(std::ostream& os, const streamarr_S_<T>& s)
{
  const char* carr = reinterpret_cast<const char*>(s.tarr);
  os.write(carr, s.len * sizeof(T));
  return os;
}

template<typename T>
static streamarr_S_<T> streamarr(const T* tarr, std::size_t len)
{
  return streamarr_S_<T>{tarr, len};
}

class Bytes final
{
  using size_t = std::size_t;
  using uint8_t = std::uint8_t;

public:
  static size_t put(uint8_t* dst, const uint8_t* src, size_t len)
  {
    std::memcpy(dst, src, len);
    return len;
  }
  static size_t put(uint8_t* dst, const uint8_t b)
  {
    *dst = b;
    return 1;
  }
  static size_t put(uint8_t* dst, const uint16_t s)
  {
    *dst++ = s >> 8;
    *dst = s & 0xFF;
    return 2;
  }
  static size_t put(uint8_t* dst, const uint32_t i)
  {
    *dst++ = i >> 24;
    *dst++ = (i >> 16) & 0xFF;
    *dst++ = (i >> 8) & 0xFF;
    *dst = i & 0xFF;
    return 4;
  }
  static size_t get(const uint8_t* src, uint8_t& b)
  {
    b = *src;
    return 1;
  }
  static size_t get(const uint8_t* src, uint16_t& s)
  {
    s = uint16_t{*src++} << 8;
    s |= *src;
    return 2;
  }
  static size_t get(const uint8_t* src, uint32_t& i)
  {
    i = uint32_t{*src++} << 24;
    i |= uint32_t{*src++} << 16;
    i |= uint32_t{*src++} << 8;
    i |= *src;
    return 4;
  }
};
// clang-format on
}
