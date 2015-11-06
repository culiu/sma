#include <sma/context.hpp>
#include <sma/component.hpp>

namespace sma
{
Context::Context(std::string name, LinkLayer& linklayer)
//  : log(name)
  : linklayer(&linklayer)
  // prime for better randomization of lower IDs
  , prng(std::default_random_engine(std::hash<std::string>()(name)
                                    * 2305843009213693951L))
{
}
Context::Context(Context&& r)
//  : log(r.log)
  : linklayer(r.linklayer)
  , components(std::move(r.components))
{
}

Context& Context::operator=(Context&& r)
{
//  log = r.log;
  linklayer = r.linklayer;
  r.linklayer = nullptr;
  std::swap(components, r.components);
  return *this;
}

void Context::add_component(Component& c)
{
  /*for (auto& component : components)
    if (typeid(*component) == typeid(c))
      return;
  */

  components.push_back(&c);
}
}
