#pragma once

#include <sma/ccn/blockrequestargs.hpp>

#include <sma/util/serial.hpp>

#include <cstdint>


namespace sma
{
struct BlockFragmentRequest {
  TRIVIALLY_SERIALIZABLE(BlockFragmentRequest, offset, size)

  std::uint32_t offset;
  std::uint32_t size;

  BlockFragmentRequest(std::uint32_t offset, std::uint32_t size)
    : offset(offset)
    , size(size)
  {
  }

  BlockFragmentRequest(BlockFragmentRequest&&) = default;
  BlockFragmentRequest(BlockFragmentRequest const&) = default;

  BlockFragmentRequest& operator=(BlockFragmentRequest&&) = default;
  BlockFragmentRequest& operator=(BlockFragmentRequest const&) = default;
};


struct BlockRequest {
  TRIVIALLY_SERIALIZABLE(BlockRequest, requests)

  std::vector<BlockRequestArgs> requests;


  BlockRequest(std::vector<BlockRequestArgs> requests)
    : requests(std::move(requests))
  {
  }

  BlockRequest(BlockRequest&&) = default;
  BlockRequest(BlockRequest const&) = default;

  BlockRequest& operator=(BlockRequest&&) = default;
  BlockRequest& operator=(BlockRequest const&) = default;
};
}
