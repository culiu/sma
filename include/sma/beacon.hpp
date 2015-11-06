#pragma once

#include <sma/util/serial.hpp>
#include <sma/util/vec2d.hpp>

#include <cstdint>

namespace sma
{
struct Beacon {
  TRIVIALLY_SERIALIZABLE(Beacon, position, is_response)

  /****************************************************************************
   * Serialized Fields
   */
  Vec2d position;
  bool is_response;
  /***************************************************************************/

  Beacon(Vec2d position, bool is_response = false)
    : position(position)
    , is_response(is_response)
  {
  }

  Beacon(Beacon const&) = default;
  Beacon& operator=(Beacon const&) = default;
};
}
