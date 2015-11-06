#pragma once

#include <sma/util/hash.hpp>
#include <sma/ccn/storedblock.hpp>

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <utility>


namespace sma
{
class ContentStore;


class StoredContent
{
public:
  StoredContent();
  virtual ~StoredContent() {}

  virtual StoredBlock* block(std::uint32_t index) = 0;
  virtual StoredBlock const* cblock(std::uint32_t index) const = 0;

  virtual StoredBlock& create_block(std::uint32_t index, std::uint32_t size)
      = 0;
};
}
