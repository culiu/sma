#pragma once

#include <sma/util/serial.hpp>

#include <string>
#include <iosfwd>

namespace sma
{
struct ContentName {
  TRIVIALLY_SERIALIZABLE(ContentName, value)

  ContentName(std::string value)
    : value(value)
  {
  }

  std::size_t size() { return value.size(); }

  bool operator==(ContentName const& r) const { return value == r.value; }
  bool operator!=(ContentName const& r) const { return value != r.value; }
  bool operator<(ContentName const& r) const { return value < r.value; }
  bool operator>(ContentName const& r) const { return value > r.value; }
  bool operator<=(ContentName const& r) const { return value <= r.value; }
  bool operator>=(ContentName const& r) const { return value >= r.value; }

  explicit operator std::string() const { return value; }

private:
  friend struct std::hash<ContentName>;

  /****************************************************************************
   * Serialized Fields - Order matters!
   */
  std::string value;
  /***************************************************************************/
};

inline std::ostream& operator<<(std::ostream& os, ContentName const& name)
{
  return os << std::string(name);
}
}

namespace std
{
template <>
struct hash<sma::ContentName> {
  using argument_type = sma::ContentName;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const
  {
    return hash<std::string>()(a.value);
  }
};
}
