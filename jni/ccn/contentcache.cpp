#include <sma/ccn/contentcache.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contentmetadata.hpp>
#include <sma/ccn/blockdata.hpp>

//#include <sma/io/log>

#include <string>
#include <cstring>
#include <istream>
#include <cassert>
#include <algorithm>


namespace sma
{
namespace detail
{
  template <typename T>
  T next_power_of_two(T const& t)
  {
    T pow = 1;
    while (pow < t)
      pow *= 2;
    return pow;
  }
}


constexpr std::size_t ContentCache::block_size;


/*void ContentCache::log_utilization() const
{
  if (not expandable) {
    log.i("cache capacity, %v", capacity);
    int frozen_slots = 0;
    int unfrozen_slots = 0;
    auto it = occupied_idxs.begin();
    while (it != occupied_idxs.end()) {
      if (slots[*it].frozen)
          frozen_slots++;
      else
          unfrozen_slots++;
      it++;
    }
//    log.d("occupied_idxs.size(): %v", occupied_idxs.size());
//    log.d("block_size: %v", block_size);
//    log.d("capacity: %v", capacity);
    log.i("cache utilization, %v, %v",
          double(frozen_slots * block_size) / capacity,
          double(occupied_idxs.size() * block_size) / capacity);
  }
}*/


ContentCache::ContentCache(CcnNode& node, std::size_t capacity, bool expandable)
//  : log(node.log)
  : capacity(capacity == 0 ? 0 : detail::next_power_of_two(capacity))
  , slots(capacity / block_size)
  , free_idxs(capacity / block_size)
  , expandable(expandable)
{
  std::size_t i = 0;
  for (auto& idx : free_idxs)
    idx = i++;
}


BlockData ContentCache::end() const { return BlockData(); }


void ContentCache::grow_to_fit(std::size_t count)
{
  if (count <= free_idxs.size())
    return;

  auto slots_to_add = count - free_idxs.size();
  auto new_idx = slots.size();
  slots.resize(new_idx + slots_to_add);
  for (std::size_t i = 0; i < slots_to_add; ++i)
      free_idxs.push_back(new_idx++);
}


void ContentCache::free_slots(std::size_t count)
{
  if (count == 0)
    return;

  count = std::min(count, occupied_idxs.size());

  auto begin = occupied_idxs.begin();
//  auto it = --occupied_idxs.end();
  auto it = occupied_idxs.end();
  while (it != begin && count != 0) {
    it--;
    auto const idx = *it;
    auto& slot = slots[idx];
    if (not slot.frozen) {
      slot.size = slot.expected_size = 0;
      slot.block_index = 0;
//      it = --occupied_idxs.erase(it);
      it = occupied_idxs.erase(it);
      free_idxs.push_back(idx);
      count--;
    } 
    // else {
//      --it;
 //   }
//    if (it == begin)
//      count = 1;
  }
}


std::size_t ContentCache::ensure_capacity(std::size_t count)
{
  if (free_idxs.size() < count) {
//    if (capacity == 0)
    if (expandable)
      grow_to_fit(count);
    else
      free_slots(count - free_idxs.size());
  }

  return std::min(count, free_idxs.size());
}


std::size_t ContentCache::reserve_slot()
{
  auto const min_capacity = ensure_capacity(1);
//  assert(min_capacity == 1
//         && "Can't reserve slot; cache is full and no slots can be freed.");
  if (min_capacity <= 0) {
    auto const idx = occupied_idxs.back();
    occupied_idxs.pop_back();
    occupied_idxs.push_front(idx);
    return idx;
  }

  auto const idx = free_idxs.back();
  free_idxs.pop_back();
  occupied_idxs.push_front(idx);

  return idx;
}


std::vector<std::size_t> ContentCache::reserve_slots(std::size_t count)
{
  auto const min_capacity = ensure_capacity(count);
  assert(min_capacity == count
         && "Can't reserve slots; cache is full and no slots can be freed.");

  // Copy and erase slot indices all at once; first to the result vector and
  // then to the head of the occupied list.
  auto const& end = free_idxs.cend();
  auto const& begin = end - count;
  auto idxs = std::vector<std::size_t>(begin, end);
  occupied_idxs.insert(occupied_idxs.begin(), begin, end);
  free_idxs.erase(begin, end);

  return idxs;
}


void ContentCache::promote(std::size_t idx)
{
  auto it = occupied_idxs.begin();
  while (it != occupied_idxs.end()){
    if (*it == idx) {
      occupied_idxs.erase(it);
      occupied_idxs.push_front(idx);
      return;
    }
    it++;
  }
}


Hash ContentCache::store(void const* src, std::size_t size)
{
  std::size_t const block_count = 1 + ((size - 1) / block_size);

  auto hash = Hasher(src, size).digest();

  auto existing = content.find(hash);
  if (existing != content.end()) {
    assert(existing->second.size() == block_count
           && "Data already cached with this hash has different block count.");
//    LOG(DEBUG) << "Not overwriting existing cache entry for " << hash;
    return hash;
  }

  auto taken_idxs = reserve_slots(block_count);

  // Total size read in bytes
  std::size_t read = 0;
  BlockIndex block_index = 0;    // of block currently being created

  auto bsrc = reinterpret_cast<std::uint8_t const*>(src);
  while (read != size) {
    auto& slot = slots[taken_idxs[block_index]];
    slot.block_index = block_index++;

    auto to_read = std::min(block_size, size - read);
    std::memcpy(slot.data, bsrc + read, to_read);
    read += to_read;
    slot.expected_size = slot.size = to_read;
  }

//  log.d("New content cached");
//  log.d("| hash: %v", hash);
//  log.d("| size: %v", read);
//  log.d("| blocks: %v", taken_idxs.size());

  content.emplace(hash, std::move(taken_idxs));

//  log_utilization();

  return hash;
}


BlockData ContentCache::store(BlockRef ref,
                              std::size_t expected_size,
                              void const* src,
                              std::size_t size)
{
  auto& idxs = content[ref.hash];
  for (auto& idx : idxs)
    if (slots[idx].block_index == ref.index)
      return end();

  auto idx = reserve_slot();
  idxs.push_back(idx);

  assert (idx < slots.size()); 
  auto& slot = slots[idx];
  std::memcpy(slot.data, src, size);
  slot.expected_size = expected_size;
  slot.size = size;
  slot.block_index = ref.index;

//  log_utilization();

  return BlockData(this, idx);
}

bool ContentCache::copy_block_from_to (ContentCache& src,
                                       ContentCache& dst,
                                       BlockRef block)
{
//  assert (src != nullptr && dst != nullptr);
  auto const& src_data = src.find(block);
  assert (src_data.exists());
  auto const& dst_data = dst.find(block);
  if (dst_data.exists()) {
    // ToDo: should verify the data in both slots match.
    return false;
  } else {
    assert(src_data.complete());
    auto new_block = dst.store (block,
                                src_data.size(), 
                                src_data.data(),
                                src_data.size());
    assert (new_block.exists());
    return true;
  }
}

BlockData ContentCache::find(BlockRef ref)
{
  auto it = content.find(ref.hash);
  if (it != content.end()) {
    auto const& slot_idxs = it->second;
    for (auto const idx : slot_idxs)
    { 
      if (slots[idx].block_index == ref.index)
        return BlockData(this, idx);
    }
  }

  return end();
}


BlockData ContentCache::operator[](BlockRef ref) { return find(ref); }


bool ContentCache::validate_data(ContentMetadata const& metadata) const
{
  auto it = content.find(metadata.hash);
  if (it == content.end())
    return false;

  auto const block_count = metadata.block_count();

  auto const& slot_idxs = it->second;
  if (slot_idxs.size() != block_count)
    return false;

  std::size_t size = 0;
  for (auto const idx : slot_idxs)
    size += slots[idx].size;
  assert(size == metadata.size && "Block count valid, but data size is not.");

  return true;
}


std::vector<BlockIndex>
ContentCache::missing_blocks(ContentMetadata const& metadata) const
{
  auto block_count = metadata.block_count();
  std::vector<bool> missing(block_count, true);

  auto it = content.find(metadata.hash);
  if (it != content.end()) {
    auto const& slot_idxs = it->second;
    for (auto const idx : it->second) {
      if (slots[idx].size < slots[idx].expected_size)
          continue;
      missing[slots[idx].block_index] = false;
      --block_count;
    }
  }

  std::vector<BlockIndex> missing_idxs;
  missing_idxs.reserve(block_count);
  for (std::size_t i = 0; i < missing.size(); ++i)
    if (missing[i])
      missing_idxs.push_back(i);

  return missing_idxs;
}

bool
ContentCache::has_content_stored(Hash filename) const
{
  //cache, not permanent store
  if (!expandable)
    return false;
  else {
    auto find_result = content.find(filename);
    return (find_result != content.end());
  }
}
}

/*
std::vector<std::size_t> negate1d(std::vector<std::size_t> segments,
                                  std::size_t max)
{
}

std::vector<std::size_t> intersect1d(std::vector<std::size_t> a,
                                     std::vector<std::size_t> b)
{
  assert(a.size() % 2 == 0);
  assert(b.size() % 2 == 0);

  std::vector<std::size_t> intersection;

  for (std::size_t ai = 0; ai < a.size() - 1; ai += 2) {
    al = a[ai];
    ar = a[ai + 1];
    assert(al < ar);

    for (std::size_t bi = 0; bi < b.size() - 1; bi += 2) {
      bl = b[bi];
      br = b[bi + 1];
      assert(bl < br);

      // Not yet reached our segment
      if (br <= al)
        continue;
      // B will only increase farther past our segment
      if (bl >= ar)
        break;

      if (bl <= al && br >= ar) {
        // A is inside B
        intersection.push_back(al);
        intersection.push_back(ar);
      } else if (al <= bl && ar >= br) {
        // B is inside A
        intersection.push_back(bl);
        intersection.push_back(br);
      }
    }
  }
}
*/
