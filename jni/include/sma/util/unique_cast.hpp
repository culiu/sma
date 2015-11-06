#pragma once

#include <memory>
#include <utility>


namespace sma
{
template <typename D, typename B, typename Del>
std::unique_ptr<D, Del> static_unique_cast(std::unique_ptr<B, Del>&& p)
{
  auto d = static_cast<D*>(p.release());
  return std::unique_ptr<D, Del>(d, std::move(p.get_deleter()));
}

template<typename D, typename B, typename Del>
std::unique_ptr<D, Del> dynamic_unique_cast(std::unique_ptr<B, Del>&& p)
{
  if (D* d = dynamic_cast<D*>(p.get())) {
    p.release();
    return std::unique_ptr<D, Del>(d, std::move(p.get_deleter()));
  }
  return std::unique_ptr<D, Del>(nullptr, p.get_deleter());
}
}

