#include <sma/neighborhelperimpl.hpp>

#include <sma/nodeid.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/messageheader.hpp>
#include <sma/beacon.hpp>

#include <sma/async.hpp>
#include <sma/context.hpp>

#include <sstream>
#include <iomanip>

#include <chrono>

namespace sma
{

constexpr unsigned int NeighborHelperImpl::INITIAL_REFRESH_MS;

NeighborHelperImpl::NeighborHelperImpl(CcnNode& node)
  : NeighborHelper(node)
{
  schedule_beacon(std::chrono::milliseconds(100));
}

std::vector<Neighbor> NeighborHelperImpl::get() const
{
  std::vector<Neighbor> v;
  for (auto& pair : neighbors)
    v.push_back(
        Neighbor{pair.first, pair.second.position(), pair.second.velocity});
  return v;
}

void NeighborHelperImpl::saw(NodeId const& node, Vec2d const& position)
{
  auto it = neighbors.find(node);
  if (it == neighbors.end()) {
    neighbors.emplace(node, NeighborRecord(position));
  } else {
    it->second.saw(position);
  }
}

Vec2d NeighborHelperImpl::get_position (NodeId id) const
{
  auto neighbor_it = neighbors.find(id);
  if (neighbor_it != neighbors.end())
    return (neighbor_it->second).position();
  else
    return node.position();
}

void NeighborHelperImpl::receive(MessageHeader header, Beacon msg)
{
  saw(header.sender, msg.position);
  std::ostringstream ss;
  std::size_t count = neighbors.size();
  for (auto& pair : neighbors) {
    ss << pair.first;
    if (--count != 0)
      ss << ", ";
  }
//  log.t("neighbors: %v", ss.str());

  if (!msg.is_response)
    node.post(Beacon(node.position(), true));
}

using millis = std::chrono::milliseconds;
void NeighborHelperImpl::schedule_beacon(millis delay)
{
  using unit = millis::rep;
  unit min = delay.count() / 2;
  delay = millis(min + rand() % delay.count());
  asynctask(&NeighborHelperImpl::beacon, this).do_in(delay);
}

void NeighborHelperImpl::beacon()
{
  node.post(Beacon(node.position()));
  schedule_beacon(std::chrono::seconds(2));
}

void NeighborHelperImpl::refresh_neighbors()
{
  if (neighbors.empty())
    return;

  bool will_beacon = false;
  std::vector<NodeId> dropped;

  auto it = neighbors.begin();
  while (it != neighbors.end()) {
    if (it->second.older_than(millis(INITIAL_REFRESH_MS))) {
      if (it->second.times_pinged++ < 3) {
        will_beacon = true;
      } else {
        dropped.push_back(it->first);
        it = neighbors.erase(it);
        continue;
      }
    }
    ++it;
  }
  if (will_beacon) {
    beacon();
    schedule_refresh(query_response_delay);
  }

  on_departure(std::move(dropped));
}

void NeighborHelperImpl::schedule_refresh(millis const& delay)
{
  asynctask(&NeighborHelperImpl::refresh_neighbors, this).do_in(delay);
}
}
