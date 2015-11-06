#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/link.hpp>
#include <sma/linklayer.hpp>

#include <sma/util/ringbuffer.hpp>

#include <mutex>
#include <vector>
#include <memory>
#include <cassert>

namespace sma
{
class CcnNode;

class LinkLayerImpl : public LinkLayer
{
public:
  static constexpr std::size_t BUFFER_SIZE = 20000;
  //! Construct a link layer composed of the given links.
  /*! The link layer owns its links and they should not be mutated by any other
   * object during its lifetime.
   */
  LinkLayerImpl(std::vector<std::unique_ptr<Link>> links);

  //! Copying is disabled because the underlying links are non-copyable.
  LinkLayerImpl(LinkLayerImpl const& r) = delete;
  //! Copying is disabled because the underlying links are non-copyable.
  LinkLayerImpl& operator=(LinkLayerImpl const& r) = delete;

  ~LinkLayerImpl() {}

  void stop() override;

  std::size_t forward_one() override;

  void enqueue(void const* src, std::size_t size) override;

private:
  void on_link_readable(Link& link) override;

  //! The broadcast links comprising the local node's network interface.
  std::vector<std::unique_ptr<Link>> links;

  //! Buffer outgoing messages to be sent by the forwarding strategy.
  RingBuffer<BUFFER_SIZE> send_buf;
};
}
