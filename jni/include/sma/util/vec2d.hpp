#pragma once

#include <sma/util/serial.hpp>
#include <sma/util/floating.hpp>

#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>
#include <ostream>
#include <cmath>


namespace sma
{
struct Vec2d {
  using value_type = double;
  using Floating = FloatingPoint<value_type>;

  static Vec2d exp(Vec2d const& v)
  {
    return Vec2d(std::exp(v.x), std::exp(v.y));
  }

  static Vec2d divide(value_type const& dividend, Vec2d const& divisor)
  {
    return Vec2d(dividend / divisor.x, dividend / divisor.y);
  }

  static double distance(Vec2d const& a, Vec2d const& b)
  {
    return std::sqrt((a.x-b.x)*(a.x-b.x) + (a.y-b.y)*(a.y-b.y));  
  }

  static double angle (Vec2d const& a, Vec2d const& b)
  {
    return acos ( a.normalized().dot (b.normalized()) ); 
  }


  TRIVIALLY_SERIALIZABLE(Vec2d, x, y)

  Vec2d()
    : x(0.0)
    , y(0.0)
  {
  }

  Vec2d(value_type const& x, value_type const& y)
    : x(x)
    , y(y)
  {
  }

  bool operator==(Vec2d const& rhs) const
  {
    return Floating(x).AlmostEquals(Floating(rhs.x))
           && Floating(y).AlmostEquals(Floating(rhs.y));
  }
  bool operator!=(Vec2d const& rhs) const { return !(*this == rhs); }

  value_type length_sq() const { return x * x + y * y; }
  value_type length() const { return std::sqrt(length_sq()); }

  Vec2d normalized() const
  {
    auto len = 1.0 / length();
    return Vec2d(x * len, y * len);
  }

  Vec2d& operator+=(Vec2d const& rhs)
  {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }
  Vec2d& operator-=(Vec2d const& rhs)
  {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  Vec2d& operator+=(value_type const& scalar)
  {
    x += scalar;
    y += scalar;
    return *this;
  }
  Vec2d& operator-=(value_type const& scalar)
  {
    x -= scalar;
    y -= scalar;
    return *this;
  }

  Vec2d operator+(Vec2d const& rhs) const { return Vec2d(*this) += rhs; }
  Vec2d operator-(Vec2d const& rhs) const { return Vec2d(*this) -= rhs; }

  Vec2d operator+(value_type const& scalar) { return Vec2d(*this) += scalar; }
  Vec2d operator-(value_type const& scalar) { return Vec2d(*this) -= scalar; }

  Vec2d& operator*=(value_type const& scalar)
  {
    x *= scalar;
    y *= scalar;
    return *this;
  }
  Vec2d& operator/=(value_type const& scalar)
  {
    x /= scalar;
    y /= scalar;
    return *this;
  }

  Vec2d& operator*=(Vec2d const& rhs)
  {
    x *= rhs.x;
    y *= rhs.y;
    return *this;
  }
  Vec2d& operator/=(Vec2d const& rhs)
  {
    x /= rhs.x;
    y /= rhs.y;
    return *this;
  }

  Vec2d operator*(value_type const& scalar) const
  {
    return Vec2d(x * scalar, y * scalar);
  }
  Vec2d operator*(Vec2d const& rhs) const
  {
    return Vec2d(x * rhs.x, y * rhs.y);
  }
  Vec2d operator/(value_type const& scalar) const
  {
    return Vec2d(x / scalar, y / scalar);
  }
  Vec2d operator/(Vec2d const& rhs) const
  {
    return Vec2d(x / rhs.x, y / rhs.y);
  }

  value_type dot(Vec2d const& rhs) const { return x * rhs.x + y * rhs.y; }


  std::string to_string(std::size_t precision) const
  {
    std::ostringstream ss;
    ss << "(" << std::setprecision(precision) << x << ", "
       << std::setprecision(precision) << y << ")";
    return ss.str();
  }
  explicit operator std::string() const { return to_string(7); }


  value_type x;
  value_type y;
};

inline std::ostream& operator<<(std::ostream& os, Vec2d const& v)
{
  return os << std::string(v);
}
}
