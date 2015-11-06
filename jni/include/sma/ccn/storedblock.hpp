#pragma once

#include <cstdlib>
#include <cstdint>


namespace sma
{
class StoredBlock
{
public:
  // Index of start of gap (inclusive) and end of gap (exclusive)
  using gaps_vec = std::vector<std::uint32_t>;

  StoredBlock(std::uint32_t index, std::uint32_t size)
    : index_(index)
    , size_(size)
  {
  }
  virtual ~StoredBlock() {}

  std::uint32_t index() const { return index_; }
  std::uint32_t size() const { return size_; }

  virtual bool complete() const = 0;

  virtual std::size_t
  read(std::uint8_t* dst, std::size_t src_off, std::size_t size) const = 0;

  std::size_t read(std::uint8_t* dst, std::size_t size) const
  {
    return read(dst, 0, size);
  }

  virtual void
  insert(std::size_t dst_off, std::uint8_t const* src, std::size_t size) = 0;

  virtual gaps_vec gaps() const = 0;

private:
  std::uint32_t index_;
  std::uint32_t size_;
};
}
