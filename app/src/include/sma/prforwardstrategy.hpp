#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/forwardstrategy.hpp>

#include <mutex>
#include <chrono>
#include <random>

namespace sma
{
class LinkLayer;
struct Context;

class PrForwardStrategy : public ForwardStrategy
{
public:
  PrForwardStrategy(Context& context);

  virtual ~PrForwardStrategy() {}

  virtual void notify() override;

private:
  using Lock = std::lock_guard<std::recursive_mutex>;

  void schedule_timeslot();
  void do_timeslot();

  std::chrono::nanoseconds timestep = std::chrono::nanoseconds(10000);
  double threshold = 0.5;

  std::recursive_mutex mx;
  bool is_scheduled = false;

  std::uniform_real_distribution<> distribute;

  Context* context;
};
}
