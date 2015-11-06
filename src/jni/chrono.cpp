#include <sma/chrono.hpp>
//#include <sma/io/log>

namespace sma
{
namespace chrono
{
  system_clock::time_point system_clock::from_time_t(std::time_t t)
  {
    return std::chrono::time_point_cast<duration>(
        std::chrono::time_point<std::chrono::system_clock,
                                std::chrono::seconds>(std::chrono::seconds(t)));
  }

  std::time_t system_clock::to_time_t(system_clock::time_point const& tp)
  {
    return std::time_t(std::chrono::duration_cast<std::chrono::seconds>(
                           tp.time_since_epoch()).count());
  }

  std::string system_clock::strftime(system_clock::time_point const& tp,
                                     const char* format)
  {
    auto tm = system_clock::to_time_t(tp);
    char tmstr[32];
    auto size = std::strftime(tmstr, sizeof tmstr, format, std::gmtime(&tm));
    if (size)
      return std::string(tmstr);
//    else
      // So... if LOG() uses this, as it does in the NS3 implementation...
      // and the current time is overflowing........
      //
      //                                            i'm not good with computers
//      LOG(ERROR) << "std::strftime overflow converting time_point; "
//                    "resulting string is longer than " << (sizeof tmstr)
//                 << " characters.";
    return "";
  }

  std::string system_clock::utcstrftime(system_clock::time_point const& tp)
  {
    return strftime(tp, "%c %Z");
  }

  system_clock::time_point system_clock::now() 
  { 
//    auto start_time = detail::start_time_s_<system_clock>::get().start_time; 
//    return start_time + 
      return  std::chrono::system_clock::now(); 
  } 
}
}
