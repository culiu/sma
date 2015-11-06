#pragma once

#include <sma/gps.hpp>
#include <sma/component.hpp>

namespace sma
{
class GpsComponent : public Component
{
public:
  virtual ~GpsComponent() {}

  virtual GPS::Coord position() const = 0;
};
}
