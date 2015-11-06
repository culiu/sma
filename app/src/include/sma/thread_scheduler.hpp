#pragma once

#include "threadpool.hpp"
#include "delay_queue.hpp"

#include <chrono>
#include <mutex>
#include <functional>


namespace sma
{
namespace detail
{
  class thread_scheduler final
  {
  public:
    static thread_scheduler& instance()
    {
      static thread_scheduler instance;
      return instance;
    }

    void schedule(std::function<void()> task, std::chrono::nanoseconds delay);

  private:
    thread_scheduler(){};
    thread_scheduler(thread_scheduler const&);
    void operator=(thread_scheduler const&);

    std::mutex mutex;
    threadpool threads;
    delay_queue<std::function<void()>> tasks;
  };
}
}
