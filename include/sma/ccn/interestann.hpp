#pragma once

#include <sma/nodeid.hpp>
#include <sma/ccn/interest.hpp>

#include <sma/util/serial.hpp>

#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>


namespace sma
{
struct InterestAnn {
  /****************************************************************************
   * Serialized Fields
   */
  std::uint8_t count;
  std::uint16_t size;
  std::uint8_t* data = nullptr;
  /***************************************************************************/

  InterestAnn(std::size_t count, std::uint8_t const* data, std::size_t size)
    : count(count)
    , size(size)
    , data(new std::uint8_t[size])
  {
    std::memcpy(this->data, data, size);
  }

  InterestAnn(InterestAnn&& rhs)
    : count(rhs.count)
    , size(rhs.size)
  {
    std::swap(data, rhs.data);
  }
  InterestAnn& operator=(InterestAnn&& rhs)
  {
    count = rhs.count;
    size = rhs.size;
    std::swap(data, rhs.data);
    return *this;
  }

  DESERIALIZING_CTOR(InterestAnn)
    : INIT_FIELDS(count, size)
    , data(new std::uint8_t[size])
  {
    GET_BYTES(data, size);
  }

  ~InterestAnn() { delete[] data; }

  SERIALIZER()
  {
    PUT_FIELDS(count, size);
    PUT_BYTES(data, size);
  }

  InterestAnn(InterestAnn const&) = delete;
  InterestAnn& operator=(InterestAnn const&) = delete;
};
}
