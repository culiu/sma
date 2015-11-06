#pragma once

#include <sma/util/serial.hpp>

#include <string>
#include <iosfwd>
#include <cstdint>
#include <type_traits>

namespace sma
{
struct NodeId {
  TRIVIALLY_SERIALIZABLE(NodeId, value)

  using value_type = std::uint16_t;

  template <typename T,
            typename std::enable_if<std::is_integral<T>::value
                                    && sizeof(T)
                                       >= sizeof(value_type)>::type* = nullptr>
  NodeId(T value)
    : value{value}
  {
  }

  NodeId(NodeId const&) = default;
  NodeId& operator=(NodeId const&) = default;

  bool operator==(NodeId const& r) const { return value == r.value; }
  bool operator!=(NodeId const& r) const { return value != r.value; }

  explicit operator std::uint64_t() const { return value; }
  explicit operator std::uint32_t() const { return value; }
  explicit operator std::uint16_t() const { return value; }
  explicit operator std::string() const { return std::to_string(value); }

private:
  friend struct std::hash<NodeId>;

  value_type value;
};

inline std::ostream& operator<<(std::ostream& os, NodeId const& value)
{
  return os << std::string(value);
}
}

namespace std
{
template <>
struct hash<sma::NodeId> {
  using argument_type = sma::NodeId;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const { return a.value; }
};
}
