#pragma once

#include <sma/ccn/interesthelper.hpp>

#include <sma/ccn/ccnfwd.hpp>

#include <sma/ccn/interest.hpp>
#include <sma/ccn/interestrank.hpp>
#include <sma/ccn/remoteinterest.hpp>

#include <sma/chrono.hpp>

//#include <sma/io/log.hpp>

#include <cstdlib>
#include <random>

#include <map>
#include <deque>
#include <unordered_map>


namespace sma
{
//! Manages the storage, announcement, and replication of content interests.
/*! Consumers express interest in content in order to become targets of
 * dissemination from remote, unknown, providers.
 *
 * For provisioned content to reliably reach consumers a majority of nodes
 * should know generally what content are in demand; they can then make
 * intelligent forwarding and caching decisions to facilitate content where it's
 * wanted without completely flooding the network.
 *
 * While it does appear to be a different sort of flooding, the interest data
 * are intended to be extremely light weight relative to the content metadata
 * and should require a minimum of effort to handle.
 */
class InterestHelperImpl : public InterestHelper
{
public:
  InterestHelperImpl(CcnNode& node);

  void receive(MessageHeader header, InterestAnn announcement) override;

  void create_local(ContentType type) override;
  void create_local(std::vector<ContentType> types) override;
  void clear_local() override;

  void delete_local(ContentType type) override;

  std::vector<Interest> all() const override;
  std::vector<Interest> local() const override;
  std::vector<Interest> remote() const override;

  bool interested_in(ContentMetadata const& metadata) const override;
  bool know_remote(ContentType const& type) const override;
  bool contains_any(std::vector<ContentType> const& types) const override;

  std::uint16_t announce() override;

private:
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;

  void log_dump();

  void learn_remote(Interest const& interest);

  // Always ordered in decreasing time since last announced (never being
  // shortest).
  // On announcing, interests are selected off the head and replaced to the tail
  // until no more can fit in the announcement.
  std::deque<Interest> interests;

  time_point next_announce_time;
  std::size_t to_announce;

  bool auto_announce = false;

  static constexpr auto min_announce_interval = std::chrono::milliseconds(500);
  static constexpr std::size_t fuzz_announce_min_ms = 0;
  static constexpr std::size_t fuzz_announce_max_ms = 500;

  static constexpr auto default_initial_ttl = std::chrono::seconds(60);

  std::default_random_engine rng;
};
}
