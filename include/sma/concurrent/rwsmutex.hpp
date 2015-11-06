#pragma once

#include <atomic>


namespace sma
{
struct RwsMutex {
  std::atomic_size_t ww{0};                 // writers waiting for readers
  std::atomic_size_t w{0};                  // writers waiting to write
  std::atomic_size_t r{0};                  // readers reading
  std::atomic_flag f = ATOMIC_FLAG_INIT;    // writing now
};

class WriterLock
{
public:
  WriterLock(RwsMutex& mx)
    : mx(mx)
  {
    // Begin writer cohort by blocking new readers from joining while we
    // wait for existing readers to finish.
    ++mx.ww;
    // ------------------------------------
    // 1. WRITER COHORT BEING JOINED - CLOSED to READERS, OPEN to WRITERS
    //    All writers that reach this point before we decrement mx.ww
    //    will join a cohort that cannot be preempted by a reader.

    while (mx.r)
      ;
    // ------------------------------------
    // 2. EXISTING READERS FINISHED - still CLOSED to READERS
    ++mx.w;
    --mx.ww;
    if (!mx.ww)
      // ------------------------------------
      // 3. WRITER COHORT OPENED - now OPEN to READERS and WRITERS
      //  The first reader to enter will finalize this cohort and close
      //  the lock for writers.
      stage = 1;
    // Spin until the previous state was cleared, leaving it set
    while (mx.f.test_and_set())
      ;
    stage = 2;
    // readers and writers are locked; begin critical section
  }

  ~WriterLock()
  {
    if (stage > 0) {
      if (stage == 2) {
        // If some writer is spinning on mx.f, at this point she will see
        // it return false and thus she will acquire the write lock.
        mx.f.clear();
      }
      // Let waiting readers start reading.
      // If another writer was waiting for us, she will have already started
      // as we cleared mx.f. Regardless, she has incremented mx.w so readers
      // cannot be unblocked until she also gets here.
      --mx.w;
    } else {
      // We can only get here if WriterLock goes out of scope before
      // successfully acquiring a lock (e.g. exception while waiting for
      // mx.r).
      --mx.ww;
    }
  }

private:
  RwsMutex& mx;
  int stage{0};
};

class ReaderLock
{
public:
  ReaderLock(RwsMutex& mx)
    : mx(mx)
  {
    while (mx.ww)
      ;
    ++mx.r;
    locked = true;
    while (mx.w)
      ;
  }

  ~ReaderLock()
  {
    if (locked) {
      --mx.r;
    }
  }

private:
  RwsMutex& mx;
  bool locked{false};
};
}
