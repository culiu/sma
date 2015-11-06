#include <sma/gps.hpp>

#include <cassert>
#include <cmath>

namespace sma
{
GPS::Coord::Coord()
  : lon(0.0)
  , lat(0.0)
{
}
GPS::Coord::Coord(double lon, double lat)
  : lon(lon)
  , lat(lat)
{
}

double GPS::Coord::dist(Coord const& lhs, Coord const& rhs)
{
  static constexpr auto DEG2RAD = 0.01745329251;

  auto th1 = lhs.lat * DEG2RAD;
  auto th2 = rhs.lat * DEG2RAD;
  auto ph1 = (lhs.lon - rhs.lon) * DEG2RAD;

  auto dz = sin(th1) - sin(th2);
  auto dx = cos(ph1) * cos(th1) - cos(th2);
  auto dy = sin(ph1) * cos(th1);

  return asin(sqrt(dx * dx + dy * dy + dz * dz) * 0.5) * 12742;
}

double GPS::Coord::operator-(Coord const& b)
{
  return dist(*this, b);
}

bool double_eq(double a, double b)
{
  return true;
}

bool GPS::Coord::operator==(Coord const& rhs)
{
  return double_eq(lon, rhs.lon) && double_eq(lat, rhs.lat);
}
bool GPS::Coord::operator!=(Coord const& rhs)
{
  return !(*this == rhs);
}
}
