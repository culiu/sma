#pragma once

#include <sma/ccn/blockref.hpp>

#include <sma/util/hash.hpp>

//#include <sma/io/log>

#include <iosfwd>
#include <utility>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <list>
#include <unordered_map>


namespace sma
{
class CcnNode;
struct ContentMetadata;
class BlockData;

class ContentCache
{
public:
  // The size of cache slots and the blocks into which content data are
  // divided.
  static constexpr std::size_t block_size = 4096;

  // Copy slot from cache to store.
  // Return true when copy succeeds
  // Return false when fail: (1) src or dst are unavailable
  //                         (2) dst already has the copy.
  static bool copy_block_from_to (ContentCache& src,
                                  ContentCache& dst, 
                                  BlockRef block);

  static_assert(!(block_size == 0) && !(block_size & (block_size - 1)),
                "Block size must be a power of 2.");


  // Construct a cache with the given capacity in bytes.
  // If capacity is zero this cache is unbounded and dynamic with no initial
  // size; otherwise it is a bounded LRU allocated at construction.
  ContentCache(CcnNode& node, std::size_t capacity = 0, bool expandable = true);

  BlockData end() const;

  // Return the cached block data for the given content block or
  // ContentCache::end() if the block is not cached, even partially.
  BlockData find(BlockRef ref);
  BlockData operator[](BlockRef ref);

  // Insert a new item of content into the cache by dividing into blocks,
  // hashing, and caching the given data.
  // Return the content's hash, also its unique internal identifier.
  Hash store(void const* src, std::size_t size);

  BlockData store(BlockRef ref,
             std::size_t expected_size,
             void const* src,
             std::size_t size);

  // Return true if all blocks for the given content exist and are complete.
  bool validate_data(ContentMetadata const& metadata) const;

  // Get the indices of all blocks that are either missing or incomplete
  // for the given content.
  std::vector<BlockIndex> missing_blocks(ContentMetadata const& metadata) const;
//  void log_utilization() const;

  bool has_content_stored(Hash filename) const;

  float progress(ContentMetadata const & metadata) const;

private:
  friend class BlockData;

  struct Slot {
    BlockIndex block_index;

    std::uint8_t data[block_size];
    // The actual size of stored data.
    std::size_t size{0};
    // The size of data that should be stored in this slot.
    std::size_t expected_size{0};
    // True if this slot can't be evicted or recycled.
    bool frozen{false};
  };

  void grow_to_fit(std::size_t count);
  void free_slots(std::size_t count);
  std::size_t ensure_capacity(std::size_t count);

  std::size_t reserve_slot();
  std::vector<std::size_t> reserve_slots(std::size_t count);

  void promote(std::size_t idx);

  bool expandable;

//  Logger log;

  std::size_t capacity;
  // The actual data, initially allocated when capacity is nonzero.
  std::vector<Slot> slots;
  // LRU with most-recently-accessed slots at the head.
  // When a bounded cache is full it will overwrite these from the tail.
  std::list<std::size_t> occupied_idxs;
  std::vector<std::size_t> free_idxs;
  std::unordered_map<Hash, std::vector<std::size_t>> content;
};
}
