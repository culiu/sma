#pragma once

#include <sma/helper.hpp>
#include <sma/neighbor.hpp>
#include <sma/smafwd.hpp>

//#include <sma/io/log.hpp>

#include <vector>


namespace sma
{
class CcnNode;

class NeighborHelper : public Helper
{
public:
  NeighborHelper(CcnNode& node);

  virtual ~NeighborHelper() {}

  //! Update the neighbor table to include the specified node.
  virtual void saw(NodeId const& node, Vec2d const& position) = 0;

  //! Handle an incoming beacon from a neighbor.
  virtual void receive(MessageHeader header, Beacon msg) = 0;

  virtual std::vector<Neighbor> get() const = 0;

  virtual Vec2d get_position (NodeId id) const = 0;
};
}
