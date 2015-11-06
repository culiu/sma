#pragma once

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentmetadata.hpp>
#include <sma/util/serial.hpp>
#include <sma/chrono.hpp>

#include <chrono>
#include <iosfwd>
#include <cstdlib>
#include <cstdint>
#include <string>


namespace sma
{
struct Interest {
private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

public:
  TRIVIALLY_SERIALIZABLE(Interest, type, ttl_ms, hops)

  template <typename D>
  Interest(ContentType type, D ttl, std::size_t hops = 0)
    : type(type)
    , hops(hops)
    , expiry_time(clock::now() + ttl)
  {
    this->ttl(ttl);
  }

  /****************************************************************************
   * Serialized Fields - Order matters!
   */
  ContentType type;
  std::uint32_t ttl_ms;
  std::uint8_t hops;
  /***************************************************************************/

  template <typename D>
  D ttl() const
  {
    return std::chrono::duration_cast<D>(std::chrono::milliseconds(ttl_ms));
  }

  template <typename D>
  void ttl(D ttl)
  {
    ttl_ms = std::chrono::duration_cast<std::chrono::milliseconds>(ttl).count();
  }

  bool local() const { return hops == 0; }
  bool remote() const { return hops != 0; }

  bool closer_than(Interest const& i) const { return hops < i.hops; }

  bool newer_than(Interest const& i) const { return ttl_ms > i.ttl_ms; }

  bool update_with(Interest const& i)
  {
    bool updated = false;
    if (i.closer_than(*this)) {
      hops = i.hops;
      updated = true;
    }
    if (i.newer_than(*this)) {
      ttl(i.ttl<std::chrono::milliseconds>());
      updated = true;
    }
    return updated;
  }

  void elapse_ttl()
  {
    // A node always refreshes the TTL of its own interests.
    if (local()) {
      expiry_time = clock::now() + std::chrono::milliseconds(ttl_ms);
      return;
    }

    auto now = clock::now();
    if (now >= expiry_time)
      ttl_ms = 0;
    else
      ttl(expiry_time - now);
  }

  bool expired() const { return ttl_ms == 0; }

  bool operator==(Interest const& rhs) const { return type == rhs.type; }
  bool operator!=(Interest const& rhs) const { return !(*this == rhs); }
  bool operator<(Interest const& rhs) const { return type < rhs.type; }
  bool operator>(Interest const& rhs) const { return type > rhs.type; }
  bool operator<=(Interest const& rhs) const { return type <= rhs.type; }
  bool operator>=(Interest const& rhs) const { return type >= rhs.type; }

  explicit operator std::string() const { return std::string(type); }

private:
  friend struct std::hash<Interest>;

  /****************************************************************************
   * Transient - only used locally for bookkeeping
   */
  time_point expiry_time;
};

inline std::ostream& operator<<(std::ostream& os, Interest const& interest)
{
  return os << std::string(interest);
}
}

namespace std
{
template <>
struct hash<sma::Interest> {
  using argument_type = sma::Interest;
  using result_type = std::size_t;

  result_type operator()(argument_type const& a) const
  {
    return hash<sma::ContentType>()(a.type);
  }
};
}
