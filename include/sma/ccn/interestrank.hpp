#pragma once

#include <string>
#include <cstdint>
#include <iosfwd>

namespace sma
{
struct InterestRank {
  friend struct std::hash<InterestRank>;

private:
  using value_type = std::uint32_t;

public:
  value_type value;

  InterestRank()
    : value(0)
  {
  }

  bool operator==(InterestRank const& r) const { return value == r.value; }
  bool operator!=(InterestRank const& r) const { return value != r.value; }
  bool operator<=(InterestRank const& r) const { return value <= r.value; }
  bool operator>=(InterestRank const& r) const { return value >= r.value; }
  bool operator<(InterestRank const& r) const { return value < r.value; }
  bool operator>(InterestRank const& r) const { return value > r.value; }

  explicit operator value_type() const { return value; }
  explicit operator std::string() const { return std::to_string(value); }
};

std::ostream& operator<<(std::ostream& os, InterestRank const& value);
}

namespace std
{
template <>
struct hash<sma::InterestRank> {
  using argument_type = sma::InterestRank;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const { return a.value; }
};
}
