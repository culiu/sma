#pragma once

#include <sma/nodeid.hpp>
#include <sma/util/hash.hpp>
#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>
#include <sma/util/vec2d.hpp>

#include <sma/util/serial.hpp>

#include <sma/chrono.hpp>

#include <chrono>
#include <cstdint>
#include <vector>


namespace sma
{
struct ContentMetadata {
private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;
  using Ms = std::chrono::milliseconds;

public:
  Hash hash;
  std::uint32_t size;
  std::uint32_t block_size;

  std::vector<ContentType> types;
  ContentName name;

  Vec2d origin;
  NodeId publisher;

  std::uint64_t publish_time_ms;

  std::uint8_t hops;
  // Pseudo-field; derived from expiry_time relative to current time
  // std::uint32_t ttl_ms;

  // nodeid from whom the msg was received
  // but will not be serialized
  NodeId from{255};

  // Transient
  time_point expiry_time;


  template <typename D>
  ContentMetadata(Hash hash,
                  std::uint32_t size,
                  std::uint32_t block_size,
                  std::vector<ContentType> types,
                  ContentName name,
                  Vec2d origin,
                  NodeId publisher,
                  std::uint64_t publish_time_ms,
                  D ttl,
                  std::uint8_t hops = 0)
    : hash(hash)
    , size(size)
    , block_size(block_size)
    , types(types)
    , name(name)
    , origin(origin)
    , publisher(publisher)
    , publish_time_ms(publish_time_ms)
    , hops(hops)
  {
    this->ttl(ttl);
  }

  DESERIALIZING_CTOR(ContentMetadata)
    : INIT_FIELDS(hash,
                  size,
                  block_size,
                  types,
                  name,
                  origin,
                  publisher,
                  publish_time_ms,
                  hops)
  {
    std::uint32_t ttl_ms = 0;
    GET_FIELD(ttl_ms);
    ttl(Ms(ttl_ms));
  }

  SERIALIZER()
  {
    PUT_FIELDS(hash,
               size,
               block_size,
               types,
               name,
               origin,
               publisher,
               publish_time_ms,
               hops);

    std::uint32_t const ttl_ms = ttl<Ms>().count();
    PUT_FIELD(ttl_ms);
  }

  std::size_t block_count() const { return 1 + ((size - 1) / block_size); }

  bool expired() const { return clock::now() >= expiry_time; }

  template <typename D>
  void ttl(D ttl)
  {
    expiry_time = clock::now() + ttl;
  }

  template <typename D>
  D ttl() const
  {
    auto const now = clock::now();
    if (now >= expiry_time)
      return D(0);
    else
      return std::chrono::duration_cast<D>(expiry_time - now);
  }
};
}
