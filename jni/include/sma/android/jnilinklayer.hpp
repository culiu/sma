#pragma once

#include <sma/android/jvm.hpp>

#include <memory>
#include <cstdint>
#include <cstdlib>
#include <utility>


namespace sma
{
using packet_type = std::pair<std::unique_ptr<std::uint8_t[]>, std::size_t>;

packet_type pop_packet();
void push_packet(void const* src, std::size_t size);

void start_link_read_thread();
void stop_link_read_thread();
}