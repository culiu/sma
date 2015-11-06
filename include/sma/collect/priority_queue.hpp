#pragma once

#include <cstdlib>
#include <vector>
#include <mutex>
#include <utility>
#include <type_traits>
#include <functional>



namespace sma
{

template <typename T, typename Compare = std::less<T>>
class PriorityQueue
{
  static_assert(std::is_nothrow_move_constructible<T>::value,
                "T should not throw exceptions in its move constructor.");

public:
  PriorityQueue()
    : better()
  {
  }

  void push(T t)
  {
    // Start on the tail and swap with our parent until she's better than us
    v.push_back(std::move(t));
    std::size_t i = v.size() - 1;
    std::size_t p_i = (i + 1) / 2 - 1;
    while (i > 0 && better(v[i], v[p_i])) {
      std::swap(v[i], v[p_i]);
      i = p_i;
      p_i = (i + 1) / 2 - 1;
    }
  }

  const T& top()
  {
    return v[0];
  }

  T pop()
  {
    T popped = std::move(v[0]);
    if (v.size() == 1) {
      v.pop_back();
      return popped;
    }

    std::swap(v[v.size() - 1], v[0]);
    v.pop_back();

    const std::size_t sz = v.size();

    std::size_t i = 0;
    for (;;) {
      std::size_t best = (i + 1) * 2 - 1;
      // If we have no children then we're done
      if (best >= sz)
        break;
      // Pick the best of the left and right children
      if (best + 1 < sz)
        best += better(v[best + 1], v[best]) ? 1 : 0;
      // If we're better than the best child then we're done
      if (!better(v[best], v[i]))
        break;
      // Promote the best child upward
      std::swap(v[best], v[i]);
      i = best;
    }
    return popped;
  }

  void clear()
  {
    std::vector<T>().swap(v);
  }

  bool empty()
  {
    return v.empty();
  }

  std::size_t size()
  {
    return v.size();
  }

private:
  Compare better;
  std::vector<T> v;
};
}
