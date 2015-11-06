#pragma once

#include <cstdint>

namespace sma
{
namespace detail
{
  // clang-format off
  /****************************************************************************
   * Find the integral type with the given size in chars.
   * Used to find a suitable uint for punning floating point types.
   */
  template <std::size_t Size> struct uint_with_size;
  template <> struct uint_with_size<sizeof(std::uint8_t)>  { typedef std::uint8_t type; };
  template <> struct uint_with_size<sizeof(std::uint16_t)> { typedef std::uint16_t type; };
  template <> struct uint_with_size<sizeof(std::uint32_t)> { typedef std::uint32_t type; };
  template <> struct uint_with_size<sizeof(std::uint64_t)> { typedef std::uint64_t type; };
  // clang-format on
}
}
