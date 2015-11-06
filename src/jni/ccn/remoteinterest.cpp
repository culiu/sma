#include <sma/ccn/remoteinterest.hpp>

namespace sma
{
RemoteInterest::RemoteInterest()
{
  touch();
}

void RemoteInterest::touch() { last_seen = clock::now(); }

bool RemoteInterest::update()
{
  touch();
  return false;
}
}
