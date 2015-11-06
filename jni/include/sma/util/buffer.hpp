#pragma once

#include <sma/util/serial.hpp>

#include <cstdint>
#include <memory>
#include <limits>
#include <cstring>
#include <cassert>

namespace sma
{
template <typename SizeT>
struct Buffer {
  Buffer() {}
  Buffer(std::size_t size);
  Buffer(std::uint8_t const* data, std::size_t size);

  Buffer(Buffer&& r);
  Buffer(Buffer const& r);
  Buffer& operator=(Buffer&& r);
  Buffer& operator=(Buffer const& r);

  DESERIALIZING_CTOR(Buffer)
    : INIT_FIELD_AS(sz, SizeT)
  {
    data = std::make_unique<std::uint8_t[]>(sz);
    if (sz != 0)
      GET_BYTES(data.get(), sz);
  }

  SERIALIZER()
  {
    PUT_FIELD(SizeT(sz));
    if (sz != 0)
      PUT_BYTES(data.get(), sz);
  }


  std::size_t size() const { return sz; }
  std::uint8_t const* cdata() const { return data.get(); }

private:
  std::size_t sz{0};
  std::unique_ptr<std::uint8_t[]> data;
};


template <typename SizeT>
Buffer<SizeT>::Buffer(std::size_t size)
  : data(std::make_unique<std::uint8_t[]>(size))
  , sz(size)
{
  assert(sz <= std::numeric_limits<SizeT>::max());
}

template <typename SizeT>
Buffer<SizeT>::Buffer(std::uint8_t const* src, std::size_t size)
  : Buffer(size)
{
  std::memcpy(data.get(), src, size);
}

template <typename SizeT>
Buffer<SizeT>::Buffer(Buffer const& r)
  : Buffer(r.sz)
{
  std::memcpy(data.get(), r.data.get(), sz);
}
template <typename SizeT>
Buffer<SizeT>& Buffer<SizeT>::operator=(Buffer const& r)
{
  sz = r.sz;
  data = std::make_unique<std::uint8_t[]>(r.sz);
  std::memcpy(data.get(), r.data.get(), sz);
}

template <typename SizeT>
Buffer<SizeT>::Buffer(Buffer&& r)
  : data(std::move(r.data))
  , sz(r.sz)
{
}

template <typename SizeT>
Buffer<SizeT>& Buffer<SizeT>::operator=(Buffer&& r)
{
  sz = r.sz;
  std::swap(data, r.data);

  return *this;
}
}
