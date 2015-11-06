#pragma once
// Suppress "empty struct has size 0 in C, size 1 in C++"
#pragma GCC diagnostic ignored "-Wextern-c-compat"

#include <sma/linklayer.hpp>

#include <sma/component.hpp>
//#include <sma/io/log.hpp>

#include <string>
#include <vector>
#include <random>

namespace sma
{
class Async;

struct Context final {
  using prng_type = std::default_random_engine;

public:
  using prand_value = prng_type::result_type;

  Context(std::string name, LinkLayer& linklayer);
  Context(Context&& r);
  Context& operator=(Context&& r);

  void add_component(Component& c);
  template <typename T>
  T* try_get_component() const;

  prand_value rand() { return prng(); }

  prng_type prng;

//  Logger log;
  LinkLayer* linklayer;

private:
  std::vector<Component*> components;
};


template <typename T>
T* Context::try_get_component() const
{
/*
  static_assert(std::is_base_of<Component, T>::value,
                "Type must derive from Component");

  for (auto& c : components) {
    auto t = dynamic_cast<T*>(c);
    if (t)
      return t;
  }
  */
  return nullptr;
}
}
