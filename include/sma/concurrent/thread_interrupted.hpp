#pragma once

#include <exception>

namespace sma
{

class thread_interrupted final : public std::exception {};

}