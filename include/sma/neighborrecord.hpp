#pragma once

#include <sma/chrono.hpp>
#include <sma/util/vec2d.hpp>
#include <sma/util/circularbuffer.hpp>


namespace sma
{
struct NeighborRecord {
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  NeighborRecord(Vec2d position)
    : first_seen(clock::now())
  {
    saw(position);
  }

  bool is_new() const { return first_seen == last_seen; }

  template <typename D>
  bool older_than(D const& age) const
  {
    return std::chrono::duration_cast<D>(clock::now() - last_seen) >= age;
  }

  void saw(Vec2d position)
  {
    last_seen = clock::now();

    positions.push_back(std::make_pair(last_seen, position));
    velocity = compute_velocity();

    times_pinged = 0;
  }

  Vec2d position() const { return positions.clast().second; }

  CircularBuffer<std::pair<time_point, Vec2d>, 4> positions;
  Vec2d velocity;
  time_point first_seen;
  time_point last_seen;
  unsigned int times_pinged;

private:
  Vec2d mean_position;

  Vec2d compute_velocity()
  {
    if (positions.size() < 4)
      return Vec2d(0.0, 0.0);

    Vec2d last_mean;
    Vec2d dev;
    for (std::size_t k = 1; k < positions.size(); ++k) {
      last_mean = mean_position;
      auto const& position = positions[k].second;
      mean_position += (positions[k].second - last_mean) / k;
      dev += (position - last_mean) * (position - mean_position);
      dev *= dev;
    }

    return position() / Vec2d::exp(Vec2d::divide(4.0, mean_position) * dev);
  }
};
}
