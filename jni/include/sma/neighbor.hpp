#pragma once

#include <sma/nodeid.hpp>
#include <sma/util/vec2d.hpp>


namespace sma
{
struct Neighbor {
  NodeId id;
  Vec2d position;
  Vec2d velocity;
};
}
