#include <sma/util/typecode.hpp>
#include <sma/util/typesieve.hpp>

#include <sma/beacon.hpp>
#include <sma/ccn/interestann.hpp>
#include <sma/ccn/contentann.hpp>
#include <sma/ccn/blockrequest.hpp>
#include <sma/ccn/blockresponse.hpp>


namespace sma
{
  using MessageTypecode = Typecode;

  using MessageTypes = TypeSieve<MessageTypecode>
    ::map<0, Beacon>
    ::map<16, InterestAnn>
    ::map<17, ContentAnn>
    ::map<18, BlockRequest>
    ::map<19, BlockResponse>;
}
