#include <sma/forwardstrategy.hpp>
#include <sma/linklayer.hpp>

namespace sma
{
  ForwardStrategy::ForwardStrategy(LinkLayer& llayer)
    : llayer(&llayer)
  {
    llayer.forward_strategy(*this);
  }
}
