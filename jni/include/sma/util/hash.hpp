#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/util/serial.hpp>

#include <sma/util/detail/sha256.hpp>

#include <string>
#include <cstdint>
#include <cassert>
#include <functional>


namespace sma
{
struct Hash {
  friend struct Hasher;
  friend struct std::hash<Hash>;

  static constexpr std::size_t LENGTH = 32;

  Hash(Hash const&);
  Hash& operator=(Hash const&);

  DESERIALIZING_CTOR(Hash) { GET_BYTES(data, LENGTH); }

  SERIALIZER() { PUT_BYTES(data, LENGTH); }

  int compare(Hash const&) const;

  bool operator==(Hash const& r) const { return compare(r) == 0; }
  bool operator!=(Hash const& r) const { return compare(r) != 0; }
  bool operator<(Hash const& r) const { return compare(r) < 0; }
  bool operator>(Hash const& r) const { return compare(r) > 0; }
  bool operator<=(Hash const& r) const { return compare(r) <= 0; }
  bool operator>=(Hash const& r) const { return compare(r) >= 0; }

  explicit operator std::string() const;

private:
  Hash() {}

  std::uint8_t data[LENGTH];
};

struct Hasher {
  Hasher() {}
  Hasher(void const* src, std::size_t size);
  Hasher(std::string const& s);

  Hasher& operator()(void const* src, std::size_t size);
  Hasher& operator()(std::string const& s);

  Hash digest();

private:
  SHA256 sha;
};

inline std::ostream& operator<<(std::ostream& os, Hash const& h)
{
  return os << std::string(h);
}
}

namespace std
{
template <>
struct hash<sma::Hash> {
  size_t operator()(sma::Hash const& a) const
  {
    size_t hash = 1;
    for (size_t i = 0; i < sma::Hash::LENGTH; i += 4)
      hash = 31 * hash + (size_t(a.data[i]) << 24 | size_t(a.data[i + 1]) << 16
                          | size_t(a.data[i + 2]) << 8
                          | a.data[i + 3]);
    return hash;
  }
};
}
