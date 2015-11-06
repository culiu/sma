#pragma once

#include <sma/helper.hpp>

#include <vector>

#include <cstdlib>
#include <utility>
#include <cstdint>
#include <chrono>


namespace sma
{
struct Hash;
struct CcnNode;
struct NodeId;
struct ContentType;
struct ContentName;
struct ContentMetadata;

struct MessageHeader;
struct ContentAnn;
struct BlockRef;
struct BlockRequest;
struct BlockResponse;
struct BlockRequestArgs;

template <typename...>
struct Event;


class ContentHelper : public Helper
{
public:
  ContentHelper(CcnNode& node);

  virtual ~ContentHelper() {}

  //! Receive a content metadata announcement.
  /*! A metadata announcement indicates that a piece of content is available
   * from the publisher named in the metadata.  The ann may be
   * forwarded so the publisher may be an undefined number of hops remote.
   */
  virtual void receive(MessageHeader header, ContentAnn msg) = 0;

  virtual void receive(MessageHeader header, BlockRequest req) = 0;
  virtual void receive(MessageHeader header, BlockResponse resp) = 0;

  virtual ContentMetadata create_new(std::vector<ContentType> types,
                                     ContentName name,
                                     void const* src,
                                     std::size_t size) = 0;

  virtual std::vector<ContentMetadata> metadata() = 0;
  virtual std::vector<ContentMetadata> remote_metadata() = 0;
  virtual std::uint16_t announce_metadata() = 0;

  //! Send one message to the network containing all of the given block
  //! requests.
  virtual std::uint16_t request(std::vector<BlockRequestArgs> requests) = 0;

  //! Send one message to the network the content request
  virtual void request_content(Hash hash, float utility, std::uint32_t ttl) = 0;

  //! Broadcast completely the data for the specified block.
  // \return An interface that can be used to keep or mark as free the cache
  //         entry for the block data that were broadcast.
  virtual bool broadcast(BlockRef block, std::uint16_t & bytes_sent) = 0;

  virtual std::size_t frozen(std::vector<BlockRef> const& blocks, bool enabled)
      = 0;

  //! Fired when a metadata item arrives that matches one of our interests.
  virtual Event<ContentMetadata>& on_interesting_content() = 0;

  //! Fired when a nonempty set of block requests arrives from the network.
  virtual Event<NodeId, std::vector<BlockRequestArgs>>& on_blocks_requested()
      = 0;
  //! Fired when a previously broadcast request exceeds its Time to Live
  //! argument.
  virtual Event<BlockRef>& on_request_timeout() = 0;
  //! Fired when block data arrive and are cached by the content helper.
  // The \a CacheEntry argument provides functions to keep or release the cache
  // entry for the given data.
  virtual Event<NodeId, BlockRef>& on_block_arrived() = 0;
  //! Fired when the last block for the given content arrives.
  virtual Event<Hash>& on_content_complete() = 0;

  virtual bool has_content_stored(Hash filename) const = 0;

//  using millis = std::chrono::milliseconds;
  virtual std::chrono::milliseconds  get_avg_latency() = 0;
};
}
