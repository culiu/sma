#include <sma/android/jnilinklayer.hpp>

#include <sma/android/bsdinetlink.hpp>

#include <sma/android/jninativeservice.hpp>

#include <thread>
#include <mutex>
#include <deque>
#include <cstring>
#include <cassert>


namespace sma
{
std::thread read_thread;
bool continue_read_links = true;

std::deque<packet_type> inbox;
std::mutex inbox_mx;

packet_type pop_packet()
{
  std::lock_guard<std::mutex> lock(inbox_mx);

  if (inbox.empty())
    return std::make_pair(nullptr, 0);

  auto packet = std::move(inbox.front());
  inbox.pop_front();
  return packet;
}

void push_packet(void const* src, std::size_t size)
{
  std::unique_ptr<std::uint8_t[]> packet_data(new std::uint8_t[size]);
  std::memcpy(reinterpret_cast<void*>(packet_data.get()), src, size);

  std::lock_guard<std::mutex> lock(inbox_mx);
  inbox.emplace_back(std::move(packet_data), size);
}

void read_link_loop()
{
  while (continue_read_links && android_service != NULL && link_instance != nullptr)
  {
    link_instance->receive_packet();
    // Tell the service thread to handle the packet
    
    JNIEnv* env = nullptr;
    get_env(env);

    assert(android_service != NULL);

    auto cls = env->GetObjectClass(android_service);
    auto packetAvailable = env->GetMethodID(cls, "packetAvailable", "()V");
    env->DeleteLocalRef(cls);
    assert(packetAvailable != NULL);

    if (continue_read_links && android_service != NULL);
    env->CallVoidMethod(android_service, packetAvailable);
  }
  //Logger("LinkLayer").d("Link-reading thread dying");
}

void start_link_read_thread()
{
  read_thread = std::thread(read_link_loop);
  read_thread.detach();
}

void stop_link_read_thread()
{
  continue_read_links = false;
}
}
