#include <sma/async.hpp>
#include <sma/android/jniasynchelper.hpp>

//#include <sma/io/log.hpp>


namespace sma
{
//static Logger log("Async");

void Async::schedule(std::function<void()> f, std::chrono::nanoseconds delay)
{
  using clock = std::chrono::system_clock;
  auto const clock_delay = std::chrono::duration_cast<clock::duration>(delay);
  asynctaskqueue.emplace(clock::now() + clock_delay, std::move(f));
  schedule_async_on_service_thread(delay);
}

void Async::purge()
{
//  log.d("Purging async tasks");
  asynctaskqueue.clear();
}
}
