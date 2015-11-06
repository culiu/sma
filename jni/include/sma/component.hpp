#pragma once

namespace sma
{
class Component
{
public:
  virtual ~Component() = 0;
};

inline Component::~Component() {}
}
