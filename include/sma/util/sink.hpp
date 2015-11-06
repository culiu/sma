#pragma once

#include <vector>
#include <utility>


namespace sma
{
template <typename T>
class Sink
{
public:
  virtual ~Sink() {}

  virtual void accept(T t) = 0;
};


template <typename T>
class SinkSet
{
  using Myt = SinkSet<T>;
  using raw_ptr = Sink<T>*;
  using sinks_type = std::vector<raw_ptr>;
  using iterator = typename sinks_type::iterator;
  using const_iterator = typename sinks_type::const_iterator;

public:
  Myt& add(raw_ptr sink);
  Myt& remove(raw_ptr sink);

  Myt& operator+=(raw_ptr sink) { return add(std::move(sink)); }
  Myt& operator-=(raw_ptr sink) { return remove(std::move(sink)); }

  bool empty() const noexcept { return sinks.empty(); }

  iterator begin() noexcept { return sinks.begin(); }
  iterator end() noexcept { return sinks.end(); }
  const_iterator cbegin() noexcept { return sinks.cbegin(); }
  const_iterator cend() noexcept { return sinks.cend(); }

private:
  sinks_type sinks;
};


template <typename T>
SinkSet<T>& SinkSet<T>::add(raw_ptr sink)
{
  for (auto& s : sinks)
    if (s == sink)
      return *this;
  sinks.push_back(std::move(sink));
  return *this;
}

template <typename T>
SinkSet<T>& SinkSet<T>::remove(raw_ptr sink)
{
  auto it = sinks.begin();
  while (it != sinks.end())
    if (*it == sink)
      sinks.erase(it++);
    else
      ++it;
  return *this;
}
}
