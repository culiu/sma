#pragma once

#include <sma/util/serial.hpp>

#include <iosfwd>
#include <cstdint>
#include <string>

namespace sma
{
struct ContentType {
  TRIVIALLY_SERIALIZABLE(ContentType, value)

  ContentType(std::string value)
    : value(value)
  {
  }

  bool operator==(ContentType const& r) const { return value == r.value; }
  bool operator!=(ContentType const& r) const { return value != r.value; }
  bool operator<(ContentType const& r) const { return value < r.value; }
  bool operator>(ContentType const& r) const { return value > r.value; }
  bool operator<=(ContentType const& r) const { return value <= r.value; }
  bool operator>=(ContentType const& r) const { return value >= r.value; }

  explicit operator std::string() const { return value; }

private:
  friend struct std::hash<ContentType>;

  /****************************************************************************
   * Serialized Fields - Order matters!
   */
  std::string value;
  /***************************************************************************/
};

inline std::ostream& operator<<(std::ostream& os, ContentType const& value)
{
  return os << std::string(value);
}
}

namespace std
{
template <>
struct hash<sma::ContentType> {
  size_t operator()(sma::ContentType const& a) const
  {
    return hash<string>()(a.value);
  }
};
}
