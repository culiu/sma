#pragma once

#include <sma/util/hash.hpp>

#include <sma/util/serial.hpp>

#include <cstdint>
#include <cstring>


namespace sma
{
struct BlockResponse {
  BlockRef block;
  std::uint32_t size;
  std::uint8_t* data = nullptr;

  BlockResponse(BlockRef block, std::uint8_t const* src, std::size_t size)
    : block(block)
    , size(size)
    , data(new std::uint8_t[size])
  {
    std::memcpy(data, src, size);
  }

  BlockResponse(BlockResponse&& rhs)
    : block(rhs.block)
    , size(rhs.size)
    , data(rhs.data)
  {
    rhs.data = nullptr;
  }

  BlockResponse& operator=(BlockResponse&& rhs)
  {
    block = rhs.block;
    size = rhs.size;
    data = rhs.data;
    rhs.data = nullptr;
    return *this;
  }

  ~BlockResponse() { delete[] data; }

  DESERIALIZING_CTOR(BlockResponse)
    : INIT_FIELDS(block, size)
    , data(new std::uint8_t[size])
  {
    GET_BYTES(data, size);
  }

  SERIALIZER()
  {
    PUT_FIELDS(block, size);
    PUT_BYTES(data, size);
  }

  BlockResponse(BlockResponse const&) = delete;
  BlockResponse& operator=(BlockResponse const&) = delete;
};
}
