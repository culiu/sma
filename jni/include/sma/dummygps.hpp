#pragma once

#include <sma/gps.hpp>
#include <sma/gpscomponent.hpp>

#include <ns3/mobility-helper.h>
#include <ns3/mobility-model.h>


namespace sma
{
class DummyGps : public GpsComponent
{
public:
  DummyGps(ns3::Ptr<ns3::MobilityModel> mob)
    : mob(mob)
  {
  }
  virtual ~DummyGps() {}

  virtual GPS::Coord position() const override
  {
    auto pos = mob->GetPosition();
    return GPS::Coord{pos.x, pos.y};
  }

protected:
  ns3::Ptr<ns3::MobilityModel> mob;
};
}
