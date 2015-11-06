#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/interestann.hpp>

#include <sma/ccn/ccnnode.hpp>

#include <sma/context.hpp>
#include <sma/async.hpp>

#include <sma/util/binaryformat.hpp>

#include <sma/stats.hpp>

#include <chrono>
#include <limits>
#include <sstream>
#include <iomanip>



namespace sma
{
constexpr std::chrono::milliseconds InterestHelperImpl::min_announce_interval;
constexpr std::size_t InterestHelperImpl::fuzz_announce_min_ms;
constexpr std::size_t InterestHelperImpl::fuzz_announce_max_ms;

constexpr std::chrono::seconds InterestHelperImpl::default_initial_ttl;


InterestHelperImpl::InterestHelperImpl(CcnNode& node)
  : InterestHelper(node)
  , next_announce_time(clock::now())
  , to_announce(0)
  , rng(std::random_device()())
{
  if (auto_announce)
    asynctask(&InterestHelperImpl::announce, this).do_in(min_announce_interval);
}

void InterestHelperImpl::receive(MessageHeader header, InterestAnn msg)
{
  std::vector<Interest> interests;
  interests.reserve(msg.count);
  Reader<BinaryInput> reader(msg.data, msg.size);
  for (std::size_t i = 0; i < msg.count; ++i)
    interests.emplace_back(reader);

  for (auto& interest : interests) {
    stats::Ints::on_received(node.id, interest);
    ++interest.hops;
    learn_remote(interest);
  }
}

void InterestHelperImpl::create_local(ContentType type)
{
  std::size_t idx = 0;
  for (auto it = interests.begin(); it != interests.end(); ++it, ++idx) {
    if (it->type == type) {
      if (it->local())
        return;

      interests.erase(it);
      // If we replaced a remote interest that was going to be announced then
      // don't count it as another interest to announce.
      if (idx < to_announce)
        --to_announce;

      break;
    }
  }

  interests.emplace_front(type, default_initial_ttl);
//log  log.d("I'm interested in %v! (and %v others)", type, to_announce);

  ++to_announce;
}

void InterestHelperImpl::create_local(std::vector<ContentType> types)
{
  for (auto& type : types)
    create_local(type);
}

void InterestHelperImpl::clear_local()
{
  auto it = interests.begin();
  while (it != interests.end())
    if (it->local())
      it = interests.erase(it);
    else
      ++it;
}

void InterestHelperImpl::delete_local(ContentType type)
{
  auto it = interests.begin();
  while (it != interests.end())
    if (it->type == type)
      interests.erase(it);
    else
      ++it;
}

std::vector<Interest> InterestHelperImpl::all() const
{
  std::vector<Interest> all;
  all.reserve(interests.size());
  for (auto& i : interests)
    all.emplace_back(i);
  return all;
}

std::vector<Interest> InterestHelperImpl::local() const
{
  std::vector<Interest> locals;
  for (auto& i : interests)
    if (i.local())
      locals.push_back(i);
  return locals;
}

std::vector<Interest> InterestHelperImpl::remote() const
{
  std::vector<Interest> remotes;
  for (auto& i : interests)
    if (i.remote())
      remotes.push_back(i);
  return remotes;
}

bool InterestHelperImpl::interested_in(ContentMetadata const& metadata) const
{
  for (auto& i : interests)
    if (i.local())
      for (auto& type : metadata.types)
        if (i.type == type)
          return true;
  return false;
}

void InterestHelperImpl::learn_remote(Interest const& interest)
{
  std::size_t idx = 0;
  for (auto it = interests.begin(); it != interests.end(); ++it, ++idx)
    if (it->type == interest.type) {
      if (it->update_with(interest)) {
        auto i = std::move(*it);
        interests.erase(it);
        interests.push_front(std::move(i));
        // Check if we promoted an already-announced element
        if (idx >= to_announce)
          ++to_announce;
      }
      return;
    }

  interests.emplace_front(
      interest.type, interest.ttl<std::chrono::milliseconds>(), interest.hops);
  ++to_announce;
}

bool InterestHelperImpl::know_remote(ContentType const& type) const
{
  for (auto& i : interests)
    if (i.remote() && i.type == type)
      return true;
  return false;
}

bool InterestHelperImpl::contains_any(
    std::vector<ContentType> const& types) const
{
  for (auto& i : interests)
    for (auto& type : types)
      if (i.type == type)
        return true;
  return false;
}

std::uint16_t InterestHelperImpl::announce()
{
  if (auto_announce)
    asynctask(&InterestHelperImpl::announce, this).do_in(min_announce_interval);

  if (clock::now() < next_announce_time)
    return 0;

  // Max size of one announcement message to be filled with interests
  std::size_t const max_size = 1024;
  // Serialize the interests into a contiguous block of stack to
  // a) avoid fragmenting allocations and b) provide a ready structure to
  // write the serialized interests into a message.
  std::uint8_t data_buf[max_size * 2];
  // Mark the start of the current interest's buffer space
  std::uint8_t* data = data_buf;
  // Total serialized length
  std::size_t size = 0;
  std::size_t count = 0;

  while (count < interests.size()) {
    auto& i = interests.front();
    i.elapse_ttl();
    if (i.expired()) {
      interests.pop_front();
      --to_announce;
      continue;
    }

    stats::Ints::on_sent(node.id, i);

    BinaryOutput out(data, sizeof(data_buf) - size);
    out << i;
    auto const wrote = out.size();
    // Don't overrun the message
    if (size + wrote > max_size)
      break;

    size += wrote;
    data += size;
    ++count;
    // Swap the interest to the end of the deque so others might be sent.
    interests.push_back(std::move(i));
    interests.pop_front();
    --to_announce;

    if (size == max_size)
      break;
  }

  if (to_announce == 0) {
    to_announce = interests.size();
    auto dist = std::uniform_int_distribution<>(fuzz_announce_min_ms,
                                                fuzz_announce_max_ms);
    next_announce_time = clock::now() + min_announce_interval
                         + std::chrono::milliseconds(dist(rng));
  }

  std::uint16_t bytes_sent = 0;
  if (count != 0) {
    // The Interest Announcement message just dumps the buffer into the
    // message data and reads it back out at the other end.
    InterestAnn msg(count, data_buf, size);
    bytes_sent = node.post(msg);
  }

//  return count;
  return bytes_sent;
}

void InterestHelperImpl::log_dump()
{
/*  if (not interests.empty()) {
    log.d("Interests");
    log.d("My interest table");
    for (auto const& i : local())
      log.d("| %v (local)", i.type);
    auto rit = remote();
    for (auto const& i : rit)
      log.d("| %v (%v hops)", i.type, std::uint32_t(i.hops));
  }

  asynctask(&InterestHelperImpl::log_dump, this).do_in(std::chrono::seconds(1));
 */
}
}
