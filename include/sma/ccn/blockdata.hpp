#pragma once

#include <cstdlib>
#include <cstdint>


namespace sma
{
class ContentCache;

class BlockData
{
public:
  bool exists() const;
  operator bool() const;

  bool complete() const;

  std::uint8_t* data() const;
  std::uint8_t const* cdata() const;

  std::size_t size() const;

  bool frozen() const;
  bool frozen(bool enable);

  bool operator==(BlockData const& rhs) const;
  bool operator!=(BlockData const& rhs) const;

private:
  friend class ContentCache;

  BlockData();
  BlockData(ContentCache* cache, std::size_t idx);

  ContentCache* cache;
  std::size_t idx;
};
}
