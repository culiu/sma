#pragma once

#include <sma/helper.hpp>
#include <sma/ccn/interest.hpp>
#include <sma/ccn/ccnfwd.hpp>

#include <vector>
#include <cstdint>


namespace sma
{
class InterestHelper : public Helper
{
public:
  InterestHelper(CcnNode& node);

  virtual ~InterestHelper() {}

  virtual void receive(MessageHeader header, InterestAnn announcement) = 0;

  virtual void create_local(ContentType type) = 0;
  virtual void create_local(std::vector<ContentType> types) = 0;
  virtual void clear_local() = 0;
  virtual void delete_local(ContentType type) = 0;

  virtual std::vector<Interest> local() const = 0;
  virtual std::vector<Interest> all() const = 0;
  virtual std::vector<Interest> remote() const = 0;

  virtual bool interested_in(ContentMetadata const& metadata) const = 0;
  virtual bool know_remote(ContentType const& type) const = 0;
  virtual bool contains_any(std::vector<ContentType> const& types) const = 0;

  virtual std::uint16_t announce() = 0;
};
}
