#pragma once

#include <atomic>

#include <cstdint>
#include <utility>
#include <cassert>
#include <stdexcept>
#include <type_traits>


namespace sma
{

template <std::size_t Size>
class RingBuffer
{
  using Buft = RingBuffer<Size>;
  //! An unsigned type that can index all entries in the buffer and all
  //! elements in an entry.
  using size_type = std::size_t;

  using value_type = std::uint8_t;

  //! Monotonically increasing sequence counter.
  using mono_type = std::uint64_t;
  //! Read/write ordered monotonic sequence number.
  using sequence = std::atomic<mono_type>;
  //! Produces the real array index when ANDed with a monotonic sequence.
  mono_type idx_mask;

  //! A read/write synchronized entry in the buffer.
  struct Entry {
    using value_type = Buft::value_type;
    using size_type = Buft::size_type;
    static constexpr size_type capacity = Size;

    Entry() = default;

    Entry(Entry const&) = delete;
    Entry& operator=(Entry const&) = delete;

    value_type data[capacity];
    size_type size;
    //! The entry's sequence number indicates the freshness of its contents.
    /*! Readers will not read an entry with an older sequence number than their
     * own; if the next entry has a lower sequence then the buffer is empty.
     */
    sequence seq;
    //! The number of readers holding locks on the entry.
    /*! Writers will not claim an entry with readers. If the next entry to be
     * written is read-locked then the buffer is overrun.
     */
    std::atomic_size_t readers;
    //! \a set if a writer has claimed the entry.
    /*! No other reader or writer will proceed to claim the entry while set.
     */
    std::atomic_flag writing = ATOMIC_FLAG_INIT;
  };

  friend class WritableEntry;
  friend class ReadableEntry;

public:
  //! Construct a new buffer with \a size entries of \a ENTRY_SIZE bytes.
  RingBuffer(size_type size)
    : entries(new Entry[size]())
    , idx_mask(size - 1)
  {
    assert(size != 0 && ((size & (size - 1)) == 0));
  }

  //! Delete the buffer entries without regard for open readers or writers.
  ~RingBuffer() { delete[] entries; }

  //! Get the number of unread entries in the buffer.
  size_type size() { return next_write - next_read - 1; }

  //! A scoped lock on a mutable buffer entry.
  /*! The next writable sequence number is claimed, but not assigned to the
   * entry until the writer is released.
   * Readers will be blocked on that entry because its sequence is lower than
   * theirs, and writers will be given a subsequent sequence number because
   * it's atomically incremented when assigned.
   */
  class WritableEntry
  {
    friend class RingBuffer<Size>;

    Entry* entry;

    WritableEntry(Entry& entry)
      : entry(&entry)
      , data(&(entry.data[0]))
      , size(&entry.size)
    {
    }

  public:
    constexpr typename Entry::size_type capacity() const { return Entry::capacity; }

    typename Entry::value_type* data;
    typename Entry::size_type* size;

    WritableEntry(WritableEntry&& r)
    {
      std::swap(entry, r.entry);
      data = r.data;
      size = r.size;
    }

    WritableEntry& operator=(WritableEntry&& r)
    {
      std::swap(entry, r.entry);
      data = r.data;
      size = r.size;
      return *this;
    }

    //! Release the buffer entry for reading.
    /*! This is guaranteed safe without locking because the sequence number
     * was atomically incremented when taken and readers are blocked on a cheap
     * ordered read until this commit takes place.
     */
    ~WritableEntry()
    {
      if (entry)
        entry->writing.clear();
    }
  };

  //! A scoped lock of an immutable buffer entry.
  /*! Unlimited readers may immutably reference an entry and it will not be
   * modified until they are all destructed.
   * This blocks writing, and since the ring buffer is neither destructive nor
   * dynamic may cause an overrun exception.
   */
  class ReadableEntry
  {
    friend class RingBuffer<Size>;

    Entry* entry;

    ReadableEntry(Entry* entry)
      : entry(entry)
    {
    }

  public:
    constexpr typename Entry::size_type capacity() const { return Entry::capacity; }

    ReadableEntry(ReadableEntry&& r) { std::swap(entry, r.entry); }
    ReadableEntry(ReadableEntry const& r)
    {
      if (r.entry)
        ++r.entry->readers;
      entry = r.entry;
    }

    ReadableEntry& operator=(ReadableEntry&& r)
    {
      std::swap(entry, r.entry);
      return *this;
    }
    ReadableEntry& operator=(ReadableEntry const& r)
    {
      if (r.entry)
        ++r.entry->reader;
      entry = r.entry;
      return *this;
    }

    //! Release a read lock on the entry.
    ~ReadableEntry()
    {
      if (entry)
        --entry->readers;
    }

    bool acquired() const { return entry != nullptr; }

    typename Entry::value_type const* cdata() const
    {
      assert(entry);
      return entry->data;
    }

    typename Entry::size_type size() const
    {
      assert(entry);
      return entry->size;
    }
  };

  //! Lock the next buffer entry for writing for the lifetime of the returned
  // object.
  /*! This is equivalent to lazily emplacing an entry in a queue such that it
   * is guaranteed to be read before subsequently emplaced entries.
   * Consequently, while further writes may take place concurrently, all reads
   * will be blocked at this entry until it has committed.
   */
  WritableEntry acquire()
  {
    // Try until we find a slot not being written to.
    // This may occur if every slot is being written to, in which case we busy
    // loop waiting for one to open.
    mono_type seq;
    Entry* entry;
    do {
      // Claim the next sequence number and the according buffer slot.
      seq = next_write++;
      // The sequence numbers outrun the buffer indices by one because the
      // readers need to start behind the first writer.
      entry = entries + ((seq - 1) & idx_mask);
      // If another writer set the flag then find a new slot.
    } while (entry->writing.test_and_set());
    // We have set the writing flag

    assert(entry->readers == 0 && "Ring buffer overrun");

    return WritableEntry(*entry);
  }

  ReadableEntry try_pop()
  {
    // Pretend to claim the next read position and, if we're successful,
    // CAS until we actually commit it.
    mono_type seq = next_read;
    Entry* entry;
    while (true) {
      entry = entries + (seq & idx_mask);
      ++entry->readers;
      // Writers are now blocked from claiming it, but one may have it already.
      if (entry->writing.test_and_set())
        return nullptr;

      entry->writing.clear();

      // Since we just pretended to increment the read position (in case we
      // failed), someone else might have succeeded while we tried.
      if (next_read.compare_exchange_weak(seq, seq + 1))
        break;
      // We failed the CAS; remove ourselves from the entry we chose and retry.
      --entry->readers;
    }
    return ReadableEntry(entry);
  }

private:
  Entry* entries;
  //! The next sequence number to be written.
  sequence next_write{1};
  //! The low-water mark for readers.
  /*! Readers will not enter an entry with a sequence number <= this value. */
  sequence next_read{0};
};
}
