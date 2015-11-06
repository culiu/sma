#pragma once

#include <sma/util/hash.hpp>
#include <sma/util/serial.hpp>

#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <functional>


namespace sma
{
using BlockIndex = std::uint16_t;

struct BlockRef {
  TRIVIALLY_SERIALIZABLE(BlockRef, hash, index);

  BlockRef(Hash hash, BlockIndex index)
    : hash(hash)
    , index(index)
  {
  }

  bool operator==(BlockRef const& rhs) const
  {
    return hash == rhs.hash && index == rhs.index;
  }
  bool operator!=(BlockRef const& rhs) const { return !(*this == rhs); }

  Hash hash;
  BlockIndex index;
};

inline std::ostream& operator<<(std::ostream& os, BlockRef const& ref)
{
  return os << "(" << std::string(ref.hash) << ")[" << ref.index << "]";
}
}

namespace std
{
template <>
struct hash<sma::BlockRef> {
  size_t operator()(sma::BlockRef const& block) const
  {
    return 37 * hash<sma::Hash>()(block.hash) + block.index;
  }
};

template <>
struct less<sma::BlockRef> {
  using arg_type = sma::BlockRef;
  bool operator()(arg_type const& lhs, arg_type const& rhs) const
  {
    return (lhs.hash < rhs.hash) && (lhs.index < rhs.index);
  }
};
}
