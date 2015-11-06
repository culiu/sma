#pragma once

#include <sma/ccn/contentmetadata.hpp>
#include <sma/networkdistance.hpp>

#include <sma/chrono.hpp>

#include <chrono>
#include <cstdint>


namespace sma
{
class ContentHelper;

class RemoteContent
{
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

public:
  RemoteContent(ContentHelper& helper,
                ContentMetadata metadata,
                NetworkDistance distance);

  ContentMetadata metadata() const { return meta; }

  bool saw(NetworkDistance distance);

private:
  void touch() { last_seen = clock::now(); }

  ContentHelper& helper;

  ContentMetadata meta;
  time_point last_seen;
  NetworkDistance dist;
};
}
