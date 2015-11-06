#pragma once

#include <cstdint>
#include <mutex>
#include <condition_variable>
#include <functional>


namespace sma
{

class cyclic_barrier
{
 public:
  cyclic_barrier(std::size_t nthreads);
  cyclic_barrier(std::size_t nthreads, std::function<void()>&& on_open);
  ~cyclic_barrier();

  void wait();

 private:
  std::function<void()>     on_open;

  const std::size_t         nthreads;
  std::size_t               nwaiting;
  std::size_t               nto_leave;

  std::mutex                mutex;
  std::condition_variable   closed;
  std::condition_variable   open;

  bool                      forced_open;
};

}
