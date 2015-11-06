#pragma once

#include <cstdlib>

namespace sma
{
class LinkLayer;

class Link
{
public:
  Link() {}
  Link(Link const&) = delete;
  Link& operator=(Link const&) = delete;

  virtual ~Link() {}

  void receive_to(LinkLayer& ll) { linklayer = &ll; }

  bool readable();

  virtual std::size_t read(void* dst, std::size_t size) = 0;
  virtual std::size_t write(void const* src, std::size_t size) = 0;

  virtual void close() = 0;

protected:
  void readable(bool r);

private:
  LinkLayer* linklayer = nullptr;
  bool is_readable = false;
};
}
