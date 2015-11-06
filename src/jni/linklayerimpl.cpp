#include <sma/linklayerimpl.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/messageheader.hpp>
#include <sma/messagetypes.hpp>

#include <sma/forwardstrategy.hpp>

#include <sma/util/reader.hpp>
#include <sma/util/binaryinput.hpp>

//#include <sma/io/log.hpp>
#include <sma/chrono.hpp>

#include <cstring>
#include <cassert>

#include <unordered_map>
#include <utility>

#include <android/log.h>


namespace sma
{
using clock = sma::chrono::system_clock;

// FIXME: Replace with real statistics tracking
#ifdef TRACK_PACKETS
std::unordered_map<std::uint64_t, std::uint64_t> packets;
static std::uint64_t g_largest = 0;
static std::uint64_t g_lost = 0;
static std::uint64_t g_bytes_out = 0;
static std::uint64_t g_bytes_in = 0;

static std::uint64_t g_next_send = 0;
static std::uint64_t g_bytes_frame = 0;
static double g_bps = 0.0;
static clock::time_point g_frame_start = clock::now();
#endif


struct Unmarshal {
  static Unmarshal instance;

  template <typename M, typename Reader>
  void operator()(MessageHeader&& header, Reader& reader, CcnNode& node)
  {
    node.receive(std::move(header), M(reader));
  }
};
// Avoid constructing it for every message
Unmarshal Unmarshal::instance;

LinkLayerImpl::LinkLayerImpl(std::vector<std::unique_ptr<Link>> links)
  : links(std::move(links))
  , send_buf(16)
{
  assert(!this->links.empty());
  for (auto& link : this->links)
    link->receive_to(*this);
}

void LinkLayerImpl::stop() { node = nullptr; }

void LinkLayerImpl::enqueue(void const* src, std::size_t size)
{
  /*
  {
    auto buf = send_buf.acquire();
    assert(size <= buf.capacity());
    if (size > g_largest)
      g_largest = size;
    std::memcpy(buf.data, src, size);
    *buf.size = size;
  }

  if (fwd_strat)
    fwd_strat->notify();
  else
    forward_one();
    */
  for (auto& link : links)
    if (link->write(src, size) == 0)
      assert(0 && "Wrote no bytes to socket");
    else {
      std::stringstream oss;
      oss << "wrote " << size << " bytes";
      __android_log_write(ANDROID_LOG_INFO,"LinklayerImpl", oss.str().c_str());
    }
}

std::size_t LinkLayerImpl::forward_one()
{
#ifdef TRACK_PACKETS
  {
    auto lock = send_buf.try_pop();
    if (!lock.acquired())
      return 0;

    auto frame_time = clock::now() - g_frame_start;
    if (frame_time >= std::chrono::seconds(1)) {
      g_bps = 0.5
              * (g_bps + (g_bytes_frame
                          / std::chrono::duration_cast<std::chrono::seconds>(
                                frame_time).count()));
      g_bytes_frame = 0;
      g_frame_start = clock::now();
      std::uint64_t lost = 0;
      for (auto it : packets)
        lost += it.second;
      LOG(DEBUG) << g_bps << " Bps (" << g_bytes_out << " out, " << g_bytes_in
                 << " in, " << g_largest << " max, " << lost << " lost ("
                 << (100.0 * (double(lost) / g_bytes_out)) << "%))";
    }

    char buf[BUFFER_SIZE];
    std::memcpy(buf, &g_next_send, sizeof(std::uint64_t));
    std::memcpy(buf + sizeof(std::uint64_t), lock.cdata(), lock.size());
    g_bytes_out += lock.size();
    packets.emplace(g_next_send++, lock.size());
    ++g_next_send;

    for (auto& link : links)
      if (link->write(buf, sizeof(std::uint64_t) + lock.size()) == 0)
        assert(0 && "Wrote no bytes to socket");
  }

  return send_buf.size();
#endif
  return 0;
}

void LinkLayerImpl::on_link_readable(Link& link)
{

//  __android_log_write(ANDROID_LOG_INFO, "linklayerimpl", "on_link_readable");
  std::uint8_t buf[BUFFER_SIZE];
  std::size_t read;
  while (node && (read = link.read(buf, BUFFER_SIZE))) {

    Reader<BinaryInput> reader(buf, read);
#ifdef TRACK_PACKETS
    // Consume the hacked in packet sequence
    reader.template get<std::uint64_t>();
    // Read the packet seq
    std::uint64_t packetseq;
    std::memcpy(&packetseq, buf, sizeof(std::uint64_t));

    // Count the packet as received
    if (packets.erase(packetseq) != 0) {
      g_bytes_frame += read - 8;
      g_bytes_in += read - 8;
    }
#endif

    // Read the actual message from the packet
    MessageHeader header(reader);

    // Reject broadcast loopbacks
    if (header.sender == node->id)
       continue;


    // Reject if addressed and not to me
    if (!header.recipients.empty()) {
      auto const& this_node = node->id;
      auto const nrecp = header.recipients.size();
      decltype(header.recipients)::size_type i = 0;
      while (i++ < nrecp)
        if (header.recipients[i] == this_node)
          break;
      if (i == nrecp)
        continue;
    }

    auto msg_typecode = reader.template get<MessageTypes::typecode_type>();
    // Deduce the type of the message from its typecode and call the unmarshal
    // operator to deserialize a message of that type and pass it to the node.
    assert(node);
    if (!MessageTypes::apply(msg_typecode,
                             Unmarshal::instance,
                             std::move(header),
                             reader,
                             *node))
//      node->log.w("Unhandled message");
      __android_log_write(ANDROID_LOG_INFO, "linklayerimpl", "unhandled message");
//    else
//      __android_log_write(ANDROID_LOG_INFO, "linklayerimpl", "message handled");

  }
}
}
