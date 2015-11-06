#pragma once

#include <sma/ccn/contentmetadata.hpp>

#include <sma/util/serial.hpp>

#include <vector>


namespace sma
{
struct ContentAnn {
  TRIVIALLY_SERIALIZABLE(ContentAnn, metadata)

  std::vector<ContentMetadata> metadata;

  ContentAnn(std::vector<ContentMetadata> metadata)
    : metadata(std::move(metadata))
  {
  }

  ContentAnn(ContentAnn&&) = default;
  ContentAnn(ContentAnn const&) = default;

  ContentAnn& operator=(ContentAnn&&) = default;
  ContentAnn& operator=(ContentAnn const&) = default;
};
}
