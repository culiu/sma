#include <sma/ccn/contenthelper.hpp>
#include <sma/ccn/ccnnode.hpp>

namespace sma
{
ContentHelper::ContentHelper(CcnNode& node)
  : Helper(node)
{
  node.content = this;
}
}
