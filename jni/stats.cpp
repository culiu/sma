#include <sma/stats.hpp>

#include <sma/async.hpp>
//#include <sma/io/log.hpp>

#include <chrono>


namespace sma
{
namespace stats
{
  std::uint32_t time = 0;

  void print_stats()
  {
    //Ints::print_received_prop();

    asynctask(print_stats).do_in(std::chrono::seconds(1));
  }

  ///
  /// Nodes
  ///
  std::vector<NodeId> all_nodes;

  ///
  /// Interests
  ///
  std::unordered_map<ContentType, time_point> Ints::sent;
  std::unordered_map<ContentType, std::vector<NodeId>> Ints::received;

  void Ints::on_sent(NodeId sender, Interest interest)
  {
    auto it = sent.find(interest.type);
    if (it == sent.end()) {
      sent.emplace(interest.type, clock::now());
 //     LOG(INFO) << "Interest sent, " << sender << ", " << interest.type;
    }
  }

  void Ints::on_received(NodeId recipient, Interest interest)
  {
    auto& recps = received[interest.type];
    for (auto& recp : recps)
      if (recp == recipient)
        return;
    recps.push_back(recipient);

#if 0
    auto sent_time = sent[interest.type];

    LOG(INFO) << "Interest received, " << recipient << ", " << interest.type
              << ", "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     clock::now() - sent_time).count() << ", "
              << (double(recps.size()) / all_nodes.size());
#endif
  }

  void Ints::print_received_prop()
  {
    double prop_sum = 0.0;

    for (auto& sent_int : Ints::sent) {
      auto& recvd = Ints::received[sent_int.first];
      prop_sum += (double(recvd.size()) / all_nodes.size());
    }

    if (not Ints::sent.empty())
      prop_sum /= Ints::sent.size();

//    LOG(INFO) << time++ << ", " << prop_sum;
  }


  ///
  /// Metadata
  ///
  std::unordered_map<Hash, time_point> Meta::sent;
  std::unordered_map<Hash, std::vector<NodeId>> Meta::received;

  void Meta::on_sent(NodeId sender, ContentMetadata meta)
  {
    auto it = sent.find(meta.hash);
    if (it == sent.end()) {
      sent.emplace(meta.hash, clock::now());
//      LOG(INFO) << "Metadata sent, " << sender << ", " << meta.hash;
    }
  }

  void Meta::on_received(NodeId recp, ContentMetadata meta)
  {
    auto& recps = received[meta.hash];
    for (auto& r : recps)
      if (r == recp)
        return;
    recps.push_back(recp);

    auto sent_time = sent[meta.hash];

//    LOG(INFO) << "Metadata received, " << recp << ", " << meta.types[0] << ", "
//              << std::chrono::duration_cast<std::chrono::milliseconds>(
 //                    clock::now() - sent_time).count();

  }
}
}
