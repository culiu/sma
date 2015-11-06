#include <sma/ccn/interesthelper.hpp>
#include <sma/ccn/ccnnode.hpp>

namespace sma
{
InterestHelper::InterestHelper(CcnNode& node)
  : Helper(node)
{
  node.interests = this;
}
}
