#pragma once
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/nodeid.hpp>

#include <sma/ccn/contenttype.hpp>
#include <sma/ccn/interest.hpp>

#include <sma/chrono.hpp>

#include <unordered_map>


namespace sma
{
namespace stats
{
  using clock = sma::chrono::system_clock;
  using time_point = clock::time_point;


  extern std::vector<NodeId> all_nodes;

  extern std::uint32_t time;

  void print_stats();

  struct Ints {
    static void on_sent(NodeId sender, Interest interest);
    static void on_received(NodeId recipient, Interest interest);
    static void print_received_prop();

    static std::unordered_map<ContentType, time_point> sent;
    static std::unordered_map<ContentType, std::vector<NodeId>> received;
  };


  struct Meta {
    static void on_sent(NodeId sender, ContentMetadata meta);
    static void on_received(NodeId recp, ContentMetadata meta);

    static std::unordered_map<Hash, time_point> sent;
    static std::unordered_map<Hash, std::vector<NodeId>> received;
  };
}
}
