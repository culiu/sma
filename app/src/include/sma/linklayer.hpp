#pragma once

#include <cstdlib>


namespace sma
{
class CcnNode;
class Link;
class ForwardStrategy;

class LinkLayer
{
public:
  LinkLayer() {}

  LinkLayer(LinkLayer const& r) = delete;
  LinkLayer& operator=(LinkLayer const& r) = delete;

  virtual ~LinkLayer() {}

  //! Set the node that will receive messages received by the link layer.
  void receive_to(CcnNode& node) { this->node = &node; }

  //! Set the strategy used to manage the outgoing message queue.
  /*! If a forwarding strategy is set, it will be invoked after an outgoing
   * message is buffered and must call the link layer's \a forward_one
   * method to send the message.
   */
  LinkLayer& forward_strategy(ForwardStrategy& strategy)
  {
    this->fwd_strat = &strategy;
    return *this;
  }

  //! Gracefully stop communication between the node and the links.
  virtual void stop() = 0;
  //! Forward the next queued outgoing message to the network.
  /*! \return the number of queued messages remaining.
   */
  virtual std::size_t forward_one() = 0;

  //! Place the message in the outgoing message queue.
  virtual void enqueue(void const* src, std::size_t size) = 0;

protected:
  //! The target of incoming messages.
  CcnNode* node = nullptr;

  //! The strategy for forwarding outgoing messages to the network.
  /*! Naive blasting to contended networks will cause avoidable loss. The
   * strategy should make intelligent decisions to share the network capacity
   * with other peers.
   */
  ForwardStrategy* fwd_strat = nullptr;

private:
  friend class Link;
  //! Callback from link implementations when they have data to be read.
  virtual void on_link_readable(Link& link) = 0;
};
}
