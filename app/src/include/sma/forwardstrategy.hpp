#pragma once

namespace sma
{
class LinkLayer;

class ForwardStrategy
{
public:
  ForwardStrategy(LinkLayer& llayer);

  virtual ~ForwardStrategy() {}

  virtual void notify() = 0;

protected:
  LinkLayer* llayer;
};
}
