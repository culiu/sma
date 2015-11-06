#include <sma/link.hpp>
#include <sma/linklayer.hpp>


namespace sma
{
void Link::readable(bool r)
{
  if ((is_readable = r) && linklayer)
    linklayer->on_link_readable(*this);
}

bool Link::readable() { return is_readable; }
}
