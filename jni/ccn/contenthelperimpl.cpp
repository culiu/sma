#include <sma/ccn/contenthelperimpl.hpp>

#include <sma/neighbor.hpp>
#include <sma/ccn/ccnnode.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/contentname.hpp>

#include <sma/ccn/contentcache.hpp>
#include <sma/ccn/blockdata.hpp>

#include <sma/messageheader.hpp>
#include <sma/ccn/contentann.hpp>
#include <sma/ccn/blockresponse.hpp>

#include <sma/ccn/interesthelper.hpp>

#include <sma/async.hpp>

#include <ctime>
#include <string>


namespace sma
{
constexpr std::chrono::milliseconds ContentHelperImpl::min_announce_interval;
constexpr std::chrono::milliseconds ContentHelperImpl::cache_log_interval;
constexpr std::size_t ContentHelperImpl::fuzz_announce_min_ms;
constexpr std::size_t ContentHelperImpl::fuzz_announce_max_ms;

constexpr std::chrono::milliseconds ContentHelperImpl::default_initial_ttl;


ContentHelperImpl::ContentHelperImpl(CcnNode& node)
  : ContentHelper(node)
  , cache(new ContentCache(node, 256 * 1024, false))
  , store(new ContentCache(node))
  , to_announce(0)
{

//  asynctask(&ContentHelperImpl::log_cache_utilization,this)
//      .do_in(cache_log_interval);

  if (auto_announce)
    asynctask(&ContentHelperImpl::announce_metadata, this)
        .do_in(min_announce_interval);
}


ContentMetadata ContentHelperImpl::create_new(std::vector<ContentType> types,
                                              ContentName name,
                                              void const* src,
                                              std::size_t size)
{
//  auto hash = cache->store(src, size);
  auto hash = store->store(src, size);

  auto publish_time_ms = std::chrono::duration_cast<millis>(
                             clock::now().time_since_epoch()).count();

  auto metadata = ContentMetadata(hash,
                                  size,
                                  ContentCache::block_size,
                                  std::move(types),
                                  name,
                                  node.position(),
                                  node.id,
                                  publish_time_ms,
                                  default_initial_ttl);

  lmt.push_back(metadata);
  ann_queue.push_back(hash);
  ++to_announce;

  return metadata;
}

void ContentHelperImpl::log_cache_utilization() const
{
//  cache->log_utilization();
  asynctask(&ContentHelperImpl::log_cache_utilization,this)
      .do_in(cache_log_interval);
}

std::uint16_t ContentHelperImpl::announce_metadata()
{

//  node.log.d ("announcing metadata...");
  if ((lmt.empty() && rmt.empty()))
    return 0;

  std::vector<ContentMetadata> will_announce;
  will_announce.reserve(lmt.size() + rmt.size());

  std::size_t max_announce = 3;

  auto const now = clock::now();

  // Local metadata do not expire, but their announcement frequency is limited.
  // We announce them regardless of interests, but our neighbors are discerning.
  std::random_shuffle(lmt.begin(), lmt.end());
  for (auto& local : lmt) {
    if (now >= local.next_announce) {
      if (max_announce-- <= 0)
          break;
      will_announce.push_back(local.data);
      local.announced();
    }
  }

  // Remotes are not announced if it's been too long since they were received
  // from the original node; every intermediate node decays their TTL so they
  // expire across the entire network at once.
  std::random_shuffle(rmt.begin(), rmt.end());
  auto it = rmt.begin();
  bool delete_meta = true;
  while (it != rmt.end())
    if (it->data.expired()) {
      auto content_name = it->data.hash;
      auto prt_it = prt.begin();
      while (prt_it != prt.end()) {
        if (prt_it->first.hash == content_name) {
          delete_meta = false;
          break;
        }
        prt_it++;
      }
      if (delete_meta)
        it = rmt.erase(it);
      else
        ++it;
    }
    else {
      if (node.interests->contains_any(it->data.types) && 
              now >= it->next_announce) {
        if (max_announce-- <= 0)
          break;
        will_announce.push_back(it->data);
        it->announced();
      }
      ++it;
    }

  will_announce.shrink_to_fit();


  std::uint16_t bytes_sent = 0;

  if (not will_announce.empty()) {
     bytes_sent = node.post(ContentAnn(std::move(will_announce)));
  }

//  if (auto_announce)
//    asynctask(&ContentHelperImpl::announce_metadata, this)
//        .do_in(min_announce_interval);

  return bytes_sent;

//  return announced;
}


void ContentHelperImpl::receive(MessageHeader header, ContentAnn msg)
{
  for (auto metadata : msg.metadata) {
    // Break loops
    if (metadata.publisher == node.id)
      continue;

    // Count the link this came over

    ++metadata.hops;

    discover(header.sender, metadata);
  }
}


bool ContentHelperImpl::discover(NodeId sender, ContentMetadata meta)
{
  for (auto& existing : rmt)
    if (existing.data.hash == meta.hash) {
      if (meta.hops < existing.data.hops) {
        existing.data.hops = meta.hops;
        existing.data.from = sender;
        return true;
      } else
        return false;
    }
 
  for (auto& existing : lmt)
    if (existing.data.hash == meta.hash) {
      return false;
    }

  meta.from = sender;
  rmt.emplace_back(meta);
  auto current_time = sma::chrono::system_clock::now();
  auto after_publish = std::chrono::duration_cast<std::chrono::milliseconds>(current_time.time_since_epoch()).count() 
        - meta.publish_time_ms;
  int hops = meta.hops;
//  log.d ("Receive meta:  publisher %v hops %v after %v (ms): %v", meta.publisher, hops, after_publish);


  if (node.interests->contains_any(meta.types)) {
    interesting_content_event(meta);
    if (auto_fetch) {
      for (std::size_t i = 0; i < meta.block_count(); ++i)
        auto_fetch_queue.emplace_back(meta.hash, i);
      do_auto_fetch();
    }
  }

  return true;
}


void ContentHelperImpl::do_auto_fetch()
{
  if (auto_fetch_queue.empty())
    return;

  std::vector<BlockRequestArgs> reqs;

  for (std::size_t i = 0; i < 4 && !auto_fetch_queue.empty(); ++i) {
    auto block = auto_fetch_queue.front();
    auto it = rmt.begin();
    for (auto const& meta : rmt)
      if (meta.data.hash == block.hash) {
        reqs.emplace_back(block,
                          1.0,
                          default_initial_ttl,
                          node.id,
                          node.position(),
                          meta.data.hops,
                          true);
        break;
      }

    auto_fetch_queue.pop_front();
  }

//  log.d("Auto-fetching %v interesting blocks (%v more enqueued)",
//        reqs.size(),
//        auto_fetch_queue.size());

  request(std::move(reqs));

  if (not auto_fetch_queue.empty())
    asynctask(&ContentHelperImpl::do_auto_fetch, this).do_in(std::chrono::seconds(1));
}


std::uint16_t ContentHelperImpl::request(std::vector<BlockRequestArgs> requests)
{
  // Detect reentrance
/*  if (already_in_request)
    log.w("ContentHelperImpl::request has been reentered. Are you SURE the "
          "caller is reentrant?");
	  */
  already_in_request = true;

  std::vector<BlockRef> already_have;

  auto req = requests.begin();
  while (req != requests.end()) {
    // Skip a request if we have the block in cache or store.
    auto cached_block = cache->find(req->block);
    if (cached_block == cache->end())
      cached_block = store->find(req->block);

    auto const new_expiry = clock::now() + req->ttl<millis>();
    auto const half_expiry = clock::now() + req->ttl<millis>()/2;

    if ((cached_block != cache->end() || cached_block != store->end())
        && cached_block.complete()) {
      prt.erase(req->block);
     //Add a pending request to facilitate content complete
      //Otherwise, the content complete event will not be poped up.
      prt.emplace(
          req->block,
          PendingRequest{clock::now(), 
          new_expiry, 
          req->keep_on_arrival,
          half_expiry,
          req->utility,
          req->dest_position,
          false
          });
      check_pending_requests(half_expiry);

//      already_have.push_back(std::move(req->block));

      // WARNING: don not use move copy
      already_have.push_back(req->block);
      req = requests.erase(req);
 
      continue;
    }

    // directional forward
    // if from self, angle = nan
    
    Vec2d self_location = node.position();
//    log.d("self_location: %v %v", self_location.x, self_location.y);
    Vec2d neighbor_location 
        = node.neighbors->get_position (req->requester);
//    log.d("neighbor_location: %v %v", neighbor_location.x, neighbor_location.y);
    Vec2d dst_location = req->dest_position; 
//    log.d("dst_location: %v %v", dst_location.x, dst_location.y);

    double angle_value = Vec2d::angle (dst_location - neighbor_location, 
                self_location - neighbor_location); 
//    log.d ("angle_value: %v", angle_value);
    if (angle_value > 3.14/2) {
//      log.d (" not in the same direction: %v ", angle_value);
      requests.erase(req);
      continue;
    }


    auto it = prt.find(req->block);
    if (it != prt.end()) {
      // Avoid redudant request forwarding.
      requests.erase(req);
      auto& pending = it->second;
      // Update existing pending requests to have a longer TTL or keep the
      // requested block.
      if (pending.expiry < new_expiry) {
        pending.expiry = new_expiry;
        pending.half_expiry = half_expiry;
        check_pending_requests(half_expiry);
      }
      pending.keep_on_arrival |= req->keep_on_arrival;
    } else {
      // Add a new pending request to facilitate timeout and block storage.
      prt.emplace(
          req->block,
          PendingRequest{clock::now(), 
          new_expiry, 
          req->keep_on_arrival,
          half_expiry, 
          req->utility,
          req->dest_position,
          false});
      check_pending_requests(half_expiry);
      // IMPORTANT:
      // change requester to itself before transmitting out to next hop
      req->requester = node.id;
 
      ++req;
   }
  }

  std::uint16_t bytes_sent = 0;
  if (not requests.empty()) {
//    log.d("Handling %v requests", requests.size());
    for (auto& req : requests) {

      //FIX ME: aggregate requests destined to the same node
      NodeId to = 255;
      for (auto const & remote_meta : rmt) {
        if (remote_meta.data.hash == req.block.hash) {
            to = remote_meta.data.from; 
        }
      }

      if (unicast_support && to != 255) {
//        log.d("| request block:");
//       log.d("| %v %v", req.block.hash, req.block.index);
//        log.d("| from node: %v", to);
        // Only one node possible in the the recipient list
        bytes_sent += node.post(BlockRequest(std::vector<BlockRequestArgs>{req}),
                std::vector<NodeId>{to});
      } 
//      else {
//        log.d("| from all neighbors");
//        bytes_sent += node.post(BlockRequest(std::vector<BlockRequestArgs>{req}));
//      }
    }
//culiu    bytes_sent = node.post(BlockRequest(std::move(requests)));
  }

  // WARNING: caller must be reentrant if the callback invokes it!
  // For example: the caller enters this function while looping over a
  // collection that it modifies. If the block arrived handler reenters that
  // caller, the inner execution will modify the collection while the outer
  // execution is still iterating, invalidating its iterators.
  for (auto& block : already_have) {
//    log.d("block %v %v is locally stored/cached", block.hash, block.index);
    // check the pending request table to facilitate content complete.
    auto pending = prt.find(block);
    // There should be one pending request from the node itself.
    assert (pending != prt.end());

    // the block may exist only in the cache.
    // copy it from cache to store if keep_on_arrival = true
    // otherwise, store->validate will fail
    if (pending->second.keep_on_arrival
       and (store->find(block) == store->end())) {
      if (cache->find(block) != cache->end()) {
//        log.d("copying block %v %v from cache to store", block.hash, block.index); 
        ContentCache::copy_block_from_to(*cache, *store, block);
        // unfreeze the cache if frozen
        frozen({block}, false);
      }
    } else if ((store->find(block) != store->end())
            and (cache->find(block) == cache->end())) {
//      log.d("copying block %v %v from store to cache", block.hash, block.index);
      ContentCache::copy_block_from_to(*store, *cache, block);
    }

    auto it = rmt.begin();
    while (it != rmt.end()) {
//    for (auto const& meta : rmt)
      if (it->data.hash == block.hash) {
        if (store->validate_data(it->data)) {
          content_complete_event(block.hash); 
          lmt.push_back(*it);
          rmt.erase(it);
        } else
          it++;
      } else
          it++;
    }
    block_arrived_event(node.id, block);
  }

  already_in_request = false;
  
  return bytes_sent;
}

void ContentHelperImpl::request_content (Hash content_name, 
                                         float utility_per_block, 
                                         std::uint32_t ttl)
{
  // check local meta
  for (auto const & local_meta : lmt) {
    if (local_meta.data.hash == content_name) {
//      node.log.i ("Content %v is self published.", content_name);
      content_complete_event(content_name);
      return; // more complicated function can be added, e.g., return data
    }
  }
  
  Vec2d dest_position; // will be updated;
  // check remote meta
  int num_of_blocks = 0;

  for (auto const & remote_meta : rmt) {
    if (remote_meta.data.hash == content_name) {
      num_of_blocks = 1 + (remote_meta.data.size-1)/(remote_meta.data.block_size);
      dest_position = remote_meta.data.origin;
    }
  }

  assert (num_of_blocks != 0); // disable for real implementation
  if (num_of_blocks == 0)
    return;

  int* shuffle_idx_arr = new int [num_of_blocks];
  //shuffle
  for (std::size_t i=0; i<num_of_blocks; i++)
    shuffle_idx_arr[i] = i;

  std::random_shuffle (shuffle_idx_arr, shuffle_idx_arr + num_of_blocks);
  std::vector<BlockRequestArgs> requests;
  for (std::size_t i=0; i<num_of_blocks; i++) {
    BlockRef block (content_name, shuffle_idx_arr[i]);
    auto cached_block = cache->find(block);

    if (cached_block == cache->end())
      cached_block = store->find(block);

    if (cached_block != cache->end() || cached_block != store->end()
        && cached_block.complete()){
      continue;
    }

    requests.push_back (BlockRequestArgs(BlockRef(content_name, shuffle_idx_arr[i]),
                                         utility_per_block,
                                         std::chrono::milliseconds(ttl),
                                         node.id,
                               //          node.position(),
                                         dest_position,
                                         0,
                                         true));
  }

  delete[] shuffle_idx_arr;
  if (requests.size() > 0)
    this->request (requests); 
  else
    content_complete_event(content_name);
}

void ContentHelperImpl::receive(MessageHeader header, BlockRequest msg)
{
//  log.d("Got %v requests from %v", msg.requests.size(), header.sender);

  auto reqs = std::move(msg.requests);
  auto reqIt = reqs.begin();
  while (reqIt != reqs.end()) {
    auto it = prt.find(reqIt->block);
    if (it != prt.end())
      reqs.erase(reqIt);
    else
      reqIt++;
  }

//  if (msg.requests.empty())
  if (reqs.empty())
    return;

  blocks_requested_event(header.sender, std::move(reqs));

  if (not auto_respond and not auto_forward_requests)
    return;

/*  for (auto const& req : msg.requests) {
    if (auto_respond) {
      broadcast(req.block);

    } else if (auto_forward_requests) {
      auto prt_search = prt.find(req.block);
      if (prt_search != prt.end()) {
        auto& pending = prt_search->second;
        pending.local_only = false;
        auto new_expiry = clock::now() + req.ttl<millis>();
        if (pending.expiry < new_expiry) {
          pending.expiry = new_expiry;
          check_pending_requests(new_expiry);
        }
      }
    }
  }
  */
}


bool ContentHelperImpl::broadcast(BlockRef ref, std::uint16_t & bytes_sent)
{
  ContentCache* source = nullptr;
  auto block = cache->find(ref);
  if (block == cache->end()) {
    block = store->find(ref);
    if (block == store->end()) {
//      bytes_sent = 0;
      return false;
    }
  }

//  log.d("Send block");
//  log.d("| %v", ref);
//  log.d("| size: %v bytes", block.size());
//  log.d("");

  bytes_sent += node.post(BlockResponse(ref, block.data(), block.size()));

  return true;
}


void ContentHelperImpl::receive(MessageHeader header, BlockResponse msg)
{
//  log.d("Got block from node %v", header.sender);
//  log.d("| %v", msg.block);
  

  bool is_stored = false;
  bool will_rebroadcast = false;
  bool meta_missing = true;

  // Skip the cache if we're expecting to keep all this content's data.
  auto pending = prt.find(msg.block);
  if (pending != prt.end()) {
    if (pending->second.keep_on_arrival
        and (store->find(msg.block) == store->end())) {
//      log.d("| I'll store this permanently");
      auto latency = clock::now() - pending->second.requested;
      auto latency_in_ms = std::chrono::duration_cast<millis>
          (latency);
      consider_latency (latency_in_ms);
//      log.d("block latency : %v", latency_in_ms.count());

      if (store->store(msg.block, msg.size, msg.data, msg.size)
              !=store->end())
      for (auto const& meta : rmt)
        if (meta.data.hash == msg.block.hash) {
          meta_missing = false;
          if (store->validate_data(meta.data)) {
//            log.i("Content %v complete at %v",
//                  msg.block.hash,
//                  std::chrono::duration_cast<millis>(
//                      clock::now().time_since_epoch()).count());
            content_complete_event(msg.block.hash);
          }
          break;
        }
 //     if (meta_missing)
 //       node.log.d("meta data is missing for %v", msg.block.hash);
      is_stored = true;
    }

//    will_rebroadcast = auto_respond and not pending->second.local_only;
  }

  is_stored |= (store->find(msg.block) != store->end());

  // Cache all blocks we come across
  if (not is_stored and (cache->find(msg.block) == cache->end())) {
    // routing-based; should remove opportunistically caching.
    cache->store(msg.block, msg.size, msg.data, msg.size);
//    log.i ("cached block %v %v opportunistically", msg.block.hash, msg.block.index);
  }


  block_arrived_event(header.sender, msg.block);

//  if (will_rebroadcast) {
//    log.d("| I'll rebroadcast it, too");
//    broadcast(msg.block);
//  }
}

void ContentHelperImpl::consider_latency(millis latency)
{
  if (latencies_in_window.size() ==  LATENCY_WIN_SZ)
    latencies_in_window.pop_front();

  latencies_in_window.push_back(latency);
}

using millis = std::chrono::milliseconds;
millis ContentHelperImpl::get_avg_latency()
{
  millis sum{0};
  for (auto const & latency : latencies_in_window)
    sum += latency;

  if (latencies_in_window.empty())
    return sum;

  return sum / latencies_in_window.size();
}


void ContentHelperImpl::check_pending_requests(time_point when)
{
  if (when != time_point()) {
    asynctask(&ContentHelperImpl::check_pending_requests, this, time_point())
        .do_in(when - clock::now());
    return;
  }

  std::vector<BlockRef> timed_out;

  auto const now = clock::now();
  auto it = prt.begin();
  while (it != prt.end()) {
    auto const& pending = it->second;
    if (now >= it->second.expiry) {
      timed_out.push_back(it->first);

      // Only Those Timeoutd Local Request Will be Considered to Latency
      if (it->second.keep_on_arrival) {
        consider_latency (std::chrono::duration_cast<millis>(now 
                  - it->second.requested));
      }
      it = prt.erase(it);
    } else {
        
      // Repeat request is a plain broadcast
      if (!it->second.repeated 
              && now >= it->second.half_expiry 
              && it->second.keep_on_arrival) {

        auto cached_block = cache->find(it->first);
        if (cached_block == cache->end())
          cached_block = store->find(it->first);

        if (cached_block == cache->end()
                && cached_block == store->end()) {
//          node.log.d ("re issue request for block:");
//          node.log.d ("| %v %v", it->first.hash, it->first.index);
//          log.d("| from all neighbors");

          node.post(BlockRequest(std::vector<BlockRequestArgs>{
            BlockRequestArgs(
            it->first,
            it->second.utility,
            std::chrono::duration_cast<millis>(it->second.expiry - now),
            node.id,
            it->second.dest_position,
            0,
            true)}));

            it->second.repeated = true;
        }
      }

  
      ++it;
    }
  }

  for (auto& block : timed_out)
    request_timedout_event(block);
}


std::size_t ContentHelperImpl::frozen(std::vector<BlockRef> const& blocks,
                                      bool enabled)
{
  std::size_t count = 0;
  for (auto const& ref : blocks) {
    auto block = cache->find(ref);
    if (block != cache->end()) {
      block.frozen(enabled);
      ++count;
    }
  }
  
  if( count > 0)
    log_cache_utilization();

  return count;
}


std::vector<ContentMetadata> ContentHelperImpl::metadata()
{
  std::vector<ContentMetadata> v;
  v.reserve(lmt.size() + rmt.size());

  for (auto const& local : lmt)
    v.push_back(local.data);

  auto it = rmt.begin();
  while (it != rmt.end()) {
    if (!it->data.expired())
//      it = rmt.erase(it);
//    else
      v.push_back(it->data);

    it++;
  }

  v.shrink_to_fit();
  return v;
}

std::vector<ContentMetadata> ContentHelperImpl::remote_metadata()
{
  std::vector<ContentMetadata> v;
  v.reserve(rmt.size());

  auto it = rmt.begin();
  while (it != rmt.end()) {
    if (!it->data.expired())
//      it = rmt.erase(it);
//    else
      v.push_back(it->data);

    it++;
  }

  v.shrink_to_fit();
  return v;
}


void ContentHelperImpl::log_metadata(NodeId sender, ContentMetadata const& meta)
{
/*  log.d("Content metadata from n(%v)", sender);
  log.d("| distance: %v hop(s)", std::uint32_t(meta.hops));
  log.d("| ttl: %v ms", meta.ttl<millis>().count());
  log.d("| hash: %v", meta.hash);
  log.d("| size: %v bytes", meta.size);
  log.d("| block size: %v bytes", meta.block_size);
  for (auto& type : meta.types)
    log.d("| type: %v", type);
  log.d("| name: %v", meta.name);
  log.d("| publisher: %v", meta.publisher);
  log.d("| publish time: %v ms", meta.publish_time_ms);
  log.d("| publish location: %v", meta.origin);
*/
}

Event<ContentMetadata>& ContentHelperImpl::on_interesting_content()
{
  return interesting_content_event;
}

Event<NodeId, std::vector<BlockRequestArgs>>&
ContentHelperImpl::on_blocks_requested()
{
  return blocks_requested_event;
}

Event<BlockRef>& ContentHelperImpl::on_request_timeout()
{
  return request_timedout_event;
}

Event<NodeId, BlockRef>& ContentHelperImpl::on_block_arrived()
{
  return block_arrived_event;
}

Event<Hash>& ContentHelperImpl::on_content_complete()
{
  return content_complete_event;
}


ContentHelperImpl::Metadata::Metadata(ContentMetadata data)
  : data(data)
  , last_requested(clock::now())
  , next_announce(clock::now())
{
}


void ContentHelperImpl::Metadata::requested()
{
  last_requested = clock::now();
}


void ContentHelperImpl::Metadata::announced()
{
  // 2^t growth
  auto const min = std::chrono::seconds(1);
  auto const max = std::chrono::seconds(60);

  auto const now = clock::now();
  auto delay = now - last_requested;
  if (delay > max)
    delay = max;
  if (delay < min)
    delay = min;

//  next_announce = now + delay;
  next_announce = clock::now() + 2 * default_initial_ttl;
}


//ContentHelperImpl::RemoteMetadata::RemoteMetadata(ContentMetadata data)
//  : data(data)
//  , next_announce(clock::now())
//{
//}

//void ContentHelperImpl::RemoteMetadata::announced()
//{
//  next_announce = clock::now() + 2 * default_initial_ttl;
//}

bool ContentHelperImpl::has_content_stored(Hash filename) const
{
  return store->has_content_stored(filename);
}
}
