#pragma once

#include <chrono>
#include <memory>
#include <future>
#include <cassert>
#include <utility>
#include <functional>
#include <type_traits>

namespace sma
{
struct Async {
  // Defined in the platform-specific implementation
  static void purge();
  static void schedule(std::function<void()> f, std::chrono::nanoseconds delay);
};


template <typename F, typename... A>
struct AsyncTask {
  using result_type = typename std::result_of<F(A...)>::type;

  template <typename U, typename... B>
  AsyncTask(U&& f, B&&... args)
    : task(std::make_shared<std::packaged_task<result_type()>>(
          std::bind(std::forward<U>(f), std::forward<B>(args)...)))
  {
  }

  AsyncTask(AsyncTask&& t)
    : task(std::move(t.task))
  {
  }

  AsyncTask& operator=(AsyncTask&& t)
  {
    std::swap(task, t.task);
    return *this;
  }

  template <typename Delay>
  std::future<result_type> do_in(Delay delay);
  std::future<result_type> do_now()
  {
    return do_in(std::chrono::nanoseconds(0));
  }

private:
  std::shared_ptr<std::packaged_task<result_type()>> task;
};


template <typename F, typename... A>
AsyncTask<F, A...> asynctask(F&& f, A&&... args)
{
  return AsyncTask<F, A...>(std::forward<F>(f), std::forward<A>(args)...);
}

template <typename F, typename... A>
template <typename Delay>
std::future<typename AsyncTask<F, A...>::result_type>
AsyncTask<F, A...>::do_in(Delay delay)
{
  auto task = std::move(this->task);
  assert(task);
  auto fut = task->get_future();
  Async::schedule([=]() { (*task)(); },
                  std::chrono::duration_cast<std::chrono::nanoseconds>(delay));
  return fut;
}
}
