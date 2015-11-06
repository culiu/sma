#pragma once

#include <sma/ccn/contenthelper.hpp>

#include <sma/ccn/contentmetadata.hpp>

#include <sma/ccn/blockref.hpp>
#include <sma/ccn/blockrequest.hpp>

#include <sma/ccn/contentcache.hpp>

#include <sma/util/hash.hpp>

#include <sma/util/event.hpp>

#include <sma/chrono.hpp>

#include <vector>
#include <deque>
#include <unordered_map>
#include <list>

#include <chrono>
#include <cstdlib>
#include <utility>
#include <memory>
#include <cstdint>

#include <jni.h>


namespace sma
{
struct CcnNode;

struct MessageHeader;
struct ContentAnn;
struct BlockResponse;
struct BlockRequestArgs;

class ContentCache;

//! Manages the metadata, data, and traffic for content items in the network.
/*! The content helper's responsibilities include publishing, storing,
 * segmenting, caching, and replicating content and its metadata.
 *
 * The helper does not manage interests as, though they do address the content,
 * they do so only by reference and have no relation to its properties or
 * behavior.
 */
class ContentHelperImpl : public ContentHelper
{
public:
  ContentHelperImpl(CcnNode& node, JavaVM * jvm);

  void receive(MessageHeader header, ContentAnn msg) override;
  void receive(MessageHeader header, BlockRequest req) override;
  void receive(MessageHeader header, BlockResponse resp) override;

  ContentMetadata create_new(std::vector<ContentType> types,
                             ContentName name,
                             void const* src,
                             std::size_t size) override;

  void fetch_content(std::string content_name) override;

  std::vector<ContentMetadata> metadata() override;
  std::vector<ContentMetadata> remote_metadata() override;
  std::vector<ContentMetadata> local_metadata() override;

  std::uint16_t announce_metadata() override;

  // WARNING: the on_block_arrived event is fired from this function.
  // If the handler for that event calls this function's caller the caller MUST
  // be reentrant!
  // For example, calling this while iterating a collection that is modified
  // by the caller may cause the caller's iterator to be invalidated.
  std::uint16_t request(std::vector<BlockRequestArgs> requests) override;
  void request_content(Hash hash, float utility, std::uint32_t ttl) override;
  bool broadcast(BlockRef block, std::uint16_t & bytes_sent) override;
  void handle_arrived_block (BlockRef block);
  void log_cache_utilization() const;

  std::size_t frozen(std::vector<BlockRef> const& blocks,
                     bool enabled) override;

  Event<ContentMetadata>& on_interesting_content() override;
  Event<NodeId, std::vector<BlockRequestArgs>>& on_blocks_requested() override;
  Event<BlockRef>& on_request_timeout() override;
  Event<NodeId, BlockRef>& on_block_arrived() override;
  Event<Hash>& on_content_complete() override;

  bool has_content_stored(Hash filename) const;

  float download_progress(ContentMetadata const & metadata) const;

private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;
  using millis = std::chrono::milliseconds;

  JavaVM* jvm;

  struct Metadata {
    Metadata(ContentMetadata data);

    void requested();
    void announced();

    ContentMetadata data;
    time_point last_requested;
    time_point next_announce;
  };

/*  struct RemoteMetadata {
    RemoteMetadata(ContentMetadata data);

    void announced();

    ContentMetadata data;
    time_point next_announce;
  };
*/

  struct PendingRequest {
    time_point requested;
    time_point expiry;
    bool keep_on_arrival;
    time_point half_expiry;
    float utility;
    Vec2d dest_position;
    bool repeated;
  };


  static constexpr auto default_initial_ttl = millis(10000);

  // Add, or update, the given remote metadata in the Remote Metadata Table.
  // Updating involves e.g. reflecting a nearer source for the content in the
  // hop count.
  // Return true if the metadata was added or updated and false if it exists
  // and was not changed.
  bool discover(NodeId sender, ContentMetadata meta);

  // If the `when` argument is provided, schedule this function's execution at
  // that future point. Otherwise scan the pending request table and invoke the
  // `on_request_timeout` event for any that exceed their expiry time (and
  // remove them from the table).
  void check_pending_requests(time_point when = time_point());

  void do_auto_fetch();

  void log_metadata(NodeId sender, ContentMetadata const& meta);

  bool test_content_complete(Hash content_name);
  bool request_again(BlockRef block);


  Event<ContentMetadata> interesting_content_event;
  Event<NodeId, std::vector<BlockRequestArgs>> blocks_requested_event;
  Event<BlockRef> request_timedout_event;
  Event<NodeId,BlockRef> block_arrived_event;
  Event<Hash> content_complete_event;

//  std::unique_ptr<ContentCache> cache;
  std::shared_ptr<ContentCache> cache;
//  std::unique_ptr<ContentCache> store;
  std::shared_ptr<ContentCache> store;
  std::deque<Hash> ann_queue;
  std::size_t to_announce;

  // Local Metadata Table
  // Content for which we have all of the data stored permanently.
  std::vector<Metadata> lmt;

  // Remote Metadata Table
  // Content we know of on the network, but for which we don't have a permanent
  // store.
  std::vector<Metadata> rmt;


  // Calculate Average Moving Delay
  std::list<millis> latencies_in_window;
  void consider_latency (millis latency);
  millis get_avg_latency();
  static constexpr size_t LATENCY_WIN_SZ = 10;

  static constexpr bool unicast_support = true;

  // Pending Request Table
  std::unordered_map<BlockRef, PendingRequest> prt;

  bool auto_announce = true;
  bool auto_fetch = false;
  bool auto_forward_requests = true;
  bool auto_respond = true;

  std::deque<BlockRef> auto_fetch_queue;

  static constexpr auto min_announce_interval = millis(1000);
  static constexpr auto cache_log_interval = millis(1000);
  static constexpr std::size_t fuzz_announce_min_ms = 0;
  static constexpr std::size_t fuzz_announce_max_ms = 500;

  // Detect reentrance to request() so we can warn the caller
  bool already_in_request{false};
};
}
