#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/nodeid.hpp>
#include <sma/util/vec2d.hpp>

#include <sma/messagebuffer.hpp>
#include <sma/messageheader.hpp>

#include <sma/ccn/ccnfwd.hpp>
#include <sma/neighborhelper.hpp>
//#include <sma/io/log.hpp>

#include <cstdlib>
#include <vector>
#include <atomic>
#include <cstdint>

#include <queue>

namespace sma
{
class LinkLayer;

class CcnNode
{
public:
  CcnNode(NodeId id, Context& context);

  CcnNode(CcnNode const&) = delete;
  CcnNode& operator=(CcnNode const&) = delete;

  //! Gracefully terminate further network delegation or asynchronous tasks.
  void stop() { stopped = true; }

  //! Enqueue the given message data to be broadcast to the network.
  void post(void const* src, std::size_t size);
  //! Serialize the given message and enqueue it to be broadcast.
  template <typename M>
  std::uint16_t post(M const& msg,
            std::vector<NodeId> recipients = std::vector<NodeId>());


  //! 
  void do_post();
  /* Neighbor discovery */
  void receive(MessageHeader header, Beacon msg);

  /* Interest dissemination */
  void receive(MessageHeader header, InterestAnn msg);

  /* Content announcement dissemination */
  void receive(MessageHeader header, ContentAnn msg);
  void receive(MessageHeader header, BlockRequest msg);
  void receive(MessageHeader header, BlockResponse msg);

  Vec2d position() const;

  //! This node's universally unique identifier.
  NodeId const id;
  //! The application context in which this node is running.
  Context* const context;
  //! A logger tailored to provide context-sensitive output for this node.
//  Logger log;

  //! Manages the neighbor table to track what nodes are broadcasting in our
  //! neighborhood.
  NeighborHelper* neighbors = nullptr;
  //! Manages our personal interest table and that of other broadcasting nodes.
  InterestHelper* interests = nullptr;
  //! Manages content we're providing and remote content we've cached.
  ContentHelper* content = nullptr;
  //! Manages user behavior by triggering interest broadcast, content publication
  //  and content retrieval.
  BehaviorHelper* behavior = nullptr;

  ForwardScheduler* sched = nullptr;

private:
  //! \a true if the node's graceful termination has been requested.
  std::atomic_bool stopped{false};
  //! Interface to the link layer connecting this node to the network.
  LinkLayer* linklayer;
  std::queue<MessageBuffer<20000>> tolinkimpl_buf;
};

template <typename M>
std::uint16_t CcnNode::post(M const& msg, std::vector<NodeId> recipients)
{
  // Serialize the message (and its network header) into a memory buffer
  MessageBuffer<20000> buf(MessageHeader(id, std::move(recipients)), msg);
  tolinkimpl_buf.push(buf);
//  post(buf.cdata(), buf.size());
  return buf.size();
}

}
