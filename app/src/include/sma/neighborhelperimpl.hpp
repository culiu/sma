#pragma once

#include <sma/neighborhelper.hpp>

#include <sma/smafwd.hpp>

#include <sma/nodeid.hpp>
#include <sma/neighbor.hpp>
#include <sma/neighborrecord.hpp>

#include <sma/util/event.hpp>

//#include <sma/io/log.hpp>

#include <sma/chrono.hpp>
#include <chrono>

#include <unordered_map>

namespace sma
{
class CcnNode;
struct Vec2d;

//! Maintain an up-to-date record of neighbors seen broadcasting traffic.
/*! Neighbors are maintained as long as they continue to broadcast; any
 * neighbors that haven't been seen recently will be polled to test their
 * presence.
 */
class NeighborHelperImpl : public NeighborHelper
{
public:
  NeighborHelperImpl(CcnNode& node);

  //! Add or update the given node's entry in the neighbor table.
  void saw(NodeId const& node, Vec2d const& position) override;

  //! Record the sender as a neighbor and respond to the beacon.
  void receive(MessageHeader header, Beacon msg) override;

  std::vector<Neighbor> get() const override;

  //! Fired when new neighbors arrive.
  Event<std::vector<NodeId>> on_arrival;
  //! Fired when known neighbors are dropped due to inactivity.
  Event<std::vector<NodeId>> on_departure;

  Vec2d get_position(NodeId id) const override;

private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;
  using millis = std::chrono::milliseconds;

  static constexpr unsigned int INITIAL_REFRESH_MS = 5000;

  //! The time to wait for an expired neighbor to respond to a beacon.
  millis query_response_delay{5000};

  void schedule_beacon(millis delay);
  //! Broadcast a beacon message to all neighbors.
  void beacon();
  //! Check if any neighbors have not been seen in some minimum interval.
  /*! If any nodes have not been seen then a broadcast/refresh cycle will be
   * initiated. Nodes are expected to respond to broadcasts before some delay;
   * if any nodes miss a maximum number of broadcasts they are removed from
   * the neighbor table and their departure is signaled.
   */
  void refresh_neighbors();
  //! Schedule a neighbor refresh after \a delay.
  void schedule_refresh(millis const& delay = millis(INITIAL_REFRESH_MS));

  time_point next_beacon_time;

  //! Immediate (one-hop) neighbors known to this node.
  std::unordered_map<NodeId, NeighborRecord> neighbors;
};
}
