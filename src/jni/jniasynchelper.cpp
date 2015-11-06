#include <sma/android/jniasynchelper.hpp>
#include <sma/async.hpp>

//#include <sma/io/log.hpp>


namespace sma
{
std::multimap<std::chrono::system_clock::time_point, std::function<void()>> asynctaskqueue;
}
