#pragma once

#include <sma/nodeid.hpp>

#include <sma/chrono.hpp>

namespace sma
{
struct RemoteInterest {
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  RemoteInterest();

  inline void touch();
  bool update();

  template <typename Duration = std::chrono::milliseconds>
  Duration age()
  {
    return std::chrono::duration_cast<Duration>(clock::now() - last_seen);
  }

  template <typename Duration>
  bool older_than(Duration age)
  {
    return this->age<Duration>() >= age;
  }

  time_point last_seen;
};
}
