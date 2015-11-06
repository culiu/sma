#include <sma/neighborhelper.hpp>
#include <sma/ccn/ccnnode.hpp>

namespace sma
{
NeighborHelper::NeighborHelper(CcnNode& node)
  : Helper(node)
{
  node.neighbors = this;
}
}
