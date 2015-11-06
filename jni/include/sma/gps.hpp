#pragma once

namespace sma
{
class GPS final
{
public:
  struct Coord {
    static double dist(Coord const& lhs, Coord const& rhs);
    static double dist_sq(Coord const& lhs, Coord const& rhs);

    Coord();
    Coord(double lon, double lat);

    double operator-(Coord const& rhs);
    bool operator==(Coord const& rhs);
    bool operator!=(Coord const& rhs);

    double lon;
    double lat;
  };

private:
  GPS();
  GPS(GPS const&);
  void operator=(GPS const&);
};
}
