#pragma once

#include <sma/nodeid.hpp>

#include <sma/util/serial.hpp>

#include <vector>

namespace sma
{
struct MessageHeader {
  TRIVIALLY_SERIALIZABLE(MessageHeader, sender, recipients)

  NodeId sender;
  std::vector<NodeId> recipients;

  MessageHeader(NodeId sender)
    : sender(sender)
  {
  }

  MessageHeader(NodeId sender, std::vector<NodeId> recipients)
    : sender(sender)
    , recipients(std::move(recipients))
  {
  }
};
}
