#pragma once

#include <sma/messagetypes.hpp>
#include <sma/messageheader.hpp>

#include <sma/util/binaryoutput.hpp>

#include <cstdlib>
#include <cstdint>

namespace sma
{
template <std::size_t Size>
struct MessageBuffer {
  template <typename M>
  MessageBuffer(MessageHeader const& header, M const& msg)
  {
    replace(header, msg);
  }

  template <typename M>
  void replace(MessageHeader const& header, M const& msg)
  {
    BinaryOutput out(data, sizeof data);
    // clang-format off
    out << header
        << MessageTypes::typecode<M>()
        << msg;
    // clang-format on
    sz = out.size();
  }

  std::uint8_t const* cdata() { return data; }
  std::size_t size() { return sz; }

private:
  std::uint8_t data[Size];
  std::size_t sz;
};
}
