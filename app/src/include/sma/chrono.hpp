#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <chrono>

#include <ctime>
#include <string>


namespace sma
{
namespace chrono
{
  struct system_clock {
  public:
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point
        = std::chrono::time_point<std::chrono::system_clock, duration>;

    static const bool is_monotonic = false;

    static time_point now();

    static time_point from_time_t(std::time_t t);
    static std::time_t to_time_t(const time_point& tp);
    static std::string strftime(const time_point& tp, const char* format);
    static std::string utcstrftime(const time_point& tp);
  };
}
}
